/**
 ****************************************************************************************
 *
 * @file tsch.c
 *
 * @brief FTDF TSCH functions
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#include <stdlib.h>
#include <string.h>

#include <ftdf.h>
#include "internal.h"

#ifdef CONFIG_USE_FTDF
#ifndef FTDF_LITE
#define MAX_NR_OF_NODE_ADDRESSES 16

ftdf_slotframe_entry_t ftdf_slotframe_table[FTDF_MAX_SLOTFRAMES]   __attribute__ ((section(".retention")));
ftdf_link_entry_t      ftdf_link_table[FTDF_MAX_LINKS]             __attribute__ ((section(".retention")));

#ifndef FTDF_NO_TSCH
ftdf_time64_t          ftdf_tsch_slot_time                         __attribute__ ((section(".retention")));
ftdf_asn_t             ftdf_tsch_slot_asn                          __attribute__ ((section(".retention")));
ftdf_link_entry_t      *ftdf_tsch_slot_link                        __attribute__ ((section(".retention")));
ftdf_link_entry_t      *ftdf_start_links[FTDF_MAX_SLOTFRAMES]      __attribute__ ((section(".retention")));
ftdf_link_entry_t      *ftdf_end_links[FTDF_MAX_SLOTFRAMES]        __attribute__ ((section(".retention")));
int                    ftdf_lru_tsch_retry                         __attribute__ ((section(".retention")));
ftdf_tsch_retry_t      ftdf_tsch_retries[MAX_NR_OF_NODE_ADDRESSES] __attribute__ ((section(".retention")));
ftdf_time_t            ftdf_tx_offset                              __attribute__ ((section(".retention")));
FTDF_NeighborEntry     ftdf_neighbor_table[FTDF_NR_OF_NEIGHBORS]   __attribute__ ((section(".retention")));

ftdf_link_entry_t      ftdf_null_link = { 0, 0, 0, 0, 0xfffe, 0xffff, 0, NULL };

void ftdf_process_set_slotframe_request(ftdf_set_slotframe_request_t *set_slotframe_request)
{
        ftdf_status_t status = FTDF_SUCCESS;
        ftdf_size_t  nr_of_slotframes  = ftdf_pib.slotframe_table.nr_of_slotframes;
        int slotframe = 0;
        ftdf_handle_t slotframe_handle = set_slotframe_request->handle;

        while ((slotframe < nr_of_slotframes) &&
                (ftdf_slotframe_table[slotframe].slotframe_handle != slotframe_handle)) {
                slotframe++;
        }

        if (set_slotframe_request->operation == FTDF_ADD) {
                if (slotframe == nr_of_slotframes) {
                        if (nr_of_slotframes < FTDF_MAX_SLOTFRAMES) {
                                /* Find position slotframe table to add new slotframe */
                                while ((slotframe < nr_of_slotframes) &&
                                        (ftdf_slotframe_table[slotframe].slotframe_handle < slotframe_handle)) {
                                        slotframe++;
                                }

                                int add_slotframe = slotframe;

                                /* Make space for the new slotframe */
                                slotframe = nr_of_slotframes;

                                while (slotframe > add_slotframe) {
                                        ftdf_slotframe_table[slotframe] = ftdf_slotframe_table[slotframe - 1];
                                        slotframe--;
                                }

                                ftdf_slotframe_table[slotframe].slotframe_handle = set_slotframe_request->handle;
                                ftdf_slotframe_table[slotframe].slotframe_size = set_slotframe_request->size;
                                ftdf_pib.slotframe_table.nr_of_slotframes++;
                        } else {
                                status = FTDF_MAX_SLOTFRAMES_EXCEEDED;
                        }
                } else {
                        status = FTDF_INVALID_PARAMETER;
                }
        } else {
                if (slotframe == nr_of_slotframes) {
                        status = FTDF_SLOTFRAME_NOT_FOUND;
                } else {
                        if (set_slotframe_request->operation == FTDF_DELETE) {
                                while (slotframe < (nr_of_slotframes - 1)) {
                                        ftdf_slotframe_table[slotframe] = ftdf_slotframe_table[slotframe + 1];
                                        slotframe++;
                                }
                                ftdf_pib.slotframe_table.nr_of_slotframes--;
                        } else {
                                ftdf_slotframe_table[slotframe].slotframe_handle = set_slotframe_request->handle;
                                ftdf_slotframe_table[slotframe].slotframe_size   = set_slotframe_request->size;
                        }
                }
        }

        ftdf_set_slotframe_confirm_t *set_slotframe_confirm =
            (ftdf_set_slotframe_confirm_t*) FTDF_GET_MSG_BUFFER(sizeof(ftdf_set_slotframe_confirm_t));

        set_slotframe_confirm->msg_id = FTDF_SET_SLOTFRAME_CONFIRM;
        set_slotframe_confirm->handle = set_slotframe_request->handle;
        set_slotframe_confirm->status = status;

        FTDF_REL_MSG_BUFFER((ftdf_msg_buffer_t*) set_slotframe_request);

        FTDF_RCV_MSG((ftdf_msg_buffer_t*) set_slotframe_confirm);
}

static ftdf_status_t process_link_request(ftdf_set_link_request_t *set_link_request)
{
        ftdf_status_t status = FTDF_SUCCESS;
        ftdf_size_t  nr_of_links = ftdf_pib.link_table.nr_of_links;
        int link = 0;
        int slotframe;

        ftdf_handle_t link_handle = set_link_request->link_handle;

        while (link < nr_of_links) {
                if (ftdf_link_table[link].link_handle == link_handle) {
                        break;
                }

                link++;
        }

        if (set_link_request->operation == FTDF_ADD) {
                ftdf_handle_t slotframe_handle = set_link_request->slotframe_handle;
                ftdf_timeslot_t timeslot = set_link_request->timeslot;

                if (link == nr_of_links) {
                        if (nr_of_links < FTDF_MAX_LINKS) {
                               /* Link can only be added when
                                * - slotframe with given slotframe_handle exists
                                * - slotframe timeslot < slotframe_size */
                                for (slotframe = 0; slotframe < ftdf_pib.slotframe_table.nr_of_slotframes; slotframe++) {
                                        if (ftdf_slotframe_table[slotframe].slotframe_handle == slotframe_handle) {
                                                break;
                                        }
                                }

                                if ((slotframe == ftdf_pib.slotframe_table.nr_of_slotframes) ||
                                        (timeslot >= ftdf_slotframe_table[slotframe].slotframe_size)) {

                                        return FTDF_INVALID_PARAMETER;
                                }

                                /* Link cannot be added if the timeslot is in use by another link */
                                for (link = 0; link < nr_of_links; link++) {
                                        if ((ftdf_link_table[link].slotframe_handle == slotframe_handle) &&
                                                (ftdf_link_table[link].timeslot == timeslot)) {
                                                return FTDF_INVALID_PARAMETER;
                                        }
                                }

                                /* Find position in link table to add the new link */
                                link = 0;

                                while ((link < nr_of_links) &&
                                        (ftdf_link_table[link].slotframe_handle != slotframe_handle)) {
                                        link++;
                                }

                                if (link == nr_of_links) {
                                        link = 0;
                                        while ((link < nr_of_links) &&
                                                (ftdf_link_table[link].slotframe_handle < slotframe_handle)) {
                                                 link++;
                                        }
                                } else {
                                        while ((link < nr_of_links) &&
                                                (ftdf_link_table[link].slotframe_handle == slotframe_handle) &&
                                                (ftdf_link_table[link].timeslot < timeslot)) {
                                                link++;
                                        }
                                }

                                int add_link = link;

                                /* Make space for the new link */
                                link = nr_of_links;

                                while (link > add_link) {
                                        ftdf_link_table[link] = ftdf_link_table[link - 1];
                                        link--;
                                }

                                ftdf_link_table[add_link].link_handle = link_handle;
                                ftdf_link_table[add_link].slotframe_handle = slotframe_handle;
                                ftdf_link_table[add_link].timeslot = timeslot;
                                ftdf_link_table[add_link].channel_offset = set_link_request->channel_offset;
                                ftdf_link_table[add_link].link_options = set_link_request->link_options;
                                ftdf_link_table[add_link].link_type = set_link_request->link_type;
                                ftdf_link_table[add_link].node_address = set_link_request->node_address;
                                ftdf_link_table[add_link].request = NULL;
                                ftdf_pib.link_table.nr_of_links++;
                        } else {
                                status = FTDF_MAX_LINKS_EXCEEDED;
                        }
                } else {
                        status = FTDF_INVALID_PARAMETER;
                }
        } else {
                if (link == nr_of_links) {
                        status = FTDF_UNKNOWN_LINK;
                } else {
                        if (set_link_request->operation == FTDF_MODIFY) {
                                /* Since a modify is a combination of delete and add a check must be done
                                 * if the link entry can be added, otherwise the current entry would be
                                 * deleted and no new entry would be added. */
                                for (slotframe = 0; slotframe < ftdf_pib.slotframe_table.nr_of_slotframes; slotframe++) {
                                        if (ftdf_slotframe_table[slotframe].slotframe_handle == set_link_request->slotframe_handle) {
                                                break;
                                        }
                                }

                                if ((slotframe == ftdf_pib.slotframe_table.nr_of_slotframes) ||
                                      (set_link_request->timeslot >= ftdf_slotframe_table[slotframe].slotframe_size)) {
                                        return FTDF_INVALID_PARAMETER;
                                }

                                int linkentry;

                                for (linkentry = 0; linkentry < nr_of_links; linkentry++) {
                                        if ((ftdf_link_table[linkentry].slotframe_handle == set_link_request->slotframe_handle) &&
                                                (ftdf_link_table[linkentry].link_handle != link_handle) &&
                                                (ftdf_link_table[linkentry].timeslot == set_link_request->timeslot)) {
                                            return FTDF_INVALID_PARAMETER;
                                        }
                                }
                        }

                        /* In order to keep the link table sorted modify will first delete the entry
                         * and then add the new entry. */
                        while (link < (nr_of_links - 1)) {
                                ftdf_link_table[link] = ftdf_link_table[link + 1];
                                link++;
                        }

                        ftdf_pib.link_table.nr_of_links--;

                        if (set_link_request->operation == FTDF_MODIFY) {
                                set_link_request->operation = FTDF_ADD;
                                status = process_link_request(set_link_request);
                        }
                }
        }

        /* Update ftdf_start_links and ftdf_end_links */
        nr_of_links = ftdf_pib.link_table.nr_of_links;
        link = 0;

        for (slotframe = 0; slotframe < ftdf_pib.slotframe_table.nr_of_slotframes; slotframe++) {
                if ((link < nr_of_links) &&
                        (ftdf_link_table[link].slotframe_handle == ftdf_slotframe_table[slotframe].slotframe_handle)) {
                        ftdf_start_links[slotframe] = ftdf_link_table + link;
                } else {
                        /* Slotframe does not have any links */
                        ftdf_start_links[slotframe] = &ftdf_null_link;
                        ftdf_end_links[slotframe] = &ftdf_null_link;
                        continue;
                }

                ftdf_handle_t slotframe_handle = ftdf_link_table[link].slotframe_handle;

                while ((link < nr_of_links) &&
                        (ftdf_link_table[link].slotframe_handle == slotframe_handle)) {
                        link++;
                }

                ftdf_end_links[slotframe] = ftdf_link_table + link;
        }

        return status;
}

void ftdf_process_set_link_request(ftdf_set_link_request_t *set_link_request)
{
        ftdf_set_link_confirm_t *set_link_confirm =
            (ftdf_set_link_confirm_t*) FTDF_GET_MSG_BUFFER(sizeof(ftdf_set_link_confirm_t));

        ftdf_status_t status = process_link_request(set_link_request);

        if (ftdf_pib.tsch_enabled && (status == FTDF_SUCCESS) &&
                (set_link_request->operation == FTDF_ADD || FTDF_MODIFY)) {

                ftdf_schedule_tsch(NULL);
        }

        set_link_confirm->msg_id = FTDF_SET_LINK_CONFIRM;
        set_link_confirm->status = status;
        set_link_confirm->link_handle = set_link_request->link_handle;
        set_link_confirm->slotframe_handle = set_link_request->slotframe_handle;

        FTDF_REL_MSG_BUFFER((ftdf_msg_buffer_t*) set_link_request);

        FTDF_RCV_MSG((ftdf_msg_buffer_t*) set_link_confirm);
}

void ftdf_set_tsch_enabled(void)
{
        ftdf_tx_offset =
            (ftdf_pib.timeslot_template.ts_cca_offset - ftdf_pib.timeslot_template.ts_rx_offset + 8) / 16;

        ftdf_pib.tsch_enabled = FTDF_TRUE;
        ftdf_pib.le_enabled = FTDF_FALSE;
#ifndef FTDF_NO_CSL
        ftdf_set_le_enabled();
#endif /* FTDF_NO_CSL */

        REG_SETF(FTDF, FTDF_GLOB_CONTROL_0_REG, MACTSCHENABLED, 1);
        REG_SETF(FTDF, FTDF_TX_CONTROL_0_REG, MACMAXCSMABACKOFFS, 0);
        REG_SETF(FTDF, FTDF_TX_CONTROL_0_REG, MACMINBE, 0);

        ftdf_schedule_tsch(NULL);
}

void ftdf_process_tsch_mode_request(ftdf_tsch_mode_request_t* tsch_mode_request)
{
        if (tsch_mode_request->tschMode == FTDF_TSCH_ON) {
                ftdf_time64_t cur_time64 = ftdf_get_cur_time64();

                if (ftdf_is_pan_coordinator) {
                        ftdf_pib.join_priority = 0;
                        ftdf_tsch_slot_time = cur_time64;
                } else {
                        /* Convert a 32 bit timestmap into 64 bit timestamp assuming that
                         * timeslotStartTime is a timestamp in the past. */
                        ftdf_time64_t cur_time_high       = cur_time64 & 0xffffffff00000000;
                        ftdf_time_t   cur_time_low        = cur_time64 & 0x00000000ffffffff;
                        ftdf_time_t   timeslot_start_time = tsch_mode_request->timeslotStartTime;

                        if (cur_time_low < timeslot_start_time) {
                            /* The curTime has wrapped after the received beacon time used
                             * in the timeslotStartTime parameter of this TSCH-MODE request */
                            ftdf_tsch_slot_time = cur_time_high + 0x100000000 + timeslot_start_time;
                        } else {
                            ftdf_tsch_slot_time = cur_time_high + timeslot_start_time;
                        }
                }

                ftdf_tsch_slot_asn = ftdf_pib.asn;

                int n;

                for (n = 0; n < FTDF_NR_OF_REQ_BUFFERS; n++) {
                        ftdf_tx_pending_list[n].addr.short_address = 0xfffe;
                }

                for (n = 0; n < FTDF_NR_OF_NEIGHBORS; n++) {
                        ftdf_neighbor_table[n].dst_addr = 0xffff;
                }

                ftdf_set_tsch_enabled();

        } else {
                ftdf_pib.tsch_enabled = FTDF_FALSE;

                REG_SETF(FTDF, FTDF_TX_CONTROL_0_REG, MACMAXCSMABACKOFFS, ftdf_pib.max_csma_backoffs);
                REG_SETF(FTDF, FTDF_TX_CONTROL_0_REG, MACMINBE, ftdf_pib.min_be);
                REG_SETF(FTDF, FTDF_GLOB_CONTROL_0_REG, MACTSCHENABLED, 0);

                int n;

                for (n = 0; n < FTDF_NR_OF_REQ_BUFFERS; n++) {
                        ftdf_tx_pending_list[n].addr.ext_address = 0xFFFFFFFFFFFFFFFFLL;
                        ftdf_tx_pending_list[n].addr_mode = FTDF_NO_ADDRESS;
                        ftdf_tx_pending_list[n].pan_id = 0xFFFF;
                }

                for (n = 0; n < FTDF_NR_OF_NEIGHBORS; n++) {
                        if (ftdf_neighbor_table[n].dst_addr < 0xfffe) {
                                ftdf_remove_tx_pending_timer((ftdf_msg_buffer_t*)&ftdf_neighbor_table[n].msg);
                        }
                        ftdf_neighbor_table[n].dst_addr = 0xffff;
                }
        }

        ftdf_tsch_mode_confirm_t *tsch_mode_confirm =
            (ftdf_tsch_mode_confirm_t*) FTDF_GET_MSG_BUFFER(sizeof(ftdf_tsch_mode_confirm_t));

        tsch_mode_confirm->msg_id = FTDF_TSCH_MODE_CONFIRM;
        tsch_mode_confirm->tschMode = tsch_mode_request->tschMode;
        tsch_mode_confirm->status = FTDF_SUCCESS;

        FTDF_REL_MSG_BUFFER((ftdf_msg_buffer_t*) tsch_mode_request);

        FTDF_RCV_MSG((ftdf_msg_buffer_t*) tsch_mode_confirm);
}

void ftdf_process_keep_alive_request(ftdf_keep_alive_request_t *req)
{
        ftdf_keep_alive_confirm_t* keep_alive_confirm =
            (ftdf_keep_alive_confirm_t*) FTDF_GET_MSG_BUFFER(sizeof(ftdf_keep_alive_confirm_t));

        ftdf_status_t status = FTDF_SUCCESS;

        if (req->dst_address < 0xfffe) {
                if (req->keep_alive_period == 0) {
                        uint8_t n;

                        for (n = 0; n < FTDF_NR_OF_NEIGHBORS; n++) {
                                if (ftdf_neighbor_table[n].dst_addr == req->dst_address) {
                                        ftdf_remove_tx_pending_timer((ftdf_msg_buffer_t*)&ftdf_neighbor_table[n].msg);
                                        ftdf_neighbor_table[n].dst_addr = 0xffff;
                                        break;
                                }
                        }
                } else {
                        uint8_t n;

                        for (n = 0; n < FTDF_NR_OF_NEIGHBORS; n++) {
                                if (ftdf_neighbor_table[n].dst_addr == req->dst_address) {
                                        /* Remove current timer from list and add new one */
                                        ftdf_remove_tx_pending_timer((ftdf_msg_buffer_t*)&ftdf_neighbor_table[n].msg);

                                        ftdf_neighbor_table[n].period = req->keep_alive_period;

                                        ftdf_time_t ts_timeslot_length = (ftdf_time_t) ftdf_pib.timeslot_template.ts_timeslot_length / 16;
                                        ftdf_time_t delta = ts_timeslot_length * req->keep_alive_period;

                                        ftdf_add_tx_pending_timer((ftdf_msg_buffer_t*)&ftdf_neighbor_table[n].msg,
                                                                  n,
                                                                  delta,
                                                                  ftdf_process_keep_alive_timer_exp);

                                        break;
                                }
                        }

                        if (n == FTDF_NR_OF_NEIGHBORS) {
                                for (n = 0; n < FTDF_NR_OF_NEIGHBORS; n++) {
                                        if (ftdf_neighbor_table[n].dst_addr == 0xffff) {
                                                break;
                                        }
                                }

                                if (n == FTDF_NR_OF_NEIGHBORS) {
                                        /* No new entry can be made */
                                        status = FTDF_INVALID_PARAMETER;
                                } else {
                                        ftdf_link_entry_t *link = ftdf_link_table;

                                        for (n = 0; n < ftdf_pib.link_table.nr_of_links; n++) {
                                                if ((link->link_options & FTDF_LINK_OPTION_TRANSMIT) &&
                                                        (link->node_address == req->dst_address)) {

                                                        break;
                                                }

                                                link++;
                                        }

                                        if (n == ftdf_pib.link_table.nr_of_links) {
                                                /* No transmit link with given address found */
                                                status = FTDF_INVALID_PARAMETER;
                                        } else {
                                                ftdf_neighbor_table[n].dst_addr = req->dst_address;
                                                ftdf_neighbor_table[n].period = req->keep_alive_period;

                                                ftdf_time_t ts_timeslot_length =
                                                    (ftdf_time_t) ftdf_pib.timeslot_template.ts_timeslot_length / 16;

                                                ftdf_time_t delta = ts_timeslot_length * req->keep_alive_period;

                                                ftdf_add_tx_pending_timer((ftdf_msg_buffer_t*)&ftdf_neighbor_table[n].msg,
                                                                          n,
                                                                          delta,
                                                                          ftdf_process_keep_alive_timer_exp);
                                        }
                                }
                        }
                }
        } else {
                status = FTDF_INVALID_PARAMETER;
        }

        keep_alive_confirm->msg_id  = FTDF_KEEP_ALIVE_CONFIRM;
        keep_alive_confirm->status = status;

        FTDF_REL_MSG_BUFFER((ftdf_msg_buffer_t*) req);
        FTDF_RCV_MSG((ftdf_msg_buffer_t*) keep_alive_confirm);
}

ftdf_short_address_t ftdf_get_request_address(ftdf_msg_buffer_t *request)
{
        switch (request->msg_id) {
        case FTDF_DATA_REQUEST:
                if (((ftdf_data_request_t*) request)->dst_addr_mode == FTDF_SHORT_ADDRESS) {
                        return ((ftdf_data_request_t*) request)->dst_addr.short_address;
                } else {
                        return 0xffff;
                }
                break;
        case FTDF_ASSOCIATE_REQUEST:
                if (((ftdf_associate_request_t*) request)->coord_addr_mode == FTDF_SHORT_ADDRESS) {
                        return ((ftdf_associate_request_t*) request)->coord_addr.short_address;
                } else {
                        return 0xffff;
                }
                break;
        case FTDF_ASSOCIATE_RESPONSE:
                return 0xffff;
                break;
        case FTDF_DISASSOCIATE_REQUEST:
                if (((ftdf_disassociate_request_t*) request)->device_addr_mode == FTDF_SHORT_ADDRESS) {
                        return ((ftdf_disassociate_request_t*) request)->device_address.short_address;
                } else {
                        return 0xffff;
                }
                break;
        case FTDF_BEACON_REQUEST:
                if (((ftdf_beacon_request_t*) request)->dst_addr_mode == FTDF_SHORT_ADDRESS) {
                        return ((ftdf_beacon_request_t*) request)->dst_addr.short_address;
                } else {
                        return 0xffff;
                }
                break;
        case FTDF_REMOTE_REQUEST:
                if (((ftdf_remote_request_t*) request)->remote_id == FTDF_REMOTE_KEEP_ALIVE) {
                        return ((ftdf_remote_request_t*) request)->dst_addr;
                } else {
                        return 0xfffe;
                }
                break;
        default:
                return 0xfffe;
        }
}

ftdf_msg_buffer_t *ftdf_tsch_get_pending(ftdf_msg_buffer_t *request)
{
        /* Check if another request for the same destination is pending */
        ftdf_short_address_t dst_addr = ftdf_get_request_address(request);
        int n;

        for (n = 0; n < FTDF_NR_OF_REQ_BUFFERS; n++) {
                if (ftdf_tx_pending_list[n].addr.short_address == dst_addr) {
                        break;
                }
        }

        if (n < FTDF_NR_OF_REQ_BUFFERS) {
                request = ftdf_dequeue_req_tail(&ftdf_tx_pending_list[n].queue);

                if (request == NULL) {
                        /* Mark this pending queue empty */
                        ftdf_tx_pending_list[n].addr.short_address = 0xfffe;
                }
        } else {
                request = NULL;
        }

        return request;
}

/* This functios does 2 things:
 *
 *  - Searches for next link in all slot frames for which this device has to do a TX or RX
 *  - Searches for TX link in all slot frames in which the given request can be transmitted
 */
ftdf_status_t ftdf_schedule_tsch(ftdf_msg_buffer_t *request)
{
        ftdf_time_t ts_timeslot_length =
            (ftdf_time_t) ftdf_pib.timeslot_template.ts_timeslot_length / 16;

        ftdf_short_address_t  dst_addr          = 0xfffe;
        ftdf_tsch_retry_t    *tsch_retry;
        ftdf_num_of_backoffs_t nr_of_backoffs;

        if (request) {
                dst_addr = ftdf_get_request_address(request);

                ftdf_size_t nr_of_links = ftdf_pib.link_table.nr_of_links;
                ftdf_link_entry_t *link = ftdf_link_table;
                ftdf_size_t n;

                for (n = 0; n < ftdf_pib.link_table.nr_of_links; n++) {
                        if ((link->link_options & FTDF_LINK_OPTION_TRANSMIT) &&
                                (link->node_address == dst_addr) &&
                                ((request->msg_id != FTDF_BEACON_REQUEST) ||
                                 ((request->msg_id == FTDF_BEACON_REQUEST) &&
                                  (link->link_type == FTDF_ADVERTISING_LINK)))) {
                                break;
                        }

                        link++;
                }

                if (n == nr_of_links) {
                        /* Cannot find transmit link to send this request. */
                        return FTDF_INVALID_PARAMETER;
                }

                tsch_retry = ftdf_get_tsch_retry(dst_addr);

                if (tsch_retry->nr_of_retries == 0) {
                        nr_of_backoffs = 0;
                        tsch_retry->BE = ftdf_pib.min_be;
                } else if (tsch_retry->nr_of_retries < ftdf_pib.max_frame_retries) {
                        nr_of_backoffs = ftdf_get_num_of_backoffs(tsch_retry->BE);

                        tsch_retry->BE++;

                        if (tsch_retry->BE > ftdf_pib.max_be) {
                                tsch_retry->BE = ftdf_pib.max_be;
                        }
                } else {
                            tsch_retry->nr_of_retries = 0;
                            return FTDF_NO_ACK;
                }
        }

        ftdf_critical_var();
        ftdf_enter_critical();

        static ftdf_asn_t next_asn;      /* ASN of the next following timeslot */
        static ftdf_time64_t next_time;  /* Slot start time of next following timeslot */

        /* ftdf_tsch_slot_asn  = ASN of the last calculated active slot. Can be in the past or future. */
        /* ftdf_tsch_slot_time = Start time of the last calculated active slot. It is the start time of slot ftdf_tsch_slot_asn */

        ftdf_time64_t cur_time = ftdf_get_cur_time64();
        int delta = cur_time - ftdf_tsch_slot_time;
        int delta_in_slots = (delta + (int) ts_timeslot_length) / (int) ts_timeslot_length;

        next_time = ftdf_tsch_slot_time + delta_in_slots * (int) ts_timeslot_length;
        next_asn = (int) ftdf_tsch_slot_asn + delta_in_slots;

        if ((next_time - cur_time) < (FTDF_TSCH_MAX_PROCESS_REQUEST_TIME + FTDF_TSCH_MAX_SCHEDULE_TIME)) {
                next_time += ts_timeslot_length;
                next_asn++;
        }

        ftdf_slotframe_size_t max_nr_of_timeslots = 0;
        int slotframe;
        ftdf_link_entry_t *link;

        static ftdf_link_entry_t *cur_links[FTDF_MAX_SLOTFRAMES];
        static ftdf_timeslot_t cur_timeslots[FTDF_MAX_SLOTFRAMES];

        /* Get current timeslot for each slotframe */
        for (slotframe = 0; slotframe < ftdf_pib.slotframe_table.nr_of_slotframes; slotframe++) {
                ftdf_slotframe_size_t slotframe_size = ftdf_slotframe_table[slotframe].slotframe_size;

                if (slotframe_size > max_nr_of_timeslots) {
                        max_nr_of_timeslots = slotframe_size;
                }

                slotframe_size = slotframe_size ? slotframe_size : slotframe_size + 1;

                ftdf_timeslot_t timeslot = next_asn % slotframe_size;

                link = ftdf_start_links[slotframe];
                cur_links[slotframe] = link;

                while (link < ftdf_end_links[slotframe]) {
                        if (link->timeslot >= timeslot) {
                                cur_links[slotframe] = link;
                            break;
                        }

                        link++;
                }

                cur_timeslots[slotframe] = timeslot;
        }

        /* Find the next active link, this is first link for which something needs to be done. */
        uint32_t next_active_offset = 0xffffffff;
        uint32_t timeslot_offset = 0;
        ftdf_link_entry_t *next_active_link = NULL;
        ftdf_msg_buffer_t *queue_request = NULL;

        while (((next_active_link == NULL) && (timeslot_offset < max_nr_of_timeslots)) || request) {
                uint16_t min_slot_offset = 0xffff;

                /* Skip all timeslots without a link */
                for (slotframe = 0; slotframe < ftdf_pib.slotframe_table.nr_of_slotframes; slotframe++) {

                        ftdf_slotframe_size_t slotframe_size = ftdf_slotframe_table[slotframe].slotframe_size;
                        int next_slot_offset = cur_links[slotframe]->timeslot - cur_timeslots[slotframe];

                        if (next_slot_offset < 0) {
                                next_slot_offset += slotframe_size;
                        }

                        if ((uint16_t) next_slot_offset < min_slot_offset) {
                                min_slot_offset = (uint16_t) next_slot_offset;
                        }
                }

                timeslot_offset += min_slot_offset;

                for (slotframe = 0; slotframe < ftdf_pib.slotframe_table.nr_of_slotframes; slotframe++) {
                        link = cur_links[slotframe];

                        ftdf_slotframe_size_t slotframe_size = ftdf_slotframe_table[slotframe].slotframe_size;
                        ftdf_timeslot_t timeslot = cur_timeslots[slotframe] + min_slot_offset;

                        /* A while loop is used rather than modulo function because it is more efficient when
                         * timeslot is relative small. timeslot is small because we only a few timeslots ahead */
                        while (timeslot >= slotframe_size) {
                                timeslot -= slotframe_size;
                        }

                        if (link->timeslot == timeslot) {
                                if (request) {
                                        /* Check if this link can be used to transmit the request */
                                        if ((link->link_options & FTDF_LINK_OPTION_TRANSMIT) &&
                                                (link->node_address == dst_addr) &&
                                                (request->msg_id != FTDF_BEACON_REQUEST ||
                                                 ((request->msg_id == FTDF_BEACON_REQUEST) &&
                                                  (link->link_type == FTDF_ADVERTISING_LINK)))) {

                                                if (link->request == NULL) {
                                                        if (link->link_options & FTDF_LINK_OPTION_SHARED) {
                                                                if (nr_of_backoffs == 0) {

                                                                        link->request = request;
                                                                        link->requestASN = next_asn + timeslot_offset;
                                                                        request = NULL;
                                                                }

                                                                nr_of_backoffs--;
                                                        } else {
                                                                link->request = request;
                                                                link->requestASN = next_asn + timeslot_offset;
                                                                request = NULL;
                                                        }
                                                } else {
                                                        /* Found suitable transmit link but it is already
                                                         * occupied, so queue it */
                                                        queue_request = request;
                                                        request = NULL;
                                                }
                                        }
                                }

                                if ((timeslot_offset <= next_active_offset) && (timeslot_offset < slotframe_size)) {
                                        if (link->request && ((next_active_link == NULL) || (next_active_link->request == NULL)) &&
                                                (link->requestASN <= (next_asn + timeslot_offset))) {
                                                /* A TX request has been scheduled for this link */
                                                next_active_link   = link;
                                                next_active_offset = timeslot_offset;
                                        }

                                        if ((link->link_options & FTDF_LINK_OPTION_RECEIVE) &&
                                                (next_active_link == NULL)) {
                                                next_active_link   = link;
                                                next_active_offset = timeslot_offset;
                                        }
                                }

                                link++;

                                if (link >= ftdf_end_links[slotframe]) {
                                        link = ftdf_start_links[slotframe];
                                }

                                cur_links[slotframe] = link;
                        }

                        timeslot++;

                        if (timeslot >= slotframe_size) {
                                timeslot = 0;
                        }

                        cur_timeslots[slotframe] = timeslot;
                }

                timeslot_offset++;
        }

        /* Note: nextActiveLink can only be NULL when no receive links are defined. */
        if (next_active_link) {
                ftdf_tsch_slot_time = next_time + (next_active_offset * ts_timeslot_length);
                ftdf_tsch_slot_asn = next_asn + next_active_offset;
                ftdf_tsch_slot_link = next_active_link;

                ftdf_channel_number_t channel =
                    ftdf_pib.hopping_sequence_list[(ftdf_tsch_slot_asn + ftdf_tsch_slot_link->channel_offset)
                                                  % ftdf_pib.hopping_sequence_length];

                ftdf_pib.current_channel = channel;
                ftdf_set_current_channel();

                if (next_active_link->request) {
                        REG_SET_FIELD(FTDF,  FTDF_SYMBOLTIME2THR_REG, SYMBOLTIME2THR, FTDF->FTDF_SYMBOLTIME2THR_REG,
                                (((ftdf_time_t) ftdf_tsch_slot_time) + ftdf_tx_offset - FTDF_TSCH_MAX_PROCESS_REQUEST_TIME));
                        REG_SET_FIELD(FTDF, FTDF_LMAC_CONTROL_8_REG, MACCSLSTARTSAMPLETIME, FTDF->FTDF_LMAC_CONTROL_8_REG,
                                ((ftdf_time_t) ftdf_tsch_slot_time + ts_timeslot_length));
                } else {
                        REG_SET_FIELD(FTDF, FTDF_LMAC_CONTROL_8_REG, MACCSLSTARTSAMPLETIME, FTDF->FTDF_LMAC_CONTROL_8_REG,
                                (ftdf_time_t) ftdf_tsch_slot_time);
                }
        }

        ftdf_exit_critical();

        if (queue_request) {
                /* Did not find a suitable link in the current slotframes, so queue
                 * the request in a TX pending queue */

                /* Search for existing queue */
                int n;

                for (n = 0; n < FTDF_NR_OF_REQ_BUFFERS; n++) {
                        if (ftdf_tx_pending_list[n].addr.short_address == dst_addr) {
                                break;
                        }
                }

                if (n == FTDF_NR_OF_REQ_BUFFERS) {
                        /* Search for empty queue */
                        for (n = 0; n < FTDF_NR_OF_REQ_BUFFERS; n++) {
                                if (ftdf_tx_pending_list[n].addr.short_address == 0xfffe) {
                                        break;
                                }
                        }
                }

                if ((n == FTDF_NR_OF_REQ_BUFFERS) ||
                        (ftdf_queue_req_head(queue_request, &ftdf_tx_pending_list[n].queue) == FTDF_TRANSACTION_OVERFLOW)) {
                        return FTDF_TRANSACTION_OVERFLOW;
                } else {
                        ftdf_add_tx_pending_timer(queue_request,
                                                  n,
                                                  (ftdf_pib.transaction_persistence_time * FTDF_BASE_SUPERFRAME_DURATION),
                                                  ftdf_send_transaction_expired);
                }
        }

        return FTDF_SUCCESS;
}

void ftdf_tsch_process_request(void)
{
        ftdf_msg_buffer_t *request = ftdf_tsch_slot_link->request;

        /* Save any request that requires a remote response */
        ftdf_msg_buffer_t *req_current = ftdf_req_current;

        /* Save destination address */
        ftdf_short_address_t dst_addr = ftdf_get_request_address(request);

        ftdf_process_request(request);

        if (ftdf_req_current == NULL) {
                /* Processing of the request failed, so schedule a pending one */
                request = ftdf_tsch_get_pending(ftdf_tsch_slot_link->request);

                ftdf_schedule_tsch(request);

                /* Restore any request that requires a remote response */
                ftdf_req_current = req_current;

                return;
        }

        ftdf_critical_var();
        ftdf_enter_critical();

        ftdf_time64_t cur_time = ftdf_get_cur_time64();
        ftdf_boolean_t too_late = FTDF_FALSE;

        if (cur_time < (ftdf_tsch_slot_time + ftdf_tx_offset - 2)) {
                REG_SETF(FTDF, FTDF_LMAC_CONTROL_8_REG, MACCSLSTARTSAMPLETIME,
                        (((ftdf_time_t) ftdf_tsch_slot_time) + ftdf_tx_offset));

                REG_SETF(FTDF, FTDF_TX_SET_OS_REG, TX_FLAG_SET, (1 << FTDF_TX_DATA_BUFFER));
        } else {
                too_late = FTDF_TRUE;
        }

        ftdf_exit_critical();

        if (too_late) {
                /* Too late to do something for the next slot, so reschedule */
                ftdf_tsch_slot_link->request = NULL;

                ftdf_schedule_tsch(request);

                /* Restore any request that requires a remote response */
                ftdf_req_current = req_current;
        } else if (dst_addr < 0xfffe) {
                ftdf_reset_keep_alive_timer(dst_addr);
        }
}

ftdf_size_t ftdf_get_tsch_sync_sub_ie(void)
{
        return 8;
}

ftdf_octet_t *ftdf_add_tsch_sync_sub_ie(ftdf_octet_t *tx_ptr)
{
        *tx_ptr++ = 0x06;
        *tx_ptr++ = 0x1a;

        int n;
        ftdf_octet_t *p = (ftdf_octet_t*) &ftdf_tsch_slot_asn;

        for (n = 0; n < 5; n++) {
                *tx_ptr++ = *p++;
        }

        *tx_ptr++ = ftdf_pib.join_priority;

        return tx_ptr;
}

ftdf_octet_t *ftdf_add_corr_time_ie(ftdf_octet_t *tx_ptr, ftdf_time_t rx_timestamp)
{
        static ftdf_octet_t time_correction[2];
        static ftdf_ie_descriptor_t tsch_ie = { 0x1e, 2, { time_correction } };
        static ftdf_ie_list_t tsch_ie_list = { 1, &tsch_ie };

        /* rx_timestamp is 10 symbols (8 preamble + 2 start) after the start of the frame */
        ftdf_time_t delta = ((ftdf_time_t) ftdf_tsch_slot_time) - rx_timestamp + 10;
        int delta_in_us = ((int) delta * 16) + ((int) ftdf_pib.timeslot_template.ts_rx_wait / 2);

        *(uint16_t*) time_correction = (uint16_t) (delta_in_us & 0xfff);

        tx_ptr = ftdf_add_ies(tx_ptr, &tsch_ie_list, &ftdf_pib.e_ack_ie_list, FTDF_FALSE);

        return tx_ptr;
}

void ftdf_correct_slot_time(ftdf_time_t rx_timestamp)
{
        if ((ftdf_tsch_slot_link->link_options & FTDF_LINK_OPTION_TIME_KEEPING) == 0) {
                return;
        }

        int16_t corr_time_in_symbols = (int16_t) (((ftdf_time_t) ftdf_tsch_slot_time) - rx_timestamp + 10 +
                                            (ftdf_pib.timeslot_template.ts_rx_wait + 8) / 32);
        int16_t corr_time  = corr_time_in_symbols * 16;

        if ((corr_time >= (int16_t) ftdf_pib.ts_sync_correct_threshold) ||
                (corr_time <= (int16_t) ftdf_pib.ts_sync_correct_threshold * -1)) {

                ftdf_tsch_slot_time = ftdf_tsch_slot_time - corr_time_in_symbols;
        }
}

void ftdf_correct_slot_time_from_ack(ftdf_ie_list_t *header_ie_list)
{
        if (((ftdf_tsch_slot_link->link_options & FTDF_LINK_OPTION_TIME_KEEPING) == 0) ||
                header_ie_list == NULL) {

                return;
        }

        int n;

        for (n = 0; n < header_ie_list->nr_of_ie; n++) {
                if (header_ie_list->ie[n].id == 0x1e) {
                        uint8_t* p  = header_ie_list->ie[n].content.raw;
                        uint16_t corr_time = (*p + (*(p + 1) << 8)) & 0xfff;

                        if (corr_time & 0x800) {
                                corr_time |= 0xf000;
                        }

                        int16_t corr_time_signed = (int16_t) corr_time;

                        if ((corr_time_signed >= (int16_t) ftdf_pib.ts_sync_correct_threshold) ||
                                (corr_time_signed <= (int16_t) ftdf_pib.ts_sync_correct_threshold * - 1)) {

                                int16_t corr_time_in_symbols = corr_time_signed / 16;
                                ftdf_tsch_slot_time = ftdf_tsch_slot_time + corr_time_in_symbols;
                        }
                }
        }
}

ftdf_tsch_retry_t *ftdf_get_tsch_retry(ftdf_short_address_t node_addr)
{
        int n;

        /* Search for existing entry */
        for (n = 0; n < MAX_NR_OF_NODE_ADDRESSES; n++) {
                if (ftdf_tsch_retries[n].nodeAddr == node_addr) {
                        return &ftdf_tsch_retries[n];
                }
        }

        n  = ftdf_lru_tsch_retry;
        ftdf_tsch_retries[n].nodeAddr = node_addr;
        ftdf_tsch_retries[n].nr_of_retries = 0;
        ftdf_tsch_retries[n].nr_of_cca_retries = 0;

        ftdf_lru_tsch_retry++;

        if (ftdf_lru_tsch_retry == MAX_NR_OF_NODE_ADDRESSES) {
                ftdf_lru_tsch_retry = 0;
        }

        /* Return least recently used entry */
        return &ftdf_tsch_retries[n];
}

void ftdf_init_tsch_retries(void)
{
        ftdf_lru_tsch_retry = 0;

        memset(ftdf_tsch_retries, 0, sizeof(ftdf_tsch_retries));
}

static uint32_t seed;

ftdf_num_of_backoffs_t ftdf_get_num_of_backoffs(ftdf_be_exponent_t be)
{
        /* A linear congruential generator implementation */
        seed = (seed * 1103515245U + 12345U) & 0x7fffffffU;

        return seed % ((1 << be) - 1);
}

void ftdf_init_backoff(void)
{
        seed = (uint32_t) ftdf_pib.ext_address & 0x7fffffffU;
}

ftdf_sn_t ftdf_process_tsch_sn(ftdf_msg_buffer_t *msg, ftdf_sn_t sn, uint8_t *priv)
{
        ftdf_tsch_retry_t *tsch_retry = ftdf_get_tsch_retry(ftdf_get_request_address(msg));

        if (tsch_retry->nr_of_retries == 0) {
                /* first try, store the SN in private data */
                *priv = sn;
                return sn;
        } else {
                /* retrieve SN from private data */
                return *priv;
        }
}
#endif /* FTDF_NO_TSCH */
#endif /* !FTDF_LITE */
#endif /* #ifdef CONFIG_USE_FTDF */
