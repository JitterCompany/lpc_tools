#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H

#include "GPIO_HAL_LPC.h"
#include "chip.h"
#include <stddef.h>

enum GPIODirectionConfig {
    GPIO_CFG_DIR_INPUT,
    GPIO_CFG_DIR_OUTPUT_HIGH,
    GPIO_CFG_DIR_OUTPUT_LOW,
    GPIO_CFG_DIR_INVALID
};

typedef struct {
    GPIO IO;
    enum GPIODirectionConfig dir;
} GPIOConfig;

enum GPIO_ID;

enum ADCConfig {
    ADC_CFG_CH0     = ADC_CH0,
    ADC_CFG_CH1     = ADC_CH1,
    ADC_CFG_CH2     = ADC_CH2,
    ADC_CFG_CH3     = ADC_CH3,
    ADC_CFG_CH4     = ADC_CH4,
    ADC_CFG_CH5     = ADC_CH5,
    ADC_CFG_CH6     = ADC_CH6,
    ADC_CFG_CH7     = ADC_CH7,

    ADC_CFG_INVALID
};

typedef struct {
    IRQn_Type irq;
    uint32_t priority;
} NVICConfig;


#if defined(MCU_PLATFORM_lpc43xx)
    #define PinMuxConfig PINMUX_GRP_T

#elif defined(MCU_PLATFORM_lpc11xxx) || defined(MCU_PLATFORM_11uxx)

    // Some variants have weird randomized IOCON register addresses.
    // The chip library handles this by defining all port+pin combinations in
    // the CHIP_IOCON_PIO_T enumeration.
    #if !defined(CHIP_LPC11UXX) && !defined(CHIP_LPC11EXX) && !defined(CHIP_LPC11AXX)
        #define QUIRK_IOCON_11XX
        typedef struct {
            CHIP_IOCON_PIO_T iocon_offset;
            uint16_t modefunc;
        } PinMuxConfig;
    #else
        typedef struct {
            uint8_t pingrp;
            uint8_t pinnum;
            uint16_t modefunc;
        } PinMuxConfig;
    #endif

#else
    #error "No supported MCU_PLATFORM_* found!"
#endif



typedef struct {
    const NVICConfig *nvic_configs;
    const size_t nvic_count;

    const PinMuxConfig *pinmux_configs;
    const size_t pinmux_count;

    const GPIOConfig *GPIO_configs;
    const size_t GPIO_count;

    const enum ADCConfig *ADC_configs;
    const size_t ADC_count;

} BoardConfig;

/**
 * Set the board config.
 * This is required before using any other board_ functions
 */
void board_set_config(const BoardConfig *config);

void board_setup_NVIC(void);
void board_setup_pins(void);

/**
 * Get the GPIO according to the given ID.
 *
 * NOTE: depending on the project, the BoardConfig may contain GPIOs that are
 * configured as GPIO_CFG_DIR_INVALID (or no config may be set at all (yet))!
 * If this is the case, @see board_has_GPIO() to handle this correctly!
 */
const GPIO *board_get_GPIO(unsigned int ID);

/**
 * Test if the board has a valid GPIO for the specified GPIO ID
 *
 * Depending on the project, the BoardConfig may contain GPIOs that are
 * configured as GPIO_CFG_DIR_INVALID (or no config may be set at all (yet)).
 * 
 * @return      True if the GPIO ID is valid: board_get_GPIO() is guaranteed
 *              to return a valid GPIO pin in this case.
 *              False if invalid: board_get_GPIO() would trigger an assertion
 *              failure
 */
bool board_has_GPIO(unsigned int ID);


/**
 * Get the ADC according to the given ID.
 *
 * Similar to board_get_GPIO(), but for ADC channels.
 */
enum CHIP_ADC_CHANNEL board_get_ADC(unsigned int ID);

/**
 * Test if the board has a valid ADC channel for the specified ADC ID.
 *
 * Similar to board_get_GPIO(), but for ADC channels.
 */
bool board_has_ADC(unsigned int ID);

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

