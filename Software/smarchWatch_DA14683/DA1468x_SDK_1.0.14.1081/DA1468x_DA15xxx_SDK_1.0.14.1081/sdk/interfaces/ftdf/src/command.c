/**
 ****************************************************************************************
 *
 * @file command.c
 *
 * @brief FTDF command frame handler functions
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <string.h>
#include <stdlib.h>
#include <ftdf.h>
#include "internal.h"
#include "sdk_defs.h"

#ifdef CONFIG_USE_FTDF
#ifndef FTDF_LITE
typedef struct {
        ftdf_ie_list_t ie_list;
        ftdf_sub_ie_list_t sub_ie_list;
        ftdf_ie_descriptor_t ie[FTDF_MAX_PAYLOAD_IES];
        ftdf_sub_ie_descriptor_t sub_ie[FTDF_MAX_SUB_IES];
        ftdf_octet_t content[FTDF_MAX_IE_CONTENT];
} ftdf_ie_buffer_t;

__RETAINED_RW static ftdf_remote_request_t ftdf_remote_msg_buf = { FTDF_REMOTE_REQUEST };
static ftdf_energy_t ftdf_energies[FTDF_NR_OF_CHANNELS];
static ftdf_pan_descriptor_t ftdf_pan_descrs[FTDF_NR_OF_SCAN_RESULTS];
static ftdf_bitmap32_t ftdf_scan_channels;
static ftdf_channel_number_t ftdf_current_scan_channel;
static ftdf_channel_number_t ftdf_current_scan_result;
static ftdf_coord_realign_descriptor_t ftdf_coord_realign_descriptor;

#if !defined(FTDF_NO_CSL) || !defined(FTDF_NO_TSCH)
ftdf_octet_t *ftdf_add_ies(ftdf_octet_t *tx_ptr,
                           ftdf_ie_list_t *header_ie_list,
                           ftdf_ie_list_t *payload_ie_list,
                           ftdf_boolean_t with_termination_ie)
{
        uint8_t i;
        uint16_t ie_header;
        ftdf_octet_t *ie_header_ptr = (ftdf_octet_t*)&ie_header;

        if (header_ie_list) {
                for (i = 0; i < header_ie_list->nr_of_ie; i++) {
                        ftdf_ie_descriptor_t* ie = &header_ie_list->ie[i];

                        // IE header start (2 bytes)
                        // Type field fixed zero for header ie
                        ie_header = ie->length | ie->id << 7;

                        *tx_ptr++ = *ie_header_ptr;
                        *tx_ptr++ = *(ie_header_ptr + 1);

                        // IE header content
                        uint8_t j;

                        for (j = 0; j < ie->length; j++) {
                                *tx_ptr++ = ie->content.raw[j];
                        }
                }
        }

        if ((payload_ie_list == NULL) || (payload_ie_list->nr_of_ie == 0)) {
                if (with_termination_ie) {
                        *tx_ptr++ = 0x80;
                        *tx_ptr++ = 0x3f;
                }

                return tx_ptr;
        }

        // End of header IE list with payload IE list following ID
        *tx_ptr++ = 0x00;
        *tx_ptr++ = 0x3f;

        for (i = 0; i < payload_ie_list->nr_of_ie; i++) {
                ftdf_ie_descriptor_t *ie = &payload_ie_list->ie[i];

                // IE header start (2 bytes)
                // Type field fixed zero for header ie
                if (ie->id == 1) {
                        // MLME Group ID with sub ie
                        ftdf_sub_ie_list_t *sub_ie_list = ie->content.nested;
                        ftdf_sub_ie_descriptor_t *sub_ie = sub_ie_list->sub_ie;
                        uint16_t length = 0;
                        uint8_t n;

#ifndef FTDF_NO_TSCH
                        if (ftdf_pib.tsch_enabled) {
                                length = ftdf_get_tsch_sync_sub_ie();
                        }
#endif /* FTDF_NO_TSCH */

                        for (n = 0; n < sub_ie_list->nr_of_sub_ie; n++) {
                                length += (2 + sub_ie->length);
                                sub_ie++;
                        }

                        ie_header = length | (ie->id) << 11 | 0x8000;

                        *tx_ptr++ = *ie_header_ptr;
                        *tx_ptr++ = *(ie_header_ptr + 1);

#ifndef FTDF_NO_TSCH
                        if (ftdf_pib.tsch_enabled) {
                                tx_ptr = ftdf_add_tsch_sync_sub_ie(tx_ptr);
                        }
#endif /* FTDF_NO_TSCH */

                        sub_ie = sub_ie_list->sub_ie;

                        for (n = 0; n < sub_ie_list->nr_of_sub_ie; n++) {
                                if (sub_ie->type == FTDF_LONG_IE) {
                                        ie_header = sub_ie->length | (sub_ie->sub_id << 11) | 0x8000;
                                } else {
                                        ie_header = sub_ie->length | (sub_ie->sub_id << 8);
                                }

                                *tx_ptr++ = *ie_header_ptr;
                                *tx_ptr++ = *(ie_header_ptr + 1);

                                // IE payload content
                                uint8_t j;

                                for (j = 0; j < sub_ie->length; j++) {
                                        *tx_ptr++ = sub_ie->sub_content[j];
                                }

                                sub_ie++;
                        }
                }  else {
                        ie_header = ie->length | (ie->id << 11) | 0x8000;

                        *tx_ptr++ = *ie_header_ptr;
                        *tx_ptr++ = *(ie_header_ptr + 1);

                        // IE payload content
                        uint8_t j;

                        for (j = 0; j < ie->length; j++) {
                                *tx_ptr++ = ie->content.raw[j];
                        }
                }
        }

        if (with_termination_ie) {
                *tx_ptr++ = 0x00;
                *tx_ptr++ = 0x78;
        }

        return tx_ptr;
}

ftdf_octet_t* ftdf_get_ies(ftdf_octet_t *rx_ptr,
                           ftdf_octet_t *frame_end_ptr,
                           ftdf_ie_list_t **header_ie_list_ptr,
                           ftdf_ie_list_t **payload_ie_list_ptr)
{
        static ftdf_ie_descriptor_t header_ie_s[FTDF_MAX_HEADER_IES];
        static ftdf_ie_list_t header_ie_list;

        int n = 0;
        ftdf_ie_id_t id;
        ftdf_ie_length_t length;
        uint16_t ie_header;
        ftdf_octet_t* ie_header_ptr = (ftdf_octet_t*)&ie_header;

        do {
                *ie_header_ptr = *rx_ptr++;
                *(ie_header_ptr + 1) = *rx_ptr++;

                length = ie_header & 0x007f;
                id = ((ie_header & 0x7f80) >> 7);

                if ((id == 0x7e) || (id == 0x7f)) {
                        break;
                } else {
                        if (n < FTDF_MAX_HEADER_IES) {
                                header_ie_s[n].id = id;
                                header_ie_s[n].length = length;
                                header_ie_s[n].content.raw = rx_ptr;

                                n++;
                        }

                        rx_ptr += length;
                }
        } while (rx_ptr <= frame_end_ptr);

        if (n == 0) {
                *header_ie_list_ptr = NULL;
        } else {
                *header_ie_list_ptr = &header_ie_list;
                header_ie_list.ie = header_ie_s;
                header_ie_list.nr_of_ie = n;
        }

        if ((id == 0x7f) || (n == FTDF_MAX_HEADER_IES) || (rx_ptr >= frame_end_ptr)) {
                *payload_ie_list_ptr = NULL;

                return rx_ptr;
        }

        ftdf_ie_buffer_t *ie_buffer =
            (ftdf_ie_buffer_t*) FTDF_GET_DATA_BUFFER(sizeof(ftdf_ie_buffer_t));
        ftdf_octet_t *content = &ie_buffer->content[0];

        n = 0;

        ftdf_ie_list_t *ie_list = &ie_buffer->ie_list;
        ftdf_ie_descriptor_t *ie = &ie_buffer->ie[0];

        ie_list->ie = ie;

        do {
                *ie_header_ptr = *rx_ptr++;
                *(ie_header_ptr + 1) = *rx_ptr++;

                length = ie_header & 0x7ff;
                id = (ie_header & 0x7800) >> 11;

                if (id == 0xf) {
                        break;
                } else {
                        if (n < FTDF_MAX_PAYLOAD_IES) {
                                ie->id = id;
                                ie->length = length;
                        }

                        if (id == 1) {
                                int m = 0;
                                ftdf_sub_ie_descriptor_t *sub_ie = &ie_buffer->sub_ie[0];
                                ftdf_octet_t *end_ptr = rx_ptr + ie->length;

                                ftdf_sub_ie_list_t *sub_ie_list = &ie_buffer->sub_ie_list;

                                if (n < FTDF_MAX_PAYLOAD_IES) {
                                        ie->content.nested = sub_ie_list;
                                        sub_ie_list->sub_ie = sub_ie;
                                }

                                while (rx_ptr < end_ptr) {
                                        uint16_t sub_ie_header;
                                        ftdf_octet_t *sub_ie_header_ptr =
                                                (ftdf_octet_t*)&sub_ie_header;

                                        *sub_ie_header_ptr++ = *rx_ptr++;
                                        *sub_ie_header_ptr = *rx_ptr++;

                                        ftdf_ie_type_t type;
                                        ftdf_ie_id_t sub_id;

                                        if (sub_ie_header & 0x8000) {
                                                type = FTDF_LONG_IE;
                                                length = sub_ie_header & 0x07ff;
                                                sub_id = (sub_ie_header & 0x7800) >> 11;
                                        } else {
                                                type = FTDF_SHORT_IE;
                                                length = sub_ie_header & 0x00ff;
                                                sub_id = (sub_ie_header & 0x7f00) >> 8;
                                        }

                                        if ((n < FTDF_MAX_PAYLOAD_IES) && m < (FTDF_MAX_SUB_IES)) {
                                                sub_ie->type = type;
                                                sub_ie->sub_id = sub_id;
                                                sub_ie->length = length;
                                                sub_ie->sub_content = content;

                                                memcpy(content, rx_ptr, length);
                                                content += length;
                                                rx_ptr += length;

                                                sub_ie++;
                                                m++;
                                        }
                                }

                                if (n < FTDF_MAX_PAYLOAD_IES) {
                                        sub_ie_list->nr_of_sub_ie = m;

                                        ie++;
                                        n++;
                                }
                        } else {
                                if (n < FTDF_MAX_PAYLOAD_IES) {
                                        ie->content.raw = content;

                                        memcpy(content, rx_ptr, length);
                                        content += length;
                                        rx_ptr += length;

                                        ie++;
                                        n++;
                                } else {
                                        rx_ptr += length;
                                }
                        }
                }
        } while (rx_ptr < frame_end_ptr);

        ie_list->nr_of_ie = n;

        *payload_ie_list_ptr = (ftdf_ie_list_t*)ie_buffer;

        return rx_ptr;
}
#endif /* !FTDF_NO_CSL || !FTDF_NO_TSCH */

void ftdf_process_tx_pending(ftdf_frame_header_t *frame_header, ftdf_security_header *security_header)
{
#ifndef FTDF_NO_TSCH
        if (ftdf_pib.tsch_enabled) {
                // Ignore data requests in TSCH mode
                return;
        }
#endif /* FTDF_NO_TSCH */

        int n;
        ftdf_address_mode_t src_addr_mode = frame_header->src_addr_mode;
        ftdf_pan_id_t src_pan_id = frame_header->src_pan_id;
        ftdf_address_t src_addr = frame_header->src_addr;

        // Search for an existing indirect queue
        for (n = 0; n < FTDF_NR_OF_REQ_BUFFERS; n++) {
                if ((ftdf_tx_pending_list[n].addr_mode == src_addr_mode) &&
                    (ftdf_tx_pending_list[n].pan_id == src_pan_id)) {
                        if (src_addr_mode == FTDF_SHORT_ADDRESS) {
                                if (ftdf_tx_pending_list[n].addr.short_address == src_addr.short_address) {
                                        break;
                                }
                        } else if (src_addr_mode == FTDF_EXTENDED_ADDRESS) {
                                if (ftdf_tx_pending_list[n].addr.ext_address == src_addr.ext_address) {
                                        break;
                                }
                        } else {
                                // Invalid src_addr_mode
                                return;
                        }
                }
        }

        if (n < FTDF_NR_OF_REQ_BUFFERS) {
                ftdf_msg_buffer_t* request = ftdf_dequeue_req_tail(&ftdf_tx_pending_list[n].queue);

                if (request == NULL) {
                        return;
                }

                ftdf_remove_tx_pending_timer(request);

                if (ftdf_is_queue_empty(&ftdf_tx_pending_list[n].queue)) {
#if FTDF_FP_BIT_MODE == FTDF_FP_BIT_MODE_AUTO
                        if (src_addr_mode == FTDF_SHORT_ADDRESS) {
                                ftdf_fp_fsm_short_address_last_frame_pending(src_pan_id, src_addr.short_address);
                        } else if (src_addr_mode == FTDF_EXTENDED_ADDRESS) {
                                ftdf_fp_fsm_ext_address_last_frame_pending(src_pan_id, src_addr.ext_address);
                        } else {
                                // Invalid src_addr_mode
                                return;
                        }
#endif /* FTDF_FP_BIT_MODE == FTDF_FP_BIT_MODE_AUTO */
                        ftdf_tx_pending_list[n].addr_mode = FTDF_NO_ADDRESS;
                }

                switch (request->msg_id) {
                case FTDF_DATA_REQUEST:
                {
                        ftdf_data_request_t *data_request = (ftdf_data_request_t*) request;

                        data_request->indirect_tx = FTDF_FALSE;

                        ftdf_process_data_request(data_request);
                        break;
                }
                case FTDF_ASSOCIATE_RESPONSE:
                {
                        ftdf_associate_response_t *assoc_resp = (ftdf_associate_response_t*) request;

                        assoc_resp->fast_a = FTDF_TRUE;

                        ftdf_process_associate_response(assoc_resp);
                        break;
                }
                case FTDF_DISASSOCIATE_REQUEST:
                {
                        ftdf_disassociate_request_t *dis_req = (ftdf_disassociate_request_t*) request;

                        dis_req->tx_indirect = FTDF_FALSE;

                        ftdf_process_disassociate_request(dis_req);
                        break;
                }
                }
        } else {
                if (ftdf_req_current != NULL) {
                        return;
                }
#if FTDF_FP_BIT_TEST_MODE
                ftdf_boolean_t match_fp, fp_override, fp_force;
                ftdf_fppr_get_mode(&match_fp, &fp_override, &fp_force);
                uint32_t lmacControl3 = FTDF->FTDF_LMAC_CONTROL_3_REG;
                if (fp_override) {
                        if (!fp_force) {
                                // FP bit forced to zero. Do not send empty data.
                                return;
                        }
                } else if (match_fp) {
                        // FP bit will be zero because there is no match. Do not send empty data.
                        return;
                }
#else /* FTDF_FP_BIT_TEST_MODE */
#if FTDF_FP_BIT_MODE != FTDF_FP_BIT_MODE_ALWAYS_SET
                return;
#endif /* FTDF_FP_BIT_MODE != FTDF_FP_BIT_MODE_ALWAYS_SET */
#endif /* FTDF_FP_BIT_TEST_MODE */
                ftdf_remote_msg_buf.remote_id = FTDF_REMOTE_DATA_REQUEST;
                ftdf_req_current = (ftdf_msg_buffer_t*) &ftdf_remote_msg_buf;

                frame_header->frame_type = FTDF_DATA_FRAME;
                frame_header->dst_addr_mode = src_addr_mode;
                frame_header->dst_pan_id = src_pan_id;
                frame_header->dst_addr = src_addr;
                frame_header->options |= FTDF_OPT_ACK_REQUESTED;

                if (ftdf_is_pan_coordinator) {
                        frame_header->src_addr_mode = FTDF_NO_ADDRESS;
                } else if (ftdf_pib.short_address < 0xfffe) {
                        frame_header->src_pan_id = ftdf_pib.pan_id;
                        frame_header->src_addr_mode = FTDF_SHORT_ADDRESS;
                        frame_header->src_addr.short_address = ftdf_pib.short_address;
                } else {
                        frame_header->src_pan_id = ftdf_pib.pan_id;
                        frame_header->src_addr_mode = FTDF_EXTENDED_ADDRESS;
                        frame_header->src_addr.ext_address = ftdf_pib.ext_address;
                }

                frame_header->sn = ftdf_pib.dsn;

                ftdf_octet_t *tx_buf_ptr = (ftdf_octet_t*) &FTDF->FTDF_TX_FIFO_0_0_REG;
                ftdf_octet_t *tx_ptr = tx_buf_ptr + 1;

                tx_ptr = ftdf_add_frame_header(tx_ptr, frame_header, 0);

                security_header->security_level = ftdf_pib.mt_data_security_level;

                if (security_header->security_level > 0) {
                        frame_header->options |= FTDF_OPT_SECURITY_ENABLED;
                        security_header->key_id_mode = ftdf_pib.mt_data_key_id_mode;
                        security_header->key_source = ftdf_pib.mt_data_key_source;
                        security_header->key_index = ftdf_pib.mt_data_key_index;
                        security_header->frame_counter = ftdf_pib.frame_counter;
                        security_header->frame_counter_mode = ftdf_pib.frame_counter_mode;
                } else {
                        frame_header->options &= ~FTDF_OPT_SECURITY_ENABLED;
                }

                tx_ptr = ftdf_add_security_header(tx_ptr, security_header);

                ftdf_nr_of_retries = 0;

                ftdf_status_t status = ftdf_send_frame(ftdf_pib.current_channel,
                                                      frame_header,
                                                      security_header,
                                                      tx_ptr,
                                                      0,
                                                      NULL);

                if (status == FTDF_SUCCESS) {
                        ftdf_pib.dsn++;
                }
        }
}

void ftdf_process_command_frame(ftdf_octet_t *rx_buffer,
                                ftdf_frame_header_t *frame_header,
                                ftdf_security_header *security_header,
                                ftdf_ie_list_t *payload_ie_list)
{
        switch (frame_header->command_frame_id) {
        case FTDF_COMMAND_DATA_REQUEST:
        {
                FTDF_REL_DATA_BUFFER((ftdf_octet_t*)payload_ie_list);
                ftdf_process_tx_pending(frame_header, security_header);
                break;
        }
        case FTDF_COMMAND_BEACON_REQUEST:
        {
                if (ftdf_pib.short_address == 0xffff ||
                        ftdf_req_current != NULL) {
                        FTDF_REL_DATA_BUFFER((ftdf_octet_t*)payload_ie_list);
                        return;
                }

                if (ftdf_pib.beacon_auto_respond == FTDF_FALSE) {
                        ftdf_send_beacon_request_indication(frame_header, payload_ie_list);

                        return;
                }

                FTDF_REL_DATA_BUFFER((ftdf_octet_t*)payload_ie_list);

                ftdf_remote_msg_buf.remote_id = FTDF_REMOTE_BEACON;
                ftdf_req_current = (ftdf_msg_buffer_t*)&ftdf_remote_msg_buf;

                ftdf_beacon_type_t beacon_type =
                        frame_header->frame_version == FTDF_FRAME_VERSION_E ?
                                FTDF_ENHANCED_BEACON : FTDF_NORMAL_BEACON;

                frame_header->frame_type = FTDF_BEACON_FRAME;
                frame_header->src_pan_id = ftdf_pib.pan_id;
                frame_header->dst_addr_mode = FTDF_NO_ADDRESS;

                if (beacon_type == FTDF_NORMAL_BEACON) {
                        frame_header->options = 0;
                        frame_header->sn = ftdf_pib.bsn;
                } else {
                        if (ftdf_pib.eb_ie_list.nr_of_ie > 0 ||
                                ftdf_pib.tsch_enabled) {
                                frame_header->options = FTDF_OPT_IES_PRESENT;
                        } else {
                                frame_header->options = FTDF_OPT_ENHANCED;
                        }

                        frame_header->sn = ftdf_pib.eb_sn;
                }

                security_header->security_level = 0;

                if (ftdf_pib.short_address < 0xfffe) {
                        frame_header->src_addr_mode = FTDF_SHORT_ADDRESS;
                        frame_header->src_addr.short_address = ftdf_pib.short_address;
                } else {
                        frame_header->src_addr_mode = FTDF_EXTENDED_ADDRESS;
                        frame_header->src_addr.ext_address = ftdf_pib.ext_address;
                }

                ftdf_octet_t *tx_buf_ptr = (ftdf_octet_t *) &FTDF->FTDF_TX_FIFO_0_0_REG;
                ftdf_octet_t *tx_ptr = tx_buf_ptr + 1;

                tx_ptr = ftdf_add_frame_header(tx_ptr, frame_header, 0);

#if !defined(FTDF_NO_CSL) || !defined(FTDF_NO_TSCH)
                if (beacon_type == FTDF_ENHANCED_BEACON) {
#ifndef FTDF_NO_TSCH
                        if (ftdf_pib.tsch_enabled &&
                                ftdf_pib.eb_ie_list.nr_of_ie == 0) {
                                // Empty MLME IE, ftdf_add_ies will add the TSCH Synchronisation IE
                                const ftdf_sub_ie_list_t sub_ie_list = { 0, NULL };
                                const ftdf_ie_descriptor_t ie = { 1, 0, { (ftdf_octet_t*)&sub_ie_list } };
                                const ftdf_ie_list_t ie_list = { 1, (ftdf_ie_descriptor_t*)&ie };

                                tx_ptr = ftdf_add_ies(tx_ptr, NULL, (ftdf_ie_list_t*)&ie_list, FTDF_TRUE);
                        } else
#endif /* FTDF_NO_TSCH */
                        if (ftdf_pib.eb_ie_list.nr_of_ie != 0) {
                                tx_ptr = ftdf_add_ies(tx_ptr, NULL, &ftdf_pib.eb_ie_list, FTDF_TRUE);
                        }
                }
#endif /* !FTDF_NO_CSL || !FTDF_NO_TSCH */

                *tx_ptr++ = ftdf_pib.beacon_order & 0x0f;
                *tx_ptr++ = (ftdf_is_pan_coordinator ? 0x40 : 0) | (ftdf_pib.association_permit ? 0x80 : 0);
                *tx_ptr++ = 0;
                *tx_ptr++ = 0;

                ftdf_status_t status = ftdf_send_frame(ftdf_pib.current_channel,
                                                       frame_header,
                                                       security_header,
                                                       tx_ptr,
                                                       ftdf_pib.beacon_payload_length,
                                                       ftdf_pib.beacon_payload);

                if (status != FTDF_SUCCESS) {
                        ftdf_send_comm_status_indication(ftdf_req_current, status,
                                                         ftdf_pib.pan_id,
                                                         frame_header->src_addr_mode,
                                                         frame_header->src_addr,
                                                         frame_header->dst_addr_mode,
                                                         frame_header->dst_addr,
                                                         security_header->security_level,
                                                         security_header->key_id_mode,
                                                         security_header->key_source,
                                                         security_header->key_index);
                } else {
                        if (beacon_type == FTDF_NORMAL_BEACON) {
                                ftdf_pib.bsn++;
                        } else {
                                ftdf_pib.eb_sn++;
                        }
                }
                break;
        }
        case FTDF_COMMAND_ASSOCIATION_REQUEST:
        {
                FTDF_REL_DATA_BUFFER((ftdf_octet_t*)payload_ie_list);

                if (!ftdf_pib.association_permit) {
                        return;
                }

                ftdf_associate_indication_t *assoc_ind =
                        (ftdf_associate_indication_t*) FTDF_GET_MSG_BUFFER(
                                sizeof(ftdf_associate_indication_t));
                ftdf_octet_t *rx_ptr = rx_buffer;

                assoc_ind->msg_id = FTDF_ASSOCIATE_INDICATION;
                assoc_ind->device_address = frame_header->src_addr.ext_address;
                assoc_ind->capability_information = *rx_ptr;
                assoc_ind->security_level = security_header->security_level;
                assoc_ind->key_id_mode = security_header->key_id_mode;
                assoc_ind->key_index = security_header->key_index;
                assoc_ind->channel_offset = FTDF_TBD;
                assoc_ind->hopping_sequence_id = FTDF_TBD;

                uint8_t n;

                if (assoc_ind->key_id_mode == 0x2) {
                        for (n = 0; n < 4; n++) {
                                assoc_ind->key_source[n] = security_header->key_source[n];
                        }
                } else if (assoc_ind->key_id_mode == 0x3) {
                        for (n = 0; n < 8; n++) {
                                assoc_ind->key_source[n] = security_header->key_source[n];
                        }
                }

                FTDF_RCV_MSG((ftdf_msg_buffer_t*)assoc_ind);
                break;
        }
        case FTDF_COMMAND_ASSOCIATION_RESPONSE:
        {
                FTDF_REL_DATA_BUFFER((ftdf_octet_t*)payload_ie_list);

                if (ftdf_req_current && (ftdf_req_current->msg_id == FTDF_ASSOCIATE_REQUEST)) {
                        ftdf_short_address_t assoc_short_addr;
                        ftdf_association_status_t a_status;
                        ftdf_status_t status = FTDF_SUCCESS;

                        ftdf_octet_t *rx_ptr = rx_buffer;
                        ftdf_octet_t *short_addr_ptr = (ftdf_octet_t*)&assoc_short_addr;

                        *short_addr_ptr++ = *rx_ptr++;
                        *short_addr_ptr = *rx_ptr++;
                        a_status = *rx_ptr;

                        switch (a_status) {
                        case FTDF_ASSOCIATION_SUCCESSFUL:
                                case FTDF_FAST_ASSOCIATION_SUCCESSFUL:
                                ftdf_pib.coord_ext_address = frame_header->src_addr.ext_address;
                                status = FTDF_SUCCESS;
                                break;
                        case FTDF_ASSOCIATION_PAN_AT_CAPACITY:
                                status = FTDF_PAN_AT_CAPACITY;
                                break;
                        case FTDF_ASSOCIATION_PAN_ACCESS_DENIED:
                                status = FTDF_PAN_ACCESS_DENIED;
                                break;
                        case FTDF_ASSOCIATION_HOPPING_SEQUENCE_OFFSET_DUPLICATION:
                                status = FTDF_HOPPING_SEQUENCE_OFFSET_DUPLICATION;
                                break;
                        }

                        if (status != FTDF_SUCCESS) {
                                assoc_short_addr = 0xffff;
                        }

                        ftdf_send_associate_confirm((ftdf_associate_request_t*)ftdf_req_current,
                                                    status,
                                                    assoc_short_addr);
                }
                break;
        }
        case FTDF_COMMAND_DISASSOCIATION_NOTIFICATION:
        {
                FTDF_REL_DATA_BUFFER((ftdf_octet_t*)payload_ie_list);

                if (ftdf_req_current && ftdf_req_current->msg_id == FTDF_POLL_REQUEST) {
                        ftdf_send_poll_confirm((ftdf_poll_request_t*)ftdf_req_current,
                                FTDF_SUCCESS);
                }

                ftdf_disassociate_indication_t *dis_ind =
                        (ftdf_disassociate_indication_t*) FTDF_GET_MSG_BUFFER(
                                sizeof(ftdf_disassociate_indication_t));
                ftdf_octet_t *rx_ptr = rx_buffer;

                dis_ind->msg_id = FTDF_DISASSOCIATE_INDICATION;
                dis_ind->device_address = frame_header->src_addr.ext_address;
                dis_ind->disassociate_reason = *rx_ptr;
                dis_ind->security_level = security_header->security_level;
                dis_ind->key_id_mode = security_header->key_id_mode;
                dis_ind->key_index = security_header->key_index;

                uint8_t n;

                if (dis_ind->key_id_mode == 0x2) {
                        for (n = 0; n < 4; n++) {
                                dis_ind->key_source[n] = security_header->key_source[n];
                        }
                } else if (dis_ind->key_id_mode == 0x3) {
                        for (n = 0; n < 8; n++) {
                                dis_ind->key_source[n] = security_header->key_source[n];
                        }
                }

                if ((dis_ind->disassociate_reason == FTDF_COORD_WISH_DEVICE_LEAVE_PAN) &&
                        (dis_ind->device_address == ftdf_pib.coord_ext_address)) {

                        ftdf_pib.associated_pan_coord = FTDF_FALSE;
                        ftdf_pib.coord_short_address = 0xffff;
                        ftdf_pib.short_address = 0xffff;
                        ftdf_set_short_address();
                        ftdf_pib.pan_id = 0xffff;
                        ftdf_setpan_id();
                }

                FTDF_RCV_MSG((ftdf_msg_buffer_t*)dis_ind);
                break;
        }
        case FTDF_COMMAND_ORPHAN_NOTIFICATION:
        {
                FTDF_REL_DATA_BUFFER((ftdf_octet_t*)payload_ie_list);

                ftdf_orphan_indication_t* orphan_ind =
                    (ftdf_orphan_indication_t*) FTDF_GET_MSG_BUFFER(sizeof(ftdf_orphan_indication_t));

                orphan_ind->msg_id = FTDF_ORPHAN_INDICATION;
                orphan_ind->orphan_address = frame_header->src_addr.ext_address;
                orphan_ind->security_level = security_header->security_level;
                orphan_ind->key_id_mode = security_header->key_id_mode;
                orphan_ind->key_index = security_header->key_index;

                uint8_t n;

                if (orphan_ind->key_id_mode == 0x2) {
                        for (n = 0; n < 4; n++) {
                                orphan_ind->key_source[n] = security_header->key_source[n];
                        }
                } else if (orphan_ind->key_id_mode == 0x3) {
                        for (n = 0; n < 8; n++) {
                                orphan_ind->key_source[n] = security_header->key_source[n];
                        }
                }

                FTDF_RCV_MSG((ftdf_msg_buffer_t*)orphan_ind);
                break;
        }
        case FTDF_COMMAND_COORDINATOR_REALIGNMENT:
        {
                FTDF_REL_DATA_BUFFER((ftdf_octet_t*)payload_ie_list);

                if (!ftdf_req_current || ftdf_req_current->msg_id != FTDF_SCAN_REQUEST) {
                        return;
                }

                ftdf_scan_request_t* scan_req = (ftdf_scan_request_t*)ftdf_req_current;

                if (scan_req->scan_type != FTDF_ORPHAN_SCAN) {
                        return;
                }

                ftdf_octet_t *rx_ptr = rx_buffer;
                ftdf_octet_t *octet_ptr = (ftdf_octet_t*)&ftdf_coord_realign_descriptor.coord_pan_id;

                *octet_ptr++ = *rx_ptr++;
                *octet_ptr = *rx_ptr++;

                octet_ptr = (ftdf_octet_t*)&ftdf_coord_realign_descriptor.coord_short_addr;

                *octet_ptr++ = *rx_ptr++;
                *octet_ptr = *rx_ptr++;

                ftdf_coord_realign_descriptor.channel_number = *rx_ptr++;

                octet_ptr = (ftdf_octet_t*)&ftdf_coord_realign_descriptor.short_addr;

                *octet_ptr++ = *rx_ptr++;
                *octet_ptr = *rx_ptr++;

                // channel page only present with frame version 1
                if (frame_header->frame_version == FTDF_FRAME_VERSION_2011) {
                        ftdf_coord_realign_descriptor.channel_page = *rx_ptr;
                } else {
                        ftdf_coord_realign_descriptor.channel_page = 0;
                }

                ftdf_current_scan_result++;

                ftdf_send_scan_confirm(scan_req, FTDF_SUCCESS);
                break;
        }
        case FTDF_COMMAND_PAN_ID_CONFLICT_NOTIFICATION:
        {
                FTDF_REL_DATA_BUFFER((ftdf_octet_t*)payload_ie_list);

                if (ftdf_is_pan_coordinator) {
                        ftdf_send_sync_loss_indication(FTDF_PAN_ID_CONFLICT, security_header);
                }

                break;
        }
        default:
                {
                FTDF_REL_DATA_BUFFER((ftdf_octet_t*)payload_ie_list);
                break;
        }
        }
}

void ftdf_process_beacon_request(ftdf_beacon_request_t *beacon_request)
{
        if (ftdf_pib.short_address == 0xffff) {
                ftdf_send_beacon_confirm(beacon_request, FTDF_INVALID_PARAMETER);
                return;
        }

#ifndef FTDF_NO_TSCH
        if (ftdf_pib.tsch_enabled && (ftdf_tsch_slot_link->request != beacon_request)) {
                ftdf_status_t status;

                if (beacon_request->dst_addr_mode == FTDF_SHORT_ADDRESS) {
                        status = ftdf_schedule_tsch((ftdf_msg_buffer_t*)beacon_request);

                        if (status == FTDF_SUCCESS) {
                                return;
                        }
                } else {
                        status = FTDF_INVALID_PARAMETER;
                }

                ftdf_send_beacon_confirm(beacon_request, status);

                return;
        }
#endif /* FTDF_NO_TSCH */

        if (ftdf_req_current == NULL) {
                ftdf_req_current = (ftdf_msg_buffer_t*)beacon_request;
        } else {
                if (ftdf_queue_req_head((ftdf_msg_buffer_t*)beacon_request,
                        &ftdf_req_queue) == FTDF_TRANSACTION_OVERFLOW) {
                        ftdf_send_beacon_confirm(beacon_request, FTDF_TRANSACTION_OVERFLOW);
                }

                return;
        }

        ftdf_frame_header_t *frame_header = &ftdf_fh;
        ftdf_security_header *security_header = &ftdf_sh;

        frame_header->frame_type = FTDF_BEACON_FRAME;
        frame_header->src_pan_id = ftdf_pib.pan_id;
        frame_header->dst_pan_id = ftdf_pib.pan_id;
        frame_header->dst_addr_mode = beacon_request->dst_addr_mode;
        frame_header->dst_addr = beacon_request->dst_addr;

        if (beacon_request->beacon_type == FTDF_NORMAL_BEACON) {
                frame_header->options = 0;

#ifndef FTDF_NO_TSCH
                if (ftdf_pib.tsch_enabled) {
                        frame_header->sn = ftdf_process_tsch_sn((ftdf_msg_buffer_t*)beacon_request,
                                                                ftdf_pib.bsn,
                                                                &beacon_request->requestSN);
                } else
                #endif /* FTDF_NO_TSCH */
                {
                        frame_header->sn = ftdf_pib.bsn;
                }
        } else {
                frame_header->options = FTDF_OPT_ENHANCED |
                    (beacon_request->bsn_suppression == FTDF_TRUE ? FTDF_OPT_SEQ_NR_SUPPRESSED : 0);

                if ((ftdf_pib.eb_ie_list.nr_of_ie > 0) || ftdf_pib.tsch_enabled) {
                        frame_header->options |= FTDF_OPT_IES_PRESENT;
                }

#ifndef FTDF_NO_TSCH
                if (ftdf_pib.tsch_enabled) {
                        frame_header->sn = ftdf_process_tsch_sn((ftdf_msg_buffer_t*)beacon_request,
                                                                ftdf_pib.eb_sn,
                                                                &beacon_request->requestSN);
                } else
                #endif /* FTDF_NO_TSCH */
                {
                        frame_header->sn = ftdf_pib.eb_sn;
                }
        }

        security_header->security_level = beacon_request->beacon_security_level;

        if (beacon_request->beacon_security_level > 0) {
                frame_header->options |= FTDF_OPT_SECURITY_ENABLED;
                security_header->key_id_mode = beacon_request->beacon_key_id_mode;
                security_header->key_index = beacon_request->beacon_key_index;
                security_header->key_source = beacon_request->beacon_key_source;
                security_header->frame_counter = ftdf_pib.frame_counter;
                security_header->frame_counter_mode = ftdf_pib.frame_counter_mode;
        }

        if (ftdf_pib.short_address < 0xfffe) {
                frame_header->src_addr_mode = FTDF_SHORT_ADDRESS;
                frame_header->src_addr.short_address = ftdf_pib.short_address;
        } else {
                frame_header->src_addr_mode = FTDF_EXTENDED_ADDRESS;
                frame_header->src_addr.ext_address = ftdf_pib.ext_address;
        }

        ftdf_octet_t *tx_buf_ptr = (ftdf_octet_t *) &FTDF->FTDF_TX_FIFO_0_0_REG;
        ftdf_octet_t *tx_ptr = tx_buf_ptr + 1;

        tx_ptr = ftdf_add_frame_header(tx_ptr, frame_header, 0);

        tx_ptr = ftdf_add_security_header(tx_ptr, security_header);

#if !defined(FTDF_NO_CSL) || !defined(FTDF_NO_TSCH)
        if (beacon_request->beacon_type == FTDF_ENHANCED_BEACON) {
#ifndef FTDF_NO_TSCH
                if (ftdf_pib.tsch_enabled && (ftdf_pib.eb_ie_list.nr_of_ie == 0)) {

                        // Empty MLME IE, ftdf_add_ies will add the TSCH Synchronisation IE
                        const ftdf_sub_ie_list_t subIEList = { 0, NULL };
                        const ftdf_ie_descriptor_t IE = { 1, 0, { (ftdf_octet_t*)&subIEList } };
                        const ftdf_ie_list_t ie_list = { 1, (ftdf_ie_descriptor_t*)&IE };

                        tx_ptr = ftdf_add_ies(tx_ptr,  NULL, (ftdf_ie_list_t*)&ie_list,  FTDF_TRUE);
                } else
#endif /* FTDF_NO_TSCH */
                if (ftdf_pib.eb_ie_list.nr_of_ie != 0) {
                        tx_ptr = ftdf_add_ies(tx_ptr, NULL, &ftdf_pib.eb_ie_list, FTDF_TRUE);
                }
        }
#endif /* !FTDF_NO_CSL || !FTDF_NO_TSCH */

        *tx_ptr++ = ((beacon_request->superframe_order & 0x0f) << 4) | (ftdf_pib.beacon_order & 0x0f);
        *tx_ptr++ = (ftdf_is_pan_coordinator ? 0x40 : 0) | (ftdf_pib.association_permit ? 0x80 : 0);
        *tx_ptr++ = 0;
        *tx_ptr++ = 0;

        ftdf_status_t status = ftdf_send_frame(beacon_request->channel,
                frame_header,
                security_header,
                tx_ptr,
                ftdf_pib.beacon_payload_length,
                ftdf_pib.beacon_payload);

        if (status != FTDF_SUCCESS) {
                ftdf_send_beacon_confirm(beacon_request, status);
        } else {
                if (beacon_request->beacon_type == FTDF_NORMAL_BEACON &&
                        frame_header->sn == ftdf_pib.bsn) {
                        ftdf_pib.bsn++;
                } else if (beacon_request->beacon_type == FTDF_ENHANCED_BEACON &&
                        frame_header->sn == ftdf_pib.eb_sn) {
                        ftdf_pib.eb_sn++;
                }
        }
}

void ftdf_process_remote_request(ftdf_remote_request_t *remote_request)
{
#ifndef FTDF_NO_TSCH
        if ((remote_request->remote_id != FTDF_REMOTE_KEEP_ALIVE) ||
                (ftdf_pib.tsch_enabled == FTDF_FALSE)) {
                return;
        }

        if (ftdf_tsch_slot_link->request != remote_request) {
                (void)ftdf_schedule_tsch((ftdf_msg_buffer_t*)remote_request);
                return;
        }

        if (ftdf_req_current == NULL) {
                ftdf_req_current = (ftdf_msg_buffer_t*)remote_request;
        } else {
                (void)ftdf_queue_req_head((ftdf_msg_buffer_t*)remote_request, &ftdf_req_queue);
                return;
        }

        ftdf_frame_header_t *frame_header = &ftdf_fh;
        ftdf_security_header *security_header = &ftdf_sh;

        frame_header->frame_type = FTDF_DATA_FRAME;
        frame_header->options = FTDF_OPT_ACK_REQUESTED;
        frame_header->src_pan_id = ftdf_pib.pan_id;
        frame_header->dst_pan_id = ftdf_pib.pan_id;
        frame_header->src_addr_mode = FTDF_SHORT_ADDRESS;
        frame_header->src_addr.short_address = ftdf_pib.short_address;
        frame_header->dst_addr_mode = FTDF_SHORT_ADDRESS;
        frame_header->dst_addr.short_address = remote_request->dst_addr;
        frame_header->sn = ftdf_pib.dsn;

        security_header->security_level = 0;

        ftdf_octet_t *tx_buf_ptr = (ftdf_octet_t*) &FTDF->FTDF_TX_FIFO_0_0_REG;
        ftdf_octet_t *tx_ptr = tx_buf_ptr + 1;

        tx_ptr = ftdf_add_frame_header(tx_ptr, frame_header, 0);

        tx_ptr = ftdf_add_security_header(tx_ptr, security_header);

        ftdf_nr_of_retries = 0;

        ftdf_status_t status = ftdf_send_frame(ftdf_pib.current_channel,
                                               frame_header,
                                               security_header,
                                               tx_ptr,
                                               0,
                                               NULL);

        if (status == FTDF_SUCCESS) {
                ftdf_pib.dsn++;
        }
#endif /* FTDF_NO_TSCH */
}

void ftdf_process_scan_request(ftdf_scan_request_t *scan_request)
{
        if (ftdf_req_current == NULL) {
                ftdf_req_current = (ftdf_msg_buffer_t *)scan_request;
        } else if (ftdf_req_current->msg_id == FTDF_SCAN_REQUEST) {
                ftdf_send_scan_confirm(scan_request, FTDF_SCAN_IN_PROGRESS);
                return;
        } else {
                if (ftdf_queue_req_head((ftdf_msg_buffer_t *)scan_request, &ftdf_req_queue) ==
                        FTDF_TRANSACTION_OVERFLOW) {

                        ftdf_send_scan_confirm(scan_request, FTDF_TRANSACTION_OVERFLOW);
                }

                return;
        }

        ftdf_scan_channels = scan_request->scan_channels & 0x07fff800;
        ftdf_current_scan_result = 0;

        if ((scan_request->channel_page != 0) || ((ftdf_scan_channels & 0x07fff800) == 0)) {
                ftdf_send_scan_confirm(scan_request, FTDF_INVALID_PARAMETER);

                return;
        }

        ftdf_current_scan_channel = 11;

        while ((ftdf_scan_channels & (1 << ftdf_current_scan_channel)) == 0) {
                ftdf_current_scan_channel++;
        }

        uint16_t phy_attr_1 = 0;
        if (scan_request->scan_type == FTDF_ED_SCAN) {
                REG_SET_FIELD(FTDF, FTDF_LMAC_CONTROL_1_REG, PHYRXATTR_DEM_PTI, phy_attr_1, 0x04);
                REG_SET_FIELD(FTDF, FTDF_LMAC_CONTROL_1_REG, PHYRXATTR_CN, phy_attr_1,
                              (ftdf_current_scan_channel - 11));
        } else {
                REG_SET_FIELD(FTDF, FTDF_LMAC_CONTROL_1_REG, PHYRXATTR_CN, phy_attr_1,
                              (ftdf_current_scan_channel  - 11));
        }
        FTDF->FTDF_LMAC_CONTROL_1_REG = phy_attr_1;

        uint32_t phy_attr_4 = FTDF->FTDF_LMAC_CONTROL_4_REG;
        REG_SET_FIELD(FTDF, FTDF_LMAC_CONTROL_4_REG, PHYACKATTR_DEM_PTI, phy_attr_4, 0x08);
        REG_SET_FIELD(FTDF, FTDF_LMAC_CONTROL_4_REG, PHYACKATTR_CN, phy_attr_4,
                      (ftdf_current_scan_channel - 11));
        REG_SET_FIELD(FTDF, FTDF_LMAC_CONTROL_4_REG, PHYACKATTR_CALCAP, phy_attr_4, 0);
        REG_SET_FIELD(FTDF, FTDF_LMAC_CONTROL_4_REG, PHYACKATTR_RF_GPIO_PINS, phy_attr_4,
                      (ftdf_pib.tx_power & 0x7));
        REG_SET_FIELD(FTDF, FTDF_LMAC_CONTROL_4_REG, PHYACKATTR_HSI, phy_attr_4, 0);
        FTDF->FTDF_LMAC_CONTROL_4_REG = phy_attr_4;

        uint32_t scan_duration = FTDF_BASE_SUPERFRAME_DURATION * ((1 << scan_request->scan_duration) + 1);

        switch (scan_request->scan_type) {
        case FTDF_ED_SCAN:
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_OS_REG, RXENABLE, 0);
                REG_SETF(FTDF, FTDF_RX_CONTROL_0_REG, RXBEACONONLY, 1);
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_2_REG, EDSCANDURATION, scan_duration);
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_2_REG, EDSCANENABLE, 1);
                REG_SETF(FTDF, FTDF_LMAC_MASK_REG, EDSCANREADY_M, 1);
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_OS_REG, RXENABLE, 1);
                break;
        case FTDF_ACTIVE_SCAN:
                case FTDF_ENHANCED_ACTIVE_SCAN:
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_OS_REG, RXENABLE, 0);
                REG_SETF(FTDF, FTDF_RX_CONTROL_0_REG, RXBEACONONLY, 1);
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_0_REG, RXONDURATION, scan_duration);
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_OS_REG, RXENABLE, 1);
                ftdf_send_beacon_request(ftdf_current_scan_channel);
                break;
        case FTDF_PASSIVE_SCAN:
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_OS_REG, RXENABLE, 0);
                REG_SETF(FTDF, FTDF_RX_CONTROL_0_REG, RXBEACONONLY, 1);
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_0_REG, RXONDURATION, scan_duration);
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_OS_REG, RXENABLE, 1);
                break;
        case FTDF_ORPHAN_SCAN:
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_OS_REG, RXENABLE, 0);
                REG_SETF(FTDF, FTDF_RX_CONTROL_0_REG, RXCOORDREALIGNONLY, 1);
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_0_REG, RXONDURATION, (ftdf_pib.response_wait_time *
                        FTDF_BASE_SUPERFRAME_DURATION));
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_OS_REG, RXENABLE, 1);
                ftdf_send_orphan_notification(ftdf_current_scan_channel);
                break;
        }
}

void ftdf_send_scan_confirm(ftdf_scan_request_t *scan_request, ftdf_status_t status)
{
        if ((status != FTDF_TRANSACTION_OVERFLOW) && (status != FTDF_SCAN_IN_PROGRESS) &&
                (status != FTDF_INVALID_PARAMETER)) {

                REG_SETF(FTDF, FTDF_RX_CONTROL_0_REG, RXCOORDREALIGNONLY, 0);
                REG_SETF(FTDF, FTDF_RX_CONTROL_0_REG, RXBEACONONLY, 0);
                REG_SETF(FTDF, FTDF_LMAC_MASK_REG, EDSCANREADY_M, 0);
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_0_REG, RXONDURATION, 0);
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_2_REG, EDSCANENABLE, 0);

                ftdf_set_current_channel();
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_OS_REG, RXENABLE, 1);
        }

        if ((status == FTDF_SUCCESS) && (ftdf_pib.auto_request == FTDF_TRUE) &&
                ((scan_request->scan_type == FTDF_ACTIVE_SCAN) ||
                 (scan_request->scan_type == FTDF_PASSIVE_SCAN) ||
                 (scan_request->scan_type == FTDF_ENHANCED_ACTIVE_SCAN)) &&
                (ftdf_current_scan_result == 0)) {

                status = FTDF_NO_BEACON;

        } else if ((status == FTDF_SUCCESS) && (scan_request->scan_type == FTDF_ORPHAN_SCAN)) {

                if (ftdf_current_scan_result == 0) {
                        status = FTDF_NO_BEACON;
                } else {
                        ftdf_current_scan_result = 0;
                }
        }

        ftdf_scan_confirm_t *scan_confirm =
            (ftdf_scan_confirm_t *) FTDF_GET_MSG_BUFFER(sizeof(ftdf_scan_confirm_t));

        scan_confirm->msg_id = FTDF_SCAN_CONFIRM;
        scan_confirm->status = status;
        scan_confirm->scan_type = scan_request->scan_type;
        scan_confirm->channel_page = 0;
        scan_confirm->unscanned_channels = ftdf_scan_channels & 0xf80007ff;
        scan_confirm->result_list_size = ftdf_current_scan_result;
        scan_confirm->energy_detect_list = ftdf_energies;
        scan_confirm->pan_descriptor_list = ftdf_pan_descrs;
        scan_confirm->coord_realign_descriptor = &ftdf_coord_realign_descriptor;

        if (ftdf_req_current == (ftdf_msg_buffer_t *)scan_request) {
                ftdf_req_current = NULL;
        }

        FTDF_REL_MSG_BUFFER((ftdf_msg_buffer_t *)scan_request);
        FTDF_RCV_MSG((ftdf_msg_buffer_t *)scan_confirm);

        ftdf_process_next_request();
}

void ftdf_scan_ready(ftdf_scan_request_t *scan_request)
{
        if (scan_request->scan_type == FTDF_ED_SCAN) {
                ftdf_energies[ftdf_current_scan_result] =
                    REG_GETF(FTDF, FTDF_LMAC_CONTROL_STATUS_REG, EDSCANVALUE);
                ftdf_current_scan_result++;
        }

        do {
                ftdf_current_scan_channel++;
        } while (((ftdf_scan_channels & (1 << ftdf_current_scan_channel)) == 0) &&
                (ftdf_current_scan_channel < (FTDF_NR_OF_CHANNELS + 11)));

        if (ftdf_current_scan_channel == (FTDF_NR_OF_CHANNELS + 11)) {
#ifdef FTDF_PIB_LINK_QUALITY_MODE
                /* Restore link quality settings */
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_1_REG, PHYRXATTR_DEM_PTI,
                              (ftdf_pib.link_quality_mode == FTDF_LINK_QUALITY_MODE_RSSI) ? 0x8 : 0);
#endif
                ftdf_send_scan_confirm(scan_request, FTDF_SUCCESS);
                return;
        } else {
                uint16_t phy_attr_1 = 0;
                if (scan_request->scan_type == FTDF_ED_SCAN) {
                        REG_SET_FIELD(FTDF, FTDF_LMAC_CONTROL_1_REG, PHYRXATTR_DEM_PTI, phy_attr_1,
                                      (0x04));
                        REG_SET_FIELD(FTDF, FTDF_LMAC_CONTROL_1_REG, PHYRXATTR_CN, phy_attr_1,
                                      (ftdf_current_scan_channel - 11));
                } else {
                        REG_SET_FIELD(FTDF, FTDF_LMAC_CONTROL_1_REG, PHYRXATTR_CN, phy_attr_1,
                                      (ftdf_current_scan_channel - 11));
                }
                FTDF->FTDF_LMAC_CONTROL_1_REG = phy_attr_1;

                uint32_t phy_attr_4 = FTDF->FTDF_LMAC_CONTROL_4_REG;
                REG_SET_FIELD(FTDF, FTDF_LMAC_CONTROL_4_REG, PHYACKATTR_DEM_PTI, phy_attr_4, 0x08);
                REG_SET_FIELD(FTDF, FTDF_LMAC_CONTROL_4_REG, PHYACKATTR_CN, phy_attr_4,
                              (ftdf_current_scan_channel - 11));
                REG_SET_FIELD(FTDF, FTDF_LMAC_CONTROL_4_REG, PHYACKATTR_CALCAP, phy_attr_4, 0);
                REG_SET_FIELD(FTDF, FTDF_LMAC_CONTROL_4_REG, PHYACKATTR_RF_GPIO_PINS, phy_attr_4,
                              (ftdf_pib.tx_power & 0x3));
                REG_SET_FIELD(FTDF, FTDF_LMAC_CONTROL_4_REG, PHYACKATTR_HSI, phy_attr_4, 0);
                FTDF->FTDF_LMAC_CONTROL_4_REG = phy_attr_4;
        }

        switch (scan_request->scan_type) {
        case FTDF_ED_SCAN:
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_OS_REG, RXENABLE, 1);
                break;
        case FTDF_ACTIVE_SCAN:
        case FTDF_ENHANCED_ACTIVE_SCAN:
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_OS_REG, RXENABLE, 1);
                ftdf_send_beacon_request(ftdf_current_scan_channel);
                break;
        case FTDF_PASSIVE_SCAN:
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_OS_REG, RXENABLE, 1);
                break;
        case FTDF_ORPHAN_SCAN:
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_OS_REG, RXENABLE, 1);
                ftdf_send_orphan_notification(ftdf_current_scan_channel);
                break;
        }
}

void ftdf_add_pan_descriptor(ftdf_pan_descriptor_t *pan_descr)
{
        if (ftdf_current_scan_result < FTDF_NR_OF_CHANNELS) {
                int n;

                for (n = 0; n < ftdf_current_scan_result; n++) {
                        if (pan_descr->coord_pan_id == ftdf_pan_descrs[n].coord_pan_id &&
                                pan_descr->coord_addr_mode == ftdf_pan_descrs[n].coord_addr_mode &&
                                (((pan_descr->coord_addr_mode == FTDF_SHORT_ADDRESS) &&
                                        (pan_descr->coord_addr.short_address ==
                                                ftdf_pan_descrs[n].coord_addr.short_address)) ||
                                        ((pan_descr->coord_addr_mode == FTDF_EXTENDED_ADDRESS) &&
                                                (pan_descr->coord_addr.ext_address ==
                                                        ftdf_pan_descrs[n].coord_addr.ext_address)))) {
                                // Not unique
                                return;
                        }
                }

                ftdf_pan_descrs[ftdf_current_scan_result] = *pan_descr;

                ftdf_current_scan_result++;
        }
}

void ftdf_send_beacon_request(ftdf_channel_number_t channel)
{
        ftdf_frame_header_t *frame_header = &ftdf_fh;
        ftdf_security_header *security_header = &ftdf_sh;
        ftdf_scan_request_t *scan_req = (ftdf_scan_request_t*)ftdf_req_current;

        frame_header->frame_type = FTDF_MAC_COMMAND_FRAME;
        frame_header->src_addr_mode = FTDF_NO_ADDRESS;
        frame_header->dst_addr_mode = FTDF_SHORT_ADDRESS;
        frame_header->dst_pan_id = 0xffff;
        frame_header->dst_addr.short_address = 0xffff;
        frame_header->sn = ftdf_pib.dsn;

        if (scan_req->scan_type == FTDF_ENHANCED_ACTIVE_SCAN) {
                frame_header->options = FTDF_OPT_ENHANCED;
        } else {
                frame_header->options = 0;
        }

        security_header->security_level = 0;

        ftdf_octet_t *tx_ptr =
            (ftdf_octet_t *) &FTDF->FTDF_TX_FIFO_0_0_REG + (FTDF_BUFFER_LENGTH * FTDF_TX_DATA_BUFFER);

        // Skip PHY header (= MAC length)
        tx_ptr++;

        tx_ptr = ftdf_add_frame_header(tx_ptr, frame_header, 1);

        *tx_ptr++ = FTDF_COMMAND_BEACON_REQUEST;

        (void)ftdf_send_frame(channel, frame_header, security_header, tx_ptr, 0, NULL);

        ftdf_pib.dsn++;
}

void ftdf_send_beacon_confirm(ftdf_beacon_request_t *beacon_request, ftdf_status_t status)
{
        ftdf_beacon_confirm_t* beacon_confirm =
            (ftdf_beacon_confirm_t*) FTDF_GET_MSG_BUFFER(sizeof(ftdf_beacon_confirm_t));

        beacon_confirm->msg_id = FTDF_BEACON_CONFIRM;
        beacon_confirm->status = status;

        if (ftdf_req_current == (ftdf_msg_buffer_t*)beacon_request) {
                ftdf_req_current = NULL;
        }

        FTDF_REL_MSG_BUFFER((ftdf_msg_buffer_t*)beacon_request);
        FTDF_RCV_MSG((ftdf_msg_buffer_t*)beacon_confirm);

        ftdf_process_next_request();
}

void ftdf_send_orphan_notification(ftdf_channel_number_t channel)
{
        ftdf_scan_request_t *scan_req = (ftdf_scan_request_t *)ftdf_req_current;
        ftdf_frame_header_t *frame_header = &ftdf_fh;
        ftdf_security_header *security_header = &ftdf_sh;

        frame_header->frame_type = FTDF_MAC_COMMAND_FRAME;
        frame_header->options = scan_req->security_level > 0 ? FTDF_OPT_SECURITY_ENABLED : 0;
        frame_header->src_addr_mode = FTDF_EXTENDED_ADDRESS;
        frame_header->dst_addr_mode = FTDF_SHORT_ADDRESS;
        frame_header->src_pan_id = 0xffff;
        frame_header->dst_pan_id = 0xffff;
        frame_header->dst_addr.short_address = 0xffff;
        frame_header->sn = ftdf_pib.dsn;

        security_header->security_level = scan_req->security_level;
        security_header->key_id_mode = scan_req->key_id_mode;
        security_header->key_index = scan_req->key_index;
        security_header->key_source = scan_req->key_source;
        security_header->frame_counter = ftdf_pib.frame_counter;
        security_header->frame_counter_mode = ftdf_pib.frame_counter_mode;

        ftdf_octet_t *tx_ptr = (ftdf_octet_t *) &FTDF->FTDF_TX_FIFO_0_0_REG + (FTDF_BUFFER_LENGTH * FTDF_TX_DATA_BUFFER);

        // Skip PHY header (= MAC length)
        tx_ptr++;

        tx_ptr = ftdf_add_frame_header(tx_ptr, frame_header, 1);

        tx_ptr = ftdf_add_security_header(tx_ptr, security_header);

        *tx_ptr++ = FTDF_COMMAND_ORPHAN_NOTIFICATION;

        (void)ftdf_send_frame(channel, frame_header, security_header, tx_ptr, 0, NULL);

        ftdf_pib.dsn++;
}

void ftdf_process_associate_request(ftdf_associate_request_t *associate_request)
{
#ifndef FTDF_NO_TSCH
        if (ftdf_pib.tsch_enabled && (ftdf_tsch_slot_link->request != associate_request)) {
                ftdf_status_t status;

                // In TSCH mode only fast associations are allowed
                if ((associate_request->coord_addr_mode == FTDF_SHORT_ADDRESS) &&
                        (associate_request->capability_information & 0x10)) {
                        status = ftdf_schedule_tsch((ftdf_msg_buffer_t*)associate_request);

                        if (status == FTDF_SUCCESS) {
                                return;
                        }
                } else {
                        status = FTDF_INVALID_PARAMETER;
                }

                ftdf_send_associate_confirm(associate_request, status, 0xffff);

                return;
        }
#endif /* FTDF_NO_TSCH */

        if (ftdf_req_current == NULL) {
                ftdf_req_current = (ftdf_msg_buffer_t*)associate_request;
        } else {
                if (ftdf_queue_req_head((ftdf_msg_buffer_t*)associate_request,
                        &ftdf_req_queue) == FTDF_TRANSACTION_OVERFLOW) {

                        ftdf_send_associate_confirm(associate_request, FTDF_TRANSACTION_OVERFLOW, 0xffff);
                }

                return;
        }

        if (ftdf_is_pan_coordinator == FTDF_TRUE) {
                ftdf_send_associate_confirm(associate_request, FTDF_INVALID_PARAMETER, 0xffff);

                return;
        }

        // Update the PIB
        if (associate_request->coord_addr_mode == FTDF_SHORT_ADDRESS) {
                ftdf_pib.coord_short_address = associate_request->coord_addr.short_address;
        } else if (associate_request->coord_addr_mode == FTDF_EXTENDED_ADDRESS) {
                ftdf_pib.coord_ext_address = associate_request->coord_addr.ext_address;
        } else {
                ftdf_send_associate_confirm(associate_request, FTDF_INVALID_PARAMETER, 0xffff);

                return;
        }

        ftdf_pib.pan_id = associate_request->coord_pan_id;
        ftdf_setpan_id();

        ftdf_frame_header_t *frame_header = &ftdf_fh;
        ftdf_security_header *security_header = &ftdf_sh;
        ftdf_assoc_admin_t *assoc_admin = &ftdf_aa;

#ifndef FTDF_NO_TSCH
        if (ftdf_pib.tsch_enabled) {
                frame_header->sn = ftdf_process_tsch_sn((ftdf_msg_buffer_t*)associate_request,
                                                        ftdf_pib.dsn,
                                                        &associate_request->requestSN);
        } else
        #endif /* FTDF_NO_TSCH */
        {
                ftdf_pib.current_channel = associate_request->channel_number;
                ftdf_set_current_channel();

                frame_header->sn = ftdf_pib.dsn;
        }

        frame_header->frame_type = FTDF_MAC_COMMAND_FRAME;
        frame_header->command_frame_id = FTDF_COMMAND_ASSOCIATION_REQUEST;
        frame_header->options = (associate_request->security_level > 0 ? FTDF_OPT_SECURITY_ENABLED : 0) |
            FTDF_OPT_ACK_REQUESTED;
        frame_header->src_addr_mode = FTDF_EXTENDED_ADDRESS;
        frame_header->src_pan_id = 0xffff;
        frame_header->dst_addr_mode = associate_request->coord_addr_mode;
        frame_header->dst_pan_id = associate_request->coord_pan_id;
        frame_header->dst_addr = associate_request->coord_addr;

        security_header->security_level = associate_request->security_level;
        security_header->key_id_mode = associate_request->key_id_mode;
        security_header->key_index = associate_request->key_index;
        security_header->key_source = associate_request->key_source;
        security_header->frame_counter = ftdf_pib.frame_counter;
        security_header->frame_counter_mode = ftdf_pib.frame_counter_mode;

        assoc_admin->fast_a =
                associate_request->capability_information & 0x10 ? FTDF_TRUE : FTDF_FALSE;
        assoc_admin->data_r = FTDF_FALSE;

        // Always use the first TX buffer
        ftdf_octet_t *tx_ptr = (ftdf_octet_t *) &FTDF->FTDF_TX_FIFO_0_0_REG;

        // Skip PHY header (= MAC length)
        tx_ptr++;

        tx_ptr = ftdf_add_frame_header(tx_ptr, frame_header, 1);

        tx_ptr = ftdf_add_security_header(tx_ptr, security_header);

        *tx_ptr++ = FTDF_COMMAND_ASSOCIATION_REQUEST;

        ftdf_status_t status = ftdf_send_frame(ftdf_pib.current_channel,
                                               frame_header,
                                               security_header,
                                               tx_ptr,
                                               1,
                                               &associate_request->capability_information);

        if (status != FTDF_SUCCESS) {
                ftdf_send_associate_confirm(associate_request, status, 0xffff);

                return;
        }

        ftdf_nr_of_retries = 0;

        if (frame_header->sn == ftdf_pib.dsn) {
                ftdf_pib.dsn++;
        }
}

void ftdf_send_associate_confirm(ftdf_associate_request_t *assoc_req,
                                 ftdf_status_t status,
                                 ftdf_short_address_t assoc_short_addr)
{
        ftdf_associate_confirm_t *associate_confirm =
                (ftdf_associate_confirm_t*) FTDF_GET_MSG_BUFFER(sizeof(ftdf_associate_confirm_t));

        associate_confirm->msg_id = FTDF_ASSOCIATE_CONFIRM;
        associate_confirm->assoc_short_address = assoc_short_addr;
        associate_confirm->status = status;

        if (assoc_req->security_level != 0) {
                associate_confirm->security_level = assoc_req->security_level;
                associate_confirm->key_id_mode = assoc_req->key_id_mode;
                associate_confirm->key_index = assoc_req->key_index;

                uint8_t n;

                if (associate_confirm->key_id_mode == 0x2) {
                        for (n = 0; n < 4; n++) {
                                associate_confirm->key_source[n] = assoc_req->key_source[n];
                        }
                } else if (associate_confirm->key_id_mode == 0x3) {
                        for (n = 0; n < 8; n++) {
                                associate_confirm->key_source[n] = assoc_req->key_source[n];
                        }
                }
        } else {
                associate_confirm->security_level = 0;
        }

        associate_confirm->channel_offset = FTDF_TBD;
        associate_confirm->hopping_sequence_length = FTDF_TBD;
        associate_confirm->hopping_sequence = FTDF_TBD;

        if (status != FTDF_SUCCESS) {
                ftdf_pib.associated_pan_coord = FTDF_FALSE;
                ftdf_pib.coord_short_address = 0xffff;
                ftdf_pib.pan_id = 0xffff;
                ftdf_setpan_id();
        }

        ftdf_pib.short_address = assoc_short_addr;
        ftdf_set_short_address();

        if (ftdf_req_current == (ftdf_msg_buffer_t*)assoc_req) {
                ftdf_req_current = NULL;
        }

        FTDF_REL_MSG_BUFFER((ftdf_msg_buffer_t*)assoc_req);
        FTDF_RCV_MSG((ftdf_msg_buffer_t*)associate_confirm);

        ftdf_process_next_request();
}

void ftdf_send_associate_data_request(void)
{
        if ((ftdf_req_current == NULL) || (ftdf_req_current->msg_id != FTDF_ASSOCIATE_REQUEST)) {
                return;
        }

        ftdf_associate_request_t *assoc_req = (ftdf_associate_request_t*)ftdf_req_current;

        ftdf_frame_header_t *frame_header = &ftdf_fh;
        ftdf_security_header *security_header = &ftdf_sh;

        frame_header->frame_type = FTDF_MAC_COMMAND_FRAME;
        frame_header->options = (assoc_req->security_level > 0 ? FTDF_OPT_SECURITY_ENABLED : 0) |
            FTDF_OPT_ACK_REQUESTED;
        frame_header->src_addr_mode = FTDF_EXTENDED_ADDRESS;
        frame_header->src_pan_id = ftdf_pib.pan_id;
        frame_header->dst_addr_mode = assoc_req->coord_addr_mode;
        frame_header->dst_pan_id = assoc_req->coord_pan_id;
        frame_header->dst_addr = assoc_req->coord_addr;
        frame_header->sn = ftdf_pib.dsn;

        security_header->security_level = assoc_req->security_level;
        security_header->key_id_mode = assoc_req->key_id_mode;
        security_header->key_index = assoc_req->key_index;
        security_header->key_source = assoc_req->key_source;
        security_header->frame_counter = ftdf_pib.frame_counter;
        security_header->frame_counter_mode = ftdf_pib.frame_counter_mode;

        // Always use the first TX buffer
        ftdf_octet_t *tx_ptr = (ftdf_octet_t *) &FTDF->FTDF_TX_FIFO_0_0_REG;

        // Skip PHY header (= MAC length)
        tx_ptr++;

        tx_ptr = ftdf_add_frame_header(tx_ptr, frame_header, 1);

        tx_ptr = ftdf_add_security_header(tx_ptr, security_header);

        *tx_ptr++ = FTDF_COMMAND_DATA_REQUEST;

        ftdf_status_t status = ftdf_send_frame(ftdf_pib.current_channel,
                                               frame_header,
                                               security_header,
                                               tx_ptr,
                                               0,
                                               NULL);

        if (status != FTDF_SUCCESS) {
                ftdf_send_associate_confirm(assoc_req, status, 0xffff);

                return;
        }

        ftdf_nr_of_retries = 0;
        ftdf_pib.dsn++;
}

void ftdf_process_associate_response(ftdf_associate_response_t *assoc_resp)
{
#ifndef FTDF_NO_TSCH
        if (ftdf_pib.tsch_enabled &&
                ftdf_tsch_slot_link->request != assoc_resp) {
                ftdf_status_t status;

                if (assoc_resp->fast_a) {
                        status = ftdf_schedule_tsch((ftdf_msg_buffer_t*)assoc_resp);

                        if (status == FTDF_SUCCESS) {
                                return;
                        }
                } else {
                        status = FTDF_INVALID_PARAMETER;
                }

                ftdf_address_t src_addr, dst_addr;
                src_addr.ext_address = ftdf_pib.ext_address;
                dst_addr.ext_address = assoc_resp->device_address;

                ftdf_send_comm_status_indication((ftdf_msg_buffer_t*)assoc_resp,
                                                 status,
                                                 ftdf_pib.pan_id,
                                                 FTDF_EXTENDED_ADDRESS,
                                                 src_addr,
                                                 FTDF_EXTENDED_ADDRESS,
                                                 dst_addr,
                                                 assoc_resp->security_level,
                                                 assoc_resp->key_id_mode,
                                                 assoc_resp->key_source,
                                                 assoc_resp->key_index);

                return;
        }
#endif /* FTDF_NO_TSCH */

        // indirect TX
        if (assoc_resp->fast_a == FTDF_FALSE) {
                int n;
                ftdf_address_mode_t dst_addr_mode = FTDF_EXTENDED_ADDRESS;
                ftdf_pan_id_t dst_pan_Id = ftdf_pib.pan_id;
                ftdf_address_t dst_addr;

                dst_addr.ext_address = assoc_resp->device_address;

                // Search for an existing indirect queue
                for (n = 0; n < FTDF_NR_OF_REQ_BUFFERS; n++) {
                        if ((ftdf_tx_pending_list[n].addr_mode == dst_addr_mode) &&
                                (ftdf_tx_pending_list[n].addr.ext_address == dst_addr.ext_address)) {

                                if (ftdf_queue_req_head((ftdf_msg_buffer_t*)assoc_resp,
                                        &ftdf_tx_pending_list[n].queue) == FTDF_SUCCESS) {

                                        ftdf_add_tx_pending_timer((ftdf_msg_buffer_t*)assoc_resp,
                                                                  n,
                                                                  (ftdf_pib.transaction_persistence_time *
                                                                   FTDF_BASE_SUPERFRAME_DURATION),
                                                                  ftdf_send_transaction_expired);

                                        return;
                                } else {
                                        goto transaction_overflow;
                                }
                        }
                }
#if FTDF_FP_BIT_MODE == FTDF_FP_BIT_MODE_AUTO
               uint8_t entry;
               if (ftdf_fppr_get_free_ext_address(&entry) == FTDF_FALSE) {
                       goto transaction_overflow;
               }
#endif

                // Search for an empty indirect queue
                for (n = 0; n < FTDF_NR_OF_REQ_BUFFERS; n++) {
                        if (ftdf_tx_pending_list[n].addr_mode == FTDF_NO_ADDRESS) {
                                ftdf_tx_pending_list[n].addr_mode = dst_addr_mode;
                                ftdf_tx_pending_list[n].pan_id = dst_pan_Id;
                                ftdf_tx_pending_list[n].addr = dst_addr;

                                if (ftdf_queue_req_head((ftdf_msg_buffer_t*)assoc_resp,
                                            &ftdf_tx_pending_list[n].queue) == FTDF_SUCCESS) {
#if FTDF_FP_BIT_MODE == FTDF_FP_BIT_MODE_AUTO
                                        ftdf_fppr_set_ext_address(entry, dst_addr.ext_address);
                                        ftdf_fppr_set_ext_address_valid(entry, FTDF_TRUE);
#endif /* FTDF_FP_BIT_MODE == FTDF_FP_BIT_MODE_AUTO */
                                        ftdf_add_tx_pending_timer((ftdf_msg_buffer_t*)assoc_resp,
                                                                  n,
                                                                  (ftdf_pib.transaction_persistence_time *
                                                                   FTDF_BASE_SUPERFRAME_DURATION),
                                                                  ftdf_send_transaction_expired);
                                        return;
                                } else {
                                        break;
                                }
                        }
                }

                ftdf_address_t src_addr;

transaction_overflow:
                src_addr.ext_address = ftdf_pib.ext_address;
                dst_addr.ext_address = assoc_resp->device_address;

                // Did not find an existing or an empty queue
                ftdf_send_comm_status_indication((ftdf_msg_buffer_t*)assoc_resp,
                                                 FTDF_TRANSACTION_OVERFLOW,
                                                 ftdf_pib.pan_id,
                                                 FTDF_EXTENDED_ADDRESS,
                                                 src_addr,
                                                 FTDF_EXTENDED_ADDRESS,
                                                 dst_addr,
                                                 assoc_resp->security_level,
                                                 assoc_resp->key_id_mode,
                                                 assoc_resp->key_source,
                                                 assoc_resp->key_index);

                return;
        }

        // direct TX
        if (ftdf_req_current == NULL) {
                ftdf_req_current = (ftdf_msg_buffer_t*)assoc_resp;
        } else {
                if (ftdf_queue_req_head((ftdf_msg_buffer_t*)assoc_resp, &ftdf_req_queue) ==
                        FTDF_TRANSACTION_OVERFLOW) {

                        ftdf_address_t src_addr, dst_addr;
                        src_addr.ext_address = ftdf_pib.ext_address;
                        dst_addr.ext_address = assoc_resp->device_address;

                        ftdf_send_comm_status_indication((ftdf_msg_buffer_t*)assoc_resp,
                                                         FTDF_TRANSACTION_OVERFLOW,
                                                         ftdf_pib.pan_id,
                                                         FTDF_EXTENDED_ADDRESS,
                                                         src_addr,
                                                         FTDF_EXTENDED_ADDRESS,
                                                         dst_addr,
                                                         assoc_resp->security_level,
                                                         assoc_resp->key_id_mode,
                                                         assoc_resp->key_source,
                                                         assoc_resp->key_index);
                }

                return;
        }

        ftdf_frame_header_t *frame_header = &ftdf_fh;
        ftdf_security_header *security_header = &ftdf_sh;

        frame_header->frame_type = FTDF_MAC_COMMAND_FRAME;
        frame_header->options = (assoc_resp->security_level > 0 ? FTDF_OPT_SECURITY_ENABLED : 0) |
            FTDF_OPT_ACK_REQUESTED;
        frame_header->src_addr_mode = FTDF_EXTENDED_ADDRESS;
        frame_header->dst_addr_mode = FTDF_EXTENDED_ADDRESS;
        frame_header->dst_pan_id = ftdf_pib.pan_id;
        frame_header->dst_addr.ext_address = assoc_resp->device_address;

        security_header->security_level = assoc_resp->security_level;
        security_header->key_id_mode = assoc_resp->key_id_mode;
        security_header->key_index = assoc_resp->key_index;
        security_header->key_source = assoc_resp->key_source;
        security_header->frame_counter = ftdf_pib.frame_counter;
        security_header->frame_counter_mode = ftdf_pib.frame_counter_mode;

#ifndef FTDF_NO_TSCH
        if (ftdf_pib.tsch_enabled) {
                frame_header->sn = ftdf_process_tsch_sn((ftdf_msg_buffer_t*)assoc_resp,
                                                        ftdf_pib.dsn,
                                                        &assoc_resp->requestSN);
        } else
        #endif /* FTDF_NO_TSCH */
        {
                frame_header->sn = ftdf_pib.dsn;
        }

        // Always use the first TX buffer
        ftdf_octet_t *tx_ptr = (ftdf_octet_t *) &FTDF->FTDF_TX_FIFO_0_0_REG;

        // Skip PHY header (= MAC length)
        tx_ptr++;

        tx_ptr = ftdf_add_frame_header(tx_ptr, frame_header, 4);

        tx_ptr = ftdf_add_security_header(tx_ptr, security_header);

        *tx_ptr++ = FTDF_COMMAND_ASSOCIATION_RESPONSE;

        ftdf_octet_t payload[3];
        ftdf_octet_t* short_addr_ptr = (ftdf_octet_t*)&assoc_resp->assoc_short_address;

        payload[0] = *short_addr_ptr++;
        payload[1] = *short_addr_ptr;
        payload[2] = assoc_resp->status;

        ftdf_status_t status = ftdf_send_frame(ftdf_pib.current_channel,
                                               frame_header,
                                               security_header,
                                               tx_ptr,
                                               3,
                                               payload);

        if (status != FTDF_SUCCESS) {
                ftdf_address_t src_addr, dst_addr;
                src_addr.ext_address = ftdf_pib.ext_address;
                dst_addr.ext_address = assoc_resp->device_address;

                ftdf_send_comm_status_indication((ftdf_msg_buffer_t*)assoc_resp,
                                                 status,
                                                 ftdf_pib.pan_id,
                                                 FTDF_EXTENDED_ADDRESS,
                                                 src_addr,
                                                 FTDF_EXTENDED_ADDRESS,
                                                 dst_addr,
                                                 assoc_resp->security_level,
                                                 assoc_resp->key_id_mode,
                                                 assoc_resp->key_source,
                                                 assoc_resp->key_index);

                return;
        }

        ftdf_nr_of_retries = 0;

        if (frame_header->sn == ftdf_pib.dsn) {
                ftdf_pib.dsn++;
        }
}

void ftdf_process_disassociate_request(ftdf_disassociate_request_t *dis_req)
{
        if (dis_req->device_pan_id != ftdf_pib.pan_id) {
                ftdf_send_disassociate_confirm(dis_req, FTDF_INVALID_PARAMETER);
                return;
        }

#ifndef FTDF_NO_TSCH
        if (ftdf_pib.tsch_enabled &&
                (ftdf_tsch_slot_link->request != dis_req)) {

                ftdf_status_t status;

                // In TSCH mode only fast disassociations are allowed
                if ((dis_req->device_addr_mode == FTDF_SHORT_ADDRESS) &&
                        (dis_req->tx_indirect == FTDF_FALSE)) {

                        status = ftdf_schedule_tsch((ftdf_msg_buffer_t*)dis_req);

                        if (status == FTDF_SUCCESS) {
                                return;
                        }
                } else {
                        status = FTDF_INVALID_PARAMETER;
                }

                ftdf_send_disassociate_confirm(dis_req, status);

                return;
        }
#endif /* FTDF_NO_TSCH */

        ftdf_status_t status = FTDF_SUCCESS;

        // indirect TX (initiated by coordinator)
        if (dis_req->tx_indirect == FTDF_TRUE) {
                int n;
                ftdf_address_mode_t dst_addr_mode = dis_req->device_addr_mode;
                ftdf_pan_id_t dst_pan_Id = dis_req->device_pan_id;
                ftdf_address_t dst_addr = dis_req->device_address;

                // Search for an existing indirect queue
                for (n = 0; n < FTDF_NR_OF_REQ_BUFFERS; n++) {
                        if (dst_addr_mode == FTDF_SHORT_ADDRESS) {
                                if ((ftdf_tx_pending_list[n].addr_mode == dst_addr_mode) &&
                                        (ftdf_tx_pending_list[n].addr.short_address == dst_addr.short_address)) {

                                        status = ftdf_queue_req_head((ftdf_msg_buffer_t*)dis_req,
                                                                     &ftdf_tx_pending_list[n].queue);

                                        if (status == FTDF_SUCCESS) {
                                                ftdf_add_tx_pending_timer((ftdf_msg_buffer_t*)dis_req,
                                                                          n,
                                                                          (ftdf_pib.transaction_persistence_time *
                                                                           FTDF_BASE_SUPERFRAME_DURATION),
                                                                          ftdf_send_transaction_expired);

                                                return;
                                        } else {
                                                break;
                                        }
                                }

                        } else if (dst_addr_mode == FTDF_EXTENDED_ADDRESS) {

                                if ((ftdf_tx_pending_list[n].addr_mode == dst_addr_mode) &&
                                        (ftdf_tx_pending_list[n].addr.ext_address == dst_addr.ext_address)) {

                                        status = ftdf_queue_req_head((ftdf_msg_buffer_t*)dis_req,
                                                                     &ftdf_tx_pending_list[n].queue);

                                        if (status == FTDF_SUCCESS) {
                                                ftdf_add_tx_pending_timer((ftdf_msg_buffer_t*)dis_req,
                                                                          n,
                                                                          (ftdf_pib.transaction_persistence_time *
                                                                           FTDF_BASE_SUPERFRAME_DURATION),
                                                                          ftdf_send_transaction_expired);
                                                return;
                                        } else {
                                                break;
                                        }
                                }
                        } else {
                                status = FTDF_INVALID_PARAMETER;
                                break;
                        }
                }

                if (status != FTDF_SUCCESS) {
                        ftdf_send_disassociate_confirm(dis_req, status);
                        return;
                }
#if FTDF_FP_BIT_MODE == FTDF_FP_BIT_MODE_AUTO
                uint8_t entry, shortAddrIdx;
                if (dst_addr_mode == FTDF_SHORT_ADDRESS) {
                        if (ftdf_fppr_get_free_short_address(&entry, &shortAddrIdx) == FTDF_FALSE) {
                                goto transaction_overflow;
                        }
                } else if (dst_addr_mode == FTDF_EXTENDED_ADDRESS) {
                        if (ftdf_fppr_get_free_ext_address(&entry) == FTDF_FALSE) {
                                goto transaction_overflow;
                        }
                } else {
                        status = FTDF_INVALID_PARAMETER;
                }
#endif
                // Search for an empty indirect queue
                for (n = 0; n < FTDF_NR_OF_REQ_BUFFERS; n++) {
                        if (ftdf_tx_pending_list[n].addr_mode == FTDF_NO_ADDRESS) {
                                ftdf_tx_pending_list[n].addr_mode = dst_addr_mode;
                                ftdf_tx_pending_list[n].pan_id = dst_pan_Id;
                                ftdf_tx_pending_list[n].addr = dst_addr;

                                if (ftdf_queue_req_head((ftdf_msg_buffer_t*)dis_req,
                                        &ftdf_tx_pending_list[n].queue) == FTDF_SUCCESS) {
#if FTDF_FP_BIT_MODE == FTDF_FP_BIT_MODE_AUTO
                                if (dst_addr_mode == FTDF_SHORT_ADDRESS) {
                                        ftdf_fppr_set_short_address(entry, shortAddrIdx, dst_addr.short_address);
                                        ftdf_fppr_set_short_address_valid(entry, shortAddrIdx, FTDF_TRUE);
                               } else if (dst_addr_mode == FTDF_EXTENDED_ADDRESS) {
                                        ftdf_fppr_set_ext_address(entry, dst_addr.ext_address);
                                        ftdf_fppr_set_ext_address_valid(entry, FTDF_TRUE);
                               } else {
                                        status = FTDF_INVALID_PARAMETER;
                                        break;
                               }
#endif /* FTDF_FP_BIT_MODE == FTDF_FP_BIT_MODE_AUTO */
                                        ftdf_add_tx_pending_timer((ftdf_msg_buffer_t*)dis_req,
                                                                  n,
                                                                  (ftdf_pib.transaction_persistence_time *
                                                                   FTDF_BASE_SUPERFRAME_DURATION),
                                                                  ftdf_send_transaction_expired);
                                        return;
                                } else {
                                        break;
                                }
                        }
                }

                // Did not find an existing or an empty queue
#if FTDF_FP_BIT_MODE == FTDF_FP_BIT_MODE_AUTO
transaction_overflow:
#endif
                ftdf_send_disassociate_confirm(dis_req, FTDF_TRANSACTION_OVERFLOW);
                return;
        }

        // direct TX
        if (ftdf_req_current == NULL) {
                ftdf_req_current = (ftdf_msg_buffer_t*)dis_req;
        } else {
                if (ftdf_queue_req_head((ftdf_msg_buffer_t*)dis_req,
                        &ftdf_req_queue) == FTDF_TRANSACTION_OVERFLOW) {
                        ftdf_send_disassociate_confirm(dis_req, FTDF_TRANSACTION_OVERFLOW);
                }

                return;
        }

        ftdf_frame_header_t *frame_header = &ftdf_fh;
        ftdf_security_header *security_header = &ftdf_sh;

        frame_header->frame_type = FTDF_MAC_COMMAND_FRAME;
        frame_header->command_frame_id = FTDF_COMMAND_DISASSOCIATION_NOTIFICATION;
        frame_header->options = (dis_req->security_level > 0 ? FTDF_OPT_SECURITY_ENABLED : 0) |
            FTDF_OPT_ACK_REQUESTED;
        frame_header->src_addr_mode = FTDF_EXTENDED_ADDRESS;
        frame_header->dst_addr_mode = dis_req->device_addr_mode;
        frame_header->dst_pan_id = dis_req->device_pan_id;
        frame_header->dst_addr = dis_req->device_address;

        security_header->security_level = dis_req->security_level;
        security_header->key_id_mode = dis_req->key_id_mode;
        security_header->key_index = dis_req->key_index;
        security_header->key_source = dis_req->key_source;
        security_header->frame_counter = ftdf_pib.frame_counter;
        security_header->frame_counter_mode = ftdf_pib.frame_counter_mode;

#ifndef FTDF_NO_TSCH
        if (ftdf_pib.tsch_enabled) {
                frame_header->sn = ftdf_process_tsch_sn((ftdf_msg_buffer_t*)dis_req,
                                                        ftdf_pib.dsn,
                                                        &dis_req->requestSN);
        } else
        #endif /* FTDF_NO_TSCH */
        {
                frame_header->sn = ftdf_pib.dsn;
        }

        // Always use the first TX buffer
        ftdf_octet_t *tx_ptr = (ftdf_octet_t *) &FTDF->FTDF_TX_FIFO_0_0_REG;

        // Skip PHY header (= MAC length)
        tx_ptr++;

        tx_ptr = ftdf_add_frame_header(tx_ptr, frame_header, 2);

        tx_ptr = ftdf_add_security_header(tx_ptr, security_header);

        *tx_ptr++ = FTDF_COMMAND_DISASSOCIATION_NOTIFICATION;

        status = ftdf_send_frame(ftdf_pib.current_channel,
                                 frame_header,
                                 security_header,
                                 tx_ptr,
                                 1,
                                 &dis_req->disassociate_reason);

        if (status != FTDF_SUCCESS) {
                ftdf_send_disassociate_confirm(dis_req, status);

                return;
        }

        ftdf_nr_of_retries = 0;

        if (frame_header->sn == ftdf_pib.dsn) {
                ftdf_pib.dsn++;
        }
}

void ftdf_send_disassociate_confirm(ftdf_disassociate_request_t *dis_req, ftdf_status_t status)
{
        ftdf_disassociate_confirm_t* dis_conf =
                (ftdf_disassociate_confirm_t*) FTDF_GET_MSG_BUFFER(
                        sizeof(ftdf_disassociate_confirm_t));

        dis_conf->msg_id = FTDF_DISASSOCIATE_CONFIRM;
        dis_conf->status = status;
        dis_conf->device_addr_mode = dis_req->device_addr_mode;
        dis_conf->device_pan_id = dis_req->device_pan_id;
        dis_conf->device_address = dis_req->device_address;

        if ((status == FTDF_SUCCESS) &&
                (dis_req->disassociate_reason == FTDF_DEVICE_WISH_LEAVE_PAN)) {
                ftdf_pib.associated_pan_coord = FTDF_FALSE;
                ftdf_pib.coord_short_address = 0xffff;
                ftdf_pib.pan_id = 0xffff;
                ftdf_setpan_id();
                ftdf_pib.short_address = 0xffff;
                ftdf_set_short_address();
        }

        if (ftdf_req_current == (ftdf_msg_buffer_t*)dis_req) {
                ftdf_req_current = NULL;
        }

        FTDF_REL_MSG_BUFFER((ftdf_msg_buffer_t*)dis_req);
        FTDF_RCV_MSG((ftdf_msg_buffer_t*)dis_conf);
#if FTDF_FP_BIT_MODE == FTDF_FP_BIT_MODE_AUTO
        ftdf_fp_fsm_clear_pending();
#endif /* FTDF_FP_BIT_MODE */
        ftdf_process_next_request();
}

void ftdf_process_orphan_response(ftdf_orphan_response_t *orphan_resp)
{
        if (ftdf_req_current == NULL) {
                ftdf_req_current = (ftdf_msg_buffer_t*)orphan_resp;
        }
        else {
                if (ftdf_queue_req_head((ftdf_msg_buffer_t*)orphan_resp,
                        &ftdf_req_queue) == FTDF_TRANSACTION_OVERFLOW) {
                        ftdf_address_t src_addr, dst_addr;
                        src_addr.ext_address = ftdf_pib.ext_address;
                        dst_addr.ext_address = orphan_resp->orphan_address;

                        ftdf_send_comm_status_indication((ftdf_msg_buffer_t*)orphan_resp,
                                                         FTDF_TRANSACTION_OVERFLOW,
                                                         ftdf_pib.pan_id,
                                                         FTDF_EXTENDED_ADDRESS,
                                                         src_addr,
                                                         FTDF_EXTENDED_ADDRESS,
                                                         dst_addr,
                                                         orphan_resp->security_level,
                                                         orphan_resp->key_id_mode,
                                                         orphan_resp->key_source,
                                                         orphan_resp->key_index);
                }

                return;
        }

        if (orphan_resp->associated_member == FTDF_FALSE) {
                ftdf_address_t src_addr, dst_addr;
                src_addr.ext_address = ftdf_pib.ext_address;
                dst_addr.ext_address = orphan_resp->orphan_address;

                ftdf_send_comm_status_indication((ftdf_msg_buffer_t*)orphan_resp,
                                                 FTDF_INVALID_PARAMETER,
                                                 ftdf_pib.pan_id,
                                                 FTDF_EXTENDED_ADDRESS,
                                                 src_addr,
                                                 FTDF_EXTENDED_ADDRESS,
                                                 dst_addr,
                                                 orphan_resp->security_level,
                                                 orphan_resp->key_id_mode,
                                                 orphan_resp->key_source,
                                                 orphan_resp->key_index);

                return;
        }

        ftdf_frame_header_t *frame_header = &ftdf_fh;
        ftdf_security_header *security_header = &ftdf_sh;

        frame_header->frame_type = FTDF_MAC_COMMAND_FRAME;
        frame_header->options = (orphan_resp->security_level > 0 ? FTDF_OPT_SECURITY_ENABLED : 0) |
            FTDF_OPT_ACK_REQUESTED;
        frame_header->src_addr_mode = FTDF_EXTENDED_ADDRESS;
        frame_header->dst_addr_mode = FTDF_EXTENDED_ADDRESS;
        frame_header->src_pan_id = ftdf_pib.pan_id;
        frame_header->dst_pan_id = 0xffff;
        frame_header->dst_addr.ext_address = orphan_resp->orphan_address;
        frame_header->sn = ftdf_pib.dsn;

        security_header->security_level = orphan_resp->security_level;
        security_header->key_id_mode = orphan_resp->key_id_mode;
        security_header->key_index = orphan_resp->key_index;
        security_header->key_source = orphan_resp->key_source;
        security_header->frame_counter = ftdf_pib.frame_counter;
        security_header->frame_counter_mode = ftdf_pib.frame_counter_mode;

        // Always use the first TX buffer
        ftdf_octet_t *tx_ptr = (ftdf_octet_t *) &FTDF->FTDF_TX_FIFO_0_0_REG;

        // Skip PHY header (= MAC length)
        tx_ptr++;

        tx_ptr = ftdf_add_frame_header(tx_ptr, frame_header, 9);

        tx_ptr = ftdf_add_security_header(tx_ptr, security_header);

        *tx_ptr++ = FTDF_COMMAND_COORDINATOR_REALIGNMENT;

        ftdf_octet_t payload[8];
        ftdf_octet_t* octet_ptr = (ftdf_octet_t*)&ftdf_pib.pan_id;

        payload[0] = *octet_ptr++;
        payload[1] = *octet_ptr;

        octet_ptr = (ftdf_octet_t*)&ftdf_pib.short_address;

        payload[2] = *octet_ptr++;
        payload[3] = *octet_ptr;
        payload[4] = ftdf_pib.current_channel;

        octet_ptr = (ftdf_octet_t*)&orphan_resp->short_address;

        payload[5] = *octet_ptr++;
        payload[6] = *octet_ptr;
        payload[7] = 0; // channel page is only used if frame is secure

        ftdf_status_t status = ftdf_send_frame(ftdf_pib.current_channel,
                                               frame_header,
                                               security_header,
                                               tx_ptr,
                                               orphan_resp->security_level > 0 ? 8 : 7,
                                               payload);

        if (status != FTDF_SUCCESS) {
                ftdf_address_t src_addr, dst_addr;
                src_addr.ext_address = ftdf_pib.ext_address;
                dst_addr.ext_address = orphan_resp->orphan_address;

                ftdf_send_comm_status_indication((ftdf_msg_buffer_t*)orphan_resp,
                                                 status,
                                                 ftdf_pib.pan_id,
                                                 FTDF_EXTENDED_ADDRESS,
                                                 src_addr,
                                                 FTDF_EXTENDED_ADDRESS,
                                                 dst_addr,
                                                 orphan_resp->security_level,
                                                 orphan_resp->key_id_mode,
                                                 orphan_resp->key_source,
                                                 orphan_resp->key_index);

                return;
        }

        ftdf_nr_of_retries = 0;
        ftdf_pib.dsn++;
}

void ftdf_process_start_request(ftdf_start_request_t *req)
{
        ftdf_start_confirm_t *start_conf = (ftdf_start_confirm_t*) FTDF_GET_MSG_BUFFER(
                sizeof(ftdf_start_confirm_t));

        start_conf->msg_id = FTDF_START_CONFIRM;

        if (req->beacon_order != 15) {
                start_conf->status = FTDF_INVALID_PARAMETER;
        }
        else if (ftdf_pib.short_address == 0xffff) {
                start_conf->status = FTDF_NO_SHORT_ADDRESS;
        } else {
                start_conf->status = FTDF_SUCCESS;

                ftdf_pib.pan_id = req->pan_id;
                ftdf_setpan_id();

                ftdf_pib.current_channel = req->channel_number;
                ftdf_set_current_channel();

                if (req->pan_coordinator) {
                        ftdf_pib.coord_short_address = ftdf_pib.short_address;
                        ftdf_pib.coord_ext_address = ftdf_pib.ext_address;
                }

                ftdf_is_pan_coordinator = req->pan_coordinator;
                REG_SETF(FTDF, FTDF_GLOB_CONTROL_0_REG, ISPANCOORDINATOR, ftdf_is_pan_coordinator);
        }

        FTDF_REL_MSG_BUFFER((ftdf_msg_buffer_t*)req);
        FTDF_RCV_MSG((ftdf_msg_buffer_t*)start_conf);
}

void ftdf_send_sync_loss_indication(ftdf_loss_reason_t loss_reason,
        ftdf_security_header *security_header)
{
        ftdf_sync_loss_indication_t *sync_loss =
                (ftdf_sync_loss_indication_t*) FTDF_GET_MSG_BUFFER(
                        sizeof(ftdf_sync_loss_indication_t));

        sync_loss->msg_id = FTDF_SYNC_LOSS_INDICATION;
        sync_loss->loss_reason = loss_reason;
        sync_loss->pan_id = ftdf_pib.pan_id;
        sync_loss->channel_number = ftdf_pib.current_channel;
        sync_loss->channel_page = ftdf_pib.channel_page;
        sync_loss->security_level = security_header->security_level;

        if (sync_loss->security_level != 0) {
                sync_loss->key_id_mode = security_header->key_id_mode;
                sync_loss->key_index = security_header->key_index;

                uint8_t n;

                if (sync_loss->key_id_mode == 0x2) {
                        for (n = 0; n < 4; n++) {
                                sync_loss->key_source[n] = security_header->key_source[n];
                        }
                } else if (sync_loss->key_id_mode == 0x3) {
                        for (n = 0; n < 8; n++) {
                                sync_loss->key_source[n] = security_header->key_source[n];
                        }
                }
        }

        FTDF_RCV_MSG((ftdf_msg_buffer_t*)sync_loss);
}

void ftdf_sendpan_id_conflict_notification(ftdf_frame_header_t *frame_header,
        ftdf_security_header *security_header)
{
        if (ftdf_req_current != NULL) {
                return;
        }

        ftdf_remote_msg_buf.remote_id = FTDF_REMOTE_PAN_ID_CONFLICT_NOTIFICATION;
        ftdf_req_current = (ftdf_msg_buffer_t*)&ftdf_remote_msg_buf;

        frame_header->frame_type = FTDF_MAC_COMMAND_FRAME;
        frame_header->command_frame_id = FTDF_COMMAND_PAN_ID_CONFLICT_NOTIFICATION;
        frame_header->options = FTDF_OPT_ACK_REQUESTED;
        frame_header->src_addr_mode = FTDF_EXTENDED_ADDRESS;
        frame_header->dst_addr_mode = FTDF_EXTENDED_ADDRESS;
        frame_header->dst_pan_id = ftdf_pib.pan_id;
        frame_header->dst_addr.ext_address = ftdf_pib.coord_ext_address;
        frame_header->sn = ftdf_pib.dsn;

        security_header->security_level = 0;
        security_header->frame_counter = ftdf_pib.frame_counter;
        security_header->frame_counter_mode = ftdf_pib.frame_counter_mode;

        // Always use the first TX buffer
        ftdf_octet_t *tx_ptr = (ftdf_octet_t *) &FTDF->FTDF_TX_FIFO_0_0_REG;

        // Skip PHY header (= MAC length)
        tx_ptr++;

        tx_ptr = ftdf_add_frame_header(tx_ptr, frame_header, 1);

        tx_ptr = ftdf_add_security_header(tx_ptr, security_header);

        *tx_ptr++ = FTDF_COMMAND_PAN_ID_CONFLICT_NOTIFICATION;

        ftdf_nr_of_retries = 0;

        ftdf_status_t status = ftdf_send_frame(ftdf_pib.current_channel,
                                               frame_header,
                                               security_header,
                                               tx_ptr,
                                               0,
                                               NULL);

        if (status == FTDF_SUCCESS) {
                ftdf_pib.dsn++;
        }
}

void ftdf_send_beacon_request_indication(ftdf_frame_header_t *frame_header,
                                         ftdf_ie_list_t      *payload_ie_list)
{
        ftdf_beacon_request_indication_t *beacon_request_indication =
                (ftdf_beacon_request_indication_t*) FTDF_GET_MSG_BUFFER(
                        sizeof(ftdf_beacon_request_indication_t));

        beacon_request_indication->msg_id = FTDF_BEACON_REQUEST_INDICATION;
        beacon_request_indication->beacon_type = frame_header->frame_version ==
        FTDF_FRAME_VERSION_E ? FTDF_ENHANCED_BEACON : FTDF_NORMAL_BEACON;
        beacon_request_indication->src_addr_mode = frame_header->src_addr_mode;
        beacon_request_indication->src_addr = frame_header->src_addr;
        beacon_request_indication->dst_pan_id = frame_header->dst_pan_id;
        beacon_request_indication->ie_list = payload_ie_list;

        FTDF_RCV_MSG((ftdf_msg_buffer_t*)beacon_request_indication);
}

#endif /* !FTDF_LITE */
#endif /* CONFIG_USE_FTDF */
