/**
 ****************************************************************************************
 *
 * @file attm.h
 *
 * @brief Header file - ATTM.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef ATTM_H_
#define ATTM_H_

/**
 ****************************************************************************************
 * @addtogroup ATTM Attribute Manager
 * @ingroup ATT
 * @brief Attribute Manager
 *
 * The ATTM is the attribute manager of the Attribute Profile block and
 * is responsible for managing messages and providing generic attribute
 * functionalities to @ref ATTC "ATTC" and @ref ATTS "ATTS".
 *
 *
 * @{
 *
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "co_version.h"
#include "rwip_config.h"
#include "ke_task.h"
#include "ke_timer.h"
#include "co_error.h"
#include "attm_cfg.h"
#include "att.h"
#include <string.h>

/*
 * DEFINES
 ****************************************************************************************
 */


/// Macro used to retrieve access permission rights
#define PERM_GET(perm, access)\
        (((perm) & (PERM_MASK_ ## access)) >> (PERM_POS_ ## access))

/// Macro used to retrieve permission value from access and rights on attribute.
#define PERM(access, right) \
    (((PERM_RIGHT_ ## right) << (PERM_POS_ ## access)) & (PERM_MASK_ ## access))

/// Macro used know if permission is set or not.
#define PERM_IS_SET(perm, access, right) \
    (((perm) & (((PERM_RIGHT_ ## right) << (PERM_POS_ ## access))) \
                & (PERM_MASK_ ## access)) == PERM(access, right))

/// Macro used to create permission value
#define PERM_VAL(access, perm) \
    ((((perm) << (PERM_POS_ ## access))) & (PERM_MASK_ ## access))



/// Retrieve attribute security level from attribute right and service right
#define ATT_GET_SEC_LVL(att_right, svc_right) \
    co_max(((att_right) & PERM_RIGHT_AUTH), ((svc_right) & PERM_RIGHT_AUTH));

/// Retrieve UUID LEN from UUID Length Permission
#define ATT_UUID_LEN(uuid_len_perm) ((uuid_len_perm == 0) ? ATT_UUID_16_LEN : \
        ((uuid_len_perm == 1) ? ATT_UUID_32_LEN  :                        \
        ((uuid_len_perm == 2) ? ATT_UUID_128_LEN : 0)))

/// Initialization of attribute element
#define ATT_ELEMT_INIT                                   {{NULL}, false}

#if (RWBLE_SW_VERSION_MAJOR >= 8)
/**
 *
 *  31              20  19   18   17   16   15   14   13   12   11   10   9    8    7    6    5    4    3    2    1    0
 * +----+----+--------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * |     Reserved     |UUID_LEN | WR | WS | WC |EKS | B  |EXT |      NTF     |      IND     |      WR      |      RD      |
 * +----+----+--------+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 *
 * Bit [0-2]  : Read Permission         (0 = Disable, 1 = Enable, 2 = UNAUTH, 3 = AUTH)
 * Bit [3-5]  : Write Permission        (0 = Disable, 1 = Enable, 2 = UNAUTH, 3 = AUTH)
 * Bit [6-8]  : Indication Permission   (0 = Disable, 1 = Enable, 2 = UNAUTH, 3 = AUTH)
 * Bit [9-11] : Notification Permission (0 = Disable, 1 = Enable, 2 = UNAUTH, 3 = AUTH)
 * Bit [12]   : Extended properties present (only relevant for a characteristic value)
 * Bit [13]   : Broadcast permission        (only relevant for a characteristic value)
 * Bit [14]   : Encryption key Size must be 16 bytes
 * Bit [15]   : Write Command accepted
 * Bit [16]   : Write Signed accepted
 * Bit [17]   : Write Request accepted
 * Bit [18-19]: UUID Length             (0 = 16 bits, 1 = 32 bits, 2 = 128 bits, 3 = RFU)
 * Bit [20-31]: Reserved
 */
enum attm_perm_mask
{
    /// Retrieve all permission info
    PERM_MASK_ALL           = 0x0000,
    // Read Access Mask
    PERM_MASK_RD            = 0x0007,
    PERM_POS_RD             = 0,
    // Write Access Mask
    PERM_MASK_WR            = 0x0038,
    PERM_POS_WR             = 3,
    // Indication Access Mask
    PERM_MASK_IND           = 0x01C0,
    PERM_POS_IND            = 6,
    // Notification Access Mask
    PERM_MASK_NTF           = 0x0E00,
    PERM_POS_NTF            = 9,
    // Extended properties descriptor present
    PERM_MASK_EXT           = 0x1000,
    PERM_POS_EXT            = 12,
    // Broadcast descriptor present
    PERM_MASK_BROADCAST     = 0x2000,
    PERM_POS_BROADCAST      = 13,
    // Check Encryption key size Mask
    PERM_MASK_EKS           = 0x4000,
    PERM_POS_EKS            = 14,
    // Write Command Enabled attribute Mask
    PERM_MASK_WRITE_COMMAND = 0x8000,
    PERM_POS_WRITE_COMMAND  = 15,
    // Write Signed Enabled attribute Mask
    PERM_MASK_WRITE_SIGNED  = 0x10000,
    PERM_POS_WRITE_SIGNED   = 16,
    // Write Request Enabled attribute Mask
    PERM_MASK_WRITE_REQ     = 0x20000,
    PERM_POS_WRITE_REQ      = 17,
    // UUID Length
    PERM_MASK_UUID_LEN      = 0xC0000,
    PERM_POS_UUID_LEN       = 18,
};
#else
/**
 *  15   14   13   12   11   10   9    8    7    6    5    4    3    2    1    0
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * |UUID_LEN |EKS | WR | WS | WC | B  |EXT |    N    |    I    |   WP    |    RD   |
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 *
 * Bit [0-1]  : Read Permission         (0 = Disable, 1 = Enable, 2 = UNAUTH, 3 = AUTH)
 * Bit [2-3]  : Write Permission        (0 = Disable, 1 = Enable, 2 = UNAUTH, 3 = AUTH)
 * Bit [4-5]  : Indication Permission   (0 = Disable, 1 = Enable, 2 = UNAUTH, 3 = AUTH)
 * Bit [6-7]  : Notification Permission (0 = Disable, 1 = Enable, 2 = UNAUTH, 3 = AUTH)
 * Bit [8]    : Extended properties present (only relevant for a characteristic value)
 * Bit [9]    : Broadcast permission        (only relevant for a characteristic value)
 * Bit [10]   : Write Command accepted
 * Bit [11]   : Write Signed accepted
 * Bit [12]   : Write Request accepted
 * Bit [13]   : Encryption key Size must be 16 bytes
 * Bit [15-14]: UUID Length             (0 = 16 bits, 1 = 32 bits, 2 = 128 bits, 3 = RFU)
 */
enum attm_perm_mask
{
    /// retrieve all permission info
    PERM_MASK_ALL          = 0x0000,
    /// Read Access Mask
    PERM_MASK_RD           = 0x0003,
    PERM_POS_RD            = 0,
    /// Write Access Mask
    PERM_MASK_WR           = 0x000C,
    PERM_POS_WR            = 2,
    /// Indication Access Mask
    PERM_MASK_IND          = 0x0030,
    PERM_POS_IND           = 4,
    /// Notification Access Mask
    PERM_MASK_NTF          = 0x00C0,
    PERM_POS_NTF           = 6,
    /// Extended properties descriptor present
    PERM_MASK_EXT           = 0x0100,
    PERM_POS_EXT            = 8,
    /// Broadcast descriptor present
    PERM_MASK_BROADCAST     = 0x0200,
    PERM_POS_BROADCAST      = 9,
    /// Check Encryption key size Mask
    PERM_MASK_EKS           = 0x0400,
    PERM_POS_EKS            = 10,
    /// Write Command Enabled attribute Mask
    PERM_MASK_WRITE_COMMAND = 0x0800,
    PERM_POS_WRITE_COMMAND  = 11,
    /// Write Signed Enabled attribute Mask
    PERM_MASK_WRITE_SIGNED  = 0x1000,
    PERM_POS_WRITE_SIGNED   = 12,
    /// Write Request Enabled attribute Mask
    PERM_MASK_WRITE_REQ     = 0x2000,
    PERM_POS_WRITE_REQ      = 13,
    /// UUID Length
    PERM_MASK_UUID_LEN      = 0xC000,
    PERM_POS_UUID_LEN       = 14,
};
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

/**
 * Value permission bit field
 *
 *  15   14   13   12   11   10   9    8    7    6    5    4    3    2    1    0
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 * | RI |               MAX_LEN (RI = 1) / Value Offset  (RI = 0)                  |
 * +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 *
 * Bit [0-14]: Maximum Attribute Length or Value Offset pointer
 * Bit [15]  : Trigger Read Indication (0 = Value present in Database, 1 = Value not present in Database)
 */
enum attm_value_perm_mask
{
    /// Maximum Attribute Length
    PERM_MASK_MAX_LEN     = 0x7FFF,
    PERM_POS_MAX_LEN      = 0,
    /// Attribute value Offset
    PERM_MASK_VAL_OFFSET  = 0x7FFF,
    PERM_POS_VAL_OFFSET   = 0,
    /// Read trigger Indication
    PERM_MASK_RI          = 0x8000,
    PERM_POS_RI           = 15,
};

#if (RWBLE_SW_VERSION_MAJOR >= 8)
/**
 *  Service permissions
 *  
 *   7    6    5    4    3    2    1    0
 *  +----+----+----+----+----+----+----+----+
 *  | P  |UUID_LEN |      AUTH    |EKS | MI |
 *  +----+----+----+----+----+----+----+----+
 *  
 *  Bit [0]  : Task that manage service is multi-instantiated (Connection index is conveyed)
 *  Bit [1]  : Encryption key Size must be 16 bytes
 *  Bit [2-3]: Service Permission      (0 = Disable, 1 = Enable, 2 = UNAUTH, 3 = AUTH, 4 = Secure connection)
 *  Bit [4-5]: UUID Length             (0 = 16 bits, 1 = 32 bits, 2 = 128 bits, 3 = RFU)
 *  Bit [6]  : Primary Service         (1 = Primary Service, 0 = Secondary Service)
 *  Bit [7]  : Reserved for future use
 *  
 */
enum attm_svc_perm_mask
{
    // Task that manage service is multi-instantiated
    PERM_MASK_SVC_MI        = 0x01,
    PERM_POS_SVC_MI         = 0,
    // Check Encryption key size for service Access
    PERM_MASK_SVC_EKS       = 0x02,
    PERM_POS_SVC_EKS        = 1,
    // Service Permission authentication
    PERM_MASK_SVC_AUTH      = 0x1C,
    PERM_POS_SVC_AUTH       = 2,
    /// Service UUID Length
    PERM_MASK_SVC_UUID_LEN  = 0x60,
    PERM_POS_SVC_UUID_LEN   = 5,
    // Service type Primary
    PERM_MASK_SVC_PRIMARY   = 0x80,
    PERM_POS_SVC_PRIMARY    = 7,
};
#else
/**
 * Service permissions
 *
 *  7    6    5    4    3    2    1    0
 * +----+----+----+----+----+----+----+----+
 * |RFU | P  |UUID_LEN |  AUTH   |EKS | MI |
 * +----+----+----+----+----+----+----+----+
 *
 * Bit [0]  : Task that manage service is multi-instantiated (Connection index is conveyed)
 * Bit [1]  : Encryption key Size must be 16 bytes
 * Bit [2-3]: Service Permission      (0 = Disable, 1 = Enable, 2 = UNAUTH, 3 = AUTH)
 * Bit [4-5]: UUID Length             (0 = 16 bits, 1 = 32 bits, 2 = 128 bits, 3 = RFU)
 * Bit [6]  : Primary Service         (1 = Primary Service, 0 = Secondary Service)
 * Bit [7]  : Reserved for future use
 */
enum attm_svc_perm_mask
{
    /// Task that manage service is multi-instantiated
    PERM_MASK_SVC_MI        = 0x01,
    PERM_POS_SVC_MI         = 0,
    /// Check Encryption key size for service Access
    PERM_MASK_SVC_EKS       = 0x02,
    PERM_POS_SVC_EKS        = 1,
    /// Service Permission authentication
    PERM_MASK_SVC_AUTH      = 0x0C,
    PERM_POS_SVC_AUTH       = 2,
    /// Service UUID Length
    PERM_MASK_SVC_UUID_LEN  = 0x30,
    PERM_POS_SVC_UUID_LEN   = 4,
    /// Service type Primary
    PERM_MASK_SVC_PRIMARY   = 0x40,
    PERM_POS_SVC_PRIMARY    = 6,
};
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

/// Attribute & Service access rights
enum
{
    /// Disable access
    PERM_RIGHT_DISABLE  = 0,
    /// Enable access
    PERM_RIGHT_ENABLE   = 1,
    /// Access Requires Unauthenticated link
    PERM_RIGHT_UNAUTH   = 2,
    /// Access Requires Authenticated link
    PERM_RIGHT_AUTH     = 3,
#if (RWBLE_SW_VERSION_MAJOR >= 8)
    /// Access Requires Secure Connection
    PERM_RIGHT_SECURE   = 4,
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
};

/// Attribute & Service UUID Length
enum
{
    /// 16  bits UUID
    PERM_UUID_16         = 0,
    /// 32  bits UUID
    PERM_UUID_32         = 1,
    /// 128 bits UUID
    PERM_UUID_128        = 2,
    /// Invalid
    PERM_UUID_RFU        = 3,
};

/// execute flags
enum
{
    /// Cancel All the Reliable Writes
    ATT_CANCEL_ALL_PREPARED_WRITES = 0x00,
    /// Write All the Reliable Writes
    ATT_EXECUTE_ALL_PREPARED_WRITES
};



/*
 * DATA STRUCTURES
 ****************************************************************************************
 */



#if(BLE_ATTS)

#if (RWBLE_SW_VERSION_MAJOR >= 8)
typedef uint32_t att_perm_type;
#else
typedef uint16_t att_perm_type;
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

/**
 * Attribute Description
 */
struct attm_att_desc
{
    /**
     * 16 bits UUID or data offset used to retrieve 32 or 128 bits UUID in service memory block
     */
    uint16_t uuid;

    /**
     *  Attribute Permission (@see attm_perm_mask)
     */
    att_perm_type perm;

    /**
     * value information (@see attm_value_perm_mask)
     */
    union att_info
    {
        /// attribute value
        uint16_t value;

        /// attribute max length (RI = 1)
        uint16_t max_lengh;

        /// attribute value offset pointer (RI = 0)
        uint16_t offset;
    } info;
};

/// attribute value if present in database
struct attm_att_value
{
    /// Maximum attribute length
    uint16_t max_length;
    /// currrent attribute length that can be read.
    uint16_t length;
    ///value data pointer
    uint8_t  value[__ARRAY_EMPTY];
};

/// service description
struct attm_svc_desc
{
    /// Service Start Handle
    uint16_t start_hdl;
    /// Service End Handle
    uint16_t end_hdl;
    /// Task identifier that manages service
    uint16_t task_id;

    /**
     * Service Permission (@see attm_svc_perm_mask)
     */
    uint8_t perm;

    /// number of attributes present in service (end_hdl - start_hdl - 1)
    uint8_t nb_att;

    /// Service 16 bits UUID (LSB First) or data offset used to retrieve 32 or 128 bits
    /// UUID in service memory block
    uint16_t uuid;
};

/**
 * Service description present in attribute database
 */
struct attm_svc
{
    /// Next Service
    struct attm_svc* next;

    /// service description
    struct attm_svc_desc svc;

    /**
     * List of attribute description present in service.
     */
    struct attm_att_desc atts[__ARRAY_EMPTY];
};

/// Attribute element information
struct attm_elmt
{
    /// element info
    union elem_info
    {
        /// attribute info pointer
        struct attm_att_desc* att;

        /// service info pointer
        struct attm_svc_desc* svc;
    } info;

    /// use to know if current element is a service or an attribute
    bool service;
};

/// ATTM General Information Manager
struct attm_db
{
    /**
     * **************************************************************************************
     * @brief Attribute database
     *
     * The Attribute database is a list of attribute services sorted by handle number.
     * This database shall be initiate by GAP, GATT, profiles and application process at
     * startup and must not change during runtime.
     *
     * Database initialization shall be deterministic in order to always have service handle
     * at same position in database during all product life-cycle. This is required since
     * database client can save position of services in database to not perform service
     * discovery at each connection.
     ***************************************************************************************
     */
    struct attm_svc * svcs;

    /**
     ***************************************************************************************
     * Last attribute service searched.
     *
     * Used as a cached variable, it's used to reduce handle search duration.
     ***************************************************************************************
     */
    struct attm_svc * cache;

    /**
     * Temporary value used for read operation on service and characteristics attributes
     */
    uint8_t  temp_val[ATT_UUID_128_LEN + ATT_HANDLE_LEN + ATT_HANDLE_LEN];
};


/// Internal 16bits UUID service description
struct attm_desc
{
    /// 16 bits UUID LSB First
    uint16_t uuid;
    /// Attribute Permissions (@see enum attm_perm_mask)
    att_perm_type perm;
    /// Attribute Max Size (@see enum attm_value_perm_mask)
    /// note: for characteristic declaration contains handle offset
    /// note: for included service, contains target service handle
    uint16_t max_size;
};


#endif //(BLE_ATTS)

#if (BLE_CENTRAL || BLE_PERIPHERAL)
/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief Compare if two UUIDs matches
 *
 * @param[in]  uuid_a      UUID A value
 * @param[in]  uuid_a_len  UUID A length
 * @param[in]  uuid_b      UUID B value
 * @param[in]  uuid_b_len  UUID B length
 *
 * @return true if UUIDs matches, false otherwise
 ****************************************************************************************
 */
bool attm_uuid_comp(uint8_t *uuid_a, uint8_t uuid_a_len,
                      uint8_t *uuid_b, uint8_t uuid_b_len);


/**
 ****************************************************************************************
 * @brief Check if two UUIDs matches (2nd UUID is a 16 bits UUID with LSB First)
 *
 * @param[in]  uuid_a      UUID A value
 * @param[in]  uuid_a_len  UUID A length
 * @param[in]  uuid_b      UUID B 16 bit value
 *
 * @return true if UUIDs matches, false otherwise
 ****************************************************************************************
 */
bool attm_uuid16_comp(uint8_t *uuid_a, uint8_t uuid_a_len, uint16_t uuid_b);


/**
 ****************************************************************************************
 * @brief Convert UUID value to 128 bit UUID
 *
 * @param[out] uuid128   converted 32-bit Bluetooth UUID to 128-bit UUID
 * @param[in]  uuid      UUID to convert to 128-bit UUID
 * @param[in]  uuid_len  UUID length
 *
 ****************************************************************************************
 */
void attm_convert_to128(uint8_t *uuid128, uint8_t *uuid, uint8_t uuid_len);

/**
 ****************************************************************************************
 * @brief Check if it's a Bluetooth 16-bits UUID for 128-bit input
 *
 * @param[in]  uuid      128-bit UUID
 *
 * @return true if uuid  is a Bluetooth 16-bit UUID, false else.
 ****************************************************************************************
 */
bool attm_is_bt16_uuid(uint8_t *uuid);

/**
 ****************************************************************************************
 * @brief Check if it's a Bluetooth 32 bits UUID for 128-bit input
 *
 * @param[in]  uuid      128-bit UUID
 *
 * @return true if uuid  is a Bluetooth 32-bits UUID, false else.
 ****************************************************************************************
 */
bool attm_is_bt32_uuid(uint8_t *uuid);

#endif // #if (BLE_CENTRAL || BLE_PERIPHERAL)
/// @} ATTM
#endif // ATTM_H_
