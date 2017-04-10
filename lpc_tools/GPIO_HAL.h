#ifndef GPIO_HAL_H
#define GPIO_HAL_H

#include <stdbool.h>

struct GPIOImplementation;
typedef struct GPIOImplementation GPIO;

enum GPIO_direction {
    GPIO_DIR_INPUT,
    GPIO_DIR_OUTPUT
};

enum GPIO_state {
    HIGH = true,
    LOW = false
};

void GPIO_HAL_set_dir(const GPIO *gpio, enum GPIO_direction direction);
void GPIO_HAL_set(const GPIO *gpio, enum GPIO_state state);
enum GPIO_state GPIO_HAL_get(const GPIO *gpio);

#endif

