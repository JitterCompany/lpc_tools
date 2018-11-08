#ifndef GPIO_HAL_LPC_H
#define GPIO_HAL_LPC_H

#include "GPIO_HAL.h"
#include <stdint.h>
#include <chip.h>

#if defined(MCU_PLATFORM_43xx_m0) || defined(MCU_PLATFORM_43xx_m4)
#define GPIO_PERIPHERAL LPC_GPIO_PORT
#elif defined(MCU_PLATFORM_11uxx) || defined(MCU_PLATFORM_lpc11xxx)
#define GPIO_PERIPHERAL LPC_GPIO
#else
#error "Unsupported MCU platform!"
#endif



struct GPIOImplementation {
    uint8_t port;
    uint8_t pin;
};

void GPIO_HAL_init(GPIO *gpio,
        const uint8_t port, const uint8_t pin);

#endif

