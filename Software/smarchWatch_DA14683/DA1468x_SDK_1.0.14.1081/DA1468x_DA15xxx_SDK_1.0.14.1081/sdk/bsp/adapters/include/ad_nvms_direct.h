/**
 * \addtogroup BSP
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup NVMS_DIRECT_ADAPTER
 *
 * \brief NVMS direct driver
 * 
 * NVMS direct driver allows to write to flash without explicit erase.
 * To achieve this, driver has to have RAM memory buffer to hold flash sector in case erase
 * is needed.
 * For flexibility of driver, it's possible to configure how memory needed for sector is handled.
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_nvms_direct.h
 *
 * @brief NVMS direct access driver API
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_NVMS_DIRECT_H_
#define AD_NVMS_DIRECT_H_

#if dg_configNVMS_ADAPTER

#include <ad_nvms.h>

/*
 * 1. Dynamic sector buffer allocates memory when write changes data in a way that erase
 * is required. After writing data RAM is freed. In low memory condition this can lead to write
 * failure.
 * 2. Static sector buffer, in this case driver keeps sector size memory buffer in no retention
 * RAM all the time. Memory is always available.
 * 3. No sector buffer, write will fail if sector is not manually erased before.
 */
#define DIRECT_DRIVER_DYNAMIC_SECTOR_BUF 1
#define DIRECT_DRIVER_STATIC_SECTOR_BUF  2
#define DIRECT_DRIVER_NO_SECTOR_BUF      3

extern const partition_driver_t ad_nvms_direct_driver;

/**
 * \brief Initialize NVMS direct access driver
 *
 */
void ad_nvms_direct_init(void);

#endif /* dg_configNVMS_ADAPTER */

#endif /* AD_NVMS_DIRECT_H_ */

/**
 \}
 \}
 \}
 */
