/**
 \addtogroup INTERFACES
 \{
 \addtogroup BLE
 \{
 \addtogroup API
 \{
 */

/**
 ****************************************************************************************
 *
 * @file ble_bufops.h
 *
 * @brief Helpers to put and get data from BLE buffers
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#include <stdint.h>
#include <string.h>

#ifndef BLE_BUFOPS_H_
#define BLE_BUFOPS_H_

/**
 * \brief Get uint8 from buffer
 *
 * \param [in] buffer data buffer
 *
 * \return value
 *
 */
static inline uint8_t get_u8(const uint8_t *buffer)
{
        return buffer[0];
}

/**
 * \brief Get uint16 from buffer
 *
 * \param [in] buffer data buffer
 *
 * \return value
 *
 */
static inline uint16_t get_u16(const uint8_t *buffer)
{
        return (buffer[0]) | (buffer[1] << 8);
}

/**
 * \brief Get uint32 from buffer
 *
 * \param [in] buffer data buffer
 *
 * \return value
 *
 */
static inline uint32_t get_u32(const uint8_t *buffer)
{
        return (buffer[0]) | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
}

/**
 * \brief Get uint8 from buffer and increase pointer
 *
 * \param [in,out] buffer data buffer
 *
 * \return value
 *
 */
static inline uint8_t get_u8_inc(const uint8_t **buffer)
{
        const uint8_t *b = *buffer;

        (*buffer) += sizeof(uint8_t);

        return get_u8(b);
}

/**
 * \brief Get uint16 from buffer and increase pointer
 *
 * \param [in,out] buffer data buffer
 *
 * \return value
 *
 */
static inline uint16_t get_u16_inc(const uint8_t **buffer)
{
        const uint8_t *b = *buffer;

        (*buffer) += sizeof(uint16_t);

        return get_u16(b);
}

/**
 * \brief Get uint32 from buffer and increase pointer
 *
 * \param [in,out] buffer data buffer
 *
 * \return value
 *
 */
static inline uint32_t get_u32_inc(const uint8_t **buffer)
{
        const uint8_t *b = *buffer;

        (*buffer) += sizeof(uint32_t);

        return get_u32(b);
}

/**
 * \brief Put uint8 to buffer
 *
 * \param [in,out] buffer data buffer
 * \param [in]     value  value to put
 *
 */
static inline void put_u8(uint8_t *buffer, uint8_t value)
{
        buffer[0] = value;
}

/**
 * \brief Put uint16 to buffer
 *
 * \param [in,out] buffer data buffer
 * \param [in]     value  value to put
 *
 */
static inline void put_u16(uint8_t *buffer, uint16_t value)
{
        buffer[0] = value;
        buffer[1] = value >> 8;
}

/**
 * \brief Put uint32 to buffer
 *
 * \param [in,out] buffer data buffer
 * \param [in]     value  value to put
 *
 */
static inline void put_u32(uint8_t *buffer, uint32_t value)
{
        buffer[0] = value;
        buffer[1] = value >> 8;
        buffer[2] = value >> 16;
        buffer[3] = value >> 24;
}

/**
 * \brief Put uint8 to buffer and increase pointer
 *
 * \param [in,out] buffer data buffer
 * \param [in]     value  value to put
 *
 */
static inline void put_u8_inc(uint8_t **buffer, uint8_t value)
{
        uint8_t *b = *buffer;

        (*buffer) += sizeof(uint8_t);

        put_u8(b, value);
}

/**
 * \brief Put uint16 to buffer and increase pointer
 *
 * \param [in,out] buffer data buffer
 * \param [in]     value  value to put
 *
 */
static inline void put_u16_inc(uint8_t **buffer, uint16_t value)
{
        uint8_t *b = *buffer;

        (*buffer) += sizeof(uint16_t);

        put_u16(b, value);
}

/**
 * \brief Put uint32 to buffer and increase pointer
 *
 * \param [in,out] buffer data buffer
 * \param [in]     value  value to put
 *
 */
static inline void put_u32_inc(uint8_t **buffer, uint32_t value)
{
        uint8_t *b = *buffer;

        (*buffer) += sizeof(uint32_t);

        put_u32(b, value);
}

/**
 * \brief Put data to buffer and increase pointer
 *
 * \param [in,out] buffer data buffer
 * \param [in]     length length of data to put
 * \param [in]     data   data to put
 *
 */
static inline void put_data_inc(uint8_t **buffer, uint16_t length, const void *data)
{
        memcpy(*buffer, data, length);

        (*buffer) += length;
}

/**
 * \brief Put string to buffer and increase pointer
 *
 * \param [in,out] buffer data buffer
 * \param [in]     str    string to put
 *
 */
static inline void put_str_inc(uint8_t **buffer, const char *str)
{
        put_data_inc(buffer, strlen(str), str);
        put_u8_inc(buffer, '\0');
}

#endif /* BLE_BUFOPS_H_ */
/**
 \}
 \}
 \}
 */
