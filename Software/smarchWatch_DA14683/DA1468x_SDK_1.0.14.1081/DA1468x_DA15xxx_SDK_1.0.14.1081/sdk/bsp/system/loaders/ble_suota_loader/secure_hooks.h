/**
 ****************************************************************************************
 *
 * @file secure_hooks.h
 *
 * @brief Secure bootloader's hooks - API
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SECURE_HOOKS_H_
#define SECURE_HOOKS_H_

#include "suota_security_ext.h"

/**
 * Reason of initialization failure
 */
typedef enum {
        /** Dummy value - there is no error */
        FAILURE_REASON_NO_ERROR                         = 0,
        /**
         * CRC placed in the OTP header doesn't match the CRC computed for Secure Boot Loader
         * image placed in the OTP
         */
        FAILURE_BOOTLOADER_CRC_MISMATCH,
        /**
         * Two first bytes in the image header (image signature) is not valid - they don't
         * match the SUOTA 1.4 images signature
         */
        FAILURE_REASON_INVALID_IMAGE_SIGNATURE,
        /**
         * CRC placed in the image header doesn't match the CRC computed for given application
         * image
         */
        FAILURE_REASON_CRC_MISMATCH,
        /**
         * Image's reset vector is insane - its address is lower than the image address or
         * THUMB instruction address is even. This check could be done only for images placed on
         * the exec. partition.
         */
        FAILURE_REASON_IMAGE_INSANE,
        /**
         * Security extension in image header is invalid: one or more mandatory entries are not
         * included, some entry is not valid or mandatory field is duplicated.
         */
        FAILURE_REASON_SEC_EXTENSION_INVALID,
        /**
         * Root key which should be used for validating image signature is invalid or cannot be
         * read.
         */
        FAILURE_REASON_INVALID_ROOT_KEY,
        /**
         * Validation of the image signature (created using e.g. ECDSA - do not confuse with 2 first
         * bytes in header) failed. Possible reasons: application image has been modified or
         * replaced, header comes from another image, during signature generation mismatched private
         * key, different hash method or different curve has been used.
         */
        FAILURE_REASON_IMAGE_SIGNATURE,
        /**
         * Firmware version placed in the image header (as string) does not match the firmware
         * version number placed in the security extension.
         */
        FAILURE_REASON_FW_VERSION_MISMATCH,
        /**
         * Firmware version number of this image is lower than required minimum (value stored in
         * the OTP memory).
         */
        FAILURE_REASON_FW_VERSION_TOO_LOW,
        /**
         * Firmware version number of the update image is lower than firmware version number of the
         * current image.
         */
        FAILURE_REASON_FW_VERSION_UPDATE_LOWER_THAN_CURRENT,
        /**
         * The minimum firmware version array contains invalid value.
         */
        FAILURE_REASON_FW_VERSION_ARRAY_BROKEN,
        /**
         * The minimum firmware version array has been not initialized earlier - there is no value
         * in it.
         */
        FAILURE_REASON_FW_VERSION_ARRAY_EMPTY,
        /**
         * There is no valid root keys in the OTP memory. They have been invalidated or not written
         * properly.
         */
        FAILURE_REASON_INVALID_ROOT_KEYS,
        /**
         * There is no valid symmetric key in the OTP memory and symmetric key area is not empty.
         */
        FAILURE_REASON_INVALID_SYMMETRIC_KEYS,
        /**
         * Symmetric keys have been not written yet.
         */
        FAILURE_REASON_EMPTY_SYMMETRIC_KEYS,
} failure_reason_t;

/**
 * Source, which triggers failure hook
 */
typedef enum {
        /** Device integrity checks */
        FAILURE_SOURCE_DEVICE,
        /** Update FW validation */
        FAILURE_SOURCE_UPDATE_IMAGE,
        /** Current executable FW validation */
        FAILURE_SOURCE_EXEC_IMAGE,
} failure_source_t;

/**
 * \brief Secure Boot Loader failure hook
 *
 * This function should define failure actions of the secure bootloader initialization/startup
 * procedure for different failure reasons. It may handle the same reason differently
 * depending on the source.
 *
 * \param [in] reason   the reason of the hook call
 * \param [in] source   the source of the hook call (device/update image/current image)
 *
 * \return true if procedure, which called this hook (e.g FW validation, device integrity check)
 *         should be continued, false otherwise
 */
bool secure_boot_failure_hook(failure_reason_t reason, failure_source_t source);

/**
 * \brief Verify image's digital signature
 *
 * This function verifies digital signature of the image. Signature must cover device administration
 * section (part of the header) and the executable binary. These two parts could be placed
 * discontinuously considering partitioning of the flash memory (header and exec. partitions).
 *
 * \note \p dev_adm should include 0xFF padding to the 1024 bytes boundary (counting from the
 * header beginning).
 *
 * \param [in] mode             signature verification mode (algorithm ID)
 * \param [in] curve            elliptic curve ID
 * \param [in] hash_method      hash method ID
 * \param [in] public_key       buffer with public key
 * \param [in] public_key_len   public key length (in bytes)
 * \param [in] dev_adm          device administration section content
 * \param [in] dev_adm_len      device administration section length
 * \param [in] exec             application executable
 * \param [in] dev_adm_len      application executable length
 * \param [in] signature        signature
 * \param [in] signature_len    signature length
 *
 * \return true if all arguments are valid and the signature is valid for given parameters, false
 *         otherwise
 */
bool verify_signature_hook(security_hdr_mode_t mode, security_hdr_ecc_curve_t curve,
                                security_hdr_hash_t hash_method, const uint8_t *public_key,
                                size_t public_key_len, const uint8_t *dev_adm, size_t dev_adm_len,
                                const uint8_t *exec, size_t exec_len, const uint8_t *signature,
                                                                        size_t signature_len);

/**
 * \brief Compare version numbers
 *
 * \param [in] version_1        first version number
 * \param [in] version_2        second version number
 *
 * \return value < 0 if first version number is lower than the second, 0 if they are equal and
 *         value > 0 if the second version number is greater than the first one
 */
int compare_version_hook(const security_hdr_fw_version_t *version_1,
                                                        const security_hdr_fw_version_t *version_2);

/**
 * \brief Update minimum FW version array
 *
 * The function writes new entry to the minimum firmware version array. If new minimum version is
 * the same as (or lower than) previously written or there is no space for the new entry in the
 * array then write won't be performed.
 *
 * \note This hook could be used for writing the first minimum FW version to the array.
 *
 * \param [in] version          new min. FW version
 *
 */
void update_version_hook(const security_hdr_fw_version_t *version);

#endif /* SECURE_HOOKS_H_ */
