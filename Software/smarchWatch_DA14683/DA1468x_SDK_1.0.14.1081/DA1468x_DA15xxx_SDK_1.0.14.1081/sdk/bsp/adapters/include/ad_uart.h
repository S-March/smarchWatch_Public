/**
 * \addtogroup BSP
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup UART_ADAPTER
 *
 * \brief UART adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_uart.h
 *
 * @brief UART adapter API
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_UART_H_
#define AD_UART_H_

#if dg_configUART_ADAPTER

#include <osal.h>
#include <resmgmt.h>
#include <hw_uart.h>
#include <hw_dma.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \def CONFIG_UART_USE_RESMGMT
 *
 * \brief Controls whether UART resource acquisition will be used
 *
 * UART adapter provides resource access management in order to protect UART resources (devices and
 * buses) from being used by multiple tasks simultaneously. Unsetting this macro will remove this
 * feature, improving performance and potentially reducing code size (if the resource management API
 * is not used by other modules). All acquisition and release UART adapter calls omit calls to
 * the resource management API when this macro is unset.
 *
 * \sa CONFIG_UART_USE_DMA_RESMGMT
 */
#ifndef CONFIG_UART_USE_RESMGMT
# define CONFIG_UART_USE_RESMGMT                (1)
#endif

/**
 * \def CONFIG_UART_USE_DMA_RESMGMT
 *
 * \brief Controls whether DMA resource acquisition will be used by the UART adapter
 *
 * UART adapter provides DMA resource access management in order to protect DMA resources from being
 * used by multiple tasks simultaneously. Unsetting this macro will remove this feature, improving
 * performance and potentially reducing code size (if the resource management API is not used by
 * other modules). Unset this macro if, for example, you have exclusively assigned DMA channels to
 * the UART interface.
 *
 * \sa CONFIG_UART_USE_RESMGMT
 */
#ifndef CONFIG_UART_USE_DMA_RESMGMT
# define CONFIG_UART_USE_DMA_RESMGMT            (1)
#endif

/**
 * \brief Device pointer, handle to use with ad_uart_read(), ad_uart_write() etc.
 *
 */
typedef void *uart_device;

/**
 * \brief Device id, those are created by UART_BUS macro in platform_devices.h
 *
 */
typedef const void *uart_device_id;

/**
 * \brief Calls to \ref ad_uart_read_async(), \ref ad_uart_write_async() acquire bus.
 * \note This macro is deprecated. Please use \ref AD_UART_DEVICE_FLAGS_LOCKING_ASYNC.
 */
#define AD_UART_FLAGS_LOCKING_ASYNC     (2)

/**
 * \brief UART device flags.
 */
typedef enum
{
        /**
         * If set, calls to \ref ad_uart_read_async(), \ref ad_uart_write_async() perform bus
         * acquisition.
         */
        AD_UART_DEVICE_FLAGS_LOCKING_ASYNC = 0x2,
} AD_UART_DEVICE_FLAGS;

#ifndef UART_BUS

/**
 * \brief Entry for slave device
 *
 * \param [in] bus_id valid values: UART1, UART2
 * \param [in] name of uart may be COM1, or something like this
 * \param [in] _baud_rate required uart baud rate from enum HW_UART_BAUDRATE
 * \param [in] data_bits value from enum HW_UART_DATABITS
 * \param [in] _parity value from enum HW_UART_PARITY
 * \param [in] _stop value from enum HW_UART_STOPBITS
 * \param [in] _auto_flow_control if 1 auto flow control should be used
 * \param [in] _use_fifo if 1 fifo should be used
 * \param [in] dma_channel_tx DMA number for tx channel, rx msut have the previous number, pass HW_DMA_CHANNEL_INVALID for no DMA
 * \param [in] dma_channel_rx DMA number for rx channel, tx must have the next number, pass HW_DMA_CHANNEL_INVALID for no DMA
 * \param [in] tx_fifo_tr_lvl the tx fifo trigger level, valid numbers 0..3
 * \param [in] rx_fifo_tr_lvl the rx fifo trigger level, valid numbers 0..3
 *
 */
#define UART_BUS(bus_id, name, _baud_rate, data_bits, _parity, _stop, _auto_flow_control,\
        _use_fifo, dma_channel_tx, dma_channel_rx, tx_fifo_tr_lvl, rx_fifo_tr_lvl) \
        extern const void *const name;
#endif

#ifndef UART_DEV

/**
 * \brief Entry for serial device
 *
 * \param [in] bus_id valid values: UART1, UART2
 * \param [in] name of uart may be COM1, or something like this
 * \param [in] _baud_rate required uart baud rate from enum HW_UART_BAUDRATE
 * \param [in] data_bits value from enum HW_UART_DATABITS
 * \param [in] _parity value from enum HW_UART_PARITY
 * \param [in] _stop value from enum HW_UART_STOPBITS
 * \param [in] _auto_flow_control if 1 auto flow control should be used
 * \param [in] _use_fifo if 1 fifo should be used
 * \param [in] dma_channel_tx DMA number for tx channel, rx msut have the previous number, pass HW_DMA_CHANNEL_INVALID for no DMA
 * \param [in] dma_channel_rx DMA number for rx channel, tx must have the next number, pass HW_DMA_CHANNEL_INVALID for no DMA
 * \param [in] tx_fifo_tr_lvl the tx fifo trigger level, valid numbers 0..3
 * \param [in] rx_fifo_tr_lvl the rx fifo trigger level, valid numbers 0..3
 * \param [in] _flags run time flags
 *
 * This macro creates similar device to UART_BUS, with added flags
 */
#define UART_DEV(bus_id, name, _baud_rate, data_bits, _parity, _stop, _auto_flow_control,\
        _use_fifo, dma_channel_tx, dma_channel_rx, tx_fifo_tr_lvl, rx_fifo_tr_lvl, _flags) \
        extern const void *const name;

#endif

/**
 * \brief UART resource types.
 */
typedef enum
{
        /** Configuration resource. Acquiring this resource will block calls to ad_uart_open() for
         * the same UART bus. */
        AD_UART_RES_TYPE_CONFIG,

        /** Tx (write) resource. Acquiring this resource will block write operations on the same
         * bus. */
        AD_UART_RES_TYPE_WRITE,

        /** Rx (read) resource. Acquiring this resource will block read operations on the same
         * bus. */
        AD_UART_RES_TYPE_READ,

        /** Enumeration end. */
        AD_UART_RES_TYPES
} AD_UART_RES_TYPE;

/**
 * \brief Initialize adapter
 *
 */
void ad_uart_init(void);

/**
 * \brief Asynchronous callback function
 *
 */
typedef void (*ad_uart_user_cb)(void *user_data, uint16_t transferred);

/**
 * \brief Open UART device
 *
 * First call to this function will configure UART block. Later calls from other tasks will just
 * return handle to already initialized UART.
 *
 * \param [in] dev_id identifier as passed to to UART_BUS()
 *
 * \return device handle that can be used with other functions
 */
uart_device ad_uart_open(const uart_device_id dev_id);

/**
 * \brief Close UART device
 *
 * \param [in] device handle to device opened with ad_uart_open()
 *
 * \sa ad_uart_open()
 *
 */
void ad_uart_close(uart_device device);

/**
 * \brief Write to UART
 *
 * This function performs write to UART.
 * This function is blocking. It can wait first for bus access, then it waits till transaction is
 * completed.
 *
 * \param [in] dev handle to UART device
 * \param [in] wbuf pointer to data to be sent
 * \param [in] wlen size of data to be sent
 *
 * \sa ad_uart_open()
 * \sa ad_uart_close()
 *
 */
void ad_uart_write(uart_device dev, const char *wbuf, size_t wlen);

/**
 * \brief Start asynchronous write to UART
 *
 * This function sets up write of \p wlen bytes to UART.
 * When write is done user callback \p cb will be called just after ad_uart_bus_release() is called.
 * When write operation is started, user must not release memory pointer \p wbuf.
 *
 * \param [in] dev handle to UART device
 * \param [in] wbuf pointer to data to be sent
 * \param [in] wlen size of data to be sent
 * \param [in] cb function to call after write finishes
 * \param [in] user_data pointer to pass to \p cb
 *
 * \sa ad_uart_open()
 * \sa ad_uart_close()
 *
 * \note The callback is called from within UART ISR and at that time resources
 *       are released
 *
 * \warning Do not call this function consecutively without guaranteeing that the previous
 *          async transaction has completed.
 *
 * \warning After the callback is called, it is not guaranteed that the scheduler will give
 *          control to the task waiting for this transaction to complete. This is important to
 *          consider if more than one tasks are using this API.
 */
void ad_uart_write_async(uart_device dev, const char *wbuf, size_t wlen, ad_uart_user_cb cb,
                                                                                void *user_data);

/**
 * \brief Read from UART
 *
 * This function reads rlen bytes from UART.
 * This function is blocking. It can wait first for bus access, then it waits till transaction is
 * completed.
 * If timeout is OS_EVENT_FOREVER, exactly \p rlen bytes must be received.
 * If timeout is specified, function can exit after timeout with less bytes than requested.
 *
 * \param [in] dev handle to UART device
 * \param [out] rbuf pointer to data to be read from UART
 * \param [in] rlen size of buffer pointed by rbuf
 * \param [in] timeout time in ticks to wait for data
 *
 * \return number of bytes read, 0 if no data arrived in specified time
 *
 * \sa ad_uart_open()
 * \sa ad_uart_close()
 *
 */
int ad_uart_read(uart_device dev, char *rbuf, size_t rlen, OS_TICK_TIME timeout);

/**
 * \brief Start asynchronous read from UART
 *
 * This function sets up read of \p rlen bytes from UART. The supplied callback is called when the
 * operation finishes.
 * If \ref AD_UART_DEVICE_FLAGS_LOCKING_ASYNC flag has been set for the associated device, this
 * function will also perform bus acquisition.
 * In that case and in order to avoid being blocked, the user should call ad_uart_bus_acquire()
 * beforehand. The bus will be released just before user callback is executed.
 *
 * When read operation is started, user must not release memory pointer \p rbuf.
 *
 * To abort started read operation ad_uart_abort_read_async() should be called.
 *
 * \param [in] dev handle to UART device
 * \param [out] rbuf pointer to data to be read from UART
 * \param [in] rlen size of buffer pointed by rbuf
 * \param [in] cb function to call after write is finished or aborted
 * \param [in] user_data pointer to pass to \p cb
 *
 * \sa ad_uart_open()
 * \sa ad_uart_close()
 * \sa ad_uart_abort_read_async()
 *
 * \note The callback is called from within UART ISR and at that time resources
 *       are released
 *
 * \warning Do not call this function consecutively without guaranteeing that the previous
 *          async transaction has completed.
 *
 * \warning After the callback is called, it is not guaranteed that the scheduler will give
 *          control to the task waiting for this transaction to complete. This is important to
 *          consider if more than one tasks are using this API.
 */
void ad_uart_read_async(uart_device dev, char *rbuf, size_t rlen, ad_uart_user_cb cb,
                                                                                void *user_data);

/**
 * \brief Finish asynchronous read
 *
 * Asynchronous reads allow to create reads with timeouts, after requested number of characters
 * arrive user callback is executed from ISR.
 * At any moment after read is started application can decide that if less than requested
 * characters were received, read request should end.
 * In such case application code should call ad_uart_complete_async_read() to get number of
 * characters read.
 * This function must be called when task still owns bus, it means that application code
 * calls \sa ad_uart_bus_acquire() explicitly before calling \sa ad_uart_async_read().
 * After calling this function application code can release bus with \sa ad_uart_bus_release().
 *
 * \param [in] dev handle to UART device
 *
 * \return number for successfully read characters
 *
 * \sa ad_uart_read_async()
 * \sa ad_uart_bus_acquire()
 * \sa ad_uart_bus_release()
 *
 */
int ad_uart_complete_async_read(uart_device dev);

/**
 * \brief Abort previously started asynchronous read
 *
 * This function aborts asynchronous read started with \sa ad_uart_read_async().
 * To avoid unpredictable results ad_uart_bus_acquire() must be called before ad_uart_read_async(),
 * because ad_uart_bus_release() can be called from interrupt fired after user code decided to
 * abort read operation. In that case ad_uart_bus_release() could already release uart for other
 * tasks and calling abort would interfere with next read.
 * If resource is blocked there is no risk of calling abort when read is finishing. User callback
 * will be called only once.
 *
 * \param [in] dev handle to UART device
 *
 * \sa ad_uart_read_async()
 * \sa ad_uart_bus_release()
 *
 */
void ad_uart_abort_read_async(uart_device dev);

/**
 * \brief Acquire access to UART bus
 *
 * This function waits for UART bus to be available, and locks it for current task's use only.
 * This function can be called several times, but number of ad_uart_bus_release() calls must match
 * number of calls to this function.
 *
 * This function should be used if normal ad_uart_read(), ad_uart_write() are not enough
 * and some other UART controller calls are required. In this case typical usage for this function
 * would look like this:
 *
 * ad_uart_bus_acquire(dev);
 * id = ad_uart_get_hw_uart_id(dev);
 * ...
 * hw_uart_set...(id, ...);
 * hw_uart_write_buffer(id, ...)
 * ...
 * ad_uart_bus_release(dev);
 *
 * \param [in] dev handle to UART device
 *
 */
void ad_uart_bus_acquire(uart_device dev);

/**
 * \brief Release access to UART bus
 *
 * This function decrements acquire counter for this device and when it reaches 0 UART bus is
 * released and can be used by other tasks.
 *
 * \param [in] dev handle to UART device
 *
 * \sa ad_uart_bus_acquire
 */
void ad_uart_bus_release(uart_device dev);

/**
 * \brief Acquire specific UART resource and its DMA counterpart.
 *
 * This is similar to ad_uart_bus_acquire(), but acquires only a specific UART resource.
 *
 * \param [in] dev handle to UART device
 *
 * \param [in] res_type resource type
 */
void ad_uart_bus_acquire_ex(uart_device dev, AD_UART_RES_TYPE res_type);

/**
 * \brief Release specific UART resource and its DMA counterpart.
 *
 * This is similar to ad_uart_bus_release(), but releases only a specific UART resource.
 *
 * \param [in] dev handle to UART device
 *
 * \param [in] res_type resource type
 */
void ad_uart_bus_release_ex(uart_device dev, AD_UART_RES_TYPE res_type);

/**
 * \brief Get UART controller id
 *
 * This function returns id that can be used to get UART controller id. This id is argument
 * for lower level functions starting with hw_uart_ prefix.
 *
 * \param [in] dev handle to UART device
 *
 * \return id that can be used with hw_uart_... functions
 */
HW_UART_ID ad_uart_get_hw_uart_id(uart_device dev);

#if dg_configUART_SOFTWARE_FIFO
/**
 * \brief Set software fifo
 *
 * This function configures UART to use software FIFO. This allows to receive data when there is
 * no active read in progress.
 *
 * \param [in] dev handle to UART device
 * \param [in] buf buffer to use as software FIFO
 * \param [in] size software FIFO size
 *
 */
void ad_uart_set_soft_fifo(uart_device dev, uint8_t *buf, uint8_t size);
#endif

/**
 * \brief UART device data run time data
 *
 * Variables of this type are automatically generated by \sa UART_BUS() macro.
 * Structure keeps runtime data related to UART.
 *
 */
typedef struct {
        /** Event used for synchronization in accessing UART controller for sending data. */
        OS_EVENT event_write;
        /** Event used for synchronization in accessing UART controller for receiving data. */
        OS_EVENT event_read;
#if CONFIG_UART_USE_RESMGMT
        struct {
                /** Task that acquired this resource */
                OS_TASK owner;

                /** This keeps track of number of acquisitions for this resource */
                int8_t bus_acquire_count;
        } res_states[AD_UART_RES_TYPES];
#endif
        int8_t open_count;     /**< Open count */
        ad_uart_user_cb read_cb;  /**< User function to call after asynchronous read finishes */
        ad_uart_user_cb write_cb; /**< User function to call after asynchronous write finishes */
        void *read_cb_data;    /**< Data to pass to read_cb */
        void *write_cb_data;   /**< Data to pass to write_cb */
        uint8_t cts_pin;       /**< Port (high nibble) and pin (low nibble) for CTS */
        uint16_t read_cnt;     /**< Number of bytes read in last async read */
#if dg_configUART_RX_CIRCULAR_DMA
        bool use_rx_circular_dma; /** true if UART is using circular DMA on RX */
        void *read_cb_ptr;        /** original pointer passed to read, used only with circular DMA */
#endif
} uart_bus_dynamic_data;

/**
 * \brief UART bus constant data
 *
 * Variable of this type keeps static configuration needed to configure UART bus.
 * Those variables are generated by UART_BUS().
 *
 */
typedef struct uart_device_config {
        HW_UART_ID bus_id;              /**< UART id as needed by hw_uart_... functions */
        RES_ID bus_res_id;              /**< UART resource ID RES_ID_UART1 or RES_ID_UART2 */
        uart_config_ex hw_init;         /**< UART hardware configuration to use */
        uart_bus_dynamic_data *bus_data; /**< pointer to dynamic bus data */
        AD_UART_DEVICE_FLAGS flags;     /**< Device flags */
} uart_device_config;

#ifdef __cplusplus
}
#endif

#endif /* dg_configUART_ADAPTER */

#endif /* AD_UART_H_ */

/**
 * \}
 * \}
 * \}
 */

