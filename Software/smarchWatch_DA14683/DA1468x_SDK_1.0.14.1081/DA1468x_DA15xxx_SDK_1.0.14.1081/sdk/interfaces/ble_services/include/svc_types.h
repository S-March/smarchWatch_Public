/**
 ****************************************************************************************
 *
 * @file svc_types.h
 *
 * @brief Characteristics common types
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SVC_TYPES_H_
#define SVC_TYPES_H_

#include <stdint.h>
#include <ble_bufops.h>

#define SVC_IEEE11073_SFLOAT_NAN                        (0x07FF)
#define SVC_IEEE11073_SFLOAT_NRES                       (0x0800)
#define SVC_IEEE11073_SFLOAT_PLUS_INFINITY              (0x07FE)
#define SVC_IEEE11073_SFLOAT_MINUS_INFINITY             (0x0802)

#define SVC_IEEE11073_FLOAT_NAN                         (0x007FFFFF)
#define SVC_IEEE11073_FLOAT_NRES                        (0x00800000)
#define SVC_IEEE11073_FLOAT_PLUS_INFINITY               (0x007FFFFE)
#define SVC_IEEE11073_FLOAT_MINUS_INFINITY              (0x00800002)

/**
 * Service date time
 */
typedef struct {
        uint16_t year;          /**< 1582 .. 9999, 0 - unknown */
        uint8_t  month;         /**< 1..12, 0 - unknown */
        uint8_t  day;           /**< 1..31, 0 - unknown */
        uint8_t  hours;         /**< 0..23 */
        uint8_t  minutes;       /**< 0..59 */
        uint8_t  seconds;       /**< 0..59 */
} svc_date_time_t;

/**
 * FLOAT/SFLOAT type from IEEE 11073 standard
 */
typedef struct {
        int8_t exp;
        int32_t mantissa;
} svc_ieee11073_float_t;

/**
 * \brief Pack date time
 *
 * Function puts date_time to buffer ptr and increases pointer
 *
 * \param [in] date_time        date time
 * \param [in,out] ptr          pointer to data buffer
 *
 */
static inline void pack_date_time(const svc_date_time_t *date_time, uint8_t **ptr)
{
        put_u16_inc(ptr, date_time->year);
        put_u8_inc(ptr, date_time->month);
        put_u8_inc(ptr, date_time->day);
        put_u8_inc(ptr, date_time->hours);
        put_u8_inc(ptr, date_time->minutes);
        put_u8_inc(ptr, date_time->seconds);
}

/**
 * \brief Unpack date time
 *
 * Function puts values from buffer ptr to date_time and increases pointer
 *
 * \param [in,out] ptr          pointer to data buffer
 * \param [out] date_time       date time
 *
 */
static inline void unpack_date_time(const uint8_t **ptr, svc_date_time_t *date_time)
{
        date_time->year = get_u16_inc(ptr);
        date_time->month = get_u8_inc(ptr);
        date_time->day = get_u8_inc(ptr);
        date_time->hours = get_u8_inc(ptr);
        date_time->minutes = get_u8_inc(ptr);
        date_time->seconds = get_u8_inc(ptr);
}

/**
 * \brief Convert float value to ISO/IEEE Std. 11073-20601™-2008 standard
 *
 * \param [in] val              value to convert
 * \param [in] precision        number of digits to the right of decimal separator
 * \param [out] ieee_val        value converted to IEEE 11073 standard
 *
 */
static inline void float_to_ieee11703(float val, int8_t precision, svc_ieee11073_float_t *ieee_val)
{
        ieee_val->exp = -precision;

        if (precision > 0) {
                while (precision) {
                        val *= 10;
                        precision--;
                }
        }

        if (precision < 0) {
                while (precision) {
                        val /= 10;
                        precision++;
                }
        }

        ieee_val->mantissa = (int32_t) val;
}

/**
 * \brief Convert ISO/IEEE Std. 11073-20601™-2008 standard to float value
 *
 * \param [in] value            IEEE 11073 standard converted to float value
 *
 * \return float value
 *
 */
static inline float ieee11703_to_float(const svc_ieee11073_float_t *value)
{
        float float_value;
        int8_t exp;

        float_value = value->mantissa;
        exp = value->exp;

        while (exp > 0) {
                float_value *= 10;
                exp--;
        }

        while (exp < 0) {
                float_value /= 10;
                exp++;
        }

        return float_value;
}

/**
 * \brief Pack IEEE 11073 value to SFLOAT-Type
 *
 * \param [in] val              value to pack
 *
 * \return packed value in SFLOAT-Type
 *
 */
static inline uint16_t pack_ieee11703_sfloat(const svc_ieee11073_float_t *val)
{
        /*
         * ISO/IEEE Std. 11073-20601™-2008 defines SFLOAT-Type data type which consist of 12-bit
         * mantissa and 4-bit exponent in below order:
         *      Exponent        Mantissa
         *       4 bit           12 bit
         */

        /* maximum exponent = 7
         * minimum exponent = -8 (signed 4 bits)
         */
        if (val->exp > 7 || val->exp < -8) {
                return SVC_IEEE11073_SFLOAT_NRES;
        }

        /* maximum mantissa = 2047      - 0x000007FF    | +infinity
         * minimum mantissa = -2048     - 0xFFFFF800    | -infinity
         */
        if (val->mantissa > 2047 || val->mantissa < -2048) {
                return SVC_IEEE11073_SFLOAT_NRES;
        }

        return ((val->exp & 0x0F) << 12) | (val->mantissa & 0x0FFF);
}

/**
 * \brief Unpack SFLOAT-Type to IEEE 11073 value
 *
 * \param [in] sfloat_val       sfloat value
 * \param [out] val             value after unpack
 *
 */
static inline void unpack_ieee11703_sfloat(uint16_t sfloat_val, svc_ieee11073_float_t *val)
{
        /*
         * ISO/IEEE Std. 11073-20601™-2008 defines SFLOAT-Type data type which consist of 12-bit
         * mantissa and 4-bit exponent in below order:
         *      Exponent        Mantissa
         *       4 bits         12 bits
         */

        /*
         * mantissa and exponent are signed values so the shifts have to be done
         * with signed value of sfloat_val.
         */
        val->mantissa = ((int16_t) (sfloat_val << 4) >> 4);
        val->exp = ((int16_t) sfloat_val >> 12);
}

/**
 * \brief Pack IEEE 11073 value to FLOAT-Type
 *
 * \param [in] val              value to pack
 *
 * \return packed value in FLOAT-Type
 *
 */
static inline uint32_t pack_ieee11703_float(const svc_ieee11073_float_t *val)
{
        /*
         * ISO/IEEE Std. 11073-20601™-2008 defines FLOAT-Type data type which consist of 24-bit
         * mantissa and 8-bit exponent in below order:
         *      Exponent        Mantissa
         *      1 octet         3 octets
         */

        /* maximum exponent = 127
         * minimum exponent = -128 (signed 1 octet)
         */
        if (val->exp > 127 || val->exp < -128) {
                return SVC_IEEE11073_FLOAT_NRES;
        }

        /* maximum mantissa = 8388607   - 0x007FFFFF    | +infinity
         * minimum mantissa = -8388608  - 0xFF800000    | -infinity
         */
        if (val->mantissa > 8388607 || val->mantissa < -8388608) {
                return SVC_IEEE11073_FLOAT_NRES;
        }

        return (val->exp << 24) | (val->mantissa & 0xFFFFFF);
}

/**
 * \brief Unpack FLOAT-Type to IEEE 11073 value
 *
 * \param [in] float_val        float value
 * \param [out] val             value after unpack
 *
 */
static inline void unpack_ieee11703_float(uint32_t float_val, svc_ieee11073_float_t *val)
{
        /*
         * ISO/IEEE Std. 11073-20601™-2008 defines FLOAT-Type data type which consist of 24-bit
         * mantissa and 8-bit exponent in below order:
         *      Exponent        Mantissa
         *      1 octet         3 octets
         */

        /*
         * mantissa and exponent are signed values so the shifts have to be done
         * with signed value of float_val.
         */
        val->mantissa = ((int32_t) (float_val << 8) >> 8);
        val->exp = ((int32_t) float_val >> 24);
}

#endif /* SVC_TYPES_H_ */
