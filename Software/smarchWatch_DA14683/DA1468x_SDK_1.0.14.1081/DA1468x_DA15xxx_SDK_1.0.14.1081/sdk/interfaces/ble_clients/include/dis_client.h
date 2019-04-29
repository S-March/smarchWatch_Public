/**
 ****************************************************************************************
 *
 * @file dis_client.h
 *
 * @brief Device Information Service Client header file
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef DIS_CLIENT_H
#define DIS_CLIENT_H

/**
 * Capabilities (supported characteristics)
 */
typedef enum {
        /** Manufacturer Name String Characteristic */
        DIS_CLIENT_CAP_MANUFACTURER_NAME = 0x0001,
        /** Model Number String Characteristic */
        DIS_CLIENT_CAP_MODEL_NUMBER = 0x0002,
        /** Serial Number String Characteristic */
        DIS_CLIENT_CAP_SERIAL_NUMBER = 0x0004,
        /** Hardware Revision String Characteristic */
        DIS_CLIENT_CAP_HARDWARE_REVISION = 0x0008,
        /** Firmware Revision String Characteristic */
        DIS_CLIENT_CAP_FIRMWARE_REVISION = 0x0010,
        /** Software Revision String Characteristic */
        DIS_CLIENT_CAP_SOFTWARE_REVISION = 0x0020,
        /** System Id Characteristic */
        DIS_CLIENT_CAP_SYSTEM_ID = 0x0040,
        /** IEEE 11073-20601 Regulatory Certification Data List Characteristic */
        DIS_CLIENT_CAP_REG_CERT = 0x0080,
        /** PNP ID Characteristic */
        DIS_CLIENT_CAP_PNP_ID = 0x0100,
} dis_client_cap_t;

/**
 * Device Information Service System ID
 */
typedef struct __attribute__ ((packed)) {
        uint8_t manufacturer[5];
        uint8_t oui[3];
} dis_client_system_id_t;

/**
 * Device Information Service PNP ID
 */
typedef struct __attribute__ ((packed)) {
        uint8_t   vid_source;
        uint16_t  vid;
        uint16_t  pid;
        uint16_t  version;
} dis_client_pnp_id_t;

typedef void (* dis_client_read_completed_cb_t) (ble_client_t *dis_client, att_error_t status,
                                                        dis_client_cap_t capability,
                                                        uint16_t length, const uint8_t *value);

/**
 * DIS Client application callbacks
 */
typedef struct {
        /** DIS Client read completed callback */
        dis_client_read_completed_cb_t  read_completed;
} dis_client_callbacks_t;

/**
 * \brief Register DIS Client instance
 *
 * Function registers DIS Client
 *
 * \param [in] cb       application callbacks
 * \param [in] evt      browse svc event with Device Information Service details
 *
 * \return Client instance
 *
 */
ble_client_t *dis_client_init(const dis_client_callbacks_t *cb,
                                                        const ble_evt_gattc_browse_svc_t *evt);
/**
 * \brief Get DIS capabilities
 *
 * Functions returns bit mask with capabilities (supported characteristics).
 *
 * \param [in] dis_client       client instance
 *
 * \return Bit mask with capabilities
 */
dis_client_cap_t dis_client_get_capabilities(ble_client_t *dis_client);

/**
 * \brief Read DIS capability (supported characteristic)
 *
 * Function reads capability (supported characteristic).
 *
 * \param [in] dis_client       client instance
 * \param [in] capability       capability (supported characteristic type)
 *
 * \return True if read request has been sent successfully, false otherwise
 *
 */
bool dis_client_read(ble_client_t *dis_client, dis_client_cap_t capability);

/**
 * \brief Initialize and register DIS Client instance from data buffer
 *
 * Function sematics is very similar to dis_client_init() but internal data is initialized by
 * buffered context and the client is automatically added to active clients collection.
 *
 * \param [in] conn_idx         connection index
 * \param [in] cb               application callbacks
 * \param [in] data             buffered context data
 * \param [in] length           data buffer's length
 *
 * \return client instance when initialized properly, NULL otherwise
 */
ble_client_t *dis_client_init_from_data(uint16_t conn_idx, const dis_client_callbacks_t *cb,
                                                                   const void *data, size_t length);

#endif /* DIS_CLIENT_H */
