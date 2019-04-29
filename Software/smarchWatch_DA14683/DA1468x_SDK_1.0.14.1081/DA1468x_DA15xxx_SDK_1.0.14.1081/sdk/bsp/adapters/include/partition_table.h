/**
 ****************************************************************************************
 *
 * @file adapters/include/partition_table.h
 *
 * Copyright (C) 2016-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

/*
 * When partition_table is not overridden by project defined partition_table
 * this file can be used to select partition table by macro definition.
 *
 * To use layout other than SDK one, add include path in project
 * settings that will point to folder with custom partition_table file.
 */

#if defined(USE_PARTITION_TABLE_16MB)
#include <16M/partition_table.h>
#elif defined(USE_PARTITION_TABLE_2MB)
#include <2M/partition_table.h>
#elif defined(USE_PARTITION_TABLE_2MB_WITH_SUOTA)
#include <2M/suota/partition_table.h>
#elif defined(USE_PARTITION_TABLE_512K)
#include <512K/partition_table.h>
#elif defined(USE_PARTITION_TABLE_512K_WITH_SUOTA)
#include <512K/suota/partition_table.h>
#elif defined(USE_PARTITION_TABLE_1MB_WITH_SUOTA)
#include <1M/suota/partition_table.h>
#else
#include <1M/partition_table.h>
#endif

