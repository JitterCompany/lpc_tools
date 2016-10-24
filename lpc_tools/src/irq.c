#include "irq.h"
#include <stdint.h>
#include <stdbool.h>

#ifndef __STATIC_INLINE
#define __STATIC_INLINE static inline
#endif
#ifndef __ASM
#define __ASM asm
#endif
#include "core_cmFunc.h"

bool irq_is_enabled(void)
{
    return !(__get_PRIMASK() & 1);
}

bool irq_disable(void)
{
    bool prev_enabled = irq_is_enabled();
    __disable_irq();
    return prev_enabled;
}

void irq_enable(void)
{
    __enable_irq();
}

void irq_restore(bool previous_value)
{
    if(previous_value) {
        irq_enable();
    }
}

