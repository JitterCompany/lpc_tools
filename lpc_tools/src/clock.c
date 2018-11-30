#include "clock.h"
#include "chip.h"
#include <c_utils/max.h>
#include <c_utils/round.h>

#define clk_freq_pll_mid_level  100000000

// Delay using a loop (deprecated).
static void _delay_loop_us(uint32_t clk_freq, uint32_t us)
{
    uint32_t wait = clk_freq * (us / 1000000.0);
    volatile uint32_t i = 0;
    while (i++ < wait);
}


#if defined(MCU_PLATFORM_43xx_m0) || defined(MCU_PLATFORM_43xx_m4)
static void PLL1_configure(unsigned int nsel, unsigned int msel,
        unsigned int psel, unsigned int direct)
{
    uint32_t PLLReg = LPC_CGU->PLL1_CTRL;

    // set input source to crystal
    PLLReg &= ~(0x1F << 24);
    PLLReg |= (CLKIN_CRYSTAL << 24);

    PLLReg &= ~((1      << 1)  // BYPASS
            |	(1      << 6)  // FBSEL
            | 	(1      << 7)  // DIRECT
            |	(0x03   << 8)  // PSEL
            |   (0xFF   << 16) // MSEL
            |   (0x03   << 12) // NSEL
            );
    PLLReg |= (msel << 16)
        | (nsel << 12)
        | (psel << 8)
        | (direct << 7)
        | (0 << 6);             // FBSEL: 0 = from CCO
                                // (FBSEL=1 may be more energy efficient)

    LPC_CGU->PLL1_CTRL = PLLReg & ~(1 << 0);
}

static unsigned int set_clock_pll(unsigned int target_freq)
{
    // freq shoud be >= 10Mhz to use the pll (limit of post divider)
    target_freq = max(10000000, target_freq);

	int m = 0, n = 3, p_sel = -1, p_divide = 1, direct = 1;

    uint32_t cco_rate = target_freq;
    while((cco_rate < 156000000) && (p_sel < 3)) {
        cco_rate*= 2;
        p_sel++;
        p_divide*=2;
        direct = 0;
    }
    p_sel = max(0, p_sel);

    int in_freq = OscRateIn / n;
    m = divide_round_up(cco_rate, in_freq);

    unsigned int result_freq;
    while((result_freq = (in_freq*m)/p_divide) > MAX_CLOCK_FREQ) {
        m--;
        if(m <= 0) {
            return 0; // fail: required frequency too high
        }
    }

    PLL1_configure(n-1, m-1, p_sel, direct);

    return result_freq;
}

void clock_set_frequency(unsigned int freq)
{
    // set worst case flash acceleration timing
    Chip_CREG_SetFlashAcceleration(MAX_CLOCK_FREQ);

    // Switch main system clocking to IRC
    Chip_Clock_SetBaseClock(CLK_BASE_MX, CLKIN_IRC, true, false);
    _delay_loop_us(OscRateIn, 250);
    // Switch main system clocking to crystal
    Chip_Clock_EnableCrystal();

    // Setup PLL for 100MHz and switch main system clocking
    set_clock_pll(clk_freq_pll_mid_level);

    Chip_Clock_SetBaseClock(CLK_BASE_MX, CLKIN_MAINPLL, true, false);
    _delay_loop_us(clk_freq_pll_mid_level, 50);

    // Setup PLL for maximum clock
    unsigned int result_freq = set_clock_pll(freq);
    _delay_loop_us(result_freq, 50);

    // set fastest allowable flash acceleration timing
    Chip_CREG_SetFlashAcceleration(result_freq);

    SystemCoreClock = result_freq;
}
#endif

// LPC11XXX
// 1. The input frequency range is 10 MHz to 25 MHz
// 2. CCO frequency range is 156 MHz to 320 MHz
// 3. These clocks are either divided by 2*P by the programmable
// post divider to create the output clocks, or are sent directly
// to the outputs.
// 4. The main output clock is then divided by M by the
// programmable feedback divider to generate the feedback clock.

#if defined(MCU_PLATFORM_lpc11xxx)

#define LPC11XXX_MAX_FREQ 50*1e6
#define LPC11XXX_MAX_FCCO 320*1e6
#define LPC11XXX_MIN_FCCO 156*1e6

static unsigned int set_clock_pll(unsigned int target_freq)
{
    //  1. Specify the input clock frequency Fclkin.
    // => OscRateIn should be defined by the application code

    if (target_freq > LPC11XXX_MAX_FREQ) {
        target_freq = LPC11XXX_MAX_FREQ;
    }

    // No fractional divider, so adjust target_freq to
    // closest lower possible frequency.
    uint32_t f_adj = target_freq % OscRateIn;
    if (f_adj < target_freq) {
        target_freq -= f_adj;
    }

    // 2. Calculate M to obtain the desired output frequency Fclkout with M = Fclkout / Fclkin.
    uint32_t M = target_freq / OscRateIn;
    if (M < 1) {
        M = 1;
    }
    if (M > 32) {
        M = 32;
    }

    uint32_t Fclkout = OscRateIn * M;

    // 3. Find avalue so that FCCO = 2 * P * Fclkout.
    uint32_t FCCO = 0;
    uint32_t P_sel = 0;
    while(FCCO < LPC11XXX_MIN_FCCO && (P_sel <= 3)) {
        uint32_t P = (1 << P_sel);
        FCCO = 2 * P * Fclkout;
        P_sel++;
    }

    // Determine parameters to configure the register.
    // subtract 1, to get true value for P_sel,
    // because we incremented once unneccessary.
    P_sel--;
    uint32_t M_sel = M - 1;

    Chip_Clock_SetupSystemPLL(M_sel, P_sel);

    return Fclkout;
}

void clock_set_frequency(unsigned int freq)
{
	/* Powerup main oscillator */
	Chip_SYSCTL_PowerUp(SYSCTL_POWERDOWN_SYSOSC_PD);

	/* Wait 200us for OSC to be stablized, no status
	   indication, dummy wait. */
    _delay_loop_us(OscRateIn, 200);

	/* Set system PLL input to main oscillator */
	Chip_Clock_SetSystemPLLSource(SYSCTL_PLLCLKSRC_MAINOSC);

	/* Power down PLL to change the PLL divider ratio */
	Chip_SYSCTL_PowerDown(SYSCTL_POWERDOWN_SYSPLL_PD);

    uint32_t actual_freq = set_clock_pll(freq);

	/* Powerup system PLL */
	Chip_SYSCTL_PowerUp(SYSCTL_POWERDOWN_SYSPLL_PD);

	/* Wait for PLL to lock */
	while (!Chip_Clock_IsSystemPLLLocked()) {}

	/* Set system clock divider to 1 */
	Chip_Clock_SetSysClockDiv(1);

    FMC_FLASHTIM_T flash_access_time = FLASHTIM_50MHZ_CPU;
    if (actual_freq < 20e6) {
        flash_access_time = FLASHTIM_20MHZ_CPU;
    } else if (actual_freq < 40e6) {
        flash_access_time = FLASHTIM_40MHZ_CPU;
    }
	/* Setup FLASH access to 3 clocks */
	Chip_FMC_SetFLASHAccess(flash_access_time);

	/* Set main clock source to the system PLL. This will drive 48MHz
	   for the main clock and 48MHz for the system clock */
	Chip_Clock_SetMainClockSource(SYSCTL_MAINCLKSRC_PLLOUT);
}

#endif
