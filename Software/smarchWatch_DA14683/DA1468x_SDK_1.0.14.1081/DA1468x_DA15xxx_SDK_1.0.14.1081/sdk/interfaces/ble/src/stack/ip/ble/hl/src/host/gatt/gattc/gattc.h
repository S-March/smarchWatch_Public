/**
 ****************************************************************************************
 *
 * @file gattc.h
 *
 * @brief Header file - GATT Controller.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef GATTC_H_
#define GATTC_H_

/**
 ****************************************************************************************
 * @addtogroup GATTC Generic Attribute Profile Controller
 * @ingroup GATT
 * @brief Generic Attribute Profile Controller.
 *
 * This GATT module is responsible for providing an API for all attribute related operations
 * related to a BLE connection.
 * It is responsible for all the service framework activities using the Attribute protocol
 * for discovering services and for reading and writing characteristic values on a peer device.
 * To achieve this, the GATT interfaces with @ref ATTC "ATTC" and the @ref ATTS "ATTS".
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
#if (BLE_CENTRAL || BLE_PERIPHERAL)

#include "ke_task.h"
#include "gattc_task.h"
#include "gatt.h"
#include "attc.h"
#include "atts.h"
/*
 * DEFINES
 ****************************************************************************************
 */
/// retrieve on-going operation command
#define GATT_OPERATION_CMD(conidx, op_type, cmd) \
    ((struct cmd*) gattc_get_operation_ptr(conidx, op_type))

#define GATT_WRITE_ERROR_CODE (0xFFFF)

/*
 * MACRO DEFINITIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/// GATT controller environment variable structure.
struct gattc_env_tag
{
    /// Request operation Kernel message
    void* operation[GATTC_OP_MAX];

    #if (BLE_ATTC)
    struct attc_env client;
    #endif // (BLE_ATTC)

    #if (BLE_ATTS)
    struct atts_env server;
    #endif // (BLE_ATTS)

    /// Current MTU Size
    uint16_t mtu_size;

    /// A transaction timeout occurs, reject next attribute commands
    bool     trans_timeout;
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */
extern struct gattc_env_tag* gattc_env[GATTC_IDX_MAX];

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief Initialization of the GATT controller module.
 * This function performs all the initialization steps of the GATT module.
 *
 * @param[in] reset  true if it's requested by a reset; false if it's boot initialization
 *
 ****************************************************************************************
 */
void gattc_init(bool reset);


/**
 ****************************************************************************************
 * @brief Initialize GATT controller for connection.
 *
 * @param[in] conidx    connection record index
 * @param[in] role   device role after connection establishment
 *
 ****************************************************************************************
 */
void gattc_create(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Created link connection parameters (from bond data) has been set, connection
 * ready to be used.
 *
 * @param[in] conidx     Connection Index
 *
 ****************************************************************************************
 */
void gattc_con_enable(uint8_t conidx);


/**
 ****************************************************************************************
 * @brief Cleanup GATT controller resources for connection
 *
 * @param[in] conidx   connection record index
 *
 ****************************************************************************************
 */
void gattc_cleanup(uint8_t conidx);


/**
 ****************************************************************************************
 * @brief Gets the negotiated MTU. This function gets the negotiated MTU.
 *
 * @param[in] idx   connection record index
 *
 * @return MTU negotiated
 *
 ****************************************************************************************
 */
uint16_t gattc_get_mtu(uint8_t idx);

/**
 ****************************************************************************************
 * @brief Sets the negotiated MTU This function stores the negotiated MTU.
 *
 * @param[in] idx   connection record index
 * @param[in] mtu   negotiated MTU
 *
 * @return status   indicates if the MTU setting operation is successful
 *
 ****************************************************************************************
 */
void gattc_set_mtu(uint8_t idx, uint16_t mtu);


/**
 * @brief Send a complete event of ongoing executed operation to requester.
 * It also clean-up variable used for ongoing operation.
 *
 * @param[in] conidx Connection index
 * @param[in] op_type       Operation type.
 * @param[in] status Status of completed operation
 */
void gattc_send_complete_evt(uint8_t conidx, uint8_t op_type, uint8_t status);

/**
 ****************************************************************************************
 * @brief Send operation completed message with status error code not related to a
 * running operation.
 *
 * @param[in] conidx    Connection index
 * @param[in] operation Operation code
 * @param[in] seq_num   Operation sequence number
 * @param[in] requester requester of operation
 * @param[in] status    Error status code
 ****************************************************************************************
 */
void gattc_send_error_evt(uint8_t conidx, uint8_t operation, uint16_t seq_num, const ke_task_id_t requester, uint8_t status);


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
uint8_t gattc_get_operation(uint8_t conidx, uint8_t op_type);

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
void* gattc_get_operation_ptr(uint8_t conidx, uint8_t op_type);


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
void gattc_set_operation_ptr(uint8_t conidx, uint8_t op_type, void* op);

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
bool gattc_reschedule_operation(uint8_t conidx, uint8_t op_type);

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
ke_task_id_t gattc_get_requester(uint8_t conidx, uint8_t op_type);



/**
 ****************************************************************************************
 * @brief Get Operation Sequence Number
 *
 * @param[in] conidx        Connection Index
 * @param[in] op_type       Operation type.
 *
 * @return Sequence number provided for operation execution
 ****************************************************************************************
 */
uint16_t gattc_get_op_seq_num(uint8_t conidx, uint8_t op_type);


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
void gattc_update_state(uint8_t conidx, ke_state_t state, bool busy);
#endif /* (BLE_CENTRAL || BLE_PERIPHERAL) */

/// @} GATTC
#endif // GATTC_H_
