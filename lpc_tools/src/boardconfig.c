#include "boardconfig.h"
#include <stddef.h>

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

const GPIO *board_get_GPIO(unsigned int ID)
{
    if(!g_config) {
        while(1); // fatal error
    }

    size_t max_ID = g_config->GPIO_count;
    if(ID >= max_ID) {
        while(1); // fatal error
    }
    return &(g_config->GPIO_configs[ID].IO);
}

static void board_setup_muxing(void)
{
    Chip_SCU_SetPinMuxing(g_config->pinmux_configs, g_config->pinmux_count);
}

static void board_setup_GPIO(void)
{
    for(size_t i=0; i<g_config->GPIO_count; i++) {
        GPIO IO = g_config->GPIO_configs[i].IO;

        switch(g_config->GPIO_configs[i].dir) {
            case GPIO_CFG_DIR_OUTPUT_LOW: {
                Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT,
                                          IO.port, IO.pin);
                Chip_GPIO_SetPinState(LPC_GPIO_PORT,
                                      IO.port, IO.pin, false);
                break;
            }
            case GPIO_CFG_DIR_OUTPUT_HIGH: {
                Chip_GPIO_SetPinDIROutput(LPC_GPIO_PORT,
                                          IO.port, IO.pin);
                Chip_GPIO_SetPinState(LPC_GPIO_PORT,
                                      IO.port, IO.pin, true);
                break;
            }
            case GPIO_CFG_DIR_INPUT:
            default: {
                Chip_GPIO_SetPinDIRInput(LPC_GPIO_PORT,
                                         IO.port, IO.pin);
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

