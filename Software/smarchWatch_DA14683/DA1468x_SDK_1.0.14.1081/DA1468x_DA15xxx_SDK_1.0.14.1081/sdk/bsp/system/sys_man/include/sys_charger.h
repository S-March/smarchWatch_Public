/**
 * \addtogroup BSP
 * \{
 * \addtogroup SYSTEM
 * \{
 * \addtogroup USB_CHARGER
 * 
 * \brief USB Charger
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file sys_charger.h
 *
 * @brief USB Charger header file.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_CHARGER_H_
#define SYS_CHARGER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <osal.h>

typedef enum usb_charger_states {
        USB_CHARGER_IDLE = 0,
        USB_CHARGER_STABILIZING,
        USB_CHARGER_CONTACT_DET,
        USB_CHARGER_PRIMARY_DET,
        USB_CHARGER_SECONDARY_DET,
        USB_CHARGER_APPLE_DET,
        USB_CHARGER_ACA_DET,
        USB_CHARGER_ATTACHED,
        USB_CHARGER_DETACHED,
        USB_CHARGER_SDP,
        USB_CHARGER_CHARGING,
        USB_CHARGER_CONNECT_PENDING,
        USB_CHARGER_CHARGING_CONNECTED,
        USB_CHARGER_PAUSED,
} usb_charger_state_t;

typedef enum usb_charging_states {
        USB_CHARGING_OFF = 0,
        USB_CHARGING_BLOCKED,
        USB_PRE_CHARGING_ON,
        USB_CHARGING_ON,
} usb_charging_state_t;

typedef enum {
        TEMP_LIMIT = 0,
        OVER_VOLTAGE,
        CHARGING_TIMEOUT,
        CHARGING_CC_TIMEOUT,
        CHARGING_CV_TIMEOUT,
        ENTER_USB_SUSPEND,
} usb_charging_abort_status_t;

#define USB_CHARGER_LOW_BATTERY         (1 << 1)
#define USB_CHARGER_ATTACH              (1 << 2)
#define USB_CHARGER_DETACH              (1 << 3)
#define USB_CHARGER_STABLE              (1 << 4)
#define USB_CHARGER_ABORT               (1 << 5)
#define USB_CHARGER_RESET               (1 << 6)
#define USB_CHARGER_SUSPEND             (1 << 7)
#define USB_CHARGER_RESUME              (1 << 8)

#if (DEBUG_USB_CHARGER == 1) || (DEBUG_USB_CHARGER_FSM == 1)
void usb_printf(const char *fmt, ...);

#else

#define usb_printf(a, ...)
#endif

#define debug_print(fmt, ...) \
            do { if (DEBUG_USB_CHARGER_PRINT) usb_printf(fmt, __VA_ARGS__); } while (0)

extern OS_TASK xUSBChrgTaskHandle;

/**
 * \brief Handles the initialization of the USB Charger.
 *
 */
void usb_charger_init(void);

/**
 * \brief Informs the USB Charger that the USB has connected.
 *
 * \details The application is allowed to set the current limit for the charger. In most cases the
 *        current limit will be set to dg_configBATTERY_CHARGE_CURRENT as the bMaxPower will be set
 *        to 500mA and the active power consumption of the device is expected to be less than 100mA.
 *
 * \param[in] curr_lim The current limit negotiated with the Host.
 *
 */
void usb_charger_connected(uint32_t curr_lim);

/**
 * \brief Informs the USB Charger that the USB has disconnected.
 *
 */
void usb_charger_disconnected(void);

/**
 * \brief Resets the pre-charging current to the default level.
 *
 * \warning The pre-charge current will be changed only if the charger is in pre-charging state!
 *
 */
void usb_reset_precharge_current(void);

/**
 * \brief Sets the pre-charging current to a level.
 *
 * \param[in] current The pre-charging current level. The value must be in the range [16, 29] as
 *         described in the comments for the dg_configBATTERY_PRECHARGE_CURRENT in bsp_defaults.h.
 *
 * \warning The pre-charge current will be changed only if the charger is in pre-charging state!
 *
 */
void usb_set_precharge_current(uint16_t current);

/**
 * \brief Checks if Vbat is too low.
 *
 * \return True, if the battery voltage is too low, else false.
 *
 * \warning Interrupts must be disabled before calling this function.
 *
 */
bool usb_charger_is_battery_low(void);

/**
 * \brief Callback fired on interrupt from Charger
 *
 * \param [in] status interrupt events status.
 *
 */
void hw_charger_usb_cb(uint16_t status);

/**
 * \brief Callback fired to inform the USB Charger that the USB Node has been suspended.
 *
 */
void hw_charger_suspended_cb(void);

/**
 * \brief Callback fired to inform the USB Charger that the USB Node has been resumed.
 *
 */
void hw_charger_resumed_cb(void);

/**
 * \brief Callback fired on interrupt from VBUS monitoring
 *
 * \param [in] state VBUS state: true for rising, false for falling.
 *
 */
void hw_charger_vbus_cb(bool state);

#endif /* SYS_CHARGER_H_ */

/**
 \}
 \}
 \}
 */
