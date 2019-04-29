/**
 ****************************************************************************************
 *
 * @file ias.h
 *
 * @brief Immediate Alert Service implementation API
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef IAS_H_
#define IAS_H_

#include <stdbool.h>
#include <stdint.h>
#include <ble_service.h>

/**
 * \brief IAS alert level callback
 *
 * \param [in] conn_idx         connection index
 * \param [in] level            alert level
 *
 */
typedef void (* ias_alert_level_cb_t) (uint16_t conn_idx, uint8_t level);

/**
 * \brief Register Immediate Alert Service instance
 *
 * Function registers IAS instance.
 * \p alert_level_cb is called when client writes new value to Alert Level characteristic
 *
 * \param [in] alert_level_cb   alert level callback
 *
 * \return service instance
 *
 */
ble_service_t *ias_init(ias_alert_level_cb_t alert_level_cb);

#endif /* IAS_H_ */
