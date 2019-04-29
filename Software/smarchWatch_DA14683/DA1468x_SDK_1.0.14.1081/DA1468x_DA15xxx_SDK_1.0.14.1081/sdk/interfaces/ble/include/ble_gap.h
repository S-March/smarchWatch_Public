/**
 \addtogroup INTERFACES
 \{
 \addtogroup BLE
 \{
 \addtogroup API
 \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_gap.h
 *
 * @brief BLE GAP API
 *
 * Copyright (C) 2015-2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BLE_GAP_H_
#define BLE_GAP_H_

#include <stddef.h>
#include <stdint.h>
#include "co_bt.h"
#include "co_version.h"
#include "ble_att.h"
#include "ble_common.h"
#include "ble_config.h"

/** Maximum length of advertising data for connectable advertising packets in bytes
 * (3 bytes reserved for AD flags) */
#define BLE_ADV_DATA_LEN_MAX      (ADV_DATA_LEN - 3)

/** Maximum length of advertising data for non-connectable advertising packets */
#if ((dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A) && \
        (dg_configBLACK_ORCA_IC_STEP == BLACK_ORCA_IC_STEP_E))
        #define BLE_NON_CONN_ADV_DATA_LEN_MAX   (ADV_DATA_LEN - 3)
#else
        #define BLE_NON_CONN_ADV_DATA_LEN_MAX   (ADV_DATA_LEN)
#endif

/** Maximum length of scan response data in bytes */
#define BLE_SCAN_RSP_LEN_MAX      (SCAN_RSP_DATA_LEN)

/* Maximum length of device name in bytes (as defined by Bluetooth Core v4.2 / GAP) */
#define BLE_GAP_DEVNAME_LEN_MAX   (BD_NAME_SIZE)

/** Channel map size in bytes */
#define BLE_GAP_CHANNEL_MAP_LEN   (LE_CHNL_MAP_LEN)

/** Maximum number of connected devices */
#define BLE_GAP_MAX_CONNECTED   (defaultBLE_MAX_CONNECTIONS)

/** Maximum number of bonded devices */
#define BLE_GAP_MAX_BONDED      (defaultBLE_MAX_BONDED)

/** Convert time in milliseconds to advertising interval value */
#define BLE_ADV_INTERVAL_FROM_MS(MS) ((MS) * 1000 / 625)
/** Convert advertising interval value to time in milliseconds */
#define BLE_ADV_INTERVAL_TO_MS(VAL)  ((VAL) * 625 / 1000)
/** Convert time in milliseconds to scan interval value */
#define BLE_SCAN_INTERVAL_FROM_MS(MS) ((MS) * 1000 / 625)
/** Convert scan interval value to time in milliseconds */
#define BLE_SCAN_INTERVAL_TO_MS(VAL)  ((VAL) * 625 / 1000)
/** Convert time in milliseconds to scan window value */
#define BLE_SCAN_WINDOW_FROM_MS(MS) ((MS) * 1000 / 625)
/** Convert scan window value to time in milliseconds */
#define BLE_SCAN_WINDOW_TO_MS(VAL)  ((VAL) * 625 / 1000)
/** Convert time in milliseconds to connection event length value */
#define BLE_CONN_EVENT_LENGTH_FROM_MS(MS) ((MS) * 1000 / 625)
/** Convert connection event length value to time in milliseconds */
#define BLE_CONN_EVENT_LENGTH_TO_MS(VAL)  ((VAL) * 625 / 1000)
/** Convert time in milliseconds to connection interval value */
#define BLE_CONN_INTERVAL_FROM_MS(MS) ((MS) * 100 / 125)
/** Convert connection interval value to time in milliseconds */
#define BLE_CONN_INTERVAL_TO_MS(VAL)  ((VAL) * 125 / 100)
/** Convert time in milliseconds to supervision timeout value */
#define BLE_SUPERVISION_TMO_FROM_MS(MS) ((MS) / 10)
/** Convert supervision timeout value to time in milliseconds */
#define BLE_SUPERVISION_TMO_TO_MS(VAL)  ((VAL) * 10)

/**
 * Value for invalid connection index
 *
 * Portable code should use this value wherever it's required to mark connection index as invalid.
 */
#define BLE_CONN_IDX_INVALID    (0xFFFF)

/** RSSI value not available */
#define BLE_RSSI_NOT_AVAILABLE  (127)

/** GAP device external appearance */
typedef enum {
        BLE_GAP_APPEARANCE_UNKNOWN = 0,
        BLE_GAP_APPEARANCE_GENERIC_PHONE = 64,
        BLE_GAP_APPEARANCE_GENERIC_COMPUTER = 128,
        BLE_GAP_APPEARANCE_GENERIC_WATCH = 192,
        BLE_GAP_APPEARANCE_WATCH_SPORTS_WATCH = 193,
        BLE_GAP_APPEARANCE_GENERIC_CLOCK = 256,
        BLE_GAP_APPEARANCE_GENERIC_DISPLAY = 320,
        BLE_GAP_APPEARANCE_GENERIC_REMOTE_CONTROL = 384,
        BLE_GAP_APPEARANCE_GENERIC_EYE_GLASSES = 448,
        BLE_GAP_APPEARANCE_GENERIC_TAG = 512,
        BLE_GAP_APPEARANCE_GENERIC_KEYRING = 576,
        BLE_GAP_APPEARANCE_GENERIC_MEDIA_PLAYER = 640,
        BLE_GAP_APPEARANCE_GENERIC_BARCODE_SCANNER = 704,
        BLE_GAP_APPEARANCE_GENERIC_THERMOMETER = 768,
        BLE_GAP_APPEARANCE_THERMOMETER_EAR = 769,
        BLE_GAP_APPEARANCE_GENERIC_HEART_RATE_SENSOR = 832,
        BLE_GAP_APPEARANCE_HEART_RATE_SENSOR_HEART_RATE_BELT = 833,
        BLE_GAP_APPEARANCE_GENERIC_BLOOD_PRESSURE = 896,
        BLE_GAP_APPEARANCE_BLOOD_PRESSURE_ARM = 897,
        BLE_GAP_APPEARANCE_BLOOD_PRESSURE_WRIST = 898,
        BLE_GAP_APPEARANCE_GENERIC_HID = 960,
        BLE_GAP_APPEARANCE_HID_KEYBOARD = 961,
        BLE_GAP_APPEARANCE_HID_MOUSE = 962,
        BLE_GAP_APPEARANCE_HID_JOYSTICK = 963,
        BLE_GAP_APPEARANCE_HID_GAMEPAD = 964,
        BLE_GAP_APPEARANCE_HID_DIGITIZER_TABLET = 965,
        BLE_GAP_APPEARANCE_HID_CARD_READER = 966,
        BLE_GAP_APPEARANCE_HID_DIGITAL_PEN = 967,
        BLE_GAP_APPEARANCE_HID_BARCODE_SCANNER = 968,
        BLE_GAP_APPEARANCE_GENERIC_GLUCOSE_METER = 1024,
        BLE_GAP_APPEARANCE_GENERIC_RUNNING_WALKING_SENSOR = 1088,
        BLE_GAP_APPEARANCE_RUNNING_WALKING_SENSOR_IN_SHOE = 1089,
        BLE_GAP_APPEARANCE_RUNNING_WALKING_SENSOR_ON_SHOE = 1090,
        BLE_GAP_APPEARANCE_RUNNING_WALKING_SENSOR_ON_HIP = 1091,
        BLE_GAP_APPEARANCE_GENERIC_CYCLING = 1152,
        BLE_GAP_APPEARANCE_CYCLING_CYCLING_COMPUTER = 1153,
        BLE_GAP_APPEARANCE_CYCLING_SPEED_SENSOR = 1154,
        BLE_GAP_APPEARANCE_CYCLING_CADENCE_SENSOR = 1155,
        BLE_GAP_APPEARANCE_CYCLING_POWER_SENSOR = 1156,
        BLE_GAP_APPEARANCE_CYCLING_SPEED_AND_CADENCE_SENSOR = 1157,
        BLE_GAP_APPEARANCE_GENERIC_PULSE_OXIMETER = 3136,
        BLE_GAP_APPEARANCE_PULSE_OXIMETER_FINGERTIP = 3137,
        BLE_GAP_APPEARANCE_PULSE_OXIMETER_WRIST_WORN = 3138,
        BLE_GAP_APPEARANCE_GENERIC_WEIGHT_SCALE = 3200,
        BLE_GAP_APPEARANCE_GENERIC_OUTDOOR_SPORTS_ACTIVITY = 5184,
        BLE_GAP_APPEARANCE_OUTDOOR_SPORTS_ACT_LOCATION_DISPLAY = 5185,
        BLE_GAP_APPEARANCE_OUTDOOR_SPORTS_ACT_LOCATION_AND_NAVIGATION_DISPLAY = 5186,
        BLE_GAP_APPEARANCE_OUTDOOR_SPORTS_ACT_LOCATION_POD = 5187,
        BLE_GAP_APPEARANCE_OUTDOOR_SPORTS_ACT_LOCATION_AND_NAVIGATION_POD = 5188,
        // dummy appearance ID
        BLE_GAP_APPEARANCE_LAST,
} gap_appearance_t;

/**
 * GAP Advertising Data Types, as defined by Bluetooth Core 4.2 specification
 *
 * \note: only data types valid for Advertising Data are included
 */
typedef enum {
        GAP_DATA_TYPE_FLAGS               = 0x01,
        GAP_DATA_TYPE_UUID16_LIST_INC     = 0x02,
        GAP_DATA_TYPE_UUID16_LIST         = 0x03,
        GAP_DATA_TYPE_UUID32_LIST_INC     = 0x04,
        GAP_DATA_TYPE_UUID32_LIST         = 0x05,
        GAP_DATA_TYPE_UUID128_LIST_INC    = 0x06,
        GAP_DATA_TYPE_UUID128_LIST        = 0x07,
        GAP_DATA_TYPE_SHORT_LOCAL_NAME    = 0x08,
        GAP_DATA_TYPE_LOCAL_NAME          = 0x09,
        GAP_DATA_TYPE_TX_POWER_LEVEL      = 0x0A,
        GAP_DATA_TYPE_SLAVE_CONN_INTV     = 0x12,
        GAP_DATA_TYPE_UUID16_SOLIC        = 0x14,
        GAP_DATA_TYPE_UUID32_SOLIC        = 0x1F,
        GAP_DATA_TYPE_UUID128_SOLIC       = 0x15,
        GAP_DATA_TYPE_UUID16_SVC_DATA     = 0x16,
        GAP_DATA_TYPE_UUID32_SVC_DATA     = 0x20,
        GAP_DATA_TYPE_UUID128_SVC_DATA    = 0x21,
        GAP_DATA_TYPE_PUBLIC_ADDRESS      = 0x17,
        GAP_DATA_TYPE_RANDOM_ADDRESS      = 0x18,
        GAP_DATA_TYPE_APPEARANCE          = 0x19,
        GAP_DATA_TYPE_ADV_INTERVAL        = 0x1A,
        GAP_DATA_TYPE_MANUFACTURER_SPEC   = 0xFF,
} gap_data_type_t;

/**
 * \brief GAP events 
 */
enum ble_evt_gap {
        /** Connection established */
        BLE_EVT_GAP_CONNECTED = BLE_EVT_CAT_FIRST(BLE_EVT_CAT_GAP),
        /** Advertising report */
        BLE_EVT_GAP_ADV_REPORT,
        /** Disconnection event */
        BLE_EVT_GAP_DISCONNECTED,
        /** Disconnect failed event */
        BLE_EVT_GAP_DISCONNECT_FAILED,
        /** Advertising operation completed */
        BLE_EVT_GAP_ADV_COMPLETED,
        /** Scan operation completed */
        BLE_EVT_GAP_SCAN_COMPLETED,
        /** Connection parameter update request from peer */
        BLE_EVT_GAP_CONN_PARAM_UPDATE_REQ,
        /** Connection parameters updated */
        BLE_EVT_GAP_CONN_PARAM_UPDATED,
        /** Pairing request */
        BLE_EVT_GAP_PAIR_REQ,
        /** Pairing completed */
        BLE_EVT_GAP_PAIR_COMPLETED,
        /** Security request from peer */
        BLE_EVT_GAP_SECURITY_REQUEST,
        /** Passkey notification */
        BLE_EVT_GAP_PASSKEY_NOTIFY,
        /** Passkey request */
        BLE_EVT_GAP_PASSKEY_REQUEST,
        /** Security level changed indication */
        BLE_EVT_GAP_SEC_LEVEL_CHANGED,
        /** Random address resolved */
        BLE_EVT_GAP_ADDRESS_RESOLVED,
        /** Set security level failed */
        BLE_EVT_GAP_SET_SEC_LEVEL_FAILED,
        /** Connection parameters update completed */
        BLE_EVT_GAP_CONN_PARAM_UPDATE_COMPLETED,
        /** Data length changed */
        BLE_EVT_GAP_DATA_LENGTH_CHANGED,
        /** Data length set failed */
        BLE_EVT_GAP_DATA_LENGTH_SET_FAILED,
        /** Connection operation completed */
        BLE_EVT_GAP_CONNECTION_COMPLETED,
        /** Numeric request */
        BLE_EVT_GAP_NUMERIC_REQUEST,
        /** Address resolution failed */
        BLE_EVT_GAP_ADDRESS_RESOLUTION_FAILED,
        /** Long Term Key missing */
        BLE_EVT_GAP_LTK_MISSING,
};

/**
 * Advertise/Scan Response structure type representing AD Data Format
 * [BT Core 5.0, Vol 3, Part C, 11]
 *
 * \sa ::ble_gap_adv_ad_struct_set
 *
 */
typedef struct {
         uint8_t len;           ///< AD payload data length
         uint8_t type;          ///< AD type of payload data
         const uint8_t *data;   ///< AD payload data
} gap_adv_ad_struct_t;

/**
 * Helper macro to initialize single gap_adv_ad_struct_t object with fixed AD data like in this
 * call example GAP_ADV_AD_STRUCT(GAP_DATA_TYPE_LOCAL_NAME, DEVICE_ADV_NAME_LEN, DEVICE_ADV_NAME)
 */
#define GAP_ADV_AD_STRUCT(_ad_data_type, _ad_data_len, _ad_data)    \
                         { .type = (_ad_data_type),                 \
                           .len  = (_ad_data_len),                  \
                           .data = (const uint8_t *)(_ad_data) }

/**
 * Helper macro to initialize single gap_adv_ad_struct_t object by specifying set of individual
 * octets of AD data like this one GAP_ADV_AD_STRUCT_BYTES(GAP_DATA_TYPE_UUID16_LIST, 0x12, 0x18)
 */
#define GAP_ADV_AD_STRUCT_BYTES(_ad_data_type, _ad_data_bytes...)                          \
                                GAP_ADV_AD_STRUCT(_ad_data_type,                           \
                                                  sizeof((uint8_t[]) { _ad_data_bytes}),   \
                                                  ((uint8_t[]) { _ad_data_bytes}))

/**
 * Helper macro to instantiate & initialize single gap_adv_ad_struct_t object on stack with AD data
 * returning address of the object.
 */
#define GAP_ADV_AD_STRUCT_DECLARE(_ad_data_type, _ad_data_len, _ad_data)                     \
                                  (&(gap_adv_ad_struct_t) GAP_ADV_AD_STRUCT(_ad_data_type,   \
                                                                            _ad_data_len,    \
                                                                            _ad_data))

/**
 * Device properties
 *
 * \sa ble_gap_get_devices
 *
 */
typedef struct {
        bd_address_t address;           ///< Device address */
        uint16_t conn_idx;              ///< Connection index */
        bool connected : 1;             ///< True if device is currently connected */
        bool bonded : 1;                ///< True if device is currently bonded */
        bool paired : 1;                ///< True if device is currently paired
        bool mitm : 1;                  ///< True if keys are authenticated, i.e. with MITM protection (only valid if paired)
        bool secure : 1;                ///< True if there is currently a secure connection with the device
} gap_device_t;

/**
 * Device filter type
 *
 * \sa ble_gap_get_devices
 *
 */
typedef enum {
        GAP_DEVICE_FILTER_ALL,          ///< All known devices
        GAP_DEVICE_FILTER_CONNECTED,    ///< All connected devices
        GAP_DEVICE_FILTER_BONDED,       ///< All bonded devices
        GAP_DEVICE_FILTER_ADDRESS,      ///< Device with matching address
        GAP_DEVICE_FILTER_CONN_IDX,     ///< Device with matching connection index
} gap_device_filter_t;

/**
 * Additional device filter data
 *
 * \sa ble_gap_get_devices
 *
 */
typedef union {
        uint16_t conn_idx;              ///< Connection index
        bd_address_t address;           ///< Bluetooth device address
} gap_device_filter_data_t;

/** GAP security key structure */
typedef struct {
        uint8_t key[16];                ///< 128-bit key
} gap_sec_key_t;

/** GAP roles */
typedef enum {
        GAP_NO_ROLE = 0x00,             ///< No role
        GAP_OBSERVER_ROLE = 0x01,       ///< Observer role
        GAP_BROADCASTER_ROLE = 0x02,    ///< Broadcaster role
        GAP_CENTRAL_ROLE = 0x04,        ///< Central role
        GAP_PERIPHERAL_ROLE = 0x08,     ///< Peripheral role
        GAP_ALL_ROLES = (GAP_OBSERVER_ROLE|GAP_BROADCASTER_ROLE|GAP_CENTRAL_ROLE|GAP_PERIPHERAL_ROLE),  ///< All roles
} gap_role_t;

/** Link Layer channel map */
typedef struct gap_chnl_map {
        uint8_t   map[BLE_GAP_CHANNEL_MAP_LEN]; ///< GAP channel map
} gap_chnl_map_t;

/** GAP connectivity modes */
typedef enum {
        GAP_CONN_MODE_NON_CONN,         ///< Non-connectable mode
        GAP_CONN_MODE_UNDIRECTED,       ///< Undirected mode
        GAP_CONN_MODE_DIRECTED,         ///< Directed mode
        GAP_CONN_MODE_DIRECTED_LDC,     ///< Directed Low Duty Cycle mode
} gap_conn_mode_t;

/** GAP discoverability modes */
typedef enum {
        GAP_DISC_MODE_NON_DISCOVERABLE, ///< Non-Discoverable mode
        GAP_DISC_MODE_GEN_DISCOVERABLE, ///< General-Discoverable mode
        GAP_DISC_MODE_LIM_DISCOVERABLE, ///< Limited-Discoverable mode
        GAP_DISC_MODE_BROADCASTER,      ///< Broadcaster mode
} gap_disc_mode_t;

/** Channels used for advertising */
typedef enum {
        GAP_ADV_CHANNEL_37  = 0x01,     ///< Advertising Channel 37 (2402MHz)
        GAP_ADV_CHANNEL_38  = 0x02,     ///< Advertising Channel 38 (2426MHz)
        GAP_ADV_CHANNEL_39  = 0x04,     ///< Advertising Channel 39 (2480MHz)
} gap_adv_chnl_t;

/** Advertising filter policy */
typedef enum {
        ADV_ALLOW_SCAN_ANY_CONN_ANY,    ///< Allow all scan and connect requests
        ADV_ALLOW_SCAN_WLIST_CONN_ANY,  ///< Allow all connect requests and scan requests only from whitelist
        ADV_ALLOW_SCAN_ANY_CONN_WLIST,  ///< Allow all scan requests and connect requests only from whitelist
        ADV_ALLOW_SCAN_WLIST_CONN_WLIST,///< Allow scan and connect requests only from whitelist
} adv_filt_pol_t;

/** Advertising report event types */
enum {
        GAP_ADV_IND,                    ///< General advertising indication
        GAP_ADV_DIRECT_IND,             ///< Direct connection indication
        GAP_ADV_SCAN_IND,               ///< Scannable advertising indication
        GAP_ADV_NONCONN_IND,            ///< Non-connectable advertising indication
        GAP_SCAN_RSP,                   ///< Active scanning response
};

/** Scanning types */
typedef enum {
        GAP_SCAN_ACTIVE,                ///< Active Scan type
        GAP_SCAN_PASSIVE,               ///< Passive Scan type
} gap_scan_type_t;

/** Scanning modes */
typedef enum {
        GAP_SCAN_GEN_DISC_MODE,         ///< General-Discoverable mode
        GAP_SCAN_LIM_DISC_MODE,         ///< Limited-Discoverable mode
        GAP_SCAN_OBSERVER_MODE,         ///< Observer mode
} gap_scan_mode_t;

/** GAP authentication options */
typedef enum {
        GAP_AUTH_NO_MITM_NO_BOND = 0x00, ///< No MITM no bonding
        GAP_AUTH_NO_MITM_BOND    = 0x01, ///< No MITM bonding
        GAP_AUTH_MITM_NO_BOND    = 0x04, ///< MITM no bonding
        GAP_AUTH_MITM_BOND       = 0x05, ///< MITM bonding
} gap_auth_t;

/** GAP security levels */
typedef enum {
        GAP_SEC_LEVEL_1         = 0x00, ///< No security
        GAP_SEC_LEVEL_2         = 0x01, ///< Unauthenticated pairing with encryption
        GAP_SEC_LEVEL_3         = 0x02, ///< Authenticated pairing with encryption
        GAP_SEC_LEVEL_4         = 0x03, ///< Authenticated LE Secure Connections pairing with encryption

} gap_sec_level_t;

/** GAP Input/Output capabilities */
typedef enum {
        GAP_IO_CAP_DISP_ONLY           = 0x00, ///< Display only
        GAP_IO_CAP_DISP_YES_NO         = 0x01, ///< Display yes no
        GAP_IO_CAP_KEYBOARD_ONLY       = 0x02, ///< Keyboard only
        GAP_IO_CAP_NO_INPUT_OUTPUT     = 0x03, ///< No input no output
        GAP_IO_CAP_KEYBOARD_DISP       = 0x04, ///< Keyboard display
} gap_io_cap_t;

/** GAP connection parameters */
typedef struct {
        uint16_t interval_min;                          ///< Minimum connection interval
        uint16_t interval_max;                          ///< Maximum connection interval
        uint16_t slave_latency;                         ///< Slave latency
        uint16_t sup_timeout;                           ///< Supervision timeout
} gap_conn_params_t;

/** GAP scan parameters */
typedef struct {
        uint16_t interval;                              ///< Scan interval
        uint16_t window;                                ///< Scan window
} gap_scan_params_t;

/** Structure for ::BLE_EVT_GAP_CONNECTED event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint16_t           conn_idx;                    ///< Connection index
        bd_address_t       own_addr;                    ///< Own device BD address
        bd_address_t       peer_address;                ///< Peer device BD address
        gap_conn_params_t  conn_params;                 ///< Connection parameters
} ble_evt_gap_connected_t;

/** Structure for ::BLE_EVT_GAP_DISCONNECTED event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint16_t           conn_idx;                    ///< Connection index
        bd_address_t       address;                     ///< BD address of disconnected device
        uint8_t            reason;                      ///< Reason of disconnection
} ble_evt_gap_disconnected_t;

/** Structure for ::BLE_EVT_GAP_DISCONNECT_FAILED event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint16_t           conn_idx;                    ///< Connection index
        uint8_t            status;                      ///< Error status
} ble_evt_gap_disconnect_failed_t;

/** Structure for ::BLE_EVT_GAP_CONN_PARAM_UPDATE_REQ event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint16_t           conn_idx;                    ///< Connection index
        gap_conn_params_t  conn_params;                 ///< Connection parameters
} ble_evt_gap_conn_param_update_req_t;

/** Structure for ::BLE_EVT_GAP_CONN_PARAM_UPDATE_COMPLETED event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint16_t           conn_idx;                    ///< Connection index
        uint8_t            status;                      ///< Completion status
} ble_evt_gap_conn_param_update_completed_t;

/** Structure for ::BLE_EVT_GAP_CONN_PARAM_UPDATED event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint16_t           conn_idx;                    ///< Connection index
        gap_conn_params_t  conn_params;                 ///< Connection parameters
} ble_evt_gap_conn_param_updated_t;

/** Structure for ::BLE_EVT_GAP_ADV_COMPLETED event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint8_t            adv_type;                    ///< Advertising type
        uint8_t            status;                      ///< Completion status
} ble_evt_gap_adv_completed_t;

/** Structure for ::BLE_EVT_GAP_ADV_REPORT event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint8_t            type;                        ///< Type of advertising packet
        bd_address_t       address;                     ///< BD address of advertising device
        uint8_t            rssi;                        ///< RSSI
        uint8_t            length;                      ///< Length of advertising data
        uint8_t            data[BLE_ADV_DATA_LEN_MAX];  ///< Advertising data or scan response data
} ble_evt_gap_adv_report_t;

/** Structure for ::BLE_EVT_GAP_SCAN_COMPLETED event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint8_t            scan_type;                   ///< Scan type
        uint8_t            status;                      ///< Completion status
} ble_evt_gap_scan_completed_t;

/** Structure for ::BLE_EVT_GAP_PAIR_REQ event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint16_t           conn_idx;                    ///< Connection index
        bool               bond;                        ///< Enable bond
} ble_evt_gap_pair_req_t;

/** Structure for ::BLE_EVT_GAP_PAIR_COMPLETED event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint16_t           conn_idx;                    ///< Connection index
        uint8_t            status;                      ///< Completion status
        bool               bond;                        ///< Bond enabled flag
        bool               mitm;                        ///< MITM protection enabled flag
} ble_evt_gap_pair_completed_t;

/** Structure for ::BLE_EVT_GAP_SECURITY_REQUEST event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint16_t           conn_idx;                    ///< Connection index
        bool               bond;                        ///< Bond requested flag
        bool               mitm;                        ///< MITM requested flag
} ble_evt_gap_security_request_t;

/** Structure for ::BLE_EVT_GAP_PASSKEY_NOTIFY event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint16_t           conn_idx;                    ///< Connection index
        uint32_t           passkey;                     ///< Passkey
} ble_evt_gap_passkey_notify_t;

/** Structure for ::BLE_EVT_GAP_PASSKEY_REQUEST event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint16_t           conn_idx;                    ///< Connection index
} ble_evt_gap_passkey_request_t;

#if (dg_configBLE_SECURE_CONNECTIONS == 1)
/** Structure for ::BLE_EVT_GAP_NUMERIC_REQUEST event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint16_t           conn_idx;                    ///< Connection index
        uint32_t           num_key;                     ///< Numeric comparison key
} ble_evt_gap_numeric_request_t;
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */

/** Structure for ::BLE_EVT_GAP_ADDRESS_RESOLVED event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint16_t           conn_idx;                    ///< Connection index
        bd_address_t       resolved_address;            ///< Static address
        bd_address_t       address;                     ///< Random address
} ble_evt_gap_address_resolved_t;

/** Structure for ::BLE_EVT_GAP_ADDRESS_RESOLUTION_FAILED event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint16_t           status;                      ///< Status
} ble_evt_gap_address_resolution_failed_t;

/** Structure for ::BLE_EVT_GAP_SEC_LEVEL_CHANGED event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint16_t           conn_idx;                    ///< Connection index
        gap_sec_level_t    level;                       ///< Security level
} ble_evt_gap_sec_level_changed_t;

/** Structure for ::BLE_EVT_GAP_SET_SEC_LEVEL_FAILED event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint16_t           conn_idx;                    ///< Connection index
        ble_error_t        status;                      ///< Completion status
} ble_evt_gap_set_sec_level_failed_t;

/** Structure for ::BLE_EVT_GAP_DATA_LENGTH_CHANGED event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint16_t           conn_idx;                    ///< Connection index
        uint16_t           max_rx_length;               ///< Maximum number of payload octets in RX
        uint16_t           max_rx_time;                 ///< Maximum time used for RX
        uint16_t           max_tx_length;               ///< Maximum number of payload octets in TX
        uint16_t           max_tx_time;                 ///< Maximum time used for TX
} ble_evt_gap_data_length_changed_t;

/** Structure for ::BLE_EVT_GAP_DATA_LENGTH_SET_FAILED event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint16_t           conn_idx;                    ///< Connection index
        uint16_t           status;                      ///< Error status
} ble_evt_gap_data_length_set_failed_t;

/** Structure for ::BLE_EVT_GAP_CONNECTION_COMPLETED event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint8_t            status;                      ///< Completion status
} ble_evt_gap_connection_completed_t;

/** Structure for ::BLE_EVT_GAP_LTK_MISSING event */
typedef struct {
        ble_evt_hdr_t      hdr;                         ///< Event header
        uint16_t           conn_idx;                    ///< Connection index
} ble_evt_gap_ltk_missing_t;

/**
 * \brief Retrieve the currently set BD address
 *
 * This API call is used to retrieve the currently set BD address of the device.
 *
 * \param [out]  address    Buffer to store the BD address
 *
 * \return result code
 */
ble_error_t ble_gap_address_get(own_address_t *address);

/**
 * \brief Set the address of the device
 *
 * This API call is used to set the BD address of the device. If the address type is not
 * ::PRIVATE_STATIC_ADDRESS the address passed is ignored (public static is set either in NVPARAM or
 * using defaultBLE_STATIC_ADDRESS and private random addresses are automatically generated by the
 * BLE stack every \p renew_dur tens of ms).
 *
 * \note
 * When the address of a peripheral device is set to be non-resolvable, then the advertising type
 * has to be non-connectable.
 *
 * \note
 * This API function has to be called prior to creating the attribute database of the device. This
 * is because the device configuration is going to be modified, which will result in clearing the
 * current attribute database (if it exists).
 *
 * \param [in] address    Pointer to the address to be set
 * \param [in] renew_dur  Random address renew duration in steps of 10ms - minimum is 150s by stack
 *
 * \return result code
 */
ble_error_t ble_gap_address_set(const own_address_t *address, uint16_t renew_dur);

/**
 * \brief Set the device name used for GAP service
 *
 * \note
 * This API function has to be called prior to creating the attribute database of the device. This
 * is because the device configuration is going to be modified, which will result in clearing the
 * current attribute database (if it exists).
 *
 * \param [in] name      Pointer to the device name
 * \param [in] perm      Device name attribute write permission
 *
 * \return result code
 */
ble_error_t ble_gap_device_name_set(const char *name, att_perm_t perm);

/**
 * \brief Get the device name used for GAP service
 *
 * \param [in]     name      Pointer to empty buffer where the device name (NULL-terminated) shall
 *                           be placed.
 * \param [in,out] length    Empty buffer size on input, currently set device name length on output.
 *
 * \return result code
 */
ble_error_t ble_gap_device_name_get(char *name, uint8_t *length);

/**
 * \brief Set the appearance used for GAP service
 *
 * \note
 * This API function has to be called prior to creating the attribute database of the device. This
 * is because the device configuration is going to be modified, which will result in clearing the
 * current attribute database (if it exists).
 *
 * \param [in] appearance  Appearance value
 * \param [in] perm        Appearance attribute write permission
 *
 * \return result code
 */
ble_error_t ble_gap_appearance_set(gap_appearance_t appearance, att_perm_t perm);

/**
 * \brief Get the appearance used for GAP service
 *
 * \param [in] appearance  Pointer to where the appearance value should be stored
 *
 * \return result code
 */
ble_error_t ble_gap_appearance_get(gap_appearance_t *appearance);

/**
 * \brief Set the peripheral preferred connection parameters used for GAP service
 *
 * \note
 * This API function has to be called prior to creating the attribute database of the device. This
 * is because the device configuration is going to be modified, which will result in clearing the
 * current attribute database (if it exists).
 *
 * \param [in] conn_params  Preferred connection parameters.
 *
 * \return result code
 */
ble_error_t ble_gap_per_pref_conn_params_set(const gap_conn_params_t *conn_params);

/**
 * \brief Get the peripheral preferred connection parameters currently set for GAP service
 *
 * \param [in] conn_params  Pointer to where the preferred connection parameters shall be stored.
 *
 * \return result code
 */
ble_error_t ble_gap_per_pref_conn_params_get(gap_conn_params_t *conn_params);

/**
 * \brief Start advertising
 *
 * This API call is used to start an advertising air operation. If \p adv_type is set to be
 * ::GAP_CONN_MODE_NON_CONN or ::GAP_CONN_MODE_UNDIRECTED, the air operation will go on until it is
 * stopped using ble_gap_adv_stop(). If \p adv_type is set to be ::GAP_CONN_MODE_DIRECTED or
 * ::GAP_CONN_MODE_DIRECTED_LDC (low duty cycle advertising), the air operation will automatically
 * stop after 1.28s. In both cases, upon advertising completion, a ::BLE_EVT_GAP_ADV_COMPLETED event
 * will be sent to the application.
 *
 * \param [in] adv_type  Type of advertising
 *
 * \return result code
 *
 */
ble_error_t ble_gap_adv_start(gap_conn_mode_t adv_type);

/**
 * \brief Stop advertising
 *
 * This API call is used to stop a previously started advertising air operation. If advertising is
 * successfully stopped, the application will receive a ::BLE_EVT_GAP_ADV_COMPLETED with status set
 * to ::BLE_ERROR_CANCELED.
 *
 * \return result code
 *
 */
ble_error_t ble_gap_adv_stop(void);

/**
 * \brief Set advertising Data and scan response data
 *
 * This API call is used to modify the advertising data and scan response Data used. It can be used
 * while an advertising operation is in progress. If an advertising operation is not in progress,
 * the new Advertising Data and/or new Scan Response Data will be used the next time function
 * ble_gap_adv_start() is called. The maximum Advertising Data length for undirected connectable
 * advertising is BLE_ADV_DATA_LEN_MAX bytes (31 minus 3 that are reserved to set the Advertising Data
 * type flags - which shall not be set in Advertising Data using this function). The equivalent max
 * length for non-connectable advertising is BLE_NON_CONN_ADV_DATA_LEN_MAX bytes.
 *
 * \param [in] adv_data_len         Length of the Advertising Data
 * \param [in] adv_data             Pointer to the Advertising Data
 * \param [in] scan_rsp_data_len    Length of the Scan Response Data
 * \param [in] scan_rsp_data        Pointer to the Scan Response Data
 *
 * \return result code
 */
ble_error_t ble_gap_adv_data_set(uint8_t adv_data_len, const uint8_t *adv_data,
                                 uint8_t scan_rsp_data_len, const uint8_t *scan_rsp_data);

/**
 * \brief Set Advertising Data and Scan Response Data using ::gap_adv_ad_struct_t type
 *
 * This API call sets advertising and scan response data. The function is to be a wrapper
 * to ::ble_gap_adv_data_set() API. Internally it makes validation and then transforms inputs
 * advertising data before it reuses call to ble_gap_adv_data_set().
 * The API uses as input ::gap_adv_ad_struct_t data type that logically expressing AD Data format
 * that conforms to Bluetooth Core Specification [Core 5.0, Vol 3, Part C, 11]
 * Such individual input objects can be easily instantiated by making call to helper macros
 * ::GAP_ADV_AD_STRUCT(), ::GAP_ADV_AD_STRUCT_BYTES() and ::GAP_ADV_AD_STRUCT_DECLARE().
 * Such objects can be used separately or can be aggregated into an array to feed the API input.
 *
 * \param [in] ad_len               Number of Advertising Data objects
 * \param [in] ad                   Pointer to Advertising Data object(s)
 * \param [in] sd_len               Number of Scan Response Data objects
 * \param [in] sd                   Pointer to Scan Response Data object(s)
 *
 * \return result code
 */
ble_error_t ble_gap_adv_ad_struct_set(size_t ad_len, const gap_adv_ad_struct_t *ad, size_t sd_len,
                                                                    const gap_adv_ad_struct_t *sd);

/**
 * \brief Get currently used Advertising Data and Scan Response Data
 *
 * Get the Advertising Data and Scan Response Data currently used. This can be used while an
 * advertising operation is in progress. \p adv_data_len and \p scan_rsp_data_len can be set to 0 to
 * get the length of the currently set advertising data and scan response data before allocating
 * any buffers.
 *
 * \param [in,out] adv_data_len         Empty buffer size on input, length of advertising data set
 *                                      on output
 * \param [in]     adv_data             Pointer to the empty buffer the advertising aata should be
 *                                      copied into
 * \param [in,out] scan_rsp_data_len    Empty buffer size on input, length of scan response data set
 *                                      on output
 * \param [in]     scan_rsp_data        Pointer to the empty buffer the scan response data should be
 *                                      copied into
 *
 * \return result code
 */
ble_error_t ble_gap_adv_data_get(uint8_t *adv_data_len, uint8_t *adv_data,
                                 uint8_t *scan_rsp_data_len, uint8_t *scan_rsp_data);

/**
 * \brief Get the currently set advertising interval
 *
 * Get the minimum and maximum advertising intervals currently set.
 *
 * \param [out] adv_intv_min   Minimum interval in steps of 0.625ms
 * \param [out] adv_intv_max   Maximum interval in steps of 0.625ms
 *
 * \return result code
 */
ble_error_t ble_gap_adv_intv_get(uint16_t *adv_intv_min, uint16_t *adv_intv_max);

/**
 * \brief Set the advertising interval
 *
 * Set the minimum and maximum interval to be used for advertising. Intervals are set in steps of
 * 0.625ms. Allowed values for intervals span from 0x20 (20ms) to 0x4000 (10.24s), while for
 * non-connectable advertising the range is 0xA0 (100ms) to 0x4000 (10.24s).
 *
 * \note This function has to be called prior to an advertising start (\sa ble_gap_adv_start()) and
 *       it will not modify the advertising interval of an ongoing advertising operation.
 *
 * \param [in] adv_intv_min    Minimum interval in steps of 0.625ms
 * \param [in] adv_intv_max    Maximum interval in steps of 0.625ms
 *
 * \return result code
 */
ble_error_t ble_gap_adv_intv_set(uint16_t adv_intv_min, uint16_t adv_intv_max);

/**
 * \brief Get the advertising channel map currently set
 *
 * \note \p chnl_map will be constructed using the members of ::gap_adv_chnl_t:
 *       ::GAP_ADV_CHANNEL_37, ::GAP_ADV_CHANNEL_38, ::GAP_ADV_CHANNEL_39.
 *
 * \param [out] chnl_map    Channel map currently used for advertising
 *
 * \return result code
 */
ble_error_t ble_gap_adv_chnl_map_get(uint8_t *chnl_map);

/**
 * \brief Set the advertising channel map
 *
 * \note This function has to be called prior to an advertising start (\sa ble_gap_adv_start()) and
 *       it will not modify the channel map of an ongoing advertising operation.
 *
 * \note \p chnl_map must be constructed using the members of ::gap_adv_chnl_t:
 *       ::GAP_ADV_CHANNEL_37, ::GAP_ADV_CHANNEL_38, ::GAP_ADV_CHANNEL_39.
 *
 * \param [in] chnl_map    Channel map used for advertising
 *
 * \return result code
 */
ble_error_t ble_gap_adv_chnl_map_set(uint8_t chnl_map);

/**
 * \brief Get the discoverability mode used for advertising
 *
 * \param [out] adv_mode   Discoverability mode
 *
 * \return result code
 */
ble_error_t ble_gap_adv_mode_get(gap_disc_mode_t *adv_mode);

/**
 * \brief Set the discoverability mode used for advertising
 *
 * \note This function has to be called prior to an advertising start (\sa ble_gap_adv_start()) and
 *       it will not modify the discoverability mode of an ongoing advertising operation.
 *
 * \param [in] adv_mode    Discoverability mode used for advertising
 *
 * \return result code
 */
ble_error_t ble_gap_adv_mode_set(gap_disc_mode_t adv_mode);

/**
 * \brief Get the filtering policy used for advertising
 *
 * \param [out] filt_policy    Filtering policy used for advertising
 *
 * \return result code
 */
ble_error_t ble_gap_adv_filt_policy_get(adv_filt_pol_t *filt_policy);

/**
 * \brief Set the filtering policy used for advertising
 *
 * \note This function has to be called prior to an advertising start (\sa ble_gap_adv_start()) and
 *       it will not modify the filtering policy of an ongoing advertising operation.
 *
 * \param [in] filt_policy    Filtering policy used for advertising
 *
 * \return result code
 */
ble_error_t ble_gap_adv_filt_policy_set(adv_filt_pol_t filt_policy);

/**
 * \brief Get the peer address used for directed advertising
 *
 * \param [out] address    Peer address used for directed advertising
 *
 * \return result code
 *
 */
ble_error_t ble_gap_adv_direct_address_get(bd_address_t *address);

/**
 * \brief Set peer address used for directed advertising
 *
 * \note This function has to be called prior to an advertising start (\sa ble_gap_adv_start()) and
 *       it will not modify the direct address used for an ongoing advertising operation.
 *
 * \param [in] address    Peer address used for directed advertising
 *
 * \return result code
 */
ble_error_t ble_gap_adv_direct_address_set(const bd_address_t *address);

/**
 * \brief Start scanning for devices
 *
 * This call initiates a scan procedure. The scan duration depends on the scan mode selected.
 * In General-discoverable and Limited-discoverable modes, the scan will stop after 10s of activity.
 * In Observer mode, the scan operation will continue until it is stopped using ble_gap_scan_stop().
 * The scan \p interval and \p window can be set in steps of 0.625ms. Allowed values for interval
 * span in the range of 0x4 (2.5ms) to 0x4000 (10.24s).
 *
 * \param [in] type        Active or passive scanning
 * \param [in] mode        Scan for General-discoverable, Limited-discoverable or for all devices
 * \param [in] interval    Scan interval in steps of 0.625ms
 * \param [in] window      Scan window in steps of 0.625ms
 * \param [in] filt_wlist  Enable or disable white list filtering
 * \param [in] filt_dupl   Enable or disable filtering of duplicates
 *
 * \return result code
 */
ble_error_t ble_gap_scan_start(gap_scan_type_t type, gap_scan_mode_t mode, uint16_t interval,
                               uint16_t window, bool filt_wlist, bool filt_dupl);

/**
 * \brief Stop scanning for devices
 *
 * This call stops a scan procedure previously started using ble_gap_scan_start().
 *
 * \return result code
 *
 */
ble_error_t ble_gap_scan_stop(void);

/**
 * \brief Get the scan parameters used for connections
 *
 * This call retrieves the scan parameters used when a connection is initiated.
 *
 * \param [out] scan_params   Pointer to the structure where the scan parameters will be stored.
 *
 * \return result code
 */
ble_error_t ble_gap_scan_params_get(gap_scan_params_t *scan_params);

/**
 * \brief Set the scan parameters used for connections
 *
 * This call sets the scan parameters used for initiated connections. This call won't change
 * the scan parameters of a connection attempt which is in progress (the scan parameters will be
 * set for the next connection attempt).
 *
 * \note This call should be used prior to ble_gap_connect(). If a connection attempt is in
 *       progress, one should cancel it using the ble_gap_connect_cancel() call, set the desired
 *       scan parameters, and call ble_gap_connect() again.
 *
 * \param [in] scan_params   Pointer to the scan parameters structure
 *
 * \return result code
 */
ble_error_t ble_gap_scan_params_set(const gap_scan_params_t *scan_params);

/**
 * \brief Connect to a device
 *
 * This call initiates a direct connection procedure to a specified device. The application will get
 * a ::BLE_EVT_GAP_CONNECTED event when the connection is established and a
 * ::BLE_EVT_GAP_CONNECTION_COMPLETED event when the connection procedure is completed either
 * successfully or with error (in the second case, ::BLE_EVT_GAP_CONNECTED will not be received).
 *
 * \param [in]  peer_addr    Pointer to the BD address of the peer device.
 * \param [in]  conn_params  Pointer to the connection parameters to be used.
 *
 * \return result code
 */
ble_error_t ble_gap_connect(const bd_address_t *peer_addr, const gap_conn_params_t *conn_params);

/**
 * \brief Connect to a device with a defined connection event length
 *
 * This is an extension to ble_gap_connect(). It initiates a direct connection procedure to a
 * specified device. The application will get a ::BLE_EVT_GAP_CONNECTED event when the connection is
 * established and a ::BLE_EVT_GAP_CONNECTION_COMPLETED event when the connection procedure is
 * completed either successfully or with error (in the second case, ::BLE_EVT_GAP_CONNECTED will not
 * be received). Compared to ble_gap_connect(), this allows setting the minimum and maximum lengths
 * for the connection event when used by the master of the connection (\p ce_len_min and
 * \p ce_len_max will be ignored when used by the slave of the connection).
 *
 * \param [in]  peer_addr    Pointer to the BD address of the peer device.
 * \param [in]  conn_params  Pointer to the connection parameters to be used.
 * \param [in]  ce_len_min   Minimum connection event length (in steps of 0.625us)
 * \param [in]  ce_len_max   Maximum connection event length (in steps of 0.625us)
 *
 * \return result code
 */
ble_error_t ble_gap_connect_ce(const bd_address_t *peer_addr, const gap_conn_params_t *conn_params,
                               uint16_t ce_len_min, uint16_t ce_len_max);
/**
 * \brief Cancel an initiated connection
 *
 * This call cancels a previously started connection procedure using ble_gap_connect(). The
 * application will receive a ::BLE_EVT_GAP_CONNECTION_COMPLETED event with status set to
 * ::BLE_ERROR_CANCELED if the connection procedure is successfully canceled.
 *
 * \return result code
 */
ble_error_t ble_gap_connect_cancel(void);

/**
 * \brief Terminate a connection
 *
 * This call initiates a disconnection procedure on an established link.
 *
 * \param [in]  conn_idx       Connection index
 * \param [in]  reason         Reason for disconnection.
 *
 * \return result code
 *
 * \note Valid reasons for initiating a disconnection are:
 *       ::BLE_HCI_ERROR_AUTH_FAILURE
 *       ::BLE_HCI_ERROR_REMOTE_USER_TERM_CON
 *       ::BLE_HCI_ERROR_REMOTE_DEV_TERM_LOW_RESOURCES
 *       ::BLE_HCI_ERROR_REMOTE_DEV_POWER_OFF
 *       ::BLE_HCI_ERROR_UNSUPPORTED_REMOTE_FEATURE
 *       ::BLE_HCI_ERROR_PAIRING_WITH_UNIT_KEY_NOT_SUP
 *       ::BLE_HCI_ERROR_UNACCEPTABLE_CONN_INT
 *       If API is called with a different reason, disconnection will fail with return status
 *       ::BLE_ERROR_INVALID_PARAM.
 *
 * \note After calling this function, the application will receive one of the following messages:
 *       ::BLE_EVT_GAP_DISCONNECTED when the disconnection procedure was successful.
 *       ::BLE_EVT_GAP_DISCONNECT_FAILED with error status when the disconnection procedure failed.
 *
 */
ble_error_t ble_gap_disconnect(uint16_t conn_idx, ble_hci_error_t reason);

/**
 * \brief Retrieve the RSSI of a connection
 *
 * This call retrieves the RSSI of an established connection. Value ::BLE_RSSI_NOT_AVAILABLE of
 * parameter \p conn_rssi informs that RSSI is not available.
 *
 * \param [in]   conn_idx       Connection index
 * \param [out]  conn_rssi      Connection RSSI
 *
 * \return result code
 *
 */
ble_error_t ble_gap_conn_rssi_get(uint16_t conn_idx, int8_t *conn_rssi);

/**
 * \brief Get the GAP role currently set
 *
 * This call gets the currently set GAP role of the device.
 *
 * \param [out]  role   GAP role
 *
 * \return result code
 *
 */
ble_error_t ble_gap_role_get(gap_role_t *role);

/**
 * \brief Set GAP role
 *
 * This call sets the GAP role of the device.
 *
 * \note If an air operation is in progress, this function will return with ::BLE_ERROR_NOT_ALLOWED
 *       status.
 *
 * \note This API function has to be called prior to creating the attribute database of the device.
 *       This is because the device configuration is going to be modified, which will result in
 *       clearing the current attribute database (if it exists).
 *
 * \param [in]  role   GAP role
 *
 * \return result code
 *
 */
ble_error_t ble_gap_role_set(const gap_role_t role);

/**
 * \brief Get MTU size
 *
 * This call retrieves the Maximum Protocol Unit size that is used in exchange MTU transactions
 * with peers.
 *
 * \param [out]  mtu_size  MTU size
 *
 * \return result code
 *
 */
ble_error_t ble_gap_mtu_size_get(uint16_t *mtu_size);

/**
 * \brief Set MTU size
 *
 * This call sets the Maximum Protocol Unit size that will be used in exchange MTU transactions
 * with peers.
 *
 * \note
 * This API function has to be called prior to creating the attribute database of the device. This
 * is because the device configuration is going to be modified, which will result in clearing the
 * current attribute database (if it exists).
 *
 * \param [in]  mtu_size   MTU size
 *
 * \return result code
 *
 */
ble_error_t ble_gap_mtu_size_set(uint16_t mtu_size);

/**
 * \brief Get the currently set channel map of the device (device has to be configured as central)
 *
 * This API call is used to retrieve the channel map currently set for the device. The channel map
 * consists of 37 bits. The n-th bit (in the range 0 to 36) contains the value for the link layer
 * channel index n. A bit equal to 0 indicates that the corresponding channel is unused, and a bit
 * equal to 1 indicates that the corresponding channel is used. The most significant bits are
 * reserved and shall be set to 0.
 *
 * \param [out]  chnl_map   Channel map (only the 37 least significant bits are used)
 *
 * \return result code
 */
ble_error_t ble_gap_channel_map_get(uint64_t *chnl_map);

/**
 * \brief Set the channel map of the device (device has to be configured as central)
 *
 * This API call is used to modify the channel map of the device. The device has to be central to
 * have the ability to change the channel map. The channel map consists of 37 bits. The n-th bit
 * (in the range 0 to 36) contains the value for the link layer channel index n. A bit equal to 0
 * indicates that the corresponding channel is unused, and a bit equal to 1 indicates that the
 * corresponding channel is used. The most significant bits are reserved and shall be set to 0.
 *
 * \param [in]  chnl_map   Channel map (only the 37 least significant bits are used)
 *
 * \return result code
 */
ble_error_t ble_gap_channel_map_set(const uint64_t chnl_map);

/**
 * \brief Initiate a connection parameter update
 *
 * This call can be used by both the master and the slave of the connection to initiate a connection
 * parameter update. For the master of the connection, the new connection parameters will be applied
 * immediately. For the slave of the connection, a connection parameter update request will be send
 * to the master. If the master accepts the connection parameters, it will be in charge of applying
 * them (which will result in a ::BLE_EVT_GAP_CONN_PARAM_UPDATED event message to the slave that
 * initiated the connection parameter update process). If 30s elapse without a response from the
 * master, the connection will be terminated.
 *
 * \param [in]  conn_idx       Connection index
 * \param [in]  conn_params    Connection parameters
 *
 * \return result code
 */
ble_error_t ble_gap_conn_param_update(uint16_t conn_idx, const gap_conn_params_t *conn_params);

/**
 * \brief Reply to a connection parameter update request
 *
 * This call should be used to reply to a connection parameter update request event
 * (::BLE_EVT_GAP_CONN_PARAM_UPDATE_REQ) message.
 *
 * \param [in]  conn_idx  Connection index
 * \param [in]  accept    Accept flag (1 to accept, 0 to reject)
 *
 * \return result code
 */
ble_error_t ble_gap_conn_param_update_reply(uint16_t conn_idx, bool accept);

/**
 * \brief Start pairing
 *
 * This call starts a pairing or bonding procedure. Depending on whether the device is master or
 * slave on the connection, it will send a pairing or a security request respectively.
 *
 * \param [in]  conn_idx        Connection index
 * \param [in]  bond            Whether it starts pairing or bonding procedure
 *
 * \return ::BLE_STATUS_OK if request has been send successfully.
 *         ::BLE_ERROR_FAILED if request hasn't been send successfully
 *         ::BLE_ERROR_ALREADY_DONE if device is already paired or bonded respectively
 *         ::BLE_ERROR_INS_RESOURCES if there is BLE_GAP_MAX_BONDED number of bonded
 *           devices
 *
 */
ble_error_t ble_gap_pair(uint16_t conn_idx, bool bond);

/**
 * \brief Respond to a pairing request
 *
 * The application should use this function to respond to a ::BLE_EVT_GAP_PAIR_REQ event.
 *
 * \param [in]  conn_idx        Connection index
 * \param [in]  accept          Accept flag
 * \param [in]  bond            Bonding flag
 *
 * \return ::BLE_STATUS_OK if reply has been send successfully.
 *         ::BLE_ERROR_FAILED if reply hasn't been send successfully
 *         ::BLE_ERROR_INS_RESOURCES if there is BLE_GAP_MAX_BONDED number of bonded
 *           devices
 *
 */
ble_error_t ble_gap_pair_reply(uint16_t conn_idx, bool accept, bool bond);

/**
 * \brief Get connected devices list
 *
 * Connection indexes for currently connected devices are returned in \p conn_idx buffer which is
 * allocated internally and should be freed by called when not needed.
 *
 * \param [out] length          Length of returned array
 * \param [out] conn_idx        Array of connections indexes
 *
 */
ble_error_t ble_gap_get_connected(uint8_t *length, uint16_t **conn_idx);

/**
 * \brief Get bonded devices list
 *
 * Addresses for currently bonded devices are returned in \p addr buffer which is
 * allocated internally and should be freed by called when not needed.
 *
 * \param [out] length          Length of returned array
 * \param [out] addr            Array of bonded addresses
 *
 */
ble_error_t ble_gap_get_bonded(uint8_t *length, bd_address_t **addr);

/**
 * \brief Get the I/O capabilities of the device
 *
 * Get the currently set Input/Output Capabilities of the device (combined with the peer's I/O
 * capabilities, this will determine which pairing algorithm will be used).
 *
 * \param [out] io_cap  IO capabilities
 *
 */
ble_error_t ble_gap_get_io_cap(gap_io_cap_t *io_cap);

/**
 * \brief Set the I/O capabilities of the device
 *
 * Set the Input/Output Capabilities of the device (combined with the peer's I/O capabilities, this
 * will determine which pairing algorithm will be used).
 *
 * \param [in] io_cap   new IO capabilities
 *
 */
ble_error_t ble_gap_set_io_cap(gap_io_cap_t io_cap);

/**
 * \brief Respond to a passkey request
 *
 * Respond to a ::BLE_EVT_GAP_PASSKEY_REQUEST event.
 *
 * \param [in] conn_idx   Connection index
 * \param [in] accept     Accept flag
 * \param [in] passkey    Passkey entered by user
 */
ble_error_t ble_gap_passkey_reply(uint16_t conn_idx, bool accept, uint32_t passkey);

#if (dg_configBLE_SECURE_CONNECTIONS == 1)
/**
 * \brief Respond to a numeric comparison request
 *
 * Respond to a ::BLE_EVT_GAP_NUMERIC_REQUEST event.
 *
 * \param [in] conn_idx   Connection index
 * \param [in] accept     Accept flag
 */
ble_error_t ble_gap_numeric_reply(uint16_t conn_idx, bool accept);
#endif /* (dg_configBLE_SECURE_CONNECTIONS == 1) */

/**
 * \brief Get connection security level
 *
 * Get the currently established security level on a connection.
 *
 * \param [in]  conn_idx   Connection index
 * \param [out] level      Connection security level
 */
ble_error_t ble_gap_get_sec_level(uint16_t conn_idx, gap_sec_level_t *level);

/**
 * \brief Unpair command
 *
 * Use this function to unpair a device. This will also remove the device bond data from BLE storage.
 *
 * \param [in]  addr       Remote device address
 *
 * \return result code
 *
 */
ble_error_t ble_gap_unpair(const bd_address_t *addr);

/**
 * \brief Set connection security level
 *
 * Use this function to set the security level for a connection. If the device is already bonded,
 * it will use the existing LTK or request a new bonding. If the device is not bonded, it will
 * create a pairing or a security request (depending on whether the device is master or slave on the
 * connection) with the bond flag set to false.
 *
 * \param [in] conn_idx   Connection index
 * \param [in] level      New security level
 *
 * \return result code
 *
 */
ble_error_t ble_gap_set_sec_level(uint16_t conn_idx, gap_sec_level_t level);

/**
 * \brief Return list of known devices
 *
 * \param [in]     filter       device list filter to be applied
 * \param [in]     filter_data  additional data for filtering (depends on \p filter)
 * \param [in,out] length       length of devices list
 *                              (maximum allowed as input, number of returned as output)
 * \param [out]    gap_devices  returned devices
 *
 * \return result code
 *
 */
ble_error_t ble_gap_get_devices(gap_device_filter_t filter, gap_device_filter_data_t *filter_data,
                                                        size_t *length, gap_device_t *gap_devices);

/**
 * \brief Get device object by device address
 *
 * \param [in]  addr            Device address
 * \param [out] gap_device      Device object
 *
 * \return BLE_STATUS_OK if found, BLE_ERROR_NOT_FOUND otherwise
 *
 */
ble_error_t ble_gap_get_device_by_addr(const bd_address_t *addr, gap_device_t *gap_device);

/**
 * \brief Get device object by connection index
 *
 * \param [in]  conn_idx        Connection index
 * \param [out] gap_device      Device object
 *
 * \return BLE_STATUS_OK if found, BLE_ERROR_NOT_FOUND otherwise
 *
 */
ble_error_t ble_gap_get_device_by_conn_idx(uint16_t conn_idx, gap_device_t *gap_device);

/**
 * \brief Get bond state of device (by connection index)
 *
 * \param [in]  conn_idx        Connection index
 * \param [out] bonded          Flag specifying if the device is bonded
 *
 * \return result code
 *
 */
ble_error_t ble_gap_is_bonded(uint16_t conn_idx, bool *bonded);

/**
 * \brief Get bond state of device (by address)
 *
 * \param [in]  addr            Device address
 * \param [out] bonded          Flag specifying if the device is bonded
 *
 * \return result code
 *
 */
ble_error_t ble_gap_is_addr_bonded(const bd_address_t *addr, bool *bonded);

#if (dg_configBLE_SKIP_LATENCY_API == 1)
/**
 * \brief Temporarily ignore the connection latency
 *
 * This will allow the specific \p conn_idx connection to wake up on every connection event
 * regardless of the connection latency currently applied.
 *
 * \param [in] conn_idx        Connection index
 * \param [in] enable         Preferred status for the skip latency feature
 */
ble_error_t ble_gap_skip_latency(uint16_t conn_idx, bool enable);
#endif /* (dg_configBLE_SKIP_LATENCY_API == 1) */

/**
 * \brief Set the data length used for TX
 *
 * This function will set the maximum transmit data channel PDU payload length and time depending
 * on the \p conn_idx provided. If \p conn_idx is set to ::BLE_CONN_IDX_INVALID then this API sets
 * the preferred TX data length and time for subsequent connections. If \p conn_idx corresponds to
 * an existing connection, it will set the TX data length and time for the specific connection (and
 * possibly will initiate a Data Length Update procedure as defined in Bluetooth Core v_4.2).
 *
 * \param [in] conn_idx      Connection index (if set to ::BLE_CONN_IDX_INVALID then the API will
 *                           set the preferred data length for new connections)
 * \param [in] tx_length     Length for TX data channel PDU payload in octets
 * \param [in] tx_time       Time for TX data channel PDU payload (if set to 0 it will be
 *                           calculated based on the tx_length (with regard to Bluetooth Core v_4.2)
 *
 * \note The application will receive one of the following events as response to this API:
 *       ::BLE_EVT_GAP_DATA_LENGTH_CHANGED if data length has been changed
 *       ::BLE_EVT_GAP_DATA_LENGTH_SET_FAILED with error code if data length could not be set
 *
 * \note If data length is not changed (i.e. if it is set by application to a value larger than the
 *       peer's previously reported RX length) no event will be sent to application. Even though
 *       ble_gap_data_length_set() be successfully completed, the data length has not changed.
 */
ble_error_t ble_gap_data_length_set(uint16_t conn_idx, uint16_t tx_length, uint16_t tx_time);

/**
 * \brief Resolve a BD address
 *
 * Resolve a BD address using the set of IRKs stored in BLE storage.
 *
 * \param [in] address  BD address to resolve (has to be resolvable private address)
 */
ble_error_t ble_gap_address_resolve(bd_address_t address);

#endif /* BLE_GAP_H_ */
/**
 \}
 \}
 \}
 */
