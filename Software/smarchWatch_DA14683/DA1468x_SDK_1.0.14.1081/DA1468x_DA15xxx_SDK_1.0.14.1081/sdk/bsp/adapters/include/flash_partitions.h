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

#define FLASH_SECTOR_SIZE 0x1000

#define PARTITION_TABLE_ADDR (0x080000 - (FLASH_SECTOR_SIZE))

PARTITION_TABLE_BEGIN
#include <partition_table.h>
PARTITION_TABLE_END

#endif /* FLASH_PARTITIONS_H_ */

