#include "boardconfig.h"
#include <stddef.h>
#include "irq.h"
#include <c_utils/assert.h>

const BoardConfig *g_config = 0;

void board_set_config(const BoardConfig *config)
{
    g_config = config;
}

void board_setup_NVIC(void)
{
    if(!g_config) {
        return;
    }

    for(size_t i=0; i<g_config->nvic_count; i++) {
        NVICConfig cfg = g_config->nvic_configs[i];
        NVIC_SetPriority(cfg.irq, cfg.priority);
    }
}

bool board_NVIC_backup(uint32_t *buffer, const size_t sizeof_buffer)
{
    if(sizeof_buffer < sizeof(NVIC->ISER)) {
        return false;
    }

    const bool irq_state = irq_disable();
    const size_t len = (sizeof(NVIC->ISER)/sizeof(NVIC->ISER[0]));
    for(size_t i=0;i<len;i++) {
        buffer[i] = NVIC->ISER[i];
    }
    irq_restore(irq_state);
    return true;
}

bool board_NVIC_restore(const uint32_t *buffer, const size_t sizeof_buffer)
{
    if(sizeof_buffer < sizeof(NVIC->ISER)) {
        return false;
    }

    const bool irq_state = irq_disable();
    const size_t len = (sizeof(NVIC->ISER)/sizeof(NVIC->ISER[0]));
    for(size_t i=0;i<len;i++) {
        NVIC->ISER[i] = buffer[i];
    }
    irq_restore(irq_state);
    return true;
}

void board_NVIC_disable(const uint32_t priority_threshold)
{
    const bool irq_state = irq_disable();
    for(size_t i=0; i<g_config->nvic_count; i++) {
        NVICConfig cfg = g_config->nvic_configs[i];
        if(cfg.priority >= priority_threshold) {
            NVIC_DisableIRQ(cfg.irq);
        }
    }
    irq_restore(irq_state);
}

const GPIO *board_get_GPIO(unsigned int ID)
{
    assert(board_has_GPIO(ID));

    return &(g_config->GPIO_configs[ID].IO);
}

bool board_has_GPIO(unsigned int ID)
{
    if(!g_config) {
        return false;
    }

    const size_t max_ID = g_config->GPIO_count;
    if(ID >= max_ID) {
        return false;
    }

    return (g_config->GPIO_configs[ID].dir != GPIO_CFG_DIR_INVALID);
}

enum CHIP_ADC_CHANNEL board_get_ADC(unsigned int ID)
{
    assert(board_has_ADC(ID));

    return g_config->ADC_configs[ID];
}

bool board_has_ADC(unsigned int ID)
{
    if(!g_config) {
        return false;
    }

    const size_t max_ID = g_config->ADC_count;
    if(ID >= max_ID) {
        return false;
    }

    return (g_config->ADC_configs[ID] < ADC_CFG_INVALID);
}


static void board_setup_muxing(void)
{

#if defined(MCU_PLATFORM_lpc11xxx)

	// Chips with IOCON peripheral (e.g. lpc11uxx)
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_IOCON);

	for (size_t i=0; i<g_config->pinmux_count; i++) {
		const PinMuxConfig cfg = g_config->pinmux_configs[i];

    #if defined(QUIRK_IOCON_11XX)
		Chip_IOCON_PinMuxSet(LPC_IOCON, cfg.iocon_offset, cfg.modefunc);
    #else
		Chip_IOCON_PinMuxSet(LPC_IOCON, cfg.pingrp, cfg.pinnum, cfg.modefunc);
    #endif
	}

#else

	// Chips with SCU peripheral (e.g. lpc43xx)
    Chip_SCU_SetPinMuxing(g_config->pinmux_configs, g_config->pinmux_count);

#endif
}

static void board_setup_GPIO(void)
{
    for(size_t i=0; i<g_config->GPIO_count; i++) {
        GPIO IO = g_config->GPIO_configs[i].IO;

        switch(g_config->GPIO_configs[i].dir) {
            case GPIO_CFG_DIR_INVALID:
                break;

            case GPIO_CFG_DIR_OUTPUT_LOW: {
                GPIO_HAL_set_dir(&IO, GPIO_DIR_OUTPUT);
                GPIO_HAL_set(&IO, LOW);
                break;
            }
            case GPIO_CFG_DIR_OUTPUT_HIGH: {

                GPIO_HAL_set_dir(&IO, GPIO_DIR_OUTPUT);
                GPIO_HAL_set(&IO, HIGH);
                break;
            }
            case GPIO_CFG_DIR_INPUT:
            default: {
                GPIO_HAL_set_dir(&IO, GPIO_DIR_INPUT);
                break;
            }
        }
    }
}

void board_setup_pins(void)
{
    if(!g_config) {
        return;
    }

    board_setup_muxing();
    board_setup_GPIO();
}

