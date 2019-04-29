/**
 * \addtogroup BSP
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup NVMS_ADAPTER
 *
 * \brief NVMS adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_nvms.h
 *
 * @brief NVMS adapter API
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_NVMS_H_
#define AD_NVMS_H_

#if dg_configNVMS_ADAPTER

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <partition_def.h>
#include <ad_flash.h>

/*
 * DEFINES
 ****************************************************************************************
 */

typedef void *nvms_t;

/**
 * \brief Initialize NVMS adapter.
 */
void ad_nvms_init(void);

/**
 * \brief Open partition to read/write access.
 *
 * There can be only one partition with given id.
 *
 * \param [in] id partition to get access to
 *
 * \returns handle to use for partition access, NULL if partition does not exists
 */
nvms_t ad_nvms_open(nvms_partition_id_t id);

/**
 * \brief Get partition size.
 *
 * \return size in bytes of partition
 */
size_t ad_nvms_get_size(nvms_t handle);

/**
 * \brief Read partition data.
 *
 * \param [in] handle partition handle
 * \param [in] addr starting offset
 * \param [out] buf buffer to read data to
 * \param [in] len number of bytes to read
 *
 * \returns number of bytes read, < 0 in case of error
 */
int ad_nvms_read(nvms_t handle, uint32_t addr, uint8_t *buf, uint32_t len);

/**
 * \brief Writes data to partition.
 *
 * This function allows to write any number of bytes to partition.
 *
 * \param [in] handle partition handle
 * \param [in] addr offset in partition to write data to
 * \param [in] buf pointer to data to write
 * \param [in] size number of bytes to write
 *
 * return number of bytes written, < 0 if case of error
 *
 */
int ad_nvms_write(nvms_t handle, uint32_t addr, const uint8_t *buf, uint32_t size);

/**
 * \brief Erase partition region.
 *
 * \note: All sectors that have offset in range of <addr, add + size) will be erased.
 *
 * \param [in] handle partition handle
 * \param [in] addr starting offset of sector
 * \param [in] size number of bytes to erase
 *
 * \return true on success
 *
 */
bool ad_nvms_erase_region(nvms_t handle, uint32_t addr, size_t size);

/**
 * \brief Get direct read buffer pointer.
 *
 * Function will fill ptr with CPU address that can be used to directly access partition data.
 * Parameter size specifies how many bytes caller wants to access. Note however that returned
 * size can be smaller if addr + size exceeds partition size. This value can also be smaller
 * if partition uses wear leveling that results in non linear data storage.
 * If partition driver does not support direct mapping function will return 0 and no valid ptr will
 * be returned.
 *
 * \param [in] handle partition handle
 * \param [in] addr requested address
 * \param [in] size number of bytes
 * \param [out] ptr pointer to store direct address
 *
 * \return number of bytes that can be accessed with this pointer
 *
 */
size_t ad_nvms_get_pointer(nvms_t handle, uint32_t addr, size_t size, const void **ptr);

/**
 * \brief Get partition erase size.
 *
 * \return minimum size that can be erased
 *
 */
size_t ad_nvms_erase_size(void);

/**
 * \brief NVMS partition type
 */
struct partition_t;

/**
 * \brief Partition driver functions.
 */
typedef struct driver_t {
        bool (* bind)(struct partition_t *part);
        size_t (* get_size)(struct partition_t *part);
        int (* read)(struct partition_t *part, uint32_t addr, uint8_t *buf, uint32_t size);
        int (* write)(struct partition_t *part, uint32_t addr, const uint8_t *buf, uint32_t size);
        bool (* erase)(struct partition_t *part, uint32_t addr, uint32_t size);
        size_t (* get_ptr)(struct partition_t *part, uint32_t addr, uint32_t size, const void **ptr);
        void (* flush)(struct partition_t *part, bool free_mem);
} partition_driver_t;

/**
 * \brief Partition structure for RAM
 */
typedef struct partition_t {
        struct partition_t *next;        /**< Next partition */
        const partition_driver_t *driver;/**< Bound driver */
        void *driver_data;               /** Driver data */
        partition_entry_t data;          /** Partition information data */
} partition_t;

/**
 * \brief Get partition count
 *
 * \return Number of partitions
 *
 */
size_t ad_nvms_get_partition_count(void);

/**
 * \brief Get partition info
 *
 * \return true if partition exists, false otherwise
 *
 */
bool ad_nvms_get_partition_info(size_t index, partition_entry_t *info);

/**
 * \brief Control cache flushing on modifications (writes or erases) to a partition.
 *
 * This function can be used to define a partition region that will not trigger cache
 * flushing when writes or erases occur in that region. Only one such partition region
 * can be active at any time, in total.
 *
 * This feature is useful when the programmer knows in advance that a partition region
 * is going to be updated (e.g. firmware update). However, reads from that region
 * should be avoided, as they might lead to cache incoherency.
 *
 * \note The effect of this function is at ad_flash_* layer and higher.
 *       For example, direct use of the hw_qspi_* API will not be affected by the use
 *       of this function.
 *
 * \param [in] handle  Partition handle.
 * \param [in] base    Starting offset of the partition region that should not trigger
 *                     cache flushes.
 * \param [in] size    The size of the partition region that should not trigger cache
 *                     flushes. If size is such that it would extend beyond the end
 *                     of the partition, the region size is truncated to match the
 *                     end of the partition.
 *                     If size is 0, selective cache flushing is disabled.
 *
 * \return true if partition exists and base , false otherwise
 *
 * \sa ad_nvms_mandatory_cache_flushing()
 */
bool ad_nvms_no_cache_flushing(nvms_t handle, uint32_t base, uint32_t size);

/**
 * Turn off any selective cache flushing that may have been enabled by
 * \sa ad_nvms_no_cache_flushing().
 *
 */
static inline void ad_nvms_mandatory_cache_flushing(void)
{
        ad_flash_skip_cache_flushing(AD_FLASH_ALWAYS_FLUSH_CACHE, 0);
}

/**
 * \brief Flush all data buffered in RAM to partition.
 *
 * In order to improve erase/write performance, data may not be written to storage device
 * immediately. Call this function to make sure all data is written.
 *
 * \param [in] handle partition handle
 *
 * \param [in] free_mem true - free allocated memory for cached data,
 *                      false - flush without freeing memory for cached data. Copy of data still
 *                              exists in RAM cache
 */
void ad_nvms_flush(nvms_t handle, bool free_mem);

#endif /* dg_configNVMS_ADAPTER */

#endif /* AD_NVMS_H_ */

/**
 \}
 \}
 \}
 */
