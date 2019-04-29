/**
 ****************************************************************************************
 *
 * @file cts.h
 *
 * @brief Current Time Service implementation API
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef CTS_H_
#define CTS_H_

#include <stdint.h>
#include <ble_service.h>
#include <svc_types.h>

/** CTS Current Time adjust reason valid values mask */
#define CTS_ADJUST_REASON_VALID_VALUES_MASK     (CTS_AR_NO_CHANGE                       | \
                                                 CTS_AR_CHANGE_OF_DST                   | \
                                                 CTS_AR_MANUAL_TIME_UPDATE              | \
                                                 CTS_AR_EXTERNAL_REFERENCE_TIME_UPDATE  | \
                                                 CTS_AR_CHANGE_OF_TIME_ZONE)

/** CTS additional ATT error codes */
enum cts_error {
        CTS_ERROR_DATA_FIELD_IGNORED = ATT_ERROR_APPLICATION_ERROR,
};

/** CTS Current Time adjust reason */
typedef enum {
        CTS_AR_NO_CHANGE = 0,
        CTS_AR_MANUAL_TIME_UPDATE = 1,
        CTS_AR_EXTERNAL_REFERENCE_TIME_UPDATE = 2,
        CTS_AR_CHANGE_OF_TIME_ZONE = 4,
        CTS_AR_CHANGE_OF_DST = 8
} cts_adjust_reason_t;

/** CTS Local Time Information DST */
typedef enum {
        CTS_DST_STANDARD_TIME = 0,
        CTS_DST_HALF_AN_HOUR_DAYLIGH_TIME = 2,
        CTS_DST_DAYLIGHT_TIME = 4,
        CTS_DST_DOUBLE_DAYLIGHT_TIME = 8,
        CTS_DST_UNKNOWN = 255
} cts_dst_t;

/** CTS Reference Time Source */
typedef enum {
        CTS_RTS_UNKNOWN = 0,
        CTS_RTS_NTP = 1,
        CTS_RTS_GPS = 2,
        CTS_RTS_RADIO_TIME_SIGNAL = 3,
        CTS_RTS_MANUAL = 4,
        CTS_RTS_ATOMIC_CLOCK = 5,
        CTS_RTS_CELULAR_NETOWRK = 6,
} cts_ref_time_source_t;

/** CTS Current Time */
typedef struct {
        svc_date_time_t     date_time;
        uint8_t             day_of_week;        /**< 0 - unknown, 1 - Monday, 7 - Sunday */
        uint8_t             fractions_256;      /**< 1/256th of a second */
        cts_adjust_reason_t adjust_reason;
} cts_current_time_t;

/** CTS Local Time Info */
typedef struct {
        /*
         * values for \p time_zone as definied by specification
         * helper function cts_get_time_zone() can be used to calculate value
         */
        int8_t    time_zone;
        cts_dst_t dst;
} cts_local_time_info_t;

/** CTS Reference Time Information */
typedef struct {
        cts_ref_time_source_t   source;
        /**< Accuracy (drift) of time information in steps of 1/8 of a second (125ms) compared to
         * a reference time source. Valid range from 0 to 253 (0s to 31.5s).
         * A value of 254 means Accuracy is out of range (> 31.5s).
         * A value of 255 means Accuracy is unknown.
         */
        uint8_t                 accuracy;
        /**< 0..254, 255 - means 255 or more */
        uint8_t                 days_since_update;
        /**< 0..23, 255 - 255 or more days */
        uint8_t                 hours_since_update;
} cts_ref_time_info_t;

typedef void (* cts_get_time_cb_t) (ble_service_t *svc, uint16_t conn_idx);

typedef void (* cts_set_time_cb_t) (ble_service_t *svc, uint16_t conn_idx,
                                                                const cts_current_time_t *time);

typedef void (* cts_set_local_time_info_cb_t) (ble_service_t *svc, uint16_t conn_idx,
                                                                const cts_local_time_info_t *info);

typedef void (* cts_get_ref_time_info_cb_t) (ble_service_t *svc, uint16_t conn_idx);

/** CTS application callbacks */
typedef struct {
        /** User callback to call on every get time request */
        cts_get_time_cb_t get_time;

        /** User callback to call on write request, can be NULL, that will make characteristic RO */
        cts_set_time_cb_t set_time;

        /** User callback to call on write request, can be NULL since characteristic can be RO.
         * Application should update local time information and call cts_notify_time() to notify
         * interested clients about time change. */
        cts_set_local_time_info_cb_t set_local_time_info;

        /** User callback to call on every read from Reference Time Information characteristic
         * Characteristic is optional, can be NULL if reference time information is not supported */
        cts_get_ref_time_info_cb_t get_ref_time_info;
} cts_callbacks_t;

/**
 * Register CTS instance
 *
 * Function initializes and register Current Time Service.
 * If local_time_info is NULL Local Time Information optional characteristic will not be available.
 *
 * \param [in] info     local time information
 * \param [in] cb       application callbacks
 *
 * \return service instance
 *
 */
ble_service_t *cts_init(const cts_local_time_info_t *info, const cts_callbacks_t *cb);

/**
 * Notify connected client about current time
 *
 * \param [in] svc      service instance
 * \param [in] conn_idx connection index
 * \param [in] time     current time with change reason
 *
 */
void cts_notify_time(ble_service_t *svc, uint16_t conn_idx, const cts_current_time_t *time);

/**
 * Notify connected clients about current time
 *
 * \param [in] svc      service instance
 * \param [in] time     current time with change reason
 *
 */
void cts_notify_time_all(ble_service_t *svc, const cts_current_time_t *time);

/**
 * Set local time information
 *
 * \param [in] svc      service instance
 * \param [in] info     local time information
 *
 */
void cts_set_local_time_info(ble_service_t *svc, const cts_local_time_info_t *info);

/**
 * Response for \p get_time callback
 *
 * \param [in] svc      service instance
 * \param [in] conn_idx connection index
 * \param [in] status   ATT error
 * \param [in] time     current time
 *
 */
void cts_get_time_cfm(ble_service_t *svc, uint16_t conn_idx, att_error_t status,
                                                                const cts_current_time_t *time);

/**
 * Response for \p set_time callback
 *
 * \param [in] svc      service instance
 * \param [in] conn_idx connection index
 * \param [in] status   ATT error
 *
 */
void cts_set_time_cfm(ble_service_t *svc, uint16_t conn_idx, att_error_t status);

/**
 * Response for \p set_local_time_info callback
 *
 * \param [in] svc      service instance
 * \param [in] conn_idx connection index
 * \param [in] status   ATT error
 *
 */
void cts_set_local_time_info_cfm(ble_service_t *svc, uint16_t conn_idx, att_error_t status);

/**
 * Response for \p get_ref_time_info callback
 *
 * \param [in] svc      service instance
 * \param [in] conn_idx connection index
 * \param [in] status   ATT error
 * \param [in] info     reference time information
 *
 */
void cts_get_ref_time_info_cfm(ble_service_t *svc, uint16_t conn_idx, att_error_t status,
                                                                const cts_ref_time_info_t *info);

/**
 * Helper function to calculate time zone for \p cts_local_time_information_t
 *
 * \note
 * This will work for all current time zones, it will get incorrect result for -00:15, -00:30, -00:45
 * which are not valid time zones for now.
 *
 * \param [in] h        hours difference to UTC (can be positive or negative)
 * \param [in] m        minutes difference UTC (positive values only)
 *
 * \return time zone value
 *
 */
static inline int8_t cts_get_time_zone(int8_t h, uint8_t m)
{
        return h * 4 + (m / 15) * (h < 0 ? -1 : 1);
}

/**
 * Helper function to extract hour and minute from time_zone field
 *
 * \param [in]  tz      time zone as in \p cts_local_time_information_t
 * \param [out] h       hours difference to UTC
 * \param [out] m       minutes difference to UTC
 *
 * return true if \p tz is valid
 */
static inline bool cts_get_time_zone_offset(int8_t tz, int8_t *h, int8_t *m)
{
        if (tz == -128) {
                return false;
        }

        if (h) {
                *h = tz / 4;
        }

        if (m) {
                *m = (tz % 4) * 15;
        }

        return true;
}

/**
 * Helper function to check it contents of \p cts_current_time_t is valid
 *
 * \param [in] time     current time
 *
 * return true if \p time is valid
 *
 */
static inline bool cts_is_current_time_valid(const cts_current_time_t *time)
{
        return (((time->date_time.year >= 1582) && (time->date_time.year <= 9999))
                        || (time->date_time.year == 0)) &&
                (time->date_time.month <= 12) && (time->date_time.day <= 31) &&
                (time->date_time.hours <= 23) && (time->date_time.minutes <= 59) &&
                (time->date_time.seconds <= 59) && (time->day_of_week <= 7);
}

/**
 * Helper function to check it contents of \p cts_local_time_info_t is valid
 *
 * \param [in] info     local time information
 *
 * return true if \p info is valid
 *
 */
static inline bool cts_is_local_time_info_valid(const cts_local_time_info_t *info)
{
        return (((info->time_zone >= -48) && (info->time_zone <= 56)) || (info->time_zone == -128)) &&
                                                        (info->dst >= 0) && (info->dst <= 8);
}

#endif /* CTS_H_ */
