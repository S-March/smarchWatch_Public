/**
 \addtogroup UTILITIES
 \{
 */

/**
 ****************************************************************************************
 *
 * @file rf_tools_common.h
 *
 * @brief RF Tools common part
 *
 * Copyright (C) 2015 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef RF_TOOLS_COMMON_H
#define RF_TOOLS_COMMON_H

typedef void (*systick_cb_t)(void);

void rf_tools_start_systick(systick_cb_t cb, uint32_t ticks);
void rf_tools_stop_systick(void);

#endif /* RF_TOOLS_COMMON_H */
/**
 \}
 */
