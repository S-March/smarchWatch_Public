/**
 ****************************************************************************************
 *
 * @file main_secure.h
 *
 * @brief Secure bootloader functionalities - API
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef MAIN_SECURE_H_
#define MAIN_SECURE_H_

#include <stdint.h>
#include "suota_security_ext.h"

/** Signature min. length */
#define SIGNATURE_MIN_LENGTH    32
/** Signature max. length */
#define SIGNATURE_MAX_LENGTH    64

/** Address of the asymmetric keys inversion area */
#define INVERSE_ASYMMETRIC_KEYS_ADDRESS 0x7F8E5C0
/** Address of the asymmetric keys area */
#define ASYMMETRIC_KEYS_AREA_ADDRESS    0x7F8E6C0
/** Max. length of the asymmetric key */
#define ASYMMETRIC_KEY_MAX_LEN          64
/** Number of the asymmetric keys */
#define ASYMMETRIC_KEY_NUMBER           4

/**
 * Address of the min. FW version array (it is placed between the 'secure secondary bootloader' and
 * the 'inverse asymmetric keys' area). Each entry contains version number and its bit inversion.
 */
#define MIN_FW_VERSION_AREA_ADDRESS     0x7F8E3C0
/** Number of entries in min. FW version array */
#define MIN_FW_VERSION_ENTRIES_NUMBER   64
/** Size of the one entry in the min. FW version array (2 * 16-bits) */
#define MIN_FW_VERSION_LEN              2 * sizeof(security_hdr_fw_version_t)

/**
 * \brief Invalidate image on the update partition
 *
 * Function changes signature of the image on the update partition to 0x0000 and remove 'valid'
 * flag from the header.
 *
 */
void invalidate_update_image(void);

/**
 * \brief Write first entry to the minimum FW version array
 *
 * Function writes first minimum FW version number and its bit inversion to the minimal FW version
 * array (placed in the OTP memory). If that array contains at least one entry then no write is
 * performed.
 *
 * \note FW version number is get from the current (exec.) image header placed on header partition.
 * If image has valid header security extension then minimum FW version is used. If it is not
 * included in the header then image FW version number is used. If both values cannot be used then
 * version string from image header is parsed. If previous options are not valid then minimum FW
 * version is set to 0.0.
 *
 */
void write_first_min_version_from_header_part(void);

/**
 * \brief Generate symmetric key or keys
 *
 * Function generates 8 symmetric keys using true random number generator engine and writes them
 * into OTP memory with their bit inversions.
 *
 */
void generate_symmetric_keys(void);

/**
 * \brief Reboot device using watchdog
 */
void trigger_reboot(void);

/**
 * \brief Simplified reading from OTP memory
 *
 * Function reads from OTP memory. The OTP controller must be enabled and its speed must be set
 * before calling this function.
 *
 * \param [in] address          address in memory (not an OTP cell address)
 * \param [out] data            buffer with read data
 * \param [in] data_len         number of bytes to read
 *
 * \return false if error occurs during reading from OTP, true otherwise
 */
bool read_otp(uint32_t address, uint8_t *data, uint32_t data_len);

/**
 * \brief Simplified writing to OTP memory
 *
 * Function writes to OTP memory. The OTP controller must be enabled and its speed must be set
 * before calling this function.
 *
 * \param [in] address          address in memory (not an OTP cell address)
 * \param [in] data             buffer with data to write
 * \param [in] data_len         number of bytes to write
 *
 * \return false if error occurs during writing to OTP, true otherwise
 */
bool write_otp(uint32_t address, const uint8_t *data, uint32_t data_len);

#endif /* MAIN_SECURE_H_ */
