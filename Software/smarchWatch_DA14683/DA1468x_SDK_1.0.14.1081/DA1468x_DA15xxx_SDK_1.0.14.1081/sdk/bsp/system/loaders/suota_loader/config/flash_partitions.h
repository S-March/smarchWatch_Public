/**
 ****************************************************************************************
 *
 * @file flash_partitions.h
 *
 * @brief Default partition table
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef FLASH_PARTITIONS_H_
#define FLASH_PARTITIONS_H_

#ifndef PARTITION_TABLE_BEGIN
#define PARTITION_TABLE_BEGIN
#endif

#ifndef PARTITION_TABLE_ENTRY
/*
 * This to create partition entry in partition table.
 */
#define PARTITION_TABLE_ENTRY(start, size, id)
#endif

#ifndef PARTITION
/*
 * This macro can be used to define partition without specifying size. Size of partition created
 * with this macro will be computed using next partition starting address.
 * When this macro is used partition entries must be put in partition table in ascending starting
 * offset order.
 * Last entry in partition table can't be created with this macro.
 */
#define PARTITION(start, id, flags)
#endif

#ifndef PARTITION2
/*
 * This macro creates partition entry in similar way as PARTITION_TABLE_ENTRY but adds flags field.
 */
#define PARTITION2(start, size, id, flags)
#endif

#ifndef PARTITION_TABLE_END
#define PARTITION_TABLE_END
#endif

/* Uncomment the definition that matches the target Flash size */
#define USE_PARTITION_TABLE_1MB
//#define USE_PARTITION_TABLE_2MB

#define FLASH_SECTOR_SIZE    0x1000
#define PLATFORM_PARAMS_SIZE (2 * (FLASH_SECTOR_SIZE))
#define BOOTLOADER_SIZE      0x1E000

#define PARTITION_TABLE_ADDR (0x080000 - (FLASH_SECTOR_SIZE))
#define PLATFORM_PARAMS_ADDR ((PARTITION_TABLE_ADDR) - (PLATFORM_PARAMS_SIZE))

#define PRODUCT_HEADER_START (BOOTLOADER_SIZE)
#define PRODUCT_IMAGEHEADER_START ((PRODUCT_HEADER_START) + (FLASH_SECTOR_SIZE))
#define EXECUTABLE_START     ((PRODUCT_IMAGEHEADER_START) + (FLASH_SECTOR_SIZE))

/*
 * Default definition of flash partitions
 *
 *                          Start of partition |  Size of partition  |   Partition ID
 */
#if defined USE_PARTITION_TABLE_1MB

PARTITION_TABLE_BEGIN
        /* Boot loader */
        PARTITION(0x000000,                  NVMS_FIRMWARE_PART,        0)
        PARTITION(PRODUCT_HEADER_START,      NVMS_PRODUCT_HEADER_PART,  0)
        PARTITION(PRODUCT_IMAGEHEADER_START, NVMS_IMAGE_HEADER_PART,    0)
        PARTITION(EXECUTABLE_START,          NVMS_FW_EXEC_PART,         0)
        PARTITION(0x06F000,                  NVMS_LOG_PART,             0)
        PARTITION(PLATFORM_PARAMS_ADDR,      NVMS_PLATFORM_PARAMS_PART, PARTITION_FLAG_READ_ONLY)
        PARTITION(PARTITION_TABLE_ADDR,      NVMS_PARTITION_TABLE,      PARTITION_FLAG_READ_ONLY)
        PARTITION_TABLE_ENTRY(0x080000,  0x10000, NVMS_PARAM_PART)
        PARTITION(0x090000,                  NVMS_FW_UPDATE_PART,       0)
        PARTITION2(0x0E1000, 0x1F000,        NVMS_GENERIC_PART,         PARTITION_FLAG_VES)
PARTITION_TABLE_END

#else
#  error "Cannot determine which partition table layout to use!"
#endif

#endif /* FLASH_PARTITIONS_H_ */
