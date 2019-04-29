/**
 ****************************************************************************************
 *
 * @file sys_charger.c
 *
 * @brief USB Charger
 *
 * Copyright (C) 2015-2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/**
 \addtogroup BSP
 \{
 \addtogroup SYSTEM
 \{
 \addtogroup USB_Charger
 \{
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "sdk_defs.h"
#include <stdbool.h>
#include <stdarg.h>

#include "hw_usb.h"
#include "hw_usb_charger.h"
#include "sys_charger.h"
#include "sys_power_mgr.h"
#include "sys_socf.h"
#include "hw_cpm.h"
#include "hw_gpadc.h"
#include "hw_uart.h"
#include "timers.h"
#include "qspi_automode.h"

#if (DEBUG_USB_CHARGER == 1)
static void print_vbat_measurement(uint32_t val);
#endif

static uint32_t measure_vbat_basic_via_lld(void);

#if (dg_configUSE_USB_CHARGER == 1)
#       include "platform_devices.h"
        static uint32_t measure_vbat_basic_via_adapter(void);
#       if ((dg_configBATTERY_TYPE_CUSTOM_ADC_VOLTAGE > 3700) || (dg_configCHARGING_THRESHOLD > 3700))
#               error "Configured battery values beyond range."
#       endif
#endif

#if ((dg_configUSE_USB_CHARGER == 1) && \
     ((dg_configBATTERY_TYPE == BATTERY_TYPE_NO_RECHARGE) || (dg_configBATTERY_TYPE == BATTERY_TYPE_NO_BATTERY)))
#       error "Invalid setting for dg_configBATTERY_TYPE." \
              "USB charger cannot be used with a non-rechargeable battery or in the absence of a battery."
#endif

#if (dg_configUSE_USB == 1) && (dg_configUSE_USB_CHARGER == 0)
        #pragma message "The USB is used but charging is disabled (USB-only mode)."
#endif

#define USB_CHRG_DEBOUNCE_PERIOD        10              // in multiples of 10 msec (= 100msec)
#define USB_CHRG_CONTACT_LIMIT          60              // in multiples of 10 msec (= 600msec)
#define NO_VBUS_TIMER_CNT_TO_DETACH     60
#define FINISH_CHARGER_CHECK_CNT        50

#if (DEBUG_USB_CHARGER == 1) || (DEBUG_USB_CHARGER_FSM == 1)
#pragma message "The USB charger's Debug mode is enabled."

#if (DEBUG_USB_CHARGER_PRINT == 1)
#       ifndef CONFIG_RETARGET
#               warning "The USB charger's Debug via printf() mode is enabled but RETARGET is off..."
#       else
#               warning "The USB charger's Debug via printf() mode is enabled."
#       endif
#endif

#define NO_OF_TRANSITIONS               32
#define BUFSIZE                         32

#if (DEBUG_USB_CHARGER == 1)
static char str_buf[BUFSIZE];
#endif
PRIVILEGED_DATA usb_charger_state_t chrg_state_transitions[NO_OF_TRANSITIONS];
PRIVILEGED_DATA uint32_t chrg_state_wr_ptr = 0;

static const char * const usb_charger_state_name[] = {
        "IDLE",
        "STABILIZING",
        "CONTACT_DET",
        "PRIMARY_DET",
        "SECONDARY_DET",
        "APPLE_DET",
        "ACA_DET",
        "ATTACHED",
        "DETACHED",
        "SDP",
        "CHARGING",
        "CONNECT_PENDING",
        "CHARGING_CONNECTED",
        "PAUSED",
};

static const char * const usb_charging_state_name[] = {
        "CHARGING_OFF",
        "CHARGING_BLOCKED",
        "PRE_CHARGING_ON",
        "CHARGING_ON",
};

static const char * const usb_charging_current[] = {
        "5",
        "10",
        "30",
        "45",
        "60",
        "90",
        "120",
        "150",
        "180",
        "210",
        "270",
        "300",
        "350",
        "400",
        "N/A",
        "N/A",
        "Rsv",
        "Rsv",
        "1",
        "1.5",
        "2.1",
        "3.2",
        "4.3",
        "Rsv",
        "6.6",
        "7.8",
        "Rsv",
        "11.3",
        "13.3",
        "15.3",
        "N/A",
        "N/A"
};

#if (DEBUG_USB_CHARGER_PRINT == 1)
#define USB_NO_OF_MESSAGES              16
#define USB_MESSAGE_LENGTH              64

PRIVILEGED_DATA char usb_mess_buf[USB_NO_OF_MESSAGES][USB_MESSAGE_LENGTH];
PRIVILEGED_DATA uint32_t usb_mess_idx = 0;
PRIVILEGED_DATA uint32_t usb_mess_last_printed = 0;

void usb_printf(const char *fmt, ...)
{
        va_list args;
        int next_mess_idx;

        next_mess_idx = usb_mess_idx + 1;
        next_mess_idx %= USB_NO_OF_MESSAGES;

        if (next_mess_idx == usb_mess_last_printed) { /* Overflow! */
                ASSERT_WARNING(0);
                return;
        }

        va_start(args, fmt);
        vsnprintf(usb_mess_buf[usb_mess_idx], USB_MESSAGE_LENGTH, fmt, args);
        va_end(args);

        usb_mess_idx = next_mess_idx;
}

void usb_print_messages_task(void *pParam)
{
        uint32_t ulNotifiedValue;
        OS_BASE_TYPE xResult __attribute__((unused));

        do {
                // Wait for the notification.
                xResult = OS_TASK_NOTIFY_WAIT(0x0, OS_TASK_NOTIFY_ALL_BITS, &ulNotifiedValue,
                                                                            OS_TASK_NOTIFY_FOREVER);

                ASSERT_WARNING(usb_mess_idx <= USB_NO_OF_MESSAGES);
                ASSERT_WARNING(usb_mess_last_printed <= USB_NO_OF_MESSAGES);

                while (usb_mess_last_printed != usb_mess_idx) {
                        printf("%s", usb_mess_buf[usb_mess_last_printed]);

                        OS_ENTER_CRITICAL_SECTION();
                        usb_mess_last_printed++;
                        usb_mess_last_printed %= USB_NO_OF_MESSAGES;
                        OS_LEAVE_CRITICAL_SECTION();
                }
        } while (1);
}
#endif

#else

void usb_print_messages_task(void *);
#endif

#define CHECK_APPLE_CHARGER_TYPE_1(x) (                         \
                ((x) & USB_USB_CHARGER_STAT_REG_USB_DP_VAL_Msk) && !((x) & USB_USB_CHARGER_STAT_REG_USB_DP_VAL2_Msk)      \
             && ((x) & USB_USB_CHARGER_STAT_REG_USB_DM_VAL_Msk) && !((x) & USB_USB_CHARGER_STAT_REG_USB_DM_VAL2_Msk))

#define CHECK_APPLE_CHARGER_TYPE_2(x) (                         \
                ((x) & USB_USB_CHARGER_STAT_REG_USB_DP_VAL_Msk) && !((x) & USB_USB_CHARGER_STAT_REG_USB_DP_VAL2_Msk)      \
             && ((x) & USB_USB_CHARGER_STAT_REG_USB_DM_VAL2_Msk))

#define CHECK_APPLE_CHARGER_TYPE_3(x) (                         \
                ((x) & USB_USB_CHARGER_STAT_REG_USB_DP_VAL2_Msk)                             \
             && ((x) & USB_USB_CHARGER_STAT_REG_USB_DM_VAL_Msk) && !((x) & USB_USB_CHARGER_STAT_REG_USB_DM_VAL2_Msk))

#define CHECK_APPLE_CHARGER_TYPE_4(x) (((x) & USB_USB_CHARGER_STAT_REG_USB_DP_VAL2_Msk) && ((x) & USB_USB_CHARGER_STAT_REG_USB_DM_VAL2_Msk))

#define VOLTAGE_SAMPLES (8)


/*
 * Call-backs
 *
 * These call-backs may be implemented by the application code in order to catch events sent by the
 * USB charger. Note that the USB charger task is started before the application task. Thus, these
 * call-backs may be called before the application task is started. The application code should
 * handle this case, if need be. In such a case, a possible implementation of such a call-back
 * could be:
 *
 * PRIVILEGED_DATA bool app_task_is_initialized;
 *
 * enum RCV_USB_INDICATIONS {
 *         RCV_USB_NONE,
 *         RCV_USB_ATTACH,
 *         ...
 * } app_usb_indication;
 *
 * void usb_attach_cb(void)
 * {
 *         if (app_task_is_initialized) {
 *                 // Do something
 *         } else {
 *                 // Raise a flag for the app to check when started.
 *                 app_usb_indication = RCV_USB_ATTACH;
 *         }
 * }
 *
 * void app_task(void *pvParameters)
 * {
 *         ...
 *         switch (app_usb_indication) {
 *         case RCV_USB_NONE:
 *                 break;
 *         case RCV_USB_ATTACH:
 *                 ...
 *                 break;
 *         ...
 *         default:
 *                 break;
 *         }
 *
 *         app_usb_indication = RCV_USB_NONE;
 *         app_task_is_initialized = true;
 * }
 *
 */
extern void usb_attach_cb(void) __attribute__((weak));
extern void usb_detach_cb(void) __attribute__((weak));
extern void usb_start_enumeration_cb(void) __attribute__((weak));
extern void usb_charging(void) __attribute__((weak));
extern void usb_precharging(void) __attribute__((weak));
extern void usb_precharging_aborted(void) __attribute__((weak));
extern void usb_charging_stopped(void) __attribute__((weak));
extern void usb_charging_aborted(void) __attribute__((weak));
extern void usb_charging_paused(void) __attribute__((weak));
extern void usb_charged(void) __attribute__((weak));
extern void usb_charger_battery_full(void) __attribute__((weak));
extern void usb_charger_bad_battery(void) __attribute__((weak));
extern void usb_charger_temp_low(void) __attribute__((weak));
extern void usb_charger_temp_high(void) __attribute__((weak));
extern void usb_is_suspended(void) __attribute__((weak));
extern void usb_is_resumed(void) __attribute__((weak));

static void usb_charger_timer_cb( OS_TIMER pxTimer );

PRIVILEGED_DATA static OS_TIMER xUSBChrgTimer;
PRIVILEGED_DATA OS_TASK xUSBChrgTaskHandle;
PRIVILEGED_DATA OS_TASK xUSBChrgTaskFSMHandle;
PRIVILEGED_DATA OS_TASK xUSBChrgTaskPrintHandle __attribute__((unused));
PRIVILEGED_DATA usb_charger_state_t usb_charger_state;
PRIVILEGED_DATA static bool usb_is_connected;
PRIVILEGED_DATA static usb_charging_state_t charging_state;
PRIVILEGED_DATA static bool vbat_sampling_valid;

static bool usb_contact;
static uint32_t hit_time;
static uint32_t timer_cnt;
static uint32_t precharging_timer_cnt;
static uint32_t time_in_cc_mode;
static uint32_t time_in_cv_mode;
static uint16_t configured_current_limit;
static uint32_t voltage_overflow_cnt;
static uint32_t temp_error_cnt;
static uint32_t vbat_samples[VOLTAGE_SAMPLES];
static uint32_t vbat_sampling_period;
static int vbat_current_sample;
static uint32_t pending_charge_finish_cnt = 0;
static bool detach_in_progress;
usb_charger_state_t charge_initiation_state;
static uint32_t no_vbus_timer_cnt = 0;
static uint32_t suspend_S3_requested = 0;
static uint32_t iser_status_at_suspend = 0;
static sys_clk_t sysclk_at_suspend;

#if (dg_configBATTERY_TYPE == BATTERY_TYPE_2xNIMH)
        const uint32_t charge_level = BATTERY_TYPE_2xNIMH_ADC_VOLTAGE;
#elif (dg_configBATTERY_TYPE == BATTERY_TYPE_3xNIMH)
        const uint32_t charge_level = BATTERY_TYPE_3xNIMH_ADC_VOLTAGE;
#elif (dg_configBATTERY_TYPE == BATTERY_TYPE_LICOO2)
        const uint32_t charge_level = BATTERY_TYPE_LICOO2_ADC_VOLTAGE;
#elif (dg_configBATTERY_TYPE == BATTERY_TYPE_LIMN2O4)
        const uint32_t charge_level = BATTERY_TYPE_LIMN2O4_ADC_VOLTAGE;
#elif (dg_configBATTERY_TYPE == BATTERY_TYPE_NMC)
        const uint32_t charge_level = BATTERY_TYPE_NMC_ADC_VOLTAGE;
#elif (dg_configBATTERY_TYPE == BATTERY_TYPE_LIFEPO4)
        charge_level = BATTERY_TYPE_LIFEPO4_ADC_VOLTAGE;
#elif (dg_configBATTERY_TYPE == BATTERY_TYPE_LINICOAIO2)
        const uint32_t charge_level = BATTERY_TYPE_LINICOAIO2_ADC_VOLTAGE;
#elif (dg_configBATTERY_TYPE == BATTERY_TYPE_CUSTOM)
        const uint32_t charge_level = dg_configBATTERY_TYPE_CUSTOM_ADC_VOLTAGE;
#else
        const uint32_t charge_level = 0;
#endif

void hw_charger_usb_cb(uint16_t status)
{
        if (hw_charger_check_contact(status)) {
                usb_contact = true;
                hit_time = timer_cnt;
        } else {
                usb_contact = false;
                hit_time = 0xFFFFFF;
        }

        /* Disable interrupt to avoid toggling during debouncing. The interrupt will be re-enabled
         * by the FSM and the status will be updated at that point in time.
         */
        hw_charger_disable_charger_irq();
}

void hw_charger_suspended_cb(void)
{
        pm_stay_idle();

        usb_is_suspended();
        OS_TASK_NOTIFY_FROM_ISR(xUSBChrgTaskHandle, USB_CHARGER_SUSPEND, OS_NOTIFY_SET_BITS);
}

void hw_charger_resumed_cb(void)
{
        pm_resume_sleep();

        OS_TASK_NOTIFY_FROM_ISR(xUSBChrgTaskHandle, USB_CHARGER_RESUME, OS_NOTIFY_SET_BITS);
}

void hw_charger_vbus_cb(bool state)
{
        uint32_t value = (state)? USB_CHARGER_ATTACH: USB_CHARGER_DETACH;

        OS_TASK_NOTIFY_FROM_ISR(xUSBChrgTaskHandle, value, OS_NOTIFY_SET_BITS);

        hw_charger_mask_vbus_irq();
}

#if (dg_configUSE_USB_CHARGER == 1)
/**
 * \brief Takes a measurement of Vbat via ADC.
 *
 * \return The measured Vbat value.
 *
 * \warning This function performs oversampling while each measurement takes 15 * 8 * oversampling system
 *        clock cycles. Thus, 30usec are required for each measurement and the total delay
 *        introduced by this function is at least 30 * oversampling usec.
 *
 */
static uint32_t measure_vbat_basic_via_adapter(void)
{
        uint32_t ret;
        uint16_t val;
        battery_source src = ad_battery_open();

        val = ad_battery_read(src);
        ad_battery_close(src);

        gpadc_source_config *cfg = (gpadc_source_config *) src;

        switch (cfg->hw_init.oversampling) {
        case 0: //  1*OS (10 bits - ENOB= 9.05)
                ret = val << 2;
                break;
        case 1: //  2*OS (11 bits - ENOB= 9.45)
                ret = val << 1;
                break;
        case 2: //  4*OS (12 bits - ENOB= 9.83)
                ret = val;
                break;
        case 3: //  8*OS (13 bits - ENOB=10.21)
                ret = val >> 1;
                break;
        case 4: // 16*OS (14 bits - ENOB=10.52)
                ret = val >> 2;
                break;
        case 5: // 32*OS (15 bits - ENOB=10.85)
                ret = val >> 3;
                break;
        case 6: // 64*OS (16 bits - ENOB=11.10)
                ret = val >> 4;
                break;
        case 7: //128*OS (16 bits? - ENOB=11.27)
                ret = val >> 4;
                break;
        default:
                ret = 0;
                break;
        }
#if (DEBUG_USB_CHARGER == 1)
        print_vbat_measurement(ret);
#endif

        return ret;
}
#endif

/**
 * \brief Takes a measurement of Vbat every 50msec and puts it in the measurements' buffer.
 *
 */
static void measure_vbat_normal(void)
{
        if (vbat_sampling_period == 5) {
                vbat_sampling_period = 0;
        }

        if ((charging_state == USB_PRE_CHARGING_ON)
                && (precharging_timer_cnt < dg_configPRECHARGING_INITIAL_MEASURE_DELAY)) {
                // Pause! Stop taking measurements as Vbat may be invalid during this period!

                if (vbat_sampling_period != 0) {
                        vbat_sampling_period++;
                }
        } else {
                if (vbat_sampling_period == 0) {
                        vbat_current_sample++;
                        if (vbat_current_sample == VOLTAGE_SAMPLES) {
                                vbat_current_sample = 0;
                                vbat_sampling_valid = true;
                        }
                        /* Conditional compilation to avoid warning in case
                         * dg_configUSE_USB_CHARGER is disabled.
                         */
#if (dg_configUSE_USB_CHARGER == 1)
                        vbat_samples[vbat_current_sample] = measure_vbat_basic_via_adapter();
#endif
                }

                vbat_sampling_period++;
        }
}

/**
 * \brief Returns either the last measurement of Vbat, when once is true, or the average of the last
 *        eight valid measurements.
 *
 * \param[out] vbat_meas The measured Vbat value.
 * \param[in] once If true, the last Vbat measurement taken is returned, if it is valid. Else, the
 *        average of the last eight measurements is returned, providing that these are valid.
 *
 * \return True, if the Vbat measurements that are stored in the measurements' buffer are valid,
 *        indicating that the result in vbat_meas is valid, else false.
 *
 */
static bool sample_vbat(uint32_t *vbat_meas, bool once)
{
        int i;
        bool ret = false;

        if (vbat_sampling_valid) {
                ret = true;

                if (once) {
                        *vbat_meas = vbat_samples[vbat_current_sample];
                } else {
                        *vbat_meas = 0;
                        for (i = 0; i < VOLTAGE_SAMPLES; i++) {
                                *vbat_meas += vbat_samples[i];
                        }
                        *vbat_meas /= VOLTAGE_SAMPLES;
                }
        }

        return ret;
}

/**
 * \brief Used in debug mode to keep track of the charging state transitions.
 *
 */
#if (DEBUG_USB_CHARGER_FSM == 1)

static void print_charging_state_transitions(void)
{
        debug_print("USB Charging state: %s\r\n", usb_charging_state_name[(int)charging_state]);
}

#else

#define print_charging_state_transitions()
#endif

/**
 * \brief Used in debug mode to keep track of the FSM state transitions.
 *
 */
#if (DEBUG_USB_CHARGER_FSM == 1)

static void update_state_transitions(void)
{
        if (chrg_state_transitions[chrg_state_wr_ptr] != usb_charger_state) {
                chrg_state_wr_ptr++;
                chrg_state_wr_ptr %= NO_OF_TRANSITIONS;
                chrg_state_transitions[chrg_state_wr_ptr] = usb_charger_state;
        }
}

#if (DEBUG_USB_CHARGER_PRINT == 1)
static void print_state_transitions(void)
{
        debug_print("USB FSM state: %s\r\n",
                usb_charger_state_name[(int)usb_charger_state]);
}

#else
#define print_state_transitions()
#endif

#else
#define update_state_transitions()
#define print_state_transitions()
#endif

/**
 * \brief Handles the critical part of the USB cable attach procedure.
 *
 */
static void usb_attach_critical(void)
{
        hw_charger_set_vbus_irq_low();

        timer_cnt = 0;
        precharging_timer_cnt = 0;
        voltage_overflow_cnt = 0;
        temp_error_cnt = 0;
        usb_contact = false;
        detach_in_progress = false;
        hit_time = 0;
        pending_charge_finish_cnt = 0;
        vbat_current_sample = -1;
        vbat_sampling_period = 0;
        vbat_sampling_valid = false;
        suspend_S3_requested = 0;

        hw_charger_enable_usb_pads_passive();

        hw_charger_start_contact_detection();
        hw_charger_enable_charger_irq();

        usb_charger_state = USB_CHARGER_CONTACT_DET;
        print_state_transitions();

        pm_stay_alive();                                // Powered from VBUS so sleep is blocked.
        if (dg_configUSE_DCDC == 1) {
                hw_cpm_switch_to_ldos();                // Switch to LDO operation.
        }
}

/**
 * \brief Handles the USB cable attach procedure.
 *
 */
static void usb_attach(void)
{
        OS_ENTER_CRITICAL_SECTION();

        DBG_CONFIGURE_HIGH(USB_CHARGER_TIMING_DEBUG, CHRGDBG_CRITICAL_SECTION);

        if (hw_charger_check_vbus()) {
                usb_attach_critical();
        } else {
                timer_cnt = 0;
                usb_charger_state = USB_CHARGER_STABILIZING;
                print_state_transitions();
        }

        DBG_CONFIGURE_LOW(USB_CHARGER_TIMING_DEBUG, CHRGDBG_CRITICAL_SECTION);

        OS_LEAVE_CRITICAL_SECTION();

        usb_attach_cb();                                // Notify Application task (opt.)

        no_vbus_timer_cnt = 0;

        OS_TIMER_RESET(xUSBChrgTimer, OS_TIMER_FOREVER);// Start charger FSM.
}

/**
 * \brief Handles the USB cable detach procedure.
 *
 */
static void usb_detach(void)
{
        if (hw_charger_check_vbus()) {
                hw_charger_set_vbus_irq_low();
                return;                                 // Do not detach yet as it was only a sink of VBUS.
        }

        detach_in_progress = true;
}

#if (DEBUG_USB_CHARGER == 1)
/**
 * \brief Prints charging time over UART.
 *
 */
static void print_statistics(uint32_t time, int mode)
{
        uint32_t digit;
        uint32_t value = time / 100;

        if (mode == 0) {
                str_buf[0] = 'C';
                str_buf[1] = 'H';
        } else if (mode == 1) {
                str_buf[0] = 'C';
                str_buf[1] = 'C';
        } else {
                str_buf[0] = 'C';
                str_buf[1] = 'V';
        }
        str_buf[2] = ':';
        str_buf[3] = ' ';

        digit = value / 10000;
        str_buf[4] = 0x30 + digit;
        value -= digit * 10000;

        digit = value / 1000;
        str_buf[5] = 0x30 + digit;
        value -= digit * 1000;

        digit = value / 100;
        str_buf[6] = 0x30 + digit;
        value -= digit * 100;

        digit = value / 10;
        str_buf[7] = 0x30 + digit;
        value -= digit * 10;

        str_buf[8] = 0x30 + value;

        str_buf[9]  = 's';
        str_buf[10] = 'e';
        str_buf[11] = 'c';
        str_buf[12] = '\r';
        str_buf[13] = '\n';
        str_buf[14] = '\0';

        hw_uart_send(HW_UART2, str_buf, strlen(str_buf)+1, NULL, NULL);
}
#endif

/**
 * \brief Charging and detection stop.
 *
 */
static void stop_charging(void)
{
#if (DEBUG_USB_CHARGER == 1)
        if (dg_configCHARGING_TIMEOUT != 0) {
                print_statistics(timer_cnt, 0);
        } else {
                print_statistics(time_in_cc_mode, 1);
                print_statistics(time_in_cv_mode, 2);
        }
#endif

        timer_cnt = 0;
        precharging_timer_cnt = 0;
        time_in_cc_mode = 0;
        time_in_cv_mode = 0;
        voltage_overflow_cnt = 0;
        temp_error_cnt = 0;
        vbat_current_sample = -1;
        vbat_sampling_period = 0;

        if (dg_configUSE_USB_ENUMERATION == 0) {
                hw_charger_disable_usb_node();          // Disable USB pad pull-up
        }

        if (hw_charger_is_charging()) {
                hw_charger_stop_charging();
                usb_charging_stopped();                 // Notify Application task (opt.)
        }
        pending_charge_finish_cnt = 0;
        vbat_sampling_valid = false;
        hw_charger_disable_detection();
        hw_charger_disable_charger_irq();

        charging_state = USB_CHARGING_OFF;
        print_charging_state_transitions();
}

#if (DEBUG_USB_CHARGER == 1)
/**
 * \brief Prints charging time over UART..
 *
 */
static void print_ntc(int mode)
{
        str_buf[0] = 'N';
        str_buf[1] = 'T';
        str_buf[2] = 'C';
        str_buf[3] = '-';

        if (mode == 0) {
                str_buf[4] = 'L';
        } else {
                str_buf[4] = 'H';
        }
        str_buf[5] = '\r';
        str_buf[6] = '\n';
        str_buf[7] = '\0';

        OS_ENTER_CRITICAL_SECTION();

        hw_uart_send(HW_UART2, str_buf, strlen(str_buf)+1, NULL, NULL);

        OS_LEAVE_CRITICAL_SECTION();
}
#endif

/**
 * \brief Starts charging using the already used settings.
 *
 */
static void start_charging_primitive(void)
{
        hw_charger_configure();

        if (charging_state == USB_PRE_CHARGING_ON) {
                hw_charger_set_charge_current(dg_configBATTERY_PRECHARGE_CURRENT);
                debug_print("* Set charging current (1) to: %d (%s mA)\r\n",
                        dg_configBATTERY_PRECHARGE_CURRENT,
                        usb_charging_current[dg_configBATTERY_PRECHARGE_CURRENT]);
                usb_precharging();                      // Notify Application task (opt.)
        } else {
                usb_charging();                         // Notify Application task (opt.)
        }

        hw_charger_start_charging();
}

/**
 * \brief Starts charging using the default settings.
 *
 */
static void start_charging_default(void)
{
        if ((charging_state != USB_CHARGING_OFF) && (charging_state != USB_CHARGING_BLOCKED)) {
                configured_current_limit = dg_configBATTERY_CHARGE_CURRENT;
                debug_print("* Set configured current (1) to: %d (%s mA)\r\n",
                        configured_current_limit, usb_charging_current[configured_current_limit]);

                start_charging_primitive();
        }
}

void usb_set_precharge_current(uint16_t current)
{
        /* Requested precharge current must be within limits */
        ASSERT_WARNING((current > 15) && (current < 30));

        OS_ENTER_CRITICAL_SECTION();

        DBG_CONFIGURE_HIGH(USB_CHARGER_TIMING_DEBUG, CHRGDBG_CRITICAL_SECTION);

        if (charging_state == USB_PRE_CHARGING_ON) {
                hw_charger_set_charge_current(current);
                debug_print("* Set charging current (2) to: %d (%s mA)\r\n", current,
                        usb_charging_current[current]);
        }

        DBG_CONFIGURE_LOW(USB_CHARGER_TIMING_DEBUG, CHRGDBG_CRITICAL_SECTION);

        OS_LEAVE_CRITICAL_SECTION();
}


void usb_reset_precharge_current(void)
{
        OS_ENTER_CRITICAL_SECTION();

        DBG_CONFIGURE_HIGH(USB_CHARGER_TIMING_DEBUG, CHRGDBG_CRITICAL_SECTION);

        if (charging_state == USB_PRE_CHARGING_ON) {
                hw_charger_set_charge_current(dg_configBATTERY_PRECHARGE_CURRENT);
                debug_print("* Set charging current (3) to: %d (%s mA)\r\n",
                        dg_configBATTERY_PRECHARGE_CURRENT,
                        usb_charging_current[dg_configBATTERY_PRECHARGE_CURRENT]);
        }

        DBG_CONFIGURE_LOW(USB_CHARGER_TIMING_DEBUG, CHRGDBG_CRITICAL_SECTION);

        OS_LEAVE_CRITICAL_SECTION();
}

/**
 * \brief Aborts charging process.
 *
 */
static void abort_charging(usb_charging_abort_status_t status)
{
        debug_print("* Charging aborted (status = %d)\r\n", status);

        stop_charging();

        if (status != ENTER_USB_SUSPEND) {
                charging_state = USB_CHARGING_BLOCKED;
                print_charging_state_transitions();

                usb_charger_state = USB_CHARGER_ATTACHED;
                print_state_transitions();

                usb_charging_aborted();                         // Notify Application task (opt.)
        } else {
                usb_charger_state = USB_CHARGER_PAUSED;
                print_state_transitions();

                usb_charging_paused();                          // Notify Application task (opt.)
        }
}

/**
 * \brief Monitors charging process.
 *
 */
static void process_end_of_charging(void)
{
        if (dg_configUSE_USB_CHARGER == 1) {
                usb_charging_abort_status_t abort_status;
                bool do_abort = false;

                do {

                        if ((charging_state == USB_CHARGING_OFF) || (charging_state == USB_CHARGING_BLOCKED)) {
                                usb_charger_state = USB_CHARGER_ATTACHED;
                                print_state_transitions();
                        } else if (hw_charger_temp_low()) {
#if (DEBUG_USB_CHARGER == 1)
                                print_ntc(0);
#endif

                                temp_error_cnt++;
                                if (temp_error_cnt == 3) {
                                        usb_charger_temp_low();
                                        abort_status = TEMP_LIMIT;
                                        do_abort = true;
                                        break;
                                }
                        } else if (hw_charger_temp_high()) {
#if (DEBUG_USB_CHARGER == 1)
                                print_ntc(1);
#endif

                                temp_error_cnt++;
                                if (temp_error_cnt == 3) {
                                        usb_charger_temp_high();
                                        abort_status = TEMP_LIMIT;
                                        do_abort = true;
                                        break;
                                }
                        } else {
                                temp_error_cnt = 0;

                                if (hw_charger_end_of_charge()) {
                                        if (FINISH_CHARGER_CHECK_CNT == pending_charge_finish_cnt) {
                                                stop_charging();
                                                usb_charger_state = USB_CHARGER_ATTACHED;
                                                print_state_transitions();
                                                if (dg_configUSE_SOC == 1) {
                                                        socf_full_charged_notification();
                                                }
                                                usb_charged();  // Notify Application task (opt.)
                                        } else {
                                                pending_charge_finish_cnt++;
                                                break;
                                        }
                                } else if (charging_state == USB_CHARGING_ON) {
                                        uint32_t vbat_level;
                                        bool vbat_is_valid;

                                        vbat_is_valid = sample_vbat(&vbat_level, false);
                                        if (!vbat_is_valid) {
                                                vbat_level = 0;
                                        }

                                        if (vbat_level > (charge_level + dg_configBATTERY_CHARGE_GAP)) {
                                                voltage_overflow_cnt++;
                                                if (voltage_overflow_cnt == 5) {
                                                        abort_status = OVER_VOLTAGE;
                                                        do_abort = true;        // Over-voltage protection
                                                        break;
                                                }
                                        } else {
                                                voltage_overflow_cnt = 0;
                                        }

                                        if (dg_configCHARGING_TIMEOUT != 0) {
                                                if (timer_cnt - precharging_timer_cnt > dg_configCHARGING_TIMEOUT) {
                                                        abort_status = CHARGING_TIMEOUT;
                                                        do_abort = true;
                                                        break;
                                                }
                                        } else {
                                                if (hw_charger_in_cc_mode()) {
                                                        time_in_cc_mode = timer_cnt - precharging_timer_cnt;
                                                        if (time_in_cc_mode > dg_configCHARGING_CC_TIMEOUT) {
                                                                abort_status = CHARGING_TIMEOUT;
                                                                do_abort = true;
                                                                break;
                                                        }
                                                } else if (hw_charger_in_cv_mode()) {
                                                        time_in_cv_mode = timer_cnt - time_in_cc_mode - precharging_timer_cnt;
                                                        if (time_in_cv_mode > dg_configCHARGING_CV_TIMEOUT) {
                                                                abort_status = CHARGING_TIMEOUT;
                                                                do_abort = true;
                                                                break;
                                                        }
                                                }
                                        }
                                }
                        }

                        pending_charge_finish_cnt = 0;
                } while (0);

                if (do_abort) {
                        pending_charge_finish_cnt = 0;
                        abort_charging(abort_status);
                }
        }
}

/**
 * \brief Handles entry to / exit from "paused system" mode.
 *
 */
__RETAINED_CODE static void pause_system(void);

static void pause_system(void)
{
        /*
         * Put the Flash in Power-Down.
         */
        if ((dg_configFLASH_CONNECTED_TO != FLASH_IS_NOT_CONNECTED)
                        && (dg_configFLASH_POWER_DOWN == 1)) {
                qspi_automode_flash_power_down();
        }

        /*
         * Call __WFI() from here to pause operation and lower power
         * consumption
         */
        REG_SETF(CRG_TOP, CLK_AMBA_REG, HCLK_DIV, (uint32_t)ahb_div16);
        __WFI();
        REG_SETF(CRG_TOP, CLK_AMBA_REG, HCLK_DIV, (uint32_t)ahb_div1);

        /*
         * Take the the Flash out of Power-Down in case the system did not go to sleep due
         * to a pending interrupt (so the uCode wasn't executed).
         */
        if ((dg_configFLASH_CONNECTED_TO != FLASH_IS_NOT_CONNECTED)
                        && (dg_configFLASH_POWER_DOWN == 1)) {
                qspi_automode_flash_power_up();
        }
}

/**
 * \brief Implements the main part of the USB Charger's FSM.
 *
 * \param [in] pxTimer ignored.
 *
 */
static void usb_charger_timer_cb( OS_TIMER pxTimer )
{
        OS_TASK_NOTIFY(xUSBChrgTaskFSMHandle, 1, OS_NOTIFY_SET_BITS);
}

/**
 * \brief USB Charger's Task function.
 *
 * \param [in] pvParameters ignored.
 *
 */
static void usb_charger_fsm_task( void *pvParameters )
{
        uint32_t ulNotifiedValue;
        OS_BASE_TYPE xResult __attribute__((unused));

        do {
                OS_BASE_TYPE result;
                uint32_t vbat_level = 0;

                DBG_CONFIGURE_LOW(USB_CHARGER_TIMING_DEBUG, CHRGDBG_FSM_TASK);

                // Wait for the internal notifications.
                xResult = OS_TASK_NOTIFY_WAIT(0x0, OS_TASK_NOTIFY_ALL_BITS, &ulNotifiedValue,
                                                                        OS_TASK_NOTIFY_FOREVER);

                DBG_CONFIGURE_HIGH(USB_CHARGER_TIMING_DEBUG, CHRGDBG_FSM_TASK);

                update_state_transitions();

                if ((DEBUG_USB_CHARGER_FSM == 1) && (DEBUG_USB_CHARGER_PRINT == 1)) {
                        OS_TASK_NOTIFY(xUSBChrgTaskPrintHandle, 1, OS_NOTIFY_SET_BITS);
                }


                timer_cnt++;
                (void) result;  // To satisfy the compiler...

                if (dg_configUSE_USB_CHARGER == 1) {
                        measure_vbat_normal();

                        /* Sample Vbat level. Note that when the vbat_level is checked, we know that the
                         * measurement is valid...
                         */
                        sample_vbat(&vbat_level, false);
                }

                if (detach_in_progress) {
                        detach_in_progress = false;
                        if (USB_CHARGER_IDLE != usb_charger_state) {
                                usb_charger_state = USB_CHARGER_DETACHED;
                                print_state_transitions();
                                update_state_transitions();
                        } else {
                                result = OS_TIMER_STOP(xUSBChrgTimer, OS_TIMER_FOREVER);
                                continue;
                        }
                }

                if ((USB_CHARGER_DETACHED != usb_charger_state)
                                && (USB_CHARGER_STABILIZING != usb_charger_state)) {
                        if (!hw_charger_check_vbus()) {
                                no_vbus_timer_cnt++;
                                if (NO_VBUS_TIMER_CNT_TO_DETACH == no_vbus_timer_cnt) {
                                        detach_in_progress = true;
                                }
                                continue;
                        } else {
                                no_vbus_timer_cnt = 0;
                        }
                }

                if ((dg_configUSB_SUSPEND_MODE != 0)
                        && (suspend_S3_requested != 0)
                        && (((USB_CHARGER_CHARGING_CONNECTED == usb_charger_state)
                                                && (timer_cnt == (suspend_S3_requested + 10)))
                                || (USB_CHARGER_ATTACHED == usb_charger_state))) {
                        /* USB will be SUSPENDED since VBUS is still on and SD3 was requested */

                        OS_ENTER_CRITICAL_SECTION();

                        if ((DEBUG_USB_CHARGER_FSM == 1) && (DEBUG_USB_CHARGER_PRINT == 1)) {
                                OS_UBASE_TYPE priority;

                                abort_charging(ENTER_USB_SUSPEND);

                                OS_LEAVE_CRITICAL_SECTION();

                                debug_print("** USB IS SUSPENDED (%d)!\r\n", 0);

                                OS_TASK_NOTIFY(xUSBChrgTaskPrintHandle, 1, OS_NOTIFY_SET_BITS);
                                priority = OS_TASK_PRIORITY_GET(NULL);
                                OS_TASK_PRIORITY_SET(NULL, OS_TASK_PRIORITY_NORMAL);
                                OS_TASK_YIELD();
                                OS_TASK_PRIORITY_SET(NULL, priority);

                                OS_ENTER_CRITICAL_SECTION();
                        } else {
                                abort_charging(ENTER_USB_SUSPEND);
                        }

                        if (dg_configUSB_SUSPEND_MODE == 1) {
                                /* Stop Low Power clock (pause system timing) */
                                REG_CLR_BIT(CRG_TOP, CLK_32K_REG, RC32K_ENABLE);
                                REG_SETF(CRG_TOP, CLK_CTRL_REG, CLK32K_SOURCE, LP_CLK_IS_RC32K);
                        }

                        OS_LEAVE_CRITICAL_SECTION();

                        /*
                         * Set system clock to XTAL16M and turn off PLL
                         */
                        sysclk_at_suspend = cm_sys_clk_get();
                        cm_sys_clk_set(sysclk_XTAL16M);
                        hw_cpm_pll_sys_off();

                        if (dg_configUSB_SUSPEND_MODE == 1) {
                                OS_ENTER_CRITICAL_SECTION();

                                /*
                                 * Disable all IRQs except for VBUS and USB
                                 */
                                iser_status_at_suspend = NVIC->ISER[0];

                                NVIC->ICER[0] = iser_status_at_suspend
                                        & ~((uint32_t)(1 << VBUS_IRQn) | (uint32_t)(1 << USB_IRQn));

                                /*
                                 * Clear all interrupts except for VBUS and USB so that the
                                 * system is not resumed by an already pending interrupt
                                 */
                                NVIC->ICPR[0] = iser_status_at_suspend
                                        & ~((uint32_t)(1 << VBUS_IRQn) | (uint32_t)(1 << USB_IRQn));

                                pause_system();

#if (dg_configUSB_SUSPEND_MODE == 1)
                                hw_usb_restore_int_mask_at_resume();
#endif

                                OS_LEAVE_CRITICAL_SECTION();
                        } else {
                                OS_TIMER_STOP(xUSBChrgTimer, OS_TIMER_FOREVER);
                        }
                }

                switch (usb_charger_state) {
                case USB_CHARGER_IDLE:
                        ASSERT_WARNING(0);      // When in IDLE state, the timer should have been stopped.

                        // In any case, make sure that the timer is stopped eventually.
                        result = OS_TIMER_STOP(xUSBChrgTimer, OS_TIMER_FOREVER);

                        break;

                case USB_CHARGER_STABILIZING:
//                        OS_ENTER_CRITICAL_SECTION();

//                        DBG_CONFIGURE_HIGH(USB_CHARGER_TIMING_DEBUG, CHRGDBG_CRITICAL_SECTION);

                        if (hw_charger_check_vbus()) {
                                OS_TASK_NOTIFY(xUSBChrgTaskHandle, USB_CHARGER_STABLE, OS_NOTIFY_SET_BITS);
                        } else if (timer_cnt == USB_CHRG_CONTACT_LIMIT) {
                                result = OS_TIMER_STOP(xUSBChrgTimer, OS_TIMER_FOREVER);
                                /* Debug test to make sure the Timer queue is sufficient */
                                ASSERT_WARNING(result == OS_OK);

                                OS_TASK_NOTIFY(xUSBChrgTaskHandle, USB_CHARGER_RESET, OS_NOTIFY_SET_BITS);
                        }

//                        DBG_CONFIGURE_LOW(USB_CHARGER_TIMING_DEBUG, CHRGDBG_CRITICAL_SECTION);

//                        OS_LEAVE_CRITICAL_SECTION();

                        break;

                case USB_CHARGER_CONTACT_DET:
                        hw_charger_enable_charger_irq();

                        if ( (usb_contact && (timer_cnt >= hit_time + USB_CHRG_DEBOUNCE_PERIOD))
                                        || (timer_cnt == USB_CHRG_CONTACT_LIMIT) ) {
                                OS_ENTER_CRITICAL_SECTION();

                                DBG_CONFIGURE_HIGH(USB_CHARGER_TIMING_DEBUG, CHRGDBG_CRITICAL_SECTION);

                                timer_cnt = 0;
                                hw_charger_disable_charger_irq();
                                hw_charger_start_primary_detection();
                                usb_charger_state = USB_CHARGER_PRIMARY_DET;
                                print_state_transitions();

                                DBG_CONFIGURE_LOW(USB_CHARGER_TIMING_DEBUG, CHRGDBG_CRITICAL_SECTION);

                                OS_LEAVE_CRITICAL_SECTION();

                                if (dg_configUSE_USB_CHARGER == 1) {
                                        if (vbat_level < dg_configPRECHARGING_THRESHOLD) {
                                                charging_state = USB_PRE_CHARGING_ON;
                                        } else if (vbat_level < charge_level) {
                                                charging_state = USB_CHARGING_ON;
                                        } else if (vbat_level <= (charge_level + dg_configBATTERY_CHARGE_GAP)) {
                                                charging_state = USB_CHARGING_OFF;
                                                usb_charger_battery_full(); // Notify Application task (opt.)
                                        } else {
                                                charging_state = USB_CHARGING_BLOCKED;
                                                usb_charger_bad_battery(); // Notify Application task (opt.)
                                        }
                                } else {
                                        charging_state = USB_CHARGING_BLOCKED;
                                }
                                print_charging_state_transitions();
                        }
                        break;

                case USB_CHARGER_PRIMARY_DET:
                        if (timer_cnt == 5) {
                                timer_cnt = 0;

                                if (hw_charger_check_primary()) {
                                        /*if (dg_configUSE_USB_ENUMERATION == 1) {
                                                hw_charger_disable_detection();
                                                usb_charger_state = USB_CHARGER_CONNECT_PENDING;
                                        } else */{
                                                hw_charger_start_secondary_detection();
                                                usb_charger_state = USB_CHARGER_SECONDARY_DET;
                                        }
                                } else {
                                        hw_charger_stop_any_detection();
                                        usb_charger_state = USB_CHARGER_APPLE_DET;
                                }
                                print_state_transitions();
                        }
                        break;

                case USB_CHARGER_SECONDARY_DET:
                        if (timer_cnt == 5) {
                                if (hw_charger_check_secondary()) {             // DCP (500mA guaranteed)
                                        hw_charger_set_dp_high();
                                } else {                                        // CDP (1.5A guaranteed)
                                }
                                hw_charger_disable_detection();
                                start_charging_default();
                                if (dg_configUSE_USB_ENUMERATION == 1) {
                                        usb_start_enumeration_cb();
                                        usb_charger_state = USB_CHARGER_CONNECT_PENDING;
                                        charge_initiation_state = USB_CHARGER_SECONDARY_DET;
                                } else {
                                        usb_charger_state = USB_CHARGER_CHARGING;
                                        charge_initiation_state = USB_CHARGER_CHARGING;
                                }
                                print_state_transitions();
                        }
                        break;

                case USB_CHARGER_APPLE_DET:
                        if (timer_cnt == 5) {
                                uint16_t status = hw_charger_get_status();

                                if (CHECK_APPLE_CHARGER_TYPE_1(status)          // D+ = 2.0V, D- = 2.0V (up to 500mA)
                                 || CHECK_APPLE_CHARGER_TYPE_2(status)          // D+ = 2.0V, D- > 2.5V (up to 1A)
                                 || CHECK_APPLE_CHARGER_TYPE_3(status)) {       // D+ > 2.5V, D- = 2.0V (up to 2A)
                                        hw_charger_disable_detection();
                                        start_charging_default();
                                        usb_charger_state = USB_CHARGER_CHARGING;
                                        charge_initiation_state = USB_CHARGER_CHARGING;
                                } else if (CHECK_APPLE_CHARGER_TYPE_4(status)) {  // > 2.5V (undefined)
                                        ASSERT_WARNING(0); // Don't know if it's allowed to charge...
                                        timer_cnt = 0;
                                        usb_charger_state = USB_CHARGER_ATTACHED;
                                } else {
                                        timer_cnt = 0;
                                        hw_charger_set_dp_high();
                                        usb_charger_state = USB_CHARGER_ACA_DET;
                                }
                                print_state_transitions();
                        }
                        break;

                case USB_CHARGER_ACA_DET:
                        if (timer_cnt == 5) {
                                uint16_t status = hw_charger_get_status();

                                hw_charger_disable_detection();

                                if ( (status & USB_USB_CHARGER_STAT_REG_USB_DP_VAL2_Msk)
                                                && (status & USB_USB_CHARGER_STAT_REG_USB_DM_VAL_Msk)
                                                && hw_charger_check_primary()) {
                                        start_charging_default();
                                        usb_charger_state = USB_CHARGER_CHARGING;
                                        charge_initiation_state = USB_CHARGER_CHARGING;
                                } else {
                                        timer_cnt = 0;
                                        usb_charger_state = USB_CHARGER_SDP;
                                }
                                print_state_transitions();
                        }
                        break;

                case USB_CHARGER_SDP:
                        if (timer_cnt == 1) {
                                charge_initiation_state = USB_CHARGER_SDP;

                                if ((dg_configUSE_USB_CHARGER == 1)
                                                && (dg_configALLOW_CHARGING_NOT_ENUM == 1)
                                                && (charging_state != USB_CHARGING_OFF)
                                                && (charging_state != USB_CHARGING_BLOCKED)) {
                                        hw_charger_configure();

                                        configured_current_limit = dg_configBATTERY_CHARGE_CURRENT;
                                        debug_print("* Set configured current (2) to: %d (%s mA)\r\n",
                                                configured_current_limit,
                                                usb_charging_current[configured_current_limit]);

                                        if ((dg_configBATTERY_CHARGE_CURRENT >= 6) && (dg_configBATTERY_CHARGE_CURRENT < 16)) {// >= 100mA
                                                        hw_charger_set_charge_current(5);       // 90 mA
                                                        debug_print("* Set charging current (4) to: %d (%s mA)\r\n", 5,
                                                                usb_charging_current[5]);
                                                        configured_current_limit = 5;
                                                        debug_print("* Set configured current (3) to: %d (%s mA)\r\n",
                                                                configured_current_limit,
                                                                usb_charging_current[configured_current_limit]);
                                        }

                                        if (charging_state == USB_PRE_CHARGING_ON) {
                                                hw_charger_set_charge_current(dg_configBATTERY_PRECHARGE_CURRENT);
                                                debug_print("* Set charging current (5) to: %d (%s mA)\r\n",
                                                        dg_configBATTERY_PRECHARGE_CURRENT,
                                                        usb_charging_current[dg_configBATTERY_PRECHARGE_CURRENT]);
                                                usb_precharging();              // Notify Application task (opt.)
                                        } else {
                                                usb_charging();                 // Notify Application task (opt.)
                                        }

                                        hw_charger_start_charging();

                                        if (dg_configUSE_USB_ENUMERATION == 0) {
                                                // Show as Connected to be able to draw up to 100mA
                                                hw_charger_enable_usb_node();
                                                hw_charger_enable_usb_pullup();
                                        }
                                }

                                if (dg_configUSE_USB_ENUMERATION == 1) {
                                        usb_start_enumeration_cb();
                                        usb_charger_state = USB_CHARGER_CONNECT_PENDING;
                                        charge_initiation_state = USB_CHARGER_SDP;
                                }
                                print_state_transitions();
                        } else if ((dg_configUSE_USB_CHARGER == 1)
                                        && (dg_configUSE_NOT_ENUM_CHARGING_TIMEOUT == 1)
                                        && (timer_cnt >= 269800)) { // ~45min
                                stop_charging();
                                usb_charger_state = USB_CHARGER_ATTACHED;
                                print_state_transitions();
                        } else {
                                process_end_of_charging();
                        }
                        break;

                case USB_CHARGER_CHARGING:
                        process_end_of_charging();
                        break;

                case USB_CHARGER_CONNECT_PENDING:
                        if (usb_is_connected) {
                                if (dg_configUSE_USB_CHARGER == 1) {
                                        if ((charging_state != USB_CHARGING_OFF) && (charging_state != USB_CHARGING_BLOCKED)) {
                                                hw_charger_stop_charging();
                                                hw_charger_configure();
                                                if (charging_state == USB_PRE_CHARGING_ON) {
                                                        hw_charger_set_charge_current(dg_configBATTERY_PRECHARGE_CURRENT);
                                                        debug_print("* Set charging current (6) to: %d (%s mA)\r\n",
                                                                dg_configBATTERY_PRECHARGE_CURRENT,
                                                                usb_charging_current[dg_configBATTERY_PRECHARGE_CURRENT]);
                                                        usb_precharging();                      // Notify Application task (opt.)
                                                } else {
                                                        hw_charger_set_charge_current(configured_current_limit);
                                                        debug_print("* Set charging current (7) to: %d (%s mA)\r\n",
                                                                configured_current_limit,
                                                                usb_charging_current[configured_current_limit]);
                                                        usb_charging();                         // Notify Application task (opt.)
                                                }

                                                hw_charger_start_charging();
                                        }
                                        charge_initiation_state = USB_CHARGER_CHARGING_CONNECTED;
                                }
                                usb_charger_state = USB_CHARGER_CHARGING_CONNECTED;
                                print_state_transitions();
                        } else if ((dg_configUSE_USB_CHARGER == 1)
                                        && (dg_configUSE_NOT_ENUM_CHARGING_TIMEOUT == 1)
                                        && (timer_cnt >= 269800)) { // ~45min
                                stop_charging();
                                usb_charger_state = USB_CHARGER_ATTACHED;
                                print_state_transitions();
                        } else {
                                process_end_of_charging();
                        }
                        break;

                case USB_CHARGER_CHARGING_CONNECTED:
                        if (!usb_is_connected) {
                                if (dg_configUSE_USB_CHARGER == 1) {
                                        if (charging_state == USB_CHARGING_ON) {
                                                if ((dg_configBATTERY_CHARGE_CURRENT >= 6)
                                                                && (dg_configBATTERY_CHARGE_CURRENT < 16)
                                                                && (charge_initiation_state == USB_CHARGER_SDP)) {    // >= 100mA
                                                        hw_charger_set_charge_current(5);       // 90 mA
                                                        debug_print("* Set charging current (8) to: %d (%s mA)\r\n", 5,
                                                                usb_charging_current[5]);
                                                        configured_current_limit = 5;
                                                        debug_print("* Set configured current (4) to: %d (%s mA)\r\n",
                                                                configured_current_limit,
                                                                usb_charging_current[configured_current_limit]);
                                                } else {
                                                        configured_current_limit = dg_configBATTERY_CHARGE_CURRENT;
                                                        debug_print("* Set configured current (5) to: %d (%s mA)\r\n",
                                                                configured_current_limit,
                                                                usb_charging_current[configured_current_limit]);
                                                }
                                        }
                                }
                                timer_cnt = 0;
                                usb_charger_state = USB_CHARGER_CONNECT_PENDING;
                                print_state_transitions();
                        } else {
                                process_end_of_charging();
                        }
                        break;

                case USB_CHARGER_ATTACHED:
                        if ((dg_configUSE_USB_CHARGER == 1)
                                        && (timer_cnt >= dg_configUSB_CHARGER_POLLING_INTERVAL)) {
                                uint32_t replenish_level;

                                timer_cnt = 0;

                                if (charge_level != 0) {
                                        replenish_level = charge_level - dg_configBATTERY_REPLENISH_GAP;
                                } else {
                                        replenish_level = 0;
                                }

                                if (dg_configBATTERY_TYPE < BATTERY_TYPE_NO_RECHARGE) {
                                        if ((charging_state == USB_CHARGING_OFF)
                                                        && (vbat_level < replenish_level)) {
                                                if (vbat_level < dg_configPRECHARGING_THRESHOLD) {
                                                        charging_state = USB_PRE_CHARGING_ON;
                                                        hw_charger_set_charge_current(dg_configBATTERY_PRECHARGE_CURRENT);
                                                        debug_print("* Set charging current (10) to: %d (%s mA)\r\n",
                                                                dg_configBATTERY_PRECHARGE_CURRENT,
                                                                usb_charging_current[dg_configBATTERY_PRECHARGE_CURRENT]);
                                                } else {
                                                        charging_state = USB_CHARGING_ON;
                                                        hw_charger_set_charge_current(configured_current_limit);
                                                        debug_print("* Set charging current (11) to: %d (%s mA)\r\n",
                                                                configured_current_limit,
                                                                usb_charging_current[configured_current_limit]);
                                                }
                                                print_charging_state_transitions();

                                                if (charge_initiation_state == USB_CHARGER_CHARGING_CONNECTED) {
                                                        if (usb_is_connected) {
                                                                start_charging_primitive();
                                                                usb_charger_state = USB_CHARGER_CHARGING_CONNECTED;
                                                        } else {
                                                                usb_charger_state = USB_CHARGER_SDP;
                                                        }
                                                } else if (charge_initiation_state == USB_CHARGER_CHARGING) {
                                                        start_charging_primitive();
                                                        usb_charger_state = USB_CHARGER_CHARGING;
                                                } else if (charge_initiation_state == USB_CHARGER_SDP) {
                                                        if (!usb_is_connected) {
                                                                usb_charger_state = USB_CHARGER_SDP;
                                                        } else {
                                                                hw_charger_start_charging();
                                                                usb_charger_state = USB_CHARGER_CONNECT_PENDING;
                                                        }
                                                } else if (charge_initiation_state == USB_CHARGER_SECONDARY_DET) {
                                                        if (!usb_is_connected) {
                                                                usb_charger_state = USB_CHARGER_CONNECT_PENDING;
                                                        } else {
                                                                usb_charger_state = USB_CHARGER_CHARGING_CONNECTED;
                                                        }
                                                        hw_charger_start_charging();
                                                } else {
                                                        /* Illegal state */
                                                        ASSERT_WARNING(0);
                                                }
                                                print_state_transitions();
                                        }
                                }
                        }
                        break;

                case USB_CHARGER_DETACHED:
                        result = OS_TIMER_STOP(xUSBChrgTimer, OS_TIMER_FOREVER);
                        /* Debug test to make sure Timer queue is sufficient */
                        ASSERT_WARNING(result == OS_OK);

                        hw_charger_disable_usb_pads();

                        usb_contact = false;

                        usb_detach_cb();                        // Notify Application task (opt.)

                        stop_charging();

                        if (dg_configUSE_DCDC == 1) {
                                hw_cpm_dcdc_on();               // Resume DCDC for maximum efficiency.
                        }

                        usb_charger_state = USB_CHARGER_IDLE;
                        print_state_transitions();

                        OS_TASK_NOTIFY(xUSBChrgTaskHandle, USB_CHARGER_ABORT, OS_NOTIFY_SET_BITS);
                        break;

                case USB_CHARGER_PAUSED:
                        timer_cnt = 0;
                        break;

                default:
                        ASSERT_WARNING(0);
                        break;
                }

                update_state_transitions();

                if ((dg_configUSE_USB_CHARGER == 1) && (charging_state == USB_PRE_CHARGING_ON)) {
                        if (hw_charger_is_charging()) {
                                precharging_timer_cnt++;
                        }

                        if (vbat_level > dg_configCHARGING_THRESHOLD) {
                                charging_state = USB_CHARGING_ON;
                                print_charging_state_transitions();
                                if (hw_charger_is_charging()) {
                                        hw_charger_stop_charging();
                                        hw_charger_set_charge_current(configured_current_limit);
                                        debug_print("* Set charging current (9) to: %d (%s mA)\r\n",
                                                configured_current_limit,
                                                usb_charging_current[configured_current_limit]);
                                        hw_charger_start_charging();
                                        usb_charging();                 // Notify Application task (opt.)
                                }
                        }
                        else if (timer_cnt > dg_configPRECHARGING_TIMEOUT) {
                                if (hw_charger_is_charging()) {
                                        stop_charging();
                                        usb_charger_state = USB_CHARGER_ATTACHED;
                                        print_state_transitions();
                                }
                                charging_state = USB_CHARGING_BLOCKED;
                                print_charging_state_transitions();
                                usb_precharging_aborted();              // Notify Application task (opt.)

                                continue;
                        }
                }
        } while (1);
}

/**
 * \brief Ensures the ATTACH interrupt is not missed.
 *
 * \note This function is to be called during ABORT or RESET handling.
 *
 */
static inline void secure_check_attach_proc(void)
{
        NVIC_DisableIRQ(VBUS_IRQn);

        hw_charger_set_vbus_irq_high();

        if (hw_charger_check_vbus()) {
                usb_attach();
                NVIC_ClearPendingIRQ(VBUS_IRQn);
        }

        NVIC_EnableIRQ(VBUS_IRQn);
}

/**
 * \brief Handles the Node Resume from the USB Host.
 *
 */
static void handle_node_resume(void)
{
        if (dg_configUSB_SUSPEND_MODE == 0) {
                return;
        }

        OS_ENTER_CRITICAL_SECTION();

        if (dg_configUSB_SUSPEND_MODE == 1) {
                /* Resume Low Power clock (resume system timing) */
                if (dg_configLP_CLK_SOURCE == LP_CLK_IS_DIGITAL) {
                        hw_cpm_lp_set_ext32k();
                } else if ((dg_configUSE_LP_CLK == LP_CLK_32000)
                                || (dg_configUSE_LP_CLK == LP_CLK_32768)) {
                        hw_cpm_lp_set_xtal32k();
                } else {
                        hw_cpm_lp_set_rcx();
                }
                hw_cpm_enable_rc32k();
        }

        /*
         * Restore system clock
         */
        cm_sys_restore_sysclk(sysclk_at_suspend);

        if (dg_configUSB_SUSPEND_MODE == 1) {
                /*
                 * Restore all IRQs
                 */
                NVIC->ISER[0] = iser_status_at_suspend;
        }

        debug_print("** USB IS RESUMED (%d)!\r\n", 0);

        usb_charger_state = USB_CHARGER_ATTACHED;
        print_state_transitions();

        usb_is_resumed();

        OS_LEAVE_CRITICAL_SECTION();
}

/**
 * \brief USB Charger's Task function.
 *
 * \param [in] pvParameters ignored.
 *
 */
static void usb_charger_task( void *pvParameters )
{
        uint32_t ulNotifiedValue;
        OS_BASE_TYPE xResult __attribute__((unused));

        if (dg_configUSE_SOC == 1) {
                socf_init_soc();
        }

        if (hw_charger_check_vbus()) {
                usb_attach();           // VBUS is available - Execute Attach procedure
        } else {
                hw_charger_set_vbus_irq_high();
        }
        hw_charger_enable_vbus_irq();

        do {
                DBG_CONFIGURE_LOW(USB_CHARGER_TIMING_DEBUG, CPMDBG_CONTROL_TASK);

                // Wait for the internal notifications.
                xResult = OS_TASK_NOTIFY_WAIT(0x0, OS_TASK_NOTIFY_ALL_BITS, &ulNotifiedValue,
                                                                            OS_TASK_NOTIFY_FOREVER);

                DBG_CONFIGURE_HIGH(USB_CHARGER_TIMING_DEBUG, CPMDBG_CONTROL_TASK);

                if (ulNotifiedValue & USB_CHARGER_ABORT) {
                        pm_resume_sleep();      // Powered from VBAT so sleep is resumed.

                        secure_check_attach_proc();
                }

                if (ulNotifiedValue & USB_CHARGER_DETACH) {
                        suspend_S3_requested = 0;
                        if (usb_charger_state == USB_CHARGER_PAUSED) {
                                handle_node_resume();
                        }

                        usb_detach();           // Execute Detach procedure
                }

                if (ulNotifiedValue & USB_CHARGER_STABLE) {
                        OS_ENTER_CRITICAL_SECTION();

                        DBG_CONFIGURE_HIGH(USB_CHARGER_TIMING_DEBUG, CHRGDBG_CRITICAL_SECTION);

                        usb_attach_critical();  // Execute Attach procedure (critical part only)

                        DBG_CONFIGURE_LOW(USB_CHARGER_TIMING_DEBUG, CHRGDBG_CRITICAL_SECTION);

                        OS_LEAVE_CRITICAL_SECTION();
                }

                if (ulNotifiedValue & USB_CHARGER_RESET) {
                        timer_cnt = 0;

                        usb_detach_cb();

                        usb_charger_state = USB_CHARGER_IDLE;
                        print_state_transitions();

                        secure_check_attach_proc();
                }

                if (ulNotifiedValue & USB_CHARGER_ATTACH) {
                        usb_attach();           // Execute Attach procedure
                }

                if (dg_configUSB_SUSPEND_MODE != 0) {
                        bool restart_fsm = false;

                        OS_ENTER_CRITICAL_SECTION();

                        if ((ulNotifiedValue & USB_CHARGER_SUSPEND)
                                        && !(ulNotifiedValue & USB_CHARGER_RESUME)) {
                                suspend_S3_requested = timer_cnt;
                        }

                        if ((ulNotifiedValue & USB_CHARGER_RESUME)
                                        && !(ulNotifiedValue & USB_CHARGER_SUSPEND)) {
                                suspend_S3_requested = 0;

                                if (usb_charger_state == USB_CHARGER_PAUSED) {
                                        handle_node_resume();
                                }

                                if (dg_configUSB_SUSPEND_MODE == 2) {
                                        restart_fsm = true;
                                }
                        }

                        OS_LEAVE_CRITICAL_SECTION();

                        if (restart_fsm) {      // Restart charger FSM
                                OS_TIMER_RESET(xUSBChrgTimer, OS_TIMER_FOREVER);
                        }
                }
        } while (1);
}

void usb_charger_init(void)
{
        OS_BASE_TYPE status;
#if ((DEBUG_USB_CHARGER_FSM == 1) && (DEBUG_USB_CHARGER_PRINT == 1))
        const uint32_t stack_size = 2 * configMINIMAL_STACK_SIZE * OS_STACK_WORD_SIZE;
#else
        const uint32_t stack_size = configMINIMAL_STACK_SIZE * OS_STACK_WORD_SIZE;
#endif

        usb_charger_state = USB_CHARGER_IDLE;
        print_state_transitions();
        charging_state = USB_CHARGING_OFF;
        print_charging_state_transitions();
        usb_is_connected = false;
        configured_current_limit = 0;

        // Create the FSM Timer.
        xUSBChrgTimer = OS_TIMER_CREATE("TimerChrg",
                                OS_MS_2_TICKS(10),              // Expire after 10 msec
                                OS_TIMER_SUCCESS,               // Run repeatedly
                                (void *) 0,
                                usb_charger_timer_cb );
        OS_ASSERT(xUSBChrgTimer != NULL);

        // Create the USB Charger task
        status = OS_TASK_CREATE("USBC",                         // The text name of the task.
                                usb_charger_task,               // The function that implements the task.
                                ( void * ) NULL,                // No parameter is passed to the task.
                                stack_size,                     // The size of the stack to allocate.
                                (OS_TASK_PRIORITY_HIGHEST - 3), // The priority assigned to the task.
                                xUSBChrgTaskHandle);            // The task handle is required.
        OS_ASSERT(status == OS_OK);

        // Create the USB Charger FSM task
        status = OS_TASK_CREATE("USBF",                         // The text name of the task.
                                usb_charger_fsm_task,           // The function that implements the task.
                                ( void * ) NULL,                // No parameter is passed to the task.
                                stack_size,                     // The size of the stack to allocate.
                                (OS_TASK_PRIORITY_HIGHEST - 2), // The priority assigned to the task.
                                xUSBChrgTaskFSMHandle);         // The task handle is required.
        OS_ASSERT(status == OS_OK);

        if ((DEBUG_USB_CHARGER_FSM == 1) && (DEBUG_USB_CHARGER_PRINT == 1)) {
                // Create the USB Charger print task
                status = OS_TASK_CREATE("USBP",
                                usb_print_messages_task,
                                ( void * ) NULL,
                                stack_size,
                                (OS_TASK_PRIORITY_NORMAL),
                                xUSBChrgTaskPrintHandle);
                OS_ASSERT(status == OS_OK);
        }

        (void) status;                                          // To satisfy the compiler
}

void usb_charger_connected(uint32_t curr_lim)
{
        OS_ENTER_CRITICAL_SECTION();

        usb_is_connected = true;
        configured_current_limit = curr_lim;
        debug_print("* Set configured current (6) to: %d (%s mA)\r\n", configured_current_limit,
                usb_charging_current[configured_current_limit]);
        if (configured_current_limit > dg_configBATTERY_CHARGE_CURRENT) {
                configured_current_limit = dg_configBATTERY_CHARGE_CURRENT;
                debug_print("* Set configured current (7) to: %d (%s mA)\r\n",
                        dg_configBATTERY_CHARGE_CURRENT,
                        usb_charging_current[dg_configBATTERY_CHARGE_CURRENT]);
        }

#if (dg_configUSB_SUSPEND_MODE != 0)
                hw_usb_enable_suspend();
#endif

        OS_LEAVE_CRITICAL_SECTION();
}

void usb_charger_disconnected(void)
{
#if (dg_configUSB_SUSPEND_MODE != 0)
        hw_usb_disable_suspend();
#endif

        usb_is_connected = false;
        configured_current_limit = 0;
        debug_print("* Set configured current (8) to: %d (%s mA)\r\n", configured_current_limit,
                usb_charging_current[configured_current_limit]);
}

bool usb_charger_is_battery_low(void)
{
        uint32_t vbat_level;

        if (dg_configUSE_USB_CHARGER == 1) {
                if (!sample_vbat(&vbat_level, true)) {
                        vbat_level = measure_vbat_basic_via_lld();
                } else {
                        return false;
                }
        } else {
                if (dg_configBATTERY_TYPE == BATTERY_TYPE_NO_RECHARGE) {
                        /* Hibernation is disabled for non-rechargeable batteries */
                        ASSERT_WARNING(0);
                        return false;
                } else {
                        vbat_level = measure_vbat_basic_via_lld();
                }
        }

        if (vbat_level < dg_configBATTERY_LOW_LEVEL) {
                return true;
        } else {
                return false;
        }
}

#if (DEBUG_USB_CHARGER == 1)
/**
 * \brief Prints vbat measurement over UART.
 *
 */
static void print_vbat_measurement(uint32_t val)
{
        uint32_t digit;

        str_buf[0] = 'A';
        str_buf[1] = 'D';
        str_buf[2] = 'C';
        str_buf[3] = ':';
        str_buf[4] = ' ';

        digit = val / 1000;
        str_buf[5] = 0x30 + digit;
        val -= digit * 1000;

        digit = val / 100;
        str_buf[6] = 0x30 + digit;
        val -= digit * 100;

        digit = val / 10;
        str_buf[7] = 0x30 + digit;
        val -= digit * 10;

        str_buf[8] = 0x30 + val;

        str_buf[9] = '\r';
        str_buf[10] = '\n';
        str_buf[11] = '\0';

        hw_uart_send(HW_UART2, str_buf, strlen(str_buf)+1, NULL, NULL);
}
#endif

/**
 * \brief Takes a measurement of Vbat via ADC.
 *
 * \return The measured Vbat value.
 *
 * \warning This function performs oversampling by 4 while each measurement takes 15 * 32 system
 *        clock cycles. Thus, 30usec are required for each measurement and the total delay
 *        introduced by this function is at least 120usec.
 *
 * \note This function is called when the scheduler is suspended hence the gpadc adapter
 *        cannot be used.
 *
 */
static uint32_t measure_vbat_basic_via_lld(void)
{
        uint32_t ret;

        DBG_CONFIGURE_HIGH(USB_CHARGER_TIMING_DEBUG, CHRGDBG_CRITICAL_SECTION);

        hw_gpadc_reset();

        hw_gpadc_set_input(HW_GPADC_INPUT_SE_VBAT);
        hw_gpadc_set_input_mode(HW_GPADC_INPUT_MODE_SINGLE_ENDED);
        hw_gpadc_set_ldo_constant_current(true);
        hw_gpadc_set_ldo_dynamic_current(true);
        hw_gpadc_adc_measure();         // dummy (fast) measurement
        hw_gpadc_set_sample_time(15);
        hw_gpadc_set_chopping(true);
        hw_gpadc_set_oversampling(2);   // 4 samples
        for (volatile int i = 10; i > 0; i--);   // Make sure 1usec has passed since the mode setting (VBAT).

        hw_gpadc_adc_measure();

        ret = (uint32_t)(hw_gpadc_get_raw_value() >> 4);

        DBG_CONFIGURE_LOW(USB_CHARGER_TIMING_DEBUG, CHRGDBG_CRITICAL_SECTION);

#if (DEBUG_USB_CHARGER == 1)
        print_vbat_measurement(ret);
#endif

        return ret;
}

/**
 \}
 \}
 \}
 */
