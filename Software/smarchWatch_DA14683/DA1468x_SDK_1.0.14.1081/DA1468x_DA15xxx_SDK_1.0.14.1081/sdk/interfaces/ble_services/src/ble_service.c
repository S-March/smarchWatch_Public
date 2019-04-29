/**
 ****************************************************************************************
 *
 * @file ble_service.c
 *
 * @brief Services handling routines
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stddef.h>
#include <ble_service.h>
#include "FreeRTOS.h"

#if CONFIG_BLE_SERVICES_MAX_NUM > 0
#       define MAX_SERVICES (CONFIG_BLE_SERVICES_MAX_NUM)
#else
#       define MAX_SERVICES (10)
#endif

#define PERM_READ_MASK  (ATT_PERM_READ | ATT_PERM_READ_ENCRYPT | ATT_PERM_READ_AUTH)
#define PERM_WRITE_MASK (ATT_PERM_WRITE | ATT_PERM_WRITE_ENCRYPT | ATT_PERM_WRITE_AUTH)

PRIVILEGED_DATA static ble_service_t *services[MAX_SERVICES];

static ble_service_t *find_service_by_handle(uint16_t handle)
{
        int i;

        for (i = 0; i < MAX_SERVICES; i++) {
                ble_service_t *svc = services[i];

                if (svc && handle >= svc->start_h && handle <= svc->end_h) {
                        return svc;
                }
        }

        return NULL;
}

void ble_service_add(ble_service_t *svc)
{
        int i;

        for (i = 0; i < MAX_SERVICES; i++) {
                if (services[i] == svc) {
                        return;
                }
        }

        for (i = 0; i < MAX_SERVICES; i++) {
                if (!services[i]) {
                        services[i] = svc;
                        break;
                }
        }
}

void ble_service_remove(ble_service_t *svc)
{
        int i;

        for (i = 0; i < MAX_SERVICES; i++) {
                if (services[i] == svc) {
                        services[i] = NULL;
                        break;
                }
        }
}

void ble_service_cleanup(ble_service_t *svc)
{
        if (!svc || !svc->cleanup) {
                return;
        }

        svc->cleanup(svc);
}

static void connected_evt(const ble_evt_gap_connected_t *evt)
{
        int i;

        for (i = 0; i < MAX_SERVICES; i++) {
                ble_service_t *svc = services[i];

                if (svc && svc->connected_evt) {
                        svc->connected_evt(svc, evt);
                }
        }
}

static void disconnected_evt(const ble_evt_gap_disconnected_t *evt)
{
        int i;

        for (i = 0; i < MAX_SERVICES; i++) {
                ble_service_t *svc = services[i];

                if (svc && svc->disconnected_evt) {
                        svc->disconnected_evt(svc, evt);
                }
        }
}

static bool read_req(const ble_evt_gatts_read_req_t *evt)
{
        ble_service_t *svc = find_service_by_handle(evt->handle);

        if (!svc) {
                return false;
        }

        if (svc->read_req) {
                svc->read_req(svc, evt);
        }

        return true;
}

static bool write_req(const ble_evt_gatts_write_req_t *evt)
{
        ble_service_t *svc = find_service_by_handle(evt->handle);

        if (!svc) {
                return false;
        }

        if (svc->write_req) {
                svc->write_req(svc, evt);
        }

        return true;
}

static bool prepare_write_req(const ble_evt_gatts_prepare_write_req_t *evt)
{
        ble_service_t *svc = find_service_by_handle(evt->handle);

        if (!svc) {
                return false;
        }

        if (svc->prepare_write_req) {
                svc->prepare_write_req(svc, evt);
        }

        return true;
}

static bool event_sent(const ble_evt_gatts_event_sent_t *evt)
{
        ble_service_t *svc = find_service_by_handle(evt->handle);

        if (!svc) {
                return false;
        }

        if (svc->event_sent) {
                svc->event_sent(svc, evt);
        }

        return true;
}

bool ble_service_handle_event(const ble_evt_hdr_t *evt)
{
        switch (evt->evt_code) {
        case BLE_EVT_GAP_CONNECTED:
                connected_evt((const ble_evt_gap_connected_t *) evt);
                return false; // make it "not handled" so app can handle
        case BLE_EVT_GAP_DISCONNECTED:
                disconnected_evt((const ble_evt_gap_disconnected_t *) evt);
                return false; // make it "not handled" so app can handle
        case BLE_EVT_GATTS_READ_REQ:
                return read_req((const ble_evt_gatts_read_req_t *) evt);
        case BLE_EVT_GATTS_WRITE_REQ:
                return write_req((const ble_evt_gatts_write_req_t *) evt);
        case BLE_EVT_GATTS_PREPARE_WRITE_REQ:
                return prepare_write_req((const ble_evt_gatts_prepare_write_req_t *) evt);
        case BLE_EVT_GATTS_EVENT_SENT:
                return event_sent((const ble_evt_gatts_event_sent_t *) evt);
        }

        return false;
}

static gap_sec_level_t get_sec_level_read_perm(att_perm_t perm)
{
        gap_sec_level_t sec_level = GAP_SEC_LEVEL_1;

        if (perm & ATT_PERM_READ_ENCRYPT) {
                sec_level = GAP_SEC_LEVEL_2;
        }

        if (perm & ATT_PERM_READ_AUTH) {
                sec_level = GAP_SEC_LEVEL_3;
        }

        return sec_level;
}

static gap_sec_level_t get_sec_level_write_perm(att_perm_t perm)
{
        gap_sec_level_t sec_level = GAP_SEC_LEVEL_1;

        if (perm & ATT_PERM_WRITE_ENCRYPT) {
                sec_level = GAP_SEC_LEVEL_2;
        }

        if (perm & ATT_PERM_WRITE_AUTH) {
                sec_level = GAP_SEC_LEVEL_3;
        }

        return sec_level;
}

att_perm_t ble_service_config_elevate_perm(att_perm_t perm, const ble_service_config_t *config)
{
        att_perm_t new_perm = ATT_PERM_NONE;
        gap_sec_level_t sec_level;

        if (!config || config->sec_level == GAP_SEC_LEVEL_1) {
                return perm;
        }

        sec_level = config->sec_level;

        if (perm & PERM_READ_MASK) {
                if (get_sec_level_read_perm(perm) < sec_level) {
                        switch (sec_level) {
                        case GAP_SEC_LEVEL_2:
                                new_perm |= ATT_PERM_READ_ENCRYPT;
                                break;
                        case GAP_SEC_LEVEL_3:
                        case GAP_SEC_LEVEL_4:
                                new_perm |= ATT_PERM_READ_AUTH;
                                break;
                        default:
                                break;
                        }
                } else {
                        new_perm |= perm & PERM_READ_MASK;
                }
        }

        if (perm & PERM_WRITE_MASK) {
                if (get_sec_level_write_perm(perm) < sec_level) {
                        switch (sec_level) {
                        case GAP_SEC_LEVEL_2:
                                new_perm |= ATT_PERM_WRITE_ENCRYPT;
                                break;
                        case GAP_SEC_LEVEL_3:
                        case GAP_SEC_LEVEL_4:
                                new_perm |= ATT_PERM_WRITE_AUTH;
                                break;
                        default:
                                break;
                        }
                } else {
                        new_perm |= perm & PERM_WRITE_MASK;
                }
        }

        new_perm |= perm & ATT_PERM_KEYSIZE_16;

        return new_perm;
}

void ble_service_config_add_includes(const ble_service_config_t *config)
{
        uint8_t i;

        if (!config) {
                return;
        }

        for (i = 0; i < config->num_includes; i++) {
                ble_gatts_add_include(config->includes[i]->start_h, NULL);
        }
}
