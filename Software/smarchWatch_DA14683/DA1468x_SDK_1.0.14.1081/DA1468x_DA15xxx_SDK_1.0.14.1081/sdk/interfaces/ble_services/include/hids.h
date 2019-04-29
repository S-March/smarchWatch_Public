/**
 ****************************************************************************************
 *
 * @file hids.h
 *
 * @brief HID Service implementation
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HIDS_H_
#define HIDS_H_

#include "ble_gap.h"
#include "ble_service.h"

/**
 * HID Service protocol modes
 */
typedef enum {
        HIDS_PROTOCOL_MODE_BOOT = 0x00,         /**< Protocol Mode Boot */
        HIDS_PROTOCOL_MODE_REPORT = 0x01,       /**< Protocol Mode Report */
} hids_protocol_mode_t;

/**
 * HID Service control point values
 */
typedef enum {
        HIDS_CONTROL_POINT_SUSPEND = 0x00,      /**< Control Point Suspend */
        HIDS_CONTROL_POINT_EXIT_SUSPEND = 0x01, /**< Control Point Exit Suspend */
} hids_cp_command_t;

/**
 * HID Service report types
 */
typedef enum {
        HIDS_REPORT_TYPE_INPUT = 0x01,          /**< Report Type Input */
        HIDS_REPORT_TYPE_OUTPUT = 0x02,         /**< Report Type Output */
        HIDS_REPORT_TYPE_FEATURE = 0x03,        /**< Report Type Feature */
} hids_report_type_t;

typedef void (* hids_set_protocol_mode_cb_t) (ble_service_t *svc, hids_protocol_mode_t mode);
typedef void (* hids_control_point_cb_t) (ble_service_t *svc, hids_cp_command_t command);
typedef void (* hids_boot_keyboard_output_write_cb_t) (ble_service_t *svc, uint16_t length,
                                                                        const uint8_t *data);
typedef void (* hids_report_write_cb_t) (ble_service_t *svc, hids_report_type_t report_type,
                                        uint8_t report_id, uint16_t length, const uint8_t *data);
typedef void (* hids_report_sent_t) (ble_service_t *svc);
typedef void (* hids_notify_boot_mouse_input_report_completed_cb_t) (ble_service_t *svc,
                                                                                bool success);
typedef void (* hids_notify_boot_keyboard_input_report_completed_cb_t) (ble_service_t *svc,
                                                                                bool success);
typedef void (* hids_notify_input_report_completed_cb_t) (ble_service_t *svc, uint8_t report_id,
                                                                                bool success);


/**
 * HIDS application callbacks
 */
typedef struct {
        /** Indicates that client changed protocol mode */
        hids_set_protocol_mode_cb_t set_protocol_mode;
        /** Indicates that client changed control point value */
        hids_control_point_cb_t control_point;
        /** Indicates data has been written to Keyboard Output characteristic */
        hids_boot_keyboard_output_write_cb_t boot_keyboard_write;
        /** Indicates data has been written to Report characteristic */
        hids_report_write_cb_t report_write;
        /* Indicates that report notification has been sent */
        hids_report_sent_t report_sent;
        /** Boot mouse input report notification sent callback */
        hids_notify_boot_mouse_input_report_completed_cb_t
                                                        notify_boot_mouse_input_report_completed;
        /** Boot keyboard input report notification sent callback */
        hids_notify_boot_keyboard_input_report_completed_cb_t
                                                notify_boot_keyboard_input_report_completed;
        /** Input report notification sent callback */
        hids_notify_input_report_completed_cb_t notify_input_report_completed;
} hids_callbacks_t;

/**
 * HID service report
 */
typedef struct {
        hids_report_type_t type;        /**< Report type */
        uint8_t report_id;              /**< Report ID */
        uint16_t length;                /**< Report value length */
} hids_report_t;

typedef enum {
        HIDS_INFO_FLAG_REMOTE_WAKE = 0x01,
        HID_INFO_FLAG_NORMALLY_CONNECTABLE = 0x02,
} hids_info_flag_t;

typedef struct {
        uint16_t bcd_hid;
        uint8_t country_code;
        uint8_t flags;
} hids_hid_info_t;

/**
 * HID Service boot device flags
 */
typedef enum {
        /** Boot Device Keyboard */
        HIDS_BOOT_DEVICE_KEYBOARD = 0x01,
        /** Boot Device Mouse */
        HIDS_BOOT_DEVICE_MOUSE = 0x02,
        /** Boot Device Mouse and Boot Device Keyboard */
        HIDS_BOOT_DEVICE_COMBO = HIDS_BOOT_DEVICE_KEYBOARD | HIDS_BOOT_DEVICE_MOUSE,
} hids_boot_device_t;

/**
 * HID Service config used during initialization of service
 */
typedef struct {
        /** Number of reports in HID service */
        uint8_t num_reports;
        /** Reports array */
        const hids_report_t *reports;
        /** Application defined report map */
        const uint8_t *report_map;
        /** Report map length */
        uint16_t report_map_length;
        /** HID service info */
        hids_hid_info_t hids_info;
        /** Boot characteristics setup */
        hids_boot_device_t boot_device;
} hids_config_t;

/**
 * \brief Register HID Service instance
 *
 * Function registers HID Service
 *
 * \note
 * HID Service can handle only one connected host at any time. For this reason application has to
 * attach particular connection to HIDS before it can use HIDS - see hids_attach_connection().
 * It's up to application on how connection is selected but generally it's recommended that only
 * one connection with other device is allowed when running HIDS.
 *
 * \param [in] service_config           general service config
 * \param [in] config                   HID Service specific config
 * \param [in] callbacks                application callbacks
 *
 * \return service instance
 *
 * \sa hids_attach_connection
 *
 */
ble_service_t *hids_init(const ble_service_config_t *service_config, const hids_config_t *config,
                                                                const hids_callbacks_t *callbacks);

/**
 * \brief Attach connection to HID Service instance
 *
 * Function attaches connection to be used by HID Service instance. Only one connection can have
 * access to HIDS instance. Connection is automatically detached upon disconnection.
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 *
 * \return true if new connection is attached, false if there is already other connection attached
 *
 */
bool hids_attach_connection(ble_service_t *svc, uint16_t conn_idx);

/**
 * \brief Set Boot Mouse Input value
 *
 * Function sets value of Boot Mouse Input characteristic and notifies clients
 * if protocol is set to HIDS_PROTOCOL_MODE_BOOT
 *
 * \param [in] svc              service instance
 * \param [in] length           length of data
 * \param [in] data             new value
 *
 * \return true if value has been set successfully and notification sent, otherwise false.
 *
 * \deprecated This function is deprecated. User shall call hids_set_boot_mouse_input_value() and
 * hids_set_boot_mouse_input_report instead.
 */
bool hids_set_boot_mouse_input_value(ble_service_t *svc, uint16_t length, const uint8_t *data)
                                                                __attribute__ ((deprecated));

/**
 * \brief Set Boot Mouse Input Report
 *
 * Function sets value of Boot Mouse Input characteristic.
 *
 * \param [in] svc              service instance
 * \param [in] length           length of data
 * \param [in] data             new value
 *
 * \return true if value has been set successfully, otherwise false.
 *
 */
bool hids_set_boot_mouse_input_report(ble_service_t *svc, uint16_t length, const uint8_t *data);

/**
 * \brief Notify Boot Mouse Input Report
 *
 * Function sends Boot Mouse Input characteristic notification.
 *
 * \param [in] svc              service instance
 * \param [in] length           length of data
 * \param [in] data             new value
 *
 * \return false if there is no attached connection or if protocol mode is not
 * ::HIDS_PROTOCOL_MODE_BOOT or if notifications are not enabledd by remote device.
 * True if notification has been sent successfully.
 *
 */
bool hids_notify_boot_mouse_input_report(ble_service_t *svc, uint16_t length, const uint8_t *data);

/**
 * \brief Set Boot Keyboard Input value
 *
 * Function sets value of Boot Keyboard Input characteristic and notifies clients
 * if protocol is set to HIDS_PROTOCOL_MODE_BOOT
 *
 * \param [in] svc              service instance
 * \param [in] length           length of data
 * \param [in] data             new value
 *
 * \return true if value has been set successfully and notification sent, otherwise false.
 *
 * * \deprecated This function is deprecated. User shall call hids_set_boot_keyboard_input_report()
 * and hids_notify_boot_keyboard_input_report() instead.
 */
bool hids_set_boot_keyboard_input_value(ble_service_t *svc, uint16_t length, const uint8_t *data)
                                                                __attribute__ ((deprecated));

/**
 * \brief Set Boot Keyboard Input Report
 *
 * Function sets value of Boot Keyboard Input characteristic.
 *
 * \param [in] svc              service instance
 * \param [in] length           length of data
 * \param [in] data             new value
 *
 * \return true if value has been set successfully, otherwise false.
 */
bool hids_set_boot_keyboard_input_report(ble_service_t *svc, uint16_t length, const uint8_t *data);

/**
 * \brief Notify Boot Keyboard Input Report
 *
 * Function sets value of Boot Keyboard Input characteristic and notifies clients
 * if protocol is set to HIDS_PROTOCOL_MODE_BOOT
 *
 * \param [in] svc              service instance
 * \param [in] length           length of data
 * \param [in] data             new value
 *
 * \return false if there is no attached connection or if protocol mode is not
 * ::HIDS_PROTOCOL_MODE_BOOT or if notifications are not enabled by remote device.
 * True if notification has been sent successfully.
 */
bool hids_notify_boot_keyboard_input_report(ble_service_t *svc, uint16_t length,
                                                                        const uint8_t *data);

/**
 * \brief Set Report value
 *
 * Function sets value of Report Characteristic and optionally notifies client if
 * HIDS_PROTOCOL_MODE_REPORT is enabled and if report's type is HIDS_REPORT_TYPE_INPUT.
 *
 * \param [in] svc              service instance
 * \param [in] type             report type
 * \param [in] report_id        report ID
 * \param [in] length           length of data
 * \param [in] data             new value
 *
 * \return true if value has been set successfully and notification sent, otherwise false.
 *
 * \deprecated This function is deprecated. User shall call hids_set_report() and
 * hids_notify_input_report() instead.
 *
 */
bool hids_set_report_value(ble_service_t *svc, hids_report_type_t type, uint8_t report_id,
                                uint16_t length, const uint8_t *data) __attribute__ ((deprecated));

/**
 * \brief Set Report
 *
 * Function sets value of specified Report Characteristic.
 *
 * \param [in] svc              service instance
 * \param [in] type             report type
 * \param [in] report_id        report ID
 * \param [in] length           length of data
 * \param [in] data             new value
 *
 * \return true if value has been set successfully.
 *
 */
bool hids_set_report(ble_service_t *svc, hids_report_type_t type, uint8_t report_id,
                                                        uint16_t length, const uint8_t *data);

/**
 * \brief Notify Input Report
 *
 * Function sends notification of specified Input Report Characteristic value.
 *
 * \param [in] svc              service instance
 * \param [in] report_id        input report ID
 * \param [in] length           length of data
 * \param [in] data             new value
 *
 * \return true if notification has been sent successfully, otherwise false
 * (if CCC descriptor is not configured properly it will return false).
 * Callback notify_input_report_completed() will be called once completed.
 */
bool hids_notify_input_report(ble_service_t *svc, uint8_t report_id, uint16_t length,
                                                                        const uint8_t *data);

#endif /* HIDS_H_ */
