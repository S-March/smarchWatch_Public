/**
 ****************************************************************************************
 *
 * @file tps.h
 *
 * @brief Tx Power Service implementation API
 *
 * Copyright (C) 2015 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef TPS_H_
#define TPS_H_

#include <stdint.h>
#include "ble_service.h"

/**
 * Register Tx Power Service instance
 *
 * \param [in] level  TX power level
 *
 * \return service instance
 *
 */
ble_service_t *tps_init(int8_t level);

#endif /* TPS_H_ */
