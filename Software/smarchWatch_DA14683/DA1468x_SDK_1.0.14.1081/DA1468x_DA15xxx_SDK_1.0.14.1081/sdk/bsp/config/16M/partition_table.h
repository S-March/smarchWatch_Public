/**
 ****************************************************************************************
 *
 * @file 16M/partition_table.h
 *
 * Copyright (C) 2016-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

PARTITION2( 0x0000000 , 0x007F000 , NVMS_FIRMWARE_PART        , 0 )
PARTITION2( 0x007F000 , 0x0001000 , NVMS_PARTITION_TABLE      , PARTITION_FLAG_READ_ONLY )
PARTITION2( 0x0080000 , 0x0010000 , NVMS_PARAM_PART           , 0 )
PARTITION2( 0x0090000 , 0x0030000 , NVMS_BIN_PART             , 0 )
PARTITION2( 0x00C0000 , 0x0020000 , NVMS_LOG_PART             , 0 )
PARTITION2( 0x00E0000 , 0x0020000 , NVMS_GENERIC_PART         , PARTITION_FLAG_VES )
PARTITION2( 0x0100000 , 0x0F00000 , NVMS_FLASH_STORAGE        , 0 )

