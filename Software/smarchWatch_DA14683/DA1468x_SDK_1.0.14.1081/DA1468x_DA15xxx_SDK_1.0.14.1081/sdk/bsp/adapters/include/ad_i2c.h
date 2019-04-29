/**
 * \addtogroup BSP
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup I2C_ADAPTER
 *
 * \brief I2C adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_i2c.h
 *
 * @brief I2C device access API
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_I2C_H_
#define AD_I2C_H_

#if dg_configI2C_ADAPTER

#include <osal.h>
#include <resmgmt.h>
#include <hw_i2c.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \def CONFIG_I2C_ONE_DEVICE_ON_BUS
 *
 * \brief Macro to configure only one device on the I2C bus
 *
 * Set this macro to 1 if only one I2C device exists on the bus to reduce code size and
 * improve performance.
 */
#ifndef CONFIG_I2C_ONE_DEVICE_ON_BUS
# define CONFIG_I2C_ONE_DEVICE_ON_BUS   (0)
#endif

/**
 * \def CONFIG_I2C_EXCLUSIVE_OPEN
 *
 * \brief Macro to configure exclusive use of devices
 *
 * Set this macro to 1 in order to enable preventing multiple tasks opening the
 * same device. When set to 1, ad_i2c_device_acquire() and ad_i2c_device_release()
 * are no longer necessary.
 *
 * \note The setting of this macro is irrelevant if \ref CONFIG_I2C_USE_RESMGMT is unset.
 */
#ifndef CONFIG_I2C_EXCLUSIVE_OPEN
# define CONFIG_I2C_EXCLUSIVE_OPEN      (0)
#endif

/**
 * \def CONFIG_I2C_RESOURCE_STATIC_ID
 *
 * \brief Macro to configure resource ID assignment for each device
 *
 * Set this macro to 1 to enable unique resource ID assignment per device.
 */
#ifndef CONFIG_I2C_RESOURCE_STATIC_ID
# define CONFIG_I2C_RESOURCE_STATIC_ID  (0)
#endif

/**
 * \def CONFIG_I2C_ENABLE_CRITICAL_SECTION
 *
 * \brief Enable critical sections within some I2C adapter functions
 *
 * Some I2C peripherals do not respond well to a write-read transaction if a STOP
 * condition occurs after the write operation. By setting this macro to 1, ad_i2c_transact()
 * will execute the write and the setup of read within a critical section, thus
 * ensuring that no STOP condition will occur after write due to I2C FIFO underflow.
 */
#ifndef CONFIG_I2C_ENABLE_CRITICAL_SECTION
# define CONFIG_I2C_ENABLE_CRITICAL_SECTION  (1)
#endif

/**
 * \def CONFIG_I2C_USE_ASYNC_TRANSACTIONS
 *
 * \brief Controls whether I2C asynchronous transaction API will be used
 *
 * I2C asynchronous transaction API (see "ad_i2c_async_*" API) maintains state in
 * retention RAM for every I2C bus declared. If the API is not to be used, setting this macro 
 * to 0 will save retention RAM.
 */
#ifndef CONFIG_I2C_USE_ASYNC_TRANSACTIONS
# define CONFIG_I2C_USE_ASYNC_TRANSACTIONS	  (1)
#endif

/**
 * \def CONFIG_I2C_USE_RESMGMT
 *
 * \brief Controls whether I2C resource acquisition will be used
 *
 * I2C adapter provides resource access management in order to protect I2C resources (devices and
 * buses) from being used by multiple tasks simultaneously. Unsetting this macro will remove this
 * feature, improving performance and potentially reducing code size (if the resource management API
 * is not used by other modules). All acquisition and release I2C adapter calls omit calls to
 * the resource management API when this this macro is unset.
 *
 * \note Unsetting this macro will not affect DMA resource management see \ref
 * CONFIG_I2C_USE_DMA_RESMGMT.
 */
#ifndef CONFIG_I2C_USE_RESMGMT
# define CONFIG_I2C_USE_RESMGMT              (1)
#endif

/**
 * \def CONFIG_I2C_USE_DMA_RESMGMT
 *
 * \brief Controls whether DMA resource acquisition will be used by the I2C adapter
 *
 * I2C adapter provides DMA resource access management in order to protect DMA resources from being
 * used by multiple tasks simultaneously. Unsetting this macro will remove this feature, improving
 * performance and potentially reducing code size (if the resource management API is not used by
 * other modules). Unset this macro if, for example, you have exclusively assigned DMA channels to
 * the I2C interface.
 */
#ifndef CONFIG_I2C_USE_DMA_RESMGMT
# define CONFIG_I2C_USE_DMA_RESMGMT          (1)
#endif

/**
 * \brief Device pointer, handle to use with ad_i2c_read(), ad_i2c_write() etc.
 *
 */
typedef void *i2c_device;

/**
 * \brief Device id, those are created by I2C_SLAVE_DEVICE or I2C_SLAVE_TO_EXT_MASTER macro in
 * platform_devices.h
 *
 */
typedef const void *i2c_device_id;

#ifndef I2C_SLAVE_DEVICE

/**
 * \brief Entry for slave device
 *
 * \param [in] bus_id value must match I2C_BUS() argument valid values: I2C1, I2C2
 * \param [in] name name that will be used later to open device
 * \param [in] addr slave device address
 * \param [in] addr_mode slave device address mode from HW_I2C_ADDRESSING enum
 * \param [in] speed I2C clock speed from HW_I2C_SPEED enum
 *
 */
#define I2C_SLAVE_DEVICE(bus_id, name, addr, addr_mode, speed) \
                                I2C_SLAVE_DEVICE_DMA(bus_id, name, addr, addr_mode, speed, -1)

/**
 * \brief Entry for slave device with DMA
 *
 * \param [in] bus_id value must match I2C_BUS() argument valid values: I2C1, I2C2
 * \param [in] name name that will be used later to open device
 * \param [in] addr slave device address
 * \param [in] addr_mode slave device address mode from HW_I2C_ADDRESSING enum
 * \param [in] speed I2C clock speed from HW_I2C_SPEED enum
 * \param [in] dma_channel DMA channel to use, -1 for no DMA
 *
 */
#define I2C_SLAVE_DEVICE_DMA(bus_id, name, addr, addr_mode, speed, dma_channel) \
        extern const void *const name;

/**
 * \brief Entry for device representing I2C controller in slave mode
 *
 * \param [in] bus_id value must match I2C_BUS() argument valid values: I2C1, I2C2
 * \param [in] name name that will be used later to open device
 * \param [in] addr slave device address
 * \param [in] addr_mode slave device address mode from HW_I2C_ADDRESSING enum
 * \param [in] speed I2C clock speed from HW_I2C_SPEED enum
 * \param [in] dma_channel DMA channel to use, -1 for no DMA
 *
 * \note Slave mode is currently unsupported.
 */
#define I2C_SLAVE_TO_EXT_MASTER(bus_id, name, addr, addr_mode, speed, dma_channel) \
        extern const void *const name;

/**
 * \brief Starting entry for I2C bus devices
 *
 * \param [in] bus_id identifies I2C bus I2C1 or I2C2
 *
 */
#define I2C_BUS(bus_id) \
        extern i2c_bus_dynamic_data dynamic_##bus_id;

/**
 * \brief This ends I2C bus device list started with I2C_BUS()
 *
 */
#define I2C_BUS_END

/**
 * \brief Initialization of I2C bus variables
 *
 * This macro must be invoked somewhere during system startup to initialize variables needed to
 * manage I2C bus. It will create some OS specific synchronization primitives.
 * Each bus created with I2C_BUS() must have corresponding call to this macro.
 * If both I2Cs are used somewhere in initialization sequence must be:
 *      I2C_BUS_INIT(I2C1);
 *      I2C_BUS_INIT(I2C2);
 *
 * \param [in] bus_id identifies I2C bus I2C or I2C
 *
 */
#define I2C_BUS_INIT(bus_id) ad_i2c_bus_init(&dynamic_##bus_id)

/**
 * \brief Initialization of I2C bus devices
 *
 * This macro must be invoked somewhere during system startup to initialize variables needed to
 * manage access to I2C devices.
 * This is important if CONFIG_I2C_RESOURCE_STATIC_ID is defined as 0, because resource
 * id required for device access needs to be created at some point.
 * Each device created with I2C_SLAVE_DEVICE() and I2C_SLAVE_TO_EXT_MASTER() must have
 * corresponding call to this macro.
 * Example:
 * \code{.c}
 *      I2C_BUS_INIT(I2C1);
 *      I2C_DEVICE_INIT(MEM_24LC256);
 * \endcode
 *
 * \param [in] name identifies I2C device connected to bus
 *
 */
#define I2C_DEVICE_INIT(name) ad_i2c_device_init(name)

#endif

/**
 * \brief Asynchronous callback function
 *
 */
typedef void (*ad_i2c_user_cb)(void *user_data, HW_I2C_ABORT_SOURCE error);

/**
 * \brief Initialize adapter
 *
 */
void ad_i2c_init(void);

/**
 * \brief Initialize bus variables
 *
 * \warning Don't call this function directly use I2C_BUS_INIT macro.
 *
 */
void ad_i2c_bus_init(void *bus_data);

/**
 * \brief Initialize device variables
 *
 * \warning Don't call this function directly use I2C_DEVICE_INIT macro.
 *
 */
void ad_i2c_device_init(const i2c_device_id id);

/**
 * \brief Open device connected to I2C bus
 *
 * If system is configured for single device on each I2C bus
 * (defined CONFIG_I2C_ONE_DEVICE_ON_BUS as 1)
 * This function will setup configuration parameters to I2C controller.
 * If bus is not dedicated to single device only, configuration is performed during call to
 * ad_i2c_bus_acquire().
 *
 * \param [in] dev_id identifier of device connected to I2C bus (name should match entries defined
 *             by I2C_SLAVE_DEVICE or I2C_SLAVE_TO_EXT_MASTER
 *
 * \return device handle that can be used with other functions
 */
i2c_device ad_i2c_open(i2c_device_id dev_id);

/**
 * \brief Close I2C device
 *
 * \param [in] device handle to device opened with ad_i2c_open()
 *
 * \sa ad_i2c_open()
 *
 */
void ad_i2c_close(i2c_device device);

/**
 * \brief Perform an I2C read after write transaction
 *
 * This function performs a write transaction followed by a read transaction, an operation
 * which is typical when reading data from I2C peripherals where an address needs to be specified
 * through a write, before reading the data. The function will first wait for the device and bus
 * resources to become available. It then proceeds with the write, without waiting for the
 * STOP condition. If no error occurs by the time the last byte is placed in the transmit FIFO
 * the function continues with the read operation and waits until it is completed.
 *
 * Example usage:
 * \code{.c}
 * {
 *   i2c_device dev = ad_i2c_open(PRESSURE_SENSOR);
 *   while (1) {
 *     ad_i2c_transact(dev, command, sizeof(command), response, sizeof(response));
 *     ...
 *   }
 * }
 * \endcode
 *
 * \param [in] dev handle to I2C device
 * \param [in] wbuf buffer containing the data to be sent to the device
 * \param [in] wlen size of data to be sent to the device
 * \param [out] rbuf buffer to store the data read from the device
 * \param [in] rlen size of buffer pointed by rbuf
 *
 * \return 0 on success, value from HW_I2C_ABORT_SOURCE enum on failure
 *
 * \sa ad_i2c_open()
 * \sa ad_i2c_close()
 *
 * \warning If #CONFIG_I2C_ENABLE_CRITICAL_SECTION is enabled, it must be noted that
 *          the time within the critical section depends on the value of \a wlen
 *
 */
int ad_i2c_transact(i2c_device dev, const uint8_t *wbuf, size_t wlen, uint8_t *rbuf,
                                                                                size_t rlen);

/**
 * \brief Perform write only transaction
 *
 * This function performs a write only transaction on I2C bus. It waits first for device and bus
 * resources to become available and then it will wait until the write transaction is completed
 * and the STOP condition is detected.
 *
 * \param [in] dev handle to I2C device
 * \param [in] wbuf buffer containing the data to be sent to the device
 * \param [in] wlen size of data to be sent to the device
 *
 * \return 0 on success, value from HW_I2C_ABORT_SOURCE enum on failure
 *
 * \sa ad_i2c_open()
 * \sa ad_i2c_close()
 *
 */
int ad_i2c_write(i2c_device dev, const uint8_t *wbuf, size_t wlen);

/**
 * \brief Perform read only transaction
 *
 * This function performs a read only transaction on I2C bus. It waits first for device and bus
 * resources to become available and then it will wait until the read transaction is completed.
 *
 * \param [in] dev handle to I2C device
 * \param [out] rbuf buffer to store the data read from the device
 * \param [in] rlen size of buffer pointed by rbuf
 *
 * \return 0 on success, value from HW_I2C_ABORT_SOURCE enum on failure
 *
 * \sa ad_i2c_open()
 * \sa ad_i2c_close()
 *
 */
int ad_i2c_read(i2c_device dev, uint8_t *rbuf, size_t rlen);

/**
 * \brief Acquire access to I2C bus
 *
 * This function waits for I2C bus to available, and locks it for \p dev's use only.
 * This function can be called several times, but number of ad_i2c_bus_release() calls must match
 * number of calls to this function.
 *
 * This function should be used if normal ad_i2c_transact(), ad_i2c_read(), ad_i2c_write() are not
 * enough and some other I2C controller calls are required. In this case typical usage for this
 * function would look like this:
 *
 * \code{.c}
 * ad_i2c_bus_acquire(dev);
 * id = ad_i2c_get_hw_i2c_id(dev);
 * ...
 * hw_i2c_set...(id, ...);
 * hw_i2c_write_buffer(id, ...)
 * ...
 * ad_i2c_bus_release(dev);
 * \endcode
 *
 * \param [in] dev handle to I2C device
 *
 * \warning The device must be already acquired through ad_i2c_device_acquire() before calling
 *          this function.
 * \warning This function shouldn't be used for Slave I2C devices when dg_configI2C_ADAPTER_SLAVE_SUPPORT
 *          is enabled. Its functionality is included in ad_i2c_start_slave() function.
 *
 * \sa ad_i2c_start_slave
 */
void ad_i2c_bus_acquire(i2c_device dev);

/**
 * \brief Release access to I2C bus
 *
 * This function decrements acquire counter for this device and when it reaches 0 I2C bus is
 * released and can be used by other devices.
 *
 * \param [in] dev handle to I2C device
 *
 * \warning This function shouldn't be used for Slave I2C devices when dg_configI2C_ADAPTER_SLAVE_SUPPORT
 *          is enabled. Its functionality is included in ad_i2c_stop_slave() function.
 *
 * \sa ad_i2c_stop_slave
 * \sa ad_i2c_bus_acquire
 * \sa dg_configI2C_ADAPTER_SLAVE_SUPPORT
 *
 */
void ad_i2c_bus_release(i2c_device dev);

/**
 * \brief Acquire access to I2C device
 *
 * This function waits for device to be available, and locks it for current task use only.
 * This function can be called several times, but number of ad_i2c_device_release() calls must match
 * number of calls to this function.
 *
 * This function should be used if several transaction on this device should not be interrupted.
 * If writing to part of flash memory require reading and erasing, call to ad_i2c_device_acquire()
 * will reserve device for current task not necessarily blocking whole I2C bus.
 *
 * \code{.c}
 * ad_i2c_device_acquire(dev);
 * ...
 * ad_i2c_transact(id, read_page_commad...);
 * ad_i2c_write(id, write_enable_command...);
 * ad_i2c_transact(id, erase_page_command...);
 * ad_i2c_write(id, write_enable_command...);
 * ad_i2c_transact(id, write_page_command...);
 * ...
 * ad_i2c_device_release(dev);
 * \endcode
 *
 * \param [in] dev handle to I2C device
 *
 * \warning This function shouldn't be used for Slave I2C devices when dg_configI2C_ADAPTER_SLAVE_SUPPORT
 *          is enabled. Its functionality is included in ad_i2c_start_slave() function.
 *
 * \sa ad_i2c_start_slave
 * \sa ad_i2c_device_release
 * \sa CONFIG_I2C_EXCLUSIVE_OPEN
 * \sa dg_configI2C_ADAPTER_SLAVE_SUPPORT
 *
 */
void ad_i2c_device_acquire(i2c_device dev);

/**
 * \brief Release access to I2C device
 *
 * This function decrements acquire counter for this device and when it reaches 0 device is
 * released and can be used by other tasks.
 *
 * \param [in] dev handle to I2C device
 *
 * \warning This function shouldn't be used for Slave I2C devices when dg_configI2C_ADAPTER_SLAVE_SUPPORT
 *          is enabled. Its functionality is included in ad_i2c_stop_slave() function.
 *
 * \sa ad_i2c_stop_slave
 * \sa ad_i2c_device_acquire
 * \sa CONFIG_I2C_EXCLUSIVE_OPEN
 * \sa dg_configI2C_ADAPTER_SLAVE_SUPPORT
 *
 */
void ad_i2c_device_release(i2c_device dev);

/**
 * \brief Get I2C controller id
 *
 * This function returns id that can be used to get I2C controller id. This id is argument
 * for lower level functions starting with hw_i2c_ prefix.
 *
 * \param [in] dev handle to I2C device
 *
 * \return id that can be used with hw_i2c_... functions
 *
 */
HW_I2C_ID ad_i2c_get_hw_i2c_id(i2c_device dev);

/**
 * \brief Get current device for I2C controller
 *
 * This function returns i2c_device that is currently configured in I2C controller.
 * Function is intended to use to retrieve i2c_device from interrupt context where
 * only HW_I2C_ID is passed from hw_i2c driver.
 *
 * \param [in] id identifier of I2C bus (HW_I2C1 or HW_I2C2)
 *
 * \return device that was used during ad_i2c_bus_acquire() function
 *
 */
i2c_device ad_i2c_get_device_by_hw_id(HW_I2C_ID id);

#ifndef I2C_ASYNC_ACTIONS_SIZE
/*
 * 10 elements should be enough for normal read, read/write transactions with one callback
 * 11 is enough for read, read/write with two callbacks
 */
#define I2C_ASYNC_ACTIONS_SIZE 11
#endif

/**
 * \brief I2C data run time data
 *
 * Variables of this type are automatically generated by \sa I2C_BUS() macro.
 * Structure keeps data related to I2C controller that allows easy usage of bus by several
 * tasks and devices connected to I2C controller
 *
 */
typedef struct {
        OS_EVENT event;        /**< Event used for synchronization in accessing I2C controller */
#if !CONFIG_I2C_ONE_DEVICE_ON_BUS
        struct i2c_device_config_t *current_device; /**< This keeps track of last device that was
                                 used on I2C bus. When device changes all controller parameters must
                                 be also changed. If CONFIG_I2C_ONE_DEVICE_ON_BUS is defined as 1
                                 there is no need to check/update controller settings at every
                                 transfer so this field is not needed */
#endif
#if CONFIG_I2C_USE_ASYNC_TRANSACTIONS
        uint8_t transaction_ix;
        uint32_t transaction[I2C_ASYNC_ACTIONS_SIZE];
#endif
} i2c_bus_dynamic_data;

#define I2C_TAG_CALLBACK0       0xFE000000
#define I2C_TAG_CALLBACK1       0xFD000000
#define I2C_TAG_SEND            0xFC000000
#define I2C_TAG_RECEIVE         0xFB000000
#define I2C_TAG_SEND_STOP       0xFA000000
#define I2C_TAG_RECEIVE_STOP    0xF9000000

#if CONFIG_I2C_USE_ASYNC_TRANSACTIONS
/*
 * The following macros are used to construct asynchronous transactions on I2C device.
 */

/**
 * \brief Send data and wait until all data are placed in FIFO
 *
 * \sa ad_i2c_async_transact
 */
#define I2C_SND(wbuf, len)      (uint32_t) ((len) | I2C_TAG_SEND), (uint32_t) (wbuf)

/**
 * \brief Send data and wait until STOP condition is detected
 *
 * \sa ad_i2c_async_transact
 */
#define I2C_SND_ST(wbuf, len)   (uint32_t) ((len) | I2C_TAG_SEND_STOP), (uint32_t) (wbuf)

/**
 * \brief Receive data, generate stop condition after last byte
 *
 * \sa ad_i2c_async_transact
 */
#define I2C_RCV(rbuf, len)      (uint32_t) ((len) | I2C_TAG_RECEIVE_STOP), (uint32_t) (rbuf)

/**
 * \brief Receive data, do not generate stop condition
 *
 * \sa ad_i2c_async_transact
 */
#define I2C_RCV_NS(rbuf, len)   (uint32_t) ((len) | I2C_TAG_RECEIVE), (uint32_t) (rbuf)

/**
 * \brief Callback to call after transaction completes
 *
 * \sa ad_i2c_async_transact
 */
#define I2C_CB(cb)              (uint32_t) I2C_TAG_CALLBACK0, (uint32_t) (cb)

/**
 * \brief Callback with arguments to call after transaction completes
 *
 * \sa ad_i2c_async_transact
 */
#define I2C_CB1(cb, arg)        (uint32_t) I2C_TAG_CALLBACK1, (uint32_t) (cb), (uint32_t) (arg)

/**
 * \brief Mark end of transactions
 *
 * \sa ad_i2c_async_transact
 */
#define I2C_END                 (uint32_t) 0

/**
 * \brief Start asynchronous I2C transaction
 *
 * Arguments are actions that should be taken to perform full transaction.
 *
 * It is important to know that transaction starts from acquiring device and I2C bus and this is
 * done synchronously meaning that function can wait for device and bus access.
 * It is possible to acquire device and bus in advance and then ad_i2c_async_transact() will not
 * block.
 * Buffers passed to this function should not be reused before final callback is called.
 *
 * I2C adapter allows to create asynchronous transaction that consists of a number of reads, writes,
 * and callback calls.
 * This allows to create a time efficient way to manage all I2C related actions. Most of the actions
 * will be executed in ISR context.
 * These are the possible asynchronous transactions, passed in a form of arguments:
 *
 *  Action                       |  Macro
 * ----------------------------- | ------------------
 *  Send data                    |  I2C_SND
 *  Send data and wait for STOP  |  I2C_SND_ST
 *  Receive data                 |  I2C_RCV
 *  Callback execution           |  I2C_CB
 *  Callback with data           |  I2C_CB1
 *  Mark end of transactions     |  I2C_END
 *
 * Typical I2C transaction steps:
 * sending command/receiving response
 *
 * Asynchronous I2C transaction allows to build this sequence, start it and wait for final callback
 * after everything is done.
 *
 * Example usage:
 * \code{.c}
 * {
 *   i2c_device dev = ad_i2c_open(PRESSURE_SENSOR);
 *   while (1) {
 *     ad_i2c_async_transact(dev, I2C_SND(command, sizeof(command)),
 *                                I2C_RCV(response, sizeof(response),
 *                                I2C_CB(final_callback),
 *                                I2C_END);
 *     ...
 *
 *     // Wait here for final callback notification
 *
 *     // Do something with the response
 *
 *     ...
 *   }
 * }
 * \endcode
 *
 * \param [in] dev handle to I2C device
 *
 * \note Callbacks are called from within I2C ISRs.
 *
 * \note If the callback is the last action, resources (device and bus) will be released before
 *       the callback is called.
 *
 * \warning Do not call this function consecutively without guaranteeing that the previous
 *          async transaction has completed.
 *
 * \warning After the callback is called, it is not guaranteed that the scheduler will give
 *          control to the task waiting for this transaction to complete. This is important to
 *          consider if more than one tasks are using this API.
 *
 */
void ad_i2c_async_transact(i2c_device dev, ...);

/**
 * \brief Start asynchronous write I2C transaction
 *
 * This is convenience macro that builds typical write only transaction and executes it.
 * After all is done and stop condition is detected user callback is executed.
 *
 * \param [in] dev handle to I2C device
 * \param [in] wbuf data to send
 * \param [in] wlen number of bytes to write
 * \param [in] cb callback to call after transaction is over (from ISR context)
 * \param [in] ud user data to pass to \p cb
 *
 * \note The callback is called from within I2C ISR and at that time resources
 *       (device and bus) are released
 *
 * \warning Do not call this function consecutively without guaranteeing that the previous
 *          async transaction has completed.
 *
 * \warning After the callback is called, it is not guaranteed that the scheduler will give
 *          control to the task waiting for this transaction to complete. This is important to
 *          consider if more than one tasks are using this API.
 */
#define ad_i2c_async_write(dev, wbuf, wlen, cb, ud) \
        ad_i2c_async_transact(dev, I2C_SND_ST(wbuf, wlen), I2C_CB1(cb, ud), I2C_END)

/**
 * \brief Deprecated alias for \ref ad_i2c_async_write.
 *
 * \deprecated Use \ref ad_i2c_async_write() directly.
 */
#define i2c_async_write(dev, wbuf, wlen, cb, ud) \
        ad_i2c_async_write(dev, wbuf, wlen, cb, ud)

 /**
  * \brief Start asynchronous read I2C transaction
  *
  * This is convenience macro that builds read transaction and executes it.
  * After all is done user callback is executed.
  *
  * \param [in] dev handle to I2C device
  * \param [out] rbuf buffer for incoming data
  * \param [in] rlen number of bytes to read
  * \param [in] cb callback to call after transaction is over (from ISR context)
  * \param [in] ud user data passed to cb callback
  *
  * Example usage:
  * \code{.c}
  * {
  *   i2c_device dev = ad_i2c_open(PRESSURE_SENSOR);
  *   while (1) {
  *     ad_i2c_async_read(dev, response, sizeof(response), final_callback, cb_arg);
  *
  *     // wait here for final callback notification
  *     // do something with response
  *     ...
  *   }
  * }
  * \endcode
  *
  * \note The callback is called from within I2C ISR and at that time resources
  *       (device and bus) are released
  *
  * \warning Do not call this function consecutively without guaranteeing that the previous
  *          async transaction has completed.
  *
  * \warning After the callback is called, it is not guaranteed that the scheduler will give
  *          control to the task waiting for this transaction to complete. This is important to
  *          consider if more than one tasks are using this API.
  */
 #define ad_i2c_async_read(dev, rbuf, rlen, cb, ud) \
         ad_i2c_async_transact(dev, I2C_RCV(rbuf, rlen), I2C_CB1(cb, ud), I2C_END)

#endif /* CONFIG_I2C_USE_ASYNC_TRANSACTIONS */
/**
 * \brief I2C device data run time data
 *
 * Variables of this type are automatically generated by \sa I2C_SLAVE_DEVICE() and
 * \sa I2C_SLAVE_TO_EXT_MASTER() macros.
 * Structure keeps data related to device connected to I2C controller.
 *
 */
typedef struct {
#if CONFIG_I2C_USE_RESMGMT
#if !CONFIG_I2C_ONE_DEVICE_ON_BUS
        int8_t bus_acquire_count; /**< this keeps track of number of calls to ad_i2c_bus_acquire() */
#endif
#if !CONFIG_I2C_EXCLUSIVE_OPEN
        int8_t dev_acquire_count; /**< Number of device_acquire calls */
        OS_TASK owner;            /**< Task that acquired this device */
#endif

#if !CONFIG_I2C_RESOURCE_STATIC_ID
        RES_ID dev_res_id;        /**< dynamically created (\sa resource_add()) resource ID for this
                                  device */
#endif
#endif /* CONFIG_I2C_USE_RESMGMT */
} i2c_dev_dynamic_data;

#if dg_configI2C_ADAPTER_SLAVE_SUPPORT

typedef void (* ad_i2c_slave_event)(i2c_device dev, void *user_data);
typedef void (* ad_i2c_slave_data_event)(i2c_device dev, uint16_t len, bool success, void *user_data);

/**
 * \brief Slave event callbacks.
 *
 * A callback field can be NULL (i.e. no callback configured). All callbacks are executed in the
 * corresponding I2C ISR context.
 */
typedef struct {
        ad_i2c_slave_data_event data_sent;      /**< called after data from output_buffer is sent */
        ad_i2c_slave_data_event data_received;  /**< called after data input_buffer is filled */

        ad_i2c_slave_event data_ready;          /**< called when data arrived but there is no input_buffer */
        ad_i2c_slave_event read_request;        /**< called when master wants to read but there is no output_buffer */
} i2c_dev_slave_event_callbacks;

/**
 * \brief Slave state bits.
 */
typedef enum {
        /** Slave stopped or uninitialized.*/
        AD_I2C_SLAVE_STATE_STOPPED = 0,

        /** Initial state. */
        AD_I2C_SLAVE_STATE_INIT = 0x1,

        /** Slave read pending. */
        AD_I2C_SLAVE_STATE_READ_PENDING = 0x2,

        /** Slave write pending. */
        AD_I2C_SLAVE_STATE_WRITE_PENDING = 0x4,
} AD_I2C_SLAVE_STATE;

/**
 * \brief Slave state data.
 */
typedef struct {
        i2c_dev_dynamic_data i2c;               /**< Base part common for master and slave */

        const i2c_dev_slave_event_callbacks *event_callbacks;
        void *user_data;                        /**< User data to pass to all callbacks */
        const uint8_t *output_buffer;           /**< Data to sent when master wants to read */
        uint16_t output_buffer_len;
        uint8_t *input_buffer;                  /**< Buffer for data in case master writes */
        uint16_t input_buffer_len;

        /** State to support read/write or write/read operations with ad_i2c_start_slave(). */
        AD_I2C_SLAVE_STATE state;
        /** Event used for notification when slave's read or write is completed */
        OS_EVENT operation_done_event;
} i2c_dev_dynamic_data_slave;


/**
 * \brief Start slave transmission/reception
 *
 * When I2C is configured in slave mode, this function sets up input and/or output buffers
 * to use for master initiated transmission and/or reception. It also specifies user callbacks that
 * will be called when transmission or reception starts or finishes.
 *
 * If the user specifies valid (wbuf, wlen) pair, data will be sent on incoming read request from
 * master. After reception, data_sent() callback will be called.
 * If wbuf is NULL or wlen is 0, read_request() callback will be called to notify the user about
 * master read request.
 *
 * If (rbuf, rlen) pair is specified, data will be received when master starts writing. When data is
 * received, callback data_received() will be called.
 * If rbuf is NULL or rlen is 0, data_ready() callback will be called to notify user about master
 * write. If the user fails to read from the Rx FIFO before it becomes full then data loss may
 * occur.
 *
 *
 *
 * \param [in] dev handle to I2C device
 * \param [in] wbuf buffer for outgoing data
 * \param [in] wlen number of bytes to write in case master wants to read
 * \param [out] rbuf buffer for incoming data
 * \param [in] rlen number of bytes to read
 * \param [in] events structure with callback to call after transaction is over (from ISR context), if
 * NULL, no callbacks will be called.
 * \param [in] user_data user data to pass to each callback in events
 *
 * Example usage:
 * \code{.c}
 * {
 *   i2c_device dev = ad_i2c_open(I2C_IN_SLAVE_MODE);
 *   while (1) {
 *     uint8_t cmd[4];
 *     uint8_t response[6];
 *
 *     ad_i2c_start_slave(dev, NULL, 0, cmd, sizeof(cmd), slave_callbacks, NULL);
 *
 *     // wait while callbacks handle write request
 *     ...
 *     // prepare response in out buffer
 *     ad_i2c_start_slave(dev, response, sizeof(response), NULL, 0, slave_callbacks, NULL);
 *     ...
 *     // wait for master to read response
 *     ...
 *     ad_i2c_stop_slave(dev);
 *   }
 * }
 * \endcode
 *
 * \note All callbacks are called from within I2C ISR
 *
 */
void ad_i2c_start_slave(i2c_device dev, const uint8_t *wbuf, uint16_t wlen, uint8_t *rbuf,
                uint16_t rlen, const i2c_dev_slave_event_callbacks *events, void *user_data);

/**
 * \brief Stop slave response
 *
 * This function will make a slave device stop responding to external master requests for read or
 * write operations. If such an operation is ongoing, the function will wait for its completion
 * before returning. The bus and device are released by this function.
 *
 * \param [in] dev handle to I2C device
 *
 * \warning This function should only be called only if ad_i2c_start_slave() has been already used
 *          for starting the slave operation.
 *
 * \sa ad_i2c_start_slave()
 *
 */
void ad_i2c_stop_slave(i2c_device dev);

#endif

/**
 * \brief I2C device constant data
 *
 * Variable of this type keeps static configuration needed to access device on I2C bus.
 * Those variables are generated by \sa I2C_SLAVE_DEVICE() and \sa I2C_SLAVE_TO_EXT_MASTER() macros.
 *
 */
typedef struct i2c_device_config_t {
        HW_I2C_ID bus_id;               /**< I2C id as needed by hw_i2c_... functions */
#if CONFIG_I2C_USE_RESMGMT
        RES_ID bus_res_id;              /**< I2C resource ID RES_ID_I2C1 or RES_ID_I2C2 */
#endif
        int8_t dma_channel;             /**< DMA channel for I2C */
        i2c_config hw_init;             /**< I2C hardware configuration */
        i2c_bus_dynamic_data *bus_data; /**< pointer to dynamic bus data */
        i2c_dev_dynamic_data *data;     /**< pointer to dynamic device data */
#if CONFIG_I2C_RESOURCE_STATIC_ID && CONFIG_I2C_USE_RESMGMT
        RES_ID dev_res_id;              /**< If CONFIG_I2C_RESOURCE_STATIC_ID is defined, RES_ID contains
                                         value for this device and it can be stored in constant
                                         data instead of dynamic */
#endif
} i2c_device_config;

#ifdef __cplusplus
}
#endif

#endif /* dg_configI2C_ADAPTER */

#endif /* AD_I2C_H_ */

/**
 * \}
 * \}
 * \}
 */
