/**
 ****************************************************************************************
 *
 * @file scps.h
 *
 * @brief Scan Parameters Service sample implementation API
 *
 * Copyright (C) 2015 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SCPS_H_
#define SCPS_H_

#include <stdint.h>
#include "ble_service.h"

/**
 * \brief Scan Interval Window value updated by client callback
 *
 * \param [in] conn_idx connection index
 * \param [in] interval scan interval
 * \param [in] window   scan window
 *
 */
typedef void (* scps_scan_updated_cb_t) (uint16_t conn_idx, uint16_t interval, uint16_t window);

/**
 * \brief CCC for Scan Interval Window updated by client callback
 *
 * \param [in] conn_idx connection index
 * \param [in] value    value written by client
 *
 */
typedef void (* scps_ccc_changed_cb_t) (uint16_t conn_idx, uint16_t value);

/**
 * \brief Client disconnected callback
 *
 * ScPS client disconnected, last known values of Scan Interval and Scan Window are passed.
 * Application should store these values if required since they are no longer maintained
 * by ScPS instance.
 *
 * \param [in] conn_idx connection index
 * \param [in] interval scan interval
 * \param [in] window   scan window
 *
 */
typedef void (* scps_disconnected_cb_t) (uint16_t conn_idx, uint16_t interval, uint16_t window);

/** ScPS application callbacks */
typedef struct {
        /** Scan Interval Window value updated by client */
        scps_scan_updated_cb_t  scan_updated;

        /** CCC for Scan Interval Window updated by client */
        scps_ccc_changed_cb_t   ccc_changed;

        /** ScPS client disconnected, last known values of Scan Interval Window are passed.
         *  Application should store these values if required since they are no longer maintained
         *  by ScPS instance.
         */
        scps_disconnected_cb_t  disconnected;
} scps_callbacks_t;

/**
 * \brief ScPS initialization
 *
 * \param [in] cb       structure with ScPS callbacks
 *
 */
ble_service_t *scps_init(const scps_callbacks_t *cb);

/**
 * \brief Request Scan Refresh from client
 *
 * \param [in] svc      service instance
 * \param [in] conn_idx connection index
 *
 */
void scps_notify_refresh(ble_service_t *svc, uint16_t conn_idx);

/**
 * \brief Request Scan Refresh from all connected clients
 *
 * \param [in] svc      service instance
 *
 */
void scps_notify_refresh_all(ble_service_t *svc);

#endif /* SCPS_H_ */
