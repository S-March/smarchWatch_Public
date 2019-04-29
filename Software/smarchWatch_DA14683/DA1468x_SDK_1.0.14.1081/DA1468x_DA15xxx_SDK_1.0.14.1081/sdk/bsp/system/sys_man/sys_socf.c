/**
 ****************************************************************************************
 *
 * @file sys_socf.c
 *
 * @brief SOC function
 *
 * Copyright (C) 2016-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_SOC

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "sdk_defs.h"
#include <stdbool.h>
#include "hw_usb_charger.h"
#include "sys_rtc.h"
#include "sys_charger.h"
#include "sys_power_mgr.h"
#include "hw_cpm.h"
#include "hw_gpadc.h"
#include "ad_battery.h"
#include "hw_uart.h"
#include "timers.h"
#include "hw_soc.h"
#include "ad_nvparam.h"
#include "platform_nvparam.h"
#include "hw_gpadc.h"
#include "custom_socf_battery_profile.h"

#define SOCF_SAMPLING_TIME 10000
#define SOCF_MAX_COULOMB_COUNT_PER_MA_SEC 73584LL
#define SOCF_DEFAULT_OFFSET_CURRENT 480
#define SOCF_OFFSET_VOLTAGE 5
#define SOCF_VOLTAGE_ADJUST_SOC 900
#define SOCF_MAX_SOC_VAL 1000
#define SOCF_MIN_SOC_VAL 0
#define SOCF_PT_REPORT_TIME 1000
#define SOCF_MAX_SOC_CHECK_TIME 1000
#define SOCF_SOC_INTERVAL 100
#define SOCF_MIN_CURRENT_CHECK_TIME 0
#define SOCF_MAX_ALLOWED_DISCHA_CURRENT (SOCF_BATT_HIGH_CURRENT * 4)

#define SOCF_TRANS_CONST1 ((1000000LL << 20) / SOCF_MAX_COULOMB_COUNT_PER_MA_SEC)
#define SOCF_COUNT_TO_CURRENT(count, period) \
        ((((count) * (int64_t)SOCF_TRANS_CONST1) / (period)) >> 20)

#define SOCF_TRANS_CONST2 ((SOCF_MAX_COULOMB_COUNT_PER_MA_SEC << 20) / 1000000)
#define SOCF_CURRENT_TO_COUNT(current, period) \
        (((int64_t)SOCF_TRANS_CONST2 * (current) * (period)) >> 20)

#define SOCF_TRANS_CONST3 ((1000LL << 30) / (SOCF_MAX_COULOMB_COUNT_PER_MA_SEC * 3600))
#define SOCF_COUNT_TO_SOC(count) \
        (uint16_t)((((count) * (int64_t)SOCF_TRANS_CONST3) / (socf_batt_capacitance)) >> 30)

#define SOCF_TRANS_CONST4 (((SOCF_MAX_COULOMB_COUNT_PER_MA_SEC << 20) * 3600) / 1000)
#define SOCF_SOC_TO_COUNT(soc) \
        (((soc) * socf_batt_capacitance * (int64_t)SOCF_TRANS_CONST4) >> 20)

#if (dg_configUSE_LP_CLK == LP_CLK_32000)
#define SOCF_REF_LP_FACTOR ((1000 << 20) / 32000)
#elif (dg_configUSE_LP_CLK == LP_CLK_32768)
#define SOCF_REF_LP_FACTOR ((1000 << 20) / 32768)
#elif (dg_configUSE_LP_CLK == LP_CLK_RCX)
#define SOCF_REF_LP_FACTOR ((1000 << 20) / rcx_clock_hz)
#elif (dg_configUSE_LP_CLK == LP_CLK_ANY)
// Must be defined in the custom_config_<>.h file.
#else
# error "SOCF_REF_LP_FACTOR is not defined!"
#endif

#define SOCF_GET_DURATION(now,pre) \
        ((((now) - (pre)) * (int64_t)SOCF_REF_LP_FACTOR) >> 20)

#define SOCF_AVG_FILTER(pre, new) \
        (((pre) * 15 + (new)) >> 4)

static const int16_t socf_ref[VOL2SOC_LUT_SIZE] = {
        0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000
};
__RETAINED static int16_t socf_soc;
__RETAINED static TimerHandle_t xSocfTimer;
__RETAINED static int16_t socf_batt_capacitance;
__RETAINED static int16_t socf_batt_voltage_gain_cal;
__RETAINED static uint64_t socf_start_count_time;
__RETAINED static uint64_t socf_pre_sample_time;
__RETAINED static int64_t socf_accum_charge_count;
#if (SOCF_BATT_CAPACITANCE_ADJ == 1)
__RETAINED static int16_t socf_voltage_comp;
#endif
__RETAINED static int16_t *vol_dis_low, *vol_dis_high, *vol_chg_0;

static int16_t socf_chk_lut(const int16_t* x, const int16_t* y, int16_t v,
        int16_t l)
{
        int16_t i;
        int16_t ret;

        if (v < x[0]) {
                ret = y[0];
        } else if (v >= x[l - 1]) {
                ret = y[l - 1];
        } else {
                for (i = 1; i < l; i++) {
                        if (v < x[i]) {
                                break;
                        }
                }
                ret = y[i - 1];
                ret = ret + ((v - x[i - 1]) * (y[i] - y[i - 1])) / (x[i] - x[i - 1]);
        }
        return ret;
}

#if (SOCF_SECONDARY_BATT == 1)
static uint16_t socf_read_batt_id(void)
{
        uint16_t adc_val = 0;

        hw_gpio_configure_pin(SOCF_BATT_ID_PORT, SOCF_BATT_ID_PIN, HW_GPIO_MODE_INPUT_PULLUP, HW_GPIO_FUNC_GPIO, true);
        hw_gpio_configure_pin_power(SOCF_BATT_ID_PORT, SOCF_BATT_ID_PIN, HW_GPIO_POWER_VDD1V8P);

        hw_gpadc_reset();

        hw_gpadc_set_input(SOCF_BATT_ID_INPUT);
        hw_gpadc_set_input_mode(HW_GPADC_INPUT_MODE_SINGLE_ENDED);
        hw_gpadc_adc_measure();
        hw_gpadc_set_sample_time(15);
        hw_gpadc_set_chopping(true);
        hw_gpadc_set_oversampling(4);
        for (volatile int i = 10; i > 0; i--)
        ;
        hw_gpadc_adc_measure();

        adc_val = (int16_t) (((uint32_t) (hw_gpadc_get_value()) * 1800) / 16383);
        hw_gpio_configure_pin(SOCF_BATT_ID_PORT, SOCF_BATT_ID_PIN, HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_ADC, false);
        return adc_val;
}
#endif

static int16_t socf_vbat_compensation(int16_t voltage)
{
#if (SOCF_BATT_CAPACITANCE_ADJ == 1)
        if (socf_soc > SOCF_VOLTAGE_ADJUST_SOC) {
                return (voltage + SOCF_OFFSET_VOLTAGE
                        + ((socf_voltage_comp * (socf_soc - SOCF_VOLTAGE_ADJUST_SOC))
                                / (SOCF_MAX_SOC_VAL - SOCF_VOLTAGE_ADJUST_SOC)));
        } else {
                return (voltage + SOCF_OFFSET_VOLTAGE);
        }
#else
        return (voltage + SOCF_OFFSET_VOLTAGE);
#endif
}

static int16_t socf_vbat_via_adapter(void)
{
        int16_t voltage;
        battery_source src = ad_battery_open();

        voltage = (int16_t)ad_battery_raw_to_mvolt(src, ad_battery_read(src));
        ad_battery_close(src);

        return socf_vbat_compensation(voltage);
}

static int16_t socf_measure_vbat(void)
{
        int16_t voltage;
        uint8_t sample_number = 4;

        hw_gpadc_reset();

        hw_gpadc_set_input(HW_GPADC_INPUT_SE_VBAT);
        hw_gpadc_set_input_mode(HW_GPADC_INPUT_MODE_SINGLE_ENDED);
        hw_gpadc_adc_measure();
        hw_gpadc_set_sample_time(15);
        hw_gpadc_set_chopping(true);
        hw_gpadc_set_oversampling(sample_number);
        hw_cpm_delay_usec(1);
        hw_gpadc_adc_measure();

        voltage = (int16_t)(((uint32_t)(hw_gpadc_get_value()) * 5000)
                / ((1024 << sample_number) - 1));

        return socf_vbat_compensation(voltage);
}

static void socf_start_count()
{
        hw_soc_init(&soc_cfg_recommended);
        hw_soc_enable();
        socf_start_count_time = rtc_get_fromISR();
}

static int64_t socf_compensation_count(int64_t count, uint32_t period)
{
        int32_t avg_current, offset_current;

        avg_current = SOCF_COUNT_TO_CURRENT(count, period);

        if (count > 0) {
                offset_current = 500 - (250 * avg_current) / 5000;
        } else {
                offset_current = SOCF_DEFAULT_OFFSET_CURRENT - (avg_current * 50) / 1500;
        }

        return (count - SOCF_CURRENT_TO_COUNT(offset_current, period));
}

static int32_t socf_get_avg_current(int64_t count, uint32_t period)
{
        count = socf_compensation_count(count, period);
        return (SOCF_COUNT_TO_CURRENT(count, period));
}

static int16_t socf_get_soc_vbat_ibat(int16_t now_voltage, int32_t current, const int16_t *v_low,
        const int16_t *v_high)
{
        int16_t i;
        int16_t vol[VOL2SOC_LUT_SIZE];

        if (-current < SOCF_BATT_LOW_CURRENT) {
                return (socf_chk_lut(v_low, socf_ref, now_voltage, VOL2SOC_LUT_SIZE));
        } else {
                if (-current > SOCF_MAX_ALLOWED_DISCHA_CURRENT) {
                        current = -SOCF_MAX_ALLOWED_DISCHA_CURRENT;
                }

                for (i = 0; i < VOL2SOC_LUT_SIZE; i++) {
                        vol[i] = (v_low[i]
                                + ((-current - SOCF_BATT_LOW_CURRENT)
                                        * (v_high[i] - v_low[i]))
                                        / (SOCF_BATT_HIGH_CURRENT - SOCF_BATT_LOW_CURRENT));
                }
                return (socf_chk_lut(vol, socf_ref, now_voltage, VOL2SOC_LUT_SIZE));
        }
}

static int64_t socf_get_count(bool reset)
{
        uint64_t coulumb_count;

        coulumb_count = hw_soc_read_charge();
        if (reset == true) {
                hw_soc_reset_charge();
                hw_cpm_delay_usec(10);
                hw_soc_release_counters();
        }

        if ((coulumb_count & (uint64_t)0x8000000000) != 0) {
                return (-(int64_t)(coulumb_count ^ 0xFFFFFFFFFF));
        } else {
                return ((int64_t)coulumb_count);
        }
}

void socf_full_charged_notification()
{
#if (SOCF_BATT_CAPACITANCE_ADJ == 1)
        int16_t voltage;

        if (socf_soc == 0) {
               return;
        }

        if (socf_soc < SOCF_MAX_SOC_VAL) {
                socf_batt_capacitance = (socf_batt_capacitance * (socf_soc + 5))
                        / SOCF_MAX_SOC_VAL;
                socf_soc = SOCF_MAX_SOC_VAL;
                socf_accum_charge_count = SOCF_SOC_TO_COUNT(socf_soc);
        }

        voltage = socf_vbat_via_adapter();

        if (voltage > (vol_dis_low[VOL2SOC_LUT_SIZE - 1] + socf_voltage_comp)) {
                socf_voltage_comp = 0;
        } else {
                socf_voltage_comp = socf_voltage_comp + vol_dis_low[VOL2SOC_LUT_SIZE - 1] - voltage;
        }
#endif
}

/*If you want use a certain capacitance, this set function must be called before soc_init_soc */
void socf_set_capacity(int16_t capacitance)
{
        socf_batt_capacitance = capacitance;
}

int16_t socf_get_capacity(void)
{
        return socf_batt_capacitance;
}

int16_t socf_get_soc(void)
{
        return socf_soc;
}

#define SOCF_TRANS_CONST7 (SOCF_CHARGING_CURRENT * 1000)
static int16_t socf_get_soc_active(void)
{
        int64_t active_count;
        uint64_t now_time, active_period;
        int16_t new_soc, trans_voltage;

        now_time = rtc_get_fromISR();
        active_period = SOCF_GET_DURATION(now_time, socf_start_count_time);

        if (active_period > SOCF_MIN_CURRENT_CHECK_TIME) {
                active_count = socf_get_count(true);

                if (hw_charger_check_vbat() == false) {
                        socf_start_count_time = now_time;
                        socf_pre_sample_time = now_time;
                        return socf_soc;
                }

                active_count = socf_compensation_count(active_count, active_period);
                socf_accum_charge_count = socf_accum_charge_count + active_count;
                if (socf_accum_charge_count < 0) {
                        socf_accum_charge_count = 0;
                }

                if (socf_accum_charge_count > SOCF_SOC_TO_COUNT(SOCF_MAX_SOC_VAL)) {
                        socf_accum_charge_count = SOCF_SOC_TO_COUNT(SOCF_MAX_SOC_VAL);
                        new_soc = 1000;
                } else {
                        new_soc = SOCF_COUNT_TO_SOC(socf_accum_charge_count);
                        if (new_soc == 999) {
                                new_soc = 1000;
                        }
                }

                socf_start_count_time = now_time;
                socf_pre_sample_time = now_time;

                if (active_count > 0) {
                        if (socf_soc == SOCF_MIN_SOC_VAL) {
                                trans_voltage = (int16_t)(((*vol_chg_0 - vol_dis_low[0])
                                        * socf_get_avg_current(active_count, active_period))
                                        / SOCF_TRANS_CONST7) + vol_dis_low[0];
                                if (socf_vbat_via_adapter() < trans_voltage) {
                                        socf_accum_charge_count = 0;
                                        return socf_soc;
                                }
                        }
                        socf_soc = new_soc;
                }
                return new_soc;
        } else {
                return socf_soc;
        }
}

static void ad_socf_wake_up_ind(bool arg)
{
        socf_start_count();
}

static bool ad_socf_prepare_for_sleep(void)
{
        int64_t active_count;
        int32_t new_soc;
        uint64_t now_time, active_period;
        int16_t now_voltage;

        now_time = rtc_get_fromISR();
        if (SOCF_GET_DURATION(now_time, socf_pre_sample_time) > SOCF_SAMPLING_TIME) {
                active_count = socf_get_count(false);
                active_period = SOCF_GET_DURATION(now_time, socf_start_count_time);
                if (active_period > SOCF_MIN_CURRENT_CHECK_TIME) {
                        if (active_count > 0) {
                                return true;
                        } else {
                                new_soc = socf_get_avg_current(active_count, active_period);
                                now_voltage = ((int32_t)socf_measure_vbat() * 30 * (int32_t)socf_batt_capacitance - (new_soc * 2))
                                        / (30 * (int32_t)socf_batt_capacitance);
                                new_soc = socf_get_soc_vbat_ibat(now_voltage, new_soc, vol_dis_low,
                                        vol_dis_high);
                        }

                        if (new_soc < socf_soc) {
                                socf_soc = SOCF_AVG_FILTER(socf_soc, new_soc);
                        }

                        socf_pre_sample_time = now_time;
                        socf_accum_charge_count = SOCF_SOC_TO_COUNT(socf_soc);
                }
        }
        return true;
}

void socf_start_timer(void)
{
        if (xSocfTimer != 0) {
                OS_TIMER_STOP_FROM_ISR(xSocfTimer);
                OS_TIMER_CHANGE_PERIOD_FROM_ISR(xSocfTimer, OS_MS_2_TICKS(1000));
                OS_TIMER_START_FROM_ISR(xSocfTimer);
        }
}

static void socf_timer_cb(TimerHandle_t timer)
{
        socf_get_soc_active();
        if (pm_get_sleep_mode() <= pm_mode_idle) {
                socf_soc = SOCF_COUNT_TO_SOC(socf_accum_charge_count);
                if (socf_soc == 999) {
                        socf_soc = 1000;
                }
                OS_TIMER_CHANGE_PERIOD_FROM_ISR(xSocfTimer, OS_MS_2_TICKS(1000));
                OS_TIMER_START_FROM_ISR(xSocfTimer);
        }
}

#if defined(CONFIG_RETARGET) && defined(DEBUG_SOC)
static void socf_report_init(void);
#endif

void socf_set_init_soc(int16 soc)
{
        socf_soc = soc;
        socf_accum_charge_count = SOCF_SOC_TO_COUNT(socf_soc);
}

static const adapter_call_backs_t ad_socf_pm_call_backs = {
        .ad_prepare_for_sleep = ad_socf_prepare_for_sleep,
        .ad_sleep_canceled = NULL,
        .ad_wake_up_ind = ad_socf_wake_up_ind,
        .ad_xtal16m_ready_ind = NULL,
        .ad_sleep_preparation_time = 0
};

void socf_init_soc(void)
{
        int64_t active_count;
        uint64_t active_period;
        int32_t current;
#if dg_configSOC_GAIN_ERR_FROM_FLASH
        nvparam_t param_area;
        uint16_t len;
#endif
        int16_t now_voltage;

        pm_register_adapter(&ad_socf_pm_call_backs);

#if dg_configSOC_GAIN_ERR_FROM_FLASH
        param_area = ad_nvparam_open("ble_app");
        len = ad_nvparam_read(param_area, TAG_SOC_BATT_VOLTAGE_GAIN_CAL, sizeof(int16_t), &socf_batt_voltage_gain_cal);
        if (len != sizeof(int16_t)) {
                socf_batt_voltage_gain_cal = 10000;
        }

        ad_nvparam_close(param_area);
#else
        if (hw_gpadc_single_ended_gain_error == 0x0) {
                socf_batt_voltage_gain_cal = 10000;
        } else {
                socf_batt_voltage_gain_cal = hw_gpadc_single_ended_gain_error;
        }
#endif
        socf_pre_sample_time = rtc_get_fromISR();

        if (hw_charger_check_vbat() == false) {
                active_count = socf_get_count(true);
                socf_soc = SOCF_MIN_SOC_VAL;
                socf_accum_charge_count = SOCF_SOC_TO_COUNT(socf_soc);
        } else {
                active_period = SOCF_GET_DURATION(socf_pre_sample_time,
                        socf_start_count_time);
                if (active_period < 10) {
                        hw_cpm_delay_usec((10 - active_period) * 1000);
                        socf_pre_sample_time = rtc_get_fromISR();
                        active_period = SOCF_GET_DURATION(socf_pre_sample_time,
                                               socf_start_count_time);
                }
                active_count = socf_get_count(true);
                current = socf_get_avg_current(active_count, active_period);
                now_voltage = (socf_vbat_via_adapter() + socf_vbat_via_adapter()
                        + socf_vbat_via_adapter()) / 3;
                socf_soc = socf_get_soc_vbat_ibat(now_voltage, current, vol_dis_low, vol_dis_high);
                socf_accum_charge_count = SOCF_SOC_TO_COUNT(socf_soc);
        }
#if defined(CONFIG_RETARGET) && defined(DEBUG_SOC)
        socf_report_init();
#endif
        xSocfTimer = OS_TIMER_CREATE("SOC_Timer",
                        OS_MS_2_TICKS(1000),
                        OS_TIMER_FAIL,
                        (void *)0,
                        socf_timer_cb);
        OS_ASSERT(xSocfTimer != NULL);

        OS_TIMER_START(xSocfTimer, OS_TIMER_FOREVER);
}

void socf_init()
{
        socf_batt_capacitance = SOCF_BATTERY_CAPACITANCE;

        socf_start_count();

#if (SOCF_SECONDARY_BATT == 1)
        if (socf_read_batt_id() > SOCF_BATT_TYPE_TH) {
                vol_dis_low = vol_dis_low_1;
                vol_dis_high = vol_dis_high_1;
                vol_chg_0 = &vol_chg_1_0;
        } else {
                vol_dis_low = vol_dis_low_0;
                vol_dis_high = vol_dis_high_0;
                vol_chg_0 = &vol_chg_0_0;
        }
#else
        vol_dis_low = (int16_t *)vol_dis_low_0;
        vol_dis_high = (int16_t *)vol_dis_high_0;
        vol_chg_0 = (int16_t *)&vol_chg_0_0;
#endif
}

#if defined(CONFIG_RETARGET) && defined(DEBUG_SOC)
#define SOCF_NOTIF              (1 << 16)
#define socf_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1 )
__RETAINED static int32_t socf_log_time = 0;
PRIVILEGED_DATA static OS_TASK xSocfTaskHandle;
PRIVILEGED_DATA static TimerHandle_t xSocfReport;

static void socf_report_cb(TimerHandle_t timer)
{
        OS_TASK task = (OS_TASK)pvTimerGetTimerID(timer);
        OS_TASK_NOTIFY(task, SOCF_NOTIF, eSetBits);
}

/**
 * \brief SOCF's Task function.
 *
 * \param [in] pvParameters ignored.
 *
 */
static void socf_task(void *pvParameters)
{
        uint32_t ulNotifiedValue;
        OS_BASE_TYPE xResult __attribute__((unused));

        xSocfReport = OS_TIMER_CREATE("SOC_Report",
                                OS_MS_2_TICKS(SOCF_PT_REPORT_TIME),
                                OS_TIMER_FAIL,
                                (void *) OS_GET_CURRENT_TASK(),
                                socf_report_cb);
        OS_ASSERT(xSocfReport != NULL);

        OS_TIMER_START(xSocfReport, OS_TIMER_FOREVER);

#if (SOC_ALWAYS_ACTIVE ==1)
        pm_stay_alive();
#endif
        do {
                xResult = OS_TASK_NOTIFY_WAIT(0x0, 0xFFFFFFFF, &ulNotifiedValue,
                        portMAX_DELAY);

                if (ulNotifiedValue & SOCF_NOTIF) {
                        int16_t vol;

                        socf_log_time++;
                        vol = socf_vbat_via_adapter();

                        printf("[%8ld sec] DLG_SWFG SOC=%4d VOL=%4d\r\n", socf_log_time, socf_soc,
                                vol);

                        OS_TIMER_START(xSocfReport, OS_TIMER_FOREVER);
                }
        } while (1);
}

static void socf_report_init(void)
{
        OS_BASE_TYPE status;

        status = OS_TASK_CREATE("SOCF",
                                socf_task,
                                ( void * ) NULL,
                                768,
                                socf_TASK_PRIORITY,
                                xSocfTaskHandle);
        OS_ASSERT(status == OS_OK);

        (void)status;
}
#endif

ADAPTER_INIT_DEP1(ad_socf_adapter, socf_init_soc, ad_gpadc_adapter)

#endif /* dg_configUSE_SOC */
