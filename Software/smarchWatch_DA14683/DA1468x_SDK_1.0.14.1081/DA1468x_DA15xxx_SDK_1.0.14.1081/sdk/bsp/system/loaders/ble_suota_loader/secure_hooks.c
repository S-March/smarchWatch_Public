/**
 ****************************************************************************************
 *
 * @file secure_hooks.c
 *
 * @brief Secure bootloader's hooks
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#include <stdbool.h>
#include <stddef.h>
#include "hw_aes_hash.h"
#include "hw_ecc.h"
#include "hw_ecc_curves.h"
#include "hw_ecc_ucode.h"
#include "suota_security_ext.h"
#include "ed25519.h"
#include "secure_hooks.h"
#include "main_secure.h"

/* Hash max length */
#define HASH_MAX_LENGTH         32

/* 2kB buffer - used by ECC engine */
static uint8_t ecc_ram_base[2048] __attribute__((aligned(1024)));

/* Actions for update image */
static bool secure_boot_init_failure_hook_update_image(failure_reason_t reason)
{
        switch (reason) {
        case FAILURE_REASON_INVALID_IMAGE_SIGNATURE:
                /*
                 * Updated image could be invalidated or an invalid image (e.g. SUOTA 1.1 image)
                 * has been written - do nothing (this image won't be used anyway).
                 */
                return false;
        case FAILURE_REASON_FW_VERSION_UPDATE_LOWER_THAN_CURRENT:
                /*
                 * Update image FW version number is lower than version number of the current image.
                 * This is not permissible situation, FW version should be greater than or equal to
                 * the minimum FW version - do nothing (use this image).
                 */
                return true;
        case FAILURE_REASON_CRC_MISMATCH:
                /*
                 * CRC placed in an image header does not match the CRC computed for that image.
                 * Full image has not been sent or some error occurred during SUOTA. Invalidate
                 * this image.
                 */
        case FAILURE_REASON_SEC_EXTENSION_INVALID:
                /*
                 * Security extension in image header is invalid - invalidate the whole image.
                 */
        case FAILURE_REASON_INVALID_ROOT_KEY:
                /*
                 * Cannot use specified root key for validation image's signature -
                 * invalidate image.
                 */
        case FAILURE_REASON_IMAGE_SIGNATURE:
                /* Image signature is invalid - invalidate image. */
        case FAILURE_REASON_FW_VERSION_MISMATCH:
                /*
                 * The FW version string (placed in the SUOTA 1.1 header) and the version
                 * number (placed in the security extension) do not match. Some invalid
                 * data may have been put in the header and/or in the security
                 * extension - invalidate image.
                 */
        case FAILURE_REASON_FW_VERSION_TOO_LOW:
                /*
                 * The FW version of the image is lower than required minimum (this value
                 * is stored in the OTP memory). This image shouldn't be put on the exec.
                 * partition - invalidate image.
                 */
                invalidate_update_image();
                return false;
        default:
                return true;
        }
}

/* Actions for current executable image */
static bool secure_boot_init_failure_hook_exec_image(failure_reason_t reason)
{
        switch (reason) {
        case FAILURE_REASON_INVALID_IMAGE_SIGNATURE:
                /*
                 * Some bad data has been written to the header partition e.g. during SUOTA update
                 * or initialization flash - perform platform reboot.
                 */
        case FAILURE_REASON_CRC_MISMATCH:
                /*
                 * CRC placed in an image header does not match to the CRC computed for that image.
                 * Perhaps the image header does not match the image - perform platform reboot.
                 */
        case FAILURE_REASON_IMAGE_INSANE:
                /*
                 * Application's reset vector is broken and it cannot be started - perform platform
                 * reboot.
                 */
        case FAILURE_REASON_SEC_EXTENSION_INVALID:
                /*
                 * Security extension in image header is invalid - perform a platform reboot.
                 */
        case FAILURE_REASON_INVALID_ROOT_KEY:
                /*
                 * Cannot use specified root key for validation image's signature -
                 * perform a platform reboot.
                 */
        case FAILURE_REASON_IMAGE_SIGNATURE:
                /* Image signature is invalid - perform a platform reboot. */
        case FAILURE_REASON_FW_VERSION_ARRAY_BROKEN:
                /*
                 * Some invalid data is placed in the minimum FW version array or it cannot
                 * be read - perform a platform reboot.
                 */
        case FAILURE_REASON_FW_VERSION_MISMATCH:
                /*
                 * The FW version string (placed in the SUOTA 1.1 header) and the version
                 * number (placed in the security extension) does not match. Some invalid
                 * data could be put in the header and/or in the security extension -
                 * perform a platform reboot.
                 */
        case FAILURE_REASON_FW_VERSION_TOO_LOW:
                /*
                 * The FW version of the image is lower than required minimum (this value
                 * is stored in the OTP memory). This image shouldn't be put on the exec.
                 * partition - perform a platform reboot.
                 */
                /* This is an unrecoverable error - entering continuous reset cycle */
                trigger_reboot();
                return false;
        default:
                return true;
        }
}

/* Actions for device */
static bool secure_boot_init_failure_hook_device(failure_reason_t reason)
{
        switch (reason) {
        case FAILURE_REASON_FW_VERSION_ARRAY_EMPTY:
                /*
                 * Minimum FW version array is empty - perform first write (version number will be
                 * get from the header partition). Don't perform a platform reboot.
                 */
                write_first_min_version_from_header_part();
                return true;
        case FAILURE_REASON_EMPTY_SYMMETRIC_KEYS:
                /*
                 * Symmetric keys have been not written - generate them (and write to the OTP
                 * memory with bit inversions). Don't perform a platform reboot.
                 */
                generate_symmetric_keys();
                return true;
        case FAILURE_REASON_FW_VERSION_ARRAY_BROKEN:
                /*
                 * Some invalid data is placed in the minimum FW version array or it cannot
                 * be read - perform a platform reboot.
                 */
        case FAILURE_BOOTLOADER_CRC_MISMATCH:
                /*
                 * CRC placed in OTP header is different than CRC computed for bootloader which is
                 * also placed in the OTP. Some of these value could be altered - perform platform
                 * reboot.
                 */
        case FAILURE_REASON_INVALID_ROOT_KEYS:
                /*
                 * No valid root key is stored in the OTP memory. They could be invalidated,
                 * not written properly or they cannot be read.
                 */
        case FAILURE_REASON_INVALID_SYMMETRIC_KEYS:
                /*
                 * No valid symmetric key is stored in the OTP memory and no empty key
                 * placeholder exists - perform platform reboot.
                 */
                trigger_reboot();
                return false;
        default:
                return true;
        }
}

/*
 * This function assumes that the input data is split onto two parts: device administration section
 * and exec (application binary). Return length of computed hash if ok, 0 otherwise. 'hash' buffers
 * must have at least HASH_MAX_LENGTH bytes in length.
 */
static uint16_t hash_data(security_hdr_hash_t hash_method, const uint8_t *dev_adm,
                                        size_t dev_adm_len, const uint8_t *exec, size_t exec_len,
                                                                                uint8_t *hash)
{
        uint8_t first_size = 0;
        uint8_t second_size = 0;
        uint8_t buffer[8];
        uint16_t length = 0;

        memset(hash, 0, HASH_MAX_LENGTH);

        hw_aes_hash_enable_clock();

        /* Configure proper mode */
        switch (hash_method) {
        case SECURITY_HDR_HASH_SHA_224:
                length = 28;
                hw_aes_hash_cfg_sha_224(length);
                break;
        case SECURITY_HDR_HASH_SHA_256:
                length = 32;
                hw_aes_hash_cfg_sha_256(length);
                break;
        case SECURITY_HDR_HASH_SHA_384:
                /* ECC engine doesn't support bigger operands than 32 bytes - cut result */
                length = 32;
                hw_aes_hash_cfg_sha_384(length);
                break;
        case SECURITY_HDR_HASH_SHA_512:
                /* ECC engine doesn't support bigger operands than 32 bytes - cut result */
                length = 32;
                hw_aes_hash_cfg_sha_512(length);
                break;
        default:
                goto done;
        }

        /* Load inputs from different locations */
        hw_aes_hash_mark_input_block_as_not_last();

        /*
         * AES/HASH engine requires that the not last data block must have length which is multiple
         * of 8 for SHA - get few last bytes from device administration section part and few first
         * bytes from exec part.
         */
        if (dev_adm_len % 8) {
                first_size = dev_adm_len % 8;
                second_size = 8 - dev_adm_len % 8;

                memcpy(buffer, dev_adm + dev_adm_len - first_size, first_size);
                memcpy(&buffer[first_size], exec, second_size);
        }


        /* Load device administration section */
        hw_aes_hash_cfg_dma(dev_adm, NULL, dev_adm_len - first_size);

        if (hw_aes_hash_check_restrictions()) {
                return false;
        }

        hw_aes_hash_start();

        while (!hw_aes_hash_wait_for_in()) {
        }

        if (first_size) {
                /* Load glued data */
                hw_aes_hash_cfg_dma(buffer, NULL, 8);

                if (hw_aes_hash_check_restrictions()) {
                        return false;
                }

                hw_aes_hash_start();

                while (!hw_aes_hash_wait_for_in()) {
                }
        }

        /* Load the last input - exec data and compute sha */
        hw_aes_hash_mark_input_block_as_last();
        hw_aes_hash_cfg_dma(exec + second_size, hash, exec_len - second_size);
        hw_aes_hash_start();

        while (hw_aes_hash_is_active()) {};

done:
        hw_aes_hash_disable_clock();

        return length;
}

/* Buffers are 32 bytes - leading zeros are placed before data */
static bool ecc_val_signature(security_hdr_ecc_curve_t curve, const uint8_t *hash, const uint8_t *x,
                                        const uint8_t *y, const uint8_t *r, const uint8_t *s)
{
        volatile unsigned int ecc_status = HW_ECC_STATUS_BUSY;

        if (!hash || !x || !y || !r || !s) {
                return false;
        }

        /* Configure ECC engine and check ECDSA signature */
        hw_ecc_enable_clock();
        hw_ecc_load_ucode(hw_ecc_ucode1, HW_ECC_UCODE1_SIZE);
        hw_ecc_disable_clock();
        hw_ecc_set_base_addr(ecc_ram_base);

        switch (curve) {
        case SECURITY_HDR_ECC_CURVE_SECP192R1:
                hw_ecc_p192_load_params(ecc_ram_base);
                break;
        case SECURITY_HDR_ECC_CURVE_SECP224R1:
                hw_ecc_p224_load_params(ecc_ram_base);
                break;
        case SECURITY_HDR_ECC_CURVE_SECP256R1:
                hw_ecc_p256_load_params(ecc_ram_base);
                break;
        default:
                /* Unsupported elliptic curve */
                hw_ecc_disable_clock();
                return false;
        }

        hw_ecc_write256_r(8, x, ecc_ram_base);
        hw_ecc_write256_r(9, y, ecc_ram_base);
        hw_ecc_write256_r(10, r, ecc_ram_base);
        hw_ecc_write256_r(11, s, ecc_ram_base);
        hw_ecc_write256_r(12, hash, ecc_ram_base);
        hw_ecc_write_command_register(HW_ECC_CMD_CALCR2_TRUE,
                                      HW_ECC_CMD_SIGNB_POS,
                                      HW_ECC_CMD_SIGNA_POS,
                                      HW_ECC_CMD_OP_SIZE_256B,
                                      HW_ECC_CMD_FIELD_FP,
                                      HW_ECC_CMD_OP_ECDSA_VER_SIG);
        hw_ecc_enable_clock();
        hw_ecc_start();

        do {
                ecc_status = hw_ecc_read_status();
        } while (ecc_status & HW_ECC_STATUS_BUSY);

        hw_ecc_disable_clock();

        return ecc_status == 0;
}

bool secure_boot_failure_hook(failure_reason_t reason, failure_source_t source)
{
        switch(source) {
        case FAILURE_SOURCE_DEVICE:
                return secure_boot_init_failure_hook_device(reason);
        case FAILURE_SOURCE_UPDATE_IMAGE:
                return secure_boot_init_failure_hook_update_image(reason);
        case FAILURE_SOURCE_EXEC_IMAGE:
                return secure_boot_init_failure_hook_exec_image(reason);
        default:
                /* Unknown source - something is wrong */
                return false;
        }
}

bool verify_signature_hook(security_hdr_mode_t mode, security_hdr_ecc_curve_t curve,
                                security_hdr_hash_t hash_method, const uint8_t *public_key,
                                size_t public_key_len, const uint8_t *dev_adm, size_t dev_adm_len,
                                const uint8_t *exec, size_t exec_len, const uint8_t *signature,
                                                                        size_t signature_len)
{
        uint8_t public_key_cp[ASYMMETRIC_KEY_MAX_LEN] = { 0 };
        uint8_t signature_cp[SIGNATURE_MAX_LENGTH] = { 0 };
        uint16_t hash_len;
        uint8_t hash[32] = { 0 };

        /* Only ECDSA and EdDSA signatures are supported */
        if (mode != SECURITY_HDR_MODE_ECDSA && mode != SECURITY_HDR_MODE_EDDSA) {
                return false;
        }

        if (mode == SECURITY_HDR_MODE_EDDSA) {
                /* Check public key and signature lengths and the hash method (it must be SHA-512) */
                if (public_key_len != 32 || signature_len != 64 ||
                                                        hash_method != SECURITY_HDR_HASH_SHA_512) {
                        return false;
                }

                /* Initialize ECC engine for Ed25519 signature verification */
                ed25519_init(ecc_ram_base);

                /* Check Ed25519 signature for given raw data (it shouldn't be hashed) */
                return ed25519_image_sig_verification(dev_adm, dev_adm_len, exec, exec_len,
                                                                        public_key, signature);
        }

        /*
         * Add leading zeros to x and y parts of the public key if it is shorter than maximum length
         * e.g. xxxxxxxxyyyyyyyy00000 -> 00xxxxxxxx00yyyyyyyy. This is required by the ECC engine.
         */
        if (public_key_len < ASYMMETRIC_KEY_MAX_LEN) {
                size_t half_max_size = ASYMMETRIC_KEY_MAX_LEN / 2;
                size_t half_size = public_key_len / 2;

                memset(public_key_cp, 0, sizeof(public_key_cp));

                /* Move x and y parts to proper places */
                memcpy(public_key_cp + (half_max_size - half_size), public_key, half_size);
                memcpy(public_key_cp + (ASYMMETRIC_KEY_MAX_LEN - half_size), public_key + half_size,
                                                                                        half_size);
        } else {
                memcpy(public_key_cp, public_key, ASYMMETRIC_KEY_MAX_LEN);
        }

        /* Compute hash from the data */
        hash_len = hash_data(hash_method, dev_adm, dev_adm_len, exec, exec_len, hash);

        if (hash_len == 0) {
                return false;
        }

        /*
         * If hash length is greater than maximum length supported by specified elliptic curve
         * then reduce it.
         */
        hash_len = (hash_len > (public_key_len / 2)) ? (public_key_len / 2) : hash_len;

        /*
         * Move hash to the end of the buffer and set to zero first bytes - this is required by the
         * ECC engine.
         */
        if (hash_len < HASH_MAX_LENGTH) {
                memmove(hash + (HASH_MAX_LENGTH - hash_len), hash, hash_len);
                memset(hash, 0, HASH_MAX_LENGTH - hash_len);
        }

        /*
         * Add leading zeros to r and s parts of the signature if it is shorter than maximum length.
         * This is required by the ECC engine.
         */
        if (signature_len < SIGNATURE_MAX_LENGTH) {
                size_t half_max_size = SIGNATURE_MAX_LENGTH / 2;
                size_t half_size = signature_len / 2;

                memset(signature_cp, 0, sizeof(signature_cp));

                /* Copy r and s parts to the temporary buffer  */
                memcpy(signature_cp + (half_max_size - half_size), signature, half_size);
                memcpy(signature_cp + (SIGNATURE_MAX_LENGTH - half_size), signature + half_size,
                                                                                        half_size);
        } else {
                memcpy(signature_cp, signature, SIGNATURE_MAX_LENGTH);
        }

        return ecc_val_signature(curve, hash, public_key_cp, public_key_cp +
                                        ASYMMETRIC_KEY_MAX_LEN / 2, signature_cp, signature_cp +
                                                                        SIGNATURE_MAX_LENGTH / 2);
}

int compare_version_hook(const security_hdr_fw_version_t *version_1,
                                                        const security_hdr_fw_version_t *version_2)
{
        /* Version numbers parts are in little-endian */
        if (version_1->major < version_2->major) {
                return -1;
        }  else if (version_1->major > version_2->major) {
                return 1;
        }

        if (version_1->minor < version_2->minor) {
                return -1;
        }  else if (version_1->minor > version_2->minor) {
                return 1;
        }

        /* Version numbers are the same */
        return 0;
}

void update_version_hook(const security_hdr_fw_version_t *version)
{
        security_hdr_fw_version_t old_ver[2];
        security_hdr_fw_version_t new_ver[2];
        uint32_t ver_address;
        int i;

        /* Find empty place in minimum FW version array */
        for (i = 0; i < MIN_FW_VERSION_ENTRIES_NUMBER; i++) {
                /*
                 * Read version entry from OTP. Sometimes read cannot be performed - assume that
                 * the entry is overwritten/invalid.
                 */
                if (!read_otp(MIN_FW_VERSION_AREA_ADDRESS + i * MIN_FW_VERSION_LEN,
                                                        (uint8_t *) old_ver, MIN_FW_VERSION_LEN)) {
                        continue;
                }

                if (old_ver[0].major == 0 && old_ver[0].minor == 0 && old_ver[1].major == 0 &&
                                                                        old_ver[1].minor == 0) {
                        break;
                }
        }

        if (i == MIN_FW_VERSION_ENTRIES_NUMBER) {
                /*
                 * The last entry in the minimum FW version array cannot be altered - this is not
                 * an error.
                 */
                return;
        }

        new_ver[0] = *version;
        new_ver[1].major = ~version->major;
        new_ver[1].minor = ~version->minor;

        ver_address = MIN_FW_VERSION_AREA_ADDRESS + i * MIN_FW_VERSION_LEN;

        /* Write new FW version and its bit inversion */
        write_otp(ver_address, (uint8_t *) new_ver, MIN_FW_VERSION_LEN);
}
