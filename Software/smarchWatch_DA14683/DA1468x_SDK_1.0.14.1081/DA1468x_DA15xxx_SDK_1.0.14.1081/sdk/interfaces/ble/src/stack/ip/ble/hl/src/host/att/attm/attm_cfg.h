/**
 ****************************************************************************************
 *
 * @file attm_cfg.h
 *
 * @brief Header file - ATTMCFG.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef ATTM_CFG_H_
#define ATTM_CFG_H_

/**
 ****************************************************************************************
 * @addtogroup ATTMCFG Settings
 * @ingroup ATTM
 * @brief ATTM Configuration file
 *
 * The ATTMCFG is the attribute configuration holder for @ref ATTM "ATTM".
 *
 * @{
 ****************************************************************************************
 */
 
/*
 * INCLUDES
 ****************************************************************************************
 */
 
#include "rwip_config.h"
#include "gap_cfg.h"

/*
 * DEFINES
 ****************************************************************************************
 */
 
/// Maximum Transmission Unit
#define ATT_DEFAULT_MTU                                 (23)
/// 30 seconds transaction timer
#define ATT_TRANS_RTX                                   rom_cfg_table[att_trans_rtx_pos] //(0x0BB8)
/// Acceptable encryption key size - strict access
#define ATT_SEC_ENC_KEY_SIZE                            rom_cfg_table[att_sec_enc_key_size_pos] //(0x10)


/// Maximum attribute value length
#define ATT_MAX_VALUE                                   (GAP_MAX_LE_MTU)

/// Macro used to convert CPU integer define to LSB first 16-bits UUID
#define ATT_UUID_16(uuid)                               (uuid)
/// @} ATTMCFG

#endif // ATTM_CFG_H_
