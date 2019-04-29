/**
 ****************************************************************************************
 *
 * @file co_version.h
 *
 * @brief Version definitions for BT4.0
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef _CO_VERSION_H_
#define _CO_VERSION_H_
/**
 ****************************************************************************************
 * @defgroup CO_VERSION Version Defines
 * @ingroup COMMON
 *
 * @brief Bluetooth Controller Version definitions.
 *
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#if ((dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A) \
                && (dg_configBLACK_ORCA_IC_STEP == BLACK_ORCA_IC_STEP_E))
/// RWBT SW Major Version
#define RWBLE_SW_VERSION_MAJOR                   8
/// RWBT SW Minor Version
#define RWBLE_SW_VERSION_MINOR                   0
/// RWBT SW Build Version
#define RWBLE_SW_VERSION_BUILD                   15
/// RWBT SW Major Version
#define RWBLE_SW_VERSION_SUB_BUILD               0
#elif ((dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_B) \
                && (dg_configBLACK_ORCA_IC_STEP == BLACK_ORCA_IC_STEP_B))
/// RWBT SW Major Version
#define RWBLE_SW_VERSION_MAJOR                   8
/// RWBT SW Minor Version
#define RWBLE_SW_VERSION_MINOR                   1
/// RWBT SW Build Version
#define RWBLE_SW_VERSION_BUILD                   15
/// RWBT SW Major Version
#define RWBLE_SW_VERSION_SUB_BUILD               0
#else
#error "Unsupported chip version"
#endif


/// @} CO_VERSION


#endif // _CO_VERSION_H_
