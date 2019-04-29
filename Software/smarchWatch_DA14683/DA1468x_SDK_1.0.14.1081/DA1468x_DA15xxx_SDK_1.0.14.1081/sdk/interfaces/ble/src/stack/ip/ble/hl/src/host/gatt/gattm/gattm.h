/**
 ****************************************************************************************
 *
 * @file gattm.h
 *
 * @brief Header file - GATT Manager.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef GATTM_H_
#define GATTM_H_



/**
 ****************************************************************************************
 * @addtogroup GATTM Generic Attribute Profile Manager
 * @ingroup GATT
 * @brief Generic Attribute Profile.
 *
 * The GATT manager module is responsible for providing an API for all action operations
 * not related to a connection. It's responsible to managing internal database.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
/* kernel task */
#include "rwip_config.h"
#include "co_version.h"

#if (BLE_CENTRAL || BLE_PERIPHERAL)
#include "attm.h"

/*
 * DEFINES
 ****************************************************************************************
 */


// GATT database default features
#define GATT_DB_DEFAULT_FEAT         0x0001
// GATT database Service changed feature
#define GATT_DB_SVC_CHG_FEAT         0x000E


#if (BLE_ATTS)
/// retrieve gatt attribute handle from attribute index.
#define GATT_GET_ATT_HANDLE(idx)\
    ((gattm_env.svc_start_hdl == 0)? (0) :(gattm_env.svc_start_hdl + (idx)))
#endif // (BLE_ATTS)

/// GATT General Information Manager
struct gattm_env_tag
{
    #if (BLE_ATTS)
    /// Environment data needed by attribute database
    struct attm_db  db;

    /// GATT service start handle
    uint16_t svc_start_hdl;
    #endif // (BLE_ATTS)

    /// Maximum device MTU size
    uint16_t max_mtu;
#if (RWBLE_SW_VERSION_MAJOR >= 8)
    /// Maximum device MPS size
    uint16_t max_mps;
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */
extern struct gattm_env_tag gattm_env;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the GATT manager module.
 * This function performs all the initialization steps of the GATT module.
 *
 * @param[in] reset  true if it's requested by a reset; false if it's boot initialization
 *
 ****************************************************************************************
 */
void gattm_init(bool reset);


/**
 ****************************************************************************************
 * @brief Initialize GATT attribute database
 *
 * @param[in] start_hdl Service Start Handle
 * @param[in] feat      Attribute database features
 *
 * @return status code of attribute database initialization
 * Command status code:
 *  - @ref ATT_ERR_NO_ERROR: If database creation succeeds.
 *  - @ref ATT_ERR_INVALID_HANDLE: If start_hdl given in parameter + nb of attribute override
 *                            some existing services handles.
 *  - @ref ATT_ERR_INSUFF_RESOURCE: There is not enough memory to allocate service buffer.
 *                           or of new attribute cannot be added because all expected
 *                           attributes already add
 ****************************************************************************************
 */
uint8_t gattm_init_attr(uint16_t start_hdl, uint32_t feat);

/**
 ****************************************************************************************
 * @brief Initialize GATT resources for connection.
 *
 * @param[in] conidx connection record index
 * @param[in] role   device role after connection establishment
 *
 ****************************************************************************************
 */
void gattm_create(uint8_t conidx);
/**
 ****************************************************************************************
 * @brief Cleanup GATT resources for connection
 *
 * @param[in] conidx   connection record index
 *
 ****************************************************************************************
 */
void gattm_cleanup(uint8_t conidx);

#if (BLE_ATTS)
/**
 ****************************************************************************************
 * @brief Return the start handle of the GATT service in the database *
 ****************************************************************************************
 */
uint16_t gattm_svc_get_start_hdl(void);
#endif //(BLE_ATTS)

/**
 ****************************************************************************************
 * @brief Return the maximal MTU value
 *
 * @param[out] Maximal MTU value
 ****************************************************************************************
 */
uint16_t gattm_get_max_mtu(void);

/**
 ****************************************************************************************
 * @brief Set the maximal MTU value
 *
 * @param[in] mtu   Max MTU value (Minimum is 23)
 ****************************************************************************************
 */
void gattm_set_max_mtu(uint16_t mtu);

#endif /* (BLE_CENTRAL || BLE_PERIPHERAL) */

#if (RWBLE_SW_VERSION_MAJOR >= 8)
/**
 ****************************************************************************************
 * @brief Set the maximal MPS value
 *
 * @param[in] mps   Max MPS value
 ****************************************************************************************
 */
void gattm_set_max_mps(uint16_t mps);

/**
 ****************************************************************************************
 * @brief Get the maximal MPS value
 *
 * @param[out] Maximal MPS value
 ****************************************************************************************
 */
uint16_t gattm_get_max_mps(void);
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

/// @} GATTM
#endif // GATTM_H_
