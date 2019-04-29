/**
 ****************************************************************************************
 *
 * @file ble_client.c
 *
 * @brief GATT Client handling routines
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stddef.h>
#include "ble_client.h"
#include "osal.h"

#if CONFIG_BLE_CLIENTS_MAX_NUM > 0
#       define MAX_CLIENTS (CONFIG_BLE_CLIENTS_MAX_NUM)
#else
#       define MAX_CLIENTS (10)
#endif

PRIVILEGED_DATA static ble_client_t *clients[MAX_CLIENTS];

void ble_client_add(ble_client_t *client)
{
        int i;

        for (i = 0; i < MAX_CLIENTS; i++) {
                if (!clients[i]) {
                        clients[i] = client;
                        break;
                }
        }
}

void ble_client_remove(ble_client_t *client)
{
        int i;

        for (i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] == client) {
                        clients[i] = NULL;
                        break;
                }
        }
}

void ble_client_cleanup(ble_client_t *client)
{
        if (!client || !client->cleanup) {
                return;
        }

        client->cleanup(client);
}

void ble_client_serialize(ble_client_t *client, void *data, size_t *length)
{
        if (!client || !client->serialize) {
                *length = 0;
                return;
        }

        client->serialize(client, data, length);
}

void ble_client_attach(ble_client_t *client, uint16_t conn_idx)
{
        if (!client) {
                return;
        }

        client->conn_idx = conn_idx;

        ble_client_add(client);

        if (client->attach) {
                client->attach(client);
        }
}

static void handle_gattc_read_completed(const ble_evt_gattc_read_completed_t *evt)
{
        int i;

        for (i = 0; i < MAX_CLIENTS; i++) {
                ble_client_t *client = clients[i];

                if (!client) {
                        continue;
                }

                if (evt->conn_idx != client->conn_idx) {
                        continue;
                }

                if (client->read_completed_evt) {
                        client->read_completed_evt(client, evt);
                }
        }
}

static void handle_gattc_write_completed(const ble_evt_gattc_write_completed_t *evt)
{
        int i;

        for (i = 0; i < MAX_CLIENTS; i++) {
                ble_client_t *client = clients[i];

                if (!client) {
                        continue;
                }

                if (evt->conn_idx != client->conn_idx) {
                        continue;
                }

                if (client->write_completed_evt) {
                        client->write_completed_evt(client, evt);
                }
        }
}

static void handle_gattc_notification(const ble_evt_gattc_notification_t *evt)
{
        int i;

        for (i = 0; i < MAX_CLIENTS; i++) {
                ble_client_t *client = clients[i];

                if (!client) {
                        continue;
                }

                if (evt->conn_idx != client->conn_idx) {
                        continue;
                }

                if (client->notification_evt) {
                        client->notification_evt(client, evt);
                }
        }
}

static void handle_gattc_indication(const ble_evt_gattc_indication_t *evt)
{
        int i;

        for (i = 0; i < MAX_CLIENTS; i++) {
                ble_client_t *client = clients[i];

                if (!client) {
                        continue;
                }

                if (evt->conn_idx != client->conn_idx) {
                        continue;
                }

                if (client->indication_evt) {
                        client->indication_evt(client, evt);
                }
        }
}

static void handle_gap_disconnected(const ble_evt_gap_disconnected_t *evt)
{
        int i;

        for (i = 0; i < MAX_CLIENTS; i++) {
                ble_client_t *client = clients[i];

                if (!client) {
                        continue;
                }

                if (evt->conn_idx != client->conn_idx) {
                        continue;
                }

                if (client->disconnected_evt) {
                        client->disconnected_evt(client, evt);
                }
        }
}

void ble_client_handle_event(const ble_evt_hdr_t *evt)
{
        switch (evt->evt_code) {
        case BLE_EVT_GATTC_READ_COMPLETED:
                handle_gattc_read_completed((const ble_evt_gattc_read_completed_t *) evt);
                break;
        case BLE_EVT_GATTC_WRITE_COMPLETED:
                handle_gattc_write_completed((const ble_evt_gattc_write_completed_t *) evt);
                break;
        case BLE_EVT_GATTC_NOTIFICATION:
                handle_gattc_notification((const ble_evt_gattc_notification_t *) evt);
                break;
        case BLE_EVT_GATTC_INDICATION:
                handle_gattc_indication((const ble_evt_gattc_indication_t *) evt);
                break;
        case BLE_EVT_GAP_DISCONNECTED:
                handle_gap_disconnected((const ble_evt_gap_disconnected_t *) evt);
                break;
        }
}
