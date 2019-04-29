/**
 \addtogroup INTERFACES
 \{
 \addtogroup FTDF
 \{
 */

/**
 ****************************************************************************************
 *
 * @file ad_ftdf_config.h
 *
 * @brief FTDF Adapter API
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef AD_FTDF_CONFIG_H
#define AD_FTDF_CONFIG_H

#ifndef FTDF_PHY_API
#include "queue.h"
#endif

#include "ftdf.h"


/**
 * \brief Sleep When Idle
 *
 * If set, the block will sleep when Idle
 *
 */
#define AD_FTDF_SLEEP_WHEN_IDLE 1

/**
 * \brief Idle Timeout
 *
 * Idle Timeout, in OS scheduler ticks, after which, if still Idle, FTDF will
 * be put to sleep. Only applicable if AD_FTDF_SLEEP_WHEN_IDLE 1
 *
 */
#define AD_FTDF_IDLE_TIMEOUT 1

/**
 * \brief UP Queue size
 *
 * Defines the UP (UMAC to client app) Queue length, in number of messages
 *
 */
#define AD_FTDF_UP_QUEUE_LENGTH    16

/**
 * \brief DOWN Queue size
 *
 * Defines the DOWN (client app to UMAC) Queue length, in number of messages
 *
 */
#define AD_FTDF_DOWN_QUEUE_LENGTH  16

/**
 * \brief Low Power Clock cycle
 *
 * Defines the Low Power clock cycle in pico secs. Used for computation needed
 * for sleeping / waking up FTDF
 *
 */
#define AD_FTDF_LP_CLOCK_CYCLE 30517578

/**
 * \brief Wake Up Latency
 *
 * Defines the Wake Up Latency expressed in Low Power clock cycles, that is
 * the number of LP clock cycles needed for the FTDF to be fully operational
 * (calculations and FTDF timer synchronization)
 *
 */
#define AD_FTDF_WUP_LATENCY 10

/**
 * \brief Sleep value compensation
 *
 * Defines the sleep value compensation expressed in microseconds. This value
 * is subtracted from the sleep time returned by ftdf_can_sleep()
 */
#define AD_FTDF_SLEEP_COMPENSATION 1500

#endif /* AD_FTDF_CONFIG_H */
/**
 \}
 \}
 \}
 */
