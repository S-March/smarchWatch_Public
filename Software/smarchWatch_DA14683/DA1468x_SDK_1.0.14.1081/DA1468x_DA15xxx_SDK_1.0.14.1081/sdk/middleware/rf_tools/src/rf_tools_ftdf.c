/**
 ****************************************************************************************
 *
 * @file rf_tools_ftdf.c
 *
 * @brief FTDF test tools
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#ifdef CONFIG_USE_FTDF

#include "osal.h"
#include "ad_ftdf_phy_api.h"
#include "sdk_defs.h"
#include "rf_tools_common.h"
#include "rf_tools_ftdf.h"

PRIVILEGED_DATA static struct {
        struct {
                ftdf_short_address_t src_address;
                ftdf_pan_id_t panid;
                ftdf_short_address_t dst_address;
                uint8_t packet_size;
                ftdf_channel_number_t channel;
                uint16_t num_packets;
                uint32_t intv;
        } config;
        struct {
                uint16_t transmitted_packets;
        } stats;
        bool tx_active;
} vars ;

PRIVILEGED_DATA rf_tools_ftdf_cbs_t ftdf_cbs;

static OS_TASK txstream_wait_task_handle = NULL;
static uint8_t txstream_txdata;
static OS_MUTEX txstream_done_sema;

static void enable_transparent_mode(void)
{
        ftdf_bitmap32_t options =  FTDF_TRANSPARENT_ENABLE_FCS_GENERATION;
//        options |= FTDF_TRANSPARENT_WAIT_FOR_ACK;
//        options |= FTDF_TRANSPARENT_AUTO_ACK;
        options |= FTDF_TRANSPARENT_PASS_ALL_FRAME_TYPES;
        options |= FTDF_TRANSPARENT_PASS_ALL_FRAME_VERSION;
        options |= FTDF_TRANSPARENT_PASS_ALL_PAN_ID;
        options |= FTDF_TRANSPARENT_PASS_ALL_ADDR;
        options |= FTDF_TRANSPARENT_PASS_ALL_BEACON;
        options |= FTDF_TRANSPARENT_PASS_ALL_NO_DEST_ADDR;
        ftdf_enable_transparent_mode(FTDF_TRUE, options);

}

void rf_tools_ftdf_init(rf_tools_ftdf_cbs_t cbs)
{
        ftdf_cbs = cbs;
        enable_transparent_mode();
}

/* Generic packet transmission function. Used both by pinger and ponger */
static inline void ftdf_send_packet(ftdf_data_length_t len, ftdf_short_address_t addr)
{
        int i = 0;

        /* SN and frame buffer declared as static, so as to avoid memory management issues.
         * FTDF driver does not handle frame buffer in transparent mode. */
        static ftdf_octet_t sn = 0;

        static ftdf_octet_t frame[128];

        /* Build frame. */
        frame[i++] = 0x41;
        frame[i++] = 0x88;

        /* SN */
        frame[i++] = sn++;

        /* Source PAN */
        frame[i++] = (vars.config.panid) & 0xff;
        frame[i++] = (vars.config.panid >> 8) & 0xff;

        /* Dest Address */
        frame[i++] = (addr) & 0xff;
        frame[i++] = (addr >> 8) & 0xff;

        /* Source Address */
        frame[i++] = (vars.config.src_address) & 0xff;
        frame[i++] = (vars.config.src_address >> 8) & 0xff;

        int j;
        for (j = 0; j < len ; j++)
                frame[i++] = j;

        if (ad_ftdf_send_frame_simple(i + 2, frame, vars.config.channel, 0, FTDF_TRUE) == FTDF_TRANSPARENT_OVERFLOW) {
        }

}

static void send_next_packet(void)
{

        rf_tools_stop_systick();
        ftdf_send_packet(vars.config.packet_size, vars.config.dst_address);
}

static void schedule_next_packet(void)
{
        bool active;

        vars.stats.transmitted_packets++;

        /* If more packets to be sent, or infinite mode, keep sending them */
        if (vars.config.num_packets == 0 ||
                vars.stats.transmitted_packets < vars.config.num_packets) {

                portENTER_CRITICAL();
                active = vars.tx_active;
                portEXIT_CRITICAL();

                if (!active)
                        /* Stopped */
                        return;

                /* If interval is 0, send immediately */
                if (vars.config.intv == 0) {
                        send_next_packet();
                        return;
                }

                /* Interval non-zero. Schedule a timeout */
                rf_tools_start_systick(send_next_packet, vars.config.intv);
        } else {
                portENTER_CRITICAL();
                vars.tx_active = false;
                portEXIT_CRITICAL();

                /* Transmission ended. Notify app */
                ftdf_cbs.tx_done();
        }


}

void rf_tools_ftdf_start_tx(uint8_t ch, uint8_t len, uint16_t num_packets, uint32_t intv)
{
        vars.config.channel = ch;
        vars.config.dst_address = 0x40;
        vars.config.src_address = 0x01;
        vars.config.intv = intv;
        vars.config.num_packets = num_packets;
        vars.config.packet_size = len;

        ftdf_set_value(FTDF_PIB_PAN_ID, &vars.config.panid);
        ftdf_set_value(FTDF_PIB_CURRENT_CHANNEL,
                        &vars.config.channel);
        ftdf_set_value(FTDF_PIB_SHORT_ADDRESS,
                &vars.config.src_address);

        vars.stats.transmitted_packets = 0;

        portENTER_CRITICAL();
        vars.tx_active = true;
        portEXIT_CRITICAL();
        send_next_packet();

}

void rf_tools_ftdf_stop_tx(void)
{
        portENTER_CRITICAL();
        vars.tx_active = false;
        portEXIT_CRITICAL();

        if (vars.config.intv > 0) {
                /* Stop systick */
                rf_tools_stop_systick();
        }
}

static void txstream_feed_next_symbol_cb(void)
{
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
        REG_SETF(PLLDIG, RF_MSKMOD_CTRL2_REG, TX_DATA, txstream_txdata & 0xf);
#else
        REG_SETF(PLLDIG, RF_MSKMOD_CTRL1_REG, TX_DATA, txstream_txdata & 0xf);
#endif
        txstream_txdata++;

        /* User-defined callback, to do whatever's needed to stop this (since the OS tick is stopped for this
         * to work)
         */
        rf_tools_ftdf_check_stop_txstream();
}

static void txstream_enable_tx_valid_cb(void)
{
        /* 8uS after the first symbol. Set the 2nd symbol and set the timer
         * to be 16us periodic from now on
         */
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
        REG_SETF(PLLDIG, RF_MSKMOD_CTRL2_REG, TX_DATA, txstream_txdata & 0xf);
#else
        REG_SETF(PLLDIG, RF_MSKMOD_CTRL1_REG, TX_DATA, txstream_txdata & 0xf);
#endif
        txstream_txdata++;

        /* Set a periodic timer every 16 uS, for each symbol */
        rf_tools_start_systick(txstream_feed_next_symbol_cb, 16);
}

static void txstream_wait_phy_cb(void)
{
        /* PHY Init completed. Enable TX valid, set first symbol */
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
        REG_SETF(PLLDIG, RF_MSKMOD_CTRL2_REG, TX_DATA, txstream_txdata & 0xf);
        REG_SETF(PLLDIG, RF_MSKMOD_CTRL2_REG, TX_VALID, 0x1);
#else
        REG_SETF(PLLDIG, RF_MSKMOD_CTRL1_REG, TX_DATA, txstream_txdata & 0xf);
        REG_SETF(PLLDIG, RF_MSKMOD_CTRL1_REG, TX_VALID, 0x1);
#endif

        txstream_txdata++;
        /* Set next timeout in the middle of the symbol transmission (~8us) so
         * that jitter in response to the timer will not be a problem
         */
        rf_tools_start_systick(txstream_enable_tx_valid_cb, 8);
}

static void txstream_wait_task(void *pvParameters)
{
      while (!OS_EVENT_WAIT(txstream_done_sema, 0));

      vSemaphoreDelete(txstream_done_sema);
      NVIC_EnableIRQ(SWTIM1_IRQn);
      OS_TASK_DELETE(txstream_wait_task_handle);
}

void rf_tools_ftdf_start_txstream(uint8_t ch)
{

        OS_BASE_TYPE status = OS_TASK_CREATE("TxStreamWaitTask",              /* The text name assigned to the task - for debug only as it is not used by the kernel. */
                txstream_wait_task,                    /* The System Initialization task. */
                (void *) 0,                   /* The parameter passed to the task. */
                150,       /* The size of the stack to allocate to the task. */
                tskIDLE_PRIORITY,       /* The priority assigned to the task. */
                txstream_wait_task_handle);                      /* The task handle is not required, so NULL is passed. */
        OS_ASSERT(status == OS_TASK_CREATE_SUCCESS);
        OS_EVENT_CREATE(txstream_done_sema);


        NVIC_DisableIRQ(SWTIM1_IRQn);
        txstream_txdata = 0;

        REG_SETF(PLLDIG,RF_BMCW_REG, CN_SEL, 1);
        REG_SETF(PLLDIG,RF_BMCW_REG, CN_WR, ch);

        REG_SETF(RFCU, RF_OVERRULE_REG, RF_MODE_OVR, 0x2); // Set radio in FTDF mode
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
        REG_SETF(PLLDIG, RF_MSKMOD_CTRL2_REG, MSK_TX_SEL, 0x1);   // Select to use TXDATA from PHY
#else
        REG_SETF(PLLDIG, RF_MSKMOD_CTRL1_REG, MSK_TX_SEL, 0x1);   // Select to use TXDATA from PHY
#endif
        REG_SETF(RFCU, RF_OVERRULE_REG, TX_EN_OVR, 0x2);     // Set radio in transmit mode

        rf_tools_start_systick(txstream_wait_phy_cb, 120);  // Wait long enough, so the radio is ready to transmit

}

void rf_tools_ftdf_stop_txstream(void)
{
        rf_tools_stop_systick();

#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
        REG_SETF(PLLDIG, RF_MSKMOD_CTRL2_REG, TX_VALID, 0x0);
        REG_SETF(PLLDIG, RF_MSKMOD_CTRL2_REG, MSK_TX_SEL, 0x0);
#else
        REG_SETF(PLLDIG, RF_MSKMOD_CTRL1_REG, TX_VALID, 0x0);
        REG_SETF(PLLDIG, RF_MSKMOD_CTRL1_REG, MSK_TX_SEL, 0x0);
#endif

        REG_SETF(RFCU, RF_OVERRULE_REG, TX_EN_OVR, 0x0);
        REG_SETF(RFCU, RF_OVERRULE_REG, RF_MODE_OVR, 0x0);

        REG_SETF(PLLDIG,RF_BMCW_REG, CN_SEL, 0);
        REG_SETF(PLLDIG,RF_BMCW_REG, CN_WR, 0);

        OS_EVENT_SIGNAL(txstream_done_sema);
}

void rf_tools_ftdf_start_rx(uint8_t ch)
{
        uint32_t enable_rx = 1;
        ftdf_boolean_t metrics_enable = 1;
        ftdf_performance_metrics_t metrics = {0};
        vars.config.channel = ch;

        ftdf_set_value(FTDF_PIB_METRICS_ENABLED, &metrics_enable);
        ftdf_set_value(FTDF_PIB_PERFORMANCE_METRICS, &metrics);
        ftdf_set_value(FTDF_PIB_CURRENT_CHANNEL, &vars.config.channel);
        ftdf_set_value(FTDF_PIB_RX_ON_WHEN_IDLE, &enable_rx);
}

void rf_tools_ftdf_stop_rx(ftdf_count_t *rx_success_count, ftdf_count_t *rx_fcs_error_count)
{
        uint32_t enable_rx = 0;
        ftdf_set_value(FTDF_PIB_RX_ON_WHEN_IDLE, &enable_rx);


        ftdf_performance_metrics_t *metrics = (ftdf_performance_metrics_t *)ftdf_get_value(FTDF_PIB_PERFORMANCE_METRICS);
        *rx_success_count = metrics->rx_success_count;
        *rx_fcs_error_count = metrics->fcs_error_count;

}

void rf_tools_ftdf_send_frame_confirm(void            *handle,
                                      ftdf_bitmap32_t status)
{
        schedule_next_packet();

        switch (status) {
        case FTDF_TRANSPARENT_SEND_SUCCESSFUL:
                break;
        case FTDF_TRANSPARENT_CSMACA_FAILURE:
                break;
        case FTDF_TRANSPARENT_NO_ACK:
                break;
        case FTDF_TRANSPARENT_OVERFLOW:
                break;
        default:
                ;
        }
}


/* Receive packet from n - 1, switch on led, and start timer to send to n + 1 */
void rf_tools_ftdf_recv_frame(ftdf_data_length_t frameLength,
                               ftdf_octet_t      *frame,
                               ftdf_bitmap32_t   status)
{


}


#endif
