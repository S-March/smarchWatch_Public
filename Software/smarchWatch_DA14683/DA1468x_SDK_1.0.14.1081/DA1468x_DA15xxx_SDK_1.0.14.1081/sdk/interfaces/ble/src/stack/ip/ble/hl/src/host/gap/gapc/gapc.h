/**
 ****************************************************************************************
 *
 * @file gapc.h
 *
 * @brief Generic Access Profile Controller Header.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 ****************************************************************************************
 */


#ifndef _GAPC_H_
#define _GAPC_H_

/**
 ****************************************************************************************
 * @addtogroup GAPC Generic Access Profile Controller
 * @ingroup GAP
 * @brief  Generic Access Profile Controller.
 *
 * The GAP Controller module is responsible for providing an API to the application in
 * to perform GAP action related to a BLE connection (pairing, update parameters,
 * disconnect ...). GAP controller is multi-instantiated, one task instance per BLE
 * connection.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#include "co_version.h"

#if (BLE_CENTRAL || BLE_PERIPHERAL)

#include "ke_task.h"
#include "ke_mem.h"
#include "gap.h"
#include "gapc_task.h"
#include "smpc.h"



/*
 * DEFINES
 ****************************************************************************************
 */


/// Link security status. This status represents the authentication/authorization/bonding levels of the connection
enum gapc_lk_sec_req
{
    /// No security requirements on current link
    GAPC_LK_SEC_NONE,
    /// Link is unauthenticated
    GAPC_LK_UNAUTHENTICATED,
    /// Link is authenticated
    GAPC_LK_AUTHENTICATED,
    /// Link is bonded
    GAPC_LK_BONDED,
    /// Link is Encrypted
    GAPC_LK_ENCRYPTED,
#if (RWBLE_SW_VERSION_MAJOR >= 8)
    /// Link has secure connection
    GAPC_LK_SECURE,
#if (RWBLE_SW_VERSION_MINOR >= 1)
    /// Link has LTK
    GAPC_LK_LTK
#endif /* (RWBLE_SW_VERSION_MINOR >= 1) */
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
};


/// fields definitions.
enum gapc_fields
{
    /// Local connection role
    GAPC_ROLE         = 0,
    /// Encrypted connection or not
    GAPC_ENCRYPTED    = 1,
    /// Authentication informations
    GAPC_AUTH         = 2,
    /// Service Changed CCC configuration
#if (RWBLE_SW_VERSION_MAJOR >= 8)
    GAPC_SVC_CHG_CCC  = 6,
#if (RWBLE_SW_VERSION_MINOR >= 1)
    /// LTK present
    GAPC_LTK          = 7,
#endif /* (RWBLE_SW_VERSION_MINOR >= 1) */
#else
    GAPC_SVC_CHG_CCC  = 5,
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
};


/// fields mask definitions.
enum gapc_fields_mask
{
    /// Bit[0]
    GAPC_ROLE_MASK        = 0x01,
    /// Bit[1]
    GAPC_ENCRYPTED_MASK   = 0x02,
#if (RWBLE_SW_VERSION_MAJOR >= 8)
    /// Bit[5-2]
    GAPC_AUTH_MASK        = 0x3C,

    /// Bit[6]
    GAPC_SVC_CHG_CCC_MASK = 0x40,
#if (RWBLE_SW_VERSION_MINOR >= 1)
    /// Bit[7]
    GAPC_LTK_MASK         = 0x80,
#endif /* (RWBLE_SW_VERSION_MINOR >= 1) */
#else
    /// Bit[4-2]
    GAPC_AUTH_MASK        = 0x1C,

    /// Bit[5]
    GAPC_SVC_CHG_CCC_MASK = 0x20,
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
};


/// fields mask definitions.
enum gapc_env_values
{
    /// Source credit
    GAPC_SRC_CREDIT,
    /// destination credit
    GAPC_DEST_CREDIT,
    /// LE Protocol Service Multiplexer
    GAPC_LEPSM,
    /// source channel ID
    GAPC_SRC_CID,
    /// destination channel ID
    GAPC_DEST_CID,
    /// Maximum transfer unit
    GAPC_MTU,
    /// Maximum packet size
    GAPC_MPS,
    /// Task ID
    GAPC_TASK_ID,
};


/*
 * MACRO DEFINITIONS
 *********************************GET*******************************************************
 */
/// Set link configuration field
#define GAPC_SET_FIELD(conidx, field, value)\
    (gapc_env[conidx]->fields) = ((gapc_env[conidx]->fields) & (~GAPC_##field##_MASK)) \
                                     | (((value) << GAPC_##field) & (GAPC_##field##_MASK))


/// Get link configuration field
#define GAPC_GET_FIELD(conidx, field)\
    (((gapc_env[conidx]->fields) & (GAPC_##field##_MASK)) >> GAPC_##field)


/// Check if channel ID is within the correct range
#define L2C_IS_DYNAMIC_CID(cid) ((cid >= L2C_CID_DYN_MIN) && (cid <= L2C_CID_DYN_MAX))


/// Check if LE PSM is within the correct range
#define L2C_IS_VALID_LEPSM(lepsm) (lepsm != L2C_LEPSM_RESERVED)

/// Maximun credit
#define LECB_MAX_CREDIT     0xFFFF

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// GAP controller environment variable structure.
struct gapc_env_tag
{
    /// Request operation Kernel message
    void* operation[GAPC_OP_MAX];
    /// Task id requested disconnection
    ke_task_id_t disc_requester;


    /* Connection parameters to keep */

    #if (RW_BLE_USE_CRYPT)
    /// Security Management Protocol environment variables
    struct smpc_env smpc;
    #endif // (RW_BLE_USE_CRYPT)

    // List for credit based connections
    struct co_list cb_con_list;

    /// connection handle
    uint16_t conhdl;

    /// Configuration fields:
    ///
    /// RWBLE_SW_VERSION_MINOR < 1:
    ///   7   6   5   4   3   2   1   0
    /// +---+---+---+---+---+---+---+---+
    /// |  RFU  | SC|    AUTH   | E | R |
    /// +---+---+---+---+---+---+---+---+
    ///
    /// RWBLE_SW_VERSION_MINOR >= 1:
    ///   7   6   5   4   3   2   1   0
    /// +---+---+---+---+---+---+---+---+
    /// |LTK| SC|    AUTH       | E | R |
    /// +---+---+---+---+---+---+---+---+
    uint8_t fields;

    // BD Address used for the link that should be kept
    struct gap_bdaddr src[SMPC_INFO_MAX];

    /// Relevant information of peer LE features 8-byte array
    uint8_t features;
};

/// GAP controller environment LE credit based structure.
struct gapc_env_lecb_tag
{
    /// Pointer to the following list
    struct co_list_hdr hdr;
    /// Task id requested connection
    ke_task_id_t task_id;
    /// Security level
    uint16_t sec_lvl;
    /// Maximum Transmission Unit
    uint16_t mtu;
    /// Maximum Packet Size
    uint16_t mps;
    /// LE Protocol/Service Multiplexer
    uint16_t le_psm;
    /// Status
    uint8_t status;
    /// Packet ID
    uint8_t pkt_id;
    /// Source channel ID
    uint16_t src_cid;
    /// Destination channel ID
    uint16_t dst_cid;
    /// Source credit
    uint16_t src_credit;
    /// Destination credit
    uint16_t dst_credit;
};


/*
 * MACROS
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */
extern struct gapc_env_tag* gapc_env[GAPC_IDX_MAX];

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize Generic Access Profile Controller Module.
 *
 * @param[in] reset  true if it's requested by a reset; false if it's boot initialization
 *
 ****************************************************************************************
 */
void gapc_init(bool reset);


/**
 ****************************************************************************************
 * @brief A connection has been created, initialize Controller task.
 *
 * This function find first available task index available for new connection.
 * It triggers also connection event to task that has requested the connection.
 *
 * @param[in] con_params Connection parameters from lower layers
 * @param[in] requester  Task that request the connection to send indication(s)
 * @param[in] laddr      Local BD Address
 * @param[in] laddr_type Local BD Address Type (PUBLIC or RAND)
 *
 * @return Connection index allocated to the new connection.
 ****************************************************************************************
 */
uint8_t gapc_con_create(struct hci_le_con_cmp_evt const *con_params,
                        ke_task_id_t requester, struct bd_addr* laddr, uint8_t laddr_type);

#if (RWBLE_SW_VERSION_MAJOR >= 8)
uint8_t gapc_con_create_enh(struct hci_le_enh_con_cmp_evt const *con_params,
                            ke_task_id_t requester, struct bd_addr* laddr, uint8_t laddr_type);
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

/**
 ****************************************************************************************
 * @brief A connection has been disconnected, uninitialized Controller task.
 *
 * unregister connection, and destroy environment variable allocated for current connection.
 *
 * @param[in] conidx  Connection index
 *
 * @return Connection index of the connection.
 ****************************************************************************************
 */
uint8_t gapc_con_cleanup(uint8_t conidx);


/**
 ****************************************************************************************
 * @brief Send Disconnection indication to specific task
 *
 * @param[in] conidx  Connection index
 * @param[in] reason  Disconnection reason
 * @param[in] conhdl  Connection handle
 * @param[in] dest_id Message destination ID
 *
 ****************************************************************************************
 */
void gapc_send_disconect_ind(uint8_t conidx,  uint8_t reason, uint8_t conhdl,
                              ke_task_id_t dest_id);


/**
 ****************************************************************************************
 * @brief Retrieve connection index from connection handle.
 *
 * @param[in] conhdl Connection handle
 *
 * @return Return found connection index, GAP_INVALID_CONIDX if not found.
 ****************************************************************************************
 */
uint8_t gapc_get_conidx(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Retrieve connection handle from connection index.
 *
 * @param[in] conidx Connection index
 *
 * @return Return found connection handle, GAP_INVALID_CONHDL if not found.
 ****************************************************************************************
 */
uint16_t gapc_get_conhdl(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Retrieve connection role from connection index.
 *
 * @param[in] conidx Connection index
 *
 * @return Return found connection role
 ****************************************************************************************
 */
uint8_t gapc_get_role(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Retrieve connection address information on current link.
 *
 * @param[in] conidx Connection index
 * @param[in] src    Connection information source
 *
 * @return Return found connection address
 ****************************************************************************************
 */
struct gap_bdaddr* gapc_get_bdaddr(uint8_t conidx, uint8_t src);

/**
 ****************************************************************************************
 * @brief Retrieve connection CSRK information on current link.
 *
 * @param[in] conidx Connection index
 * @param[in] src    Connection information source
 *
 * @return Return found connection CSRK
 ****************************************************************************************
 */
struct gap_sec_key* gapc_get_csrk(uint8_t conidx, uint8_t src);

/**
 ****************************************************************************************
 * @brief Return the sign counter value for the specified connection index.
 *
 * @param[in] conidx Connection index
 * @param[in] src    Connection information source
 *
 * @return the requested signCounter value
 ****************************************************************************************
 */
uint32_t gapc_get_sign_counter(uint8_t conidx, uint8_t src);

/**
 * @brief Send a complete event of ongoing executed operation to requester.
 * It also clean-up variable used for ongoing operation.
 *
 * @param[in] conidx Connection index
 * @param[in] op_type       Operation type.
 * @param[in] status Status of completed operation
 */
void gapc_send_complete_evt(uint8_t conidx, uint8_t op_type, uint8_t status);

/**
 ****************************************************************************************
 * @brief Send operation completed message with status error code not related to a
 * running operation.
 *
 * @param[in] conidx    Connection index
 * @param[in] operation Operation code
 * @param[in] requester requester of operation
 * @param[in] status    Error status code
 ****************************************************************************************
 */
void gapc_send_error_evt(uint8_t conidx, uint8_t operation, const ke_task_id_t requester, uint8_t status);


/**
 ****************************************************************************************
 * @brief Get operation on going
 *
 * @param[in] conidx        Connection Index
 * @param[in] op_type       Operation type.
 *
 * @return operation code on going
 ****************************************************************************************
 */
uint8_t gapc_get_operation(uint8_t conidx, uint8_t op_type);

/**
 ****************************************************************************************
 * @brief Get operation pointer
 *
 * @param[in] conidx        Connection Index
 * @param[in] op_type       Operation type.
 *
 * @return operation pointer on going
 ****************************************************************************************
 */
void* gapc_get_operation_ptr(uint8_t conidx, uint8_t op_type);


/**
 ****************************************************************************************
 * @brief Set operation pointer
 *
 * @param[in] conidx        Connection Index
 * @param[in] op_type       Operation type.
 * @param[in] op            Operation pointer.
 *
 ****************************************************************************************
 */
void gapc_set_operation_ptr(uint8_t conidx, uint8_t op_type, void* op);

/**
 ****************************************************************************************
 * @brief Operation execution not finish, request kernel to reschedule it in order to
 * continue its execution
 *
 * @param[in] conidx        Connection Index
 * @param[in] op_type       Operation type.
 *
 * @return if operation has been rescheduled (not done if operation pointer is null)
 ****************************************************************************************
 */
bool gapc_reschedule_operation(uint8_t conidx, uint8_t op_type);

/**
 ****************************************************************************************
 * @brief Get requester of on going operation
 *
 * @param[in] conidx        Connection Index
 * @param[in] op_type       Operation type.
 *
 * @return task that requests to execute the operation
 ****************************************************************************************
 */
ke_task_id_t gapc_get_requester(uint8_t conidx, uint8_t op_type);


/**
 ****************************************************************************************
 * @brief Check if current link support security requirements.
 *
 * @param[in] conidx  Connection index
 * @param[in] sec_req Link security requirement to test
 *
 * @return True if link requirement is supported, False else.
 ****************************************************************************************
 */
bool gapc_is_sec_set(uint8_t conidx, uint8_t sec_req);

/**
 ****************************************************************************************
 * @brief Retrieve the encryption key size of the connection
 *
 * @param[in] conidx Connection index
 *
 * @return encryption key size (size is 7 - 16 byte range)
 *
 ****************************************************************************************
 */
uint8_t gapc_get_enc_keysize(uint8_t conidx);



/**
 ****************************************************************************************
 * @brief Set the encryption key size of the connection
 *
 * @param[in] conidx Connection index
 * @param[in] key_size encryption key size (size is 7 - 16 byte range)
 *
 ****************************************************************************************
 */
void gapc_set_enc_keysize(uint8_t conidx, uint8_t key_size);


/**
 ****************************************************************************************
 * @brief Update link status, current link is now encrypted
 *
 * @param[in] conidx Connection index
 *
 ****************************************************************************************
 */
void gapc_link_encrypted(uint8_t conidx);


/**
 ****************************************************************************************
 * @brief Update link authentication level
 *
 * @param[in] conidx Connection index
 * @param[in] auth   Link authentication level
 *
 ****************************************************************************************
 */
void gapc_auth_set(uint8_t conidx, uint8_t auth);


/**
 ****************************************************************************************
 * @brief Update task state
 *
 * @param[in] conidx Connection index
 * @param[in] state to update
 * @param[in] set state to busy (true) or idle (false)
 *
 ****************************************************************************************
 */
void gapc_update_state(uint8_t conidx, ke_state_t state, bool busy);

/**
 ****************************************************************************************
 * @brief Check LECB security permissions
 *
 * @param[in] lecb     Credit based channel
 * @param[in] conidx   Connection Index
 *
 * @return Returns status of the channel
 ****************************************************************************************
 */
uint8_t gapc_check_lecb_sec_perm(struct gapc_env_lecb_tag *lecb, uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Search channel depending on the parameter
 *
 * @param[in] conidx        Connection Index
 * @param[in] parameter     parameter to search
 * @param[in] mode          label of the parameter
 *
 * @return Returns NULL or address of the channel
 ****************************************************************************************
 */
struct gapc_env_lecb_tag* gapc_search_lecb_channel(uint8_t conidx, uint16_t parameter, uint16_t mode);

/**
 ****************************************************************************************
 * @brief Check validity of the parameters in order to send the frame
 *
 * @param[in] conidx        Connection Index
 * @param[in] cid           Channel identifier
 * @param[in] sdu_size      Total length of the SDU
 *
 * @return Returns current number of credits
 ****************************************************************************************
 */
uint16_t gapc_lecnx_check_tx(uint8_t conidx, uint16_t cid, uint16_t sdu_size);

/**
 ****************************************************************************************
 * @brief Check validity of the parameters in order to receive the frame
 *
 * @param[in] conidx        Connection Index
 * @param[in] cid           Channel identifier
 *
 * @return Returns current number of credits
 ****************************************************************************************
 */
uint16_t gapc_lecnx_check_rx(uint8_t conidx, uint16_t cid);

/**
 ****************************************************************************************
 * @brief Get field of a LE credit based structure
 *
 * @param[in] conidx        Connection Index
 * @param[in] cid           Channel identifier
 * @param[in] field         field identifier
 * @param[in] src_dest      0 for source otherwise destination
 * @param[out] value        requested value if successful
 *
 * @return status of the operation
 ****************************************************************************************
 */
uint16_t gapc_lecnx_get_field(uint8_t conidx, uint16_t cid, uint8_t field, bool src_dest, uint16_t *value);

/**
 ****************************************************************************************
 * @brief Get Service Change Client Configuration
 *
 * @param[in] conidx Connection index
 *
 * @return Service Change Client Configuration
 ****************************************************************************************
 */
bool gapc_svc_chg_ccc_get(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Set Service Change Client Configuration
 *
 * @param[in] conidx Connection index
 * @param[in] enable True if CCC is enabled, False else
 *
 ****************************************************************************************
 */
void gapc_svc_chg_ccc_set(uint8_t conidx, bool enable);

#endif // (BLE_CENTRAL || BLE_PERIPHERAL)
/// @} GAPC

#endif /* _GAPC_H_ */
