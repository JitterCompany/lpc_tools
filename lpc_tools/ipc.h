#ifndef IPC_H
#define IPC_H

/**
 * Send interrupt to other cpu
 */
void ipc_alert_remote_processor();

#ifdef CORE_M4

/**
 * Boot M0 core given an address for the code location
 */
void M0_boot(unsigned int m0_image_addr);

/**
 * Enable interrupt from M0 core
 */
void enable_M0_interrupt();
#endif

#ifdef CORE_M0
/**
 * Enable interrupt from M4 core
 */
void enable_M4_interrupt();
#endif

#endif
