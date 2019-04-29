/**
 ****************************************************************************************
 *
 * @file uartboot_types.h
 *
 * @brief Common type definitions
 *
 * Copyright (C) 2015 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef UARTBOOT_TYPES_H
#define UARTBOOT_TYPES_H

#ifdef __cplusplus
 extern "C" {
#endif


#include <stdint.h>


/*                 Memory layout
 * +=====================+==========================+
 * +        len          +          table           +
 * +    sector_size      +                          +
 * +=====================+==========================+
 * +    start_sector     +          entry 1         +
 * +    sector_count     +                          +
 * +    type             +                          +
 * +    name             +                          +
 * +=====================+==========================+
 * +        len          +          name            +
 * +                     +                          +
 * +                     +                          +
 * +                     +                          +
 * +        \0           +                          +
 * +---------------------+--------------------------+
 * +           potential   padding                  +
 * +=====================+==========================+
 * +    start_sector     +          entry N         +
 * +    sector_count     +                          +
 * +    type             +                          +
 * +    name             +                          +
 * +=====================+==========================+
 * +        len          +          name            +
 * +                     +                          +
 * +        \0           +                          +
 * +---------------------+--------------------------+
 * +                                                +
 * +           potential   padding                  +
 * +                                                +
 * +=====================+==========================+
 */

typedef struct {
        uint16_t len;
        uint8_t str;
} cmd_partition_name_t;

typedef struct {
        uint16_t start_sector;
        uint16_t sector_count;
        uint8_t  type;
        cmd_partition_name_t name;
} cmd_partition_entry_t;

typedef struct {
        uint16_t len;
        uint16_t sector_size;
        cmd_partition_entry_t entry;
} cmd_partition_table_t;

#ifdef __cplusplus
}
#endif

#endif /* UARTBOOT_TYPES_H */
