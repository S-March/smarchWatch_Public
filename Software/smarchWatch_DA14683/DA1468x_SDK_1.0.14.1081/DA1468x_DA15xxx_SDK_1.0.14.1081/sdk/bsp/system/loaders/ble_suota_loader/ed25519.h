/**
 ****************************************************************************************
 *
 * @file ed25519.h
 *
 * @brief Ed25519 signature verification - API
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef ED25519_H_
#define ED25519_H_

#include <stdbool.h>

/**
 * \brief Initialize ECC engine for Ed25519 operations
 *
 * Function sets ECC RAM address (it will be used during ECC engine operations), loads ECC u-code
 * and loads Edwards Curve 25519 parameters.
 *
 * \param [in] ecc_ram_addres           an address within SysRAM range. Should be aligned to 1KByte.
 *
 */
void ed25519_init(void *ecc_ram_address);

/**
 * \brief Verify EdDSA (Ed25519) signature of the SUOTA image
 *
 * Function verifies signature which was generated using Ed25519 algorithm for given data. This
 * function assumes that the data is split onto two parts: device administration section (with
 * some alignment) and application binary.
 *
 * \note The \p pub_key should be 32-bytes in length and the \p sig should be 64-bytes in length.
 *
 * \param [in] dev_adm_section          device administration section
 * \param [in] dev_adm_section_size     device administration section size
 * \param [in] exec                     application binary
 * \param [in] exec_size                application binary size
 * \param [in] pub_key                  public key
 * \param [in] sig                      signature
 *
 * \return true if signature is valid for given data and no error occurs, false otherwise
 */
bool ed25519_image_sig_verification(const uint8_t *dev_adm_section, uint32_t dev_adm_section_size,
                                                        const uint8_t *exec, uint32_t exec_size,
                                                        const uint8_t *pub_key, const uint8_t *sig);

#endif /* ED25519_H_ */
