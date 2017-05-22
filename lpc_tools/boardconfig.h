#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H

#include "GPIO_HAL_LPC.h"
#include "chip.h"
#include <stddef.h>

enum GPIODirectionConfig {
    GPIO_CFG_DIR_INPUT,
    GPIO_CFG_DIR_OUTPUT_HIGH,
    GPIO_CFG_DIR_OUTPUT_LOW
};

typedef struct {
    GPIO IO;
    enum GPIODirectionConfig dir;
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

/**
 * Backup / restore nvic settings to / from a buffer
 */
bool board_NVIC_backup(uint32_t *buffer, const size_t sizeof_buffer);
bool board_NVIC_restore(const uint32_t *buffer, const size_t sizeof_buffer);

/**
 * Disable all interrupts with priority >= priority_threshold
 */
void board_NVIC_disable(const uint32_t priority_threshold);

#endif

