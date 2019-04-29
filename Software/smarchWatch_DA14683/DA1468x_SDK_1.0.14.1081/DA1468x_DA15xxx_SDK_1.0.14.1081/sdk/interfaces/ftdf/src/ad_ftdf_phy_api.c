/**
 ****************************************************************************************
 *
 * @file ad_ftdf_phy_api.c
 *
 * @brief FTDF FreeRTOS Adapter
 *
 * Copyright (C) 2014-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifdef CONFIG_USE_FTDF

#ifdef FTDF_PHY_API

#include <string.h>
#include <stdbool.h>

#include "sdk_defs.h"
#include "ad_ftdf.h"
#include "ad_ftdf_phy_api.h"
#include "internal.h"
#if FTDF_DBG_BUS_ENABLE
#include "hw_gpio.h"
#endif /* FTDF_DBG_BUS_ENABLE */

#ifndef OS_FREERTOS
static unsigned long uxCriticalNesting = 0xaaaaaaaa;

void vPortEnterCritical( void )
{
        portDISABLE_INTERRUPTS();
        uxCriticalNesting++;
        __asm volatile( "dsb" );
        __asm volatile( "isb" );
}

/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{

        uxCriticalNesting--;
        if ( uxCriticalNesting == 0 )
        {
                portENABLE_INTERRUPTS();
        }
}
#endif /* !OS_FREERTOS */

/**
 * @brief ftdf_gen_irq interrupt service routine
 */
void FTDF_GEN_Handler( void)
{
        ftdf_confirm_lmac_interrupt();
        ftdf_event_handler();
}
/**
 * @brief ftdf_wakup_irq interrupt service routine
 */
void FTDF_WAKEUP_Handler( void)
{
        ad_ftdf_wake_up_async();
}

void ad_ftdf_set_ext_address( ftdf_ext_address_t address)
{
        u_ext_address = address;
}

ftdf_ext_address_t ad_ftdf_get_ext_address( void)
{
        return u_ext_address;
}

void ad_ftdf_wake_up_ready( void)
{
}

ftdf_status_t ad_ftdf_send_frame_simple( ftdf_data_length_t    frameLength,
                                   ftdf_octet_t          *frame,
                                   ftdf_channel_number_t channel,
                                   ftdf_pti_t           pti,
                                   ftdf_boolean_t       csmaSuppress)
{
        ftdf_critical_var();
        ftdf_enter_critical();
        if (ftdf_tx_in_progress == FTDF_TRUE) {
                ftdf_exit_critical();
                return FTDF_TRANSPARENT_OVERFLOW;
        }
        ftdf_tx_in_progress = FTDF_TRUE;
        ftdf_exit_critical();

        if (sleep_status == BLOCK_SLEEPING) {
                /* Wake-up the block */
                ad_ftdf_wake_up_async();
#if FTDF_DBG_BUS_ENABLE
                ftdf_check_dbg_mode();
#endif /* FTDF_DBG_BUS_ENABLE */
                sleep_status = BLOCK_ACTIVE;
        }
        return ftdf_send_frame_simple(frameLength, frame, channel, pti, csmaSuppress);
}


void ad_ftdf_sleep_when_possible( ftdf_boolean_t allow_deferred_sleep)
{
        sleep_when_possible(allow_deferred_sleep, 0);
}

void ad_ftdf_wake_up(void)
{
        if (sleep_status == BLOCK_SLEEPING) {
                /* Wake-up the block */
                ad_ftdf_wake_up_async();
#if FTDF_DBG_BUS_ENABLE
                ftdf_check_dbg_mode();
#endif /* FTDF_DBG_BUS_ENABLE */
                sleep_status = BLOCK_ACTIVE;
        }

}

void ad_ftdf_init_phy_api(void)
{
        NVIC_ClearPendingIRQ(FTDF_WAKEUP_IRQn);
        NVIC_EnableIRQ(FTDF_WAKEUP_IRQn);

        NVIC_ClearPendingIRQ(FTDF_GEN_IRQn);
        NVIC_EnableIRQ(FTDF_GEN_IRQn);
        sleep_status = BLOCK_ACTIVE;
#ifndef OS_FREERTOS
        uxCriticalNesting = 0;
#endif

        ftdf_reset(1);
}
#endif /* FTDF_PHY_API */
#endif
