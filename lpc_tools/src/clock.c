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

