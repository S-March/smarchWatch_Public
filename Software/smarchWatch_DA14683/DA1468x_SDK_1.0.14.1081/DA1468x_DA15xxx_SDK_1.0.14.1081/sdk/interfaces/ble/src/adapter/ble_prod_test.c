/**
 ****************************************************************************************
 *
 * @file ble_prod_test.c
 *
 * @brief BLE Production Test code
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifdef BLE_PROD_TEST

#include "hw_rf.h"

#include "co_bt.h"
#include "co_hci.h"
#include "ad_ble.h"
#include "ble_mgr.h"
#include "ble_mgr_common.h"

#include "lld.h"
#include "lld_data.h"
#include "reg_ble_em_rx_desc.h"

#include "ke_event.h"

#include "ble_prod_test.h"
#include "rf_tools_common.h"

enum
{
    ///NO_TEST_RUNNING
    STATE_PROD_TEST_IDLE                                        = 0x00,
    ///START_TX_TEST
    STATE_PROD_TEST_TX,                                                 //1
    ///START_RX_TEST
    STATE_PROD_TEST_RX,                                                 //2
    STATE_PROD_TEST_TX_INTV,
    STATE_PROD_UNMODULATED_TRX

};

enum
{
        TXINTV_ST_STOPPED = 0,
        TXINTV_ST_TX,
        TXINTV_ST_WAIT_TIMER,
        TXINTV_ST_WAIT_START_TX
};

volatile INITIALISED_PRIVILEGED_DATA uint32   prod_test_state = STATE_PROD_TEST_IDLE;
volatile PRIVILEGED_DATA static uint16_t prod_test_rx_packet_nr;
volatile PRIVILEGED_DATA static uint16_t prod_test_rx_packet_nr_syncerr;
volatile PRIVILEGED_DATA static uint16_t prod_test_rx_packet_nr_crcerr;
volatile PRIVILEGED_DATA static uint16_t prod_test_rx_test_rssi;
volatile static uint8_t rx_stats_params[8];

volatile uint32_t prod_test_tx_interval_us;
volatile uint32_t prod_test_tx_packet_nr;
volatile uint32_t prod_test_tx_packet_count;
static uint32_t prod_test_tx_prev_packet_count;
static bool prod_test_tx_auto_stop;

static struct hci_le_tx_test_cmd prod_test_tx_con_test;
static OS_MUTEX tx_delay_done_sema;

volatile PRIVILEGED_DATA static int tx_intv_state;

extern struct llm_le_env_tag llm_le_env;

struct ea_elt_tag *lld_evt_deferred_elt_pop(uint8_t *type, uint8_t *rx_desc_cnt);

struct co_buf_rx_desc *rxdesc;
struct lld_data_ind *msg_test;

extern struct co_buf_env_tag co_buf_env;


void set_state_stop(void)
{
        if ( (prod_test_state==STATE_PROD_TEST_TX) || (prod_test_state==STATE_PROD_TEST_RX) ||
                (prod_test_state == STATE_PROD_TEST_TX_INTV))
        {
                prod_test_state = STATE_PROD_TEST_IDLE;
        }
}



void set_state_start_tx(void)
{
        if (prod_test_state==STATE_PROD_TEST_IDLE)
        {
                prod_test_state=STATE_PROD_TEST_TX;
        }


}

void set_state_start_rx(void)
{
        if (prod_test_state==STATE_PROD_TEST_IDLE)
        {

                prod_test_state=STATE_PROD_TEST_RX;
        }
}

void set_state_start_tx_intv(void)
{
        if (prod_test_state==STATE_PROD_TEST_IDLE)
        {
                prod_test_state=STATE_PROD_TEST_TX_INTV;
        }


}

static void send_ble_evt(uint16_t cmd_opcode, uint8_t event_code, uint8_t param_length, uint8_t *params_buf)
{

        ble_mgr_common_stack_msg_t* msgBuf = NULL;
        uint8_t *bp;

        // Allocate the space needed for the message
        msgBuf = OS_MALLOC(sizeof(ble_mgr_common_stack_msg_t) + param_length - sizeof(uint8_t));

        msgBuf->hdr.op_code = BLE_MGR_COMMON_STACK_MSG;               // fill message OP code
        msgBuf->msg_type = HCI_EVT_MSG;                               // fill stack message type
        msgBuf->hdr.msg_len = HCI_EVT_HEADER_LENGTH + param_length;   // fill stack message length

        bp = (uint8_t *)&msgBuf->msg;

        *bp++ = event_code;
        *bp++ = param_length;
        *bp++ = 0x01; /* Num_HCI_Command_Packets */
        *bp++ = cmd_opcode & 0xff;
        *bp++ = (cmd_opcode >> 8) & 0xff;

        param_length -= 3;

        if (param_length > 0) {
                memcpy(bp, params_buf, param_length);
        }

        // Post item to queue
        if (ad_ble_event_queue_send(&msgBuf, 0) == pdFALSE) {
                OS_FREE(msgBuf);
        }
}

void lld_data_rx_check_custom(struct lld_evt_tag *evt,
                              struct lld_data_ind *msg,
                              uint8_t rx_cnt)
{
    uint8_t hdl = co_buf_env.rx_current; //co_buf_rx_current_get();

    // Initialize the message
    msg->rx_cnt = rx_cnt;
    msg->rx_hdl = hdl;

    //Get the event counter
    msg->evt_cnt = evt->counter;

        msg_test = msg;

    // If required, copy the received buffers from exchange memory to system RAM
    while (rx_cnt--)
    {
        #if (BLE_PERIPHERAL)
        //struct co_buf_rx_desc *rxdesc = co_buf_rx_get(hdl);
        //       rxdesc = co_buf_rx_get(hdl);
                rxdesc = &co_buf_env.rx_desc[hdl];

        // If we are waiting for the acknowledgment, and it is received, enable the slave
        // latency
        if (LLD_EVT_FLAG_GET(evt, WAITING_ACK) && !(rxdesc->rxstatus & BLE_NESN_ERR_BIT))
        {
            // We received the acknowledgment
            LLD_EVT_FLAG_RESET(evt, WAITING_ACK);
        }

        #endif //(BLE_PERIPHERAL)

                if (prod_test_state==STATE_PROD_TEST_RX)
                {
                        prod_test_rx_packet_nr++;
                }

                if ( (rxdesc->rxstatus & 1)==1 ) //SYNCERR
                {
                        if (prod_test_state==STATE_PROD_TEST_RX)
                        {
                                prod_test_rx_packet_nr_syncerr++;
                        }
                }

                if (prod_test_state==STATE_PROD_TEST_RX) {
                        if ( (rxdesc->rxstatus & 8)==8 ) //CRCERR
                        {
                                prod_test_rx_packet_nr_crcerr++;
                        }
                        else
                        {
                                prod_test_rx_test_rssi = (uint16)(((uint32)(prod_test_rx_test_rssi +
                                        (rxdesc->rxchass & 0xFF) ) ) >>1);
                        }
                }


        // Go to the next descriptor
        hdl = co_buf_rx_next(hdl);
    };



    // Move the current RX buffer
    co_buf_rx_current_set(hdl);
}

/**
 ****************************************************************************************
 * @brief Check the status of the test mode
 *
 * This function sends an event to the host when the TX or RX test mode is finished
 *
 * Basic concept copied from stack's llm_util_chk_tst_mode
 *
 ****************************************************************************************
 */
static inline void chk_tst_mode(void)
{
    /* Check whether there is a CUSTOM Test running AND
     * it's been requested to stop */
    if (prod_test_state == STATE_PROD_TEST_IDLE ||
            llm_le_env.test_mode.end_of_tst == false ||
            llm_le_env.test_mode.directtesttype == TEST_END)
            return;

    switch (prod_test_state) {
    case STATE_PROD_TEST_RX: /* RX stats (0xFC82) */
            configASSERT(llm_le_env.test_mode.directtesttype == TEST_RX);

            send_ble_evt(BPT_OPCODE_STOP_PKT_RX_STATS, BPT_EVTCODE_STOP_PKT_RX_STATS,
                    BPT_PLEN_STOP_PKT_RX_STATS, (uint8_t *)rx_stats_params);
            break;
    case STATE_PROD_TEST_TX: /* START_CONT_TX (0xFC84) */
            configASSERT(llm_le_env.test_mode.directtesttype == TEST_TX);

            send_ble_evt(BPT_OPCODE_STOP_CONT_TX, BPT_EVTCODE_STOP_CONT_TX,
                    BPT_PLEN_STOP_CONT_TX, NULL);
            break;
    case STATE_PROD_TEST_TX_INTV:
            if (tx_intv_state != TXINTV_ST_WAIT_TIMER) {
                    if (prod_test_tx_auto_stop) {
                            uint8_t status = 0;
                            send_ble_evt(BPT_OPCODE_PKT_TX_INTV, BPT_EVTCODE_PKT_TX_INTV_COMPLETE,
                                    BPT_PLEN_PKT_TX_INTV_COMPLETE, &status);
                    } else {
                            send_ble_evt(BPT_OPCODE_STOP_CONT_TX, BPT_EVTCODE_STOP_CONT_TX,
                                    BPT_PLEN_STOP_CONT_TX, NULL);
                    }
            }
            break;
    default:
            configASSERT(0);
    }

    set_state_stop();

    // enable the whitening
    ble_whit_dsb_setf(0);

    // set the env variable,
    llm_le_env.test_mode.end_of_tst = false;
    llm_le_env.test_mode.directtesttype = TEST_END;

    /* What about this?? */
    ke_msg_send_basic(LLM_STOP_IND, TASK_LLM, TASK_LLM);

}

void lld_evt_deffered_elt_handler_custom(void)
{
    // Indicate on which interrupt treatment has been postponed (RX ISR or END OF EVENT ISR)
    uint8_t type;
    // Number of RX descriptors consumed
    uint8_t rx_desc_cnt;
    // Deferred element
    struct ea_elt_tag *elt;

    // Clear kernel event
    ke_event_clear(KE_EVENT_BLE_EVT_DEFER);

    // First first deferred element
    elt = lld_evt_deferred_elt_pop(&type, &rx_desc_cnt);

    while (elt)
    {
        // Get associated BLE event
        struct lld_evt_tag *evt = LLD_EVT_ENV_ADDR_GET(elt);
        ke_task_id_t dest_id = (evt->conhdl == LLD_ADV_HDL) ? TASK_LLM : KE_BUILD_ID(TASK_LLC, evt->conhdl);

        if ((type == RWBLE_DEFER_TYPE_END) && LLD_EVT_FLAG_GET(evt, DELETE))
        {
            // Confirm the stop to the host
            ke_msg_send_basic(LLD_STOP_IND, dest_id, TASK_LLD);
            // Flush rx buffer
            lld_data_rx_flush(evt, rx_desc_cnt);
            // Delete the element
            lld_evt_elt_delete(elt,true);
            #if (DEEP_SLEEP)
            // Clear prevent bit
            rwip_prevent_sleep_clear(RW_DELETE_ELT_ONGOING);
            #endif // #if (DEEP_SLEEP)
        }
        else
        {

            // Allocate a LLD_DATA_IND message
            struct lld_data_ind *msg = KE_MSG_ALLOC(LLD_DATA_IND,
                                                    dest_id, TASK_LLD,
                                                    lld_data_ind);


            if (type == RWBLE_DEFER_TYPE_TEST_END)
            {
                // Flush rx buffer
                lld_data_rx_flush(evt, rx_desc_cnt);
                msg->rx_cnt = 0;
                // Delete the element
                lld_evt_elt_delete(elt,true);
                #if (DEEP_SLEEP)
                // Clear prevent bit
                rwip_prevent_sleep_clear(RW_DELETE_ELT_ONGOING);
                #endif // #if (DEEP_SLEEP)

                chk_tst_mode();
            }
            else
            {
            // Check received data
            lld_data_rx_check_custom(evt, msg, rx_desc_cnt);

            #if (BLE_CENTRAL || BLE_PERIPHERAL)
            // Confirm transmitted data or cntl
            lld_data_tx_check(evt, msg);
            #endif //(BLE_CENTRAL || BLE_PERIPHERAL)
            }

            // Send the message
            ke_msg_send(msg);

        }

        // Get next element
        elt = lld_evt_deferred_elt_pop(&type, &rx_desc_cnt);
    }


}

static void start_pkt_rx_stats(const ble_mgr_common_stack_msg_t *stack_msg)
{
        prod_test_rx_packet_nr = 0;
        prod_test_rx_packet_nr_crcerr = 0;
        prod_test_rx_packet_nr_syncerr = 0;
        prod_test_rx_test_rssi = 0;

        uint8_t st = ke_state_get(TASK_LLM);

        if (prod_test_state == STATE_PROD_TEST_IDLE && st == LLM_IDLE) {
                /* Test not started. Must check this, otherwise
                 * it stops and never starts again
                 */

                llm_test_mode_start_rx((struct hci_le_rx_test_cmd *)&stack_msg->msg.hci.cmd.param);
                set_state_start_rx();
                send_ble_evt(BPT_OPCODE_START_PKT_RX_STATS, BPT_EVTCODE_START_PKT_RX_STATS,
                        BPT_PLEN_START_PKT_RX_STATS, NULL);
        }
}


static void stop_pkt_rx_stats(void)
{
        uint8_t st = ke_state_get(TASK_LLM);

        if (prod_test_state == STATE_PROD_TEST_RX && st == LLM_TEST) {
                rx_stats_params[0] = prod_test_rx_packet_nr & 0xFF;
                rx_stats_params[1] = (prod_test_rx_packet_nr >> 8) & 0xFF;
                rx_stats_params[2] = prod_test_rx_packet_nr_syncerr & 0xFF;
                rx_stats_params[3] = (prod_test_rx_packet_nr_syncerr >> 8) & 0xFF;
                rx_stats_params[4] = prod_test_rx_packet_nr_crcerr & 0xFF;
                rx_stats_params[5] = (prod_test_rx_packet_nr_crcerr >> 8) & 0xFF;
                rx_stats_params[6] = prod_test_rx_test_rssi & 0xFF;
                rx_stats_params[7] = (prod_test_rx_test_rssi >> 8) & 0xFF;

                llm_le_env.test_mode.end_of_tst = true;
                lld_test_stop(llm_le_env.elt);
                // Set the state to stopping
                ke_state_set(TASK_LLM, LLM_STOPPING);
        }

        /* The response HCI message will be sent as soon as the test is really ended */
}

/* Continuous modulated, non-packetized tx */
static void start_cont_tx(const ble_mgr_common_stack_msg_t *stack_msg)
{
        uint8_t st = ke_state_get(TASK_LLM);

        if (prod_test_state == STATE_PROD_TEST_IDLE && st == LLM_IDLE) {

                REG_SETF(BLE, BLE_RFTESTCNTL_REG, INFINITETX, 1);

                REG_SETF(BLE, BLE_RFTESTCNTL_REG,TXLENGTHSRC,0);
                REG_SETF(BLE, BLE_RFTESTCNTL_REG,TXPLDSRC,0);
                REG_SETF(BLE, BLE_RFTESTCNTL_REG,TXLENGTH,0);

                /* Test not started. Must check this, otherwise
                 * it stops and never starts again
                 */
                struct hci_le_tx_test_cmd  tx_con_test;
                tx_con_test.tx_freq = stack_msg->msg.hci.cmd.param[0];
                tx_con_test.test_data_len = 37; //select a valid value.
                tx_con_test.pk_payload_type = stack_msg->msg.hci.cmd.param[1];

                llm_test_mode_start_tx( (struct hci_le_tx_test_cmd const *)&tx_con_test);
                set_state_start_tx();

                send_ble_evt(BPT_OPCODE_START_CONT_TX, BPT_EVTCODE_START_CONT_TX,
                                BPT_PLEN_START_CONT_TX, NULL);

        }

}

static void stop_cont_tx(void)
{
        uint8_t st = ke_state_get(TASK_LLM);

        if (prod_test_state == STATE_PROD_TEST_TX && st == LLM_TEST) {
                llm_le_env.test_mode.end_of_tst = true;
                lld_test_stop(llm_le_env.elt);
                // Set the state to stopping
                ke_state_set(TASK_LLM, LLM_STOPPING);

                BLE->BLE_RFTESTCNTL_REG = 0; //disable continuous mode
        }
}

static inline void stop_tx(void)
{
        uint8_t st = ke_state_get(TASK_LLM);
        /* stop_tx must only be called when an LLM_TEST is running */
        configASSERT(st == LLM_TEST);

        llm_le_env.test_mode.end_of_tst = true;
        lld_test_stop(llm_le_env.elt);
        // Set the state to stopping
        ke_state_set(TASK_LLM, LLM_STOPPING);
}

void stop_pkt_tx_interval(bool auto_stop)
{
        uint8_t st;

        rf_tools_stop_systick();

        tx_intv_state = TXINTV_ST_STOPPED;

        /* Just take the semaphore, so that it can be safely deleted */
        OS_EVENT_WAIT(tx_delay_done_sema, 0);
        vSemaphoreDelete(tx_delay_done_sema);

        prod_test_tx_auto_stop = auto_stop;
        prod_test_tx_packet_count = 0;
        prod_test_tx_prev_packet_count = 0;
        prod_test_tx_packet_nr = 0;

        st = ke_state_get(TASK_LLM);
        switch (st) {
        case LLM_TEST:
                /* Actually stop the transmission */
                stop_tx();
                break;
        case LLM_IDLE:
                /* Nothing to stop. Immediately return the response (since there will be
                 * no end event triggering this when the transmission is to be stopped
                 */
                if (auto_stop) {
                        uint8_t status = 0;
                        send_ble_evt(BPT_OPCODE_PKT_TX_INTV, BPT_EVTCODE_PKT_TX_INTV_COMPLETE,
                                BPT_PLEN_PKT_TX_INTV_COMPLETE, &status);
                } else {
                        send_ble_evt(BPT_OPCODE_STOP_CONT_TX, BPT_EVTCODE_STOP_CONT_TX,
                                BPT_PLEN_STOP_CONT_TX, NULL);
                }
                set_state_stop();
                break;
        default:
                ;
                /* LLM_STOPPING. Do nothing. TX is stopping (stopped automatically to a timer expiring).
                 * The end event will trigger the message response
                 */
        }
}


static void init_tx_falling_edge_interrupt(void)
{
        REG_SETF(BLE, BLE_DIAGCNTL2_REG, DIAG6, 0x25);                  // diag6 configured for mode 0x25
        REG_SET_BIT(BLE, BLE_DIAGCNTL2_REG, DIAG6_EN);                  // enable port
        REG_SETF(BLE, BLE_DIAGCNTL3_REG, DIAG6_BIT, 1);                 // txen on diag6

        uint32_t reg = RFCU->RF_DIAGIRQ01_REG;
        reg |= (1 << REG_POS(RFCU, RF_DIAGIRQ01_REG, DIAGIRQ_EDGE_1));  // Falling edge
        REG_SET_FIELD(RFCU, RF_DIAGIRQ01_REG, DIAGIRQ_BSEL_1, reg, 7);  // Select bit #7 (TX_EN)
        REG_SET_FIELD(RFCU, RF_DIAGIRQ01_REG, DIAGIRQ_WSEL_1, reg, 2);  // Select RADIO_DIAG1
        RFCU->RF_DIAGIRQ01_REG = reg;

        (void) RFCU->RF_DIAGIRQ_STAT_REG;

        REG_SETF(RFCU, RF_DIAGIRQ01_REG, DIAGIRQ_MASK_1, 0x1);          // Enable IRQ generation

        // Enable RF_DIAG_IRQn interrupt - The ISR is implemented with
        // the RF_DIAG_Handler() function
        NVIC_EnableIRQ(RF_DIAG_IRQn);
}

/* Continuous modulated, non-packetized tx */
static void pkt_tx_interval(const ble_mgr_common_stack_msg_t *stack_msg)
{
        uint8_t st = ke_state_get(TASK_LLM);
        if (prod_test_state == STATE_PROD_TEST_IDLE && st == LLM_IDLE) {

                /* Test not started. Must check this, otherwise
                 * it stops and never starts again
                 */
                prod_test_tx_con_test.tx_freq = stack_msg->msg.hci.cmd.param[0];
                prod_test_tx_con_test.test_data_len = stack_msg->msg.hci.cmd.param[1];
                prod_test_tx_con_test.pk_payload_type = stack_msg->msg.hci.cmd.param[2];

                prod_test_tx_packet_nr = stack_msg->msg.hci.cmd.param[3] |
                                (stack_msg->msg.hci.cmd.param[4] << 8);

                prod_test_tx_interval_us = stack_msg->msg.hci.cmd.param[5] |
                        (stack_msg->msg.hci.cmd.param[6] << 8) |
                        (stack_msg->msg.hci.cmd.param[7] << 16) |
                        (stack_msg->msg.hci.cmd.param[8] << 24);

                if (prod_test_tx_interval_us > 0) {
                        uint8_t status = 1;
                        send_ble_evt(BPT_OPCODE_PKT_TX_INTV, BPT_EVTCODE_PKT_TX_INTV_COMPLETE,
                                        BPT_PLEN_PKT_TX_INTV_COMPLETE, &status);
                        return;
                }

                init_tx_falling_edge_interrupt();
                OS_EVENT_CREATE(tx_delay_done_sema);

                volatile uint8_t st = ke_state_get(TASK_LLM);
                if (st == LLM_IDLE && tx_intv_state == TXINTV_ST_STOPPED) {
                        tx_intv_state = TXINTV_ST_TX;

                        /* This must only be called when the stack is IDLE */
                        configASSERT(ke_state_get(TASK_LLM) == LLM_IDLE);
                        llm_test_mode_start_tx(
                                (struct hci_le_tx_test_cmd const *)&prod_test_tx_con_test);
                }
                else {
                        /* At this point, the state must be STOPPING */
                        configASSERT(st == LLM_STOPPING);
                        tx_intv_state = TXINTV_ST_WAIT_START_TX;

                }
                set_state_start_tx_intv();

                send_ble_evt(BPT_OPCODE_PKT_TX_INTV, BPT_EVTCODE_PKT_TX_INTV_STARTED,
                                BPT_PLEN_PKT_TX_INTV_STARTED, NULL);
        }
}

static void tx_delay_cb(void)
{
        /* Must also wakeup the ble adapter so that this is processed */
        ad_ble_task_notify_from_isr(1);

        OS_EVENT_SIGNAL_FROM_ISR(tx_delay_done_sema);
        rf_tools_stop_systick();
}

void ble_prod_test_check_tx_packet_count(void)
{
        uint32_t count;

        GLOBAL_INT_DISABLE();
        count = prod_test_tx_packet_count;
        GLOBAL_INT_RESTORE();

        if (prod_test_state == STATE_PROD_TEST_TX_INTV) {
                /* Received a packet and not in infinite mode */
                if (prod_test_tx_packet_nr &&
                        count >= prod_test_tx_packet_nr) {
                        stop_pkt_tx_interval(true);
                        return;
                }

                if (tx_intv_state == TXINTV_ST_WAIT_START_TX) {
                        uint8_t st = ke_state_get(TASK_LLM);
                        if (st == LLM_IDLE) {
                                tx_intv_state = TXINTV_ST_TX;
                                llm_test_mode_start_tx( (struct hci_le_tx_test_cmd const *)&prod_test_tx_con_test);
                        }
                }

                /* A non-zero interval is defined. */
                if (prod_test_tx_interval_us > 0) {
                        if (tx_intv_state == TXINTV_ST_WAIT_TIMER) {
                                uint8_t st = ke_state_get(TASK_LLM);

                                if (st == LLM_IDLE && OS_EVENT_WAIT(tx_delay_done_sema, 0)) {
                                        /* Timer has expired, start next TX */
                                        tx_intv_state = TXINTV_ST_TX;
                                        llm_test_mode_start_tx( (struct hci_le_tx_test_cmd const *)&prod_test_tx_con_test);
                                }
                        }

                        if (tx_intv_state == TXINTV_ST_TX) {
                                if (count > prod_test_tx_prev_packet_count) {
                                        /* A new packet has been received. Stop TX and set a timer for the next tx */
                                        stop_tx();
                                        tx_intv_state = TXINTV_ST_WAIT_TIMER;
                                        rf_tools_start_systick(tx_delay_cb, prod_test_tx_interval_us);
                                        prod_test_tx_prev_packet_count = count;
                                }
                        }

                }
        }
}

static void unmodulated_trx(const ble_mgr_common_stack_msg_t *stack_msg)
{
        uint8_t oper;
        uint8_t freq;

        if (stack_msg->msg.hci.cmd.param_length == 2) {
                oper = stack_msg->msg.hci.cmd.param[0];
                freq = stack_msg->msg.hci.cmd.param[1];

                if (oper == 0x54) { /* Unmodulated TX */
                        if (prod_test_state == STATE_PROD_TEST_IDLE) {
                                prod_test_state = STATE_PROD_UNMODULATED_TRX;
                                hw_rf_start_continuous_wave(0x1, freq);
                        }
                } else if (oper == 0x52) { /* Unmodulated RX */
                        if (prod_test_state == STATE_PROD_TEST_IDLE) {
                                prod_test_state = STATE_PROD_UNMODULATED_TRX;
                                hw_rf_start_continuous_wave_rx(0x1, freq);
                        }
                } else if (oper == 0x4f) { /* OFF */
                        if (prod_test_state == STATE_PROD_UNMODULATED_TRX) {
                                hw_rf_stop_continuous_wave();
                                prod_test_state = STATE_PROD_TEST_IDLE;
                        }
                }
        }

        send_ble_evt(BPT_OPCODE_UNMODULATED_TRX, BPT_EVTCODE_UNMODULATED_TRX,
                        BPT_PLEN_UNMODULATED_TRX, NULL);
}

void ble_prod_test_cmd(const ble_mgr_common_stack_msg_t *stack_msg)
{
        switch (stack_msg->msg.hci.cmd.op_code) {
        case 0xFC81: /* start_pkt_rx_stats */
                start_pkt_rx_stats(stack_msg);
                break;
        case 0xFC82: /* stop_pkt_rx_stats */
                stop_pkt_rx_stats();
                break;
        case 0xFC83: /* unmodulated off/rx/tx (ble mode) */
                unmodulated_trx(stack_msg);
                break;
        case 0xFC84: /* start_cont_tx */
                start_cont_tx(stack_msg);
                break;
        case 0xFC85: /* stop_cont_tx */
                if (prod_test_state == STATE_PROD_TEST_TX)
                        stop_cont_tx();
                else if (prod_test_state == STATE_PROD_TEST_TX_INTV)
                        stop_pkt_tx_interval(false);
                break;
        case 0xFC90:
                pkt_tx_interval(stack_msg);
                break;
        }
}



#endif /* BLE_PROD_TEST */
