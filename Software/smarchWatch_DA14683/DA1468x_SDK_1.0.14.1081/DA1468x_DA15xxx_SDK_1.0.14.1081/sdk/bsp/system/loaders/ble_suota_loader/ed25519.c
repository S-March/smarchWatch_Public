/**
 ****************************************************************************************
 *
 * @file ed25519.c
 *
 * @brief Ed25519 signature verification
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <string.h>
#include "hw_aes_hash.h"
#include "hw_ecc_curves.h"
#include "hw_ecc_ucode.h"
#include "hw_ecc.h"
#include "ed25519.h"

/* Buffer in RAM which will be used by ECC engine */
static void *ecc_ram_base;

/* Compute hash (hash method: SHA-512) which is used in Ed25519 signature verification */
static bool sha512_verification(const uint8_t *r, const uint8_t *public_key,
                                const uint8_t *dev_adm_section, uint32_t dev_adm_section_size,
                                        const uint8_t *exec, uint32_t exec_size, uint8_t *result)
{
        uint8_t buffer[8];
        uint8_t first_size = 0;
        uint8_t second_size = 0;

        hw_aes_hash_enable_clock();

        /* Configure engine for SHA-512 */
        hw_aes_hash_cfg_sha_512(64);

        /* Load inputs from different locations */
        hw_aes_hash_mark_input_block_as_not_last();

        /* Load R (half of the signature) */
        hw_aes_hash_cfg_dma(r, NULL, 32);

        if (hw_aes_hash_check_restrictions()) {
                return false;
        }

        hw_aes_hash_start();

        while (!hw_aes_hash_wait_for_in()) {
        }

        /* Load public key */
        hw_aes_hash_cfg_dma(public_key, NULL, 32);

        if (hw_aes_hash_check_restrictions()) {
                return false;
        }

        hw_aes_hash_start();

        while (!hw_aes_hash_wait_for_in()) {
        }

        /*
         * AES/HASH engine requires that the not last data block must have length which is multiple
         * of 8 for SHA-512 - get few last bytes from device administration section part and few
         * first bytes from exec part.
         */
        if (dev_adm_section_size % 8) {
                first_size = dev_adm_section_size % 8;
                second_size = 8 - dev_adm_section_size % 8;

                memcpy(buffer, dev_adm_section + dev_adm_section_size - first_size, first_size);
                memcpy(&buffer[first_size], exec, second_size);
        }


        /* Load device administration section */
        hw_aes_hash_cfg_dma(dev_adm_section, NULL, dev_adm_section_size - first_size);

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
        hw_aes_hash_cfg_dma(exec + second_size, result, exec_size - second_size);

        hw_aes_hash_start();

        while (hw_aes_hash_is_active()) {
        }

        hw_aes_hash_disable_clock();

        return true;
}

/* Launch modular reduction from 512 to 256 bits (dst = src mod N) */
static int ed25519_reduce(const uint8_t *src, uint8_t *dst)
{
        volatile unsigned int ecc_status = HW_ECC_STATUS_BUSY;

        hw_ecc_write256(1, hw_ecc_edwards_curve25519_L, ecc_ram_base);
        hw_ecc_write256(6, src, ecc_ram_base);
        hw_ecc_write256(7, src + 32, ecc_ram_base);
        hw_ecc_write256(12, hw_ecc_edwards_curve25519_ed25519_red, ecc_ram_base);
        hw_ecc_cfg_ops(6, 7, 8);

        hw_ecc_write_command_register(HW_ECC_CMD_CALCR2_TRUE,
                                      HW_ECC_CMD_SIGNB_NEG,
                                      HW_ECC_CMD_SIGNA_POS,
                                      HW_ECC_CMD_OP_SIZE_256B,
                                      HW_ECC_CMD_FIELD_FP,
                                      HW_ECC_CMD_OP_EDDSA_MULTADDN);

        hw_ecc_enable_clock();
        hw_ecc_start();

        do {
                ecc_status = hw_ecc_read_status();
        } while (ecc_status & HW_ECC_STATUS_BUSY);

        hw_ecc_disable_clock();
        hw_ecc_read256(8, dst, ecc_ram_base);
        hw_ecc_write256(1, hw_ecc_edwards_curve25519_2_d_q, ecc_ram_base); // Restore overwritten parameter

        return ecc_status == 0;
}

/* Launch x-coordinate recover from y */
static bool ed25519_x_recover(const uint8_t *y, uint8_t *x)
{
        volatile unsigned int ecc_status = HW_ECC_STATUS_BUSY;

        hw_ecc_write256(4, hw_ecc_edwards_curve25519_p_5_8, ecc_ram_base);
        hw_ecc_write256(8, y, ecc_ram_base);
        hw_ecc_write256(10, hw_ecc_edwards_curve25519_d, ecc_ram_base);
        hw_ecc_write256(11, hw_ecc_edwards_curve25519_I, ecc_ram_base);

        hw_ecc_write_command_register(HW_ECC_CMD_CALCR2_TRUE,
                                      HW_ECC_CMD_SIGNB_POS,
                                      HW_ECC_CMD_SIGNA_POS,
                                      HW_ECC_CMD_OP_SIZE_256B,
                                      HW_ECC_CMD_FIELD_FP,
                                      HW_ECC_CMD_OP_ED25519_XRECOVER);

        hw_ecc_enable_clock();
        hw_ecc_start();

        do {
                ecc_status = hw_ecc_read_status();
        } while (ecc_status & HW_ECC_STATUS_BUSY);

        hw_ecc_disable_clock();
        hw_ecc_read256(6, x, ecc_ram_base);

        return ecc_status == 0;
}

/* Check if point is on curve */
static bool ed25519_check_point(const uint8_t *x, const uint8_t *y)
{
        volatile unsigned int ecc_status = HW_ECC_STATUS_BUSY;

        hw_ecc_write256(4, x, ecc_ram_base);
        hw_ecc_write256(5, y, ecc_ram_base);
        hw_ecc_write256(10, hw_ecc_edwards_curve25519_d, ecc_ram_base);
        hw_ecc_cfg_ops(4, 0, 0);

        hw_ecc_write_command_register(HW_ECC_CMD_CALCR2_TRUE,
                                      HW_ECC_CMD_SIGNB_NEG,
                                      HW_ECC_CMD_SIGNA_POS,
                                      HW_ECC_CMD_OP_SIZE_256B,
                                      HW_ECC_CMD_FIELD_FP,
                                      HW_ECC_CMD_OP_ED25519_PNTONC);

        hw_ecc_enable_clock();
        hw_ecc_start();

        do {
                ecc_status = hw_ecc_read_status();
        } while (ecc_status & HW_ECC_STATUS_BUSY);

        hw_ecc_disable_clock();

        return ecc_status == 0;
}

/* Launch negation (x = -x mod Q) */
static bool ed25519_negate(uint8_t *x)
{
        volatile unsigned int ecc_status = HW_ECC_STATUS_BUSY;

        hw_ecc_write256(8, x, ecc_ram_base);
        hw_ecc_cfg_ops(0, 8, 8);

        hw_ecc_write_command_register(HW_ECC_CMD_CALCR2_FALSE,
                                      HW_ECC_CMD_SIGNB_POS,
                                      HW_ECC_CMD_SIGNA_POS,
                                      HW_ECC_CMD_OP_SIZE_256B,
                                      HW_ECC_CMD_FIELD_FP,
                                      HW_ECC_CMD_OP_MODSUB_P);

        hw_ecc_enable_clock();
        hw_ecc_start();

        do {
                ecc_status = hw_ecc_read_status();
        } while (ecc_status & HW_ECC_STATUS_BUSY);

        hw_ecc_disable_clock();
        hw_ecc_read256(8, x, ecc_ram_base);

        return ecc_status == 0;
}

static bool ed25519_decode_point(const uint8_t *src, uint8_t *x, uint8_t *y)
{
        int xsign;

        memcpy(y, src, 32);
        xsign = (src[31] >> 7) & 1; // Extract bit 255
        y[31] &= ~0x80;             // Clear bit 255

        if (!ed25519_x_recover(y, x)) {
                return false;
        }

        /* Negate x coordinate if needed */
        if ((x[0] & 0x01) != xsign && !ed25519_negate(x)) {
                return false;
        }

        return ed25519_check_point(x, y);
}

/* Launch signature verification (check: b*s == r + a*h) */
static bool ed25519_sig_ver(const uint8_t *s, const uint8_t *rx, const uint8_t *ry,
                                        const uint8_t *ax, const uint8_t *ay, const uint8_t *h)
{
        volatile unsigned int ecc_status = HW_ECC_STATUS_BUSY;

        hw_ecc_write256(4, s, ecc_ram_base);
        hw_ecc_write256(5, h, ecc_ram_base);
        hw_ecc_write256(6, ax, ecc_ram_base);
        hw_ecc_write256(7, ay, ecc_ram_base);
        hw_ecc_write256(8, rx, ecc_ram_base);
        hw_ecc_write256(9, ry, ecc_ram_base);

        hw_ecc_write_command_register(HW_ECC_CMD_CALCR2_TRUE,
                                      HW_ECC_CMD_SIGNB_POS,
                                      HW_ECC_CMD_SIGNA_POS,
                                      HW_ECC_CMD_OP_SIZE_256B,
                                      HW_ECC_CMD_FIELD_FP,
                                      HW_ECC_CMD_OP_ED25519_CHECK_SIG);

        hw_ecc_enable_clock();
        hw_ecc_start();

        do {
                ecc_status = hw_ecc_read_status();
        } while (ecc_status & HW_ECC_STATUS_BUSY);

        hw_ecc_disable_clock();

        return ecc_status == 0;
}

void ed25519_init(void *ecc_ram_address)
{
        /* Set ECC RAM address, load u-code and Edwards Curve 25519 parameters*/
        hw_ecc_enable_clock();
        hw_ecc_load_ucode(hw_ecc_ucode1, HW_ECC_UCODE1_SIZE);
        hw_ecc_disable_clock();
        hw_ecc_set_base_addr(ecc_ram_address);
        hw_ecc_edwards_curve25519_load_params(ecc_ram_address);
        ecc_ram_base = ecc_ram_address;
}

bool ed25519_image_sig_verification(const uint8_t *dev_adm_section, uint32_t dev_adm_section_size,
                                                        const uint8_t *exec, uint32_t exec_size,
                                                        const uint8_t *pub_key, const uint8_t *sig)
{
        uint8_t rx[32] = { 0 };
        uint8_t ry[32] = { 0 };
        uint8_t ax[32] = { 0 };
        uint8_t ay[32] = { 0 };
        uint8_t hash[64] = { 0 };
        uint8_t reduced_hash[32] = { 0 };

        /* Check base address of the ECC engine data RAM and arguments */
        if (!ecc_ram_base || !dev_adm_section || !exec || !pub_key || !sig ||
                                                dev_adm_section_size == 0 || exec_size == 0) {
                return false;
        }

        if (!ed25519_decode_point(sig, rx, ry)) {
                return false;
        }

        if (!ed25519_decode_point(pub_key, ax, ay)) {
                return false;
        }

        if (!sha512_verification(sig, pub_key, dev_adm_section, dev_adm_section_size, exec,
                                                                                exec_size, hash)) {
                return false;
        }

        if (!ed25519_reduce(hash, reduced_hash)) {
                return false;
        }

        return ed25519_sig_ver(sig + 32, rx, ry, ax, ay, reduced_hash);
}
