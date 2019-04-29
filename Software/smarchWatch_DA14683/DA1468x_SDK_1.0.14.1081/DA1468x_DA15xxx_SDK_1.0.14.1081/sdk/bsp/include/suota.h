/**
 * \addtogroup BSP
 * \{
 * \addtogroup SYSTEM
 * \{
 * \addtogroup SUOTA
 * 
 * \brief SUOTA structure definitions
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file suota.h
 *
 * @brief SUOTA structure definitions
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SUOTA_H_
#define SUOTA_H_

#include <stdint.h>

#define SUOTA_VERSION_1_1       11
#define SUOTA_VERSION_1_2       12
#define SUOTA_VERSION_1_3       13
#define SUOTA_VERSION_1_4       14      // Security extension

#ifndef SUOTA_VERSION
#define SUOTA_VERSION           SUOTA_VERSION_1_3
#endif

#if SUOTA_PSM && (SUOTA_VERSION < SUOTA_VERSION_1_2)
#       error "SUOTA_PSM is only applicable to SUOTA_VERSION >= 1.2"
#endif

/**
 * \struct suota_1_1_product_header_t;
 *
 * \brief SUOTA 1.1 product header as defined by Dialog SUOTA specification.
 *
 * \note the same header is used for any SUOTA version newer than 1.1
 *
 */
typedef struct {
        uint8_t signature[2];
        uint16_t flags;
        uint32_t current_image_location;
        uint32_t update_image_location;
        uint8_t reserved[8];
} __attribute__((packed)) suota_1_1_product_header_t;

#define SUOTA_1_1_PRODUCT_HEADER_SIGNATURE_B1   0x70
#define SUOTA_1_1_PRODUCT_HEADER_SIGNATURE_B2   0x62

/**
 * \struct suota_1_1_image_header_t
 *
 * \brief SUOTA 1.1 image header as defined by Dialog SUOTA specification.
 *
 * \note the same header is used for any SUOTA version newer than 1.1
 *
 */
typedef struct {
        uint8_t signature[2];
        uint16_t flags;
        uint32_t code_size;
        uint32_t crc;
        uint8_t version[16];
        uint32_t timestamp;
        uint32_t exec_location;
} __attribute__((packed)) suota_1_1_image_header_t;

#define SUOTA_1_1_IMAGE_HEADER_SIGNATURE_B1     0x70
#define SUOTA_1_1_IMAGE_HEADER_SIGNATURE_B2     0x61

#define SUOTA_1_1_IMAGE_FLAG_FORCE_CRC          0x01
#define SUOTA_1_1_IMAGE_FLAG_VALID              0x02
#define SUOTA_1_1_IMAGE_FLAG_RETRY1             0x04
#define SUOTA_1_1_IMAGE_FLAG_RETRY2             0x08

typedef suota_1_1_image_header_t suota_image_header_t;

#endif /* SUOTA_H_ */

/**
 * \}
 * \}
 * \}
 */
