/**
 ****************************************************************************************
 *
 * @file ipss.h
 *
 * @brief IP Support Service API
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef IPSS_H_
#define IPSS_H_

#include "ble_service.h"

/**
 * \brief Register IP Support Service instance
 *
 * Function registers IP Support Service.
 *
 * \return service instance
 *
 */
ble_service_t *ipss_init(void);

#endif /* IPSS_H_ */
