/**
 * \addtogroup BSP
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup I2C
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_i2c.c
 *
 * @brief I2C device access API implementation
 *
 * Copyright (C) 2015-2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configI2C_ADAPTER

#ifndef I2C_DEFAULT_CLK_CFG
#define I2C_DEFAULT_CLK_CFG .hw_init.clock_cfg = { 0, 0, 0, 0 }
#endif

#define hw_master HW_I2C_MODE_MASTER
#define hw_slave  HW_I2C_MODE_SLAVE
#define i2c_dev_dynamic_data_master i2c_dev_dynamic_data
/*
 * Create local version of I2C bus building block macros before inclusion of platform_devices.h
 * This allows to create private variables used internally by i2c adapter implementation.
 */
#define I2C_DEVICE_DEF(bus_id, bus_res_id, dynamic_data, name, _mode, _address, _addr_mode, \
                                                                        _speed, _dma_channel) \
        __RETAINED i2c_dev_dynamic_data_##_mode dev_data_##name; \
        const i2c_device_config dev_##name = { \
                (bus_id), \
                (bus_res_id), \
                I2C_DEFAULT_CLK_CFG, \
                .hw_init.speed = (_speed), \
                .hw_init.mode = (hw_##_mode), \
                .hw_init.addr_mode = (_addr_mode), \
                .hw_init.address = (_address), \
                .hw_init.event_cb = NULL, \
                .dma_channel = (_dma_channel), \
                .bus_data = &(dynamic_data), \
                .data = (i2c_dev_dynamic_data *) &dev_data_##name \
        }; \
        const void *const name = &dev_##name; \
        DEVICE_INIT(_##name, ad_i2c_device_init, &dev_##name);

#define I2C_SLAVE_DEVICE(bus, name, _address, _addr_mode, _speed) \
        I2C_DEVICE_DEF(HW_##bus, RES_ID_##bus, dynamic_##bus, name, master,\
                                                        (_address), (_addr_mode), (_speed), -1)

#define I2C_SLAVE_DEVICE_DMA(bus, name, _address, _addr_mode, _speed, _dma_channel) \
        I2C_DEVICE_DEF(HW_##bus, RES_ID_##bus, dynamic_##bus, name, master,\
                                                (_address), (_addr_mode), (_speed), (_dma_channel))

#define I2C_SLAVE_TO_EXT_MASTER(bus, name, _address, _addr_mode, _speed, _dma_channel) \
        I2C_DEVICE_DEF(HW_##bus, RES_ID_##bus, dynamic_##bus, name, slave,\
                                                        (_address), (_addr_mode), (_speed), (_dma_channel))

#include <stdarg.h>
#include <hw_i2c.h>
#include <hw_dma.h>
#include <resmgmt.h>
#include <interrupts.h>
#include <sys_power_mgr.h>

#define I2C_BUS(bus_id) \
        __RETAINED i2c_bus_dynamic_data dynamic_##bus_id; \
        BUS_INIT(i2c_bus_##bus_id, ad_i2c_bus_init, &(dynamic_##bus_id));

#define I2C_BUS_END

#include <platform_devices.h>
#include <ad_i2c.h>

#if CONFIG_I2C_RESOURCE_STATIC_ID
#define DEVICE_RESOURCE_MASK(device) (RES_MASK(((i2c_device_config *) device)->dev_res_id))
#else
#define DEVICE_RESOURCE_MASK(device) (RES_MASK(((i2c_device_config *) device)->data->dev_res_id))
#endif

/*
 * Array to hold current device for each I2C.
 */
__RETAINED static const i2c_device_config *current_config[2];

void ad_i2c_bus_init(void *bus_data)
{
        i2c_bus_dynamic_data *data = bus_data;
        OS_EVENT_CREATE(data->event);
}

void ad_i2c_device_init(const i2c_device_id id)
{
#if !CONFIG_I2C_RESOURCE_STATIC_ID
        const i2c_device_config *config = (const i2c_device_config *) id;
#if CONFIG_I2C_USE_RESMGMT
        config->data->dev_res_id = resource_add();
#endif
#endif
}

HW_I2C_ID ad_i2c_get_hw_i2c_id(i2c_device dev)
{
        i2c_device_config *device = (i2c_device_config *) dev;

        return device->bus_id;
}

i2c_device ad_i2c_get_device_by_hw_id(HW_I2C_ID id)
{
        return (i2c_device) current_config[id == HW_I2C1 ? 0 : 1];
}

static void ad_i2c_bus_apply_config(i2c_device_config *device)
{
        const HW_I2C_ID id = ad_i2c_get_hw_i2c_id(device);

        current_config[id == HW_I2C1 ? 0 : 1] = device;
#if !CONFIG_I2C_ONE_DEVICE_ON_BUS
        device->bus_data->current_device = device;
#endif

        hw_i2c_init(id, &device->hw_init);

#if dg_configI2C_ADAPTER_SLAVE_SUPPORT
        if (device->hw_init.mode == HW_I2C_MODE_SLAVE) {
                i2c_dev_dynamic_data_slave *slave = (i2c_dev_dynamic_data_slave *) device->data;

                if (!(slave->state & AD_I2C_SLAVE_STATE_INIT)) {
                        goto after_enabling;
                }
        }
        hw_i2c_enable(device->bus_id);
after_enabling:
#else
        hw_i2c_enable(device->bus_id);
#endif
        hw_i2c_reset_abort_source(device->bus_id);
        hw_i2c_reset_int_all(device->bus_id);
}

#if CONFIG_I2C_ONE_DEVICE_ON_BUS
static void ad_i2c_bus_reset_config(i2c_device_config *device)
{
        const HW_I2C_ID id = ad_i2c_get_hw_i2c_id(device);

        current_config[id == HW_I2C1 ? 0 : 1] = NULL;
}
#endif

#if CONFIG_I2C_USE_DMA_RESMGMT
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
#endif /* CONFIG_I2C_USE_DMA_RESMGMT */

void ad_i2c_bus_acquire(i2c_device dev)
{
        i2c_device_config *device = (i2c_device_config *) dev;
#if CONFIG_I2C_USE_RESMGMT
#if !CONFIG_I2C_EXCLUSIVE_OPEN
        /* The device must be already acquired before acquiring the bus */
        OS_ASSERT(device->data->owner == OS_GET_CURRENT_TASK());
#endif
#endif

#if !CONFIG_I2C_ONE_DEVICE_ON_BUS
#if CONFIG_I2C_USE_RESMGMT
        if (device->data->bus_acquire_count++ == 0) {
                resource_acquire(RES_MASK(device->bus_res_id), RES_WAIT_FOREVER);
#else
        {
#endif
                if ((i2c_device_config *) (device->bus_data->current_device) != device) {
                        ad_i2c_bus_apply_config(device);
                }

#if CONFIG_I2C_USE_DMA_RESMGMT
                if (device->dma_channel >= 0) {
                        resource_acquire(dma_resource_mask(device->dma_channel) |
                                dma_resource_mask(device->dma_channel + 1), RES_WAIT_FOREVER);
                }
#endif
        }
#endif
}

void ad_i2c_bus_release(i2c_device dev)
{
#if !CONFIG_I2C_ONE_DEVICE_ON_BUS
        const i2c_device_config *device = (i2c_device_config *) dev;

#if CONFIG_I2C_USE_RESMGMT
        if (--device->data->bus_acquire_count == 0) {
#else
        {
#endif
#if CONFIG_I2C_USE_DMA_RESMGMT
                if (device->dma_channel >= 0) {
                        resource_release(dma_resource_mask(device->dma_channel) |
                                dma_resource_mask(device->dma_channel + 1));
                }
#endif
#if CONFIG_I2C_USE_RESMGMT
                resource_release(RES_MASK(device->bus_res_id));
#endif
        }
#endif
}

typedef struct {
        i2c_device_config *config;
        uint16_t abort_source;
        bool success;
} i2c_cb_data;

static void ad_i2c_transaction_cb(HW_I2C_ID id, void *cb_data, uint16_t len, bool success)
{
        i2c_cb_data *data = (i2c_cb_data *) cb_data;
        const i2c_device_config *device = data->config;
        uint16_t abort_source = hw_i2c_get_abort_source(id);

        if (!success && (abort_source == HW_I2C_ABORT_NONE)) {
                abort_source = HW_I2C_ABORT_SW_ERROR;
        }
        data->success = success;
        data->abort_source = abort_source;
        OS_EVENT_SIGNAL_FROM_ISR(device->bus_data->event);
}

int ad_i2c_transact(i2c_device dev, const uint8_t *wbuf, size_t wlen, uint8_t *rbuf, size_t rlen)
{
        i2c_device_config *dev_config = (i2c_device_config *)dev;
        i2c_cb_data transaction_data;
#if CONFIG_I2C_ENABLE_CRITICAL_SECTION
        HW_I2C_ABORT_SOURCE abort_code = HW_I2C_ABORT_NONE;
#endif

        transaction_data.config = dev_config;
        transaction_data.success = true;
        transaction_data.abort_source = HW_I2C_ABORT_NONE;

        ad_i2c_device_acquire(dev);
        ad_i2c_bus_acquire(dev);

#if CONFIG_I2C_ENABLE_CRITICAL_SECTION
        OS_ENTER_CRITICAL_SECTION();

        hw_i2c_write_buffer_sync(dev_config->bus_id, wbuf, wlen, &abort_code, HW_I2C_F_NONE);

        if (abort_code != HW_I2C_ABORT_NONE) {
                OS_LEAVE_CRITICAL_SECTION();
                ad_i2c_bus_release(dev);
                ad_i2c_device_release(dev);
                return abort_code;
        }

        if (dev_config->dma_channel < 0 || rlen <= 1) {
                hw_i2c_read_buffer_async(dev_config->bus_id,
                        rbuf, rlen, ad_i2c_transaction_cb, (void *)&transaction_data,
                        HW_I2C_F_ADD_STOP);
        }
        else {
                hw_i2c_read_buffer_dma_ex(dev_config->bus_id,
                        (uint8_t)dev_config->dma_channel, rbuf, rlen,
                        ad_i2c_transaction_cb, (void *)&transaction_data);
        }

        OS_LEAVE_CRITICAL_SECTION();
#else
        hw_i2c_write_then_read_async(dev_config->bus_id, wbuf, wlen, rbuf, rlen,
                        ad_i2c_transaction_cb, (void *) &transaction_data, HW_I2C_F_ADD_STOP);

#endif

        OS_EVENT_WAIT(dev_config->bus_data->event, OS_EVENT_FOREVER);

        ad_i2c_bus_release(dev);
        ad_i2c_device_release(dev);

        return (transaction_data.success) ? 0 : (int)transaction_data.abort_source;
}

int ad_i2c_write(i2c_device dev, const uint8_t *wbuf, size_t wlen)
{
        i2c_device_config *dev_config = (i2c_device_config *)dev;
        i2c_cb_data transaction_data;

        transaction_data.config = dev_config;
        transaction_data.success = true;
        transaction_data.abort_source = HW_I2C_ABORT_NONE;

        ad_i2c_device_acquire(dev);
        ad_i2c_bus_acquire(dev);

        hw_i2c_write_buffer_async(dev_config->bus_id,
                wbuf, wlen, ad_i2c_transaction_cb, (void *)&transaction_data,
                HW_I2C_F_WAIT_FOR_STOP | HW_I2C_F_ADD_STOP);

        OS_EVENT_WAIT(dev_config->bus_data->event, OS_EVENT_FOREVER);

        ad_i2c_bus_release(dev);
        ad_i2c_device_release(dev);

        return (transaction_data.success) ? 0 : (int)transaction_data.abort_source;
}

int ad_i2c_read(i2c_device dev, uint8_t *rbuf, size_t rlen)
{
        i2c_device_config *dev_config = (i2c_device_config *)dev;
        i2c_cb_data transaction_data;

        transaction_data.config = dev_config;
        transaction_data.success = true;
        transaction_data.abort_source = HW_I2C_ABORT_NONE;

        ad_i2c_device_acquire(dev);
        ad_i2c_bus_acquire(dev);

        if (dev_config->dma_channel < 0 || rlen <= 1) {
                hw_i2c_read_buffer_async(dev_config->bus_id,
                        rbuf, rlen, ad_i2c_transaction_cb, (void *)&transaction_data,
                        HW_I2C_F_ADD_STOP);
        } else {
                hw_i2c_read_buffer_dma_ex(dev_config->bus_id,
                        (uint8_t)dev_config->dma_channel, rbuf, rlen,
                        ad_i2c_transaction_cb, (void *)&transaction_data);
        }

        OS_EVENT_WAIT(dev_config->bus_data->event, OS_EVENT_FOREVER);

        ad_i2c_bus_release(dev);
        ad_i2c_device_release(dev);

        return (transaction_data.success) ? 0 : (int)transaction_data.abort_source;
}
#if CONFIG_I2C_USE_ASYNC_TRANSACTIONS
static void ad_i2c_async_do(i2c_device dev, HW_I2C_ABORT_SOURCE error);

/*
 * Callback passed to low lever driver, invoked after each transmission.
 */
static void ad_i2c_cb(HW_I2C_ID id, void *user_data, uint16_t transferred, bool success)
{
        ad_i2c_async_do((i2c_device) user_data, hw_i2c_get_abort_source(id));
}

static void ad_i2c_dma_cb(HW_I2C_ID id, void *user_data, uint16_t len, bool success)
{
        ad_i2c_async_do((i2c_device) user_data, hw_i2c_get_abort_source(id));
}

/*
 * This function executes as many actions as possible.
 */
static void ad_i2c_async_do(i2c_device dev, HW_I2C_ABORT_SOURCE error)
{
        i2c_device_config *device = (i2c_device_config *) dev;
        i2c_bus_dynamic_data *data = device->bus_data;
        uint16_t len;
        const uint8_t *wbuf;
        uint8_t *rbuf;
        ad_i2c_user_cb cb;

        while (true) {
                uint32_t cmd = data->transaction[data->transaction_ix] & 0xFF000000;
                /*
                 * When there was error skip all sends and receives
                 */
                if (error) {
                        switch (cmd) {
                        case I2C_TAG_SEND_STOP:
                        case I2C_TAG_SEND:
                        case I2C_TAG_RECEIVE:
                        case I2C_TAG_RECEIVE_STOP:
                                data->transaction_ix += 2;
                                continue;
                        }
                }
                switch (cmd) {
                case I2C_TAG_SEND_STOP:
                        /* start write operation, next action executes from interrupt */
                        len = (uint16_t) data->transaction[data->transaction_ix++];
                        wbuf = (const uint8_t *) data->transaction[data->transaction_ix++];
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
                        hw_i2c_write_buffer_async(device->bus_id, wbuf, len, ad_i2c_cb, device, HW_I2C_F_WAIT_FOR_STOP);
#else
                        hw_i2c_write_buffer_async(device->bus_id, wbuf, len, ad_i2c_cb, device,
                                                        HW_I2C_F_WAIT_FOR_STOP | HW_I2C_F_ADD_STOP);
#endif
                        return;
                case I2C_TAG_SEND:
                        /* start write operation, next action executes from interrupt */
                        len = (uint16_t) data->transaction[data->transaction_ix++];
                        wbuf = (const uint8_t *) data->transaction[data->transaction_ix++];
                        hw_i2c_write_buffer_async(device->bus_id, wbuf, len, ad_i2c_cb, device, HW_I2C_F_NONE);
                        return;
                case I2C_TAG_RECEIVE:
                case I2C_TAG_RECEIVE_STOP:
                        /* start read operation, next action executes from interrupt */
                        len = (uint16_t) data->transaction[data->transaction_ix++];
                        rbuf = (uint8_t *) data->transaction[data->transaction_ix++];
                        if (device->dma_channel < 0 || len <= 1) {
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
                                hw_i2c_read_buffer_async(device->bus_id, rbuf, len, ad_i2c_cb, device, HW_I2C_F_NONE);
#else
                                hw_i2c_read_buffer_async(device->bus_id, rbuf, len, ad_i2c_cb, device,
                                        (cmd == I2C_TAG_RECEIVE) ? HW_I2C_F_NONE :
                                                        (HW_I2C_F_WAIT_FOR_STOP | HW_I2C_F_ADD_STOP));
#endif
                        } else {
                                hw_i2c_read_buffer_dma_ex(device->bus_id,
                                                (uint8_t)device->dma_channel, rbuf, len,
                                                ad_i2c_dma_cb, device);
                        }
                        return;
                case I2C_TAG_CALLBACK0:
                        data->transaction_ix++;

                        cb = (ad_i2c_user_cb ) data->transaction[data->transaction_ix++];
                        /* If next action is I2C_END callback should be called after bus and device
                         * are released.
                         */
                        if (data->transaction[data->transaction_ix] == I2C_END) {
                                data->transaction_ix = 0;
                                ad_i2c_bus_release(dev);
                                ad_i2c_device_release(dev);
                                cb(NULL, error);
                                return;
                        } else {
                                /* In the middle callback not need to release yet */
                                cb(NULL, error);
                        }
                        break;
                case I2C_TAG_CALLBACK1:
                        data->transaction_ix++;

                        cb = (ad_i2c_user_cb ) data->transaction[data->transaction_ix++];
                        /* Take callback user data */
                        rbuf = (uint8_t *) data->transaction[data->transaction_ix++];
                        /* If next action is I2C_END callback should be called after bus and device
                         * are released.
                         */
                        if (data->transaction[data->transaction_ix] == I2C_END) {
                                data->transaction_ix = 0;
                                ad_i2c_bus_release(dev);
                                ad_i2c_device_release(dev);
                                cb(rbuf, error);
                                return;
                        } else {
                                cb(rbuf, error);
                        }
                        break;
                case I2C_END:
                        data->transaction_ix = 0;
                        /* Just release bus and device */
                        ad_i2c_bus_release(dev);
                        ad_i2c_device_release(dev);
                        return;
                }
        }
}

void ad_i2c_async_transact(i2c_device dev, ...)
{
        i2c_device_config *device = (i2c_device_config *) dev;
        i2c_bus_dynamic_data *data = device->bus_data;
        va_list ap;
        int i = 0;
        uint32_t action;

        va_start(ap, dev);

        /*
         * Acquire device and bus.
         * The will be released at the end of transaction from interrupt context.
         */
        ad_i2c_device_acquire(dev);
        ad_i2c_bus_acquire(dev);

        /*
         * i2c_device_acquire and i2c_bus_acquire prevent access from other task, but will not
         * block if current task has asynchronous transaction in progress.
         * Make sure that previous operation was finished.
         */
        OS_ASSERT(data->transaction_ix == 0);

        /*
         * Put arguments in transaction action buffer, actions will be taken from it later when
         * needed.
         */
        do {
                action = va_arg(ap, uint32);
                data->transaction[i++] = action;
                switch (action & 0xFF000000) {
                case I2C_TAG_CALLBACK0:
                case I2C_TAG_SEND:
                case I2C_TAG_RECEIVE:
                case I2C_TAG_SEND_STOP:
                case I2C_TAG_RECEIVE_STOP:
                        data->transaction[i++] = va_arg(ap, uint32);
                        break;
                case I2C_TAG_CALLBACK1:
                        data->transaction[i++] = va_arg(ap, uint32);
                        data->transaction[i++] = va_arg(ap, uint32);
                        break;
                case I2C_END:
                        break;
                default:
                        OS_ASSERT(0);
                }
        } while (action != I2C_END);
        /*
         * If this assert fails increase I2C_ASYNC_ACTIONS_SIZE to fit specific application needs.
         * It takes memory even if not in use.
         */
        OS_ASSERT(i <= I2C_ASYNC_ACTIONS_SIZE);
        va_end(ap);

        data->transaction_ix = 0;

        /*
         * Start executing transaction.
         */
        ad_i2c_async_do(dev, 0);
}
#endif

#if dg_configI2C_ADAPTER_SLAVE_SUPPORT

static void ad_i2c_slave_cb(HW_I2C_ID id, HW_I2C_EVENT event);

static void ad_i2c_slave_sent_cb(HW_I2C_ID id, void *cb_data, uint16_t len, bool success)
{
        i2c_device_config *device = (i2c_device_config *) cb_data;
        i2c_dev_dynamic_data_slave *slave = (i2c_dev_dynamic_data_slave *) device->data;

        slave->state &= ~AD_I2C_SLAVE_STATE_WRITE_PENDING;

        if (slave->state & AD_I2C_SLAVE_STATE_READ_PENDING) {
                /* More operations pending. */
                hw_i2c_set_slave_callback(id, ad_i2c_slave_cb);
        }

        if (NULL != slave->event_callbacks &&
                NULL != slave->event_callbacks->data_sent) {
                slave->event_callbacks->data_sent((i2c_device) device, len, success,
                                                                        slave->user_data);
        }

        if (slave->operation_done_event) {
                if (in_interrupt()) {
                        OS_EVENT_SIGNAL_FROM_ISR(slave->operation_done_event);
                } else {
                        OS_EVENT_SIGNAL(slave->operation_done_event);
                }
        }
}

static void ad_i2c_slave_received_cb(HW_I2C_ID id, void *cb_data, uint16_t len, bool success)
{
        i2c_device_config *device = (i2c_device_config *) cb_data;
        i2c_dev_dynamic_data_slave *slave = (i2c_dev_dynamic_data_slave *) device->data;

        slave->state &= ~AD_I2C_SLAVE_STATE_READ_PENDING;

        if (slave->state & AD_I2C_SLAVE_STATE_WRITE_PENDING) {
                /* More operations pending. */
                hw_i2c_set_slave_callback(id, ad_i2c_slave_cb);
        }

        if (NULL != slave->event_callbacks &&
                NULL != slave->event_callbacks->data_received) {
                slave->event_callbacks->data_received((i2c_device) device, len, success,
                                                                        slave->user_data);
        }

        if (slave->operation_done_event) {
                if (in_interrupt()) {
                        OS_EVENT_SIGNAL_FROM_ISR(slave->operation_done_event);
                } else {
                        OS_EVENT_SIGNAL(slave->operation_done_event);
                }
        }
}

static void i2c_slave_send(HW_I2C_ID id, i2c_device_config *device)
{
        i2c_dev_dynamic_data_slave *slave = (i2c_dev_dynamic_data_slave *) device->data;

        /*
         * Master initiated read, if user already prepared buffer, send it.
         * If not notify user that read request is pending.
         */
        if (slave->output_buffer && slave->output_buffer_len) {
                hw_i2c_write_buffer_async(id, slave->output_buffer, slave->output_buffer_len,
                                                                ad_i2c_slave_sent_cb, device, 0);
        } else if (slave->event_callbacks != NULL) {
                if (slave->event_callbacks->read_request != NULL) {
                        slave->event_callbacks->read_request((i2c_device *) device,
                                slave->user_data);
                }
        }
}

/*
 * Master initiated write, if user already prepared buffer, use it.
 * If not notify user that master started write.
 */
static void i2c_slave_receive(HW_I2C_ID id, i2c_device_config *device)
{
        i2c_dev_dynamic_data_slave *slave = (i2c_dev_dynamic_data_slave *) device->data;

        if (slave->input_buffer && slave->input_buffer_len) {
                if (device->dma_channel < 0 || slave->input_buffer_len <= 1) {
                        hw_i2c_read_buffer_async(id, slave->input_buffer, slave->input_buffer_len,
                                                        ad_i2c_slave_received_cb, device, 0);
                } else {
                        hw_i2c_register_slave_dma_read_callback(id);
                }
        } else if (slave->event_callbacks != NULL) {
                if (slave->event_callbacks->data_ready != NULL) {
                        slave->event_callbacks->data_ready((i2c_device *) device, slave->user_data);
                }
        }
}

static void ad_i2c_slave_cb(HW_I2C_ID id, HW_I2C_EVENT event)
{
        i2c_device_config *device = (i2c_device_config *) ad_i2c_get_device_by_hw_id(id);

        switch (event) {
        case HW_I2C_EVENT_READ_REQUEST:
                i2c_slave_send(id, device);
                break;
        case HW_I2C_EVENT_DATA_READY:
                i2c_slave_receive(id, device);
                break;
        default:
                break;
        }
}

void ad_i2c_start_slave(i2c_device dev, const uint8_t *wdata, uint16_t wlen, uint8_t *rdata,
                uint16_t rlen, const i2c_dev_slave_event_callbacks *events, void *user_data)
{
        i2c_device_config *device = (i2c_device_config *) dev;
        i2c_dev_dynamic_data_slave *slave = (i2c_dev_dynamic_data_slave *) device->data;
        HW_I2C_ID id = ad_i2c_get_hw_i2c_id(dev);

        /* Acquire I2C device and bus */
        ad_i2c_device_acquire(dev);
        slave->state = AD_I2C_SLAVE_STATE_INIT;
        ad_i2c_bus_acquire(dev);
        /* Enable I2C controller */
        hw_i2c_enable(id);

        slave->event_callbacks = events;
        slave->user_data = user_data;
        slave->output_buffer = wdata;
        slave->output_buffer_len = wlen;
        slave->input_buffer = rdata;
        slave->input_buffer_len = rlen;
        if (wdata != NULL && wlen) {
                slave->state |= AD_I2C_SLAVE_STATE_WRITE_PENDING;
        }
        if (rdata != NULL && rlen > 0) {
                slave->state |= AD_I2C_SLAVE_STATE_READ_PENDING;
                if (device->dma_channel >= 0) {
                        /* When DMA is used for the Rx it is better to set it up here (rather than
                         * in the read request handler) so that the slave is more responsive. */
                        hw_i2c_prepare_dma_ex(id, device->dma_channel, (uint16_t *) rdata, rlen,
                                HW_I2C_DMA_TRANSFER_SLAVE_READ, ad_i2c_slave_received_cb, device, 0);
                        hw_i2c_dma_start(id);
                }
        }
        hw_i2c_set_slave_callback(id, ad_i2c_slave_cb);
}

void ad_i2c_stop_slave(i2c_device dev)
{
        i2c_device_config *device = (i2c_device_config *) dev;
        i2c_dev_dynamic_data_slave *slave = (i2c_dev_dynamic_data_slave *) device->data;
        HW_I2C_ID id = ad_i2c_get_hw_i2c_id(dev);

        /* Current task wasn't start slave - do nothing */
#if CONFIG_I2C_USE_RESMGMT && !CONFIG_I2C_EXCLUSIVE_OPEN
        if (slave->i2c.owner != OS_GET_CURRENT_TASK()) {
                return;
        }
#endif

        OS_EVENT_CREATE(slave->operation_done_event);

        if (hw_i2c_is_slave_busy(id)) {
                OS_EVENT_WAIT(slave->operation_done_event, OS_EVENT_FOREVER);
                /* Wait for flushing TX fifo */
                while(!hw_i2c_is_tx_fifo_empty(id));
        }

        OS_EVENT_DELETE(slave->operation_done_event);
        slave->event_callbacks = NULL;
        slave->user_data = NULL;
        slave->output_buffer = NULL;
        slave->output_buffer_len = 0;
        slave->input_buffer = NULL;
        slave->input_buffer_len = 0;
        slave->state = AD_I2C_SLAVE_STATE_STOPPED;
        slave->operation_done_event = NULL;
        hw_i2c_set_slave_callback(id, NULL);

        /* Release I2C bus and device completely */
#if CONFIG_I2C_USE_RESMGMT
#if !CONFIG_I2C_ONE_DEVICE_ON_BUS
        for(;device->data->bus_acquire_count > 0; ad_i2c_bus_release(dev));
#endif
#if !CONFIG_I2C_EXCLUSIVE_OPEN
        for(;device->data->dev_acquire_count > 0; ad_i2c_device_release(dev));
#endif
#endif
        hw_i2c_disable(id);
}

#endif

void ad_i2c_device_acquire(i2c_device dev)
{
#if CONFIG_I2C_USE_RESMGMT
#if !CONFIG_I2C_EXCLUSIVE_OPEN
        i2c_dev_dynamic_data *data =  ((i2c_device_config *) dev)->data;
        OS_TASK current_task = OS_GET_CURRENT_TASK();

        if (current_task == data->owner) {
                data->dev_acquire_count++;
                return;
        }
        resource_acquire(DEVICE_RESOURCE_MASK(dev), OS_EVENT_FOREVER);
        data->owner = current_task;
        data->dev_acquire_count++;
#endif
#endif /* CONFIG_I2C_USE_RESMGMT */
}

void ad_i2c_device_release(i2c_device dev)
{
#if CONFIG_I2C_USE_RESMGMT
#if !CONFIG_I2C_EXCLUSIVE_OPEN
        i2c_device_config *config =  (i2c_device_config *) dev;

        /* A device release can only happen from the same task that owns it, or from an ISR */
        OS_ASSERT(in_interrupt() || (OS_GET_CURRENT_TASK() == config->data->owner));

        if (--config->data->dev_acquire_count == 0) {
                config->data->owner = NULL;
                resource_release(DEVICE_RESOURCE_MASK(dev));
        }
#endif
#endif /* CONFIG_I2C_USE_RESMGMT */
}

i2c_device ad_i2c_open(const i2c_device_id dev_id)
{
        i2c_device_config *device = (i2c_device_config *) dev_id;

#if CONFIG_I2C_USE_RESMGMT
#if CONFIG_I2C_EXCLUSIVE_OPEN
        resource_acquire(DEVICE_RESOURCE_MASK(device), OS_EVENT_FOREVER);
#endif
#endif

#if CONFIG_I2C_ONE_DEVICE_ON_BUS
        /*
         * If there is at most one device on each bus, there is no need to check
         * configuration each time before accessing bus. In this case configuration
         * will be applied in open and never changed.
         */
        ad_i2c_bus_apply_config(device);

#if CONFIG_I2C_USE_DMA_RESMGMT
        if (device->dma_channel >= 0) {
                resource_acquire(dma_resource_mask(device->dma_channel) |
                        dma_resource_mask(device->dma_channel + 1), RES_WAIT_FOREVER);
        }
#endif
#endif

        return (i2c_device) device;
}

void ad_i2c_close(i2c_device dev)
{
        i2c_device_config *device __attribute__((unused)) = (i2c_device_config *) dev;
#if CONFIG_I2C_ONE_DEVICE_ON_BUS
#if CONFIG_I2C_USE_DMA_RESMGMT
        if (device->dma_channel >= 0) {
                resource_release(dma_resource_mask(device->dma_channel) |
                        dma_resource_mask(device->dma_channel + 1));
        }
#endif

        ad_i2c_bus_reset_config(device);
#endif

#if CONFIG_I2C_USE_RESMGMT
#if CONFIG_I2C_EXCLUSIVE_OPEN
        resource_release(DEVICE_RESOURCE_MASK(device));
#endif
#endif
}

static bool ad_i2c_prepare_for_sleep(void)
{
        if (hw_i2c_controler_is_busy(HW_I2C1) || hw_i2c_controler_is_busy(HW_I2C2)) {
                return false;
        }
#if !CONFIG_I2C_ONE_DEVICE_ON_BUS
#if CONFIG_I2C_USE_RESMGMT
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

static void ad_i2c_sleep_canceled(void)
{
}

static void ad_i2c_wake_up_ind(bool arg)
{
#if CONFIG_I2C_ONE_DEVICE_ON_BUS || !CONFIG_I2C_USE_RESMGMT
        if (current_config[0]) {
                ad_i2c_bus_apply_config((i2c_device_config *) current_config[0]);
        }
        if (current_config[1]) {
                ad_i2c_bus_apply_config((i2c_device_config *) current_config[1]);
        }
#endif
}

const adapter_call_backs_t ad_i2c_pm_call_backs = {
        .ad_prepare_for_sleep = ad_i2c_prepare_for_sleep,
        .ad_sleep_canceled = ad_i2c_sleep_canceled,
        .ad_wake_up_ind = ad_i2c_wake_up_ind,
        .ad_xtal16m_ready_ind = NULL,
        .ad_sleep_preparation_time = 0
};

void ad_i2c_init(void)
{
        pm_register_adapter(&ad_i2c_pm_call_backs);
}

ADAPTER_INIT(ad_i2c_adapter, ad_i2c_init);

#endif

/**
 * \}
 * \}
 * \}
 */
