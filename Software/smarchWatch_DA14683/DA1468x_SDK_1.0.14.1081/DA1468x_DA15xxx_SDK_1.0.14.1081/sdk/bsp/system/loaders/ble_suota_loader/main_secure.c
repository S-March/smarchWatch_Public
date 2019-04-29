/**
 ****************************************************************************************
 *
 * @file main_secure.c
 *
 * @brief Boot loader for secure SUOTA
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include "suota.h"
#include "hw_cpm.h"
#include "hw_uart.h"
#include "hw_gpio.h"
#include "hw_watchdog.h"
#include "hw_otpc.h"
#include "hw_aes_hash.h"
#include "hw_ecc.h"
#include "hw_ecc_curves.h"
#include "hw_ecc_ucode.h"
#include "hw_crypto.h"
#include "hw_trng.h"
#include "ad_nvms.h"
#include "sdk_defs.h"
#include "flash_partitions.h"
#include "suota_security_ext.h"
#include "bootloaders_common.h"
#include "secure_hooks.h"
#include "ed25519.h"
#include "crc16.h"
#include "main_secure.h"

/* Invalid asymmetric/symmetric key address */
#define INVALID_KEY_ADDRESS            0xFFFFFFFF

/*
 * Symmetric keys constants definitions
 */
#define INVERSE_SYMMETRIC_KEYS_ADDRESS 0x7F8E7C0
#define SYMMETRIC_KEYS_AREA_ADDRESS    0x7F8E8C0
#define SYMMETRIC_KEY_LEN              32
#define SYMMETRIC_KEY_NUMBER           8

/* Secure device definitions */
#define SECURE_DEVICE_ADDRESS           0x7F8EA68
#define SECURE_DEVICE_ENABLED           0xAA

/* Address of image length in OTP header */
#define IMAGE_LENGTH_ADDRESS            0x7F8EA10

/* Address of image CRC in OTP header */
#define IMAGE_CRC_ADDRESS               0x7F8EA38

/* Address of cache architecture in OTP header */
#define CACHE_ARCHITECTURE_ADDRESS      0x7F8EA28

/*
 * Address of invalid symmetric key markers table. If some of the symmetric keys is invalid e.g.
 * revoked then its index is marked as invalid (write 0xFF bytes at proper address).
 */
#define INVALID_SYM_KEY_TABLE_ADDRESS   0x7F8E380
#define INVALID_SYM_KEY_MARKER_NUMBER   SYMMETRIC_KEY_NUMBER
#define INVALID_KEY_MARKER_LEN          8

/*
 * Address of invalid asymmetric key markers table. If some of the asymmetric keys is invalid e.g.
 * revoked then its index is marked as invalid (write 0xFF bytes at proper address).
 */
#define INVALID_ASYM_KEY_TABLE_ADDRESS  0x7F8E360
#define INVALID_ASYM_KEY_MARKER_NUMBER  ASYMMETRIC_KEY_NUMBER

#define OTP_ADDRESS_TO_CELL_OFFSET(address)     (((address) - MEMORY_OTP_BASE) >> 3)

typedef struct {
        uint16_t type;
        uint16_t length;
        uint8_t value[];
} __attribute__((packed)) tlv_t ;

typedef struct {
        security_hdr_key_type_t type;
        uint32_t id;
} key_id_t;

typedef struct {
        const uint8_t *raw_data;
        size_t raw_data_length;
        const suota_security_header_t *sec_hdr;
        const uint8_t *signature;
        uint16_t signature_length;
        const uint8_t *dev_adm_section;
        key_id_t revoke_keys_id[ASYMMETRIC_KEY_NUMBER + SYMMETRIC_KEY_NUMBER];
        uint8_t revoke_keys_number;
        security_hdr_fw_version_t fw_version_number;
        security_hdr_fw_version_t min_fw_version;
        bool min_fw_version_present : 1;
} sec_ext_t;

/*
 * Buffer for sector needed during copy from one partition to the other.
 */
static uint8_t sector_buffer[FLASH_SECTOR_SIZE];

/* NVMS partitions used by bootloader */
struct suota_partitions {
        nvms_t update_part;
        nvms_t exec_part;
        nvms_t header_part;
} suota_partitions;

/**
 * \brief Default values for cm_sysclk, cm_ahbclk, used by hw_cpm_delay_usec()
 *
 */
sys_clk_t cm_sysclk = sysclk_XTAL16M;
ahb_div_t cm_ahbclk = apb_div1;

static const uint32_t crc32_tab[] = {
        0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
        0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
        0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
        0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
        0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
        0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
        0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
        0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
        0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
        0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
        0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
        0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
        0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
        0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
        0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
        0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
        0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
        0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
        0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
        0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
        0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
        0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
        0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
        0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
        0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
        0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
        0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
        0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
        0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
        0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
        0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
        0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
        0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
        0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
        0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
        0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
        0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
        0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
        0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
        0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
        0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
        0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
        0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

/* Most of the features is available only when the device is 'secured' */
static bool secure_device = false;
/* Invalid symmetric/asymmetric keys markers */
static uint8_t invalid_asym_keys_mask;
static uint8_t invalid_sym_keys_mask;

static uint32_t update_crc(uint32_t crc, const uint8_t *data, uint32_t len)
{
        while (len--) {
                crc = crc32_tab[(crc ^ *data++) & 0xff] ^ (crc >> 8);
        }
        return crc;
}

/* Compute security header size. Returns 0 when failed. */
static size_t get_security_ext_length(const uint8_t *data_buffer, size_t buffer_size)
{
        int i;
        tlv_t *tlv_ptr;
        const uint8_t *tmp_ptr;

        tmp_ptr = &data_buffer[0];
        tlv_ptr = (tlv_t *) tmp_ptr;

        /* There should be 2 sections: security and device administration */
        for (i = 0; i < 2; i++) {
                tmp_ptr +=  sizeof(tlv_t) + tlv_ptr->length;
                tlv_ptr = (tlv_t *) tmp_ptr;

                /* Something is wrong - section is longer than read buffer */
                if (tmp_ptr > (data_buffer + buffer_size)) {
                        return 0;
                }
        }

        return tmp_ptr - data_buffer;
}

/*
 * Function returns TLV buffer length or 0 when something was wrong. 'buffer' argument will
 * contain allocated buffer with TLV data - it must be freed after use. It uses a global variable
 * 'sector_buffer' while reading.
 */
static size_t read_security_ext(nvms_t part, size_t offset, const uint8_t **buffer)
{
        size_t length = 0;
        size_t read_size = sizeof(sector_buffer) - offset;

        /* Read whole sector - TLV data should be shorter than sector */
        if (read_size != ad_nvms_read(part, offset, sector_buffer, read_size)) {
                return 0;
        }

        length = get_security_ext_length(sector_buffer, read_size);

        if (length < 1) {
                return 0;
        }

        /*
         * Image header (SUOTA 1.1 header + security section + device adm. section) must be aligned
         * to 1024 bytes. This is required in signature verification - copy pattern bytes (0xFF) also.
         */
        if ((length + sizeof(suota_1_1_image_header_t)) % 1024) {
                length += 1024 - (length + sizeof(suota_1_1_image_header_t)) % 1024;
        }

        /* Don't copy data to the buffer - use mapped flash pointer */
        ad_nvms_get_pointer(part, offset, length, (const void **) buffer);

        return length;
}

/* Return false when ID is incorrect, true otherwise */
static bool check_key_id(uint32_t key_id, security_hdr_key_type_t type)
{
        uint32_t address_lower_limit;
        uint32_t address_upper_limit;
        uint8_t index_upper_limit;

        if (type == SECURITY_HDR_KEY_TYPE_PUBLIC) {
                address_lower_limit = ASYMMETRIC_KEYS_AREA_ADDRESS;
                address_upper_limit = (ASYMMETRIC_KEYS_AREA_ADDRESS + ASYMMETRIC_KEY_MAX_LEN *
                                                                        ASYMMETRIC_KEY_NUMBER);
                index_upper_limit = ASYMMETRIC_KEY_NUMBER - 1;
        } else if (type == SECURITY_HDR_KEY_TYPE_SYMMETRIC) {
                address_lower_limit = SYMMETRIC_KEYS_AREA_ADDRESS;
                address_upper_limit = (SYMMETRIC_KEYS_AREA_ADDRESS + SYMMETRIC_KEY_LEN *
                                                                        SYMMETRIC_KEY_NUMBER);
                index_upper_limit = SYMMETRIC_KEY_NUMBER - 1;
        } else {
                /* Invalid key type */
                return false;
        }

        if (key_id >= address_lower_limit && key_id < address_upper_limit) {
                /* It is a key address */
                return true;
        }

        if (key_id <= index_upper_limit) {
                /* It is a key index */
                return true;
        }

        /* Invalid key ID */
        return false;
}

static uint32_t key_id_to_address(uint32_t id, security_hdr_key_type_t type)
{
        uint32_t area_address;
        uint32_t key_length;
        uint8_t key_number;

        if (!check_key_id(id, type)) {
                /* ID is invalid - return some dummy value */
                return INVALID_KEY_ADDRESS;
        }

        if (type == SECURITY_HDR_KEY_TYPE_PUBLIC) {
                area_address = ASYMMETRIC_KEYS_AREA_ADDRESS;
                key_length = ASYMMETRIC_KEY_MAX_LEN;
                key_number = ASYMMETRIC_KEY_NUMBER;
        } else {
                area_address = SYMMETRIC_KEYS_AREA_ADDRESS;
                key_length = SYMMETRIC_KEY_LEN;
                key_number = SYMMETRIC_KEY_NUMBER;
        }

        if (id > (key_number - 1)) {
                /* ID is an OTP address*/
                return id;
        }

        /* ID is an index - calculate to OTP address */
        return area_address + id * key_length;
}

/* Check signature generation mode, elliptic curve, hash method and public key ID */
static bool check_security_hdr(const suota_security_header_t *hdr)
{
        if (hdr->mode == SECURITY_HDR_MODE_ECDSA) {
                /* Permissible elliptic curves: secp192r1, secp224r1 and secp256r1 */
                if (hdr->curve != SECURITY_HDR_ECC_CURVE_SECP192R1 &&
                                                hdr->curve != SECURITY_HDR_ECC_CURVE_SECP224R1 &&
                                                hdr->curve != SECURITY_HDR_ECC_CURVE_SECP256R1) {
                        return false;
                }

                /* Permissible hash methods: SHA-224, SHA-256, SHA-384, SHA-512 */
                if (hdr->hash != SECURITY_HDR_HASH_SHA_224 &&
                                                        hdr->hash != SECURITY_HDR_HASH_SHA_256 &&
                                                        hdr->hash != SECURITY_HDR_HASH_SHA_384 &&
                                                        hdr->hash != SECURITY_HDR_HASH_SHA_512) {
                        return false;
                }
        } else if (hdr->mode == SECURITY_HDR_MODE_EDDSA) {
                /* Only Ed25519 algorithm is supported (Edwards 25519 curve and SHA-512) */
                if (hdr->curve != SECURITY_HDR_ECC_CURVE_EDWARDS25519 ||
                                                        hdr->hash != SECURITY_HDR_HASH_SHA_512) {
                        return false;
                }
        } else {
                /* Unsupported mode */
                return false;
        }

        /* Check public key ID (it must be an index or an OTP address */
        return check_key_id(hdr->public_key_id, SECURITY_HDR_KEY_TYPE_PUBLIC);
}

/* Get information from security section and validate them. Returns false if any data is invalid. */
static bool parse_and_validate_security_section(sec_ext_t *ext, tlv_t *security_section)
{
        /* Signature section is included in security section */
        tlv_t *signature_section = (tlv_t *) (security_section->value + sizeof(suota_security_header_t));

        /* Check secure and signature sections type */
        if (security_section->type != SECURITY_HDR_TYPE_SECURITY_SECTION ||
                                signature_section->type != SECURITY_HDR_TYPE_SIGNATURE_SECTION) {
               return false;
        }

        /* Check sections lengths */
        if (security_section->length > ext->raw_data_length ||
               security_section->length < (sizeof(suota_security_header_t) + sizeof(tlv_t) +
               signature_section->length) || security_section->length > ext->raw_data_length ||
                                               signature_section->length < SIGNATURE_MIN_LENGTH ||
                                               signature_section->length > SIGNATURE_MAX_LENGTH){
               return false;
        }

        /* Set pointers to security header structure and signature */
        ext->sec_hdr = (suota_security_header_t *) security_section->value;
        ext->signature = (uint8_t *) signature_section->value;
        ext->signature_length = signature_section->length;

        /* Check security section content */
        if (!check_security_hdr(ext->sec_hdr)) {
               return false;
        }

        return true;
}

/*
 * Get information from device administration section and validate them. Returns false if any data
 * is invalid.
 */
static bool parse_and_validate_dev_administration_section(sec_ext_t *ext, tlv_t *dev_adm_section)
{
        uint8_t *tmp_ptr = (uint8_t *) dev_adm_section;
        tlv_t *key_rev_record = NULL;
        tlv_t *fw_version_number = NULL;
        tlv_t *rollback_prevention_segment = NULL;

        if (!dev_adm_section) {
                return false;
        }

        /* Store device administration section address - it will be helpfully in signature verification */
        ext->dev_adm_section = tmp_ptr;
        tmp_ptr = dev_adm_section->value;

        /* Check section type */
        if (dev_adm_section->type != SECURITY_HDR_TYPE_DEVICE_ADMIN_SECTION) {
                return false;
        }

        while (tmp_ptr < dev_adm_section->value + dev_adm_section->length) {
                tlv_t *tlv = (tlv_t *) tmp_ptr;

                switch(tlv->type) {
                case SECURITY_HDR_TYPE_KEY_REVOCATION_RECORD:
                        key_rev_record = tlv;
                        break;
                case SECURITY_HDR_TYPE_FW_VERSION_NUMBER:
                        fw_version_number = tlv;
                        break;
                case SECURITY_HDR_TYPE_ROLLBACK_PREVENTION_SEGMENT:
                        rollback_prevention_segment = tlv;
                        break;
                }

                tmp_ptr = tlv->value + tlv->length;
        }

        /* Firmware version number is a mandatory field */
        if (!fw_version_number || fw_version_number->length < sizeof(ext->fw_version_number)) {
                return false;
        }

        /* Copy FW version number */
        memcpy(&ext->fw_version_number, fw_version_number->value, sizeof(ext->fw_version_number));

        /* Key revocation record is optional */
        if (key_rev_record) {
                const uint32_t id_size = sizeof(ext->revoke_keys_id[0].id);

                tmp_ptr = key_rev_record->value;

                while (tmp_ptr < key_rev_record->value + key_rev_record->length) {
                        if (ext->revoke_keys_number >= sizeof(ext->revoke_keys_id) / id_size) {
                                break;
                        }

                        if (*tmp_ptr != SECURITY_HDR_KEY_TYPE_PUBLIC && *tmp_ptr !=
                                                                SECURITY_HDR_KEY_TYPE_SYMMETRIC) {
                                /* Unsupported key type */
                                ++tmp_ptr;
                                continue;
                        }

                        ext->revoke_keys_id[ext->revoke_keys_number].type = *tmp_ptr;
                        ++tmp_ptr;
                        memcpy(&ext->revoke_keys_id[ext->revoke_keys_number].id, tmp_ptr, id_size);
                        tmp_ptr += id_size;
                        ++ext->revoke_keys_number;
                }
        }

        if (rollback_prevention_segment && rollback_prevention_segment->length >=
                                                                sizeof(ext->fw_version_number)) {
                memcpy(&ext->min_fw_version, rollback_prevention_segment->value,
                                                                sizeof(ext->fw_version_number));
                ext->min_fw_version_present = true;
        }

        return true;
}

/*
 * The function reads a security header extension from a proper location and parses obtained buffer
 * (with TLV entries) to more readable form. Some validation is done during parsing.
 */
static bool parse_and_validate_security_ext(nvms_t part, size_t offset, sec_ext_t *ext)
{
        tlv_t *security_section = NULL;
        tlv_t *dev_adm_section = NULL;
        const uint8_t *tmp_ptr;

        memset(ext, 0, sizeof(*ext));
        ext->raw_data_length = read_security_ext(part, offset, &ext->raw_data);

        /* Something went wrong during reading */
        if (ext->raw_data_length == 0 || !ext->raw_data) {
                return false;
        }

        /* Security section must be first */
        tmp_ptr = ext->raw_data;
        security_section = (tlv_t *) tmp_ptr;

        if (!parse_and_validate_security_section(ext, security_section)) {
                return false;
        }

        /* Device administration section is second. It must contain at least firmware version number */
        tmp_ptr = security_section->value + security_section->length;
        dev_adm_section = (tlv_t *) tmp_ptr;

        return parse_and_validate_dev_administration_section(ext, dev_adm_section);
}

/*
 * Read data from OTP. 'address' is an address in memory - not a OTP cell address. 'data_len' is a
 * number of bytes to read.
 */
bool read_otp(uint32_t address, uint8_t *data, uint32_t data_len)
{
        hw_otpc_manual_read_on(false);
        memcpy(data, (void *) address, data_len);
        hw_otpc_manual_read_off();

        return true;
}

/*
 * Write data from OTP. 'address' is an address in memory - not a OTP cell address. 'data_len' is a
 * number of bytes to write.
 */
bool write_otp(uint32_t address, const uint8_t *data, uint32_t data_len)
{
        HW_OTPC_WORD word = (address & 0x04) ? HW_OTPC_WORD_HIGH : HW_OTPC_WORD_LOW;

        return hw_otpc_manual_prog((uint32_t *) data, OTP_ADDRESS_TO_CELL_OFFSET(address), word,
                                                                                data_len / 4, false);
}

/*
 * Return true if the public key has been read properly and its bit inversion is valid, false
 * otherwise.
 */
static bool read_public_key(uint32_t key_address, uint8_t *key, size_t *key_size)
{
        int i;
        uint32_t key_inv_address;
        uint8_t key_inv[ASYMMETRIC_KEY_MAX_LEN] = { 0 };
        uint8_t key_index;

        key_index = (key_address - ASYMMETRIC_KEYS_AREA_ADDRESS) / ASYMMETRIC_KEY_MAX_LEN;

        if (invalid_asym_keys_mask & (1 << key_index)) {
                return false;
        }

        /* Read public key */
        if (!read_otp(key_address, key, ASYMMETRIC_KEY_MAX_LEN)) {
                return false;
        }

        key_inv_address = INVERSE_ASYMMETRIC_KEYS_ADDRESS + key_address - ASYMMETRIC_KEYS_AREA_ADDRESS;

        /* Read key bit inversion */
        if (!read_otp(key_inv_address, key_inv, ASYMMETRIC_KEY_MAX_LEN)) {
                return false;
        }

        *key_size = ASYMMETRIC_KEY_MAX_LEN;

        /* Check public key and its inversion */
        for (i = ASYMMETRIC_KEY_MAX_LEN - 1; i >= 0; i--) {
                if ((key_inv[i] ^ key[i]) != 0xFF) {
                        if (key[i] == 0 && key_inv[i] == 0) {
                                /* Key is shorter than maximum length */
                                *key_size = i;
                        } else {
                                /* Key part and its bit inversion is incompatible */
                                *key_size = 0;
                                break;
                        }
                }
        }

        /* 'key_size' is 0 bytes in length only if it is invalid or empty */
        return *key_size != 0;
}

/* Convert FW version string to the version number (unsigned integer 32-bits) */
static bool version_string_to_version_number(const char *fw_version_string,
                                                        security_hdr_fw_version_t *fw_version)
{
        char *end;

        if (!fw_version_string) {
                return false;
        }

        fw_version->major = (uint16_t) strtol(fw_version_string, &end, 10);

        if (*end != '.') {
                return false;
        }

        /* Any character could occur after minor version number - skip them */
        fw_version->minor = (uint16_t) strtol(end + 1, &end, 10);

        return true;
}

/*
 * Function compares FW version string from SUOTA header with FW version number included in security
 * header extension. Version string must be in proper form - 4 decimal number separated with dots.
 * Any characters after the last number will be skipped (they are acceptable).
 */
static bool compare_fw_versions(const char *fw_version_string,
                                                        const security_hdr_fw_version_t *fw_version)
{
        security_hdr_fw_version_t version_from_string;

        if (!version_string_to_version_number(fw_version_string, &version_from_string)) {
                return false;
        }

        return compare_version_hook(&version_from_string, fw_version) == 0;
}

/*
 * Function reads permissible, minimum FW version from OTP memory and store it in 'version'.
 * Function returns true if the last written entry is a valid entry (with matching 2's complements),
 * false otherwise. If no entry was written to the table then 'empty' argument is set to true.
 */
static bool read_min_fw_version(security_hdr_fw_version_t *version, bool *empty)
{
        int i;

        *empty = false;
        version->major = 0xFFFF;
        version->minor = 0xFFFF;

        /* Find the latest version entry */
        for (i = 0; i < MIN_FW_VERSION_ENTRIES_NUMBER; i++) {
                security_hdr_fw_version_t entry[2];
                /*
                 * Read version entry from OTP . If failed then it could be overwritten - goto the
                 * next entry.
                 */
                if (!read_otp(MIN_FW_VERSION_AREA_ADDRESS + i * MIN_FW_VERSION_LEN, (uint8_t *) entry,
                                                                                sizeof(entry))) {
                        continue;
                }

                /* Check that the version entry and its 2's complements match */
                if ((entry[0].major ^ entry[1].major) == 0xFFFF &&
                                                (entry[0].minor ^ entry[1].minor) == 0xFFFF) {
                        security_hdr_fw_version_t next_entry[2];

                        if (!read_otp(MIN_FW_VERSION_AREA_ADDRESS + (i + 1) * MIN_FW_VERSION_LEN,
                                                        (uint8_t *) next_entry, sizeof(next_entry))) {
                                return false;
                        }

                        if (i < MIN_FW_VERSION_ENTRIES_NUMBER - 1 && (next_entry[0].major != 0 ||
                                        next_entry[0].minor != 0 || next_entry[1].major != 0 ||
                                                                        next_entry[1].minor != 0)) {
                                /* This is not the last written entry in the table */
                                continue;
                        }

                        *version = entry[0];
                        return true;
                }

                if (entry[0].major == 0 && entry[0].minor == 0 && entry[1].major == 0 &&
                                                                        entry[1].minor == 0) {
                        /*
                         * Entry field is empty - next fields should be empty also, so don't check
                         * them.
                         */
                        break;
                }
        }

        /* In this place table is not initialized or there is no valid entry in it */
        if (i == 0) {
                *empty = true;
        }

        return false;
}

/*
 * Return false if there is no valid public key in the OTP memory.
 */
static bool check_root_keys(uint8_t *valid)
{
        int i;
        uint32_t address;
        size_t key_size;
        uint8_t key[ASYMMETRIC_KEY_MAX_LEN];
        uint8_t v = 0;

        for (i = 0; i < 4; i++) {
                address = ASYMMETRIC_KEYS_AREA_ADDRESS + i * ASYMMETRIC_KEY_MAX_LEN;

                if (read_public_key(address, key, &key_size)) {
                        v |= (1 << i);
                }
        }

        if (valid) {
                *valid = v;
        }

        return v != 0;
}

/*
 * Return false if no valid symmetric key is in OTP memory. 'valid' indicates which symmetric keys
 * are valid. 'empty' flag indicates which key containers are empty.
 */
static bool check_symmetric_keys(uint8_t *valid, uint8_t *empty)
{
        const uint8_t empty_key[SYMMETRIC_KEY_LEN] = { 0 };
        uint8_t v = 0;
        uint8_t e = 0;
        int i, j;

        for (i = 0; i < SYMMETRIC_KEY_NUMBER; i++) {
                uint8_t key[SYMMETRIC_KEY_LEN];
                uint8_t inv_key[SYMMETRIC_KEY_LEN];
                bool valid_key = true;

                if (invalid_sym_keys_mask & (1 << i)) {
                        /* Prevent bus error when reading revoked keys */
                        continue;
                }

                /* Read key - this could fail if the key has been revoked */
                if (!read_otp(SYMMETRIC_KEYS_AREA_ADDRESS + i * SYMMETRIC_KEY_LEN, key,
                                                                        SYMMETRIC_KEY_LEN)) {
                        continue;
                }

                /* Read key inversion - this could fail from the same r*/
                if (!read_otp(INVERSE_SYMMETRIC_KEYS_ADDRESS + i * SYMMETRIC_KEY_LEN, inv_key,
                                                                        SYMMETRIC_KEY_LEN)) {
                        continue;
                }

                if (!memcmp(empty_key, key, SYMMETRIC_KEY_LEN) && !memcmp(empty_key, inv_key,
                                                                        SYMMETRIC_KEY_LEN)) {
                        e |= (1 << i);
                        continue;
                }

                for (j = 0; j < SYMMETRIC_KEY_LEN; j++) {
                        if ((inv_key[j] ^ key[j]) != 0xFF) {
                                valid_key = false;
                                break;
                        }
                }

                if (valid_key) {
                        v |= (1 << i);
                }
        }

        if (valid) {
                *valid = v;
        }

        if (empty) {
                *empty = e;
        }

        return v != 0;
}

/*
 * Mark invalid (e.g. revoked) symmetric keys in the table placed in the OTP memory. This table
 * could be used by application. This function should be called after key revocation.
 */
static void mark_invalid_symmetric_keys(void)
{
        uint8_t table[INVALID_KEY_MARKER_LEN * INVALID_SYM_KEY_MARKER_NUMBER];
        uint8_t valid_keys;
        uint8_t marker[INVALID_KEY_MARKER_LEN];
        int i;

        /* Check which symmetric keys are valid */
        check_symmetric_keys(&valid_keys, NULL);

        /*
         * Read whole markers table. Write to this table should be performed once per the same
         * cell - read error shouldn't occur
         */
        read_otp(INVALID_SYM_KEY_TABLE_ADDRESS, table, sizeof(table));

        /* Prepare marker */
        memset(marker, 0xFF, INVALID_KEY_MARKER_LEN);

        for (i = 0; i < SYMMETRIC_KEY_NUMBER; i++) {
                if (!(valid_keys & (1 << i)) && memcmp(&table[i * INVALID_KEY_MARKER_LEN],
                                                        marker, INVALID_KEY_MARKER_LEN)) {
                        /* The symmetric key is not valid, but is not marked in the array */
                        write_otp(INVALID_SYM_KEY_TABLE_ADDRESS + i * INVALID_KEY_MARKER_LEN,
                                                                marker, INVALID_KEY_MARKER_LEN);
                }
        }
}

static void revoke_key(uint32_t key_address, security_hdr_key_type_t type)
{
        uint8_t invalid[ASYMMETRIC_KEY_MAX_LEN];
        uint32_t key_inv_address;
        uint32_t write_size;
        uint32_t table_address;
        uint8_t key_idx;

        if (type == SECURITY_HDR_KEY_TYPE_PUBLIC) {
                key_inv_address = INVERSE_ASYMMETRIC_KEYS_ADDRESS + key_address -
                                                                ASYMMETRIC_KEYS_AREA_ADDRESS;
                write_size = ASYMMETRIC_KEY_MAX_LEN;
                key_idx = (key_address - ASYMMETRIC_KEYS_AREA_ADDRESS) / ASYMMETRIC_KEY_MAX_LEN;
                table_address = INVALID_ASYM_KEY_TABLE_ADDRESS + key_idx * INVALID_KEY_MARKER_LEN;
        } else if (type == SECURITY_HDR_KEY_TYPE_SYMMETRIC) {
                key_inv_address = INVERSE_SYMMETRIC_KEYS_ADDRESS + key_address -
                                                                SYMMETRIC_KEYS_AREA_ADDRESS;
                write_size = SYMMETRIC_KEY_LEN;
                key_idx = (key_address - SYMMETRIC_KEYS_AREA_ADDRESS) / SYMMETRIC_KEY_LEN;
                table_address = INVALID_SYM_KEY_TABLE_ADDRESS + key_idx * INVALID_KEY_MARKER_LEN;
        } else {
                return;
        }

        memset(invalid, 0xFF, write_size);

        write_otp(table_address, invalid, INVALID_KEY_MARKER_LEN);
        write_otp(key_address, invalid, write_size);
        write_otp(key_inv_address, invalid, write_size);
}

static void update_min_version(const sec_ext_t *sec_ext)
{
        int cmp_result;
        bool empty = false;
        security_hdr_fw_version_t old_version;

        /* Handle change minimum FW version */
        if (!sec_ext->min_fw_version_present) {
                return;
        }

        if (compare_version_hook(&sec_ext->min_fw_version, &sec_ext->fw_version_number) > 0) {
                TRACE("Cannot change minimum FW version - requested version is greater than "
                                                                        "image version!\r\n");
                return;
        }

        if (!read_min_fw_version(&old_version, &empty) && !empty) {
                TRACE("Cannot change minimum FW version - previous value cannot be read!\r\n");
                return;
        }

        cmp_result = compare_version_hook(&sec_ext->min_fw_version, &old_version);

        if (cmp_result < 0) {
                TRACE("Cannot change minimum FW version - requested version is lower than previously "
                                                                                "written!\r\n");
                return;
        }

        if (cmp_result == 0) {
                /* New minimum FW version number is the same as previous - writing is not needed */
                return;
        }

        update_version_hook(&sec_ext->min_fw_version);

        /*
         * Read again, because minimum FW version table could be full - in that case no write will
         * be performed.
         */
        read_min_fw_version(&old_version, &empty);

        TRACE("Current minimum FW version: %u.%u.\r\n", old_version.major, old_version.minor);
}

static void initialize_invalid_key_markers(void)
{
        const uint8_t empty[INVALID_KEY_MARKER_LEN] = { 0 };
        int i;
        uint8_t asym[INVALID_KEY_MARKER_LEN * INVALID_ASYM_KEY_MARKER_NUMBER] = { };
        uint8_t sym[INVALID_KEY_MARKER_LEN * INVALID_SYM_KEY_MARKER_NUMBER] = { };

        read_otp(INVALID_ASYM_KEY_TABLE_ADDRESS, asym, sizeof(asym));
        read_otp(INVALID_SYM_KEY_TABLE_ADDRESS, sym, sizeof(sym));

        for (i = 0; i < INVALID_ASYM_KEY_MARKER_NUMBER; i++) {
                if (memcmp(empty, &asym[i * INVALID_KEY_MARKER_LEN], INVALID_KEY_MARKER_LEN)) {
                        invalid_asym_keys_mask |= (1 << i);
                }
        }

        for (i = 0; i < INVALID_SYM_KEY_MARKER_NUMBER; i++) {
                if (memcmp(empty, &sym[i * INVALID_KEY_MARKER_LEN], INVALID_KEY_MARKER_LEN)) {
                        invalid_sym_keys_mask |= (1 << i);
                }
        }
}

static void revoke_keys(const sec_ext_t *sec_ext)
{
        int i;
        uint8_t valid_sym_keys = 0;

        /* Handle revoke symmetric or public (root) key command */
        if (sec_ext->revoke_keys_number < 1) {
                return;
        }

        /* Get mask of the valid symmetric keys - at least one should be valid in this place */
        check_symmetric_keys(&valid_sym_keys, NULL);

        for (i = 0; i < sec_ext->revoke_keys_number; i++) {
                /* Symmetric keys are shorter than asymmetric - this buffer can hold both of them */
                uint8_t key[ASYMMETRIC_KEY_MAX_LEN];
                size_t key_len = 0;
                uint32_t rev_key_address = key_id_to_address(sec_ext->revoke_keys_id[i].id,
                                                                sec_ext->revoke_keys_id[i].type);

                if (rev_key_address == INVALID_KEY_ADDRESS) {
                        /* Key ID is invalid - skip it */
                        continue;
                }

                if (rev_key_address == key_id_to_address(sec_ext->sec_hdr->public_key_id,
                                                                SECURITY_HDR_KEY_TYPE_PUBLIC)) {
                        TRACE("Cannot revoke public key - it is a key used by current image!\r\n");
                        continue;
                }

                if (sec_ext->revoke_keys_id[i].type == SECURITY_HDR_KEY_TYPE_SYMMETRIC) {
                        uint8_t key_mask = (1 << ((rev_key_address - SYMMETRIC_KEYS_AREA_ADDRESS) /
                                                                                SYMMETRIC_KEY_LEN));

                        if (!(key_mask & valid_sym_keys)) {
                                /* Key is invalid */
                                continue;
                        }

                        if (valid_sym_keys == key_mask) {
                                /* This is the last valid symmetric key - don't invalidate it */
                                TRACE("Cannot revoke symmetric key - it is the last valid key!\r\n");
                                continue;
                        }

                        valid_sym_keys &= ~key_mask;
                } else {
                        /* Key has been already revoked or cannot be read */
                        if (!read_public_key(rev_key_address, key, &key_len)) {
                                continue;
                        }
                }

                revoke_key(rev_key_address, sec_ext->revoke_keys_id[i].type);
                TRACE("%s key at address 0x%"PRIX32" revoked.\r\n",
                        sec_ext->revoke_keys_id[i].type == SECURITY_HDR_KEY_TYPE_PUBLIC ? "Root" :
                                                                "Symmetric", rev_key_address);
        }

        /* Update markers */
        initialize_invalid_key_markers();
}

static void switch_to_pll48(void)
{
        if (hw_cpm_is_pll_locked() == 0) {
                /* Turn on PLL */
                hw_cpm_pll_sys_on();
        }

        /* Enable divider (div by 2) */
        hw_cpm_enable_pll_divider();

        /* Adjust OTP timings */
        if (hw_otpc_is_active()) {
                uint32_t clk_freq;

                clk_freq = 16 >> hw_cpm_get_hclk_div();
                clk_freq *= sysclk_PLL48;

                /* Ensure AHB clock frequency is proper for OTP access timings */
                ASSERT_WARNING((clk_freq <= 48) && (clk_freq > 0));

                hw_otpc_set_speed(hw_otpc_convert_sys_clk_mhz(clk_freq));
        }

        if (dg_configFLASH_CONNECTED_TO != FLASH_IS_NOT_CONNECTED) {
                qspi_automode_sys_clock_cfg(sysclk_PLL48);
                hw_qspi_enable_readpipe(1);
        }

        /* Set PLL as sys_clk */
        hw_cpm_set_sysclk(SYS_CLK_IS_PLL);
}

/**
 * \brief System Initialization
 *
 */
static void init(void)
{
        if (!hw_cpm_check_xtal16m_status()) {
                hw_cpm_enable_xtal16m();
                while (!hw_cpm_is_xtal16m_trimmed());
        }
        hw_cpm_set_sysclk(SYS_CLK_IS_XTAL16M);
        hw_cpm_set_hclk_div(0);
        hw_cpm_set_pclk_div(0);

        /* Enable OTP Controller */
        hw_otpc_init();

        /* Speedup booting time */
        switch_to_pll48();

        hw_watchdog_freeze();                           // stop watchdog
        hw_cpm_deactivate_pad_latches();                // enable pads
        hw_cpm_power_up_per_pd();                       // exit peripheral power down
}

/* Compare CRC stored in OTP with calculated for image - 'SBL Integrity' check */
static bool check_secure_boot_crc(void)
{
        uint32_t length;
        uint32_t crc_otp;
        uint16_t crc_calc;
        uint32_t address = MEMORY_OTP_BASE;

        if (!read_otp(IMAGE_LENGTH_ADDRESS, (uint8_t *) &length, 4) || length == 0) {
                return false;
        }

        if (!read_otp(IMAGE_CRC_ADDRESS, (uint8_t *) &crc_otp, 4)) {
                return false;
        }

        crc16_init(&crc_calc);
        length <<= 2;

        while (length) {
                uint32_t read_len;

                read_len = length > sizeof(sector_buffer) ? sizeof(sector_buffer) : length;
                read_otp(address, sector_buffer, read_len);
                crc16_update(&crc_calc, sector_buffer, read_len);
                length -= read_len;
                address += read_len;
        }

        return crc_otp == crc_calc;
}

/* Checks condition unrelated with FW image: bootloader's CRC, keys and minimum FW version array */
static bool device_integrity_check(void)
{
        uint8_t field[8] = { 0 };
        uint8_t empty;
        bool min_ver_array_empty = false;
        security_hdr_fw_version_t min_fw_ver;

        /* Check Secure Boot Loader integrity */
        if (!check_secure_boot_crc()) {
                /* This case should be handled in the same way in both hooks */
                TRACE("Secure Bootloader CRC is invalid\r\n");

                if (!secure_boot_failure_hook(FAILURE_BOOTLOADER_CRC_MISMATCH,
                                                                        FAILURE_SOURCE_DEVICE)) {
                        return false;
                }
        }

        /* Read 'Secure device' field from the OTP */
        if (!read_otp(SECURE_DEVICE_ADDRESS, field, sizeof(field))) {
                return false;
        }

        /* Check that 'Secure device' is enabled */
        if (field[0] != SECURE_DEVICE_ENABLED) {
                /* Device is not 'secured' - don't check keys */
                TRACE("Device is not secure.\r\n");
                return true;
        } else {
                secure_device = true;
        }

        /* Read which keys are invalid and mark then - prevent bus errors when read from OTP */
        initialize_invalid_key_markers();

        if (!check_symmetric_keys(NULL, &empty)) {
                if (empty == 0xFF) {
                        TRACE("Symmetric key area is empty!\r\n");

                        if (!secure_boot_failure_hook(FAILURE_REASON_EMPTY_SYMMETRIC_KEYS,
                                                                        FAILURE_SOURCE_DEVICE)) {
                                return false;
                        }
                } else {
                        TRACE("There is no valid symmetric key!\r\n");

                        if (!secure_boot_failure_hook(FAILURE_REASON_INVALID_SYMMETRIC_KEYS,
                                                                        FAILURE_SOURCE_DEVICE)) {
                                return false;
                        }
                }
        }

        /* At least one valid public key is needed */
        if (!check_root_keys(NULL)) {
                TRACE("There is no valid root key!\r\n");

                if (!secure_boot_failure_hook(FAILURE_REASON_INVALID_ROOT_KEYS,
                                                                        FAILURE_SOURCE_DEVICE)) {
                        return false;
                }
        }

        /* Only check minimum FW version array */
        if (!read_min_fw_version(&min_fw_ver, &min_ver_array_empty) && !min_ver_array_empty) {
                TRACE("Cannot read minimum FW version array or it contains invalid values!\r\n");

                if (!secure_boot_failure_hook(FAILURE_REASON_FW_VERSION_ARRAY_BROKEN,
                                                                        FAILURE_SOURCE_DEVICE)) {
                        return false;
                }
        }

        /* Minimum FW version array is empty - could be updated in hook */
        if (min_ver_array_empty) {
                TRACE("Minimum FW version array is empty.\r\n");

                return secure_boot_failure_hook(FAILURE_REASON_FW_VERSION_ARRAY_EMPTY,
                                                                        FAILURE_SOURCE_DEVICE);
        }

        return true;
}

static bool fw_validation(nvms_t header_part, nvms_t exec_part, size_t header_offset)
{
        bool is_update_part = (header_part == exec_part);
        suota_1_1_image_header_t header;
        const uint8_t *mapped_ptr;
        uint32_t crc;
        bool result = false;
        failure_source_t source = is_update_part ? FAILURE_SOURCE_UPDATE_IMAGE :
                                                                        FAILURE_SOURCE_EXEC_IMAGE;
        sec_ext_t sec_ext = { 0 };
        uint8_t public_key[ASYMMETRIC_KEY_MAX_LEN];
        size_t public_key_len = ASYMMETRIC_KEY_MAX_LEN;
        security_hdr_fw_version_t min_fw_ver = { };
        bool min_ver_array_empty = false;

        if (header_part == NULL || exec_part == NULL) {
                goto done;
        }

        if (!read_image_header(header_part, header_offset, &header)) {
                goto done;
        }

        /* Integrity check */
        if (!image_ready(&header) && !secure_boot_failure_hook(FAILURE_REASON_INVALID_IMAGE_SIGNATURE,
                                                                                        source)) {
                goto done;
        }

        crc = ~0; /* Initial value of CRC prepared by mkimage */
        /*
         * Utilize QSPI memory mapping for CRC check, this way no additional buffer is needed.
         */
        if (header.code_size != ad_nvms_get_pointer(exec_part, is_update_part ?
                        header.exec_location : 0, header.code_size, (const void **) &mapped_ptr)) {
                goto done;
        }
        crc = update_crc(crc, mapped_ptr, header.code_size);
        crc ^= ~0; /* Final XOR */

        if (crc != header.crc && !secure_boot_failure_hook(FAILURE_REASON_CRC_MISMATCH, source)) {
                goto done;
        }

        if (!secure_device) {
                /* Rest of the check is needed only when device is 'secured' */
                result = true;
                goto done;
        }

        if (!parse_and_validate_security_ext(header_part, header_offset + sizeof(header), &sec_ext)) {
                TRACE("TLV security extension is invalid!\r\n");

                if (!secure_boot_failure_hook(FAILURE_REASON_SEC_EXTENSION_INVALID, source)) {
                        goto done;
                }
        }

        if (!sec_ext.sec_hdr || !read_public_key(key_id_to_address(sec_ext.sec_hdr->public_key_id,
                                        SECURITY_HDR_KEY_TYPE_PUBLIC), public_key, &public_key_len)) {
                TRACE("Root key is not valid!\r\n");

                if (!secure_boot_failure_hook(FAILURE_REASON_INVALID_ROOT_KEY, source)) {
                        goto done;
                }
        }

        if (!sec_ext.sec_hdr || !verify_signature_hook(sec_ext.sec_hdr->mode, sec_ext.sec_hdr->curve,
                                        sec_ext.sec_hdr->hash, public_key, public_key_len,
                                        sec_ext.dev_adm_section, sec_ext.raw_data_length -
                                        (sec_ext.dev_adm_section - sec_ext.raw_data), mapped_ptr,
                                                                header.code_size, sec_ext.signature,
                                                                        sec_ext.signature_length)) {
                TRACE("Image signature verification failed!\r\n");

                if (!secure_boot_failure_hook(FAILURE_REASON_IMAGE_SIGNATURE, source)) {
                        goto done;
                }
        }

        if (!compare_fw_versions((char *) header.version, &sec_ext.fw_version_number)) {
                TRACE("FW version in SUOTA header and in security extension don't match!\r\n");

                if (!secure_boot_failure_hook(FAILURE_REASON_FW_VERSION_MISMATCH, source)) {
                        goto done;
                }
        }

        if (!read_min_fw_version(&min_fw_ver, &min_ver_array_empty) && !min_ver_array_empty) {
                TRACE("Cannot read minimum FW version array or it contains invalid values!\r\n");
                if (!secure_boot_failure_hook(FAILURE_REASON_FW_VERSION_ARRAY_BROKEN, source)) {
                        goto done;
                }
        }

        if (compare_version_hook(&sec_ext.fw_version_number, &min_fw_ver) < 0) {
                TRACE("FW version is lower than required minimum!\r\n");
                if (!secure_boot_failure_hook(FAILURE_REASON_FW_VERSION_TOO_LOW, source)) {
                        goto done;
                }
        }

        result = true;
done:

        return result;
}

void invalidate_update_image(void)
{
        suota_1_1_image_header_t header;

        if (!suota_partitions.update_part) {
                /* Partition has been not opened - read or write cannot be performed */
                return;
        }

        if (!read_image_header(suota_partitions.update_part, SUOTA_IMAGE_HEADER_OFFSET, &header)) {
                return;
        }

        if (header.signature[0] == 0 && header.signature[1] == 0 && !(header.flags &
                                                                SUOTA_1_1_IMAGE_FLAG_VALID)) {
                /* Image has been already invalidated - do nothing */
                return;
        }

        header.flags &= ~SUOTA_1_1_IMAGE_FLAG_VALID;
        header.signature[0] = 0;
        header.signature[1] = 0;

        ad_nvms_write(suota_partitions.update_part, SUOTA_IMAGE_HEADER_OFFSET, (uint8_t *) &header,
                                                                                sizeof(header));
}

void write_first_min_version_from_header_part(void)
{
        suota_1_1_image_header_t hdr;
        security_hdr_fw_version_t version = { };
        bool empty = false;
        bool sec_ext_valid;
        sec_ext_t sec_ext;

        if (!read_min_fw_version(&version, &empty) && !empty) {
                /* There is at least one entry in the minimum FW version array */
                return;
        }

        if (!suota_partitions.header_part) {
                return;
        }

        if (!read_image_header(suota_partitions.header_part, 0, &hdr)) {
                /* Cannot read image header */
                return;
        }

        sec_ext_valid = parse_and_validate_security_ext(suota_partitions.header_part, sizeof(hdr),
                                                                                        &sec_ext);

        /*
         * Use new minimum FW version, image FW version number or parse image version string (in
         * this order). If none of these values could be used then minimum FW version will be set
         * to 0.0.
         */
        if (sec_ext_valid && sec_ext.min_fw_version_present) {
                version.major = sec_ext.min_fw_version.major;
                version.minor = sec_ext.min_fw_version.minor;
        } else if (sec_ext_valid) {
                version.major = sec_ext.fw_version_number.major;
                version.minor = sec_ext.fw_version_number.minor;
        } else if (!version_string_to_version_number((char *) hdr.version, &version)) {
                version.major = 0;
                version.minor = 0;
        }

        update_version_hook(&version);
}

void generate_symmetric_keys(void)
{
        int i, j;
        uint8_t key[SYMMETRIC_KEY_LEN];
        uint8_t inv_key[SYMMETRIC_KEY_LEN];

        hw_trng_enable(NULL);

        /* Whole symmetric key area is empty */
        for (i = 0; i < SYMMETRIC_KEY_NUMBER; i++) {
                hw_trng_get_numbers((uint32_t *) key, SYMMETRIC_KEY_LEN / sizeof(uint32_t));

                /* Create bit inversion of the symmetric key */
                for (j = 0; j < SYMMETRIC_KEY_LEN; j++) {
                        inv_key[j] = ~key[j];
                }

                write_otp(SYMMETRIC_KEYS_AREA_ADDRESS + i * SYMMETRIC_KEY_LEN, key,
                                                                                SYMMETRIC_KEY_LEN);
                write_otp(INVERSE_SYMMETRIC_KEYS_ADDRESS + i * SYMMETRIC_KEY_LEN, inv_key,
                                                                                SYMMETRIC_KEY_LEN);
        }

        hw_trng_disable();
}

static bool update_image(nvms_t update_part, nvms_t exec_part, nvms_t header_part)
{
        suota_1_1_image_header_t new_header;
        size_t extension_length = 0;
        size_t left;
        size_t src_offset;
        size_t dst_offset;
        bool result = false;
        uint8_t *sec_ext_buff = NULL;

        /* Read one sector - header and security extension should be shorter */
        if (sizeof(sector_buffer) != ad_nvms_read(update_part, SUOTA_IMAGE_HEADER_OFFSET,
                                                        sector_buffer, sizeof(sector_buffer))) {
                goto done;
        }

        memcpy(&new_header, sector_buffer, sizeof(new_header));
        extension_length = get_security_ext_length(sector_buffer + sizeof(new_header),
                                                        sizeof(sector_buffer) - sizeof(new_header));

        /* Security extension is invalid */
        if (secure_device && extension_length == 0) {
                goto done;
        }

        /* Copy security extension - 'sector_buffer' will be overridden */
        sec_ext_buff = malloc(extension_length);
        if (!sec_ext_buff) {
                goto done;
        }

        memcpy(sec_ext_buff, sector_buffer + sizeof(new_header), extension_length);

        /*
         * Erase header partition. New header will be written after executable is copied.
         */
        if (!ad_nvms_erase_region(header_part, 0, sizeof(new_header) + extension_length)) {
                goto done;
        }

        /*
         * Erase executable partition.
         */
        if (!ad_nvms_erase_region(exec_part, 0, new_header.code_size)) {
                goto done;
        }

        left = new_header.code_size;    /* Whole image to copy */
        dst_offset = 0;                 /* Write from the beginning of executable partition */
        src_offset = SUOTA_IMAGE_HEADER_OFFSET + new_header.exec_location;

        while (left > 0) {
                size_t chunk = left > FLASH_SECTOR_SIZE ? FLASH_SECTOR_SIZE : left;

                if (chunk != ad_nvms_read(update_part, src_offset, sector_buffer, chunk)) {
                        goto done;
                }
                if (chunk != ad_nvms_write(exec_part, dst_offset, sector_buffer, chunk)) {
                        goto done;
                }

                left -= chunk;
                src_offset += chunk;
                dst_offset += chunk;
        }

        /*
         * Header is in different partition than executable.
         * Executable is at the beginning of partition, change location to 0.
         */
        new_header.exec_location = 0;

        /*
         * Write image header, so it can be used later and in subsequent reboots.
         */
        if (sizeof(new_header) != ad_nvms_write(header_part, 0, (uint8_t *) &new_header,
                                                                        sizeof(new_header))) {
                goto done;
        }

        /*
         * Write security extension
         */
        if (extension_length && extension_length != ad_nvms_write(header_part, sizeof(new_header),
                                                (uint8_t *) sec_ext_buff, extension_length)) {
                goto done;
        }

        /*
         * Invalidate image header in update partition.
         */
        new_header.flags &= ~SUOTA_1_1_IMAGE_FLAG_VALID;
        new_header.signature[0] = 0;
        new_header.signature[1] = 0;
        if (sizeof(new_header) == ad_nvms_write(update_part, SUOTA_IMAGE_HEADER_OFFSET,
                                                (uint8_t *) &new_header, sizeof(new_header))) {
                result = true;
        }
done:
        free(sec_ext_buff);

        return result;
}

static bool device_administration(nvms_t update_part, nvms_t exec_part, nvms_t header_part,
                                                                        size_t header_offset)
{
        sec_ext_t sec_ext;

        if (secure_device) {
                security_hdr_fw_version_t current_image_version = { };
                security_hdr_fw_version_t update_image_version = { };

                /* Get FW version number of the current image */
                if (parse_and_validate_security_ext(header_part, sizeof(suota_1_1_image_header_t),
                                                                                        &sec_ext)) {
                        current_image_version.major = sec_ext.fw_version_number.major;
                        current_image_version.minor = sec_ext.fw_version_number.minor;
                }

                /* Get FW version number of the update image */
                if (parse_and_validate_security_ext(update_part, header_offset +
                                                sizeof(suota_1_1_image_header_t), &sec_ext)) {
                        update_image_version.major = sec_ext.fw_version_number.major;
                        update_image_version.minor = sec_ext.fw_version_number.minor;
                }

                /* Compare both version numbers */
                if (compare_version_hook(&update_image_version, &current_image_version) < 0) {
                        TRACE("Update FW version number is lower than current FW version!\r\n");

                        if (!secure_boot_failure_hook(
                                                FAILURE_REASON_FW_VERSION_UPDATE_LOWER_THAN_CURRENT,
                                                                FAILURE_SOURCE_UPDATE_IMAGE)) {
                                return false;
                        }
                }
        }

        if (!update_image(update_part, exec_part, header_part)) {
                /* Some error occurs during updating FW image */
                return false;
        }

        if (!secure_device) {
                /*
                 * Rest of the device administration procedure is available only when device is
                 * 'secured'.
                 */
                return true;
        }

        if (!parse_and_validate_security_ext(header_part, header_offset +
                                                sizeof(suota_1_1_image_header_t), &sec_ext)) {
                TRACE("Invalid entry in TLV security extension!\r\n");
                goto done;
        }

        /* Try update minimum FW version */
        update_min_version(&sec_ext);
        /* Try revoke root or symmetric key/keys */
        revoke_keys(&sec_ext);

done:
        /*
         * In may cases minimum FW version cannot be update and key cannot be revoked - this is not
         * an error.
         */
        return true;
}

/* Configure cache controller */
static void configure_cache(void)
{
        uint8 cache_line_size;
        uint8 cache_associativity;
        uint8 cache_mem_size;
        uint32 cache_architecture;

        /* Read cache architecture from OTP header */
        read_otp(CACHE_ARCHITECTURE_ADDRESS, (uint8_t *) &cache_architecture, 4);

        cache_line_size = (uint8) (cache_architecture & 0x0F);
        cache_associativity = (uint8) ((cache_architecture >> 4) & 0x0F);
        cache_mem_size = (uint8) ((cache_architecture >> 8) & 0x0F);

        if (cache_mem_size == 0) {
                /* This value is not programmed - use default value (16 KB)*/
                cache_mem_size = 2;
        }

        /* Make sure the cache is disabled before configuring it */
        REG_SETF(CACHE, CACHE_CTRL2_REG, CACHE_LEN, 0);
        REG_SETF(CACHE, CACHE_CTRL2_REG, CACHE_WEN, 0);
        REG_SETF(CACHE, CACHE_CTRL2_REG, CACHE_CGEN, 0);
        REG_SETF(CACHE, CACHE_CTRL2_REG, ENABLE_ALSO_OTP_CACHED, 0);
        REG_SETF(CACHE, CACHE_CTRL2_REG, ENABLE_ALSO_QSPIFLASH_CACHED, 0);

        REG_CLR_BIT(CRG_TOP, SYS_CTRL_REG, CACHERAM_MUX);

        /* Configure the cache */
        REG_SETF(CACHE, CACHE_CTRL3_REG, CACHE_LINE_SIZE_RESET_VALUE, cache_line_size);
        REG_SETF(CACHE, CACHE_CTRL3_REG, CACHE_ASSOCIATIVITY_RESET_VALUE, cache_associativity);
        REG_SETF(CACHE, CACHE_CTRL3_REG, CACHE_RAM_SIZE_RESET_VALUE, cache_mem_size);

        /* Reset the cache controller to apply new configuration */
        REG_SET_BIT(CACHE, CACHE_CTRL3_REG, CACHE_CONTROLLER_RESET);
        REG_CLR_BIT(CACHE, CACHE_CTRL3_REG, CACHE_CONTROLLER_RESET);

        /* Disable MRM unit */
        CACHE->CACHE_MRM_CTRL_REG = 0;
        CACHE->CACHE_MRM_TINT_REG = 0;
        CACHE->CACHE_MRM_THRES_REG = 0;

        /* Set cachable area length */
        REG_SETF(CACHE, CACHE_CTRL2_REG, CACHE_LEN, 0x01ff);
}

/* Reboot device (SW reset) and configure it for running in QSPI cached mode */
static void reboot_qspi_cached_config(void)
{
        uint32_t tmp_reg;

        tmp_reg = CRG_TOP->SYS_CTRL_REG;
        REG_CLR_FIELD(CRG_TOP, SYS_CTRL_REG, REMAP_ADR0, tmp_reg);
        REG_SET_FIELD(CRG_TOP, SYS_CTRL_REG, SW_RESET, tmp_reg, 1);
        REG_SET_FIELD(CRG_TOP, SYS_CTRL_REG, REMAP_INTVECT, tmp_reg, 1);
        REG_SET_FIELD(CRG_TOP, SYS_CTRL_REG, CACHERAM_MUX, tmp_reg, 1);
        REG_SET_FIELD(CRG_TOP, SYS_CTRL_REG, REMAP_ADR0, tmp_reg, 2);
        CRG_TOP->SYS_CTRL_REG = tmp_reg;
}

void trigger_reboot(void)
{
        hw_watchdog_set_pos_val(1);
        hw_watchdog_gen_RST();
        hw_watchdog_unfreeze();

        for (;;) {
        }
}

int main(void)
{
        int32_t *int_vector_table = (int32_t *) 0;
        const int32_t *image_address;

        memset(&suota_partitions, 0, sizeof(suota_partitions));

#if defined CONFIG_RETARGET
        extern void retarget_init(void);
#endif

        /* Initialize clocks, debugger, pad latches */
        init();

        /* Setup GPIO */
        periph_init();

#if defined CONFIG_RETARGET
        retarget_init();
#endif

        TRACE("\r\nSecure Bootloader started\r\n");

        /* Init VNMS, this will read partitions needed for further processing */
        ad_nvms_init();

        suota_partitions.update_part = ad_nvms_open(NVMS_FW_UPDATE_PART);
        suota_partitions.exec_part = ad_nvms_open(NVMS_FW_EXEC_PART);
        suota_partitions.header_part = ad_nvms_open(NVMS_IMAGE_HEADER_PART);

        if (!(device_integrity_check())) {
                /* Hooks called inside function should reboot the device */
                TRACE("Device integrity check failure!\r\n");
        }

        TRACE("Checking update image...\r\n");
        /* Check if there is valid image for update */
        if (fw_validation(suota_partitions.update_part, suota_partitions.update_part,
                                                                SUOTA_IMAGE_HEADER_OFFSET)) {
                TRACE("Update image is valid - perform device administration actions...\r\n");
                if(!device_administration(suota_partitions.update_part, suota_partitions.exec_part,
                                        suota_partitions.header_part, SUOTA_IMAGE_HEADER_OFFSET)) {
                        TRACE("Device administration action failed, rebooting!\r\n");
                        trigger_reboot();
                }
        }

        /*
         * Check if current image is valid, CRC can be forced by image header but it is not
         * forced here.
         */
        if (!fw_validation(suota_partitions.header_part, suota_partitions.exec_part, 0)) {
                TRACE("No valid image, rebooting\r\n");
                trigger_reboot();
        }

        /*
         * Following code assumes that code will be executed from QSPI mapped FLASH
         *
         * Binary image that is stored in QSPI flash must be compiled for specific address,
         * this address should not be 0 since this is where boot loader is stored.
         * Image stored in QSPI (except for boot loader image) does not need to be modified
         * in any way before it is flashed.
         * This image starts from initial stack pointer, and reset handler.
         * Those two value will not be copied to RAM. All other vectors will be copied from
         * image location to RAM.
         */
        if (256 != ad_nvms_get_pointer(suota_partitions.exec_part, 0, 256,
                                                                (const void **) &image_address)) {
                trigger_reboot();
        }

        /* Check sanity of image */
        if (!image_sanity_check(image_address)) {
                TRACE("Current executable is insane\r\n");

                if (!secure_boot_failure_hook(FAILURE_REASON_IMAGE_INSANE,
                                                                FAILURE_SOURCE_EXEC_IMAGE)) {
                        trigger_reboot();
                }
        }

        if (secure_device) {
                mark_invalid_symmetric_keys();
                /* Set 'secure boot' field - available only for secure devices */
                REG_SET_BIT(CRG_TOP, SECURE_BOOT_REG, SECURE_BOOT);
        }

        TRACE("Starting image at 0x%X, reset vector 0x%X.\r\n", (unsigned int) image_address,
                                                                (unsigned int) image_address[1]);

        __disable_irq();

        /* Reconfigure from OTP mirror mode to QSPI cached mode */
        configure_cache();

        /* Copy interrupt vector table from image */
        memcpy(int_vector_table, image_address, 0x100);

        /*
         * If bootloader changed any configuration (GPIO, clocks) it should be uninitialized here
         */
        periph_deinit();

        /*
         * Reset platform - configure for start in QSPI cached mode
         */
        reboot_qspi_cached_config();

        for (;;) {
        }

        return 0;
}
