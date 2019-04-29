/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup I2C
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file hw_i2c.c
 *
 * @brief Implementation of the I2C Low Level Driver.
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configUSE_HW_I2C


#include <stdio.h>
#include <string.h>
#include <hw_i2c.h>
#include <hw_dma.h>
#include <hw_cpm.h>

#if (dg_configSYSTEMVIEW)
#  include "SEGGER_SYSVIEW_FreeRTOS.h"
#else
#  define SEGGER_SYSTEMVIEW_ISR_ENTER()
#  define SEGGER_SYSTEMVIEW_ISR_EXIT()
#endif

#define I2C_IDX(id) (!!(id - (void *)I2C_BASE))

#define I2C_ENABLE_LOOP_LIMIT   (10)

struct tx_state {
        const uint8_t           *data;
        uint16_t                len;
        uint16_t                num;
        hw_i2c_complete_cb      cb;
        void                    *cb_data;
        uint32_t                flags;
};

struct rx_state {
        uint8_t                 *data;
        uint16_t                len;
        uint16_t                num;
        uint16_t                rr;
        hw_i2c_complete_cb      cb;
        void                    *cb_data;
#if BLACK_ORCA_IC_REV_A < dg_configBLACK_ORCA_IC_REV
        uint32                  flags;
#endif
};

struct dma_state {
        void                                    *cb_data;
        union {
                hw_i2c_complete_cb              cb;
                hw_i2c_dma_completed_handler_cb depr_cb;
        };
        HW_DMA_CHANNEL                          channel;
};

struct i2c {
        struct tx_state         tx_state;
        struct rx_state         rx_state;
        struct dma_state        dma_state;

        hw_i2c_interrupt_cb     intr_cb;
        hw_i2c_event_cb         event_cb;
};

/* I2C data are not retained. The user must ensure that they are updated after exiting sleep. */
static struct i2c i2c[2];

/* handler used for write buffer operation */
static void intr_write_buffer_handler(HW_I2C_ID id, uint16_t mask);

/* handler used by hw_i2c_prepare_dma_ex() to handle STOP and ABORT for DMA writes */
static void intr_write_buffer_dma_handler(HW_I2C_ID id, uint16_t mask);
/* handler used by hw_i2c_prepare_dma_ex() to handle STOP and ABORT for DMA writes */
static void intr_write_buffer_dma_no_stop_handler(HW_I2C_ID id, uint16_t mask);

/* handler used for read buffer operation */
static void intr_read_buffer_handler(HW_I2C_ID id, uint16_t mask);
static void intr_read_buffer_dma_handler(HW_I2C_ID id, uint16_t mask);

/* handler used for slave mode with event callback */
static void intr_slave_handler(HW_I2C_ID id, uint16_t mask);

static inline struct i2c *get_i2c(HW_I2C_ID id)
{
        return &i2c[I2C_IDX(id)];
}

void hw_i2c_init(HW_I2C_ID id, const i2c_config *cfg)
{
        IRQn_Type irq_type = I2C_IRQn;
        int enable_loop_cnt = 0;

        if (id == HW_I2C2) {
                irq_type = I2C2_IRQn;
        }
        else if (id != HW_I2C1) {
                /* Requested ID must be one of HW_I2C1 or HW_I2C2 */
                ASSERT_ERROR(0);
        }

        struct i2c *i2c = get_i2c(id);

        memset(i2c, 0, sizeof(*i2c));

        GLOBAL_INT_DISABLE();
        uint32_t clk_per_reg_local = CRG_PER->CLK_PER_REG;
        REG_SET_FIELD(CRG_PER, CLK_PER_REG, I2C_CLK_SEL, clk_per_reg_local, 0);
        REG_SET_FIELD(CRG_PER, CLK_PER_REG, I2C_ENABLE, clk_per_reg_local, 1);
        CRG_PER->CLK_PER_REG = clk_per_reg_local;
        GLOBAL_INT_RESTORE();

        hw_i2c_disable(id);
        while (hw_i2c_get_enable_status(id) & I2C_I2C_ENABLE_STATUS_REG_IC_EN_Msk) {
                hw_cpm_delay_usec(500);
                enable_loop_cnt++;
                /* we shouldn't get stuck here, the HW I2C block should eventually be enabled */
                ASSERT_ERROR(enable_loop_cnt < I2C_ENABLE_LOOP_LIMIT);
        }

        IBA(id)->I2C_INTR_MASK_REG = 0x0000;

        hw_i2c_configure(id, cfg);

        NVIC_EnableIRQ(irq_type);
}

void hw_i2c_configure(HW_I2C_ID id, const i2c_config *cfg)
{
        /*
         * we always perform configuration of I2C clock (SCL) since it's essential for I2C controller
         * to work properly and in case it's not provided by caller, we just set recommended values
         * from datasheet
         */
        if (!cfg || (!cfg->clock_cfg.ss_hcnt && !cfg->clock_cfg.ss_lcnt)) {
                IBA(id)->I2C_SS_SCL_HCNT_REG = 0x48;
                IBA(id)->I2C_SS_SCL_LCNT_REG = 0x4F;
        } else {
                IBA(id)->I2C_SS_SCL_HCNT_REG = cfg->clock_cfg.ss_hcnt;
                IBA(id)->I2C_SS_SCL_LCNT_REG = cfg->clock_cfg.ss_lcnt;
        }

        if (!cfg || (!cfg->clock_cfg.ss_hcnt && !cfg->clock_cfg.ss_lcnt)) {
                IBA(id)->I2C_FS_SCL_HCNT_REG = 0x08;
                IBA(id)->I2C_FS_SCL_LCNT_REG = 0x17;
        } else {
                IBA(id)->I2C_FS_SCL_HCNT_REG = cfg->clock_cfg.fs_hcnt;
                IBA(id)->I2C_FS_SCL_LCNT_REG = cfg->clock_cfg.fs_lcnt;
        }

        if (!cfg) {
                return;
        }

        hw_i2c_set_speed(id, cfg->speed);
        hw_i2c_set_mode(id, cfg->mode);

        if (cfg->mode == HW_I2C_MODE_MASTER) {
                hw_i2c_setup_master(id, cfg->addr_mode, cfg->address);
        } else {
                hw_i2c_setup_slave(id, cfg->addr_mode, cfg->address, cfg->event_cb);
        }
}

void hw_i2c_register_int(HW_I2C_ID id, hw_i2c_interrupt_cb cb, uint16_t mask)
{
        struct i2c *i2c = get_i2c(id);

        i2c->intr_cb = cb;

        IBA(id)->I2C_INTR_MASK_REG = mask;
}

void hw_i2c_unregister_int(HW_I2C_ID id)
{
        IRQn_Type irq_type = I2C_IRQn;

        if (id == HW_I2C2) {
                irq_type = I2C2_IRQn;
        }

        hw_i2c_register_int(id, NULL, 0);
        NVIC_ClearPendingIRQ(irq_type);
}

void hw_i2c_set_int_mask(HW_I2C_ID id, uint16_t mask)
{
        IBA(id)->I2C_INTR_MASK_REG = mask;
}

uint16_t hw_i2c_get_int_mask(HW_I2C_ID id)
{
        return IBA(id)->I2C_INTR_MASK_REG;
}

void hw_i2c_setup_master(HW_I2C_ID id, HW_I2C_ADDRESSING addr_mode, uint16_t target_addr)
{
        hw_i2c_set_mode(id, HW_I2C_MODE_MASTER);
        hw_i2c_set_target_addressing_mode(id, addr_mode);
        hw_i2c_set_target_address(id, target_addr);
        while (hw_i2c_is_master_busy(id));
}

void hw_i2c_setup_slave(HW_I2C_ID id, HW_I2C_ADDRESSING addr_mode, uint16_t addr,
                                                                                 hw_i2c_event_cb cb)
{

        hw_i2c_set_mode(id, HW_I2C_MODE_SLAVE);
        hw_i2c_set_slave_addressing_mode(id, addr_mode);
        hw_i2c_set_slave_address(id, addr);
        hw_i2c_set_slave_callback(id, cb);
}

void hw_i2c_set_slave_callback(HW_I2C_ID id, hw_i2c_event_cb cb)
{
        struct i2c *i2c = get_i2c(id);

        i2c->event_cb = cb;

        /*
         * there's no need for app to specify event callback - if not specified there's no need for
         * interrupt handler as well
         */
        if (!cb) {
                hw_i2c_unregister_int(id);
                return;
        }

        /*
         * need to setup RX threshold as low as possible to have interrupt as soon as possible,
         * otherwise we'll have overruns easily.
         */
        hw_i2c_set_rx_fifo_threshold(id, 0);

        hw_i2c_register_int(id, intr_slave_handler, HW_I2C_INT_READ_REQUEST | HW_I2C_INT_RX_FULL |
                                                      HW_I2C_INT_RX_OVERFLOW | HW_I2C_INT_TX_ABORT);
}

void hw_i2c_register_slave_dma_read_callback(HW_I2C_ID id)
{
        hw_i2c_reset_int_stop_detected(id);
        hw_i2c_reset_int_read_request(id);
        hw_i2c_register_int(id, intr_read_buffer_dma_handler,
                HW_I2C_INT_STOP_DETECTED | HW_I2C_INT_READ_REQUEST);
}

bool hw_i2c_write_buffer(HW_I2C_ID id, const uint8_t *data, uint16_t len,
                hw_i2c_complete_cb cb, void *cb_data, bool wait_for_stop)
{
        if (!data) {
                return false;
        }

        if (!cb) {
                while (len--) {
                        while (!hw_i2c_is_tx_fifo_not_full(id));
#if BLACK_ORCA_IC_REV_A == dg_configBLACK_ORCA_IC_REV
                        hw_i2c_write_byte(id, *data);
#else
                        IBA(id)->I2C_DATA_CMD_REG = *data |
                                ((len == 0 && wait_for_stop) ? 0 : I2C_I2C_DATA_CMD_REG_STOP_Msk);
#endif
                        data++;
                        if (hw_i2c_get_abort_source(id)) {
                                return false;
                        }

                }

                while (!hw_i2c_is_tx_fifo_empty(id));
                while (hw_i2c_is_master_busy(id));

                if (hw_i2c_get_abort_source(id)) {
                        return false;
                }
        }
        else {
                struct i2c *i2c = get_i2c(id);

                i2c->tx_state.data = data;
                i2c->tx_state.len = len;
                i2c->tx_state.num = 0;
                i2c->tx_state.cb = cb;
                i2c->tx_state.cb_data = cb_data;
                i2c->tx_state.flags = wait_for_stop? HW_I2C_F_WAIT_FOR_STOP : HW_I2C_F_NONE;

                hw_i2c_reset_int_tx_abort(id);
                if (wait_for_stop) {
                        hw_i2c_reset_int_stop_detected(id);
                }

                hw_i2c_register_int(id, intr_write_buffer_handler, HW_I2C_INT_TX_EMPTY |
                        (wait_for_stop ? HW_I2C_INT_STOP_DETECTED : 0) | HW_I2C_INT_TX_ABORT);

                /* we want TX_EMPTY as soon as FIFO is not completely full */
                hw_i2c_set_tx_fifo_threshold(id, I2C_FIFO_DEPTH - 1);
        }

        return true;
}

size_t hw_i2c_write_buffer_sync(HW_I2C_ID id, const uint8_t *data, uint16_t len,
                                HW_I2C_ABORT_SOURCE *abrt_code, uint32_t flags)
{
        HW_I2C_ABORT_SOURCE ret = HW_I2C_ABORT_NONE;
        size_t offst = 0;

        if (!data || len == 0) {
                ret = HW_I2C_ABORT_SW_ERROR;
        }
        else {
                while (len--) {
                        while (!hw_i2c_is_tx_fifo_not_full(id));

#if BLACK_ORCA_IC_REV_A == dg_configBLACK_ORCA_IC_REV
                        hw_i2c_write_byte(id, data[offst++]);
#else
                        IBA(id)->I2C_DATA_CMD_REG = data[offst++] |
                                ((len == 0 && (flags & HW_I2C_F_ADD_STOP)) ? I2C_I2C_DATA_CMD_REG_STOP_Msk : 0);
#endif
                        ret = hw_i2c_get_abort_source(id);
                        if (ret) {
                                break;
                        }
                }

                if (!ret && (flags & HW_I2C_F_WAIT_FOR_STOP)) {
                        while (!hw_i2c_is_tx_fifo_empty(id));
                        while (hw_i2c_is_master_busy(id));
                        ret = hw_i2c_get_abort_source(id);
                }
        }

        if (abrt_code) {
                *abrt_code = ret;
        }

        if (ret) {
                hw_i2c_reset_int_tx_abort(id);
        }

        return offst;
}

int hw_i2c_write_buffer_async(HW_I2C_ID id, const uint8_t *data, uint16_t len,
                              hw_i2c_complete_cb cb, void *cb_data, uint32_t flags)
{
        struct i2c *i2c = get_i2c(id);
        uint16_t mask = HW_I2C_INT_TX_EMPTY | HW_I2C_INT_TX_ABORT;

        if (!cb || !data || len == 0) {
                if (cb) {
                        cb(id, cb_data, 0, false);
                }
                return -1;
        }

        i2c->tx_state.data = data;
        i2c->tx_state.len = len;
        i2c->tx_state.num = 0;
        i2c->tx_state.cb = cb;
        i2c->tx_state.cb_data = cb_data;
        i2c->tx_state.flags = flags;

        hw_i2c_reset_int_tx_abort(id);

        if (flags & HW_I2C_F_WAIT_FOR_STOP) {
                hw_i2c_reset_int_stop_detected(id);
                mask |= HW_I2C_INT_STOP_DETECTED;
        }

        /* we want TX_EMPTY as soon as FIFO is not completely full */
        hw_i2c_set_tx_fifo_threshold(id, I2C_FIFO_DEPTH - 1);

        hw_i2c_register_int(id, intr_write_buffer_handler, mask);

        return 0;
}

bool hw_i2c_read_buffer(HW_I2C_ID id, uint8_t *data, uint16_t len, hw_i2c_complete_cb cb,
                                                                                void *cb_data)
{
        uint16_t num = 0;
        uint16_t rr = 0;

        if (!data) {
                return false;
        }

        if (!cb) {
                while (num < len) {
                        while (rr < len && hw_i2c_is_tx_fifo_not_full(id)) {
                                hw_i2c_read_byte_trigger(id);
                                rr++;
                        }
                        if (hw_i2c_get_abort_source(id)) {
                                return false;
                        }
                        if (num < len && hw_i2c_get_rx_fifo_level(id)) {
                                data[num] = hw_i2c_read_byte(id);
                                num++;
                        }
                }

                while (hw_i2c_is_master_busy(id));

                if (hw_i2c_get_abort_source(id)) {
                        return false;
                }
        }
        else {
                struct i2c *i2c = get_i2c(id);

                i2c->rx_state.data = data;
                i2c->rx_state.len = len;
                i2c->rx_state.num = 0;
                i2c->rx_state.rr = 0;
                i2c->rx_state.cb = cb;
                i2c->rx_state.cb_data = cb_data;

                hw_i2c_set_rx_fifo_threshold(id, 0);

                hw_i2c_reset_int_tx_abort(id);

                hw_i2c_register_int(id, intr_read_buffer_handler, HW_I2C_INT_TX_EMPTY |
                                                        HW_I2C_INT_RX_FULL | HW_I2C_INT_TX_ABORT);
        }

        return true;
}

size_t hw_i2c_read_buffer_sync(HW_I2C_ID id, uint8_t *data, uint16_t len,
                               HW_I2C_ABORT_SOURCE *abrt_code, uint32_t flags)
{
        HW_I2C_ABORT_SOURCE ret = HW_I2C_ABORT_NONE;
        size_t nn = 0;
        uint16_t rr = 0;

        if (!data) {
                ret = HW_I2C_ABORT_SW_ERROR;
        }
        else {
                while (nn < len) {
                        while (rr < len && hw_i2c_is_tx_fifo_not_full(id)) {
                                rr++;
#if BLACK_ORCA_IC_REV_A == dg_configBLACK_ORCA_IC_REV
                                hw_i2c_read_byte_trigger(id);
#else
                                IBA(id)->I2C_DATA_CMD_REG = 1 << I2C_I2C_DATA_CMD_REG_CMD_Pos |
                                                ((rr == len && (flags & HW_I2C_F_ADD_STOP)) ?
                                                I2C_I2C_DATA_CMD_REG_STOP_Msk : 0);
#endif
                        }
                        while (nn < len && hw_i2c_get_rx_fifo_level(id)) {
                                data[nn] = hw_i2c_read_byte(id);
                                nn++;
                        }
                        ret = hw_i2c_get_abort_source(id);
                        if (ret) {
                                break;
                        }
                }
        }

        if (abrt_code) {
                *abrt_code = ret;
        }

        if (ret) {
                hw_i2c_reset_int_tx_abort(id);
        }

        return nn;
}

int hw_i2c_read_buffer_async(HW_I2C_ID id, uint8_t *data, uint16_t len,
                             hw_i2c_complete_cb cb, void *cb_data, uint32_t flags)
{
        struct i2c *i2c = get_i2c(id);
        bool master = HW_I2C_REG_GETF(id, I2C_CON, I2C_MASTER_MODE);
        uint16_t mask = master ? HW_I2C_INT_TX_EMPTY : HW_I2C_INT_READ_REQUEST;
        mask |= HW_I2C_INT_RX_FULL | HW_I2C_INT_TX_ABORT;

        if (!cb || !data || len == 0) {
                if (cb) {
                        cb(id, cb_data, 0, false);
                }
                return -1;
        }

        i2c->rx_state.data = data;
        i2c->rx_state.len = len;
        i2c->rx_state.num = 0;
        /*
         * In slave mode there is no need for read requests, so set rr to len and interrupt
         * will not try to fill TX FIFO.
         */
        i2c->rx_state.rr = master ? 0 : len;
        i2c->rx_state.cb = cb;
        i2c->rx_state.cb_data = cb_data;
#if BLACK_ORCA_IC_REV_A < dg_configBLACK_ORCA_IC_REV
        i2c->rx_state.flags = flags;
#endif

        i2c->tx_state.len = 0;
        i2c->tx_state.num = 0;

        hw_i2c_set_rx_fifo_threshold(id, 0);

        hw_i2c_reset_int_tx_abort(id);

        hw_i2c_register_int(id, intr_read_buffer_handler, mask);

        return (int)len;
}

int hw_i2c_write_then_read_async(HW_I2C_ID id, const uint8_t *w_data, uint16_t w_len,
                                        uint8_t *r_data, uint16_t r_len, hw_i2c_complete_cb cb,
                                        void *cb_data, uint32_t flags)
{
        struct i2c *i2c = get_i2c(id);

        if (!cb || !w_data || w_len == 0 || !r_data || r_len == 0) {
                if (cb) {
                        cb(id, cb_data, 0, false);
                }
                return -1;
        }

        i2c->tx_state.data = w_data;
        i2c->tx_state.len = w_len;
        i2c->tx_state.num = 0;
        i2c->tx_state.cb = NULL;
        i2c->tx_state.cb_data = NULL;
        i2c->tx_state.flags = flags;

        i2c->rx_state.data = r_data;
        i2c->rx_state.len = r_len;
        i2c->rx_state.num = 0;
        i2c->rx_state.rr = 0;
        i2c->rx_state.cb = cb;
        i2c->rx_state.cb_data = cb_data;

        hw_i2c_reset_int_tx_abort(id);

        hw_i2c_reset_int_stop_detected(id);

        /* we want TX_EMPTY as soon as FIFO is not completely full */
        hw_i2c_set_tx_fifo_threshold(id, I2C_FIFO_DEPTH - 1);
        hw_i2c_set_rx_fifo_threshold(id, 0);

        hw_i2c_register_int(id, intr_read_buffer_handler,
                                HW_I2C_INT_TX_EMPTY | HW_I2C_INT_RX_FULL | HW_I2C_INT_TX_ABORT);

        return 0;
}

static void hw_i2c_dma_cb(void *user_data, uint16_t len)
{
        HW_I2C_ID id = (HW_I2C_ID) user_data;
        struct i2c *i2c = get_i2c(id);

        if (i2c->dma_state.cb) {
                i2c->dma_state.cb(id, i2c->dma_state.cb_data, len, false);
                i2c->dma_state.cb = NULL;
        }

        /* disable I2C DMA */
        IBA(id)->I2C_DMA_CR_REG = 0;
}

/* Non-cached, non-retained global. */
static volatile uint16_t hw_i2c_prepare_dma_read_cmd;

void hw_i2c_prepare_dma(HW_I2C_ID id, uint8_t channel, uint16_t *data, uint16_t len,
                HW_I2C_DMA_TRANSFER type, hw_i2c_dma_completed_handler_cb cb, void *cb_data)
{
        hw_i2c_prepare_dma_read_cmd = 0x100;
        DMA_setup dma;
        struct i2c *i2c = get_i2c(id);

        /* for sanity so even if channel is set to odd number, we'll use proper pair */
        channel &= 0xfe;

        /* make sure I2C DMA is off so it's not unexpectedly triggered when channels are enabled */
        IBA(id)->I2C_DMA_CR_REG = 0;

        i2c->dma_state.cb_data = cb_data;
        i2c->dma_state.depr_cb = cb;
        i2c->dma_state.channel = channel;

        /* RX channel, not used only when writing data */
        if (type != HW_I2C_DMA_TRANSFER_WRITE) {
                dma.channel_number = channel;
                dma.bus_width = HW_DMA_BW_BYTE;
                dma.irq_enable = HW_DMA_IRQ_STATE_ENABLED;
                dma.irq_nr_of_trans = 0;
                dma.dreq_mode = HW_DMA_DREQ_TRIGGERED;
                dma.a_inc = HW_DMA_AINC_FALSE;
                dma.b_inc = HW_DMA_BINC_TRUE;
                dma.circular = HW_DMA_MODE_NORMAL;
                /*
                 * Set DMA priority to highest; see Tx channel setup below for explanation.
                 */
                dma.dma_prio = HW_DMA_PRIO_7;
                dma.dma_idle = HW_DMA_IDLE_INTERRUPTING_MODE; /* Not used by the HW in this case */
                dma.dma_init = HW_DMA_INIT_AX_BX_AY_BY;
                dma.dma_req_mux = id == HW_I2C2 ? HW_DMA_TRIG_I2C2_RXTX : HW_DMA_TRIG_I2C_RXTX;
                dma.src_address = (uint32) &IBA(id)->I2C_DATA_CMD_REG;
                dma.dest_address = (uint32_t) data;
                dma.length = len;
                dma.callback = hw_i2c_dma_cb;
                dma.user_data = (void *) id;
                hw_dma_channel_initialization(&dma);
                hw_dma_channel_enable(channel, HW_DMA_STATE_ENABLED);
        }

        /*
         * TX channel
         * used also when reading as master since we need to trigger read by writing read command
         * to TX FIFO
         */
        if (type != HW_I2C_DMA_TRANSFER_SLAVE_READ) {
                bool is_rx = (type != HW_I2C_DMA_TRANSFER_WRITE);

                dma.channel_number = channel + 1;
                dma.bus_width = HW_DMA_BW_HALFWORD;
                dma.irq_enable = HW_DMA_IRQ_STATE_ENABLED;
                dma.irq_nr_of_trans = 0;
                dma.dreq_mode = HW_DMA_DREQ_TRIGGERED;
                /* for RX no need to increment Ax, we read single value only */
                dma.a_inc = is_rx ? HW_DMA_AINC_FALSE : HW_DMA_AINC_TRUE;
                dma.b_inc = HW_DMA_BINC_FALSE;
                dma.circular = HW_DMA_MODE_NORMAL;
                /*
                 * Set DMA priority to highest, to avoid case of bus starvation due to a
                 * higher-priority DMA transaction, which will drain the FIFO and
                 * introduce a STOP bit.
                 * If both I2C and I2C2 are transmitting via DMA, their relative priority
                 * will be defined by the DMA channels they are assigned.
                 * However, the I2C bus frequency is much lower than the frequency that the
                 * DMA controller runs at, so it is not expected that the DMA for I2C will
                 * cause bus starvation to the DMA for I2C2 (and vice versa).
                 */
                dma.dma_prio = HW_DMA_PRIO_7;
                dma.dma_idle = HW_DMA_IDLE_INTERRUPTING_MODE; /* Not used by the HW in this case */
                /*
                 * We don't use HW_DMA_INIT_AX_BX_BY because it will lock the bus until
                 * the DMA transaction is finished, which might cause bus starvation to
                 * other peripherals.
                 */
                dma.dma_init =  HW_DMA_INIT_AX_BX_AY_BY;
                dma.dma_req_mux = id == HW_I2C2 ? HW_DMA_TRIG_I2C2_RXTX : HW_DMA_TRIG_I2C_RXTX;
                /* for RX we store read command separately */
                dma.src_address = (uint32_t) (is_rx ? &hw_i2c_prepare_dma_read_cmd : data);
                dma.dest_address = (uint32) &IBA(id)->I2C_DATA_CMD_REG;
                dma.length = len;
                /* we need this callback only when writing, otherwise already set fo receiving */
                dma.callback = (type != HW_I2C_DMA_TRANSFER_WRITE ? NULL : hw_i2c_dma_cb);
                dma.user_data = (void *) id;
                hw_dma_channel_initialization(&dma);
                hw_dma_channel_enable(channel + 1, HW_DMA_STATE_ENABLED);
        }

        /* we can set both, does not matter than one of them won't be used */
        IBA(id)->I2C_DMA_TDLR_REG = 2;
        IBA(id)->I2C_DMA_RDLR_REG = 0;
}

static void dma_rx_reply(HW_I2C_ID id, bool success)
{
        struct i2c *i2c = get_i2c(id);

        hw_i2c_unregister_int(id);

        i2c->rx_state.data = NULL;
        if (i2c->dma_state.cb) {
                i2c->dma_state.cb(id, i2c->dma_state.cb_data, i2c->rx_state.num, success);
        }
}

static void dma_tx_reply(HW_I2C_ID id, bool success)
{
        struct i2c *i2c = get_i2c(id);

        hw_i2c_unregister_int(id);

        i2c->tx_state.data = NULL;
        if (i2c->dma_state.cb) {
                i2c->dma_state.cb(id, i2c->dma_state.cb_data, i2c->tx_state.num, success);
        }
}

static void notify_on_dma_write_end_no_stop_cb(void *user_data, uint16 len)
{
        HW_I2C_ID id = (HW_I2C_ID) user_data;
        struct i2c *i2c = get_i2c(id);
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_B
        uint8_t *buf_data = (uint8_t *) i2c->tx_state.data;

        /* Clear stop condition bit from last data packet to keep data buffer in original state */
        buf_data[len - 1] &= ~I2C_I2C_DATA_CMD_REG_STOP_Msk;
#endif

        /* disable I2C DMA */
        IBA(id)->I2C_DMA_CR_REG = 0;

        dma_tx_reply(id, len == i2c->tx_state.len);
}

static void notify_on_dma_write_end_cb(void *user_data, uint16 len)
{
        HW_I2C_ID id = (HW_I2C_ID) user_data;
        struct i2c *i2c = get_i2c(id);
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_B
        uint8_t *buf_data = (uint8_t *) i2c->tx_state.data;

        /* Clear stop condition bit from last data packet to keep data buffer in original state */
        buf_data[len - 1] &= ~I2C_I2C_DATA_CMD_REG_STOP_Msk;
#endif

        /*
         * store len, to pass to user's cb when STOP/ABORT is detected
         */
        i2c->tx_state.num = len;

        /* disable I2C DMA */
        IBA(id)->I2C_DMA_CR_REG = 0;
}

static void notify_on_dma_read_end_cb(void *user_data, uint16 len)
{
        HW_I2C_ID id = (HW_I2C_ID) user_data;
        struct i2c *i2c = get_i2c(id);
        struct rx_state *rxs = &i2c->rx_state;

        rxs->num = len;

        /* disable I2C DMA */
        IBA(id)->I2C_DMA_CR_REG = 0;

        dma_rx_reply(id, rxs->num == rxs->len);
}

#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_B
static void notify_on_dma_read_request_end_cb(void *user_data, uint16 len)
{
        HW_I2C_ID id = (HW_I2C_ID) user_data;

        /* Add STOP to read request for last byte */
        IBA(id)->I2C_DATA_CMD_REG = I2C_I2C_DATA_CMD_REG_CMD_Msk | I2C_I2C_DATA_CMD_REG_STOP_Msk;
}
#endif

/* Non-cached, non-retained global. */
static volatile uint16_t hw_i2c_prepare_dma_ex_read_cmd;

void hw_i2c_prepare_dma_ex(HW_I2C_ID id, uint8_t channel, uint16_t *data, uint16_t len,
                HW_I2C_DMA_TRANSFER type, hw_i2c_complete_cb cb, void *cb_data, bool notify_on_stop)
{
        hw_i2c_prepare_dma_ex_read_cmd = 0x100;
        DMA_setup dma;
        struct i2c *i2c = get_i2c(id);

        /* for sanity so even if channel is set to odd number, we'll use proper pair */
        channel &= 0xfe;

        /* make sure I2C DMA is off so it's not unexpectedly triggered when channels are enabled */
        IBA(id)->I2C_DMA_CR_REG = 0;

        i2c->dma_state.cb = cb;
        i2c->dma_state.cb_data = cb_data;
        i2c->dma_state.channel = channel;

        /* RX channel, not used only when writing data */
        if (type != HW_I2C_DMA_TRANSFER_WRITE) {
                dma.channel_number = channel;
                dma.bus_width = HW_DMA_BW_BYTE;
                dma.irq_enable = HW_DMA_IRQ_STATE_ENABLED;
                dma.irq_nr_of_trans = 0;
                dma.dreq_mode = HW_DMA_DREQ_TRIGGERED;
                dma.a_inc = HW_DMA_AINC_FALSE;
                dma.b_inc = HW_DMA_BINC_TRUE;
                dma.circular = HW_DMA_MODE_NORMAL;
                /*
                 * Set DMA priority to highest; see Tx channel setup below for explanation.
                 */
                dma.dma_prio = HW_DMA_PRIO_7;
                dma.dma_idle = HW_DMA_IDLE_INTERRUPTING_MODE; /* Not used by the HW in this case */
                dma.dma_init = HW_DMA_INIT_AX_BX_AY_BY;
                dma.dma_req_mux = id == HW_I2C2 ? HW_DMA_TRIG_I2C2_RXTX : HW_DMA_TRIG_I2C_RXTX;
                dma.src_address = (uint32) &IBA(id)->I2C_DATA_CMD_REG;
                dma.dest_address = (uint32_t) data;
                dma.length = len;
                dma.callback = notify_on_dma_read_end_cb;
                i2c->rx_state.num = 0;
                i2c->rx_state.len = len;
                dma.user_data = (void *) id;
                hw_dma_channel_initialization(&dma);
                hw_dma_channel_enable(channel, HW_DMA_STATE_ENABLED);
        }

        /*
         * TX channel
         * used also when reading as master since we need to trigger read by writing read command
         * to TX FIFO
         */
        if (type != HW_I2C_DMA_TRANSFER_SLAVE_READ) {
                bool is_rx = (type != HW_I2C_DMA_TRANSFER_WRITE);

                dma.channel_number = channel + 1;
                dma.bus_width = HW_DMA_BW_HALFWORD;
                dma.irq_enable = HW_DMA_IRQ_STATE_ENABLED;
                dma.irq_nr_of_trans = 0;
                dma.dreq_mode = HW_DMA_DREQ_TRIGGERED;
                /* for RX no need to increment Ax, we read single value only */
                dma.a_inc = is_rx ? HW_DMA_AINC_FALSE : HW_DMA_AINC_TRUE;
                dma.b_inc = HW_DMA_BINC_FALSE;
                dma.circular = HW_DMA_MODE_NORMAL;
                /*
                 * Set DMA priority to highest, to avoid case of bus starvation due to a
                 * higher-priority DMA transaction, which will drain the FIFO and
                 * introduce a STOP bit.
                 * If both I2C and I2C2 are transmitting via DMA, their relative priority
                 * will be defined by the DMA channels they are assigned.
                 * However, the I2C bus frequency is much lower than the frequency that the
                 * DMA controller runs at, so it is not expected that the DMA for I2C will
                 * cause bus starvation to the DMA for I2C2 (and vice versa).
                 */
                dma.dma_prio = HW_DMA_PRIO_7;
                dma.dma_idle = HW_DMA_IDLE_INTERRUPTING_MODE; /* Not used by the HW in this case */
                /*
                 * We don't use HW_DMA_INIT_AX_BX_BY because it will lock the bus until
                 * the DMA transaction is finished, which might cause bus starvation to
                 * other peripherals.
                 */
                dma.dma_init =  HW_DMA_INIT_AX_BX_AY_BY;
                dma.dma_req_mux = id == HW_I2C2 ? HW_DMA_TRIG_I2C2_RXTX : HW_DMA_TRIG_I2C_RXTX;
                /* for RX we store read command separately */
                dma.src_address = (uint32_t) (is_rx ? &hw_i2c_prepare_dma_ex_read_cmd : data);
                dma.dest_address = (uint32) &IBA(id)->I2C_DATA_CMD_REG;
                if (dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_B && is_rx) {
                        dma.length = len - 1;
                } else {
                        dma.length = len;
                }
                dma.user_data = (void *) id;

                if (type == HW_I2C_DMA_TRANSFER_WRITE) {
                        uint16_t int_mask = HW_I2C_INT_TX_ABORT;

                        hw_i2c_reset_int_tx_abort(id);

                        i2c->tx_state.num = 0;
                        i2c->tx_state.len = len;

#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_B
                        /* Set stop condition bit in last data packet when DMA is used */
                        data[len - 1] |= I2C_I2C_DATA_CMD_REG_STOP_Msk;
#endif

                        if (notify_on_stop) {
                                int_mask |= HW_I2C_INT_STOP_DETECTED;

                                hw_i2c_reset_int_stop_detected(id);

                                dma.callback = notify_on_dma_write_end_cb;
                                /*
                                 * install an interrupt handler to detect STOP or ABORT,
                                 * which will trigger user's cb
                                 */
                                hw_i2c_register_int(id, intr_write_buffer_dma_handler, int_mask);
                                /* we want TX_EMPTY as soon as FIFO is empty */
                                hw_i2c_set_tx_fifo_threshold(id, 0);
                        } else {
                                dma.callback = notify_on_dma_write_end_no_stop_cb;
                                /*
                                 * install an interrupt handler to detect ABORT,
                                 * which will disable I2C DMA, which will trigger user's cb
                                 */
                                hw_i2c_register_int(id, intr_write_buffer_dma_no_stop_handler,
                                                int_mask);
                        }
                } else {
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_B
                        /* Add STOP to the last read request */
                        dma.callback = notify_on_dma_read_request_end_cb;
#else
                        /* Rx DMA has been taken care of already */
                        dma.callback = NULL;
#endif
                }

                hw_dma_channel_initialization(&dma);
                hw_dma_channel_enable(channel + 1, HW_DMA_STATE_ENABLED);
        }

        /* we can set both, does not matter than one of them won't be used */
        IBA(id)->I2C_DMA_TDLR_REG = 2;
        IBA(id)->I2C_DMA_RDLR_REG = 0;
}

void hw_i2c_dma_start(HW_I2C_ID id)
{
        IBA(id)->I2C_DMA_CR_REG = (1 << I2C_I2C_DMA_CR_REG_TDMAE_Pos) | (1 << I2C_I2C_DMA_CR_REG_RDMAE_Pos);
}

void hw_i2c_read_buffer_dma(HW_I2C_ID id, uint8_t channel, uint16_t *data, uint16_t len,
                                        hw_i2c_dma_completed_handler_cb cb, void *cb_data)
{
        /* don't issue warnings in the implementation of deprecated API */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        hw_i2c_prepare_dma(id, channel, data, len, HW_I2C_DMA_TRANSFER_MASTER_READ, cb, cb_data);
#pragma GCC diagnostic pop
        hw_i2c_dma_start(id);
}

void hw_i2c_write_buffer_dma(HW_I2C_ID id, uint8_t channel, const uint16_t *data, uint16_t len,
                                        hw_i2c_dma_completed_handler_cb cb, void *cb_data)
{
        /* don't issue warnings in the implementation of deprecated API */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        hw_i2c_prepare_dma(id, channel, (uint16_t *) data, len, HW_I2C_DMA_TRANSFER_WRITE, cb,
                                                                                        cb_data);
#pragma GCC diagnostic pop
        hw_i2c_dma_start(id);
}

void hw_i2c_write_buffer_dma_ex(HW_I2C_ID id, uint8_t channel, const uint16_t *data, uint16_t len,
                hw_i2c_complete_cb cb, void *cb_data, bool notify_on_stop)
{
        hw_i2c_prepare_dma_ex(id, channel, (uint16_t *)data, len, HW_I2C_DMA_TRANSFER_WRITE, cb,
                        cb_data, notify_on_stop);
        hw_i2c_dma_start(id);
}

void hw_i2c_read_buffer_dma_ex(HW_I2C_ID id, uint8_t channel, uint8_t *data, uint16_t len,
                hw_i2c_complete_cb cb, void *cb_data)
{
        bool master = HW_I2C_REG_GETF(id, I2C_CON, I2C_MASTER_MODE);

        hw_i2c_prepare_dma_ex(id, channel, (uint16_t *)data, len,
                        master ? HW_I2C_DMA_TRANSFER_MASTER_READ : HW_I2C_DMA_TRANSFER_SLAVE_READ,
                        cb, cb_data, false);
        hw_i2c_dma_start(id);
}

static void tx_reply(HW_I2C_ID id, bool success)
{
        struct i2c *i2c = get_i2c(id);

        hw_i2c_unregister_int(id);

        i2c->tx_state.data = NULL;
        if (i2c->tx_state.cb) {
                i2c->tx_state.cb(id, i2c->tx_state.cb_data, i2c->tx_state.num, success);
        }
}

static void rx_reply(HW_I2C_ID id, bool success)
{
        struct i2c *i2c = get_i2c(id);

        hw_i2c_unregister_int(id);

        i2c->rx_state.data = NULL;
        if (i2c->rx_state.cb) {
                i2c->rx_state.cb(id, i2c->rx_state.cb_data, i2c->rx_state.num, success);
        }
}

static void intr_write_buffer_handler(HW_I2C_ID id, uint16_t mask)
{
        struct i2c *i2c = get_i2c(id);
        struct tx_state *txs = &i2c->tx_state;

        if (!txs->data || mask == 0) {
                return;
        }

        if (mask & HW_I2C_INT_TX_ABORT) {
                tx_reply(id, false);

                /* clear abort */
                hw_i2c_reset_int_tx_abort(id);

                return;
        }

        if (mask & HW_I2C_INT_STOP_DETECTED) {
                tx_reply(id, txs->num == txs->len);
                hw_i2c_reset_int_stop_detected(id);
                return;
        }

        if (!(mask & HW_I2C_INT_TX_EMPTY)) {
                tx_reply(id, false);
                return;
        }

        while (txs->num < txs->len && hw_i2c_is_tx_fifo_not_full(id)) {
#if BLACK_ORCA_IC_REV_A == dg_configBLACK_ORCA_IC_REV
                hw_i2c_write_byte(id, txs->data[txs->num]);
#else
                if (txs->num + 1 < txs->len) {
                        hw_i2c_write_byte(id, txs->data[txs->num]);
                } else {
                        /* Add STOP request to last byte */
                        IBA(id)->I2C_DATA_CMD_REG = txs->data[txs->num] |
                                ((txs->flags & HW_I2C_F_ADD_STOP) ? I2C_I2C_DATA_CMD_REG_STOP_Msk : 0);
                }
#endif
                txs->num++;
        }

        /*
         * trigger reply when all data were written to TX FIFO and either TX FIFO is empty
         * (controller will generate STOP condition on bus) or caller requested immediate callback
         * (caller can continue with another transfer immediately).
         */
        if (txs->num == txs->len) {
                if (txs->flags & HW_I2C_F_WAIT_FOR_STOP) {
                        hw_i2c_set_int_mask(id, hw_i2c_get_int_mask(id) & ~HW_I2C_INT_TX_EMPTY);
                } else {
                        tx_reply(id, true);
                }
        }
}

/*
 * Interrupt handler used by hw_i2c_prepare_dma_ex() to handle ABORT for DMA writes
 */
static void intr_write_buffer_dma_no_stop_handler(HW_I2C_ID id, uint16_t mask)
{
        /* Must provide a valid (> 0) mask */
        ASSERT_WARNING(mask != 0);

        if (mask & HW_I2C_INT_TX_ABORT) {
                /* disable I2C DMA */
                IBA(id)->I2C_DMA_CR_REG = 0;

                dma_tx_reply(id, false);

                /* clear abort */
                hw_i2c_reset_int_tx_abort(id);

                return;
        }

}

/*
 * Interrupt handler used by hw_i2c_prepare_dma_ex() to handle STOP and ABORT for DMA writes
 */
static void intr_write_buffer_dma_handler(HW_I2C_ID id, uint16_t mask)
{
        struct i2c *i2c = get_i2c(id);
        struct tx_state *txs = &i2c->tx_state;

        /* Must provide a valid (> 0) mask */
        ASSERT_WARNING(mask != 0);

        if (mask & HW_I2C_INT_TX_ABORT) {
                /* disable I2C DMA */
                IBA(id)->I2C_DMA_CR_REG = 0;

                dma_tx_reply(id, false);

                /* clear abort */
                hw_i2c_reset_int_tx_abort(id);

                return;
        }

        if (mask & HW_I2C_INT_STOP_DETECTED) {

                if (IBA(id)->I2C_DMA_CR_REG != 0) {
                        hw_i2c_reset_int_stop_detected(id);
                        /*
                         * A STOP while DMA is still enabled is caused by a NACK from the slave.
                         * While servicing the STOP_DETECTED interrupt we don't need to call the
                         * reply callback. This will be done when servicing the TX_ABORT interrupt
                         * that will follow.
                         */
                        return;
                }

                dma_tx_reply(id, txs->num == txs->len);
                hw_i2c_reset_int_stop_detected(id);
                return;
        }

        /*
         */
}

static void intr_read_buffer_handler(HW_I2C_ID id, uint16_t mask)
{
        struct i2c *i2c = get_i2c(id);
        struct rx_state *rxs = &i2c->rx_state;
        struct tx_state *txs = &i2c->tx_state;

        if (mask & HW_I2C_INT_TX_ABORT) {
                rx_reply(id, false);

                /* clear abort */
                hw_i2c_reset_int_tx_abort(id);

                return;
        }

        if (!rxs->data || !(mask & (HW_I2C_INT_RX_FULL | HW_I2C_INT_TX_EMPTY |
                                        HW_I2C_INT_STOP_DETECTED | HW_I2C_INT_READ_REQUEST))) {
                return;
        }

        while (txs->num < txs->len && hw_i2c_is_tx_fifo_not_full(id)) {
                hw_i2c_write_byte(id, txs->data[txs->num]);
                txs->num++;
        }

        if (txs->num < txs->len) {
                return;
        }

        while ((rxs->rr < rxs->len) && hw_i2c_is_tx_fifo_not_full(id)) {
                rxs->rr++;
#if BLACK_ORCA_IC_REV_A == dg_configBLACK_ORCA_IC_REV
                hw_i2c_read_byte_trigger(id);
#else
                if (rxs->rr == rxs->len && (rxs->flags & HW_I2C_F_ADD_STOP)) {
                        /* Add STOP to read request for last byte */
                        IBA(id)->I2C_DATA_CMD_REG = I2C_I2C_DATA_CMD_REG_CMD_Msk |
                                                                I2C_I2C_DATA_CMD_REG_STOP_Msk;
                } else {
                        hw_i2c_read_byte_trigger(id);
                }
#endif
        }

        while (hw_i2c_get_rx_fifo_level(id) && rxs->num < rxs->len) {
                rxs->data[rxs->num] = hw_i2c_read_byte(id);
                rxs->num++;
        }

        if ((rxs->num == rxs->len) || (mask & (HW_I2C_INT_STOP_DETECTED | HW_I2C_INT_READ_REQUEST))) {
                rx_reply(id, true);
                return;
        }

        if (rxs->rr < rxs->len) {
                return;
        }

        if (mask & HW_I2C_INT_TX_EMPTY) {
                hw_i2c_set_int_mask(id, hw_i2c_get_int_mask(id) & ~(HW_I2C_INT_TX_EMPTY));
        }
}

static void intr_slave_handler(HW_I2C_ID id, uint16_t mask);

static void intr_read_buffer_dma_handler(HW_I2C_ID id, uint16_t mask)
{
        struct i2c *i2c = get_i2c(id);

        if (mask & HW_I2C_INT_TX_ABORT) {
                hw_dma_channel_stop(i2c->dma_state.channel);

                /* clear abort */
                hw_i2c_reset_int_tx_abort(id);

                return;
        }

        if (mask & (HW_I2C_INT_STOP_DETECTED | HW_I2C_INT_READ_REQUEST)) {
                hw_i2c_reset_int_stop_detected(id);
                hw_i2c_reset_int_read_request(id);
                hw_i2c_unregister_int(id);
                hw_dma_channel_stop(i2c->dma_state.channel);
                bool master = HW_I2C_REG_GETF(id, I2C_CON, I2C_MASTER_MODE);
                if (!master) {
                        /* We need to handle the read request that stopped the master tx. */
                        intr_slave_handler(id, mask);
                }
                return;
        }
}

static void intr_slave_handler(HW_I2C_ID id, uint16_t mask)
{
        struct i2c *i2c = get_i2c(id);
        hw_i2c_event_cb cb = i2c->event_cb;

        if (mask & HW_I2C_INT_READ_REQUEST) {
                if (cb) {
                        cb(id, HW_I2C_EVENT_READ_REQUEST);
                }

                hw_i2c_reset_int_read_request(id);
        }

        if (mask & HW_I2C_INT_RX_FULL) {
                if (cb) {
                        cb(id, HW_I2C_EVENT_DATA_READY);
                }
        }

        if (mask & HW_I2C_INT_TX_ABORT) {
                if (cb) {
                        cb(id, HW_I2C_EVENT_TX_ABORT);
                }

                /* clear abort */
                hw_i2c_reset_int_tx_abort(id);
        }

        if (mask & HW_I2C_INT_RX_OVERFLOW) {
                if (cb) {
                        cb(id, HW_I2C_EVENT_RX_OVERFLOW);
                }

                hw_i2c_reset_int_rx_overflow(id);
        }
}

static inline void intr_handler(HW_I2C_ID id, uint16_t mask)
{
        struct i2c *i2c = get_i2c(id);

        if (i2c->intr_cb) {
                i2c->intr_cb(id, mask);
        }
}

void I2C_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        uint16_t mask = I2C->I2C_INTR_STAT_REG;

        intr_handler(HW_I2C1, mask);

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

void I2C2_Handler(void)
{
        SEGGER_SYSTEMVIEW_ISR_ENTER();

        uint16_t mask = I2C2->I2C2_INTR_STAT_REG;

        intr_handler(HW_I2C2, mask);

        SEGGER_SYSTEMVIEW_ISR_EXIT();
}

#endif /* dg_configUSE_HW_I2C */
/**
 * \}
 * \}
 * \}
 */
