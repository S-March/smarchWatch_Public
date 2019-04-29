/**
 ****************************************************************************************
 *
 * @file crc16.c
 *
 * @brief CRC16 calculation
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#include "crc16.h"

void crc16_init(uint16_t *crc16)
{
        *crc16 = 0xFFFF;
}

void crc16_update(uint16_t *crc16, const uint8_t *buf, size_t len)
{
        size_t i, j;

        for (i = 0; i < len; i++) {
                uint8_t b = buf[i];

                for (j = 0; j < 8; j++) {
                        uint16_t need_xor = (*crc16 & 0x8000);

                        *crc16 <<= 1;

                        if (b & 0x80) {
                                *crc16 |= 1;
                        }

                        if (need_xor) {
                                *crc16 ^= 0x1021; // CRC16-CCITT polynomial
                        }

                        b <<= 1;
                }
        }
}

uint16_t crc16_calculate(const uint8_t *buf, size_t len)
{
        uint16_t crc;

        crc16_init(&crc);
        crc16_update(&crc, buf, len);

        return crc;
}
