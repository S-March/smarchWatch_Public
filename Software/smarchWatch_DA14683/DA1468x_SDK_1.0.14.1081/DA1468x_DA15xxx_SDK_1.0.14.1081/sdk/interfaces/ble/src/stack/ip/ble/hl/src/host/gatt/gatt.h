/**
 ****************************************************************************************
 *
 * @file gatt.h
 *
 * @brief Header file - GATT.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef GATT_H_
#define GATT_H_

/**
 ****************************************************************************************
 * @addtogroup GATT Generic Attribute Profile
 * @ingroup HOST
 * @brief Generic Attribute Profile.
 *
 * The GATT module is responsible for providing an API for all attribute related operations.
 * It is responsible for all the service framework activities using the Attribute protocol
 * for discovering services and for reading and writing characteristic values on a peer device.
 * To achieve this, the GATT interfaces with @ref ATTM "ATTM", @ref ATTC "ATTC" and the
 * @ref ATTS "ATTS".
 *
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#include "att.h"


/*
 * DEFINE DECLARATIONS
 ****************************************************************************************
 */




#if (BLE_ATTS)
/// GATT Attribute database handles
/// Generic Attribute Profile Service
enum gatt_db_att
{
    GATT_IDX_PRIM_SVC,
    GATT_IDX_CHAR_SVC_CHANGED,
    GATT_IDX_SVC_CHANGED,
    GATT_IDX_SVC_CHANGED_CFG,

    GATT_IDX_NUMBER
};
#endif /* (BLE_ATTS)*/

/// GATT Client Characteristic Configuration Codes
enum gatt_ccc_val
{
    /// Stop notification/indication
    GATT_CCC_STOP_NTFIND = 0x0000,
    /// Start notification
    GATT_CCC_START_NTF,
    /// Start indication
    GATT_CCC_START_IND
};

/*
 * Type Definition
 ****************************************************************************************
 */


/// Service Changed type definition
struct gatt_svc_changed
{
    /// Service start handle which changed
    uint16_t start_hdl;
    /// Service end handle which changed
    uint16_t end_hdl;
};


/// @} GATT
#endif // GATT_H_
