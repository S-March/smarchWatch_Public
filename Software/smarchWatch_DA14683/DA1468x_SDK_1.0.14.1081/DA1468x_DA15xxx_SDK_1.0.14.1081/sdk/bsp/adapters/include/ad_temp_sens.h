/**
 * \addtogroup BSP
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup TEMPSENS_ADAPTER
 *
 * \brief Temperature sensor adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_temp_sens.h
 *
 * @brief Temperature Sensor adapter API
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_TEMP_SENS_H_
#define AD_TEMP_SENS_H_

#if dg_configTEMPSENS_ADAPTER

#include <osal.h>
#include <resmgmt.h>
#include <hw_tempsens.h>
#include <hw_gpio.h>
#include <ad_gpadc.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Source pointer, handle to use with ad_tempsens_read() etc.
 *
 */
typedef void *tempsens_source;

/**
 * \brief Asynchronous callback function
 *
 */
typedef ad_gpadc_user_cb ad_tempsens_user_cb;

/**
 * \brief Open access to Temperature Sensor
 *
 * \return handle to Temperature Sensor
 *
 */
tempsens_source ad_tempsens_open(void);

/**
 * \brief Close access to Temperature Sensor
 *
 * \param [in] src      handle to Temperature Sensor source
 *
 */
void ad_tempsens_close(tempsens_source src);

/*
 * \brief Read the value of temperature
 *
 * \param [in] src      handle to Temperature Sensor source
 *
 * \return temperature result in C degrees
 *
 */
int ad_tempsens_read(tempsens_source src);

/**
 * \brief Attempt to read temperature within a timeout period.
 *
 *
 * \param [in]  src    handle to Temperature Sensor source
 * \param [out] value  pointer to store the temperature in C degrees
 * \param [in]  timeout number of ticks to wait
 *              0 - no wait take resource if it is available
 *              RES_WAIT_FOREVER - wait until resource becomes available
 *              Other value specifies how many ticks to wait until resource becomes available
 *
 * \return true if temperature has been read, false otherwise
 *
 */
bool ad_tempsens_read_to(tempsens_source src, int *value, uint32_t timeout);

/*
 * \brief Read asynchronously the value of the temperature
 *
 * \param [in] src              handle to Temperature Sensor source
 * \param [in] cb               user callback fired after read operation completes
 * \param [in] user_data        user data for callback
 *
 */
void ad_tempsens_read_async(tempsens_source src, ad_tempsens_user_cb cb, void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* dg_configTEMPSENS_ADAPTER */

#endif /* AD_TEMP_SENS_H_ */

/**
 * \}
 * \}
 * \}
 */
