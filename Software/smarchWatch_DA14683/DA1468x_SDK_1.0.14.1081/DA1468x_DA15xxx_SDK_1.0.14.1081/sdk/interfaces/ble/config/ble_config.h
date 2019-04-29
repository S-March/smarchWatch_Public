/**
 \addtogroup INTERFACES
 \{
 \addtogroup BLE
 \{
 \addtogroup BLE_CONFIG
 *
 * \brief BLE configuration options
 *
 * The following tags are used to describe the type of each configuration option.
 *
 * - **\bsp_config_option_build**       : To be changed only in the build configuration
 *                                        of the project ("Defined symbols -D" in the
 *                                        preprocessor options).
 *
 * - **\bsp_config_option_app**         : To be changed only in the custom_config*.h
 *                                        project files.
 *
 * - **\bsp_config_option_expert_only** : To be changed only by an expert user.
 *
 *\{
 */

/**
 ****************************************************************************************
 *
 * @file ble_config.h
 *
 * @brief BLE configuration options
 *
 * Copyright (C) 2015-2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifndef _BLE_CONFIG_H_
#define _BLE_CONFIG_H_

#include <stdint.h>

/* --------------------------------- BLE configuration options ---------------------------------- */

/**
 * \brief Enable the Observer role in the BLE framework
 *
 * By default, all roles are supported by the BLE framework. However, if the application does not
 * use the Observer role, it can define this macro to 0 in its custom config file to reduce code
 * size.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configBLE_OBSERVER
#define dg_configBLE_OBSERVER                (1)
#endif

/**
 * \brief Enable the Broadcaster role in the BLE framework
 *
 * By default, all roles are supported by the BLE framework. However, if the application does not
 * use the Broadcaster role, it can define this macro to 0 in its custom config file to reduce code
 * size.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configBLE_BROADCASTER
#define dg_configBLE_BROADCASTER             (1)
#endif

/**
 * \brief Enable the Central role in the BLE framework
 *
 * By default, all roles are supported by the BLE framework. However, if the application does not
 * use the Central role, it can define this macro to 0 in its custom config file to reduce code
 * size.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configBLE_CENTRAL
#define dg_configBLE_CENTRAL                 (1)
#endif

/**
 * \brief Enable the Peripheral role in the BLE framework
 *
 * By default, all roles are supported by the BLE framework. However, if the application does not
 * use the Peripheral role, it can define this macro to 0 in its custom config file to reduce code
 * size.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configBLE_PERIPHERAL
#define dg_configBLE_PERIPHERAL              (1)
#endif

/**
 * \brief Enable the GATT Client role in the BLE framework
 *
 * By default, all GATT roles are supported by the BLE framework. However, if the application does
 * not use the GATT Client role, it can define this macro to 0 in its custom config file to reduce
 * code size.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configBLE_GATT_CLIENT
#define dg_configBLE_GATT_CLIENT             (1)
#endif

/**
 * \brief Enable the GATT Server role in the BLE framework
 *
 * By default, all GATT roles are supported by the BLE framework. However, if the application does
 * not use the GATT Server role, it can define this macro to 0 in its custom config file to reduce
 * code size.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configBLE_GATT_SERVER
#define dg_configBLE_GATT_SERVER             (1)
#endif

/**
 * \brief Enable L2CAP CoC (Connection Oriented Channels) in the BLE framework
 *
 * If the application does not use L2CAP CoC, it can define this macro to 0 in its custom config
 * file to reduce code size.
 */
#ifndef dg_configBLE_L2CAP_COC
#define dg_configBLE_L2CAP_COC               (1)
#endif

/*
 * Disable Connection Parameter Request procedure
 *
 * This option is deprecated. Using ble_gap_conn_param_update() with a peer that does
 * not support the 4.1 Connection Parameter Request feature will result in making a
 * connection update request over L2CAP to ensure interoperability.
 */
#ifdef dg_configBLE_CONN_PARAM_REQ_DISABLE
#pragma message "dg_configBLE_CONN_PARAM_REQ_DISABLE macro is deprecated: Defining it will have no effect!"
#endif


/**
 * \brief Enable Event Counters in BLE ISR
 *
 * If the application has not defined #dg_configBLE_EVENT_COUNTER_ENABLE in its custom_config
 * file, this is defined to the default value of 0 to disable the Event Counters in BLE stack ISR.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configBLE_EVENT_COUNTER_ENABLE
#define dg_configBLE_EVENT_COUNTER_ENABLE    (0)
#endif

/**
 * \brief Enable ADV_UNDERRUN workaround
 *
 * If the application has not defined #dg_configBLE_ADV_STOP_DELAY_ENABLE in its custom_config
 * file, this is defined to the default value of 0 to disable the ADV_UNDERRUN workaround in the
 * BLE adapter.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configBLE_ADV_STOP_DELAY_ENABLE
#define dg_configBLE_ADV_STOP_DELAY_ENABLE   (0)
#endif

/**
 * \brief Enable the ble_gap_skip_latency() API.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configBLE_SKIP_LATENCY_API
#define dg_configBLE_SKIP_LATENCY_API        (0)
#endif

/**
 * \brief Enable LE Privacy v1.2 functionality.
 *
 * To use this feature, ble_gap_address_set() should be called using the PRIVATE_CNTL address type.
 * The configuration of the Resolving Address List (RAL) is handled by the BLE Manager and will be used
 * by the Link Layer when advertising, scanning or connection procedures take place. RAL will be
 * automatically updated when new bonds are created or existing bonds are removed.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configBLE_PRIVACY_1_2
#define dg_configBLE_PRIVACY_1_2             (0)
#endif

/**
 * \brief Maximum Receive Data Channel PDU Payload Length
 *
 * If the application has not defined #dg_configBLE_DATA_LENGTH_RX_MAX in its custom_config file,
 * this is defined to the maximum value allowed by Bluetooth Core v_4.2, which is 251 octets.
 *
 * \note This value should be between 27 and 251.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configBLE_DATA_LENGTH_RX_MAX
#define dg_configBLE_DATA_LENGTH_RX_MAX      (251)
#else
#if ((dg_configBLE_DATA_LENGTH_RX_MAX > 251) || (dg_configBLE_DATA_LENGTH_RX_MAX < 27))
#error "dg_configBLE_DATA_LENGTH_RX_MAX value must be between 27 and 251!"
#endif
#endif

/**
 * \brief Maximum Transmit Data Channel PDU Payload Length
 *
 * If the application has not defined #dg_configBLE_DATA_LENGTH_TX_MAX in its custom_config file,
 * this is defined to the maximum value allowed by Bluetooth Core v_4.2, which is 251 octets.
 *
 * \note This value should be between 27 and 251.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configBLE_DATA_LENGTH_TX_MAX
#define dg_configBLE_DATA_LENGTH_TX_MAX      (251)
#else
#if ((dg_configBLE_DATA_LENGTH_TX_MAX > 251) || (dg_configBLE_DATA_LENGTH_TX_MAX < 27))
#error "dg_configBLE_DATA_LENGTH_TX_MAX value must be between 27 and 251!"
#endif
#endif

/*
 * \brief Initiate a data length request upon a new connection as slave
 *
 * Id defined to 1, the device will send an LL_LENGTH_REQ PDU upon a new connection as slave.
 */
#ifndef dg_configBLE_DATA_LENGTH_REQ_UPON_CONN
#define dg_configBLE_DATA_LENGTH_REQ_UPON_CONN  (0)
#endif

/** \brief Convert Receive/Transmit Data Length to Time */
#define BLE_DATA_LENGTH_TO_TIME(OCTETS)  ( ( (OCTETS) + 11 + 3 ) * 8 )

/** \brief Minimum Connection Event Length
 *
 * Minimum length for Connection Event in steps of 0.625ms. This is calculated based on the maximum
 * LE Data Lengths defined for reception and transmission (#dg_configBLE_DATA_LENGTH_RX_MAX and
 * #dg_configBLE_DATA_LENGTH_TX_MAX respectively) plus 150us for the IFS.
 *
 * \note This is used in outgoing connection requests initiated using ble_gap_connect(), and
 *       connection parameter requests and connection updates done using ble_gap_conn_param_update()
 *       and ble_gap_conn_param_update_reply(), if the maximum length for connection events is not
 *       modified by the application using ble_gap_connect_ce().
 *
 * \note This parameter applies only when the device is the master of the connection.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configBLE_CONN_EVENT_LENGTH_MIN
#define dg_configBLE_CONN_EVENT_LENGTH_MIN \
        ( (BLE_DATA_LENGTH_TO_TIME(dg_configBLE_DATA_LENGTH_RX_MAX) \
         + BLE_DATA_LENGTH_TO_TIME(dg_configBLE_DATA_LENGTH_TX_MAX) + 150 + 624) / 625)

#if (dg_configBLE_CONN_EVENT_LENGTH_MIN > 8)
        #undef dg_configBLE_CONN_EVENT_LENGTH_MIN
        #define dg_configBLE_CONN_EVENT_LENGTH_MIN (8)
#endif
#elif defined(dg_configBLE_CONN_EVENT_LENGTH_MAX) \
        && (dg_configBLE_CONN_EVENT_LENGTH_MIN > dg_configBLE_CONN_EVENT_LENGTH_MAX)
#error "dg_configBLE_CONN_EVENT_LENGTH_MIN must be lower or equal to dg_configBLE_CONN_EVENT_LENGTH_MAX."
#endif

/** \brief Maximum Connection Event Length
 *
 * Maximum length for connection events in steps of 0.625ms. This is set to a "don't care" value
 * since the BLE stack uses only #dg_configBLE_CONN_EVENT_LENGTH_MIN to determine the connection
 * event length to use for a given connection.
 *
 * \note This is used in outgoing connection requests initiated using ble_gap_connect(), and
 *       connection parameter requests and connection updates done using ble_gap_conn_param_update()
 *       and ble_gap_conn_param_update_reply(), if the maximum length for connection events is not
 *       modified by the application using ble_gap_connect_ce().
 *
 * \note This parameter applies only when the device is the master of the connection.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configBLE_CONN_EVENT_LENGTH_MAX
#define dg_configBLE_CONN_EVENT_LENGTH_MAX   (0xFFFF)
#endif

/**
 * \brief Duplicate Filtering List Maximum size
 *
 * This defines the size of the list used for duplicate filtering. When the duplicate filtering list
 * is full, additional advertising reports or scan responses will be dropped.
 *
 * \note If the size of the duplicate filtering list is defined to a large number, the BLE stack
 *       heap will have to be adjusted accordingly using #dg_configBLE_STACK_DB_HEAP_SIZE.
 *
 * \note This definition applies only to chip version DA14680/1-01 and later. For DA14680/1-00 the
 *       duplicate filtering list has a fixed size of 10 elements.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configBLE_DUPLICATE_FILTER_MAX
#define dg_configBLE_DUPLICATE_FILTER_MAX    (10)
#elif ((dg_configBLE_DUPLICATE_FILTER_MAX < 10) || (dg_configBLE_DUPLICATE_FILTER_MAX > 255))
#error "dg_configBLE_DUPLICATE_FILTER_MAX value must be between 10 and 255."
#endif

/**
 * \brief Security keys to be distributed by the pairing initiator
 *
 * This defines which security keys will be requested to be distributed by the pairing initiator
 * during a pairing feature exchange procedure.
 *
 * Available keys for distribution:
 * - GAP_KDIST_ENCKEY:  Long Term Key (LTK)
 * - GAP_KDIST_IDKEY:   Identity Resolving Key (IRK)
 * - GAP_KDIST_SIGNKEY: Connection Signature Resolving Key (CSRK)
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configBLE_PAIR_INIT_KEY_DIST
#define dg_configBLE_PAIR_INIT_KEY_DIST      (GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY | GAP_KDIST_SIGNKEY)
#endif

/**
 * \brief Security keys to be distributed by the pairing responder
 *
 * This defines which security keys will be requested to be distributed by the pairing responder
 * during a pairing feature exchange procedure.
 *
 * Available keys for distribution:
 * - GAP_KDIST_ENCKEY:  Long Term Key (LTK)
 * - GAP_KDIST_IDKEY:   Identity Resolving Key (IRK)
 * - GAP_KDIST_SIGNKEY: Connection Signature Resolving Key (CSRK)
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configBLE_PAIR_RESP_KEY_DIST
#define dg_configBLE_PAIR_RESP_KEY_DIST      (GAP_KDIST_ENCKEY | GAP_KDIST_IDKEY | GAP_KDIST_SIGNKEY)
#endif

/**
 * \brief Enable Secure Connections
 *
 * If the application has not defined #dg_configBLE_SECURE_CONNECTIONS in its custom configuration
 * file, this is defined by default to 1 to enable LE Secure Connections.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef dg_configBLE_SECURE_CONNECTIONS
#define dg_configBLE_SECURE_CONNECTIONS      (1)
#endif

/**
 * \brief Pairing successes threshold for renewing the public key
 *
 * If the application has not defined #dg_configBLE_PUB_KEY_SUCCESS_THR in its custom configuration
 * file, this is defined by default to 10 to enable public key renewal after 10 successful pairings.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configBLE_PUB_KEY_SUCCESS_THR
#define dg_configBLE_PUB_KEY_SUCCESS_THR     (10)
#elif ( dg_configBLE_SECURE_CONNECTIONS == 0 )
#pragma message "Public key is renewed only when LE Secure Connections are enabled."
#elif ( RWBLE_SW_VERSION_MINOR < 1)
#pragma message "Public key periodic renewal is not supported in DA14681-01."
#endif

/**
 * \brief Pairing failures threshold for renewing the public key
 *
 * If the application has not defined #dg_configBLE_PUB_KEY_FAILURE_THR in its custom configuration
 * file, this is defined by default to 3 to enable public key renewal after 3 failed pairings.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef dg_configBLE_PUB_KEY_FAILURE_THR
#define dg_configBLE_PUB_KEY_FAILURE_THR     (3)
#elif ( dg_configBLE_SECURE_CONNECTIONS == 0 )
#pragma message "Public key is renewed only when LE Secure Connections are enabled."
#elif ( RWBLE_SW_VERSION_MINOR < 1)
#pragma message "Public key periodic renewal is not supported in DA14681-01."
#endif

/* ---------------------------------- BLE default parameters ------------------------------------ */

/**
 * \brief Default device name (GAP service attribute only)
 *
 * Device Name used for GAP service attribute.
 *
 * \note This is not reflected on advertising data, which have to be changed either using
 *       ble_gap_adv_data_set() or by changing #defaultBLE_ADVERTISE_DATA.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_DEVICE_NAME
#define defaultBLE_DEVICE_NAME               "Dialog BLE"
#endif

/**
 * \brief Default appearance (GAP service attribute only)
 *
 * Appearance used for GAP service attribute.
 *
 * \note This is not reflected on advertising data, which have to be changed either using
 *       ble_gap_adv_data_set() or by changing #defaultBLE_ADVERTISE_DATA.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_APPEARANCE
#define defaultBLE_APPEARANCE                (0)
#endif

/**
 * \brief Default maximum number of simultaneous connections
 *
 * Maximum number of connections that can maintained simultaneously.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_MAX_CONNECTIONS
#define defaultBLE_MAX_CONNECTIONS           (8)
#endif

/**
 * \brief Default maximum number of bonded devices
 *
 * Maximum number of bonded devices for which bonding data can be maintained in persistent storage.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_MAX_BONDED
#define defaultBLE_MAX_BONDED                (8)
#endif

/**
 * \brief Default GAP role
 *
 * Default GAP role set at start-up, if not set otherwise using ble_gap_role_set(),
 * ble_peripheral_start() or ble_central_start()).
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_GAP_ROLE
#define defaultBLE_GAP_ROLE                  GAP_NO_ROLE
#endif

/**
 * \brief Default random address renew duration
 *
 * Default duration for random address generation when a random resolvable or a random
 * non-resolvable address has been set using ble_gap_address_set().
 *
 * \note Value is in steps of 10ms and minimum renew duration that can be set is 150s (values lower
 * than 15000 will be ignored).
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_ADDRESS_RENEW_DURATION
#define defaultBLE_ADDRESS_RENEW_DURATION    (15000)    // 150s (15000 * 10ms).Stack minimum is 150s
#endif

/**
 * \brief Default static BD address
 *
 * Default static BD address set if one is not retrieved from the non-volatile storage.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_STATIC_ADDRESS
#define defaultBLE_STATIC_ADDRESS            { 0x01, 0x00, 0x80, 0xCA, 0xEA, 0x80 }
#endif

/**
 * \brief Default BD address type
 *
 * Default BD address type used if one is not set using ble_gap_address_set().
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_ADDRESS_TYPE
#define defaultBLE_ADDRESS_TYPE              PUBLIC_ADDRESS
#endif

/**
 * \brief Default Identity Resolution Key
 *
 * Default Identity Resolution Key to be used upon IRK exchange.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_IRK
#define defaultBLE_IRK                       { 0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01, \
                                               0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01 }
#endif

/**
 * \brief Default attribute database configuration
 *
 * \verbatim
 * Default attribute database configuration:
 *     7     6    5     4     3    2    1    0
 * +-----+-----+----+-----+-----+----+----+----+
 * | DBG | RFU | SC | PCP | APP_PERM |NAME_PERM|
 * +-----+-----+----+-----+-----+----+----+----+
 * - Bit [0-1]: Device Name write permission requirements for peer device (see gapm_write_att_perm)
 * - Bit [2-3]: Device Appearance write permission requirements for peer device (see gapm_write_att_perm)
 * - Bit [4]  : Slave Preferred Connection Parameters present
 * - Bit [5]  : Service change feature present in GATT attribute database.
 * - Bit [6]  : Reserved
 * - Bit [7]  : Enable Debug Mode
 * \endverbatim
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef defaultBLE_ATT_DB_CONFIGURATION
#define defaultBLE_ATT_DB_CONFIGURATION      (0x10)     // Peripheral Pref. Conn. Parameters present
#endif

/**
 * \brief Maximum MTU size
 *
 * Maximum supported MTU size.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef defaultBLE_MAX_MTU_SIZE
#define defaultBLE_MAX_MTU_SIZE              (512)
#endif

/**
 * \brief Minimum MTU size
 *
 * Minimum supported MTU size as defined by Bluetooth SIG:
 * - 23 when LE Secure Connections are not used.
 * - 65 when LE Secure Connections are used.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef defaultBLE_MIN_MTU_SIZE
#if (dg_configBLE_SECURE_CONNECTIONS == 1)
#define defaultBLE_MIN_MTU_SIZE              (65)
#else
#define defaultBLE_MIN_MTU_SIZE              (23)
#endif
#elif (((dg_configBLE_SECURE_CONNECTIONS == 1) \
                && (defaultBLE_MIN_MTU_SIZE < 65)) \
        || ((dg_configBLE_SECURE_CONNECTIONS == 0) \
                && (defaultBLE_MIN_MTU_SIZE < 23)) \
        || (defaultBLE_MIN_MTU_SIZE > defaultBLE_MAX_MTU_SIZE))
#error "defaultBLE_MIN_MTU_SIZE set out of supported range!"
#endif

/**
 * \brief Default MTU size
 *
 * Default MTU size used on MTU exchange negotiations if one is not set using ble_gap_mtu_size_set().
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_MTU_SIZE
#define defaultBLE_MTU_SIZE                  (defaultBLE_MIN_MTU_SIZE)
#elif ((defaultBLE_MTU_SIZE < defaultBLE_MIN_MTU_SIZE) \
        || (defaultBLE_MTU_SIZE > defaultBLE_MAX_MTU_SIZE))
#error "defaultBLE_MTU_SIZE set out of supported range!"
#endif

/**
 * \brief Default channel map (for central role only)
 *
 * Default channel map used when device is configured with the central role if one is not set using
 * ble_gap_channel_map_set().
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_CHANNEL_MAP
#define defaultBLE_CHANNEL_MAP               {0xFF, 0xFF, 0xFF, 0xFF, 0x1F}  // All channels enabled
#endif

/**
 * \brief Default advertising mode
 *
 * Default mode used for advertising if one is not set using ble_gap_adv_mode_set().
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_ADVERTISE_MODE
#define defaultBLE_ADVERTISE_MODE            GAP_DISC_MODE_GEN_DISCOVERABLE
#endif

/**
 * \brief Default channels used for advertising
 *
 * Default channel used for advertising if they are not set using ble_gap_adv_chnl_map_set().
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_ADVERTISE_CHANNEL_MAP
#define defaultBLE_ADVERTISE_CHANNEL_MAP     (GAP_ADV_CHANNEL_37 | GAP_ADV_CHANNEL_38 | GAP_ADV_CHANNEL_39)
#endif

/**
 * \brief Default minimum interval used for advertising
 *
 * Default minimum interval used for advertising in steps of 0.625ms if one is not set using
 * ble_gap_adv_intv_set().
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_ADVERTISE_INTERVAL_MIN
#define defaultBLE_ADVERTISE_INTERVAL_MIN    (BLE_ADV_INTERVAL_FROM_MS(687.5))  // 687.5ms
#endif

/**
 * \brief Default maximum interval used for advertising
 *
 * Default maximum interval used for advertising in steps of 0.625ms if one is not set using
 * ble_gap_adv_intv_set().
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_ADVERTISE_INTERVAL_MAX
#define defaultBLE_ADVERTISE_INTERVAL_MAX    (BLE_ADV_INTERVAL_FROM_MS(687.5))  // 687.5ms
#endif

/**
 * \brief Default filtering policy used for advertising
 *
 * Default filtering policy used for advertising if one is not set using ble_gap_adv_filt_policy_set().
 *
 * \note Whitelist management API is not present in this release, so setting a filtering policy for
 *       advertising is not possible.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_ADVERTISE_FILTER_POLICY
#define defaultBLE_ADVERTISE_FILTER_POLICY   ADV_ALLOW_SCAN_ANY_CONN_ANY
#endif

/**
 * \brief Default advertising data length
 *
 * Default length of advertising data. This is set to the maximum value allowed by the stack, which
 * is 28 bytes.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_ADVERTISE_DATA_LENGTH
#define defaultBLE_ADVERTISE_DATA_LENGTH     (28)
#endif

/**
 * \brief Default advertising Data
 *
 * Default advertising data are set to advertise the device name. If the application
 * should have specific advertising data, these should be set using the ble_gap_adv_data_set().
 *
 * \note Changing #defaultBLE_DEVICE_NAME won't change the device name included by default in the
 *       advertising data.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_ADVERTISE_DATA
#define defaultBLE_ADVERTISE_DATA            { 0x0C, GAP_DATA_TYPE_LOCAL_NAME, \
                                               'D', 'i', 'a', 'l', 'o', 'g', ' ', 'B', 'L', 'E' }
#endif

/**
 * \brief Default scan response data length
 *
 * Default length of scan response data. This is set to the maximum value allowed by the stack,
 * which is 31 bytes.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_SCAN_RESPONSE_DATA_LENGTH
#define defaultBLE_SCAN_RESPONSE_DATA_LENGTH (31)
#endif

/**
 * \brief Default scan response Data
 *
 * Default scan response data are set to zero. If the application should have specific scan response
 * data, these should be set using the ble_gap_adv_data_set().
 *
 * \note Changing #defaultBLE_DEVICE_NAME won't change the device name included by default in the
 *       scan response data.
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_SCAN_RESPONSE_DATA
#define defaultBLE_SCAN_RESPONSE_DATA        { }
#endif

/**
 * \brief Default scan interval
 *
 * Default interval used for scanning in steps of 0.625ms if one is not set using
 * ble_gap_scan_params_set().
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef defaultBLE_SCAN_INTERVAL
#define defaultBLE_SCAN_INTERVAL             (BLE_SCAN_INTERVAL_FROM_MS(100))   // 100ms
#endif

/**
 * \brief Default scan window
 *
 * Default window used for scanning in steps of 0.625ms if one is not set using
 * ble_gap_scan_params_set().
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef defaultBLE_SCAN_WINDOW
#define defaultBLE_SCAN_WINDOW               (BLE_SCAN_WINDOW_FROM_MS(50))      // 50ms
#endif

/**
 * \brief Default peripheral preferred minimum connection interval
 *
 * Default minimum connection interval set in the peripheral preferred connection parameters
 * attribute in steps of 1.25ms if one is not set using ble_gap_per_pref_conn_params_set().
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef defaultBLE_PPCP_INTERVAL_MIN
#define defaultBLE_PPCP_INTERVAL_MIN         (BLE_CONN_INTERVAL_FROM_MS(10))    // 10ms
#endif

/**
 * \brief Default peripheral preferred maximum connection interval
 *
 * Default maximum connection interval set in the peripheral preferred connection parameters
 * attribute in steps of 1.25ms if one is not set using ble_gap_per_pref_conn_params_set().
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef defaultBLE_PPCP_INTERVAL_MAX
#define defaultBLE_PPCP_INTERVAL_MAX         (BLE_CONN_INTERVAL_FROM_MS(20))    // 20ms
#endif

/**
 * \brief Default peripheral preferred slave latency
 *
 * Default slave latency set in the peripheral preferred connection parameters attribute in number
 * of events if one is not set using ble_gap_per_pref_conn_params_set().
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef defaultBLE_PPCP_SLAVE_LATENCY
#define defaultBLE_PPCP_SLAVE_LATENCY        (0)          // number of events
#endif

/**
 * \brief Default peripheral preferred supervision timeout
 *
 * Default supervision timeout set in the peripheral preferred connection parameters attribute in
 * steps of 10ms if one is not set using ble_gap_per_pref_conn_params_set().
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef defaultBLE_PPCP_SUP_TIMEOUT
#define defaultBLE_PPCP_SUP_TIMEOUT          (BLE_SUPERVISION_TMO_FROM_MS(1000)) // 1s
#endif

/**
 * \brief Default Input/Output capabilities
 *
 * Default input/output capabilities set for pairing procedures if they are not set using the
 * ble_gap_set_io_cap().
 *
 * \bsp_default_note{\bsp_config_option_app,}
 */
#ifndef defaultBLE_GAP_IO_CAP
#define defaultBLE_GAP_IO_CAP                GAP_IO_CAP_NO_INPUT_OUTPUT
#endif

/**
 * \brief Use passthrough mode
 *
 * If application has enabled the external host configuration, BLE stack is configured for
 * pass-through mode.
 *
 */
#ifdef BLE_EXTERNAL_HOST
#define BLE_STACK_PASSTHROUGH_MODE
#define BLE_MGR_DIRECT_ACCESS   0
#endif

/**
 * \brief Use BLE sleep mode
 *
 * Macro #USE_BLE_SLEEP controls whether BLE will be set to sleep when it is not needed to be
 * active.
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifdef BLE_PROD_TEST
#define USE_BLE_SLEEP           0
#else
#define USE_BLE_SLEEP           1
#endif

/**
 * \brief Wake Up Latency
 *
 * Defines the Wake Up Latency expressed in Low Power clock cycles, that is
 * the number of LP clock cycles needed for the BLE to be fully operational
 * (calculations and BLE timer synchronization)
 *
 * \bsp_default_note{\bsp_config_option_app, \bsp_config_option_expert_only}
 */
#ifndef BLE_WUP_LATENCY
#if ((dg_configUSE_LP_CLK == LP_CLK_32000) || (dg_configUSE_LP_CLK == LP_CLK_32768))
#ifdef RELEASE_BUILD
        #if (dg_configCODE_LOCATION != NON_VOLATILE_IS_FLASH)   // RAM, OTP
                #define BLE_WUP_LATENCY        8
        #else                                                   // Flash
                #define BLE_WUP_LATENCY        9
        #endif
#else                                                           // !RELEASE_BUILD
        #if (dg_configCODE_LOCATION != NON_VOLATILE_IS_FLASH)
                #define BLE_WUP_LATENCY        16
        #else
                #define BLE_WUP_LATENCY        32
        #endif
#endif // RELEASE_BUILD

#elif (dg_configUSE_LP_CLK == LP_CLK_RCX) // RCX

#ifdef RELEASE_BUILD
        #if (dg_configCODE_LOCATION != NON_VOLATILE_IS_FLASH)
                #define BLE_WUP_LATENCY        cm_rcx_us_2_lpcycles(300)
        #else
                #define BLE_WUP_LATENCY        cm_rcx_us_2_lpcycles(300)
        #endif
#else // !RELEASE_BUILD
        #if (dg_configCODE_LOCATION != NON_VOLATILE_IS_FLASH)
                #define BLE_WUP_LATENCY        cm_rcx_us_2_lpcycles(500)
        #else
                #define BLE_WUP_LATENCY        cm_rcx_us_2_lpcycles(1000)
        #endif
#endif // RELEASE_BUILD

#else /* LP_CLK_ANY */
        // Must be defined in the custom_config_<>.h file.
#endif
#endif

/**
 * \brief BLE code hooks
 *
 * \details The BLE code provides the option to execute user specific code at specific
 *        instances. The mechanism works as follows: The BLE code keeps pointers to hooks. These
 *        pointers are initialized by default to the value of certain entries of the Jump Table for
 *        Functions (rom_func_addr_table_var[], entries 71-86|60-75). These entries, by default,
 *        contain the addresses of the functions that implement the functionality of each hook.
 *        Thus, if none of these functions is overridden by the application code, all these entries
 *        are initialized to NULL. If a specific hook needs to be overridden by the application then
 *        the specific function has to be defined in its code (i.e. define my_custom_pti_set() via
 *        the dg_configBLE_HOOK_PTI_MODIFY). Then, the address of this function will be set in the
 *        Jump Table and the BLE hook variable (custom_pti_set in this example) will be initialized
 *        accordingly.
 *        This is the "static" mode of this mechanism. There is also a "dynamic" mode. If, for any
 *        reason, a hook needs to be added/changed dynamically then the Jump Table may be
 *        initialized with the "default" hook and the application code can modify the value of the
 *        BLE hook variable directly.
 *
 *        Please note that only one such pointer to a hook is used. The rest are deprecated since
 *        the BLE code provides this functionality via the hooks and the events defined in the
 *        bsp_defaults.h file.
 */
extern unsigned char (*custom_pti_set)(void);   /**< pointer to function called when pti_setf macro
                                                        is called in order to update the priority
                                                        used by the arbiter */



/* ------------------------------- DO NOT ALTER AFTER THIS POINT -------------------------------- */

#ifdef dg_configBLE_HOOK_BLOCK_SLEEP
unsigned char dg_configBLE_HOOK_BLOCK_SLEEP(void);
#endif

#ifdef dg_configBLE_HOOK_PTI_MODIFY
unsigned char dg_configBLE_HOOK_PTI_MODIFY(void);
#endif

#endif /* _BLE_CONFIG_H_ */
/**
 \}
 \}
 \}
 */

