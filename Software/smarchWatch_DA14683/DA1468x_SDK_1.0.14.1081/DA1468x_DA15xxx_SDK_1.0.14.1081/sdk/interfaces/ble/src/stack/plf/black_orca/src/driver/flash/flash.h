/**
 ****************************************************************************************
 *
 * @file flash.h
 *
 * @brief Flash driver interface
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef FLASH_H_
#define FLASH_H_

#include <stdint.h>               // standard integer functions

/**
 ****************************************************************************************
 * @addtogroup FLASH
 * @ingroup DRIVERS
 *
 * @brief Flash memory driver
 *
 * @{
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

///Flash type code used to select the correct erasing and programming algorithm
#define FLASH_TYPE_UNKNOWN             0
#define FLASH_TYPE_INTEL_28F320C3      1
#define FLASH_TYPE_INTEL_28F800C3      2
#define FLASH_TYPE_NUMONYX_M25P128     3

///Base address of Flash on system bus
#define FLASH_BASE_ADDR          0x03000000

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize flash driver.
 ****************************************************************************************
 */
void flash_init(void);

/**
 ****************************************************************************************
 * @brief   Identify the flash device.
 *
 * This function is used to read the flash device ID.
 * 
 * Note: callback parameter is not used
 *
 * @param[out]   id          Pointer to id location
 * @param[in]    callback    Callback for end of identification
 * @return       status      0 if operation can start successfully
 ****************************************************************************************
 */
uint8_t flash_identify(uint8_t* id, void (*callback)(void));

/**
 ****************************************************************************************
 * @brief   Erase a flash section.
 *
 * This function is used to erase a part of the flash memory.
 * 
 * Note: callback parameter is not used
 *
 * @param[in]    flash_type  Flash type
 * @param[in]    offset      Starting offset from the beginning of the flash device
 * @param[in]    size        Size of the portion of flash to erase
 * @param[in]    callback    Callback for end of erase
 * @return       status      0 if operation can start successfully
 ****************************************************************************************
 */
uint8_t flash_erase(uint8_t flash_type, uint32_t offset, uint32_t size, void (*callback)(void));

/**
 ****************************************************************************************
 * @brief   Write a flash section.
 *
 * This function is used to write a part of the flash memory.
 * 
 * Note: callback parameter is not used
 *
 * @param[in]    flash_type  Flash type
 * @param[in]    offset      Starting offset from the beginning of the flash device
 * @param[in]    length      Size of the portion of flash to write
 * @param[in]    buffer      Pointer on data to write
 * @param[in]    callback    Callback for end of write
 * @return       status      0 if operation can start successfully
 ****************************************************************************************
 */
uint8_t flash_write(uint8_t flash_type, uint32_t offset, uint32_t length, uint8_t *buffer, void (*callback)(void));

/**
 ****************************************************************************************
 * @brief   Read a flash section.
 *
 * This function is used to read a part of the flash memory.
 * 
 * Note: callback parameter is not used
 *
 * @param[in]    flash_type  Flash type
 * @param[in]    offset      Starting offset from the beginning of the flash device
 * @param[in]    length      Size of the portion of flash to read
 * @param[out]   buffer      Pointer on data to read
 * @param[in]    callback    Callback for end of read
 * @return       status      0 if operation can start successfully
 ****************************************************************************************
 */
uint8_t flash_read(uint8_t flash_type, uint32_t offset, uint32_t length, uint8_t *buffer, void (*callback)(void));


/// @} FLASH

#endif // FLASH_H_
