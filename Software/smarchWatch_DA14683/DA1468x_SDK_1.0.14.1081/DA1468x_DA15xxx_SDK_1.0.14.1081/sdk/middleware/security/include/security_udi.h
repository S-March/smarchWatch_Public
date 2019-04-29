/**
 * \addtogroup BSP
 * \{
 * \addtogroup MIDDLEWARE
 * \{
 * \addtogroup SECURITY
 *
 * \brief Unique device identifier
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file security_udi.h
 *
 * @brief Declarations for unique device identifier.
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SECURITY_UDI_H_
#define SECURITY_UDI_H_

#if dg_configUSE_HW_OTPC

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Unique device identifier length */
#define UNIQUE_DEVICE_ID_LEN    16

/**
 * \brief Get unique device identifier
 *
 * Function reads two fields from OTP (position/package and tester/timestamp) and joins them into
 * unique device identifier.
 *
 * \note \p udi must point to the buffer which has at least UNIQUE_DEVICE_ID_LEN bytes in length.
 *
 * \param [out] udi     unique device identifier buffer
 *
 * \return true if unique device identifier has been read properly, false otherwise
 *
 * \warning Function uses OTP. hw_otpc_init() and the hw_otpc_set_speed() functions must be called
 * before using this function.
 *
 */
bool security_get_unique_device_id(uint8_t *udi);

#ifdef __cplusplus
}
#endif

#endif /* dg_configUSE_HW_OTPC */

#endif /* SECURITY_UDI_H_ */

/**
 * \}
 * \}
 * \}
 */
