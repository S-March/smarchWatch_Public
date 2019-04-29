/**
 ****************************************************************************************
 *
 * @file dis.h
 *
 * @brief Device Information Service sample implementation API
 *
 * Copyright (C) 2015 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef DIS_H_
#define DIS_H_

#include <stdint.h>
#include "ble_service.h"

/**
 * Device Information Service system ID
 */
typedef struct {
        uint8_t manufacturer[5];
        uint8_t oui[3];
} dis_system_id_t;

/**
 * Device Information Service PNP ID
 */
typedef struct {
        uint8_t   vid_source;
        uint16_t  vid;
        uint16_t  pid;
        uint16_t  version;
} dis_pnp_id_t;

/**
 * Device Information Service device information
 */
typedef struct {
        const char              *manufacturer;          /**< Manufacturer Name String */
        const char              *model_number;          /**< Model Number String */
        const char              *serial_number;         /**< Serial Number String */
        const char              *hw_revision;           /**< Hardware Revision String */
        const char              *fw_revision;           /**< Firmware Revision String */
        const char              *sw_revision;           /**< Software Revision String */
        const dis_system_id_t   *system_id;             /**< System ID */
        uint16_t                reg_cert_length;        /**< Regulatory Certification Length */
        const uint8_t           *reg_cert;              /**< Regulatory Certification */
        const dis_pnp_id_t      *pnp_id;                /**< PnP ID */
} dis_device_info_t;

/**
 * \brief Register Device Information Service instance
 *
 * Function registers Device Information Service.
 *
 * \param [in] config           general service configuration
 * \param [in] info             general information about device
 *
 * \return service instance
 *
 */
ble_service_t *dis_init(const ble_service_config_t *config, const dis_device_info_t *info);

#endif /* DIS_H_ */
