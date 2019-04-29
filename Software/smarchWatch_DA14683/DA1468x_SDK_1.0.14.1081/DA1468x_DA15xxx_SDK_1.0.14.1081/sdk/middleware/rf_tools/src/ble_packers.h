/**
 ****************************************************************************************
 *
 * @file ble_packers.h
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef PACKERS_H_
#define PACKERS_H_

#include <stdint.h>

static inline uint8_t r8le(uint8_t *p)
{
	return (uint8_t) p[0];
}

static inline uint16_t r16le(uint8_t *p)
{
	return (uint16_t) (p[0] | (p[1] << 8));
}

static inline uint16_t r32le(uint8_t *p)
{
        return (uint16_t) (p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24));
}


static inline void w8le(uint8_t *p, uint8_t v)
{
	p[0] = v;
}

static inline void w16le(uint8_t *p, uint16_t v )
{
	p[0] = v & 0xff;
	p[1] = (v >> 8) & 0xff;
}

static inline void w32le(uint8_t *p, uint32_t v )
{
        p[0] = v & 0xff;
        p[1] = (v >> 8) & 0xff;
        p[2] = (v >> 16) & 0xff;
        p[3] = (v >> 24) & 0xff;
}

#define padvN(p, N) \
	p += (N)

#define padv(ptr, type) \
	padvN(ptr, sizeof(type))


#endif /* PACKERS_H_ */
