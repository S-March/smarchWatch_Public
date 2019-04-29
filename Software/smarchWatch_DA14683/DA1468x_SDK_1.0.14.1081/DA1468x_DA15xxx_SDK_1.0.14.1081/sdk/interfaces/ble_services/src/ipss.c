/**
 ****************************************************************************************
 *
 * @file ipss.c
 *
 * @brief IP Support Service implementation
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <string.h>
#include "osal.h"
#include "ble_uuid.h"
#include "ipss.h"

static void ipss_cleanup(ble_service_t *svc)
{
        OS_FREE(svc);
}

ble_service_t *ipss_init(void)
{
        att_uuid_t uuid;
        ble_service_t *ipss;

        ipss = OS_MALLOC(sizeof(*ipss));
        memset(ipss, 0, sizeof(*ipss));

        ble_uuid_create16(UUID_SERVICE_IPSS, &uuid);
        ble_gatts_add_service(&uuid, GATT_SERVICE_PRIMARY, 0);

        ble_gatts_register_service(&ipss->start_h, 0);

        ipss->end_h = ipss->start_h;
        ipss->cleanup = ipss_cleanup;

        return ipss;
}
