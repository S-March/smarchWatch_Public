/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup Temperature_Sensor
 * \{
 * \brief Temperature Sensor
 */

/**
 ****************************************************************************************
 *
 * @file hw_tempsens.h
 *
 * @brief Implementation of the Hardware Temperature Sensor interface abstraction layer.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_TEMPSENS_H
#define HW_TEMPSENS_H

#if dg_configUSE_HW_TEMPSENS

#include <stdint.h>
#include <sdk_defs.h>

/**
 * \brief Enable temperature sensor
 *
 * Temperature sensor is a part of charging circuit and can be read during charging only. This
 * allows to enable it even when charger is off.
 */
void hw_tempsens_enable(void);

/**
 * \brief Disable temperature sensor
 *
 * Even when disabled, temperature sensor can still be read when charger is on.
 *
 * \sa hw_tempsens_enable
 *
 */
void hw_tempsens_disable(void);

/**
 * \brief Prepare ADC for temperature measurement
 *
 * Temperature sensor can be read using ADC thus it needs to be configured properly before
 * any measurement can be done.
 *
 * \sa hw_tempsens_measure
 *
 */
void hw_tempsens_prepare(void);

/**
 * \brief Convert GPADC value to temperature
 *
 * The received digital value from GPADC is converted to proper temperature value in C degrees.
 *
 * \param [in] val      digital GPADC value
 *
 * \return temperature value in C degrees
 *
 */
int hw_tempsens_convert_to_temperature(uint16_t val);

/**
 * \brief Read raw value from temperature sensor
 *
 * \return raw value from temperature sensor
 *
 * \sa hw_tempsens_read
 *
 */
uint16_t hw_tempsens_raw_read(void);

/**
 * \brief Read temperature
 *
 * \return temperature
 *
 * \sa hw_tempsens_raw_read
 *
 */
int hw_tempsens_read(void);

#endif /* dg_configUSE_HW_TEMPSENS */

#endif /* HW_TEMPSENS_H */

/**
 * \}
 * \}
 * \}
 */
