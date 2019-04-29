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
 * @file security_udi.c
 *
 * @brief Unique device identifier
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_OTPC

#include <stdbool.h>
#include <string.h>
#include "security_udi.h"
#include "hw_otpc.h"
#include "sdk_defs.h"

#define POSITION_PACKAGE_ADDRESS        0x7F8EA00
#define TESTER_TIMESTAMP_ADDRESS        0x7F8EA08

bool security_get_unique_device_id(uint8_t *udi)
{
        uint32_t cell_offset;

        memset(udi, 0, UNIQUE_DEVICE_ID_LEN);
        cell_offset = ((POSITION_PACKAGE_ADDRESS - MEMORY_OTP_BASE) >> 3);

        if (!hw_otpc_fifo_read((uint32_t *) udi, cell_offset, HW_OTPC_WORD_LOW, 2, false)) {
                return false;
        }

        cell_offset = ((TESTER_TIMESTAMP_ADDRESS - MEMORY_OTP_BASE) >> 3);

        /* Position/package has 8 bytes in length */
        if (!hw_otpc_fifo_read((uint32_t *) (udi + 8), cell_offset, HW_OTPC_WORD_LOW, 2, false)) {
                return false;
        }

        return true;
}

#endif /* dg_configUSE_HW_OTPC */

/**
 * \}
 * \}
 * \}
 */
