/**
 \addtogroup INTERFACES
 \{
 \addtogroup BLE
 \{
 \brief BLE Wireless 
 \addtogroup API
 \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_common.h
 *
 * @brief Common definitions for BLE API
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BLE_COMMON_H_
#define BLE_COMMON_H_

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/**
 * Notification bitmask for event queue
 */
#define BLE_APP_NOTIFY_MASK (1 << 0)

/** BLE error code */
typedef enum {
        BLE_STATUS_OK                   = 0x00,    /**< Success */
        BLE_ERROR_FAILED                = 0x01,    /**< Generic failure */
        BLE_ERROR_ALREADY_DONE          = 0x02,    /**< Already done */
        BLE_ERROR_IN_PROGRESS           = 0x03,    /**< Operation already in progress */
        BLE_ERROR_INVALID_PARAM         = 0x04,    /**< Invalid parameter */
        BLE_ERROR_NOT_ALLOWED           = 0x05,    /**< Not allowed */
        BLE_ERROR_NOT_CONNECTED         = 0x06,    /**< Not connected */
        BLE_ERROR_NOT_SUPPORTED         = 0x07,    /**< Not supported */
        BLE_ERROR_NOT_ACCEPTED          = 0x08,    /**< Not accepted */
        BLE_ERROR_BUSY                  = 0x09,    /**< Busy */
        BLE_ERROR_TIMEOUT               = 0x0A,    /**< Request timed out */
        BLE_ERROR_NOT_SUPPORTED_BY_PEER = 0x0B,    /**< Not supported by peer */
        BLE_ERROR_CANCELED              = 0x0C,    /**< Canceled by user */
        BLE_ERROR_ENC_KEY_MISSING       = 0x0D,    /**< encryption key missing */
        BLE_ERROR_INS_RESOURCES         = 0x0E,    /**< insufficient resources */
        BLE_ERROR_NOT_FOUND             = 0x0F,    /**< not found */
        BLE_ERROR_L2CAP_NO_CREDITS      = 0x10,    /**< no credits available on L2CAP CoC */
        BLE_ERROR_L2CAP_MTU_EXCEEDED    = 0x11,    /**< MTU exceeded on L2CAP CoC */
        BLE_ERROR_INS_BANDWIDTH         = 0x12,    /**< Insufficient bandwidth */
} ble_error_t;

/** BLE HCI error code */
typedef enum {
        BLE_HCI_ERROR_NO_ERROR                      = 0x00, /**< Success */
        BLE_HCI_ERROR_UNKNOWN_HCI_COMMAND           = 0x01, /**< Unknown HCI command */
        BLE_HCI_ERROR_UNKNOWN_CONNECTION_ID         = 0x02, /**< Unknown connection identifier */
        BLE_HCI_ERROR_HARDWARE_FAILURE              = 0x03, /**< Hardware failure */
        BLE_HCI_ERROR_PAGE_TIMEOUT                  = 0x04, /**< Page timeout */
        BLE_HCI_ERROR_AUTH_FAILURE                  = 0x05, /**< Authentication failure */
        BLE_HCI_ERROR_PIN_MISSING                   = 0x06, /**< PIN or key missing */
        BLE_HCI_ERROR_MEMORY_CAPA_EXCEED            = 0x07, /**< Memory capacity exceeded */
        BLE_HCI_ERROR_CON_TIMEOUT                   = 0x08, /**< Connection timeout */
        BLE_HCI_ERROR_CON_LIMIT_EXCEED              = 0x09, /**< Connection limit exceeded */
        BLE_HCI_ERROR_SYNC_CON_LIMIT_DEV_EXCEED     = 0x0A, /**< Synchronous connection limit to a device exceeded */
        BLE_HCI_ERROR_ACL_CON_EXISTS                = 0x0B, /**< ACL connection already exists */
        BLE_HCI_ERROR_COMMAND_DISALLOWED            = 0x0C, /**< Command disallowed */
        BLE_HCI_ERROR_CONN_REJ_LIMITED_RESOURCES    = 0x0D, /**< Connection rejected due to limited resources */
        BLE_HCI_ERROR_CONN_REJ_SECURITY_REASONS     = 0x0E, /**< Connection rejected due to security reasons */
        BLE_HCI_ERROR_CONN_REJ_UNACCEPTABLE_BDADDR  = 0x0F, /**< Connection rejected due to unacceptable BD_ADDR */
        BLE_HCI_ERROR_CONN_ACCEPT_TIMEOUT_EXCEED    = 0x10, /**< Connection accept timeout exceeded */
        BLE_HCI_ERROR_UNSUPPORTED                   = 0x11, /**< Unsupported feature or parameter value */
        BLE_HCI_ERROR_INVALID_HCI_PARAM             = 0x12, /**< Invalid HCI command parameters */
        BLE_HCI_ERROR_REMOTE_USER_TERM_CON          = 0x13, /**< Remote User terminated connection */
        BLE_HCI_ERROR_REMOTE_DEV_TERM_LOW_RESOURCES = 0x14, /**< Remote device terminated connection due to low resources */
        BLE_HCI_ERROR_REMOTE_DEV_POWER_OFF          = 0x15, /**< Remote device terminated connection due to power off */
        BLE_HCI_ERROR_CON_TERM_BY_LOCAL_HOST        = 0x16, /**< Connection terminated by local host */
        BLE_HCI_ERROR_REPEATED_ATTEMPTS             = 0x17, /**< Repeated attempts */
        BLE_HCI_ERROR_PAIRING_NOT_ALLOWED           = 0x18, /**< Pairing not allowed */
        BLE_HCI_ERROR_UNKNOWN_LMP_PDU               = 0x19, /**< Unknown LMP PDU */
        BLE_HCI_ERROR_UNSUPPORTED_REMOTE_FEATURE    = 0x1A, /**< Unsupported remote feature / Unsupported LMP feature */
        BLE_HCI_ERROR_SCO_OFFSET_REJECTED           = 0x1B, /**< SCO offset rejected */
        BLE_HCI_ERROR_SCO_INTERVAL_REJECTED         = 0x1C, /**< SCO interval rejected */
        BLE_HCI_ERROR_SCO_AIR_MODE_REJECTED         = 0x1D, /**< SCO air mode rejected */
        BLE_HCI_ERROR_INVALID_LMP_PARAM             = 0x1E, /**< Invalid LMP parameters / Invalid LL parameters */
        BLE_HCI_ERROR_UNSPECIFIED_ERROR             = 0x1F, /**< Unspecified error */
        BLE_HCI_ERROR_UNSUPPORTED_LMP_PARAM_VALUE   = 0x20, /**< Unsupported LMP parameter value / Unsupported LL parameter value */
        BLE_HCI_ERROR_ROLE_CHANGE_NOT_ALLOWED       = 0x21, /**< Role change not allowed */
        BLE_HCI_ERROR_LMP_RSP_TIMEOUT               = 0x22, /**< LMP response timeout / LL response timeout */
        BLE_HCI_ERROR_LMP_COLLISION                 = 0x23, /**< LMP error transaction collision */
        BLE_HCI_ERROR_LMP_PDU_NOT_ALLOWED           = 0x24, /**< LMP PDU not allowed */
        BLE_HCI_ERROR_ENC_MODE_NOT_ACCEPT           = 0x25, /**< Encryption mode not acceptable */
        BLE_HCI_ERROR_LINK_KEY_CANT_CHANGE          = 0x26, /**< Link key cannot be changed */
        BLE_HCI_ERROR_QOS_NOT_SUPPORTED             = 0x27, /**< Requested QoS not supported */
        BLE_HCI_ERROR_INSTANT_PASSED                = 0x28, /**< Instant passed */
        BLE_HCI_ERROR_PAIRING_WITH_UNIT_KEY_NOT_SUP = 0x29, /**< Pairing with unit key not supported */
        BLE_HCI_ERROR_DIFF_TRANSACTION_COLLISION    = 0x2A, /**< Different transaction collision */
        BLE_HCI_ERROR_QOS_UNACCEPTABLE_PARAM        = 0x2C, /**< QoS unacceptable parameter */
        BLE_HCI_ERROR_QOS_REJECTED                  = 0x2D, /**< QoS rejected */
        BLE_HCI_ERROR_CHANNEL_CLASS_NOT_SUP         = 0x2E, /**< Channel classification not supported */
        BLE_HCI_ERROR_INSUFFICIENT_SECURITY         = 0x2F, /**< Insufficient security */
        BLE_HCI_ERROR_PARAM_OUT_OF_MAND_RANGE       = 0x30, /**< Parameter out of mandatory range */
        BLE_HCI_ERROR_ROLE_SWITCH_PEND              = 0x32, /**< Role switch pending */
        BLE_HCI_ERROR_RESERVED_SLOT_VIOLATION       = 0x34, /**< Reserved slot violation */
        BLE_HCI_ERROR_ROLE_SWITCH_FAIL              = 0x35, /**< Role switch failed */
        BLE_HCI_ERROR_EIR_TOO_LARGE                 = 0x36, /**< Extended inquiry response too large */
        BLE_HCI_ERROR_SP_NOT_SUPPORTED_HOST         = 0x37, /**< Secure simple pairing not supported by host */
        BLE_HCI_ERROR_HOST_BUSY_PAIRING             = 0x38, /**< Host busy - pairing */
        BLE_HCI_ERROR_CONN_REJ_NO_SUITABLE_CHANNEL  = 0x39, /**< Connection rejected due to no suitable channel found */
        BLE_HCI_ERROR_CONTROLLER_BUSY               = 0x3A, /**< Controller busy */
        BLE_HCI_ERROR_UNACCEPTABLE_CONN_INT         = 0x3B, /**< Unacceptable connection parameters */
        BLE_HCI_ERROR_DIRECT_ADV_TO                 = 0x3C, /**< Directed advertising timeout */
        BLE_HCI_ERROR_TERMINATED_MIC_FAILURE        = 0x3D, /**< Connection terminated due to MIC failure */
        BLE_HCI_ERROR_CONN_FAILED_TO_BE_EST         = 0x3E, /**< Connection failed to be established */
        BLE_HCI_ERROR_MAC_CONNECTION_FAILED         = 0x3F, /**< MAC connection failed */
        BLE_HCI_ERROR_COARSE_CLK_ADJUST_REJECTED    = 0x40, /**< Coarse clock adjustment rejected but will try to adjust using clock dragging */
} ble_hci_error_t;

/** BLE event categories */
enum ble_evt_cat {
        BLE_EVT_CAT_COMMON,
        BLE_EVT_CAT_GAP,
        BLE_EVT_CAT_GATTS,
        BLE_EVT_CAT_GATTC,
        BLE_EVT_CAT_L2CAP,
};

/** Defines first event id in category */
#define BLE_EVT_CAT_FIRST(CAT) (CAT << 8)

/** Common header for BLE events */
typedef struct {
        uint16_t  evt_code;
        uint16_t  length;
} ble_evt_hdr_t;

/** BLE stack status */
typedef enum {
        BLE_IS_DISABLED = 0x00,
        BLE_IS_ENABLED  = 0x01,
} ble_status_t;

/** Bluetooth Address type */
typedef enum addr_types {
        PUBLIC_ADDRESS      = 0x00,    /**< Public Static Address */
        PRIVATE_ADDRESS     = 0x01,    /**< Private Random Address */
} addr_type_t;

/** Own Device Address type */
typedef enum own_addr_types {
        PUBLIC_STATIC_ADDRESS,                   /**< Public Static Address */
        PRIVATE_STATIC_ADDRESS,                  /**< Private Static Address */
        PRIVATE_RANDOM_RESOLVABLE_ADDRESS,       /**< Private Random Resolvable Address */
        PRIVATE_RANDOM_NONRESOLVABLE_ADDRESS,    /**< Private Random Non-resolvable Address */
#if (dg_configBLE_PRIVACY_1_2 == 1)
        PRIVATE_CNTL                             /**< Private Random Resolvable address using LE privacy v1.2 */
#endif /* (dg_configBLE_PRIVACY_1_2 == 1) */
} own_addr_type_t;

/** Bluetooth Device address */
typedef struct bd_address {
        addr_type_t  addr_type;
        uint8_t      addr[6];
} bd_address_t;

/** Own Bluetooth Device address */
typedef struct own_address {
        own_addr_type_t  addr_type;
        uint8_t          addr[6];
} own_address_t;

/** TX Power Level */
typedef enum {
        TX_POWER_LEVEL_MAX     = 0x00,
        TX_POWER_LEVEL_CURRENT = 0x01,
} tx_power_level_type_t;

/** Identity Resolving Key */
typedef struct {
        uint8_t key[16];
} irk_t;

/**
 * \brief Register application in BLE framework
 *
 * \return status code
 *
 */
ble_error_t ble_register_app(void);

/**
 * \brief Enable BLE module
 *
 * \return status code
 *
 */
ble_error_t ble_enable(void);

/**
 * \brief Reset BLE module
 *
 * \return status code
 *
 */
ble_error_t ble_reset(void);

/**
 * \brief Start the BLE module as a central device
 *
 * \return status code
 *
 */
ble_error_t ble_central_start(void);

/**
 * \brief Start the BLE module as a peripheral device
 *
 * \return status code
 *
 */
ble_error_t ble_peripheral_start(void);

/**
 * \brief Get event from BLE event queue
 *
 * \param [in] wait  if true, function will block until there is event in queue
 *
 * \return event buffer or NULL if no event was retrieved
 *
 */
ble_evt_hdr_t *ble_get_event(bool wait);

/**
 * \brief Checks if there's event pending in event queue
 *
 * \return true if there is at least one event waiting in queue, false otherwise
 */
bool ble_has_event(void);

/**
 * \brief Execute default event handler
 *
 * It's recommended for application to call this for any event it does not handle. This avoids
 * situation when BLE stack is waiting for response on event which application does not handle.
 *
 * \param [in] hdr   event buffer
 *
 */
void ble_handle_event_default(ble_evt_hdr_t *hdr);

/**
 * \brief Read controller TX power
 *
 * \param [in]     conn_idx   connection index
 * \param [in]     type       type of TX power
 * \param [out]    tx_power   returned tx_power in case of success
 *
 * \return status code
 */
ble_error_t ble_read_tx_power(uint16_t conn_idx, tx_power_level_type_t type, uint8_t *tx_power);

#if (dg_configBLE_EVENT_NOTIF_TYPE == BLE_EVENT_NOTIF_USER_TASK) && \
        (dg_configBLE_EVENT_NOTIF_RUNTIME_CONTROL == 1)

/**
 * \brief Enable notifications for the BLE End Event
 */
void ble_event_notif_enable_end_event(void);

/**
 * \brief Enable notifications for the BLE CSCNT Event
 */
void ble_event_notif_enable_cscnt_event(void);

/**
 * \brief Enable notifications for the BLE FINE Event
 */
void ble_event_notif_enable_fine_event(void);

/**
 * \brief Disable notifications for the BLE End Event
 */
void ble_event_notif_disable_end_event(void);

/**
 * \brief Disable notifications for the BLE CSCNT Event
 */
void ble_event_notif_disable_cscnt_event(void);

/**
 * \brief Disable notifications for the BLE FINE Event
 */
void ble_event_notif_disable_fine_event(void);

#endif

/**
 * \brief Compare two BD Addresses
 *
 * \param [in] addr1  1st address
 * \param [in] addr2  2nd address
 *
 * \return true if \p addr1 and \p addr2 are the same, false otherwise
 *
 */
static inline bool ble_address_cmp(const bd_address_t *addr1, const bd_address_t *addr2)
{
        return (addr1->addr_type == addr2->addr_type &&
                                        !memcmp(addr1->addr, addr2->addr, sizeof(addr2->addr)));
}

/**
 * \brief Convert bd_address to string
 *
 * \note The returned pointer should not be freed since it points to internal static buffer.
 *
 * \param [in]     address    address to convert
 *
 * \return converted address as a string
 *
 */
const char *ble_address_to_string(const bd_address_t *address);

/**
 * \brief Convert string to bd_address
 *
 * \param [in]     str        address as a string
 * \param [in]     addr_type  address type
 * \param [out]    address    converted address
 *
 * \return converted address as a bd_address
 *
 */
bool ble_address_from_string(const char *str, addr_type_t addr_type, bd_address_t *address);

/**
 * \brief Set FEM Voltage GPIO values for a specific channel
 *
 * This sets the values of the 3 FEM voltage trim GPIOs. Bit X, X in[0, 2] of this value
 * corresponds to the desired state for the respective GPIO.
 *
 * \note The 3 GPIOs must be configured through the FEM driver, using the
 * dg_configFEM_SKY66112_11_ANT_TRIM_X_PORT/PIN, X in [0, 2] macros for this to work.
 *
 * \param [in]     channel    The BLE physical channel to configure the GPIOs for, as defined
 *                            by channel = (freq - 2402)/2, where freq is the channel frequency.
 * \param [in]     value      The 3-bit value to be configured
 *
 * \return status code
 */
ble_error_t ble_set_fem_voltage_trim(uint8_t channel, uint8_t value);


#endif /* BLE_COMMON_H_ */
/**
 \}
 \}
 \}
 */
