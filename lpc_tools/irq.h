#ifndef IRQ_H
#define IRQ_H
#include <stdbool.h>

typedef bool irq_state_t;

/* IRQ enable / disable functions
 *
 * These can be used to create a critical section: a small piece of code that
 * needs to run without being interrupted.
 *
 * For example: when accessing data that is also accessed
 * from interrupt context, timing issues can corrupt the state
 * unless atomic access is guaranteed. This can be fixed by temporarily
 * disabling interrupts while performing actions on this shared data.
 *
 * Interrupts that become pending during the critical section
 * are preserved and triggered after re-enabling the interrupts.
 * This ensures no interrupts are missed, unless the critical section is
 * long enough that the same interrupt source may have triggered twice.
 *
 * Example code:

    bool irq_was_previously_enabled = irq_disable();

    volatile_global_a = 1;   // example: you can write multiple globals
                             // without the risk of an interrupt triggering
    volatile_global_b = 3;   // in the middle and reading an invalid combination
                             // of global_a and global_b.
                             //
    if(irq_was_previously_enabled) { // make sure to only enable the irq
        irq_enable();                // if it was previously enabled. Otherwise
                                     // problems may arise if you call this code
                                     // within another critical section.
    }
 */

/* Check if irq interrupts for the current CPU are enabled.
 * Usefull for diagnostics or when writing nested critical sections.
 * The same result is returned by irq_disable()
 *
 * @return      interrupt status: true if enabled, false if not
 */
bool irq_is_enabled(void);

/* Disable irq interrupts for the current CPU.
 * Interrupts are still being generated (e.g. by other peripherals),
 * but are not processed by irq handlers untill after irq_enable() is called.
 *
 * @return      interrupt status before disabling:
 *              true if it was enabled, false if not
 */
bool irq_disable(void);

/* Re-enable irq interrupts
 * After calling this functions, any pending interrupts are triggered.
 *
 * Note: only enable if irq was enabled before, see the return value of
 * irq_is_enabled() or irq_is_enabled() to avoid problems with nested
 * critical sections. See irq_restore().
 *
 * Note: make sure to keep the time between irq_disable() and irq_enable() short
 * to minimize jitter between interrupts. Also, if a fast interrupt source
 * generates multiple interrupts during the time between disable and enable,
 * all except one would be missed.
 */
void irq_enable(void);

/* Restore the irq enable status
 * This is a wrapper around irq_enable, it only calls enable if the previous
 * value is true. Convenient for making critical sections:
 * simply pass the return value from a previous irq_disable() and you're done.
 */
void irq_restore(bool previous_value);

#endif

