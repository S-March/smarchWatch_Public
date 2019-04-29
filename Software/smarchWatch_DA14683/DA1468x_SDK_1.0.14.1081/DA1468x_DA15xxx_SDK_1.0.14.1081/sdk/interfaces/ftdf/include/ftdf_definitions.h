/**
 \addtogroup INTERFACES
 \{
 \addtogroup FTDF
 \{
 \brief IEEE 802.15.4 Wireless
 */
/**
 ****************************************************************************************
 *
 * @file ftdf_definitions.h
 *
 * @brief FTDF configuration macro values.
 *
 * Copyright (C) 2015-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef FTDF_DEFINITIONS_H_
#define FTDF_DEFINITIONS_H_

/** \brief FP bit will always be set when acknowledging a data request command.
 *
 * \see FTDF_FP_BIT_MODE
 * */
#define FTDF_FP_BIT_MODE_ALWAYS_SET             0

/**
 * \brief FPPR HW will be used to determine the value of the FP bit when acknowledging a data
 * request command.
 *
 * FPPR (Frame Pending Processing RAM) is used to control the value of the FP bit in a HW generated
 * acknowledgment frame. In order to do so, FPPR must be programmed with the destination addresses
 * of all frames awaiting transmission. There is a number of 24 entries in the FPPR table, and each
 * entry may contain one extended or 4 short addresses.
 *
 * There are two APIs provided by the SDK. A low-level API (prefixed "FTDF_fppr*") that can be used
 * to access FPPR directly, and a high-level FSM API (prefixed "FTDF_fpFsm*"), itself built on the
 * low-level calls, that is also used by UMAC.
 *
 * \see FTDF_FP_BIT_MODE
 * \see ftdf_fp_fsm_rxt_address_new()
 * \see ftdf_fp_fsm_short_address_new
 */
#define FTDF_FP_BIT_MODE_AUTO                   1

/**
 * \brief FP bit will be software controlled. Usage of this mode is not recommended since it assumes
 * that software is fast enough to process pending indirect packets to be sent.
 *
 * \see FTDF_FP_BIT_MODE
 */
#define FTDF_FP_BIT_MODE_MANUAL                 2

#endif /* FTDF_DEFINITIONS_H_ */

/**
 * \}
 * \}
 * \}
 */

