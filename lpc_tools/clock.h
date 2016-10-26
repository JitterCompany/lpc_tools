#ifndef CLOCK_H
#define CLOCK_H

/**
 * Set clock frequency for main clock
 *
 * freq: int
 *      desired frequency, max is 204 MHz
 */
void clock_set_frequency(uint32_t freq);

#endif
