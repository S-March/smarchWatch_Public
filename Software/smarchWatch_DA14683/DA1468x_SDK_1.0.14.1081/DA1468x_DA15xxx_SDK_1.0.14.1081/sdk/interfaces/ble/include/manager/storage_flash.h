/**
 ****************************************************************************************
 *
 * @file storage_flash.h
 *
 * @brief BLE Manager flash storage interface
 *
 * Copyright (C) 2015 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef STORAGE_FLASH_H_
#define STORAGE_FLASH_H_

/**
 * Initialize flash storage
 *
 * This should be called only once on platform startup.
 *
 */
void storage_flash_init(void);

/**
 * Load BLE data from flash storage
 *
 * Loads data from BLE storage partition and updates device list.
 *
 */
void storage_flash_load(void);

/**
 * Save BLE data to flash storage
 *
 * Saves bonded devices data to BLE storage partition. This should be called when bonded devices are
 * modified.
 *
 */
void storage_flash_save(void);

#endif /* STORAGE_FLASH_H_ */
