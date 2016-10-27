#include "clock.h"
#include "chip.h"

#define clk_freq_pll_mid_level  100000000

// Delay using a loop (deprecated).
static void _delay_loop_us(uint32_t clk_freq, uint32_t us)
{
    uint32_t wait = clk_freq * (us / 1000000.0);
    volatile uint32_t i = 0;
    while (i++ < wait);
}

void clock_set_frequency(unsigned int freq)
{
    /* Switch main system clocking to IRC */
    Chip_Clock_SetBaseClock(CLK_BASE_MX, CLKIN_IRC, true, false);
    _delay_loop_us(12000000, 250);
    /* Switch main system clocking to crystal */
    Chip_Clock_EnableCrystal();

    /* Setup PLL for 100MHz and switch main system clocking */
    Chip_Clock_SetupMainPLLHz(CLKIN_CRYSTAL, OscRateIn, clk_freq_pll_mid_level,
                              clk_freq_pll_mid_level);
    Chip_Clock_SetBaseClock(CLK_BASE_MX, CLKIN_MAINPLL, true, false);
    _delay_loop_us(clk_freq_pll_mid_level, 50);
    /* Setup PLL for maximum clock */
    Chip_Clock_SetupMainPLLHz(CLKIN_CRYSTAL, OscRateIn, freq, MAX_CLOCK_FREQ);
    _delay_loop_us(freq, 50);

    SystemCoreClockUpdate();
}
