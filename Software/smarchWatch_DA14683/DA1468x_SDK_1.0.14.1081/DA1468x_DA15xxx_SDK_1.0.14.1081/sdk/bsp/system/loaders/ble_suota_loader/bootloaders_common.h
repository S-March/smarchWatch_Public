/**
 ****************************************************************************************
 *
 * @file bootloaders_common.h
 *
 * @brief Common part for both bootloaders (BLE SUOTA Loader and Secure Boot) - API
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef BOOTLOADERS_COMMON_H_
#define BOOTLOADERS_COMMON_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * Offset of image header inside partition.
 */
#define SUOTA_IMAGE_HEADER_OFFSET       0

/**
 * Specify the GPIO to be used when CFG_FORCE_SUOTA_GPIO feature is enabled.
 * Default setup corresponds to the 'K1' button (P1_6). Used only for ble_suota_loader.
 */
#define CFG_FORCE_SUOTA_GPIO_PORT   (HW_GPIO_PORT_1)
#define CFG_FORCE_SUOTA_GPIO_PIN    (HW_GPIO_PIN_6)

/**
 * Define TRACE as empty call if dg_configDEBUG_TRACE is disabled
 */
#if dg_configDEBUG_TRACE
        #define TRACE(...) printf(__VA_ARGS__)
#else
        #define TRACE(...)
#endif

/**
 * \brief Configure hardware blocks used by the bootloader.
 */
void periph_init(void);

/**
 * \brief Cleanup configuration done by bootloader.
 */
void periph_deinit(void);

/**
 * \brief Reboot device using SW_RESET
 */
void reboot(void);

/**
 * \brief Read SUOTA 1.1 header from the specified partition
 *
 * \param [in] part             partition with header
 * \param [in] offset           offset on partition
 * \param [out] header          SUOTA 1.1 header
 *
 * \return true if the header has been read properly, false otherwise
 */
static inline bool read_image_header(nvms_t part, size_t offset, suota_1_1_image_header_t *header)
{
        if (!part) {
                return false;
        }

        return sizeof(*header) == ad_nvms_read(part, offset, (uint8_t *) header, sizeof(*header));
}

/**
 * \brief Check 'valid' flag and image signature
 *
 * \note Mainly this function should be used for checking that the image placed on 'update'
 * partition is ready for update.
 *
 * \param [in] header           image header
 *
 * \return true if 'valid' flag is true and the signature is valid, false otherwise
 */
bool image_ready(suota_1_1_image_header_t *header);

/**
 * \brief Check reset vector in image
 *
 * \param [in] image_address    address of the image
 *
 * return true if reset vector is valid
 */
bool image_sanity_check(const int32_t *image_address);

#endif /* BOOTLOADERS_COMMON_H_ */
