/**
 ****************************************************************************************
 *
 * @file hids_client.h
 *
 * @brief HID Service Client header file
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HIDS_CLIENT_H
#define HIDS_CLIENT_H

#include <string.h>
#include "ble_gattc.h"
#include "ble_client.h"

/**
 * \enum hids_client_protocol_mode_t
 *
 * \brief HID Service Client protocol mode
 */
typedef enum {
        HIDS_CLIENT_PROTOCOL_MODE_BOOT = 0x00,          /**< Host Mode Boot */
        HIDS_CLIENT_PROTOCOL_MODE_REPORT = 0x01,        /**< Host Mode Report */
} hids_client_protocol_mode_t;

/**
 * \enum hids_client_report_type_t
 *
 * \brief HID Service Client report type
 */
typedef enum {
        HIDS_CLIENT_REPORT_TYPE_INPUT = 0x01,           /**< Report Type Input */
        HIDS_CLIENT_REPORT_TYPE_OUTPUT = 0x02,          /**< Report Type Output */
        HIDS_CLIENT_REPORT_TYPE_FEATURE = 0x03,         /**< Report Type Feature */
} hids_client_report_type_t;

/**
 * \enum hids_client_boot_report_type
 *
 * \brief HID Service Client boot report type
 */
typedef enum {
        HIDS_CLIENT_BOOT_MOUSE_INPUT,                   /**< Boot Mouse Input */
        HIDS_CLIENT_BOOT_KEYBOARD_INPUT,                /**< Boot Keyboard Input */
        HIDS_CLIENT_BOOT_KEYBOARD_OUTPUT,               /**< Boot Keyboard Output */
} hids_client_boot_report_type;

/**
 * \enum hids_client_cp_command_t
 *
 * \brief HID Service Client control point command
 */
typedef enum {
        HIDS_CLIENT_CONTROL_POINT_SUSPEND = 0x00,       /**< Control Point Suspend */
        HIDS_CLIENT_CONTROL_POINT_EXIT_SUSPEND = 0x01,  /**< Control Point Exit Suspend */
} hids_client_cp_command_t;

/**
 * \enum hids_client_cap_t
 *
 * \brief HIDS Client capabilities
 */
typedef enum {
        /** Protocol Mode characteristic */
        HIDS_CLIENT_CAP_PROTOCOL_MODE = 0x01,
        /** Boot Mouse Input characteristic */
        HIDS_CLIENT_CAP_BOOT_MOUSE_INPUT = 0x02,
        /** Boot Keyboard Input characteristic */
        HIDS_CLIENT_CAP_BOOT_KEYBOARD_INPUT = 0x04,
        /** Boot Keyboard Output characteristic */
        HIDS_CLIENT_CAP_BOOT_KEYBOARD_OUTPUT = 0x08,
        /** HID Info characteristic */
        HIDS_CLIENT_CAP_HID_INFO = 0x10,
        /** HID Control Point characteristic */
        HIDS_CLIENT_CAP_HID_CONTROL_POINT = 0x20,
        /** Report Map characteristic */
        HIDS_CLIENT_CAP_REPORT_MAP = 0x40,
} hids_client_cap_t;

/**
 * \struct hids_client_hid_info_t
 *
 * \brief HID Service Client info data
 */
typedef struct {
        uint16_t bcd_hid;
        uint8_t  country_code;
        uint8_t  flags;
} hids_client_hid_info_t;

typedef struct {
        hids_client_protocol_mode_t mode;
} hids_client_config_t;

typedef void (* hids_client_boot_report_cb_t) (ble_client_t *hids_client,
                                                        hids_client_boot_report_type type,
                                                        att_error_t status, uint16_t length,
                                                        const uint8_t *data);
typedef void (* hids_client_report_cb_t) (ble_client_t *hids_client, hids_client_report_type_t type,
                                                        uint8_t report_id, att_error_t status,
                                                        uint16_t length, const uint8_t *data);
typedef void (* hids_client_read_report_map_cb_t) (ble_client_t *hids_client, att_error_t status,
                                                        uint16_t length, const uint8_t *data);
typedef void (* hids_client_read_hid_info_cb_t) (ble_client_t *hids_client,
                                                        const hids_client_hid_info_t *info);
typedef void (* hids_client_input_report_get_notif_state_cb_t) (ble_client_t *hids_client,
                                                                uint8_t report_id,
                                                                att_error_t status, bool enabled);
typedef void (* hids_client_input_report_set_notif_state_cb_t) (ble_client_t *hids_client,
                                                                        uint8_t report_id,
                                                                        att_error_t status);
typedef void (* hids_client_boot_report_get_notif_state_cb_t) (ble_client_t *hids_client,
                                                                hids_client_boot_report_type type,
                                                                att_error_t status, bool enabled);
typedef void (* hids_client_boot_report_set_notif_state_cb_t) (ble_client_t *hids_client,
                                                                hids_client_boot_report_type type,
                                                                att_error_t status);
typedef void (* hids_client_get_protocol_mode_cb_t) (ble_client_t *hids_client, att_error_t status,
                                                                hids_client_protocol_mode_t mode);
typedef void (* hids_client_set_protocol_mode_cb_t) (ble_client_t *hids_client,
                                                                        att_error_t status);
typedef void (* hids_client_external_report_found_cb_t) (ble_client_t *hids_client,
                                                att_error_t status, const att_uuid_t *uuid);
typedef void (* hids_client_discover_external_reports_completed_cb_t) (ble_client_t *hids_client);
typedef void (* hids_client_report_found_cb_t) (ble_client_t *hids_client, att_error_t status,
                                                hids_client_report_type_t type, uint8_t report_id);
typedef void (* hids_client_discover_reports_completed_cb_t) (ble_client_t *hids_client);
typedef void (* hids_client_report_write_completed_cb_t) (ble_client_t *hids_client,
                                                                hids_client_report_type_t type,
                                                                uint8_t report_id,
                                                                att_error_t status);
typedef void (* hids_client_dump_service_data_cb_t) (ble_client_t *hids_client,
                                                                        const char *fmt, ...);

typedef struct {
        /** Boot report callback triggered by notification or read response */
        hids_client_boot_report_cb_t                            boot_report;
        /** Report callback triggered by notification or read response */
        hids_client_report_cb_t                                 report;
        /** Report map callback. Will be triggered only in ::HIDS_CLIENT_PROTOCOL_MODE_REPORT */
        hids_client_read_report_map_cb_t                        report_map;
        /** HID Info callback. Will be triggered only in ::HIDS_CLIENT_PROTOCOL_MODE_REPORT */
        hids_client_read_hid_info_cb_t                          hid_info;
        /** Callback indicates if notifications are enabled or disabled */
        hids_client_input_report_get_notif_state_cb_t           input_report_get_notif_state;
        /** Input report set notif state completed callback */
        hids_client_input_report_set_notif_state_cb_t           input_report_set_notif_state;
        /** Callback indicates if boot report notifications are enabled or disabled */
        hids_client_boot_report_get_notif_state_cb_t            boot_report_get_notif_state;
        /** Boot report set notif state completed callback */
        hids_client_boot_report_set_notif_state_cb_t            boot_report_set_notif_state;
        /** Get protocol mode callback */
        hids_client_get_protocol_mode_cb_t                      get_protocol_mode;
        /** Set Protocol mode callback */
        hids_client_set_protocol_mode_cb_t                      set_protocol_mode;
        /** External report found callback */
        hids_client_external_report_found_cb_t                  external_report_found;
        /** External reports discovered complete callback */
        hids_client_discover_external_reports_completed_cb_t    discover_external_reports_complete;
        /** Report found callback */
        hids_client_report_found_cb_t                           report_found;
        /** Reports discovered completed callback */
        hids_client_discover_reports_completed_cb_t             discover_reports_complete;
        /** Write report completed */
        hids_client_report_write_completed_cb_t                 report_write_complete;
} hids_client_callbacks_t;

/**
 * \brief Register HID Client instance
 *
 * Function registers HID Client
 *
 * \param [in] config   HIDS client config
 * \param [in] cb       application callbacks
 * \param [in] evt      browse svc event with HID svc details
 *
 * \return client instance
 *
 */
ble_client_t *hids_client_init(const hids_client_config_t *config, const hids_client_callbacks_t *cb,
                                                        const ble_evt_gattc_browse_svc_t *evt);

/**
 * \brief Initialize and register HID Client instance from data buffer
 *
 * Function initializes HID Client from data buffer.
 *
 * \param [in] conn_idx connection index
 * \param [in] config   HIDS client config
 * \param [in] cb       application callbacks
 * \param [in] data     data buffer
 * \param [in] length   data buffer's length
 *
 * \return client instance when initialized properly, NULL otherwise
 */
ble_client_t *hids_client_init_from_data(uint16_t conn_idx, const hids_client_config_t *config,
                                const hids_client_callbacks_t *cb,const void *data, size_t length);

/**
 * \brief Set protocol mode
 *
 * Function triggers write command to protocol mode characteristic with protocol mode defined in
 * hids_config structure. Callback set_protocol_mode will be called once operation completed.
 *
 * \param [in] client           client
 *
 * \return true if write command sent, false otherwise
 *
 */
bool hids_client_set_protocol_mode(ble_client_t *client);

/**
 * \brief Read Boot Report
 *
 * Function triggers read operation of Boot Report Characteristic. Read response will be returned
 * in boot_report callback.
 *
 * \param [in] client           client
 * \param [in] type             Type of boot report
 *
 * \return true if read request sent, otherwise false
 *
 * \note
 * This should be called when mode is == ::HIDS_CLIENT_PROTOCOL_MODE_BOOT
 */
bool hids_client_boot_report_read(ble_client_t *client, hids_client_boot_report_type type);

/**
 * \brief Write Boot Report
 *
 * Function triggers write operation of Boot Report Characteristic.
 *
 * \param [in] client           client
 * \param [in] type             type of boot report
 * \param [in] response         require response - valid only for ::HIDS_CLIENT_BOOT_KEYBOARD_OUTPUT
 *                              argument will be ignored in other cases
 * \param [in] length           report length
 * \param [in] data             report data
 *
 * \return true if write request sent, otherwise false
 *
 * \note
 * This should be called only in ::HIDS_CLIENT_PROTOCOL_MODE_BOOT
 */
bool hids_client_boot_report_write(ble_client_t *client, hids_client_boot_report_type type,
                                        bool response, uint16_t length, const uint8_t *data);

/**
 * \brief Write Report
 *
 * Function triggers write operation of Report Characteristic.
 *
 * \param [in] client           client
 * \param [in] type             type of boot report
 * \param [in] report_id        report ID
 * \param [in] response         require response - valid only for ::HIDS_CLIENT_REPORT_TYPE_OUTPUT
 *                              argument will be ignored in other cases
 * \param [in] length           report length
 * \param [in] data             report data
 *
 * \return true if write request sent, otherwise false
 *
 * \note
 * This should be called only in ::HIDS_CLIENT_PROTOCOL_MODE_REPORT
 */
bool hids_client_report_write(ble_client_t *client, hids_client_report_type_t type,
                                                uint8_t report_id, bool response, uint16_t length,
                                                const uint8_t *data);

/**
 * \brief Read Report
 *
 * Function triggers read operation of Report Characteristic. Read response will be returned
 * in report callback.
 *
 * \param [in] client           client
 * \param [in] type             type of report
 * \param [in] report_id        report ID
 *
 * \return true if read request sent, otherwise false
 *
 * \note
 * This should be called only in ::HIDS_CLIENT_PROTOCOL_MODE_REPORT
 */
bool hids_client_report_read(ble_client_t *client, hids_client_report_type_t type,
                                                                        uint8_t report_id);

/**
 * \brief Write command to control point
 *
 * Function triggers write operation to Control Point characteristic.
 *
 * \param [in] client           client
 * \param [in] command          control point command
 *
 * \return true if write command sent, otherwise false
 *
 * \note
 * This should be called only in ::HIDS_CLIENT_PROTOCOL_MODE_REPORT
 */
bool hids_client_cp_command(ble_client_t *client, hids_client_cp_command_t command);

/**
 * \brief Enable/disable notifications for Input Report
 *
 * Function triggers write request to CCC descriptor of given report and registers or unregisters
 * for notifications.
 *
 * \param [in] client           hids_client instance
 * \param [in] report_id        input report ID
 * \param [in] enable           indicates if notifications shall be enabled/disabled
 *
 * \return true if write request has been sent correctly, otherwise false
 *
 * \note
 * This should be called only in ::HIDS_CLIENT_PROTOCOL_MODE_REPORT
 */
bool hids_client_input_report_set_notif_state(ble_client_t *client, uint8_t report_id,
                                                                                bool enable);

/**
 * \brief Check if notifications are enabled for Input Report
 *
 * Function triggers read request to CCC descriptor of given report.
 *
 * \param [in] client           hids_client instance
 * \param [in] report_id        input report ID
 *
 * \return true if read request has been sent correctly, otherwise false
 *
 * \note
 * This shoud be called only in ::HIDS_CLIENT_PROTOCOL_MODE_REPORT.
 * This call will trigger input_report_is_enabled callback.
 */
bool hids_client_input_report_get_notif_state(ble_client_t *client, uint8_t report_id);

/**
 * \brief Enable/disable notifications for Boot Input reports
 *
 * Function triggers write request to CCC descriptor of given report and registers or unregisters
 * for notifications.
 *
 * \param [in] client           hids_client instance
 * \param [in] type             boot report type
 * \param [in] enable           indicates if notifications shall be enabled/disabled
 *
 * \return true if write request has been sent correctly, otherwise false
 *
 * \note
 * This should be called only in ::HIDS_CLIENT_PROTOCOL_MODE_BOOT and type
 * ::HIDS_CLIENT_BOOT_MOUSE_INPUT or ::HIDS_CLIENT_BOOT_KEYBOARD_INPUT
 */
bool hids_client_boot_report_set_notif_state(ble_client_t *client,
                                                hids_client_boot_report_type type, bool enable);

/**
 * \brief Check if notifications are enabled for Boot Input Report
 *
 * Function triggers read request to CCC descriptor of given report.
 *
 * \param [in] client           hids_client instance
 * \param [in] type             boot report type
 *
 * \return true if read request has been sent correctly, otherwise false
 *
 * \note
 * This should be called only in ::HIDS_CLIENT_PROTOCOL_MODE_BOOT and type
 * ::HIDS_CLIENT_BOOT_MOUSE_INPUT or ::HIDS_CLIENT_BOOT_KEYBOARD_INPUT
 */
bool hids_client_boot_report_get_notif_state(ble_client_t *client,
                                                                hids_client_boot_report_type type);

/**
 * \brief Get protocol mode
 *
 * Function triggers read request to protocol mode characteristic. Callback get_protocol_mode
 * will be invoked once finished.
 *
 * \param [in] client           hids_client instance
 *
 * \return true if read request has been sent correctly, otherwise false
 */
bool hids_client_get_protocol_mode(ble_client_t *client);

/**
 * \brief Get HID Info characteristic
 *
 * Function triggers read request to HID Info characteristic. Callback hid_info_cb will be
 * invoked once finished. It should be called after init in ::HIDS_CLIENT_PROTOCOL_MODE_REPORT
 * mode.
 *
 * \param [in] client           hids_client instance
 *
 * \return true if read request has been sent correctly, otherwise false
 */
bool hids_client_read_hid_info(ble_client_t *client);

/**
 * \brief Read Report Map characteristic
 *
 * Function triggers read request to Report Map characteristic. Callback report_map_cb will be
 * invoked once finished. It should be called in ::HIDS_CLIENT_PROTOCOL_MODE_REPORT after init.
 *
 * \param [in] client           hids_client instance
 *
 * \return true if read request has been sent correctly, otherwise false
 */
bool hids_client_read_report_map(ble_client_t *client);

/**
 * \brief Read External Reports descriptors
 *
 * Function triggers read request to External Report Reference Descriptors.
 * If service supports External Report References, external_report_found will be
 * invoked. Once all read requests are completed, discover_external_reports_complete callback is
 * called. This function should ba called right after init in ::HIDS_CLIENT_PROTOCOL_MODE_REPORT
 * mode.
 *
 * \param [in] client           hids_client instance
 *
 * \return true if read requests have been sent correctly, otherwise false
 */
bool hids_client_discover_external_reports(ble_client_t *client);

/**
 * \brief Read Report Reference descriptors
 *
 * Function triggers read requests to Report Reference descriptors.
 * Callback report_found will be called for each read_response. Once all read requests are
 * completed, discover_reports_completed callback is called.
 * It should be called in ::HIDS_CLIENT_PROTOCOL_MODE_REPORT right after init.
 *
 * \param [in] client           hids_client instance
 *
 * \return true if read requests have been sent correctly, otherwise false
 */
bool hids_client_discover_reports(ble_client_t *client);

/**
 * \brief Get capabilities
 *
 * Function returns bitmask with supported characteristics. Note that Report characteristics
 * won't be returned in this function as there might be multiple reports supported.
 *
 * \param client           hids_client instance
 *
 * \return supported characteristics bitmask
 */
hids_client_cap_t hids_client_get_capabilities(ble_client_t *client);

/**
 * \brief Dump HID Service data
 *
 * This function is used to print data about remote service details.
 *
 * \param [in] client           hids_client instance
 * \param [in] cb               dump service data callback
 */
void hids_client_dump_dervice_data(ble_client_t *client, hids_client_dump_service_data_cb_t cb);

#endif /* HIDS_CLIENT_H */
