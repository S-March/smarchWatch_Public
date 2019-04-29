/**
 ****************************************************************************************
 *
 * @file ftdf_config_mac_api.h
 *
 * @brief FTDF MAC API configuration template file
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef FTDF_CONFIG_MAC_API_H_
#define FTDF_CONFIG_MAC_API_H_

#include "osal.h"
#include "ftdf_definitions.h"

/**
 * \remark      phy configuration values in microseconds
 */
#define FTDF_PHYTXSTARTUP 0x4c
#define FTDF_PHYTXLATENCY 0x02
#define FTDF_PHYTXFINISH  0x00
#define FTDF_PHYTRXWAIT   0x22
#define FTDF_PHYRXSTARTUP 0x54
#define FTDF_PHYRXLATENCY 0
#define FTDF_PHYENABLE    0x20

/**
 * \remark      See FTDF_GET_MSG_BUFFER in ftdf.h
 */
#define FTDF_GET_MSG_BUFFER                 ad_ftdf_get_msg_buffer

/**
 * \remark      See FTDF_REL_MSG_BUFFER in ftdf.h
 */
#define FTDF_REL_MSG_BUFFER                 ad_ftdf_rel_msg_buffer

/**
 * \remark      See FTDF_REC_MSG in ftdf.h
 */
#define FTDF_RCV_MSG                        ad_ftdf_rcv_msg

/**
 * \remark      See FTDF_GET_DATA_BUFFER in ftdf.h
 */
#define FTDF_GET_DATA_BUFFER                ad_ftdf_get_data_buffer

/**
 * \remark      See FTDF_REL_DATA_BUFFER in ftdf.h
 */
#define FTDF_REL_DATA_BUFFER                ad_ftdf_rel_data_buffer

/**
 * \remark      See FTDF_GET_EXT_ADDRESS in ftdf.h
 */
#define FTDF_GET_EXT_ADDRESS                ad_ftdf_get_ext_address

/**
 * \remark      See FTDF_RCV_FRAME_TRANSPARENT in ftdf.h
 */
#define FTDF_RCV_FRAME_TRANSPARENT          ftdf_rcv_frame_transparent

/**
 * \remark      See FTDF_SEND_FRAME_TRANSPARENT_CONFIRM in ftdf.h
 */
#define FTDF_SEND_FRAME_TRANSPARENT_CONFIRM ftdf_send_frame_transparent_confirm

/**
 * \remark      See FTDF_WAKE_UP_READY in ftdf.h
 */
#define FTDF_WAKE_UP_READY                  ad_ftdf_wake_up_ready

/**
 * \remark      See FTDF_SLEEP_CALLBACK in ftdf.h
 */
#define FTDF_SLEEP_CALLBACK                 ad_ftdf_sleep_cb

#define FTDF_LMACREADY4SLEEP_CB             ad_ftdf_sleep_when_possible

/**
 * \remark      Critical section
 */
#define ftdf_critical_var()

#define ftdf_enter_critical()               OS_ENTER_CRITICAL_SECTION()

#define ftdf_exit_critical()                OS_LEAVE_CRITICAL_SECTION()

#ifndef FTDF_DBG_BUS_ENABLE
/**
 * \brief Whether FTDF debug bus will be available or not.
 *
 * Define to 0 for production software.
 *
 * Refer to @ref ad_ftdf_dbgBusGpioConfig for the GPIO pins used for the debug bus.
 */
#define FTDF_DBG_BUS_ENABLE                     (0)
#endif

#if FTDF_DBG_BUS_ENABLE
#define FTDF_DBG_BUS_GPIO_CONFIG   ad_ftdf_dbg_bus_gpio_config

#ifndef FTDF_DBG_BUS_USE_GPIO_P1_3_P2_2
/**
 * \brief Enables FTDF diagnostics on diagnostic pins 6 and 7 on GPIO P1_3 and P2_3.
 *
 * When enabled, UART must use pins other than the default P1_3, P2_3.
 */
#define FTDF_DBG_BUS_USE_GPIO_P1_3_P2_2         (0)
#endif

#ifndef FTDF_DBG_BUS_USE_SWDIO_PIN
/**
 * \brief Enables diagnostics on diagnostic pin 4 on GPIO P0_6.
 *
 * When enabled, the debugger must be disabled since SWD uses the same pin for SWDIO.
 */
#define FTDF_DBG_BUS_USE_SWDIO_PIN              (0)
#endif

#ifndef FTDF_DBG_BUS_USE_PORT_4
/**
 * \brief Uses Port 4 (instead of GPIOs at Ports 0, 1 and 2) for diagnostics
 *
 * When enabled, FTDF diagnostics pins uses P4_0 to P4_7
 */
#define FTDF_DBG_BUS_USE_PORT_4                 (0)
#endif

#endif /* FTDF_DBG_BUS_ENABLE */

#ifndef FTDF_DBG_BLOCK_SLEEP_ENABLE
/**
 * \brief Enable FTDF block sleep monitoring via GPIO.
 *
 * When enabled, a GPIO pin is used to monitor FTDF sleep. The pin is set low when the block
 * sleeps and high when the block is up.
 *
 * \sa FTDF_DBG_BLOCK_SLEEP_GPIO_PORT
 * \sa FTDF_DBG_BLOCK_SLEEP_GPIO_PIN
 */
#define FTDF_DBG_BLOCK_SLEEP_ENABLE             (0)
#endif

#if FTDF_DBG_BLOCK_SLEEP_ENABLE
#ifndef FTDF_DBG_BLOCK_SLEEP_GPIO_PORT
/**
 * \brief GPIO port for monitoring FTDF block sleep.
 */
#define FTDF_DBG_BLOCK_SLEEP_GPIO_PORT          HW_GPIO_PORT_4
#endif

#ifndef FTDF_DBG_BLOCK_SLEEP_GPIO_PIN
/**
 * \brief GPIO pin for monitoring FTDF block sleep.
 */
#define FTDF_DBG_BLOCK_SLEEP_GPIO_PIN           HW_GPIO_PIN_7
#endif

#endif /* FTDF_DBG_BLOCK_SLEEP_ENABLE */

#ifndef FTDF_USE_AUTO_PTI
#define FTDF_USE_AUTO_PTI                       0
#endif

#ifndef FTDF_FP_BIT_MODE
/** \brief FP bit processing mode. */
#define FTDF_FP_BIT_MODE                        FTDF_FP_BIT_MODE_AUTO
#endif /* FTDF_FP_BIT_MODE */

#ifndef FTDF_USE_LPDP
#define FTDF_USE_LPDP                           1
#endif

#ifndef FTDF_FP_BIT_TEST_MODE
#define FTDF_FP_BIT_TEST_MODE                   0
#endif /* FTDF_FP_BIT_TEST_MODE */

#ifndef FTDF_USE_SLEEP_DURING_BACKOFF
/**
 * \brief Enable FTDF block sleep during backoff.
 *
 * Sleeping during backoff is a power optimization for CSMA/CA transmissions. When enabled, the FTDF
 * adapter will attempt to put the FTDF block to sleep during backoff (as part of a CSMA/CA
 * transmission). If the backoff period is long enough, the FTDF block will be put to sleep. The
 * FTDF adapter ensures that the block is woken up in time in order to resume transmission and
 * restores all CCMA/CA state data to the hardware. Note that the hardware block supports this
 * special resume functionality, so the CSMA/CA state machine resumes as if it has never been
 * interrupted. The power gain can be quite large, especially if the random backoff period is long
 * due to a retransmission.
 */
#define FTDF_USE_SLEEP_DURING_BACKOFF           1
#endif /* FTDF_USE_SLEEP_DURING_BACKOFF */


#endif /* FTDF_CONFIG_MAC_API_H_ */

