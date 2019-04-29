/**
 ****************************************************************************************
 *
 * @file ftdf_config_template.h
 *
 * @brief FTDF configuration template file
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef FTDF_CONFIG_TEMPLATE_H_
#define FTDF_CONFIG_TEMPLATE_H_

/**
 * \remark      phy configuration values in microseconds
 */
#define FTDF_PHYTXSTARTUP 99
#define FTDF_PHYTXLATENCY 40
#define FTDF_PHYTXFINISH  19
#define FTDF_PHYTRXWAIT   16
#define FTDF_PHYRXSTARTUP 50
#define FTDF_PHYRXLATENCY 200
#define FTDF_PHYENABLE    0

/**
 * \remark      See FTDF_GET_MSG_BUFFER in ftdf.h
 */
#define FTDF_GET_MSG_BUFFER                 appl_get_msg_buffer

/**
 * \remark      See FTDF_REL_MSG_BUFFER in ftdf.h
 */
#define FTDF_REL_MSG_BUFFER                 appl_rel_msg_buffer

/**
 * \remark      See FTDF_RCV_MSG in ftdf.h
 */
#define FTDF_RCV_MSG                        appl_rcv_msg

/**
 * \remark      See FTDF_GET_DATA_BUFFER in ftdf.h
 */
#define FTDF_GET_DATA_BUFFER                appl_get_data_buffer

/**
 * \remark      See FTDF_REL_DATA_BUFFER in ftdf.h
 */
#define FTDF_REL_DATA_BUFFER                appl_rel_data_buffer

/**
 * \remark      See FTDF_GET_EXT_ADDRESS in ftdf.h
 */
#define FTDF_GET_EXT_ADDRESS                appl_get_ext_address

/**
 * \remark      See FTDF_RCV_FRAME_TRANSPARENT in ftdf.h
 */
#define FTDF_RCV_FRAME_TRANSPARENT          appl_rcv_frame_transparent

/**
 * \remark      See FTDF_SEND_FRAME_TRANSPARENT_CONFIRM in ftdf.h
 */
#define FTDF_SEND_FRAME_TRANSPARENT_CONFIRM appl_send_frame_transparent_confirm

/**
 * \remark      See FTDF_WAKE_UP_READY in ftdf.h
 */
#define FTDF_WAKE_UP_READY                  appl_wake_up_ready

/**
 * \remark      Critical section
 */
#define ftdf_critical_var() uint8_t cpu_sr

#define ftdf_enter_critical()                                                      \
        do {                                                                       \
                asm (                                                              \
                    "mrs   r0, PRIMASK\n\t"                                        \
                    "cpsid i\n\t"                                                  \
                    "strb r0, %[output]"                                           \
                    :[ output ] "=m" ( cpuSR ) ::"r0" );                           \
                    DBG_CONFIGURE_HIGH(CMN_TIMING_DEBUG, CMNDBG_CRITICAL_SECTION); \
        }                                                                          \
        while (0)

#define ftdf_exit_critical()                                                  \
        do {                                                                  \
                DBG_CONFIGURE_LOW(CMN_TIMING_DEBUG, CMNDBG_CRITICAL_SECTION); \
                asm (                                                         \
                "ldrb r0, %[input]\n\t"                                       \
                "msr PRIMASK,r0;\n\t"                                         \
                ::[ input ] "m" ( cpuSR ) : "r0" );                           \
        }                                                                     \
        while (0)

#endif /* FTDF_CONFIG_TEMPLATE_H_ */
