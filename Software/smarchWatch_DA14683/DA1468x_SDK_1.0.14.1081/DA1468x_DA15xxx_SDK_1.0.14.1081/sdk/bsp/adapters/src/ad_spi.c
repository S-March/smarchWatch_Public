/**
 * \addtogroup BSP
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup SPI
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_spi.c
 *
 * @brief SPI adapter implementation
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configSPI_ADAPTER

#include <stdint.h>
#include "sdk_defs.h"
#include "interrupts.h"
#include <sys_power_mgr.h>

#if CONFIG_SPI_RESOURCE_STATIC_ID
#define STATIC_RES_ID(name) .dev_res_id = RES_ID_DEVICE_##name
#else
#define STATIC_RES_ID(name)
#endif

#define SPI_SLAVE_DEVICE(bus, name, cs_port, cs_pin, _word_mode, pol_mode, _phase_mode, xtal_div, dma_channel) \
        __RETAINED spi_dev_dynamic_data dev_data_##name; \
        const spi_device_config dev_##name = { \
                .bus_id = HW_##bus, \
                .bus_res_id = RES_ID_##bus, \
                .hw_init.smn_role = HW_SPI_MODE_MASTER, \
                .hw_init.cs_pad = { cs_port, cs_pin }, \
                .hw_init.word_mode = _word_mode, \
                .hw_init.polarity_mode = pol_mode, \
                .hw_init.phase_mode = _phase_mode, \
                .hw_init.xtal_freq = xtal_div, \
                .hw_init.fifo_mode = HW_SPI_FIFO_RX_TX, \
                .hw_init.disabled = 0, \
                .hw_init.use_dma = dma_channel >= 0, \
                .hw_init.rx_dma_channel = dma_channel, \
                .hw_init.tx_dma_channel = dma_channel + 1, \
                .bus_data = &dynamic_##bus, \
                .data = &dev_data_##name, \
                STATIC_RES_ID(name) \
        }; \
        const void *const name = &dev_##name; \
        DEVICE_INIT(_##name, ad_spi_device_init, &dev_##name);

#define SPI_SLAVE_TO_EXT_MASTER(bus, name, _ignore_cs, _word_mode, pol_mode, _phase_mode, dma_channel) \
        __RETAINED spi_dev_dynamic_data dev_data_##name; \
        const spi_device_config dev_##name = { \
                .bus_id = HW_##bus, \
                .bus_res_id = RES_ID_##bus, \
                .hw_init.smn_role = HW_SPI_MODE_SLAVE, \
                .hw_init.cs_pad = { 0, 0 }, \
                .hw_init.word_mode = _word_mode, \
                .hw_init.polarity_mode = pol_mode, \
                .hw_init.phase_mode = _phase_mode, \
                .hw_init.xtal_freq = 0, \
                .hw_init.fifo_mode = HW_SPI_FIFO_RX_TX, \
                .hw_init.disabled = 0, \
                .hw_init.use_dma = dma_channel >= 0, \
                .hw_init.rx_dma_channel = dma_channel, \
                .hw_init.tx_dma_channel = dma_channel + 1, \
                .bus_data = &dynamic_##bus, \
                .data = &dev_data_##name, \
                .ignore_cs = _ignore_cs, \
                STATIC_RES_ID(name) \
        }; \
        const void *const name = &dev_##name; \
        DEVICE_INIT(_##name, ad_spi_device_init, &dev_##name);

#define SPI_BUS(bus_id) \
        __RETAINED spi_bus_dynamic_data dynamic_##bus_id; \
        BUS_INIT(spi_bus_##bus_id, ad_spi_bus_init, &(dynamic_##bus_id));

#define SPI_BUS_END

#include <stdarg.h>
#include <platform_devices.h>
#include <hw_spi.h>
#include <ad_spi.h>
#include <resmgmt.h>

/*
 * When CONFIG_SPI_RESOURCE_STATIC_ID each SPI device has unique resource ID, as compile-time
 * constant. When it is not defined, resource Id will be stored in retained data (non-constant).
 */
#if CONFIG_SPI_RESOURCE_STATIC_ID
#define DEVICE_RESOURCE_MASK(config) (RES_MASK(((spi_device_config *) config)->dev_res_id))
#else
#define DEVICE_RESOURCE_MASK(config) (RES_MASK(((spi_device_config *) config)->data->dev_res_id))
#endif

/*
 * Array to hold current device for each SPI.
 */
__RETAINED static const spi_device_config *current_config[2];

void ad_spi_bus_init(void *bus_data)
{
        spi_bus_dynamic_data *data = bus_data;
        OS_EVENT_CREATE(data->event);
}

void ad_spi_device_init(const spi_device_id id)
{
#if !CONFIG_SPI_RESOURCE_STATIC_ID && CONFIG_SPI_USE_RESMGMT
        const spi_device_config *config = (const spi_device_config *) id;
        config->data->dev_res_id = resource_add();
#endif
}

/*
 * Apply device specific configuration of SPI controller
 */
static void ad_spi_bus_apply_config(spi_device_config *device)
{
        const HW_SPI_ID id = ad_spi_get_hw_spi_id(device);

        current_config[id == HW_SPI1 ? 0 : 1] = device;
#if !CONFIG_SPI_ONE_DEVICE_ON_BUS
        device->bus_data->current_device = device;
#endif

        hw_spi_init(id, &device->hw_init);

        hw_spi_set_cs_ctrl(id, !device->ignore_cs);
}

#if CONFIG_SPI_ONE_DEVICE_ON_BUS
static void ad_spi_bus_reset_config(const spi_device_config *device)
{
        const HW_SPI_ID id = ad_spi_get_hw_spi_id((spi_device) device);

        current_config[id == HW_SPI1 ? 0 : 1] = NULL;
}
#endif

#if CONFIG_SPI_USE_DMA_RESMGMT
static inline uint32_t dma_resource_mask(int num)
{
        static const uint32_t res_mask[] = {
                RES_MASK(RES_ID_DMA_CH0), RES_MASK(RES_ID_DMA_CH1),
                RES_MASK(RES_ID_DMA_CH2), RES_MASK(RES_ID_DMA_CH3),
                RES_MASK(RES_ID_DMA_CH4), RES_MASK(RES_ID_DMA_CH5),
                RES_MASK(RES_ID_DMA_CH6), RES_MASK(RES_ID_DMA_CH7)
        };
        return res_mask[num];
}
#endif

void ad_spi_bus_acquire(spi_device dev)
{
        spi_device_config *device = (spi_device_config *) dev;

#if CONFIG_SPI_USE_RESMGMT
#if !CONFIG_SPI_EXCLUSIVE_OPEN
        /* The device must be already acquired before acquiring the bus */
        OS_ASSERT(device->data->owner == OS_GET_CURRENT_TASK());
#endif
#endif

#if !CONFIG_SPI_ONE_DEVICE_ON_BUS
#if CONFIG_SPI_USE_RESMGMT
        if (device->data->bus_acquire_count++ == 0) {

                resource_acquire(RES_MASK(device->bus_res_id), RES_WAIT_FOREVER);
#else
        {
#endif
                if (device->bus_data->current_device != device) {
                        ad_spi_bus_apply_config(device);
                }
#if CONFIG_SPI_USE_DMA_RESMGMT
                if (device->hw_init.use_dma) {
                        resource_acquire(dma_resource_mask(device->hw_init.rx_dma_channel) |
                                dma_resource_mask(device->hw_init.tx_dma_channel),
                                RES_WAIT_FOREVER);
                }
#endif
        }
#endif
}

void ad_spi_bus_activate_cs(spi_device dev)
{
        spi_device_config *device = (spi_device_config *) dev;
        const HW_SPI_ID id = device->bus_id;

#if !CONFIG_SPI_ONE_DEVICE_ON_BUS
        /* The device must own the bus (and the task must own the device) before toggling CS */
        OS_ASSERT(device->bus_data->current_device == device);
#endif

        if (!hw_spi_is_slave(id)) {
                hw_spi_set_cs_low(id);
        }
}

void ad_spi_bus_release(spi_device dev)
{

#if !CONFIG_SPI_ONE_DEVICE_ON_BUS
        const spi_device_config *device = (spi_device_config *) dev;
#if CONFIG_SPI_USE_RESMGMT
        if (--device->data->bus_acquire_count == 0) {
#else
        {
#endif
#if CONFIG_SPI_USE_DMA_RESMGMT
                if (device->hw_init.use_dma) {
                        resource_release(dma_resource_mask(device->hw_init.rx_dma_channel) |
                                dma_resource_mask(device->hw_init.tx_dma_channel));
                }
#endif
#if CONFIG_SPI_USE_RESMGMT
                resource_release(RES_MASK(device->bus_res_id));
#endif
        }
#endif
}

void ad_spi_bus_deactivate_cs(spi_device dev)
{
        spi_device_config *device = (spi_device_config *) dev;
        const HW_SPI_ID id = device->bus_id;


#if !CONFIG_SPI_ONE_DEVICE_ON_BUS
        /* The device must own the bus (and the task must own the device) before toggling CS */
        OS_ASSERT(device->bus_data->current_device == device);
#endif

        if (!hw_spi_is_slave(id)) {
                hw_spi_set_cs_high(id);
        }
}

void ad_spi_device_acquire(spi_device dev)
{
#if CONFIG_SPI_USE_RESMGMT
#if !CONFIG_SPI_EXCLUSIVE_OPEN
        spi_dev_dynamic_data *data =  ((spi_device_config *) dev)->data;
        OS_TASK current_task = OS_GET_CURRENT_TASK();

        if (current_task == data->owner) {
                data->dev_acquire_count++;
                return;
        }
        resource_acquire(DEVICE_RESOURCE_MASK(dev), OS_EVENT_FOREVER);
        data->owner = current_task;
        data->dev_acquire_count++;
#endif
#endif /* CONFIG_SPI_USE_RESMGMT */
}

void ad_spi_device_release(spi_device dev)
{
#if CONFIG_SPI_USE_RESMGMT
#if !CONFIG_SPI_EXCLUSIVE_OPEN
        spi_device_config *config =  (spi_device_config *) dev;

        /* A device release can only happen from the same task that owns it, or from an ISR */
        OS_ASSERT(in_interrupt() || (OS_GET_CURRENT_TASK() == config->data->owner));

        if (--config->data->dev_acquire_count == 0) {
                config->data->owner = NULL;
                resource_release(DEVICE_RESOURCE_MASK(dev));
        }
#endif
#endif /* CONFIG_SPI_USE_RESMGMT */
}

spi_device ad_spi_open(const spi_device_id dev_id)
{
        spi_device_config *device = (spi_device_config *) dev_id;

#if CONFIG_SPI_USE_RESMGMT
#if CONFIG_SPI_EXCLUSIVE_OPEN
        resource_acquire(DEVICE_RESOURCE_MASK(device), OS_EVENT_FOREVER);
#endif
#endif
        OS_ENTER_CRITICAL_SECTION();
        if (++device->bus_data->open_devices_cnt == 1) {
                hw_spi_enable(ad_spi_get_hw_spi_id(device), 1);
        }
        OS_LEAVE_CRITICAL_SECTION();

#if CONFIG_SPI_ONE_DEVICE_ON_BUS
        /*
         * If there is at most one device on each bus, there is no need to check
         * configuration each time before accessing bus. In this case configuration
         * will be applied in open and never changed.
         */
        ad_spi_bus_apply_config(device);
#if CONFIG_SPI_USE_DMA_RESMGMT
        if (device->hw_init.use_dma) {
                resource_acquire(dma_resource_mask(device->hw_init.rx_dma_channel) |
                        dma_resource_mask(device->hw_init.tx_dma_channel),
                        RES_WAIT_FOREVER);
        }
#endif
#endif

        return (spi_device) device;
}

void ad_spi_close(spi_device dev)
{
        spi_device_config *device = (spi_device_config *) dev;

#if CONFIG_SPI_ONE_DEVICE_ON_BUS
#if CONFIG_SPI_USE_DMA_RESMGMT
        if (device->hw_init.use_dma) {
                resource_release(dma_resource_mask(device->hw_init.rx_dma_channel) |
                        dma_resource_mask(device->hw_init.tx_dma_channel));
        }
#endif
        ad_spi_bus_reset_config(device);
#endif

        OS_ENTER_CRITICAL_SECTION();
        OS_ASSERT(device->bus_data->open_devices_cnt);
        if (--device->bus_data->open_devices_cnt == 0) {
                /* If there are not more opened device, disable hardware block to save power */
                hw_spi_enable(ad_spi_get_hw_spi_id(dev), 0);
        }
        OS_LEAVE_CRITICAL_SECTION();

#if CONFIG_SPI_USE_RESMGMT
#if CONFIG_SPI_EXCLUSIVE_OPEN
        resource_release(DEVICE_RESOURCE_MASK(device));
#endif
#endif
}

HW_SPI_ID ad_spi_get_hw_spi_id(spi_device dev)
{
        spi_device_config *device = (spi_device_config *) dev;

        return device->bus_id;
}

static void ad_spi_wait_event(void *p, uint16_t transferred)
{
        spi_device_config *device = (spi_device_config *) p;
        OS_EVENT_SIGNAL_FROM_ISR(device->bus_data->event);
}

void ad_spi_complex_transact(spi_device dev, spi_transfer_data *transfers, size_t count)
{
        spi_device_config *device = (spi_device_config *) dev;
        const HW_SPI_ID id = device->bus_id;
        size_t i;

        ad_spi_device_acquire(dev);
        ad_spi_bus_acquire(dev);

        ad_spi_bus_activate_cs(dev);

        for (i = 0; i < count; ++i) {
                if (transfers[i].wbuf) {
                        if (transfers[i].rbuf) {
                                hw_spi_writeread_buf(id, transfers[i].wbuf, transfers[i].rbuf,
                                                transfers[i].length, ad_spi_wait_event, device);
                        } else {
                                hw_spi_write_buf(id, transfers[i].wbuf, transfers[i].length,
                                                                        ad_spi_wait_event, device);
                        }
                } else {
                        hw_spi_read_buf(id, transfers[i].rbuf, transfers[i].length,
                                                                        ad_spi_wait_event, device);
                }
                OS_EVENT_WAIT(device->bus_data->event, OS_EVENT_FOREVER);
        }

        ad_spi_bus_deactivate_cs(dev);

        ad_spi_bus_release(dev);
        ad_spi_device_release(dev);
}

void ad_spi_transact(spi_device dev, const uint8_t *wbuf, size_t wlen, uint8_t *rbuf,
                                                                                        size_t rlen)
{
        spi_transfer_data transfers[2] = {
                { wbuf, NULL, wlen },
                { NULL, rbuf, rlen }
        };

        ad_spi_complex_transact(dev, transfers, 2);
}

void ad_spi_write(spi_device dev, const uint8_t *wbuf, size_t wlen)
{
        spi_device_config *device;
#if SPI_TIMING_DEBUG == 1
        GLOBAL_INT_DISABLE();
        DBG_CONFIGURE_HIGH(SPI_TIMING_DEBUG, SPI_TIMING_DEBUG_WRITE);
#endif
        device = (spi_device_config *) dev;
        const HW_SPI_ID id = device->bus_id;

        ad_spi_device_acquire(dev);
        ad_spi_bus_acquire(dev);

        ad_spi_bus_activate_cs(dev);

        hw_spi_write_buf(id, wbuf, wlen, ad_spi_wait_event, device);
#if SPI_TIMING_DEBUG == 1
        DBG_SET_LOW(SPI_TIMING_DEBUG, SPI_TIMING_DEBUG_WRITE);
        GLOBAL_INT_RESTORE();
#endif
        OS_EVENT_WAIT(device->bus_data->event, OS_EVENT_FOREVER);
#if SPI_TIMING_DEBUG == 1
        GLOBAL_INT_DISABLE();
        DBG_CONFIGURE_HIGH(SPI_TIMING_DEBUG, SPI_TIMING_DEBUG_WRITE);
#endif
        ad_spi_bus_deactivate_cs(dev);

        ad_spi_bus_release(dev);
        ad_spi_device_release(dev);
#if SPI_TIMING_DEBUG == 1
        DBG_SET_LOW(SPI_TIMING_DEBUG, SPI_TIMING_DEBUG_WRITE);
        GLOBAL_INT_RESTORE();
#endif
}

void ad_spi_read(spi_device dev, uint8_t *rbuf, size_t rlen)
{
        spi_device_config *device;
#if SPI_TIMING_DEBUG == 1
        GLOBAL_INT_DISABLE();
        DBG_CONFIGURE_HIGH(SPI_TIMING_DEBUG, SPI_TIMING_DEBUG_READ);
#endif
        device = (spi_device_config *) dev;
        const HW_SPI_ID id = device->bus_id;

        ad_spi_device_acquire(dev);
        ad_spi_bus_acquire(dev);

        ad_spi_bus_activate_cs(dev);

        hw_spi_read_buf(id, rbuf, rlen, ad_spi_wait_event, device);

#if SPI_TIMING_DEBUG == 1
        DBG_SET_LOW(SPI_TIMING_DEBUG, SPI_TIMING_DEBUG_READ);
        GLOBAL_INT_RESTORE();
#endif
        OS_EVENT_WAIT(device->bus_data->event, OS_EVENT_FOREVER);
#if SPI_TIMING_DEBUG == 1
        GLOBAL_INT_DISABLE();
        DBG_CONFIGURE_HIGH(SPI_TIMING_DEBUG, SPI_TIMING_DEBUG_READ);
#endif
        ad_spi_bus_deactivate_cs(dev);

        ad_spi_bus_release(dev);
        ad_spi_device_release(dev);
#if SPI_TIMING_DEBUG == 1
        DBG_SET_LOW(SPI_TIMING_DEBUG, SPI_TIMING_DEBUG_READ);
        GLOBAL_INT_RESTORE();
#endif
}

#if CONFIG_SPI_USE_ASYNC_TRANSACTIONS

static void ad_spi_async_do(spi_device dev);

/*
 * Callback passed to low lever driver, invoked after each transmission.
 */
static void ad_spi_cb(void *user_data, uint16_t transferred)
{
        ad_spi_async_do((spi_device) user_data);
}

/*
 * This function executes as many action as possible.
 */
static void ad_spi_async_do(spi_device dev)
{
        spi_device_config *device = (spi_device_config *) dev;
        spi_bus_dynamic_data *data = device->bus_data;
        uint16_t len;
        const uint8_t *wbuf;
        uint8_t *rbuf;
        ad_spi_user_cb cb;

        while (true) {
                switch (data->transaction[data->transaction_ix] & 0xFF000000) {
                case SPI_TAG_CS_ACTIVATE:
                        /* activate chip select and move to next action */
                        ad_spi_bus_activate_cs(dev);
                        data->transaction_ix++;
                        break;
                case SPI_TAG_CS_DEACTIVATE:
                        /* de-activate chip select and move to next action */
                        ad_spi_bus_deactivate_cs(dev);
                        data->transaction_ix++;
                        break;
                case SPI_TAG_SEND_RECEIVE:
                        /* start write-read operation, next action execute form interrupt */
                        len = (uint16_t) data->transaction[data->transaction_ix++];
                        wbuf = (const uint8_t *) data->transaction[data->transaction_ix++];
                        rbuf = (uint8_t *) data->transaction[data->transaction_ix++];
                        hw_spi_writeread_buf(device->bus_id, wbuf, rbuf, len, ad_spi_cb, device);
                        return;
                case SPI_TAG_SEND:
                        /* start write operation, next action execute form interrupt */
                        len = (uint16_t) data->transaction[data->transaction_ix++];
                        wbuf = (const uint8_t *) data->transaction[data->transaction_ix++];
                        hw_spi_write_buf(device->bus_id, wbuf, len, ad_spi_cb, device);
                        return;
                case SPI_TAG_RECEIVE:
                        /* start read operation, next action execute form interrupt */
                        len = (uint16_t) data->transaction[data->transaction_ix++];
                        rbuf = (uint8_t *) data->transaction[data->transaction_ix++];
                        hw_spi_read_buf(device->bus_id, rbuf, len, ad_spi_cb, device);
                        return;
                case SPI_TAG_CALLBACK0:
                        /* skip tag */
                        data->transaction_ix++;

                        cb = (ad_spi_user_cb ) data->transaction[data->transaction_ix++];
                        /* If next action is SPI_END callback should be called after bus and device
                         * are released.
                         */
                        if (data->transaction[data->transaction_ix] == SPI_END) {
                                ad_spi_bus_release(dev);
                                ad_spi_device_release(dev);
                                cb(NULL);
                                return;
                        } else {
                                /* In the middle callback not need to release yet */
                                cb(NULL);
                        }
                        break;
                case SPI_TAG_CALLBACK1:
                        /* skip tag */
                        data->transaction_ix++;

                        cb = (ad_spi_user_cb ) data->transaction[data->transaction_ix++];
                        /* Take callback user data */
                        rbuf = (uint8_t *) data->transaction[data->transaction_ix++];
                        /* If next action is SPI_END callback should be called after bus and device
                         * are released.
                         */
                        if (data->transaction[data->transaction_ix] == SPI_END) {
                                ad_spi_bus_release(dev);
                                ad_spi_device_release(dev);
                                cb(rbuf);
                                return;
                        } else {
                                cb(rbuf);
                        }
                        break;
                case SPI_END:
                        /* Just release bus and device */
                        ad_spi_bus_release(dev);
                        ad_spi_device_release(dev);
                        return;
                }
        }
}

void ad_spi_async_transact(spi_device dev, ...)
{
        spi_device_config *device = (spi_device_config *) dev;
        spi_bus_dynamic_data *data = device->bus_data;
        va_list ap;
        int i = 0;
        uint32_t action;

        va_start(ap, dev);

        /*
         * Acquire device and bus.
         * The will be released at the end of transaction from interrupt context.
         */
        ad_spi_device_acquire(dev);
        ad_spi_bus_acquire(dev);

        /*
         * Put arguments in transaction action buffer, actions will be taken from it later when
         * needed.
         */
        do {
                action = va_arg(ap, uint32);
                data->transaction[i++] = action;
                switch (action & 0xFF000000) {
                case SPI_TAG_SEND_RECEIVE:
                        data->transaction[i++] = va_arg(ap, uint32);
                        /* no break */
                case SPI_TAG_CALLBACK0:
                case SPI_TAG_SEND:
                case SPI_TAG_RECEIVE:
                        data->transaction[i++] = va_arg(ap, uint32);
                        break;
                case SPI_TAG_CALLBACK1:
                        data->transaction[i++] = va_arg(ap, uint32);
                        data->transaction[i++] = va_arg(ap, uint32);
                        break;
                case SPI_CSA:
                case SPI_CSD:
                case SPI_END:
                        break;
                default:
                        OS_ASSERT(0);
                }
        } while (action != SPI_END);
        /*
         * If this assert fails increase SPI_ASYNC_ACTIONS_SIZE to fit specific application needs.
         * It takes memory even if not in use.
         */
        OS_ASSERT(i <= SPI_ASYNC_ACTIONS_SIZE);
        va_end(ap);

        data->transaction_ix = 0;

        /*
         * Start executing transaction.
         */
        ad_spi_async_do(dev);
}
#endif

static bool ad_spi_prepare_for_sleep(void)
{
        if (hw_spi_is_busy(HW_SPI1) || hw_spi_is_busy(HW_SPI2)) {
                return false;
        }
#if !CONFIG_SPI_ONE_DEVICE_ON_BUS
#if CONFIG_SPI_USE_RESMGMT
        /* Do not sleep when current device exist and is used */
        if (current_config[0] && current_config[0]->data->bus_acquire_count != 0) {
                return false;
        }
        if (current_config[1] && current_config[1]->data->bus_acquire_count != 0) {
                return false;
        }
#endif

        /* When device exists and is not used reset current device */
        if (current_config[0]) {
                current_config[0]->bus_data->current_device = NULL;
        }
        if (current_config[1]) {
                current_config[1]->bus_data->current_device = NULL;
        }
#endif

        return true;
}

static void ad_spi_sleep_canceled(void)
{
}

static void ad_spi_wake_up_ind(bool arg)
{
#if CONFIG_SPI_ONE_DEVICE_ON_BUS
        if (current_config[0]) {
                ad_spi_bus_apply_config((spi_device_config *) current_config[0]);
        }
        if (current_config[1]) {
                ad_spi_bus_apply_config((spi_device_config *) current_config[1]);
        }
#endif
}

const adapter_call_backs_t ad_spi_pm_call_backs = {
        .ad_prepare_for_sleep = ad_spi_prepare_for_sleep,
        .ad_sleep_canceled = ad_spi_sleep_canceled,
        .ad_wake_up_ind = ad_spi_wake_up_ind,
        .ad_xtal16m_ready_ind = NULL,
        .ad_sleep_preparation_time = 0
};

void ad_spi_init(void)
{
        pm_register_adapter(&ad_spi_pm_call_backs);
}

ADAPTER_INIT(ad_spi_adapter, ad_spi_init);

#endif /* dg_configSPI_ADAPTER */

/**
 * \}
 * \}
 * \}
 */
