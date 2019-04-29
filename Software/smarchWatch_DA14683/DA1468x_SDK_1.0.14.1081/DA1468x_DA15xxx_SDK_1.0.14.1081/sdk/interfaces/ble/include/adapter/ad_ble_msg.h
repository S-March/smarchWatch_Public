/**
 \addtogroup INTERFACES
 \{
 \addtogroup BLE
 \{
 \addtogroup ADAPTER
 \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_ble_msg.h
 *
 * @brief BLE Adapter message definitions
 *
 * Copyright (C) 2015 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_BLE_MSG_H_
#define AD_BLE_MSG_H_

#include "ad_ble.h"

typedef void (* ad_ble_msg_handler_t) (ad_ble_msg_t *msg);

// BLE adapter complete event message
typedef struct {
        ad_ble_operation_t op_req;
        ad_ble_status_t    status;
} ad_ble_cmp_evt_t;


#endif /* AD_BLE_MSG_H_ */
/**
 \}
 \}
 \}
 */
