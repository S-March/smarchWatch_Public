/**
 ****************************************************************************************
 *
 * @file dlg_debug.h
 *
 * @brief Debug service API
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef DLG_DEBUG_H_
#define DLG_DEBUG_H_

#include "ble_service.h"


/**
 * \brief Create debug handler
 *
 * \param [in] CAT              category
 * \param [in] CMD              command
 * \param [in] CB               application callback
 * \param [in] UD               user data
 *
 */
#define DLGDEBUG_HANDLER(CAT, CMD, CB, UD)      \
        {                                       \
                .cat = (CAT),                   \
                .cmd = (CMD),                   \
                .cb = (CB),                     \
                .ud = (UD)                      \
        }

/**
 * \brief Command handler callback
 *
 * \param [in] conn_idx         connection index
 * \param [in] argc             arguments count
 * \param [in] argv             string arguments
 * \param [in] ud               user data
 *
 */
typedef void (* dlgdebug_call_cb_t) (uint16_t conn_idx, int argc, char **argv, void *ud);

/**
 * Debug handler
 */
typedef struct {
        /** Category */
        const char *cat;
        /** Command */
        const char *cmd;
        /** Callback */
        dlgdebug_call_cb_t cb;
        /** User-data */
        void *ud;
} dlgdebug_handler_t;

/**
 * \brief Register DLG debug instance
 *
 * Function registers DLG debug service with UUID 6b559111-c4df-4660-818e-234f9e17b290.
 *
 * \param [in] cfg              general service configuration
 *
 * \return service instance
 *
 */
ble_service_t *dlgdebug_init(const ble_service_config_t *cfg);

/**
 * \brief Register handler
 *
 * Registers handler when command is written to CP characteristic of Debug Service.
 * Command has format "<cat> <cmd> <...>".
 * Characteristic UUID: 6b559111-c4df-4660-818e-234f9e17b291.
 *
 * \param [in] svc              service instance
 * \param [in] cat              category
 * \param [in] cmd              command
 * \param [in] cb               application callback
 * \param [in] ud               user data
 *
 */
void dlgdebug_register_handler(ble_service_t *svc, const char *cat, const char *cmd,
                                                                dlgdebug_call_cb_t cb, void *ud);

/**
 * \brief Register handlers
 *
 * Registers handlers array. See dlgdebug_register_handler function's description for more details.
 *
 * \param [in] svc              service instance
 * \param [in] len              array length
 * \param [in] handlers         debug handlers array
 *
 */
void dlgdebug_register_handlers(ble_service_t *svc, size_t len, const dlgdebug_handler_t *handlers);

/**
 * \brief Notification strings
 *
 * Send notification to connected peer.
 *
 * \param [in] svc              service instance
 * \param [in] conn_idx         connection index
 * \param [in] fmt              string data
 *
 */
void dlgdebug_notify_str(ble_service_t *svc, uint16_t conn_idx, const char *fmt, ...);

#endif /* DLG_DEBUG_H_ */
