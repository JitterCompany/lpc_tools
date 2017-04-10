#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H

#include "GPIO.h"
#include "chip.h"
#include <stddef.h>

enum GPIO_direction {
    GPIO_DIR_INPUT,
    GPIO_DIR_OUTPUT_HIGH,
    GPIO_DIR_OUTPUT_LOW
};

typedef struct {
    GPIO IO;
    enum GPIO_direction dir;
} GPIOConfig;

typedef struct {
    IRQn_Type irq;
    uint32_t priority;
} NVICConfig;

enum GPIO_ID;


typedef struct {
    const NVICConfig *nvic_configs;
    const size_t nvic_count;

    const PINMUX_GRP_T *pinmux_configs;
    const size_t pinmux_count;

    const GPIOConfig *GPIO_configs;
    const size_t GPIO_count;

} BoardConfig;

/**
 * Set the board config.
 * This is required before using any other board_ functions
 */
void board_set_config(const BoardConfig *config);

void board_setup_NVIC(void);
void board_setup_pins(void);

const GPIO *board_get_GPIO(unsigned int ID);

#endif

