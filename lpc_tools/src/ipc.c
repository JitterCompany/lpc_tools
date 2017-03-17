#include "ipc.h"
#include "chip.h"

#if defined(MCU_PLATFORM_43xx_m0) || defined(MCU_PLATFORM_43xx_m4)
void ipc_alert_remote_processor()
{
    // make sure all data transactions complete
    // before next instruction is executed
    __DSB();

    // trigger the remote processor
    __SEV();
}

void ipc_commit_and_alert(Ringbuffer *ringbuffer)
{
    ringbuffer_commit(ringbuffer);
    ipc_alert_remote_processor();
}

#ifdef CORE_M4
void M0_boot(unsigned int m0_image_addr)
{
    // Make sure the alignment is OK
    if (m0_image_addr & 0xFFF) {
        return;
    }

    // Make sure the M0 core is being held in reset via the RGU
    Chip_RGU_TriggerReset(RGU_M0APP_RST);
    Chip_Clock_Enable(CLK_M4_M0APP);

    // Keep in mind the M0 image must be aligned on a 4K boundary
    Chip_CREG_SetM0AppMemMap(m0_image_addr);
    Chip_RGU_ClearReset(RGU_M0APP_RST);
}

void enable_M0_interrupt()
{
    NVIC_DisableIRQ((IRQn_Type)M0APP_IRQn);
    LPC_CREG->M0APPTXEVENT = 0;
    NVIC_ClearPendingIRQ((IRQn_Type)M0APP_IRQn);
    NVIC_EnableIRQ((IRQn_Type)M0APP_IRQn);
}
#endif

#ifdef CORE_M0
void enable_M4_interrupt()
{
    NVIC_DisableIRQ((IRQn_Type)M4_IRQn);
    LPC_CREG->M4TXEVENT = 0;
    NVIC_ClearPendingIRQ((IRQn_Type)M4_IRQn);
    NVIC_EnableIRQ((IRQn_Type)M4_IRQn);
}
#endif
#endif

