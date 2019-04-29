/**
 ****************************************************************************************
 *
 * @file attm_db.h
 *
 * @brief Header file - ATTM.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef ATTM_DB_H_
#define ATTM_DB_H_

/**
 ****************************************************************************************
 * @addtogroup ATTDB Database
 * @ingroup ATTM
 * @brief Attribute Protocol Database
 *
 * The ATTDB module is responsible for providing different sets of attribute databases
 * for Attribute Profile server.
 *
 * This module can be tailored by client, to match the requirement of the desired database.
 *
 * @{
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#if (BLE_ATTS)
#include <stdio.h>
#include <string.h>
#include "rwip_config.h"
#include "ke_task.h"
#include "attm_cfg.h"
#include "attm.h"
#include "gattm_task.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// update attribute permission on specific handle
#define ATTMDB_UPDATE_PERM(handle, access, right)\
    attmdb_att_update_perm(handle, (PERM_MASK_ ## access), PERM(access, right))

#define ATTMDB_UPDATE_PERM_VAL(handle, access, val)\
    attmdb_att_update_perm(handle, (PERM_MASK_ ## access), (val << (PERM_POS_ ## access)))



/*
 * TYPE DEF
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Add a service in database.
 *
 * According to service start handle and number of attribute, ATTM DB allocate a set of
 * attribute handles, then using other parameters it allocate a buffer used to describe
 * service, and allocate attributes + their values.
 *
 * If start_hdl = 0, it allocated service using first available handle (start_hdl is
 * modified); else it will allocate service according to given start handle.
 *
 *
 * @param[in|out] svc_desc Service description.
 *
 * @return Command status code:
 *  - @ref ATT_ERR_NO_ERROR: If service allocation succeeds.
 *  - @ref ATT_ERR_INVALID_HANDLE: If start_hdl given in parameter or UUIDs value invalid
 *  - @ref ATT_ERR_INSUFF_RESOURCE: There is not enough memory to allocate service buffer.
 ****************************************************************************************
 */
uint8_t attmdb_add_service(struct gattm_svc_desc* svc_desc);



/**
 ****************************************************************************************
 * @brief Clear database
 *
 * For debug purpose only, this function clear the database and unalloc all services
 * within database.
 *
 * This function shall be used only for qualification and tests in order to manually
 * change database without modifying software.
 ****************************************************************************************
 */
void attmdb_destroy(void);

/**
 ****************************************************************************************
 *  @brief Search in database from which service attribute handle comes from.
 *
 * @param[in] handle Attribute handle.
 *
 * @return Services that contains attribute handle; NULL if handle not available in
 *         database.
 ****************************************************************************************
 */
struct attm_svc * attmdb_get_service(uint16_t handle);

/**
 ****************************************************************************************
 *  @brief Search in database Attribute pointer using attribute handle.
 *
 * @param[in]  handle   Attribute handle.
 * @param[out] elmt     Attribute element to fill
 *
 * @return Command status code:
 *  - @ref ATT_ERR_NO_ERROR: If attribute found.
 *  - @ref ATT_ERR_INVALID_HANDLE: If No Attribute found
 ****************************************************************************************
 */
uint8_t attmdb_get_attribute(uint16_t handle, struct attm_elmt*elmt);

/**
 ****************************************************************************************
 * @brief Retrieve attribute at or after specified handle
 *
 * Retrieve first attribute with handle >= parameter handle.
 * Parameter handle is updated according found attribute.
 *
 * @param[in|out] handle   Attribute handle.
 * @param[out]    elmt     Attribute element to fill
 *
 * @return Command status code:
 *  - @ref ATT_ERR_NO_ERROR: If attribute found.
 *  - @ref ATT_ERR_INVALID_HANDLE: If No Attribute found
 ****************************************************************************************
 */
uint8_t attmdb_get_next_att(uint16_t * handle, struct attm_elmt*elmt);

/**
 ****************************************************************************************
 * Check if attribute element UUID is equals to uuid given in parameter.
 *
 * @param elmt     Attribute element that can be a UUID 16 or 128 bits
 * @param uuid16   UUID 16 bits to compare
 *
 * @return True if UUIDs matches, False else.
 ****************************************************************************************
 */
bool attmdb_uuid16_comp(struct attm_elmt *elmt, uint16_t uuid16);

/**
 ****************************************************************************************
 * @brief Update attribute value
 *
 * Updating attribute value do not trigger any notification or indication, this shall be
 * handled by GATT task.
 *
 * @param[in] handle Attribute handle.
 * @param[in] length Size of new attribute value
 * @param[in] offset Data offset of in the payload to set
 * @param[in] value  Attribute value payload
 *
 * @return Command status code:
 *  - @ref ATT_ERR_NO_ERROR: If attribute value update succeeds
 *  - @ref ATT_ERR_INVALID_HANDLE: If handle doesn't exist in database
 *  - @ref ATT_ERR_REQUEST_NOT_SUPPORTED: If attribute data not present in database or
 *                                        cannot be modified
 *  - @ref ATT_ERR_INVALID_ATTRIBUTE_VAL_LEN: If new value length exceeds maximum attribute
 *                              value length.
 *
 ****************************************************************************************
 */
uint8_t attmdb_att_set_value(uint16_t handle, att_size_t length, att_size_t offset, uint8_t* value);

/**
 ****************************************************************************************
 * @brief Retrieve attribute value Max Length
 *
 * @param[in]  elmt    Attribute element information
 * @param[out] length  Max length Size of attribute value
 *
 * @return Command status code:
 *  - @ref ATT_ERR_NO_ERROR: If request succeeds
 *  - @ref ATT_ERR_REQUEST_NOT_SUPPORTED: If attribute is read only
 ****************************************************************************************
 */
uint8_t attmdb_get_max_len(struct attm_elmt* elmt, att_size_t* length);

/**
 ****************************************************************************************
 * @brief Retrieve attribute value

 *
 * @param[in]  handle Attribute handle.
 * @param[out] length Size of attribute value
 * @param[out] value  Pointer to attribute value payload
 *
 * @return Command status code:
 *  - @ref ATT_ERR_NO_ERROR: If request succeeds
 *  - @ref ATT_ERR_INVALID_HANDLE: If handle doesn't exist in database
 *  - @ref ATT_ERR_REQUEST_NOT_SUPPORTED: If attribute data not present in database
 ****************************************************************************************
 */
uint8_t attmdb_get_value(uint16_t handle, att_size_t* length, uint8_t** value);


/**
 ****************************************************************************************
 * @brief Retrieve attribute UUID
 *
 * @param[in]  elmt     Attribute information.
 * @param[out] uuid_len Size of attribute UUID
 * @param[out] uuid     UUID value to update
 * @param[in]  srv_uuid For a service, if set, return service UUID
 * @param[in]  air      Prepare UUID for the air (For a 32 bit UUID, returns a 128 bit UUID)
 *
 * @return Command status code:
 *  - @ref ATT_ERR_NO_ERROR: If request succeeds
 *  - @ref ATT_ERR_INVALID_HANDLE: If handle doesn't exist in database
 ****************************************************************************************
 */
uint8_t attmdb_get_uuid(struct attm_elmt *elmt, uint8_t* uuid_len, uint8_t* uuid, bool srv_uuid, bool air);

/**
 ****************************************************************************************
 * @brief Update attribute permission
 *
 * @param[in] handle Attribute handle.
 *  - @ref ATT_ERR_REQUEST_NOT_SUPPORTED: If attribute data not present in database
 * @param[in] perm   New attribute permission
 *
 * @return Command status code:
 *  - @ref ATT_ERR_NO_ERROR: If request succeeds
 *  - @ref ATT_ERR_INVALID_HANDLE: If handle doesn't exist in database
 *  - @ref ATT_ERR_REQUEST_NOT_SUPPORTED: If attribute permission is fixed
 ****************************************************************************************
 */
uint8_t attmdb_att_set_permission(uint16_t handle, att_perm_type perm);

/**
 ****************************************************************************************
 * @brief Retrieve attribute permission
 * If access mask is set, service authentication or encryption key size value can be loaded.
 *
 * @param[in]  handle      Attribute handle.
 * @param[out] perm        Permission value to return
 * @param[in]  access_mask Permission Access mask to check only specific permission
 *                         parameter (0 return full attribute permission)
 * @param[in|out] elmt     Attribute information
 *
 * @return Command status code:
 *  - @ref ATT_ERR_NO_ERROR: If request succeeds
 *  - @ref ATT_ERR_INVALID_HANDLE: If handle doesn't exist in database
 ****************************************************************************************
 */
uint8_t attmdb_att_get_permission(uint16_t handle, att_perm_type* perm, att_perm_type access_mask, struct attm_elmt *elmt);

/**
 ****************************************************************************************
 * @brief Reset some permissions bit in the Handle passed as parameter.
 *
 * @param[in] handle      Attribute handle.
 * @param[in] access_mask Access mask of permission to update
 * @param[in] perm        New value of the permission to update
 *
 *
 * @return Command status code:
 *  - @ref ATT_ERR_NO_ERROR: If request succeeds
 *  - @ref ATT_ERR_INVALID_HANDLE: If handle doesn't exist in database
 *  - @ref ATT_ERR_REQUEST_NOT_SUPPORTED: If attribute permission is fixed
 ****************************************************************************************
 */
uint8_t attmdb_att_update_perm(uint16_t handle, att_perm_type access_mask, att_perm_type perm);

/**
 ****************************************************************************************
 * @brief Update attribute service permission
 *
 * @param[in] handle Attribute handle.
 * @param[in] perm   New attribute permission
 *
 * @return Command status code:
 *  - @ref ATT_ERR_NO_ERROR: If request succeeds
 *  - @ref ATT_ERR_INVALID_HANDLE: If handle doesn't exist in database
 ****************************************************************************************
 */
uint8_t attmdb_svc_set_permission(uint16_t handle, uint8_t perm);

/**
 ****************************************************************************************
 * @brief Retrieve attribute service permission
 *
 * @param[in]  handle Attribute handle.
 * @param[out] perm   Permission value to return
 *
 * @return Command status code:
 *  - @ref ATT_ERR_NO_ERROR: If request succeeds
 *  - @ref ATT_ERR_INVALID_HANDLE: If handle doesn't exist in database
 ****************************************************************************************
 */
uint8_t attmdb_svc_get_permission(uint16_t handle, uint8_t* perm);

/**
 ****************************************************************************************
 * @brief Initialize Attribute Database (clear it)
 *
 * @param[in] reset  true if it's requested by a reset; false if it's boot initialization
 ****************************************************************************************
 */
void attmdb_init(bool reset);

#if (BLE_DEBUG)

/**
 ****************************************************************************************
 * @brief Retrieve number of services.
 *
 * @return number of services
 ****************************************************************************************
 */
uint8_t attmdb_get_nb_svc(void);

/**
 ****************************************************************************************
 * @brief Retrieve services informations
 *
 * @param[in] svc_info Services information array to update
 ****************************************************************************************
 */
void attmdb_get_svc_info(struct gattm_svc_info* svc_info);
#endif /* (BLE_DEBUG) */


/**
 ****************************************************************************************
 * @brief Function use to ease service database creation.
 * Use @see attmdb_add_service function of attmdb module to create service database,
 * then use @see attmdb_add_attribute function of attmdb module to create attributes
 * according to database description array given in parameter.
 *
 * @note: database description array shall be const to reduce memory consumption (only ROM)
 * @note: It supports only 16 bits UUIDs
 *
 * @note: If shdl = 0, it return handle using first available handle (shdl is
 * modified); else it verifies if start handle given can be used to allocates handle range.
 *
 * @param[in|out] shdl          Service start handle.
 * @param[in]     uuid          Service UUID
 * @param[in|out] cfg_flag      Configuration Flag, each bit matches with an attribute of
 *                              att_db (Max: 32 attributes); if the bit is set to 1, the
 *                              attribute will be added in the service.
 * @param[in]     max_nb_att    Number of attributes in the service
 * @param[in|out] att_tbl       Array which will be fulfilled with the difference between
 *                              each characteristic handle and the service start handle.
 *                              This array is useful if several characteristics are optional
 *                              within the service, can be set to NULL if not needed.
 * @param[in]     dest_id       Task ID linked to the service. This task will be notified
 *                              each time the service content is modified by a peer device.
 * @param[in|out] att_db        Table containing all attributes information
 * @param[in]     svc_perm      Service permission (@see enum attm_svc_perm_mask)
 *
 * @return Command status code:
 *  - @ref ATT_ERR_NO_ERROR: If database creation succeeds.
 *  - @ref ATT_ERR_INVALID_HANDLE: If start_hdl given in parameter + nb of attribute override
 *                            some existing services handles.
 *  - @ref ATT_ERR_INSUFF_RESOURCE: There is not enough memory to allocate service buffer.
 *                           or of new attribute cannot be added because all expected
 *                           attributes already added or buffer overflow detected during
 *                           allocation
 ****************************************************************************************
 */
uint8_t attm_svc_create_db(uint16_t *shdl, uint16_t uuid, uint8_t *cfg_flag, uint8_t max_nb_att,
                           uint8_t *att_tbl, ke_task_id_t const dest_id,
                           const struct attm_desc *att_db, uint8_t svc_perm);
/**
 ****************************************************************************************
 * @brief Function use to verify if several services can be allocated on a contiguous
 * handle range. If this command succeed, it means that service allocation will succeed.
 *
 * If start_hdl = 0, it return handle using first available handle (start_hdl is
 * modified); else it verifies if start handle given can be used to allocates handle range.
 *
 * @param[in|out] start_hdl     Service start handle.
 * @param[in]     nb_att        Number of handle to allocate (containing service handles)
 *
 * @return Command status code:
 *  - @ref ATT_ERR_NO_ERROR: If service allocation succeeds.
 *  - @ref ATT_ERR_INVALID_HANDLE: If start_hdl given in parameter or UUIDs value invalid
 ****************************************************************************************
 */
uint8_t attmdb_reserve_handle_range(uint16_t* start_hdl, uint8_t nb_att);


#endif // #if (BLE_ATTS)

/// @} ATTDB
#endif // ATTM_DB_H_
