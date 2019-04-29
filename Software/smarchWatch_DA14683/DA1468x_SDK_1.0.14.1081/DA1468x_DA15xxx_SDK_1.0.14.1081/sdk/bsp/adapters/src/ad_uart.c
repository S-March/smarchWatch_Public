/**
 * \addtogroup BSP
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup UART
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_uart.c
 *
 * @brief UART adapter implementation
 *
 * Copyright (C) 2015-2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#if dg_configUART_ADAPTER

#define _UART_DEV(_bus_id, _bus_res_id, dynamic_data, \
        name, _baud_rate, data_bits, _parity, _stop, _auto_flow_control, \
        _use_fifo, _dma_channel_tx, _dma_channel_rx, _tx_fifo_tr_lvl, _rx_fifo_tr_lvl, _flags) \
        __RETAINED uart_bus_dynamic_data dynamic_data; \
        const uart_device_config dev_##name = { \
                .bus_id = (_bus_id), \
                .bus_res_id = (_bus_res_id), \
                .hw_init.baud_rate = _baud_rate, \
                .hw_init.data = data_bits, \
                .hw_init.parity = _parity, \
                .hw_init.stop = _stop, \
                .hw_init.auto_flow_control = _auto_flow_control, \
                .hw_init.use_fifo = _use_fifo, \
                .hw_init.use_dma = ((_dma_channel_tx != HW_DMA_CHANNEL_INVALID) && \
                        (_dma_channel_rx != HW_DMA_CHANNEL_INVALID)), \
                .hw_init.tx_fifo_tr_lvl = _tx_fifo_tr_lvl, \
                .hw_init.rx_fifo_tr_lvl = _rx_fifo_tr_lvl, \
                .hw_init.rx_dma_channel = _dma_channel_rx, \
                .hw_init.tx_dma_channel = _dma_channel_tx, \
                .bus_data = &dynamic_data, \
                .flags = _flags \
        }; \
        const void *const name = &dev_##name;

#define UART_BUS(bus, name, _baud_rate, data_bits, _parity, _stop, _auto_flow_control, \
                _use_fifo, _dma_channel_tx, _dma_channel_rx, _tx_fifo_tr_lvl, _rx_fifo_tr_lvl) \
        _UART_DEV(HW_##bus, RES_ID_##bus, dynamic_##bus, name, \
                _baud_rate, data_bits, _parity, _stop, _auto_flow_control, \
                _use_fifo, _dma_channel_tx, _dma_channel_rx, _tx_fifo_tr_lvl, _rx_fifo_tr_lvl, 0)

#define UART_DEV(bus, name, _baud_rate, data_bits, _parity, _stop, _auto_flow_control, \
                _use_fifo, _dma_channel_tx, _dma_channel_rx, _tx_fifo_tr_lvl, _rx_fifo_tr_lvl, flags) \
        _UART_DEV(HW_##bus, RES_ID_##bus, dynamic_##bus, name, \
                _baud_rate, data_bits, _parity, _stop, _auto_flow_control, \
                _use_fifo, _dma_channel_tx, _dma_channel_rx, _tx_fifo_tr_lvl, _rx_fifo_tr_lvl, flags)

#include <platform_devices.h>
#include <hw_wkup.h>
#include <hw_cpm.h>
#include <ad_uart.h>
#include <resmgmt.h>
#include <interrupts.h>
#include <sys_power_mgr.h>

#define DEVICE_RESOURCE_MASK(config, type) \
        ad_uart_resource_mask((uart_device_config *) config, type)

#ifndef dg_configADAPTERS_NO_LOCKING_ASYNC
#define dg_configADAPTERS_NO_LOCKING_ASYNC      0
#endif

/*
 * Array to hold current device for each UART.
 */
__RETAINED static const uart_device_config *current_config[2];
#define UARTIX(id) ((id) == HW_UART1 ? 0 : 1)

/*
 * Apply device specific configuration of UART controller
 */
static void ad_uart_bus_apply_config(uart_device_config *device)
{
        const HW_UART_ID id = ad_uart_get_hw_uart_id(device);

        current_config[UARTIX(id)] = device;

        hw_uart_init_ex(id, &device->hw_init);
        if (device->hw_init.auto_flow_control && id == HW_UART2) {
                /* Search GPIO settings for CTS */
                if (1 != hw_gpio_get_pins_with_function(HW_GPIO_FUNC_UART2_CTSN,
                                                                &device->bus_data->cts_pin, 1)) {
                        /* One and only one pin can be configured for CTS */
                        OS_ASSERT(0);
                }
        }

#if dg_configUART_RX_CIRCULAR_DMA

        /*
         * If circular DMA or RX is enabled on UART, we automatically use it in adapter. However,
         * it can be enabled separately for each UART so we need to check this and configure adapter
         * in runtime appropriately.
         */

        if (((id == HW_UART1) && (dg_configUART1_RX_CIRCULAR_DMA_BUF_SIZE > 0)) ||
                ((id == HW_UART2) && (dg_configUART2_RX_CIRCULAR_DMA_BUF_SIZE > 0))) {
                device->bus_data->use_rx_circular_dma = true;
        }

        if (device->bus_data->use_rx_circular_dma) {
                hw_uart_enable_rx_circular_dma(id);
        }
#endif
}

#if CONFIG_UART_USE_DMA_RESMGMT
static inline uint32_t dma_resource_mask(int num)
{
        static const uint32_t res_mask[] = {
                RES_MASK(RES_ID_DMA_CH0),
                RES_MASK(RES_ID_DMA_CH1),
                RES_MASK(RES_ID_DMA_CH2),
                RES_MASK(RES_ID_DMA_CH3),
                RES_MASK(RES_ID_DMA_CH4),
                RES_MASK(RES_ID_DMA_CH5),
                RES_MASK(RES_ID_DMA_CH6),
                RES_MASK(RES_ID_DMA_CH7),
        };
        return res_mask[num];
}
#endif

#if CONFIG_UART_USE_RESMGMT || CONFIG_UART_USE_DMA_RESMGMT

static inline uint32_t ad_uart_resource_mask(const uart_device_config *config,
        AD_UART_RES_TYPE res_type)
{
        uint32_t resource_mask = 0;
        switch (res_type) {
        case AD_UART_RES_TYPE_CONFIG:
#if CONFIG_UART_USE_RESMGMT
                resource_mask |= 1 <<
                        (config->bus_id == HW_UART1 ? RES_ID_UART1_CONFIG : RES_ID_UART2_CONFIG);
#endif
                break;
        case AD_UART_RES_TYPE_WRITE:
#if CONFIG_UART_USE_RESMGMT
                resource_mask |= 1 <<
                        (config->bus_id == HW_UART1 ? RES_ID_UART1_WRITE : RES_ID_UART2_WRITE);
#endif
#if CONFIG_UART_USE_DMA_RESMGMT
                resource_mask |= dma_resource_mask(config->hw_init.tx_dma_channel);
#endif
                break;
        case AD_UART_RES_TYPE_READ:
#if CONFIG_UART_USE_RESMGMT
                resource_mask |= 1 <<
                        (config->bus_id == HW_UART1 ? RES_ID_UART1_READ : RES_ID_UART2_READ);
#endif
#if CONFIG_UART_USE_DMA_RESMGMT
                resource_mask |= dma_resource_mask(config->hw_init.rx_dma_channel);
#endif
                break;
        default:
                /* Invalid argument. */
                OS_ASSERT(0);
        }
        return resource_mask;
}

#endif

void ad_uart_bus_acquire_ex(uart_device dev, AD_UART_RES_TYPE res_type)
{
#if CONFIG_UART_USE_RESMGMT || CONFIG_UART_USE_DMA_RESMGMT
        const uart_device_config *config = (uart_device_config *) dev;
        uart_bus_dynamic_data *data =  config->bus_data;
        OS_TASK current_task = OS_GET_CURRENT_TASK();

        if (current_task == data->res_states[res_type].owner) {
                data->res_states[res_type].bus_acquire_count++;
                return;
        }

        resource_acquire(DEVICE_RESOURCE_MASK(dev, res_type), OS_EVENT_FOREVER);
        data->res_states[res_type].owner = current_task;
        data->res_states[res_type].bus_acquire_count++;
#endif
}

void ad_uart_bus_acquire(uart_device dev)
{
#if CONFIG_UART_USE_RESMGMT || CONFIG_UART_USE_DMA_RESMGMT
        ad_uart_bus_acquire_ex(dev, AD_UART_RES_TYPE_WRITE);
        ad_uart_bus_acquire_ex(dev, AD_UART_RES_TYPE_READ);
#endif
}

void ad_uart_bus_release_ex(uart_device dev, AD_UART_RES_TYPE res_type)
{
#if CONFIG_UART_USE_RESMGMT || CONFIG_UART_USE_DMA_RESMGMT
        const uart_device_config *config =  (uart_device_config *) dev;

        /* A device release can only happen from the same task that owns it, or from an ISR */
        OS_ASSERT(in_interrupt() ||
                (OS_GET_CURRENT_TASK() == config->bus_data->res_states[res_type].owner));

        if (--config->bus_data->res_states[res_type].bus_acquire_count == 0) {
                config->bus_data->res_states[res_type].owner = NULL;
                resource_release(DEVICE_RESOURCE_MASK(dev, res_type));
        }
#endif
}

void ad_uart_bus_release(uart_device dev)
{
#if CONFIG_UART_USE_RESMGMT || CONFIG_UART_USE_DMA_RESMGMT
        ad_uart_bus_release_ex(dev, AD_UART_RES_TYPE_READ);
        ad_uart_bus_release_ex(dev, AD_UART_RES_TYPE_WRITE);
#endif
}


uart_device ad_uart_open(const uart_device_id dev_id)
{
        uart_device_config *config = (uart_device_config *) dev_id;

#if CONFIG_UART_USE_RESMGMT
        resource_acquire(DEVICE_RESOURCE_MASK(config, AD_UART_RES_TYPE_CONFIG), OS_EVENT_FOREVER);
#endif

        if (config->bus_data->open_count++ == 0) {
                ad_uart_bus_apply_config(config);
                OS_EVENT_CREATE(config->bus_data->event_write);
                OS_EVENT_CREATE(config->bus_data->event_read);
        }

#if CONFIG_UART_USE_RESMGMT
        resource_release(DEVICE_RESOURCE_MASK(config, AD_UART_RES_TYPE_CONFIG));
#endif
        return (uart_device) config;
}

void ad_uart_close(uart_device dev)
{
        uart_device_config *config = (uart_device_config *) dev;

#if CONFIG_UART_USE_RESMGMT
        resource_acquire(DEVICE_RESOURCE_MASK(config, AD_UART_RES_TYPE_CONFIG), OS_EVENT_FOREVER);
#endif
        if (--config->bus_data->open_count == 0) {
                OS_EVENT_DELETE(config->bus_data->event_read);
                OS_EVENT_DELETE(config->bus_data->event_write);
        }
#if CONFIG_UART_USE_RESMGMT
        resource_release(DEVICE_RESOURCE_MASK(config, AD_UART_RES_TYPE_CONFIG));
#endif

}

HW_UART_ID ad_uart_get_hw_uart_id(uart_device dev)
{
        uart_device_config *device = (uart_device_config *) dev;

        return device->bus_id;
}

struct uart_cb_data {
        uart_device_config *device;
        uint16_t transferred;
};

static void ad_uart_signal_event_write(void *p, uint16_t transferred)
{
        struct uart_cb_data *cd = (struct uart_cb_data *) p;
        cd->transferred = transferred;
        OS_EVENT_SIGNAL_FROM_ISR(cd->device->bus_data->event_write);
}

static void ad_uart_signal_event_read(void *p, uint16_t transferred)
{
        struct uart_cb_data *cd = (struct uart_cb_data *) p;
        cd->transferred = transferred;
        OS_EVENT_SIGNAL_FROM_ISR(cd->device->bus_data->event_read);
}

void ad_uart_write(uart_device dev, const char *wbuf, size_t wlen)
{
        uart_device_config *device = (uart_device_config *) dev;
        const HW_UART_ID id = device->bus_id;
        struct uart_cb_data cd = { .device = device, .transferred = 0 };

        ad_uart_bus_acquire_ex(dev, AD_UART_RES_TYPE_WRITE);

        OS_EVENT_WAIT(device->bus_data->event_write, 0);
        hw_uart_send(id, (const uint8_t *) wbuf, wlen, ad_uart_signal_event_write, &cd);
        OS_EVENT_WAIT(device->bus_data->event_write, OS_EVENT_FOREVER);

        ad_uart_bus_release_ex(dev, AD_UART_RES_TYPE_WRITE);
}

int ad_uart_read(uart_device dev, char *rbuf, size_t rlen, OS_TICK_TIME timeout)
{
        uart_device_config *device = (uart_device_config *) dev;
        const HW_UART_ID id = device->bus_id;
        struct uart_cb_data cd = { .device = device, .transferred = 0 };

        ad_uart_bus_acquire_ex(dev, AD_UART_RES_TYPE_READ);

        OS_EVENT_WAIT(device->bus_data->event_read, 0);
        hw_uart_receive(id, (uint8_t *) rbuf, rlen, ad_uart_signal_event_read, &cd);
        OS_EVENT_WAIT(device->bus_data->event_read, timeout);
        ad_uart_complete_async_read(dev);

        ad_uart_bus_release_ex(dev, AD_UART_RES_TYPE_READ);

        return cd.transferred;
}

static void ad_uart_read_callback(void *p, uint16_t transferred)
{
        uart_device_config *device = (uart_device_config *) p;
        uart_bus_dynamic_data *data = device->bus_data;
        ad_uart_user_cb cb = data->read_cb;
        void *user_data = data->read_cb_data;
        data->read_cb = NULL;
        data->read_cnt = transferred;

        /* A not NULL callback must be registered before starting a transaction */
        OS_ASSERT(cb != NULL);

        if ((device->flags & AD_UART_DEVICE_FLAGS_LOCKING_ASYNC)) {
                ad_uart_bus_release_ex((uart_device) device, AD_UART_RES_TYPE_READ);
        }
        cb(user_data, transferred);
}

static void ad_uart_write_callback(void *p, uint16_t transferred)
{
        uart_device_config *device = (uart_device_config *) p;
        uart_bus_dynamic_data *data = device->bus_data;
        ad_uart_user_cb cb = data->write_cb;
        void *user_data = data->write_cb_data;
        data->write_cb = NULL;

        /* A not NULL callback must be registered before starting a transaction */
        OS_ASSERT(cb != NULL);

        if ((device->flags & AD_UART_DEVICE_FLAGS_LOCKING_ASYNC)) {
                ad_uart_bus_release_ex((uart_device) device, AD_UART_RES_TYPE_WRITE);
        }
        cb(user_data, transferred);
}

void ad_uart_write_async(uart_device dev, const char *wbuf, size_t wlen, ad_uart_user_cb cb,
                                                                                void *user_data)
{
        uart_device_config *device = (uart_device_config *) dev;
        uart_bus_dynamic_data *data = device->bus_data;

        if ((device->flags & AD_UART_DEVICE_FLAGS_LOCKING_ASYNC)) {
                ad_uart_bus_acquire_ex(dev, AD_UART_RES_TYPE_WRITE);
        }
        data->write_cb = cb;
        data->write_cb_data = user_data;

        hw_uart_send(device->bus_id, (const uint8_t *) wbuf, wlen, ad_uart_write_callback, dev);
}

void ad_uart_read_async(uart_device dev, char *rbuf, size_t rlen, ad_uart_user_cb cb,
                                                                                void *user_data)
{
        uart_device_config *device = (uart_device_config *) dev;
        uart_bus_dynamic_data *data = device->bus_data;

        if ((device->flags & AD_UART_DEVICE_FLAGS_LOCKING_ASYNC)) {
                ad_uart_bus_acquire_ex(dev, AD_UART_RES_TYPE_READ);
        }
        data->read_cb = cb;
        data->read_cb_data = user_data;
#if dg_configUART_RX_CIRCULAR_DMA
        data->read_cb_ptr = rbuf;
#endif

        hw_uart_receive(device->bus_id, (uint8_t *) rbuf, rlen, ad_uart_read_callback, dev);
}

int ad_uart_complete_async_read(uart_device dev)
{
        uart_device_config *device = (uart_device_config *) dev;
        OS_EVENT_WAIT(device->bus_data->event_read, 0);

#if CONFIG_UART_USE_RESMGMT
        /* Only the device owner can abort the read transaction */
        OS_ASSERT(device->bus_data->res_states[AD_UART_RES_TYPE_READ].owner == OS_GET_CURRENT_TASK());
#endif

        /* Force callback */
        return hw_uart_abort_receive(device->bus_id);
}

void ad_uart_abort_read_async(uart_device dev)
{
        uart_device_config *device = (uart_device_config *) dev;

#if CONFIG_UART_USE_RESMGMT
        /* Only the device owner can abort the read transaction */
        OS_ASSERT(device->bus_data->res_states[AD_UART_RES_TYPE_READ].owner == OS_GET_CURRENT_TASK());
#endif

        /* Force callback */
        hw_uart_abort_receive(device->bus_id);
}

/* Return time in us for one byte transmission at 8N1 (10 bits per byte) */
static uint32_t byte_time(HW_UART_BAUDRATE baud)
{
        switch (baud) {
        case HW_UART_BAUDRATE_1000000: return 10;
        case HW_UART_BAUDRATE_230400: return 44;
        case HW_UART_BAUDRATE_115200: return 87;
        case HW_UART_BAUDRATE_57600: return 174;
        case HW_UART_BAUDRATE_38400: return 261;
        case HW_UART_BAUDRATE_28800: return 348;
        case HW_UART_BAUDRATE_19200: return 521;
        case HW_UART_BAUDRATE_14400: return 695;
        case HW_UART_BAUDRATE_9600: return 1042;
        case HW_UART_BAUDRATE_4800: return 2084;
        default:
                /* Illegal baudrate requested */
                OS_ASSERT(0);
                return 0;
        }
}

static bool ad_uart_flow_off(void)
{
        int i;
        const uart_device_config *serial2 = current_config[1];

        /* If no CTS is configured for UART2, allow platform to go to sleep */
        if (!serial2 || serial2->bus_data->cts_pin == 0) {
                return true;
        }

        /* Check if CTS is asserted, if so don't sleep */
        if (hw_uart_cts_getf(HW_UART2)) {
                return false;
        }

        /* If flow control is not enabled, allow platform to go to sleep (no need to stop flow now). */
        if (!hw_uart_afce_getf(HW_UART2)) {
                return true;
        }

        /* Stop flow, it should tell host to stop sending data */
        hw_uart_rts_setf(HW_UART2, 0);

        /*
         * Wait for 1 character duration to ensure host has not started a transmission
         * at the same time
         * 0.25us at 16MHz for one loop
         */
        i = byte_time(serial2->hw_init.baud_rate) * cm_cpu_clk_get_fromISR() / 4;
        asm volatile (
                        "    cmp %0, #0\n"
                        "    ble L_END\n"
                        "L_LOOP:\n"
                        "    sub %0, %0, #1\n"
                        "    bne L_LOOP\n"
                        "L_END:"
                        : "=l" (i) : "l" (i));

        /* Check if data has been received during wait time */
        if (hw_uart_receive_fifo_not_empty(HW_UART2)) {
                hw_uart_afce_setf(HW_UART2, 1);
                hw_uart_rts_setf(HW_UART2, 1);
                return false;
        }

        return true;
}

static void ad_uart_set_cts_as_wakeup(bool enable)
{
        const uart_device_config *serial2 = current_config[1];
        uint8_t cts_pin;

        if (serial2 && serial2->bus_data->cts_pin != 0) {
                cts_pin = serial2->bus_data->cts_pin;

                /* Enable or disable wake up source */
                hw_wkup_configure_pin(cts_pin >> 4, cts_pin & 7, enable, HW_WKUP_PIN_STATE_LOW);

                /* Set pin function to GPIO or CTS */
                hw_gpio_set_pin_function(cts_pin >> 4, cts_pin & 0x7, HW_GPIO_MODE_INPUT,
                                        enable ? HW_GPIO_FUNC_GPIO : HW_GPIO_FUNC_UART2_CTSN);
        }
}

static bool ad_uart_prepare_for_sleep(void)
{
        /* Do not sleep when there is transmission in progress */
        if (hw_uart_tx_in_progress(HW_UART1) || hw_uart_tx_in_progress(HW_UART2)) {
                return false;
        }

        if (!ad_uart_flow_off()) {
                /* Incoming data, prevent sleep */
                return false;
        }

        if (hw_uart_is_busy(HW_UART1) || hw_uart_is_busy(HW_UART2)) {
                return false;
        }

        /* Reconfigure CTS as GPIO with wake up function */
        ad_uart_set_cts_as_wakeup(true);

        return true;
}

static void ad_uart_flow_on(void)
{
        const uart_device_config *serial2 = current_config[1];

        if (serial2 && serial2->bus_data->cts_pin != 0) {
                /* Restore hardware flow control */
                hw_uart_afce_setf(HW_UART2, 1);
                hw_uart_rts_setf(HW_UART2, 1);
        }
}

static void ad_uart_sleep_canceled(void)
{
        /* UART was not powered down yet, disconnect from reconfigure CTS and turn on flow */
        ad_uart_set_cts_as_wakeup(false);
        ad_uart_flow_on();
}

static void ad_uart_wake_up_ind(bool arg)
{
}

static void ad_uart_xtal16m_ready_ind(void)
{
        if (current_config[0] != NULL) {
                hw_uart_reinit_ex(HW_UART1, &current_config[0]->hw_init);
        }
        if (current_config[1] != NULL) {
                hw_uart_reinit_ex(HW_UART2, &current_config[1]->hw_init);
                ad_uart_set_cts_as_wakeup(false);
                ad_uart_flow_on();
        }
}

const adapter_call_backs_t ad_uart_pm_call_backs = {
        .ad_prepare_for_sleep = ad_uart_prepare_for_sleep,
        .ad_sleep_canceled = ad_uart_sleep_canceled,
        .ad_wake_up_ind = ad_uart_wake_up_ind,
        .ad_xtal16m_ready_ind = ad_uart_xtal16m_ready_ind,
        .ad_sleep_preparation_time = 0
};

void ad_uart_init(void)
{
        pm_register_adapter(&ad_uart_pm_call_backs);
}

#if dg_configUART_SOFTWARE_FIFO
void ad_uart_set_soft_fifo(uart_device dev, uint8_t *buf, uint8_t size)
{
        uart_device_config *device = (uart_device_config *) dev;
        const HW_UART_ID id = device->bus_id;

        ad_uart_bus_acquire(dev);

        hw_uart_set_soft_fifo(id, buf, size);

        ad_uart_bus_release(dev);
}
#endif

ADAPTER_INIT(au_uart_adapter, ad_uart_init)

#endif /* dg_configUART_ADAPTER */

/**
 * \}
 * \}
 * \}
 */
