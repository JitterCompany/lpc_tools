#ifndef CLOCK_H
#define CLOCK_H

#if defined(MCU_PLATFORM_43xx_m0) || defined(MCU_PLATFORM_43xx_m4)
/**
 * Set clock frequency for main clock
 *
 * freq: int
 *      desired frequency, max is 204 MHz
 */
void clock_set_frequency(unsigned int freq);
#endif

#endif
