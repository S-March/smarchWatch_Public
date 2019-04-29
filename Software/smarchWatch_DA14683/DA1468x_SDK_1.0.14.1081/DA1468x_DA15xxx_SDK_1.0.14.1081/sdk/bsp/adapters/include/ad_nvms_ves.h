/**
 * \addtogroup BSP
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup NVMS_VES_ADAPTER
 *
 * \brief NVMS driver
 *
 * NVMS driver allows to use flash as Virtual EEPROM.
 * Virtual EEPROM is stored on flash partition provides virtual addressing to store data
 * to flash with wear leveling functionality. Each write to partition at given address will write
 * different flash area. It also provides power failure safety.
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_nvms_ves.h
 *
 * @brief NVMS VES driver API
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_NVMS_VES_H_
#define AD_NVMS_VES_H_

#if dg_configNVMS_VES

#include <ad_nvms.h>

#if dg_configNVMS_VES
/*
 */
extern const partition_driver_t ad_nvms_ves_driver;
#endif

/**
 * \brief Initialize NVMS Virtual EEPROM Storage driver
 *
 */
void ad_nvms_ves_init(void);

/**
 * \brief Default container size.
 */
#ifndef AD_NVMS_VES_CONTAINER_SIZE
#define AD_NVMS_VES_CONTAINER_SIZE              64
#endif

/**
 * \brief Default flash utilization multiplier.
 *
 * For flash size x available address space is:
 * 1. Without CRC
 *   x / AD_NVMS_VES_MULTIPLIER / AD_NVMS_VES_CONTAINER_SIZE * (AD_NVMS_VES_CONTAINER_SIZE - 2).
 * 2. With CRC
 *   x / AD_NVMS_VES_MULTIPLIER / AD_NVMS_VES_CONTAINER_SIZE * (AD_NVMS_VES_CONTAINER_SIZE - 4).
 */
#ifndef AD_NVMS_VES_MULTIPLIER
#define AD_NVMS_VES_MULTIPLIER                  8
#endif

/**
 * \brief NVMS maximum sector count
 *
 * If partition sector count exceeds 256, RAM usage for virtual address translation will double
 */
#ifndef AD_NVMS_MAX_SECTOR_COUNT
#define AD_NVMS_MAX_SECTOR_COUNT                256
#endif

/**
 * \brief Garbage collector threshold
 *
 * Garbage collection threshold allows to choose when sector is allowed to be recycled.
 * Sectors are recycled only if dirty container count is >= threshold.
 *
 * Setting this to -1 will choose most dirty sector during garbage collection.
 *
 * In practice this means that totally dirty sectors are recycled (no need to copy from old sector
 * to new one).
 * During random write stress tests some sector were written 8% more times than average.
 *
 * Setting this to value grater than 0 allows to recycle sectors that still contain valid data.
 * When those sectors are chosen data from partially written sectors is moved to new sector.
 * In practice this mean that more sector erase operation will occur but wear leveling will treat
 * sector more equal.
 * During random write stress, test most often erased sector was 0.2% more used then average.
 * Total sectors erase count was less then 1% higher then when most dirty sectors were strategy was
 * used.
 */
#ifndef AD_NVMS_VES_GC_THRESHOLD
#define AD_NVMS_VES_GC_THRESHOLD                -1
#endif

#endif /* dg_configNVMS_VES */

#endif /* AD_NVMS_VES_H_ */

/**
 \}
 \}
 \}
 */
