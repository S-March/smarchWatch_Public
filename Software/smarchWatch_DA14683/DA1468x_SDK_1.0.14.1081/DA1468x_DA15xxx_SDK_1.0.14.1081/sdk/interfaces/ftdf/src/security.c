/**
 ****************************************************************************************
 *
 * @file security.c
 *
 * @brief FTDF security functions
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdlib.h>
#include <ftdf.h>
#include "internal.h"

#ifdef CONFIG_USE_FTDF
#ifndef FTDF_LITE
ftdf_frame_counter_t ftdf_secure_counter;
ftdf_frame_counter_t ftdf_unsecure_counter;

ftdf_octet_t *ftdf_add_security_header(ftdf_octet_t *tx_ptr, ftdf_security_header *security_header)
{
        ftdf_security_level_t security_level = security_header->security_level;

        if (security_level == 0) {
                return tx_ptr;
        }

        ftdf_key_id_mode_t key_id_mode = security_header->key_id_mode;

        *tx_ptr++ =
            (security_level & 0x07) |
            ((key_id_mode & 0x03) << 3) |
            ((ftdf_pib.tsch_enabled == FTDF_TRUE) ? 0x20 : 0) |
            (((security_header->frame_counter_mode == 5) || (ftdf_pib.tsch_enabled == FTDF_TRUE)) ? 0x40 : 0);

        if (ftdf_pib.tsch_enabled == FTDF_FALSE) {
                int n;
                ftdf_octet_t *frame_counter = (ftdf_octet_t*)&security_header->frame_counter;

                for (n = 0; n < security_header->frame_counter_mode; n++) {
                        *tx_ptr++ = *frame_counter++;
                }
        }

        if (key_id_mode >= 0x02) {
                int key_len;

                if (key_id_mode == 0x02) {
                        key_len = 4;
                } else {
                        key_len = 8;
                }

                ftdf_octet_t *key_source = security_header->key_source;

                int n;

                for (n = 0; n < key_len; n++) {
                        *tx_ptr++ = *(key_source + n);
                }
        }

        if (key_id_mode != 0) {
                *tx_ptr++ = security_header->key_index;
        }

        return tx_ptr;
}

ftdf_octet_t *ftdf_get_security_header(ftdf_octet_t *rx_ptr, uint8_t frame_version,
                                       ftdf_security_header *security_header)
{
        uint8_t security_control = *rx_ptr++;

        security_header->security_level = security_control & 0x07;

        ftdf_key_id_mode_t key_id_mode = (security_control >> 3) & 0x03;
        ftdf_boolean_t frame_counter_suppress = FTDF_FALSE;
        ftdf_frame_counter_mode_t frame_counter_mode;

        if (frame_version == 0b10) {
                frame_counter_mode = (security_control & 0x40) ? 5 : 4;

                if (security_control & 0x20) {
                        frame_counter_suppress = FTDF_TRUE;
                }

        }  else {
                frame_counter_mode = 4;
        }

        security_header->frame_counter_mode = frame_counter_mode;
        security_header->frame_counter = 0;

        int n;

        if (frame_counter_suppress == FTDF_FALSE) {
                uint8_t *ptr = (uint8_t*)&security_header->frame_counter;

                for (n = 0; n < frame_counter_mode; n++) {
                        *ptr++ = *rx_ptr++;
                }
        }

        if (key_id_mode >= 0x02) {
                int key_len;
                static ftdf_octet_t key_source[8];

                if (key_id_mode == 0x02) {
                        key_len = 4;
                } else {
                        key_len = 8;
                }

                for (n = 0; n < key_len; n++) {
                        key_source[n] = *rx_ptr++;
                }

                security_header->key_source = key_source;
        }

        if (key_id_mode != 0) {
                security_header->key_index = *rx_ptr++;
        }

        security_header->key_id_mode = key_id_mode;

        return rx_ptr;
}

ftdf_data_length_t ftdf_get_mic_length(ftdf_security_level_t security_level)
{
        const ftdf_data_length_t mic_lengths[4] = { 0, 4, 8, 16 };

        return mic_lengths[security_level % 4];
}

ftdf_status_t ftdf_secure_frame(ftdf_octet_t         *buf_ptr,
                                ftdf_octet_t         *priv_ptr,
                                ftdf_frame_header_t  *frame_header,
                                ftdf_security_header *security_header)
{
        ftdf_address_mode_t dev_addr_mode = frame_header->dst_addr_mode;
        ftdf_pan_id_t dev_pan_id = frame_header->dst_pan_id;
        ftdf_address_t dev_addr = frame_header->dst_addr;
        ftdf_security_level_t security_level = security_header->security_level;

        if (ftdf_pib.security_enabled == FTDF_FALSE && security_level != 0) {
                return FTDF_UNSUPPORTED_SECURITY;
        }

        if (security_level == 0) {
                return FTDF_SUCCESS;
        }

        ftdf_frame_counter_t frame_counter;
        ftdf_frame_counter_mode_t frame_counter_mode;

#ifndef FTDF_NO_TSCH
        if (ftdf_pib.tsch_enabled == FTDF_TRUE) {
                frame_counter = ftdf_tsch_slot_asn;
                frame_counter_mode = 5;
        } else
#endif /* FTDF_NO_TSCH */
        {
                frame_counter = security_header->frame_counter;
                frame_counter_mode = security_header->frame_counter_mode;
        }

        ftdf_secure_counter = frame_counter;

        if (frame_counter_mode == 4) {
                if (frame_counter == 0xffffffff) {
                        return FTDF_COUNTER_ERROR;
                }
        } else {
                if (frame_counter == 0xffffffffff) {
                        return FTDF_COUNTER_ERROR;
                }
        }

        ftdf_key_descriptor_t *key_descr = ftdf_lookup_key(dev_addr_mode,
                                                           dev_pan_id,
                                                           dev_addr,
                                                           frame_header->frame_type,
                                                           security_header->key_id_mode,
                                                           security_header->key_index,
                                                           security_header->key_source);

        if (key_descr == NULL) {
                return FTDF_UNAVAILABLE_KEY;
        }

        uint32_t security_0 = 0;
        uint8_t entry = (buf_ptr - ((ftdf_octet_t*) &FTDF->FTDF_TX_FIFO_0_0_REG)) / FTDF_BUFFER_LENGTH;
        uint8_t a_length = (priv_ptr - buf_ptr) - 1;
        uint8_t m_length = *buf_ptr - a_length
                - ftdf_get_mic_length(security_level) - FTDF_FCS_LENGTH;

        if ((security_level & 0x04) == 0) {
                a_length += m_length;
                m_length = 0;
        }

        security_0 = REG_MSK(FTDF, FTDF_SECURITY_0_REG, SECTXRXN) |
                REG_MSK(FTDF, FTDF_SECURITY_0_REG, SECENCDECN);

        REG_SET_FIELD(FTDF, FTDF_SECURITY_0_REG, SECENTRY, security_0, entry);
        REG_SET_FIELD(FTDF, FTDF_SECURITY_0_REG, SECMLENGTH, security_0, m_length);
        REG_SET_FIELD(FTDF, FTDF_SECURITY_0_REG, SECALENGTH, security_0, a_length);
        FTDF->FTDF_SECURITY_0_REG = security_0;

        /* See asic_vol v40/100/15/20 for an explanation of the mapping van security level to auth
         * and encr flags */
        const uint32_t sec_to_flags[4] = { 0x00000101, 0x00000149, 0x00000159, 0x00000179 };
        FTDF->FTDF_SECURITY_1_REG = sec_to_flags[security_level & 0x03];

        volatile uint32_t *key = &FTDF->FTDF_SECKEY_0_REG;
        uint8_t *p = key_descr->key;
        int n;

        for (n = 0; n < 4; n++) {
                *key++ = p[4 * n + 3] |
                    (p[4 * n + 2] << 8) |
                    (p[4 * n + 1] << 16) |
                    (p[4 * n + 0] << 24);
        }

        volatile uint32_t *nonce = &FTDF->FTDF_SECNONCE_0_REG;
        *nonce++ = *(((uint32_t*)&ftdf_pib.ext_address) + 1);
        *nonce++ = *(((uint32_t*)&ftdf_pib.ext_address) + 0);

        if (frame_counter_mode == 4) {
                *nonce++ = (uint32_t)(frame_counter & 0xffffffff);
                *nonce = security_level;
        } else {
                *nonce++ = (uint32_t)(frame_counter >> 8);
                *nonce = (uint32_t)(frame_counter & 0xff);
        }

        REG_SET_BIT(FTDF, FTDF_SECURITY_OS_REG, SECSTART);

        uint32_t wait = 0;
#ifdef SIMULATOR

        // At the simulator BUSY is not set directly, so wait for it
        while (!REG_GETF(FTDF, FTDF_SECURITY_STATUS_REG, SECBUSY)) {
                wait++;
        }

#endif

        while (REG_GETF(FTDF, FTDF_SECURITY_STATUS_REG, SECBUSY)) {
                wait++;
        }

        ftdf_pib.frame_counter++;

        return FTDF_SUCCESS;
}

ftdf_status_t ftdf_unsecure_frame(ftdf_octet_t         *buf_ptr,
                                  ftdf_octet_t         *priv_ptr,
                                  ftdf_frame_header_t  *frame_header,
                                  ftdf_security_header *security_header)
{
        ftdf_address_mode_t dev_addr_mode = frame_header->src_addr_mode;
        ftdf_pan_id_t dev_pan_id = frame_header->src_pan_id;
        ftdf_address_t dev_addr = frame_header->src_addr;
        ftdf_security_level_t security_level = security_header->security_level;

        if (frame_header->options & FTDF_OPT_SECURITY_ENABLED) {
                if (ftdf_pib.security_enabled == FTDF_FALSE) {
                        return FTDF_UNSUPPORTED_SECURITY;
                }

                if (frame_header->frame_version == FTDF_FRAME_VERSION_2003) {
                        return FTDF_UNSUPPORTED_LEGACY;
                }
        } else {
                if (frame_header->frame_type == FTDF_ACKNOWLEDGEMENT_FRAME) {
                        return FTDF_SUCCESS;
                }

                if (ftdf_pib.security_enabled == FTDF_FALSE) {
                        return FTDF_SUCCESS;
                }
        }

        if (dev_addr_mode == FTDF_NO_ADDRESS) {
                dev_addr_mode = FTDF_EXTENDED_ADDRESS;
                dev_addr.ext_address = ftdf_pib.coord_ext_address;
        }

        ftdf_key_descriptor_t *key_descr = ftdf_lookup_key(dev_addr_mode,
                                                           dev_pan_id,
                                                           dev_addr,
                                                           frame_header->frame_type,
                                                           security_header->key_id_mode,
                                                           security_header->key_index,
                                                           security_header->key_source);

        if (key_descr == NULL) {
                return FTDF_UNAVAILABLE_KEY;
        }

        ftdf_device_descriptor_t *device_descr =
            ftdf_lookup_device(key_descr->nr_of_device_descriptor_handles,
                               key_descr->device_descriptor_handles,
                               dev_addr_mode,
                               dev_pan_id,
                               dev_addr);

        if (device_descr == NULL) {
                return FTDF_UNAVAILABLE_DEVICE;
        }

        ftdf_security_level_descriptor_t *security_level_descr =
                ftdf_get_security_level_descr(frame_header->frame_type,
                                              frame_header->command_frame_id);

        if (security_level_descr == NULL) {
                return FTDF_UNAVAILABLE_SECURITY_LEVEL;
        }

        if ((security_level_descr->device_override_security_minimum == FTDF_TRUE) &&
            (security_level == 0)) {
                if (device_descr->exempt == FTDF_FALSE) {
                        return FTDF_IMPROPER_SECURITY_LEVEL;
                }
        } else {
                if (security_level_descr->allowed_security_levels == 0) {
                        if (security_level < security_level_descr->security_minimum) {
                                return FTDF_IMPROPER_SECURITY_LEVEL;
                        }
                } else {
                        if ((security_level_descr->allowed_security_levels & (1 << security_level))
                                == 0) {
                                return FTDF_IMPROPER_SECURITY_LEVEL;
                        }
                }
        }

        ftdf_frame_counter_t frame_counter;
        ftdf_frame_counter_mode_t frame_counter_mode;

#ifndef FTDF_NO_TSCH
        if (ftdf_pib.tsch_enabled == FTDF_TRUE) {
                frame_counter = ftdf_tsch_slot_asn;
                frame_counter_mode = 5;
        } else
#endif /* FTDF_NO_TSCH */
        {
                frame_counter = security_header->frame_counter;
                frame_counter_mode = security_header->frame_counter_mode;
        }

        ftdf_unsecure_counter = frame_counter;

        if (frame_counter_mode == 4) {
                if (frame_counter == 0xffffffff) {
                        return FTDF_COUNTER_ERROR;
                }
        } else {
                if (frame_counter == 0xffffffffff) {
                        return FTDF_COUNTER_ERROR;
                }
        }

        ftdf_size_t key_usage;
        ftdf_key_usage_descriptor_t *key_usage_descriptor = key_descr->key_usage_descriptors;

        for (key_usage = 0; key_usage < key_descr->nr_of_key_usage_descriptors; key_usage++) {
                if (key_usage_descriptor->frame_type == frame_header->frame_type) {
                        if (frame_header->frame_type == FTDF_MAC_COMMAND_FRAME) {
                                if (key_usage_descriptor->command_frame_id
                                        == frame_header->command_frame_id) {
                                        break;
                                }
                        } else {
                                break;
                        }
                }

                key_usage_descriptor++;
        }

        if (key_usage == key_descr->nr_of_key_usage_descriptors) {
                return FTDF_IMPROPER_KEY_TYPE;
        }

        if (!ftdf_pib.tsch_enabled) {
                if (frame_counter < device_descr->frame_counter) {
                        return FTDF_COUNTER_ERROR;
                }

                device_descr->frame_counter = frame_counter;
        }

        uint32_t security_0 = 0;
        uint8_t entry = (buf_ptr - ((ftdf_octet_t*) &FTDF->FTDF_RX_FIFO_0_0_REG)) / FTDF_BUFFER_LENGTH;
        uint8_t a_length = (priv_ptr - buf_ptr) - 1;
        uint8_t m_length = *buf_ptr - a_length
                - ftdf_get_mic_length(security_level) - FTDF_FCS_LENGTH;

        if ((security_level & 0x04) == 0) {
                a_length += m_length;
                m_length = 0;
        }

        REG_SET_FIELD(FTDF, FTDF_SECURITY_0_REG, SECENTRY, security_0, entry);
        REG_SET_FIELD(FTDF, FTDF_SECURITY_0_REG, SECMLENGTH, security_0, m_length);
        REG_SET_FIELD(FTDF, FTDF_SECURITY_0_REG, SECALENGTH, security_0, a_length);
        FTDF->FTDF_SECURITY_0_REG = security_0;

        /* See asic_vol v40/100/15/20 for an explanation of the mapping van security level to auth
         * and encr flags */
        const uint32_t sec_to_flags[4] = { 0x00000101, 0x00000149, 0x00000159, 0x00000179 };
        FTDF->FTDF_SECURITY_1_REG = sec_to_flags[security_level & 0x03];

        volatile uint32_t *key = &FTDF->FTDF_SECKEY_0_REG;
        uint8_t *p = key_descr->key;
        int n;

        for (n = 0; n < 4; n++) {
                *key++ = p[4 * n + 3] |
                    (p[4 * n + 2] << 8) |
                    (p[4 * n + 1] << 16) |
                    (p[4 * n + 0] << 24);
        }

        volatile uint32_t *nonce = &FTDF->FTDF_SECNONCE_0_REG;
        *nonce++ = *(((uint32_t*)&device_descr->ext_address) + 1);
        *nonce++ = *(((uint32_t*)&device_descr->ext_address) + 0);

        if (frame_counter_mode == 4) {
                *nonce++ = (uint32_t)(frame_counter & 0xffffffff);
                *nonce = security_level;
        } else {
                *nonce++ = (uint32_t)(frame_counter >> 8);
                *nonce = (uint32_t)(frame_counter & 0xff);
        }

        REG_SET_BIT(FTDF, FTDF_SECURITY_OS_REG, SECSTART);

        uint32_t wait = 0;

#ifdef SIMULATOR

        /* At the simulator BUSY is not set directly, so wait for it */
        while (!REG_GETF(FTDF, FTDF_SECURITY_STATUS_REG, SECBUSY)) {
                wait++;
        }

#endif

        while (REG_GETF(FTDF, FTDF_SECURITY_STATUS_REG, SECBUSY)) {
                wait++;
        }

        if (REG_GETF(FTDF, FTDF_SECURITY_STATUS_REG, SECAUTHFAIL)) {
                return FTDF_SECURITY_ERROR;
        }

        return FTDF_SUCCESS;
}

ftdf_key_descriptor_t *ftdf_lookup_key(ftdf_address_mode_t dev_addr_mode,
                                       ftdf_pan_id_t       dev_pan_id,
                                       ftdf_address_t      dev_addr,
                                       ftdf_frame_type_t   frame_type,
                                       ftdf_key_id_mode_t  key_id_mode,
                                       ftdf_key_index_t    key_index,
                                       ftdf_octet_t        *key_source)
{
        if (key_id_mode == 0) {
                if (dev_addr_mode == FTDF_NO_ADDRESS) {
                        ftdf_short_address_t coord_short_address = ftdf_pib.coord_short_address;

                        dev_pan_id = ftdf_pib.pan_id;

                        if ((frame_type == FTDF_BEACON_FRAME) || (coord_short_address == 0xfffe)) {
                                dev_addr.ext_address = ftdf_pib.coord_ext_address;
                                dev_addr_mode = FTDF_EXTENDED_ADDRESS;
                        }

                        if (coord_short_address < 0xfffe) {
                                dev_addr.short_address = coord_short_address;
                                dev_addr_mode = FTDF_SHORT_ADDRESS;
                        }

                        if (coord_short_address == 0xffff) {
                                return NULL;
                        }
                }
        }

        ftdf_size_t key;
        ftdf_key_descriptor_t *key_descriptor = ftdf_pib.key_table.key_descriptors;

        for (key = 0; key < ftdf_pib.key_table.nr_of_key_descriptors; key++) {
                ftdf_size_t look_up;
                ftdf_key_id_lookup_descriptor_t *key_id_lookup_descriptor =
                        key_descriptor->key_id_lookup_descriptors;

                for (look_up = 0; look_up < key_descriptor->nr_of_key_id_lookup_descriptors; look_up++) {

                        if (key_id_mode != key_id_lookup_descriptor->key_id_mode) {
                                key_id_lookup_descriptor++;
                                continue;
                        }

                        if (key_id_mode == 0) {
                                if ((dev_addr_mode == key_id_lookup_descriptor->device_addr_mode) &&
                                        (dev_pan_id == key_id_lookup_descriptor->device_pan_id)) {

                                        if ((dev_addr_mode == FTDF_EXTENDED_ADDRESS) &&
                                                (dev_addr.ext_address ==
                                                 key_id_lookup_descriptor->device_address.ext_address)) {
                                                return key_descriptor;

                                        } else if ((dev_addr_mode == FTDF_SHORT_ADDRESS) &&
                                                (dev_addr.short_address ==
                                                 key_id_lookup_descriptor->device_address.short_address)) {
                                                return key_descriptor;
                                        }
                                }
                        } else {
                                if (key_index == key_id_lookup_descriptor->key_index) {
                                        if (key_id_mode == 1) {
                                                return key_descriptor;
                                        }

                                        ftdf_size_t key_source_length;

                                        if (key_id_mode == 2) {
                                                key_source_length = 4;
                                        } else {
                                                key_source_length = 8;
                                        }

                                        int x;

                                        for (x = 0; x < key_source_length; x++) {

                                                if (key_source[x] !=
                                                        key_id_lookup_descriptor->key_source[x]) {
                                                        break;
                                                }
                                        }

                                        if (x == key_source_length) {
                                                return key_descriptor;
                                        }
                                }
                        }

                        key_id_lookup_descriptor++;
                }

                key_descriptor++;
        }

        return NULL;
}

ftdf_device_descriptor_t *ftdf_lookup_device(ftdf_size_t                     nr_of_device_descriptor_handles,
                                             ftdf_device_descriptor_handle_t *device_descriptor_handles,
                                             ftdf_address_mode_t             dev_addr_mode,
                                             ftdf_pan_id_t                   dev_pan_id,
                                             ftdf_address_t                  dev_addr)
{
        if (dev_addr_mode == FTDF_NO_ADDRESS) {
                ftdf_short_address_t coord_short_address = ftdf_pib.coord_short_address;

                dev_pan_id = ftdf_pib.pan_id;

                if (coord_short_address == 0xfffe) {
                        dev_addr.ext_address = ftdf_pib.coord_ext_address;
                        dev_addr_mode = FTDF_EXTENDED_ADDRESS;
                }

                if (coord_short_address < 0xfffe) {
                        dev_addr.short_address = coord_short_address;
                        dev_addr_mode = FTDF_SHORT_ADDRESS;
                }

                if (coord_short_address == 0xffff) {
                        return NULL;
                }
        }

        ftdf_device_descriptor_handle_t *device_descriptor_handle = device_descriptor_handles;
        ftdf_size_t handle;

        for (handle = 0; handle < nr_of_device_descriptor_handles; handle++) {
                if (*device_descriptor_handle < ftdf_pib.device_table.nr_of_device_descriptors) {
                        ftdf_device_descriptor_t* device_descriptor =
                                ftdf_pib.device_table.device_descriptors + *device_descriptor_handle;

                        if ((dev_addr_mode == FTDF_EXTENDED_ADDRESS) &&
                                (dev_addr.ext_address == device_descriptor->ext_address)) {
                                return device_descriptor;
                        } else if ((dev_addr_mode == FTDF_SHORT_ADDRESS) &&
                                (dev_addr.short_address == device_descriptor->short_address) &&
                                (dev_pan_id == device_descriptor->pan_id)) {
                                return device_descriptor;
                        }
                }

                device_descriptor_handle++;
        }

        return NULL;
}

ftdf_security_level_descriptor_t *ftdf_get_security_level_descr(ftdf_frame_type_t frame_type,
                                                                ftdf_command_frame_id_t command_frame_id)
{
        ftdf_size_t security_level;
        ftdf_security_level_descriptor_t *security_level_descriptor =
                ftdf_pib.security_level_table.security_level_descriptors;

        for (security_level = 0;
                security_level < ftdf_pib.security_level_table.nr_of_security_level_descriptors;
                security_level++) {

                if (frame_type == security_level_descriptor->frame_type) {
                        if (frame_type == FTDF_MAC_COMMAND_FRAME) {
                                if (command_frame_id == security_level_descriptor->command_frame_id) {
                                        return security_level_descriptor;
                                }
                        } else {
                                return security_level_descriptor;
                        }
                }

                security_level_descriptor++;
        }

        return NULL;
}

#endif /* !FTDF_LITE */
#endif /* CONFIG_USE_FTDF */
