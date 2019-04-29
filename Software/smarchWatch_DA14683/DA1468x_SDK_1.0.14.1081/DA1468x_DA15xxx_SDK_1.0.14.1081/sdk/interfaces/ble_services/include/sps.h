/**
 ****************************************************************************************
 *
 * @file sps.h
 *
 * @brief Serial Port Service sample implementation API
 *
 * Copyright (C) 2015 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SPS_H_
#define SPS_H_

#include "ble_service.h"

/**
 * SPS Flow Control flags values
 */
typedef enum {
        SPS_FLOW_CONTROL_ON = 0x01,
        SPS_FLOW_CONTROL_OFF = 0x02,
} sps_flow_control_t;

typedef void (* sps_set_flow_control_cb_t) (ble_service_t *svc, uint16_t conn_idx, sps_flow_control_t value);

typedef void (* sps_rx_data_cb_t) (ble_service_t *svc, uint16_t conn_idx, const uint8_t *value,
                                                                                uint16_t length);

typedef void (* sps_tx_done_cb_t) (ble_service_t *svc, uint16_t conn_idx, uint16_t length);

/**
 * SPS application callbacks
 */
typedef struct {
        /** Remote client wrote new value of flow control characteristic */
        sps_set_flow_control_cb_t set_flow_control;
        /** Data received from remote client */
        sps_rx_data_cb_t          rx_data;
        /** Service finished TX transaction */
        sps_tx_done_cb_t          tx_done;
} sps_callbacks_t;

/**
 * \brief Register Serial Port Service instance
 *
 * Function registers SPS instance
 *
 * \param [in] cb               application callbacks
 *
 * \return service instance
 *
 */
ble_service_t *sps_init(sps_callbacks_t *cb);

/**
 * \brief Set flow control value
 *
 * Function updates flow control value.
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] value            flow control value
 *
 */
void sps_set_flow_control(ble_service_t *svc, uint16_t conn_idx, sps_flow_control_t value);

/**
 * \brief TX data available
 *
 * Function notifies new data is available for client. After sending data, service
 * will call tx_done callback.
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] data             tx data
 * \param [in] length           tx data length
 *
 */
void sps_tx_data(ble_service_t *svc, uint16_t conn_idx, uint8_t *data, uint16_t length);

#endif /* SPS_H_ */
