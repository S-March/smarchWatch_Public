/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup USB_Charger
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_usb_charger.c
 *
 * @brief Implementation of the USB Charger Low Level Driver.
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_USB_CHARGER

#include <stdint.h>
#include "hw_usb_charger.h"

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_USB_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_USB_ISR_EXIT()
#endif

extern void hw_charger_usb_cb(uint16_t status) __attribute__((weak));
extern void hw_charger_vbus_cb(bool state) __attribute__((weak));

bool hw_charger_check_vbus(void)
{
        uint16_t status = CRG_TOP->ANA_STATUS_REG;
        uint16_t mask = CRG_TOP_ANA_STATUS_REG_VBUS_AVAILABLE_Msk |
                        CRG_TOP_ANA_STATUS_REG_COMP_VBUS_LOW_Msk |
                        CRG_TOP_ANA_STATUS_REG_COMP_VBUS_HIGH_Msk |
                        CRG_TOP_ANA_STATUS_REG_LDO_SUPPLY_USB_OK_Msk;

        return ( ((status & mask) == mask) && !(status & CRG_TOP_ANA_STATUS_REG_LDO_SUPPLY_VBAT_OK_Msk));
}

void hw_charger_enable_vbus_irq(void)
{
        hw_charger_clear_vbus_irq();
        NVIC_EnableIRQ(VBUS_IRQn);
}

void hw_charger_enable_charger_irq(void)
{
        REG_SET_BIT(USB, USB_MAMSK_REG, USB_M_CH_EV);

        if (dg_configUSE_USB_ENUMERATION == 0) {
                hw_charger_get_charger_irq_status();            // Clear any pending interrupt
                NVIC_EnableIRQ(USB_IRQn);
        }
}

#if (dg_configUSE_USB_CHARGER == 1) && (dg_configUSE_USB_ENUMERATION == 0)
void USB_Handler(void)
{
        SEGGER_SYSTEMVIEW_USB_ISR_ENTER();

        uint16_t maev;

        maev = hw_charger_get_charger_irq_status();
        if (maev & USB_USB_MAEV_REG_USB_CH_EV_Msk) {
                uint16_t status = hw_charger_get_status();

                if (hw_charger_usb_cb) {
                        hw_charger_usb_cb(status);
                }
        }

        SEGGER_SYSTEMVIEW_USB_ISR_EXIT();
}
#endif // dg_configUSE_USB_CHARGER && dg_configUSE_USB_ENUMERATION

void VBUS_Handler(void)
{
        SEGGER_SYSTEMVIEW_USB_ISR_ENTER();

        if (dg_configUSE_USB == 1) {
                hw_charger_clear_vbus_irq();                                    // Clear IRQ

                if (REG_GETF(CRG_TOP, VBUS_IRQ_MASK_REG, VBUS_IRQ_EN_RISE)) {   // Rising
                        hw_charger_vbus_cb(true);
                }

                if (REG_GETF(CRG_TOP, VBUS_IRQ_MASK_REG, VBUS_IRQ_EN_FALL)) {   // Falling
                        hw_charger_vbus_cb(false);
                }
        }

        SEGGER_SYSTEMVIEW_USB_ISR_EXIT();
}

void hw_charger_configure_usb_pins(void)
{
        GPIO->P11_MODE_REG = 0x026;             // Configure USB pads.
        GPIO->P22_MODE_REG = 0x026;
}

#endif /* dg_configUSE_HW_USB_CHARGER */
/**
 * \}
 * \}
 * \}
 */
