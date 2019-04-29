/**
 * \addtogroup BSP
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup BATTERY_ADAPTER
 *
 * \brief Battery adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_battery.h
 *
 * @brief Battery adapter API
 *
 * Copyright (C) 2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_BATTERY_H_
#define AD_BATTERY_H_

#if dg_configBATTERY_ADAPTER

#include <osal.h>
#include <resmgmt.h>
#include <hw_gpio.h>
#include <ad_gpadc.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Source pointer, handle to use with ad_battery_read() etc.
 *
 */
typedef void *battery_source;

/**
 * \brief Asynchronous callback function
 *
 */
typedef ad_gpadc_user_cb ad_battery_user_cb;

/**
 * \brief Open access to battery
 *
 * \return handle to battery
 *
 */
battery_source ad_battery_open(void);

/**
 * \brief Close access to battery
 *
 * \param [in] src      handle to battery source
 *
 */
void ad_battery_close(battery_source src);

/**
 * \brief Read the value of battery voltage level
 *
 * Reads ADC value of battery it can be from 10 to 16 bits depending on oversampling
 * selected in source description. Use ad_battery_raw_to_mvolt() to convert this value
 * to mV.
 *
 * \param [in] src      handle to battery source
 *
 * \return raw battery voltage level
 *
 * \sa ad_battery_raw_to_mvolt
 *
 */
uint16_t ad_battery_read(battery_source src);

/**
 * \brief Read asynchronously the value of the battery voltage level
 *
 * Reads ADC value of battery it can be from 10 to 16 bits depending on oversampling
 * selected in source description. When ADC conversion finishes user callback is called.
 * Use ad_battery_raw_to_mvolt() to convert this value to mV.
 *
 * \param [in] src              handle to battery source
 * \param [in] cb               user callback fired after read operation completes
 * \param [in] user_data        user data for callback
 *
 * \sa ad_battery_raw_to_mvolt
 *
 */
void ad_battery_read_async(battery_source src, ad_battery_user_cb cb, void *user_data);

/**
 * \brief Convert raw value read from ADC to batter voltage in mV
 *
 * \param [in] src   handle to battery source
 * \param [in] value value returned from ad_battery_read() or ad_battery_read_async()
 *
 * \return value in mV
 *
 */
static inline uint16_t ad_battery_raw_to_mvolt(battery_source src, uint32_t value)
{
        /* Convert to mV, take into account that scaler from 5->1.2 */
        return (uint16_t) (((uint32_t) 5000 * value) / ad_gpadc_get_source_max(src));
}

#ifdef __cplusplus
}
#endif

#endif /* dg_configBATTERY_ADAPTER */

#endif /* AD_BATTERY_H_ */

/**
 * \}
 * \}
 * \}
 */
