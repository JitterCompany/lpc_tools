#include "GPIO_HAL_LPC.h"

void GPIO_HAL_init(GPIO *gpio,
        const uint8_t port, const uint8_t pin)
{
    gpio->port = port;
    gpio->pin = pin;
}

void GPIO_HAL_set_dir(const GPIO *gpio, enum GPIO_direction direction)
{
    if(direction == GPIO_DIR_INPUT) {
        Chip_GPIO_SetPinDIRInput(GPIO_PERIPHERAL, gpio->port, gpio->pin);
    } else {
        Chip_GPIO_SetPinDIROutput(GPIO_PERIPHERAL, gpio->port, gpio->pin);
    }
}

void GPIO_HAL_set(const GPIO *gpio, enum GPIO_state state)
{
    Chip_GPIO_SetPinState(GPIO_PERIPHERAL, gpio->port, gpio->pin,
            (state == HIGH));
}

enum GPIO_state GPIO_HAL_get(const GPIO *gpio)
{
    return Chip_GPIO_GetPinState(GPIO_PERIPHERAL, gpio->port, gpio->pin)
        ? HIGH : LOW;
}

