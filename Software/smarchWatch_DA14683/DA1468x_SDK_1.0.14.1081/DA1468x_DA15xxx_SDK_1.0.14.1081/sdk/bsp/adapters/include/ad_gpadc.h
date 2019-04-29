/**
 * \addtogroup BSP
 * \{
 * \addtogroup ADAPTERS
 * \{
 * \addtogroup GPADC_ADAPTER
 *
 * \brief GPADC adapter
 *
 * \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_gpadc.h
 *
 * @brief GPADC adapter API
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#if dg_configGPADC_ADAPTER

#ifndef AD_GPADC_H_
#define AD_GPADC_H_

#include <osal.h>
#include <resmgmt.h>
#include <hw_gpadc.h>
#include <hw_gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Source pointer, handle to use with ad_gpadc_read() etc.
 *
 */
typedef void *gpadc_source;

/**
 * \brief Connected source id, those are created by GPADC_SOURCE macro in platform_devices.h
 *
 */
typedef const void *gpadc_source_id;

/**
 * \brief GPADC input voltages
 *
 */
typedef enum {
        HW_GPADC_INPUT_VOLTAGE_UP_TO_1V2 = 0,    /**< input voltages up to 1.2 V are allowed */
        HW_GPADC_INPUT_VOLTAGE_UP_TO_3V6 = 1     /**< input voltages up to 3.6 V are allowed */
} HW_GPADC_INPUT_VOLTAGE;

/**
 * \brief GPADC oversampling
 *
 * In this mode multiple successive conversions will be executed and the results are added together
 * to increase the effective number of bits
 *
 */
typedef enum {
        HW_GPADC_OVERSAMPLING_1_SAMPLE          = 0,    /**< 1 sample is taken or 2 in case chopping is enabled */
        HW_GPADC_OVERSAMPLING_2_SAMPLES         = 1,    /**< 2 samples are taken */
        HW_GPADC_OVERSAMPLING_4_SAMPLES         = 2,    /**< 4 samples are taken */
        HW_GPADC_OVERSAMPLING_8_SAMPLES         = 3,    /**< 8 samples are taken */
        HW_GPADC_OVERSAMPLING_16_SAMPLES        = 4,    /**< 16 samples are taken */
        HW_GPADC_OVERSAMPLING_32_SAMPLES        = 5,    /**< 32 samples are taken */
        HW_GPADC_OVERSAMPLING_64_SAMPLES        = 6,    /**< 64 samples are taken */
        HW_GPADC_OVERSAMPLING_128_SAMPLES       = 7     /**< 128 samples are taken */
} HW_GPADC_OVERSAMPLING;

#ifndef GPADC_SOURCE

/**
 * \brief Entry for GPADC source
 *
 * \param [in] name             name that will be used later to open source
 * \param [in] _clock_source    internal (high-speed) or external (digital - 16/96MHz) clock source
 * \param [in] _input_mode      single-ended or differential mode
 * \param [in] input            GPIO input which will be used for measurements
 * \param [in] sample_time      sampling time (0: one clock cycle .. 15: 15*32 clock cycles)
 * \param [in] cancel_offset    offset is cancelled by chopping operation (taking two samples with
 *                              opposite signal polarity), recommended for DC and slowly changing signals
 * \param [in] _oversampling    mode used for increasing the effective number of bits
 * \param [in] _input_voltage   range of input voltage which can be used for measurements
 *
 */
#define GPADC_SOURCE(name, _clock_source, _input_mode, input, sample_time, cancel_offset, \
                                                                _oversampling, _input_voltage) \
        extern const void *const name;

/**
 * \brief Initialization of GPADC variables
 *
 * This macro must be invoked somewhere during system startup to initialize variables needed to
 * manage GPADC. It will create some OS specific synchronization primitives.
 *
 */
#define GPADC_INIT() ad_gpadc_init()

#endif // GPADC_SOURCE

/**
 * \brief Asynchronous callback function
 *
 */
typedef void (*ad_gpadc_user_cb)(void *user_data, int value);

/**
 * \brief Initialize GPADC adapter and some required variables
 *
 * Don't call this function directly use GPADC_INIT macro.
 *
 */
void ad_gpadc_init(void);

/**
 * \brief Read value of the measurement from the selected source
 *
 * This function reads measurement value synchronously.
 *
 * \param [in] source   handle to GPADC source
 * \param [out] value   pointer to data to be read from selected source
 *
 * \sa ad_gpadc_open()
 * \sa ad_gpadc_close()
 *
 */
void ad_gpadc_read(gpadc_source source, uint16_t *value);

/**
 * \brief Attempt to read the measurement from the selected source within a timeout period.
 *
 * This function attempts to read measurement value synchronously
 *
 * \param [in]  src handle to GPADC source
 * \param [out] value pointer to data to be read from selected source
 * \param [in]  timeout number of ticks to wait
 *              0 - no wait take GPADC if it is available
 *              RES_WAIT_FOREVER - wait until GPADC becomes available
 *              Other value specifies how many ticks to wait until GPADC becomes available
 *
 * \return true if measurement value has been read, false otherwise
 *
 */
bool ad_gpadc_read_to(gpadc_source src, uint16_t *value, uint32_t timeout);

/**
 * \brief Read asynchronously value of the measurement from the selected source
 *
 * This function starts asynchronous measurement read.
 *
 * \param [in] source           handle to GPADC source
 * \param [in] read_async_cb    user callback fired after read operation completes
 * \param [in] user_data        pointer to user data
 *
 * \sa ad_gpadc_open()
 * \sa ad_gpadc_close()
 *
 * \return true if read was successful, false otherwise
 *
 */
bool ad_gpadc_read_async(gpadc_source source, ad_gpadc_user_cb read_async_cb, void *user_data);

/**
 * \brief Return maximum value that can be read for ADC source
 *
 * Value returned by ad_gpadc_read() can have 10 to 16 bits (right aligned) depending on
 * oversampling specified in source description. This function will return value
 * 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF or 0xFFFF depending on oversampling.
 *
 * \param [in] source handle to GPADC source
 *
 * \return maximum value that can be returned by ad_gpadc_read
 *
 */
uint16_t ad_gpadc_get_source_max(gpadc_source source);

/**
 * \brief Acquire access to GPADC
 *
 * This function waits for GPADC to become available, and locks it.
 * This function can be called several times, but number of ad_gpadc_release() calls must match
 * number of calls to this function.
 *
 * This function should be used if normal ad_gpadc_read() is not enough and some other GPADC
 * controller calls are required. In this case typical usage for this function would look like this:
 *
 * ad_gpadc_acquire();
 * ...
 * ad_gpadc_release();
 *
 */
void ad_gpadc_acquire(void);

/**
 * \brief Attempt to acquire exclusive access on GPADC within a timeout period.
 *
 * This function waits for GPADC to become available until the timeout expires.
 * When GPADC is acquired the resource is locked and requires an ad_gpadc_release() call to
 * unlock it.
 *
 * \param [in] timeout number of ticks to wait
 *             0 - no wait take GPADC if it is available
 *             RES_WAIT_FOREVER - wait until GPADC becomes available
 *             Other value specifies how many ticks to wait until GPADC becomes available
 *
 * \return true if GPADC has been acquired, false otherwise
 *
 */
bool ad_gpadc_acquire_to(uint32_t timeout);

/**
 * \brief Release access to GPADC
 *
 * This function decrements acquire counter for GPADC and when it reaches 0 GPADC is released and
 * can be used by other tasks.
 *
 * \sa ad_gpadc_acquire()
 *
 */
void ad_gpadc_release(void);

/**
 * \brief Open source connected to GPADC
 *
 * This function will set up configuration parameters to GPADC controller. It has to be called
 * before ad_gpadc_acquire() to get a handle to the source like this:
 *
 * source_handle = ad_gpadc_open(GPADC_SOURCE);
 * ad_gpadc_acquire();
 * ...
 * ad_gpadc_release();
 * ad_gpadc_close(source_handle);
 *
 * \param [in] id       id of the source connected to GPADC (name should match entries defined by
 *                      GPADC_SOURCE macro)
 *
 * \return source handle that can be used with other functions
 *
 */
gpadc_source ad_gpadc_open(const gpadc_source_id id);

/*
 * \brief Close source connected to GPADC
 *
 * This function closes access to GPADC source using handle created by ad_gpadc_open().
 *
 * \param [in] source   handle to source opened with ad_gpadc_open()
 *
 * \sa ad_gpadc_open()
 *
 */
void ad_gpadc_close(gpadc_source source);

struct gpadc_source_config_t;

/**
 * \brief GPADC data run time data
 *
 * Structure keeps data related to GPADC controller that allows easy usage of GPADC using several
 * tasks and sources connected to GPADC controller.
 *
 */
typedef struct {
        OS_EVENT event;                                         /**< Event used for synchronization in accessing GPADC controller */
        const struct gpadc_source_config_t *current_source;     /**< This keeps track of last source that was used. */
        ad_gpadc_user_cb read_cb;                               /**< User function to call after asynchronous read finishes */
        void *user_data;                                        /**< User data for callback */
        OS_TASK owner;                                          /**< Task that acquired this source */
        int8_t gpadc_acquire_count;                             /**< This keeps track of number of calls to ad_gpadc_acquire() */
        bool read_in_progress;                                  /**< Number of source_acquire calls */
} gpadc_dynamic_data;

/**
 * \brief GPADC source run time data
 *
 * Variables of this type are automatically generated by \sa GPADC_SOURCE macro.
 * Structure keeps data related to source connected to GPADC controller.
 *
 */
typedef struct {
} gpadc_src_dynamic_data;

/**
 * \brief GPADC source constant data
 *
 * Variable of this type keeps static configuration needed to access GPADC source.
 * Those variables are generated by GPADC_SOURCE macro.
 *
 */
typedef struct gpadc_source_config_t {
        gpadc_config hw_init;           /**< Source configuration */
        gpadc_dynamic_data *gpadc_data; /**< Pointer to dynamic GPADC data */
} gpadc_source_config;

#ifdef __cplusplus
}
#endif

#endif /* AD_GPADC_H_ */

#endif /* dg_configGPADC_ADAPTER */

/**
 * \}
 * \}
 * \}
 */
