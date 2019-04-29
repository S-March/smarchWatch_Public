/**
 * \addtogroup BSP
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup SPI_ADAPTER
 *
 * \brief SPI adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_spi.h
 *
 * @brief SPI adapter API
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_SPI_H_
#define AD_SPI_H_

#if dg_configSPI_ADAPTER

#include <osal.h>
#include <resmgmt.h>
#include <hw_spi.h>
#include <hw_gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \def CONFIG_SPI_ONE_DEVICE_ON_BUS
 *
 * \brief Macro to configure only one device on the SPI bus
 *
 * Set this macro to 1 if only one SPI device exists on the bus to reduce code size and
 * improve performance.
 */
#ifndef CONFIG_SPI_ONE_DEVICE_ON_BUS
# define CONFIG_SPI_ONE_DEVICE_ON_BUS   (0)
#endif

/**
 * \def CONFIG_SPI_EXCLUSIVE_OPEN
 *
 * \brief Macro to configure exclusive use of devices
 *
 * Set this macro to 1 in order to enable preventing multiple tasks opening the
 * same device. When set to 1, ad_spi_device_acquire() and ad_spi_device_release()
 * are no longer necessary.
 *
 * \note The setting of this macro is irrelevant if \ref CONFIG_SPI_USE_RESMGMT is unset.
 *
 * \sa CONFIG_SPI_USE_RESMGMT
 */
#ifndef CONFIG_SPI_EXCLUSIVE_OPEN
#define CONFIG_SPI_EXCLUSIVE_OPEN       (0)
#endif

/**
 * \def CONFIG_SPI_RESOURCE_STATIC_ID
 *
 * \brief Macro to configure resource ID assignment for each device
 *
 * Set this macro to 1 to enable unique resource ID assignment per device.
 */
#ifndef CONFIG_SPI_RESOURCE_STATIC_ID
# define CONFIG_SPI_RESOURCE_STATIC_ID  (0)
#endif

/**
 * \def CONFIG_SPI_USE_RESMGMT
 *
 * \brief Controls whether SPI resource acquisition will be used
 *
 * SPI adapter provides resource access management in order to protect SPI resources (devices and
 * buses) from being used by multiple tasks simultaneously. Unsetting this macro will remove this
 * feature, improving performance and potentially reducing code size (if the resource management API
 * is not used by other modules). All acquisition and release SPI adapter calls omit calls to
 * the resource management API when this this macro is unset.
 *
 * \note Unsetting this macro will not affect DMA resource management see \ref
 * CONFIG_SPI_USE_DMA_RESMGMT.
 */
#ifndef CONFIG_SPI_USE_RESMGMT
# define CONFIG_SPI_USE_RESMGMT              (1)
#endif

/**
 * \def CONFIG_SPI_USE_DMA_RESMGMT
 *
 * \brief Controls whether DMA resource acquisition will be used by the SPI adapter
 *
 * SPI adapter provides DMA resource access management in order to protect DMA resources from being
 * used by multiple tasks simultaneously. Unsetting this macro will remove this feature, improving
 * performance and potentially reducing code size (if the resource management API is not used by
 * other modules). Unset this macro if, for example, you have exclusively assigned DMA channels to
 * the SPI interface.
 */
#ifndef CONFIG_SPI_USE_DMA_RESMGMT
# define CONFIG_SPI_USE_DMA_RESMGMT          (1)
#endif

/**
 * \def CONFIG_SPI_USE_ASYNC_TRANSACTIONS
 *
 * \brief Controls whether SPI asynchronous transaction API will be used
 *
 * SPI transaction API maintains state in retention RAM for every SPI bus declared. If the API is
 * not to be used, setting this macro to 0 will save retention RAM.
 */
#ifndef CONFIG_SPI_USE_ASYNC_TRANSACTIONS
# define CONFIG_SPI_USE_ASYNC_TRANSACTIONS         (1)
#endif

/**
 * \brief Device pointer, handle to use with ad_spi_read(), ad_spi_write() etc.
 *
 */
typedef void *spi_device;

/**
 * \brief Device id, those are created by SPI_SLAVE_DEVICE or SPI_SLAVE_TO_EXT_MASTER macro in
 * platform_devices.h
 *
 */
typedef const void *spi_device_id;

#ifndef SPI_SLAVE_DEVICE

/**
 * \brief Entry for slave device
 *
 * \param [in] bus_id value must match SPI_BUS() argument valid values: SPI1, SPI2
 * \param [in] name name that will be used later to open device
 * \param [in] cs_port port id for chip select
 * \param [in] cs_pin pin number for chip select
 * \param [in] word_mode SPI word size from HW_SPI_WORD enum
 * \param [in] pol_mode idle state of clock line from HW_SPI_POL enum
 * \param [in] phase_mode which edge of clock is used for latching data from HW_SPI_PHA enum
 * \param [in] xtal_div divisor of main clock, used for generating SPI clock from HW_SPI_FREQ enum
 * \param [in] dma_channel DMA number for rx channel, tx will have next number, pass -1 for no DMA
 *
 */
#define SPI_SLAVE_DEVICE(bus_id, name, cs_port, cs_pin, word_mode, pol_mode, phase_mode, xtal_div, dma_channel) \
        extern const void *const name;

/**
 * \brief Entry for parameters when SPI is in slave mode
 *
 * \param [in] bus_id value must match SPI_BUS() argument valid values: SPI1, SPI2
 * \param [in] name name that will be used later to open device only
 * \param [in] ignore_cs specifies if chip select pin should be ignored
 * \param [in] word_mode SPI word size from HW_SPI_WORD enum
 * \param [in] pol_mode idle state of clock line from HW_SPI_POL enum
 * \param [in] phase_mode which edge of clock is used for latching data from HW_SPI_PHA enum
 * \param [in] dma_channel DMA number for rx channel, tx will have next number, pass -1 for no DMA
 *
 * \note Slave mode is currently unsupported.
 */
#define SPI_SLAVE_TO_EXT_MASTER(bus_id, name, ignore_cs, word_mode, pol_mode, phase_mode, dma_channel) \
        extern const void *const name;

/**
 * \brief Starting entry for SPI bus devices
 *
 * Here is an example how to use macros to define devices connected to SPI lines.
 * \code{.c}
 * SPI_BUS(SPI2)
 *      SPI_SLAVE_DEVICE(SPI2, AT45DB011D, HW_GPIO_PORT_2, HW_GPIO_PIN_0, HW_SPI_WORD_8BIT,
 *                                      HW_SPI_POL_LOW, HW_SPI_PHA_MODE_0, HW_SPI_FREQ_DIV_2);
 *      SPI_SLAVE_DEVICE(SPI2, TC77, HW_SPI_WORD_8BIT, HW_SPI_POL_LOW, HW_SPI_PHA_MODE_0,
 *                                                                              HW_SPI_FREQ_DIV_14);
 * SPI_BUS_END
 * \endcode
 *
 * \param [in] bus_id identifies SPI bus SPI1 or SPI2
 *
 */
#define SPI_BUS(bus_id) \
        extern spi_bus_dynamic_data dynamic_##bus_id;

/**
 * \brief This ends SPI bus device list started with SPI_BUS()
 *
 */
#define SPI_BUS_END

/**
 * \brief Initialization of SPI bus variables
 *
 * This macro must be invoked somewhere during system startup to initialize variables needed to
 * manage SPI bus. It will create some OS specific synchronization primitives.
 * Each bus created with SPI_BUS() must have corresponding call to this macro.
 * If both SPIs are used somewhere in initialization sequence must be:
 *      SPI_BUS_INIT(SPI1);
 *      SPI_BUS_INIT(SPI2);
 *
 * \param [in] bus_id identifies SPI bus SPI1 or SPI2
 *
 */
#define SPI_BUS_INIT(bus_id) ad_spi_bus_init(&dynamic_##bus_id)

/**
 * \brief Initialization of SPI bus devices
 *
 * This macro must be invoked somewhere during system startup to initialize variables needed to
 * manage access to SPI devices.
 * This is important if CONFIG_SPI_RESOURCE_STATIC_ID is not defined or equals 0, because resource
 * id required for device access needs to be created at some point.
 * Each device created with SPI_SLAVE_DEVICE() and SPI_SLAVE_TO_EXT_MASTER() must have
 * corresponding call to this macro.
 * Example:
 *      SPI_BUS_INIT(SPI1);
 *      SPI_DEVICE_INIT(AT45DB011D);
 *
 * \param [in] name identifies SPI device connected to bus
 *
 */
#define SPI_DEVICE_INIT(name) ad_spi_device_init(name)

#endif

/**
 * \brief Transfer structure for complex SPI transaction
 *
 * One of wbuf and rbuf can be NULL. If both wbuf and rbuf are not NULL, duplex transmission is
 * performed. Buffer addresses and lengths must be be SPI-word-aligned (no alignment needed for
 * 9-bit SPI word configurations).
 *
 * \sa ad_spi_bus_activate_cs()
 *
 */
typedef struct {
        const void *wbuf; /**< Pointer to data to be sent */
        void *rbuf;        /**< Pointer to data to be read */
        size_t length;     /**< Size of wbuf and rbuf */
} spi_transfer_data;

/**
 * \brief Asynchronous callback function
 *
 */
typedef void (*ad_spi_user_cb)(void *user_data);

/**
 * \brief Initialize adapter
 *
 */
void ad_spi_init(void);

/**
 * \brief Initialize bus variables
 *
 * Don't call this function directly use SPI_BUS_INIT macro.
 *
 */
void ad_spi_bus_init(void *bus_data);

/**
 * \brief Initialize device variables
 *
 * Don't call this function directly use SPI_DEVICE_INIT macro.
 *
 */
void ad_spi_device_init(const spi_device_id id);

/**
 * \brief Open device connected to SPI bus
 *
 * If system is configured for single device on each SPI bus (defined CONFIG_SPI_ONE_DEVICE_ON_BUS)
 * This function will setup configuration parameters to SPI controller.
 * If bus is not dedicated to single device only, configuration is performed during call to
 * ad_spi_bus_acquire().
 *
 * \param [in] dev_id identifier of device connected to SPI bus (name should match entries defined
 *             by SPI_SLAVE_DEVICE or SPI_SLAVE_TO_EXT_MASTER
 *
 * \return device handle that can be used with other functions
 */
spi_device ad_spi_open(const spi_device_id dev_id);

/*
 * Close SPI device
 *
 * \param [in] device handle to device opened with ad_spi_open()
 *
 * \sa ad_spi_open()
 *
 */
void ad_spi_close(spi_device device);

/**
 * \brief Perform typical SPI transaction
 *
 * This function performs most typical transaction on SPI bus.
 * First chip select is activated for slave device.
 * Then buffer is sent over SPI bus.
 * Then it changes to read mode and reads data from connected device.
 * Chip select is deactivated after transaction is finished.
 *
 * This function is blocking. It can wait first for bus access, than it wait till transaction is
 * completed.
 *
 * Example usage:
 * \code{.c}
 * {
 *   spi_device dev = ad_spi_open(PRESSURE_SENSOR);
 *   while (1) {
 *     // No CS necessary will be activated when needed
 *     ad_spi_transact(dev, command, sizeof(command), response, sizeof(response));
 *     ...
 *   }
 * }
 * \endcode
 *
 * \param [in] dev handle to SPI device
 * \param [in] wbuf pointer to data to be sent to device
 * \param [in] wlen size of data to be sent to device
 * \param [out] rbuf pointer to data to be read from device
 * \param [in] rlen size of buffer pointed by rbuf
 *
 * \sa ad_spi_open()
 * \sa ad_spi_close()
 *
 * \note Supplied buffer addresses and lengths must be be SPI-word-aligned (no alignment needed for
 * 9-bit SPI word configurations).
 *
 */
void ad_spi_transact(spi_device dev, const uint8_t *wbuf, size_t wlen, uint8_t *rbuf,
                                                                                size_t rlen);

/**
 * \brief Perform write only transaction
 *
 * This function performs write only transaction on SPI bus.
 * First chip select is activated for slave device.
 * Then buffer is sent over SPI bus.
 * Chip select is deactivated after transaction is finished.
 *
 * This function is blocking. It can wait first for bus access, than it wait till transaction is
 * completed.
 *
 * \param [in] dev handle to SPI device
 * \param [in] wbuf pointer to data to be sent to device
 * \param [in] wlen size of data to be sent to device
 *
 * \sa ad_spi_open()
 * \sa ad_spi_close()
 *
 * \note Supplied buffer address and length must be be SPI-word-aligned (no alignment needed for
 * 9-bit SPI word configurations).
 */
void ad_spi_write(spi_device dev, const uint8_t *wbuf, size_t wlen);

/**
 * \brief Perform read only transaction
 *
 * This function performs read only transaction on SPI bus.
 * First chip select is activated for slave device.
 * Then data is read from SPI bus.
 * Chip select is deactivated after transaction is finished.
 *
 * This function is blocking. It can wait first for bus access, than it wait till transaction is
 * completed.
 *
 * \param [in] dev handle to SPI device
 * \param [out] rbuf pointer to data to be read from device
 * \param [in] rlen size of buffer pointed by rbuf
 *
 * \sa ad_spi_open()
 * \sa ad_spi_close()
 *
 * \note Supplied buffer address and length must be be SPI-word-aligned (no alignment needed for
 * 9-bit SPI word configurations).
 */
void ad_spi_read(spi_device dev, uint8_t *rbuf, size_t rlen);

/**
 * \brief Perform complex transaction
 *
 * This function performs complex transaction on SPI bus.
 * It can do several transaction in one chip select activation.
 * It can do duplex transmission if element in \p transfers array has both wbuf and rbuf setup.
 *
 * First chip select is activated for slave device.
 * Then it performs all sub transaction from \p transfer array.
 * Chip select is deactivated after all transaction finish.
 *
 * This function is blocking. It can wait first for bus access, than it wait till transaction is
 * completed.
 *
 * \param [in] dev handle to SPI device
 * \param [in] transfers pointer to array of reads and writes
 * \param [in] count number of elements in \p transfer array
 *
 * \sa ad_spi_open()
 * \sa ad_spi_close()
 *
 */
void ad_spi_complex_transact(spi_device dev, spi_transfer_data *transfers, size_t count);

/**
 * \brief Activate chip select for a specific device
 *
 * \param [in] dev handle to SPI device
 *
 * \warning The device must own the bus before calling this function (ad_spi_device_acquire() and
 *          ad_spi_bus_acquire() must be called previously from task execution context).
 */
void ad_spi_bus_activate_cs(spi_device dev);

/**
 * \brief Deactivate chip select for a specific device
 *
 * \param [in] dev handle to SPI device
 *
 * \warning The device must own the bus before calling this function (ad_spi_device_acquire() and
 *          ad_spi_bus_acquire() must be called previously from task execution context).
 */
void ad_spi_bus_deactivate_cs(spi_device dev);

/**
 * \brief Acquire access to SPI bus
 *
 * This function waits for SPI bus to available, and locks it for \p dev use only.
 * This function can be called several times, but number of ad_spi_bus_release() calls must match
 * number of calls to this function.
 *
 * This function should be used if normal ad_spi_transact(), ad_spi_read(), ad_spi_write() are not
 * enough and some other SPI controller calls are required. In this case typical usage for this
 * function would look like this:
 *
 * ad_spi_bus_acquire(dev);
 * id = ad_spi_get_hw_spi_id(dev);
 * ...
 * hw_spi_set...(id, ...);
 * hw_spi_write_buf(id, ...)
 * hw_spi_wait_while_busy(id),
 * ...
 * ad_spi_bus_relase(dev);
 *
 * \param [in] dev handle to SPI device
 *
 * \warning The device must be already acquired through ad_spi_device_acquire() before calling
 *          this function.
 */
void ad_spi_bus_acquire(spi_device dev);

/**
 * \brief Release access to SPI bus
 *
 * This function decrements acquire counter for this device and when it reaches 0 SPI bus is
 * released and can be used by other devices.
 *
 * \param [in] dev handle to SPI device
 *
 * \sa ad_spi_bus_acquire
 */
void ad_spi_bus_release(spi_device dev);

/**
 * \brief Acquire access to SPI device
 *
 * This function grants exclusive access to SPI device (not bus). When access is granted no
 * other task can use this device till ad_spi_device_release() is called. When platform is compiled
 * with ad_spi_open() exclusive access this function does nothing.
 * When this function ends, other tasks can still use SPI bus to access different devices.
 * For normal SPI transaction there is no need to call this function it will be called from
 * ad_spi_read() ad_spi_write() etc.. Explicit call to this is required when several transactions
 * must be executed in sequence, i.e. modifying part of flash memory can require reading it
 * to flash internal RAM buffer, overwriting flash RAM buffer, and writing flash RAM buffer to
 * main flash memory.
 *
 * \param [in] dev handle to SPI device
 *
 * \sa ad_spi_device_release
 * \sa ad_spi_bus_acquire
 *
 */
void ad_spi_device_acquire(spi_device dev);

/**
 * \brief Acquire access to SPI device
 *
 * This function release exclusive access to SPI device (not bus).
 * Number of function calls must match number of ad_spi_device_acquire() calls.
 *
 * \param [in] dev handle to SPI device
 *
 * \sa ad_spi_device_acquire
 * \sa ad_spi_bus_release
 *
 */
void ad_spi_device_release(spi_device dev);

/**
 * \brief Get SPI controller id
 *
 * This function returns id that can be used to get SPI controller id. This id is argument
 * for lower level functions starting with hw_spi_ prefix.
 *
 * \param [in] dev handle to SPI device
 *
 * \return id that can be used with hw_spi_... functions
 */
HW_SPI_ID ad_spi_get_hw_spi_id(spi_device dev);

struct spi_device_config;

#ifndef SPI_ASYNC_ACTIONS_SIZE
/*
 * 10 elements should be enough for normal read, read/write transactions with one callback
 * 13 is good value if two callback are added in SPI transaction, some demo drivers use two
 * callbacks so set it to 13.
 */
#define SPI_ASYNC_ACTIONS_SIZE 13
#endif

/**
 * \brief SPI data run time data
 *
 * Variables of this type are automatically generated by \sa SPI_BUS() macro.
 * Structure keeps data related to SPI controller that allows easy usage of bus using several
 * tasks and devices connected to SPI controller.
 *
 */
typedef struct {
        /** Event used for synchronization in accessing SPI controller. */
        OS_EVENT event;
#if !CONFIG_SPI_ONE_DEVICE_ON_BUS
        /**
         * This keeps track of last device that was used. When device changes all controller
         * parameters must also be changed. This field is cleared when platform enters sleep mode.
         */
        struct spi_device_config *current_device;
#endif
        /** Keeps count of opened devices. */
        uint8_t open_devices_cnt;
#if CONFIG_SPI_USE_ASYNC_TRANSACTIONS
        /** Transaction index. */
        uint8_t transaction_ix;

        /** Transaction data. */
        uint32_t transaction[SPI_ASYNC_ACTIONS_SIZE];
#endif
} spi_bus_dynamic_data;

/**
 * \brief SPI device data run time data
 *
 * Variables of this type are automatically generated by \sa SPI_SLAVE_DEVICE() and
 * \sa SPI_SLAVE_TO_EXT_MASTER() macros.
 * Structure keeps data related to device connected to SPI controller.
 *
 */
typedef struct {
        const struct spi_device_config *config;
#if CONFIG_SPI_USE_RESMGMT
#if !CONFIG_SPI_ONE_DEVICE_ON_BUS
        /** Keeps track of number of calls to ad_spi_bus_acquire() */
        int8_t bus_acquire_count;
#endif

#if !CONFIG_SPI_EXCLUSIVE_OPEN
        /** Number of acquisitions for this device. */
        int8_t dev_acquire_count;
        /** Task that has acquired this device. */
        OS_TASK owner;
#endif

#if !CONFIG_SPI_RESOURCE_STATIC_ID
        /** Dynamically created (\sa resource_add()) resource ID for this device */
        RES_ID dev_res_id;
#endif
#endif /* CONFIG_SPI_USE_RESMGMT */
} spi_dev_dynamic_data;

typedef void (*ad_spi_user_cb)(void *user_data);

#define SPI_TAG_CS_ACTIVATE     0xFF000000
#define SPI_TAG_CS_DEACTIVATE   0xFE000000
#define SPI_TAG_CALLBACK0       0xFD000000
#define SPI_TAG_CALLBACK1       0xFC000000
#define SPI_TAG_SEND            0xFB000000
#define SPI_TAG_RECEIVE         0xFA000000
#define SPI_TAG_SEND_RECEIVE    0xF9000000

/**
 * Asynchronous SPI transactions
 *
 * SPI adapter allows to create asynchronous transaction that consist of number of reads, writes,
 * chip select manipulation and callback calls.
 * This allow to create time efficient way to manage all SPI related actions. Most of the
 * actions will be executed in ISR context.
 * There are following asynchronous actions:
 *
 *  Action                             |  Macro
 * ----------------------------------- | ------------------
 *  Chip select activation             |  SPI_CSA
 *  Chip select de-activation          |  SPI_CSD
 *  Sending data                       |  SPI_SND
 *  Receiving data                     |  SPI_RCV
 *  Sending and receiving data         |  SPI_SRCV
 *  Callback execution                 |  SPI_CB0
 *  Callback execution with user data  |  SPI_CB1
 *
 * Typical SPI transaction steps:
 * CS activation, sending command, receiving response, CS de-activation
 *
 * Asynchronous SPI transaction allows to build this sequence, start it and wait for final callback
 * after everything is done.
 */

/**
 * Following macros are used to construct asynchronous transactions on SPI device.
 */
#define SPI_CSA                 (uint32_t) SPI_TAG_CS_ACTIVATE
#define SPI_CSD                 (uint32_t) SPI_TAG_CS_DEACTIVATE
#define SPI_SND(wbuf, len)      (uint32_t) ((len) | SPI_TAG_SEND), (uint32_t) (wbuf)
#define SPI_RCV(rbuf, len)      (uint32_t) ((len) | SPI_TAG_RECEIVE), (uint32_t) (rbuf)
#define SPI_SRCV(wbuf, rbuf, len) (uint32_t) ((len) | SPI_TAG_SEND_RECEIVE), (uint32_t) (wbuf), \
                                                                                (uint32_t) (rbuf)
#define SPI_CB(cb)              (uint32_t) SPI_TAG_CALLBACK0, (uint32_t) (cb)
#define SPI_CB1(cb, arg)        (uint32_t) SPI_TAG_CALLBACK1, (uint32_t) (cb), (uint32_t) (arg)
#define SPI_END                 (uint32_t) 0

/**
 * \brief Start asynchronous SPI transaction
 *
 * Arguments are actions that should be taken to perform full transaction.
 * Arguments usually should start with SPI_CSA and have SPI_CSD after all transfer are done.
 * It is possible to have multiply callback but usually callback should be execute after CS is
 * deactivated. It is possible to create transaction without CS activation though.
 *
 * It is important to now that transaction start from acquiring device and SPI bus and this is done
 * synchronously meaning that that function can wait for device and bus access.
 * It is possible to acquire device and bus before and then ad_spi_async_transact() will not block.
 * Buffers passed to this function should not be reused before final callback is called.
 *
 * Example usage:
 * \code{.c}
 * {
 *   spi_device dev = ad_spi_open(PRESSURE_SENSOR);
 *   while (1) {
 *     ad_spi_async_transact(dev, SPI_CSA, SPI_SND(command, sizeof(command)),
 *                            SPI_RCV(response, sizeof(response), SPI_CSD, SPI_CB0(final_callbak));
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
 * \param [in] dev handle to SPI device
 *
 * \note Callbacks are called from within SPI ISRs.
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
 */
void ad_spi_async_transact(spi_device dev, ...);

/**
 * \brief Start asynchronous write SPI transaction
 *
 * This is convenience macro that builds typical write only transaction and executes it.
 * After all is done user callback is executed.
 *
 * \param [in] dev handle to SPI device
 * \param [in] buf data to send
 * \param [in] len number of bytes to write
 * \param [in] cb callback to call after transaction is over (from ISR context)
 * \param [in] ud user data to pass to \p cb
 *
 * \note The callback is called from within SPI ISR and at that time resources
 *       (device and bus) are released
 *
 * \warning Do not call this function consecutively without guaranteeing that the previous
 *          async transaction has completed.
 *
 * \warning After the callback is called, it is not guaranteed that the scheduler will give
 *          control to the task waiting for this transaction to complete. This is important to
 *          consider if more than one tasks are using this API.
 *
 * \note Supplied buffer must be be SPI-word-aligned (no alignment needed for 9-bit SPI
 * word configurations).
 */
#define ad_spi_async_write(dev, buf, len, cb, ud) \
        ad_spi_async_transact(dev, SPI_CSA, SPI_SND(buf, len), SPI_CSD, SPI_CB1(cb, ud), SPI_END)

/**
 * \brief Start asynchronous write then read SPI transaction
 *
 * This is convenience macro that builds typical write then read transaction and executes it.
 * After all is done user callback is executed.
 *
 * \param [in] dev handle to SPI device
 * \param [in] wbuf data to send
 * \param [in] wlen number of bytes to write
 * \param [out] rbuf buffer for incoming data
 * \param [in] rlen number of bytes to read
 * \param [in] cb callback to call after transaction is over (from ISR context)
 * \param [in] ud user data to pass to \p cb
 *
 * Example usage:
 * \code{.c}
 * {
 *   spi_device dev = ad_spi_open(PRESSURE_SENSOR);
 *   while (1) {
 *     ad_spi_async_write_read(dev, command, sizeof(command), response, sizeof(response),
 *                                                                      final_callbak, cb_arg));
 *
 *     // wait here for final callback notification
 *     // do something with response
 *     ...
 *   }
 * }
 * \endcode
 *
 * \note The callback is called from within SPI ISR and at that time resources
 *       (device and bus) are released
 *
 * \warning Do not call this function consecutively without guaranteeing that the previous
 *          async transaction has completed.
 *
 * \warning After the callback is called, it is not guaranteed that the scheduler will give
 *          control to the task waiting for this transaction to complete. This is important to
 *          consider if more than one tasks are using this API.
 *
 * \note Supplied buffers must be be SPI-word-aligned (no alignment needed for 9-bit SPI
 * word configurations).
 */
#define ad_spi_async_write_read(dev, wbuf, wlen, rbuf, rlen, cb, ud) \
        ad_spi_async_transact(dev, SPI_CSA, SPI_SND(wbuf, wlen), SPI_RCV(rbuf, rlen), SPI_CSD, \
                                                                        SPI_CB1(cb, ud), SPI_END)


/**
 * \brief SPI device constant data
 *
 * Variable of this type keeps static configuration needed to access device on SPI bus.
 * Those variables are generated by \sa SPI_SLAVE_DEVICE() and \sa SPI_SLAVE_TO_EXT_MASTER() macros.
 *
 */
typedef struct spi_device_config {
        /** SPI id as needed by hw_spi_... functions */
        HW_SPI_ID bus_id;
        /** SPI resource ID RES_ID_SPI1 or RES_ID_SPI2 */
        RES_ID bus_res_id;
        /** SPI configuration passed to the SPI LLD. */
        spi_config hw_init;
        /** true if in slave mode CS line should be ignored */
        bool ignore_cs;
        /** Pointer to dynamic bus data */
        spi_bus_dynamic_data *bus_data;
        /** Pointer to dynamic device data */
        spi_dev_dynamic_data *data;
#if CONFIG_SPI_RESOURCE_STATIC_ID && CONFIG_SPI_USE_RESMGMT
        /**
         * If CONFIG_SPI_RESOURCE_STATIC_ID is defined, RES_ID contains value for this device and
         * it can be stored in constant data instead of dynamic
         */
        RES_ID dev_res_id;
#endif
} spi_device_config;

#ifdef __cplusplus
}
#endif

#endif /* dg_configSPI_ADAPTER */

#endif /* AD_SPI_H_ */

/**
 * \}
 * \}
 * \}
 */
