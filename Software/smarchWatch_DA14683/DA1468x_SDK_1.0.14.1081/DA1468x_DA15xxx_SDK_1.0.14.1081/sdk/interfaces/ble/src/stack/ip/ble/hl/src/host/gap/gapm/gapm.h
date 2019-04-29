/**
 ****************************************************************************************
 *
 * @file gapm.h
 *
 * @brief Generic Access Profile Manager Header.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 ****************************************************************************************
 */


#ifndef _GAPM_H_
#define _GAPM_H_

/**
 ****************************************************************************************
 * @addtogroup GAPM Generic Access Profile Manager
 * @ingroup GAP
 * @brief Generic Access Profile Manager.
 *
 * The GAP Manager module is responsible for providing an API to the application in order
 * to manage all non connected stuff such as configuring device to go in desired mode
 * (discoverable, connectable, etc.) and perform required actions (scanning, connection,
 * etc.). GAP Manager is also responsible of managing GAP Controller state according to
 * corresponding BLE connection states.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#include "ke_task.h"
#include "gap.h"
#include "co_version.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Scan filter size
#define GAPM_SCAN_FILTER_SIZE   rom_cfg_table[gapm_scan_filter_size_pos] //10


/// Operation type
enum gapm_op_type
{
    /// Configuration operation
    GAPM_OP_CFG         = 0x00,

    /// Air mode operation (scanning, advertising, connection establishment)
    /// Note: Restriction, only one air operation supported.
    GAPM_OP_AIR         = 0x01,

    /// Max number of operations
    GAPM_OP_MAX
};


/// Device configuration flags
///    7     6    5    4    3    2    1    0
/// +-----+-----+----+----+----+----+----+----+
/// | DBG | RFU | SC | CP | GA | RE |   ADDR  |
/// +-----+-----+----+----+----+----+----+----+
/// - Bit [0-1]: Address Type (0 = Public, 1 = Private Static, 2 = Privacy Gen Address, @see enum gapm_cfg_flag)
/// - Bit [2]  : Address to renew (only if privacy is enabled, 1 address to renew else 0)
/// - Bit [3]  : Generated Address type (1 = Resolvable, 0 = Non Resolvable)
/// - Bit [4]  : Preferred Connection parameters present in GAP DB
/// - Bit [5]  : Service Change feature present
/// - Bit [6]  : Reserved
/// - Bit [7]  : Enable Debug mode

/// Configuration flag bit description
enum gapm_cfg_flag_def
{
    /// Address Type
    GAPM_MASK_ADDR_TYPE           = 0x03,
    GAPM_POS_ADDR_TYPE            = 0x00,
    /// Address to renew
    GAPM_MASK_ADDR_RENEW          = 0x04,
    GAPM_POS_ADDR_RENEW           = 0x02,
    /// Generated Address type
    GAPM_MASK_RESOLV_ADDR         = 0x08,
    GAPM_POS_RESOLV_ADDR          = 0x03,
    /// Preferred Connection parameters present in GAP DB
    GAPM_MASK_PREF_CON_PAR_PRES   = 0x10,
    GAPM_POS_PREF_CON_PAR_PRES    = 0x04,
    /// Service Change feature present
    GAPM_MASK_SVC_CHG_EN          = 0x20,
    GAPM_POS_SVC_CHG_EN           = 0x05,
#if (RWBLE_SW_VERSION_MAJOR >= 8)
    /// Controller privacy
    GAPM_MASK_CTNL_PRIVACY        = 0x40,
    GAPM_POS_CTNL_PRIVACY         = 0x06,
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

#if (BLE_DEBUG)
    /// Service change feature present in GATT attribute database.
    GAPM_MASK_DBG_MODE_EN         = 0x80,
    GAPM_POS_DBG_MODE_EN          = 0x07,
#endif // (BLE_DEBUG)

#if (RWBLE_SW_VERSION_MAJOR >= 8)
    /// Controller privacy helper
    GAPM_MASK_CTNL_PRIVACY_HELP   = 0x100,
    GAPM_POS_CTNL_PRIVACY_HELP    = 0x08,

    /// Connection oriented zero credit discard
    GAPM_MASK_COC_NO_CREDIT_DISCARD   = 0x200,
    GAPM_POS_COC_NO_CREDIT_DISCARD    = 0x09,
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

};

/*
 * MACROS
 ****************************************************************************************
 */
#if (BLE_ATTS)
/// retrieve gap attribute handle from attribute index.
#define GAPM_GET_ATT_HANDLE(idx)\
    ((gapm_env.svc_start_hdl == 0)? (0) :(gapm_env.svc_start_hdl + (idx)))
#endif // (BLE_ATTS)

/// Macro used to retrieve field
#define GAPM_F_GET(data, field)\
        (((data) & (GAPM_MASK_ ## field)) >> (GAPM_POS_ ## field))

/// Macro used to set field
#define GAPM_F_SET(data, field, val)\
    (data) = (((data) & ~(GAPM_MASK_ ## field)) \
           | ((val << (GAPM_POS_ ## field)) & (GAPM_MASK_ ## field)))

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/// GAP Manager environment structure
struct gapm_env_tag
{
    /// Request operation Kernel message
    void* operation[GAPM_OP_MAX];

    #if (BLE_CENTRAL || BLE_OBSERVER)
    /// Scan filtering Array
    struct gap_bdaddr* scan_filter;
    #endif // (BLE_CENTRAL || BLE_OBSERVER)

    #if (BLE_ATTS)
    /// GAP service start handle
    uint16_t svc_start_hdl;
    #endif // (BLE_ATTS)

    #if (RW_BLE_USE_CRYPT)
    /// Duration before regenerate device address when privacy is enabled.
    uint16_t renew_dur;
    /// Device IRK used for resolvable random BD address generation (MSB -> LSB)
    struct gap_sec_key irk;
    #endif // (RW_BLE_USE_CRYPT)

    /// Current device Address
    struct bd_addr addr;
    /// Device Role
    uint8_t role;
    /// Number of BLE connection
    uint8_t connections;

    /// Device configuration flags - (@see enum gapm_cfg_flag_def)
#if (RWBLE_SW_VERSION_MAJOR >= 8)
    uint16_t cfg_flags;
#else
    uint8_t cfg_flags;
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */
/// GAP Manager environment variable.
extern struct gapm_env_tag gapm_env;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize Generic Access Profile Manager Module.
 *
 * @param[in] reset  true if it's requested by a reset; false if it's boot initialization
 *
 ****************************************************************************************
 */
void gapm_init(bool reset);

/**
 ****************************************************************************************
 * @brief Initialize GAP attribute database
 *
 * @param[in] start_hdl Service Start Handle
 * @param[in] feat      Attribute database features
 *
 * @return status code of attribute database initialization
 *  - @ref ATT_ERR_NO_ERROR: If database creation succeeds.
 *  - @ref ATT_ERR_INVALID_HANDLE: If start_hdl given in parameter + nb of attribute override
 *                            some existing services handles.
 *  - @ref ATT_ERR_INSUFF_RESOURCE: There is not enough memory to allocate service buffer.
 *                           or of new attribute cannot be added because all expected
 *                           attributes already add
 ****************************************************************************************
 */
uint8_t gapm_init_attr(uint16_t start_hdl, uint32_t feat);

/**
 ****************************************************************************************
 * @brief Send operation completed message according to operation type.
 * Perform also am operation clean-up
 *
 * @param[in] op_type Operation type
 * @param[in] status  Command status
 *****************************************************************************************
 */
void gapm_send_complete_evt(uint8_t op_type, uint8_t status);

/**
 ****************************************************************************************
 * @brief Send operation completed message with status error code not related to a
 * running operation.
 *
 * @param[in] operation Operation code
 * @param[in] requester requester of operation
 * @param[in] status    Error status code
 ****************************************************************************************
 */
void gapm_send_error_evt(uint8_t operation, const ke_task_id_t requester, uint8_t status);


/**
 ****************************************************************************************
 * @brief Get operation pointer
 *
 * @param[in] op_type       Operation type.
 *
 * @return operation pointer on going
 ****************************************************************************************
 */
__STATIC_INLINE void* gapm_get_operation_ptr(uint8_t op_type)
{
    ASSERT_ERR(op_type < GAPM_OP_MAX);
    // return operation pointer
    return gapm_env.operation[op_type];
}


/**
 ****************************************************************************************
 * @brief Set operation pointer
 *
 * @param[in] op_type       Operation type.
 * @param[in] op            Operation pointer.
 *
 ****************************************************************************************
 */
__STATIC_INLINE void gapm_set_operation_ptr(uint8_t op_type, void* op)
{
    ASSERT_ERR(op_type < GAPM_OP_MAX);
    // update operation pointer
    gapm_env.operation[op_type] = op;
}


/**
 ****************************************************************************************
 * @brief Check If Service changed feature is enabled or not
 *
 * @return true if enabled, false else.
 *
 ****************************************************************************************
 */
__STATIC_INLINE bool gapm_svc_chg_en(void)
{
    return (GAPM_F_GET(gapm_env.cfg_flags, SVC_CHG_EN) != 0);
}

#if (BLE_DEBUG)
/**
 ****************************************************************************************
 * @brief Check If Debug mode feature is enabled or not
 *
 * @return true if enabled, false else.
 *
 ****************************************************************************************
 */
__STATIC_INLINE bool gapm_dbg_mode_en(void)
{
    return (GAPM_F_GET(gapm_env.cfg_flags, DBG_MODE_EN) != 0);
}

#endif // (BLE_DEBUG)
/**
 ****************************************************************************************
 * @brief Get operation on going
 *
 * @param[in] op_type       Operation type.
 *
 * @return operation code on going
 ****************************************************************************************
 */
uint8_t gapm_get_operation(uint8_t op_type);


/**
 ****************************************************************************************
 * @brief Operation execution not finish, request kernel to reschedule it in order to
 * continue its execution
 *
 * @param[in] op_type       Operation type.
 *
 * @return if operation has been rescheduled (not done if operation pointer is null)
 ****************************************************************************************
 */
bool gapm_reschedule_operation(uint8_t op_type);

/**
 ****************************************************************************************
 * @brief Get requester of on going operation
 *
 * @param[in] op_type       Operation type.
 *
 * @return task that requests to execute the operation
 ****************************************************************************************
 */
ke_task_id_t gapm_get_requester(uint8_t op_type);




#if (BLE_CENTRAL || BLE_PERIPHERAL)
/**
 ****************************************************************************************
 * @brief A connection has been created, initialize host stack to be ready for connection.
 *
 * @param[in] operation  Air operation type
 * @param[in] con_params Connection parameters from lower layers
 *
 * @return Connection index allocated to the new connection.
 ****************************************************************************************
 */
uint8_t gapm_con_create(uint8_t operation, struct hci_le_con_cmp_evt const *con_params);


/**
 ****************************************************************************************
 * @brief Created link connection parameters (from bond data) has been set, connection
 * ready to be used.
 *
 * @param[in] conidx     Connection Index
 *
 ****************************************************************************************
 */
void gapm_con_enable(uint8_t conidx);


/**
 ****************************************************************************************
 * @brief A link has been disconnected, clean-up host stack for this connection.
 *
 * @param[in] conidx     Connection Index
 * @param[in] conhdl     Connection Handle
 * @param[in] reason     Reason of the disconnection
 *
 ****************************************************************************************
 */
void gapm_con_cleanup(uint8_t conidx, uint16_t conhdl, uint8_t reason);

#endif // (BLE_CENTRAL || BLE_PERIPHERAL)
/**
 ****************************************************************************************
 * @brief Retrieve Task Identifier from Task number
 * (automatically update index of task in returned task id)
 *
 * @param task Task number
 * @return Task Identifier
 ****************************************************************************************
 */
ke_task_id_t gapm_get_id_from_task(ke_msg_id_t task);

/**
 ****************************************************************************************
 * @brief Retrieve Task Number from Task Identifier
 * (automatically update index of task in returned task id)
 *
 * @param id Task Identifier
 * @return Task Number
 ****************************************************************************************
 */
ke_task_id_t gapm_get_task_from_id(ke_msg_id_t id);


/**
 ****************************************************************************************
 * Retrieve if current connection index is used for a discovery purpose such as
 * Name discovery
 *
 * @param conidx Index of the specific connection
 *
 * @return true if connection has a discovery purpose, False else
 ****************************************************************************************
 */
bool gapm_is_disc_connection(uint8_t conidx);
/// @} GAPM

#endif /* _GAPM_H_ */
