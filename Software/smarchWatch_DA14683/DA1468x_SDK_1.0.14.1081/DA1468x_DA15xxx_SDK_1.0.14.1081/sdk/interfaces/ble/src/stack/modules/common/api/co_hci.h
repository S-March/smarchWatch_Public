/**
 ****************************************************************************************
 *
 * @file co_hci.h
 *
 * @brief This file contains the HCI Bluetooth defines, enumerations and structures
 *        definitions for use by all modules in RW stack.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 ****************************************************************************************
 */

#ifndef CO_HCI_H_
#define CO_HCI_H_

/**
 ****************************************************************************************
 * @addtogroup COMMON Common SW Block
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdbool.h>       // standard boolean definitions
#include <stddef.h>        // standard definitions
#include <stdint.h>        // standard integer definitions

#include "rwip_config.h"   // IP configuration
#include "co_version.h"

/*
 * DEFINES
 ****************************************************************************************
 */


/******************************************************************************************/
/* -------------------------   H4TL DEFINITIONS Part IV.A    -----------------------------*/
/******************************************************************************************/

///HCI Transport Header length - change if different transport
#define HCI_TRANSPORT_HDR_LEN                       0x01

///UART header: command message type
#define HCI_CMD_MSG_TYPE                            0x01

///UART header: ACL data message type
#define HCI_ACL_MSG_TYPE                            0x02

///UART header: Synchronous data message type
#define HCI_SYNC_MSG_TYPE                           0x03

///UART header: event message type
#define HCI_EVT_MSG_TYPE                            0x04

/******************************************************************************************/
/* -------------------------   HCI DEFINITIONS Part II.E     -----------------------------*/
/******************************************************************************************/

///HCI Command Opcode byte length
#define HCI_CMD_OPCODE_LEN         (0x02)

///HCI Event code byte length
#define HCI_EVT_CODE_LEN           (0x01)

///HCI Command/Event parameter length field byte length
#define HCI_CMDEVT_PARLEN_LEN      (0x01)

///HCI Command header length
#define HCI_CMD_HDR_LEN            (HCI_CMD_OPCODE_LEN + HCI_CMDEVT_PARLEN_LEN)

///HCI Event header length
#define HCI_EVT_HDR_LEN            (HCI_EVT_CODE_LEN + HCI_CMDEVT_PARLEN_LEN)

/// HCI ACL header: handle and flags decoding
#define HCI_ACL_HDR_HDL_FLAGS_POS  (0)
#define HCI_ACL_HDR_HDL_FLAGS_LEN  (2)
#define HCI_ACL_HDR_HDL_POS        (0)
#define HCI_ACL_HDR_HDL_MASK       (0x0FFF)
#define HCI_ACL_HDR_PB_FLAG_POS    (12)
#define HCI_ACL_HDR_PB_FLAG_MASK   (0x3000)
#define HCI_ACL_HDR_BC_FLAG_POS    (14)
#define HCI_ACL_HDR_BC_FLAG_MASK   (0xC000)
#define HCI_ACL_HDR_DATA_FLAG_POS  (12)
#define HCI_ACL_HDR_DATA_FLAG_MASK (0xF000)

/// HCI ACL header: data length field length
#define HCI_ACL_HDR_DATA_LEN_POS   (HCI_ACL_HDR_HDL_FLAGS_LEN)
#define HCI_ACL_HDR_DATA_LEN_LEN   (2)

///HCI ACL data packet header length
#define HCI_ACL_HDR_LEN            (HCI_ACL_HDR_HDL_FLAGS_LEN + HCI_ACL_HDR_DATA_LEN_LEN)

///HCI sync data packet header length
#define HCI_SYNC_HDR_LEN           (0x03)

///HCI Command Complete Event minimum parameter length: 1(nb_pk)+2(opcode)
#define HCI_CCEVT_HDR_PARLEN       (0x03)

///HCI Command Complete Event header length:1(code)+1(len)+1(pk)+2(opcode)
#define HCI_CCEVT_HDR_LEN          (HCI_EVT_HDR_LEN + HCI_CCEVT_HDR_PARLEN)

///HCI Basic Command Complete Event packet length
#define HCI_CCEVT_BASIC_LEN        (HCI_CCEVT_HDR_LEN + 1)

///HCI Command Status Event parameter length - constant
#define HCI_CSEVT_PARLEN           (0x04)

///HCI Command Status Event length:1(code)+1(len)+1(st)+1(pk)+2(opcode)
#define HCI_CSEVT_LEN              (HCI_EVT_HDR_LEN + HCI_CSEVT_PARLEN)

///HCI Reset Command parameter length
#define HCI_RESET_CMD_PARLEN       0

/// Default return parameter length for HCI Command Complete Event
#define HCI_CCEVT_BASIC_RETPAR_LEN 1

/// Max HCI commands param size
#define HCI_MAX_CMD_PARAM_SIZE    255

/// Macro to extract OCF from OPCODE
#define HCI_OP2OCF(opcode)        ((opcode) & 0x03FF)

/// Macro to extract OGF from OPCODE
#define HCI_OP2OGF(opcode)        ((opcode) >> 10 & 0x003F)

/// Macro to create OPCODE from OGF and OCF
#define HCI_OPCODE(ocf, ogf)      (((ogf) << 10) | ocf)

/**************************************************************************************
 **************                       HCI COMMANDS                     ****************
 **************************************************************************************/

///HCI enumeration of possible Command OGF values.
enum
{
    ///HCI Link Control Commands Group OGF code
    LK_CNTL_OGF = 0x01,
    ///HCI Link Policy Commands Group OGF code
    LK_POL_OGF,
    ///HCI Controller and Baseband Commands Group OGF code
    CNTLR_BB_OGF,
    ///HCI Information Parameters Commands Group OGF code
    INFO_PAR_OGF,
    ///HCI Status Commands Group OGF code
    STAT_PAR_OGF,
    ///HCI Test Commands Group OGF code
    TEST_OGF,
    ///HCI Low Energy Commands Group OGF code
    LE_CNTLR_OGF=0x08,
    ///HCI Vendor Specific Group OGF code
    VS_OGF = 0x3F,
    MAX_OGF
};

///Commands Opcodes: OGF(6b) | OCF(10b)
/* Some Abbreviation used in names:
 *  - LK   = Link Key
 *  - RD   = Read
 *  - WR   = Write
 *  - REM  = Remote
 *  - STG  = Settings
 *  - CON  = Connection
 *  - CHG  = Change
 *  - DFT  = Default
 *  - PER  = Periodic
 */

#define HCI_NO_OPERATION_CMD_OPCODE                 0x0000

//Link Control Commands
#define HCI_INQ_CMD_OPCODE                          0x0401
#define HCI_INQ_CANCEL_CMD_OPCODE                   0x0402
#define HCI_PER_INQ_MODE_CMD_OPCODE                 0x0403
#define HCI_EXIT_PER_INQ_MODE_CMD_OPCODE            0x0404
#define HCI_CREATE_CON_CMD_OPCODE                   0x0405
#define HCI_DISCONNECT_CMD_OPCODE                   0x0406
#define HCI_CREATE_CON_CANCEL_CMD_OPCODE            0x0408
#define HCI_ACCEPT_CON_REQ_CMD_OPCODE               0x0409
#define HCI_REJECT_CON_REQ_CMD_OPCODE               0x040A
#define HCI_LK_REQ_REPLY_CMD_OPCODE                 0x040B
#define HCI_LK_REQ_NEG_REPLY_CMD_OPCODE             0x040C
#define HCI_PIN_CODE_REQ_REPLY_CMD_OPCODE           0x040D
#define HCI_PIN_CODE_REQ_NEG_REPLY_CMD_OPCODE       0x040E
#define HCI_CHG_CON_PKT_TYPE_CMD_OPCODE             0x040F
#define HCI_AUTH_REQ_CMD_OPCODE                     0x0411
#define HCI_SET_CON_ENC_CMD_OPCODE                  0x0413
#define HCI_CHG_CON_LK_CMD_OPCODE                   0x0415
#define HCI_MASTER_LK_CMD_OPCODE                    0x0417
#define HCI_REM_NAME_REQ_CMD_OPCODE                 0x0419
#define HCI_REM_NAME_REQ_CANCEL_CMD_OPCODE          0x041A
#define HCI_RD_REM_SUPP_FEATS_CMD_OPCODE            0x041B
#define HCI_RD_REM_EXT_FEATS_CMD_OPCODE             0x041C
#define HCI_RD_REM_VER_INFO_CMD_OPCODE              0x041D
#define HCI_RD_CLK_OFF_CMD_OPCODE                   0x041F
#define HCI_RD_LMP_HDL_CMD_OPCODE                   0x0420
#define HCI_SETUP_SYNC_CON_CMD_OPCODE               0x0428
#define HCI_ACCEPT_SYNC_CON_REQ_CMD_OPCODE          0x0429
#define HCI_REJECT_SYNC_CON_REQ_CMD_OPCODE          0x042A
#define HCI_IO_CAP_REQ_REPLY_CMD_OPCODE             0x042B
#define HCI_USER_CFM_REQ_REPLY_CMD_OPCODE           0x042C
#define HCI_USER_CFM_REQ_NEG_REPLY_CMD_OPCODE       0x042D
#define HCI_USER_PASSKEY_REQ_REPLY_CMD_OPCODE       0x042E
#define HCI_USER_PASSKEY_REQ_NEG_REPLY_CMD_OPCODE   0x042F
#define HCI_REM_OOB_DATA_REQ_REPLY_CMD_OPCODE       0x0430
#define HCI_REM_OOB_DATA_REQ_NEG_REPLY_CMD_OPCODE   0x0433
#define HCI_IO_CAP_REQ_NEG_REPLY_CMD_OPCODE         0x0434
#define HCI_ENH_SETUP_SYNC_CON_CMD_OPCODE           0x043D
#define HCI_ENH_ACCEPT_SYNC_CON_CMD_OPCODE          0x043E

//Link Policy Commands
#define HCI_HOLD_MODE_CMD_OPCODE                    0x0801
#define HCI_SNIFF_MODE_CMD_OPCODE                   0x0803
#define HCI_EXIT_SNIFF_MODE_CMD_OPCODE              0x0804
#define HCI_PARK_STATE_CMD_OPCODE                   0x0805
#define HCI_EXIT_PARK_STATE_CMD_OPCODE              0x0806
#define HCI_QOS_SETUP_CMD_OPCODE                    0x0807
#define HCI_ROLE_DISCOVERY_CMD_OPCODE               0x0809
#define HCI_SWITCH_ROLE_CMD_OPCODE                  0x080B
#define HCI_RD_LINK_POL_STG_CMD_OPCODE              0x080C
#define HCI_WR_LINK_POL_STG_CMD_OPCODE              0x080D
#define HCI_RD_DFT_LINK_POL_STG_CMD_OPCODE          0x080E
#define HCI_WR_DFT_LINK_POL_STG_CMD_OPCODE          0x080F
#define HCI_FLOW_SPEC_CMD_OPCODE                    0x0810
#define HCI_SNIFF_SUB_CMD_OPCODE                    0x0811

//Controller and Baseband Commands
#define HCI_SET_EVT_MASK_CMD_OPCODE                 0x0C01
#define HCI_RESET_CMD_OPCODE                        0x0C03
#define HCI_SET_EVT_FILTER_CMD_OPCODE               0x0C05
#define HCI_FLUSH_CMD_OPCODE                        0x0C08
#define HCI_RD_PIN_TYPE_CMD_OPCODE                  0x0C09
#define HCI_WR_PIN_TYPE_CMD_OPCODE                  0x0C0A
#define HCI_CREATE_NEW_UNIT_KEY_CMD_OPCODE          0x0C0B
#define HCI_RD_STORED_LK_CMD_OPCODE                 0x0C0D
#define HCI_WR_STORED_LK_CMD_OPCODE                 0x0C11
#define HCI_DEL_STORED_LK_CMD_OPCODE                0x0C12
#define HCI_WR_LOCAL_NAME_CMD_OPCODE                0x0C13
#define HCI_RD_LOCAL_NAME_CMD_OPCODE                0x0C14
#define HCI_RD_CON_ACCEPT_TO_CMD_OPCODE             0x0C15
#define HCI_WR_CON_ACCEPT_TO_CMD_OPCODE             0x0C16
#define HCI_RD_PAGE_TO_CMD_OPCODE                   0x0C17
#define HCI_WR_PAGE_TO_CMD_OPCODE                   0x0C18
#define HCI_RD_SCAN_EN_CMD_OPCODE                   0x0C19
#define HCI_WR_SCAN_EN_CMD_OPCODE                   0x0C1A
#define HCI_RD_PAGE_SCAN_ACT_CMD_OPCODE             0x0C1B
#define HCI_WR_PAGE_SCAN_ACT_CMD_OPCODE             0x0C1C
#define HCI_RD_INQ_SCAN_ACT_CMD_OPCODE              0x0C1D
#define HCI_WR_INQ_SCAN_ACT_CMD_OPCODE              0x0C1E
#define HCI_RD_AUTH_EN_CMD_OPCODE                   0x0C1F
#define HCI_WR_AUTH_EN_CMD_OPCODE                   0x0C20
#define HCI_RD_CLASS_OF_DEV_CMD_OPCODE              0x0C23
#define HCI_WR_CLASS_OF_DEV_CMD_OPCODE              0x0C24
#define HCI_RD_VOICE_STG_CMD_OPCODE                 0x0C25
#define HCI_WR_VOICE_STG_CMD_OPCODE                 0x0C26
#define HCI_RD_AUTO_FLUSH_TO_CMD_OPCODE             0x0C27
#define HCI_WR_AUTO_FLUSH_TO_CMD_OPCODE             0x0C28
#define HCI_RD_NB_BDCST_RETX_CMD_OPCODE             0x0C29
#define HCI_WR_NB_BDCST_RETX_CMD_OPCODE             0x0C2A
#define HCI_RD_HOLD_MODE_ACTIVITY_CMD_OPCODE        0x0C2B
#define HCI_WR_HOLD_MODE_ACTIVITY_CMD_OPCODE        0x0C2C
#define HCI_RD_TX_PWR_LVL_CMD_OPCODE                0x0C2D
#define HCI_RD_SYNC_FLOW_CNTL_EN_CMD_OPCODE         0x0C2E
#define HCI_WR_SYNC_FLOW_CNTL_EN_CMD_OPCODE         0x0C2F
#define HCI_SET_CTRL_TO_HOST_FLOW_CTRL_CMD_OPCODE   0x0C31
#define HCI_HOST_BUF_SIZE_CMD_OPCODE                0x0C33
#define HCI_HOST_NB_CMP_PKTS_CMD_OPCODE             0x0C35
#define HCI_RD_LINK_SUPV_TO_CMD_OPCODE              0x0C36
#define HCI_WR_LINK_SUPV_TO_CMD_OPCODE              0x0C37
#define HCI_RD_NB_SUPP_IAC_CMD_OPCODE               0x0C38
#define HCI_RD_CURR_IAC_LAP_CMD_OPCODE              0x0C39
#define HCI_WR_CURR_IAC_LAP_CMD_OPCODE              0x0C3A
#define HCI_SET_AFH_HOST_CH_CLASS_CMD_OPCODE        0x0C3F
#define HCI_RD_INQ_SCAN_TYPE_CMD_OPCODE             0x0C42
#define HCI_WR_INQ_SCAN_TYPE_CMD_OPCODE             0x0C43
#define HCI_RD_INQ_MODE_CMD_OPCODE                  0x0C44
#define HCI_WR_INQ_MODE_CMD_OPCODE                  0x0C45
#define HCI_RD_PAGE_SCAN_TYPE_CMD_OPCODE            0x0C46
#define HCI_WR_PAGE_SCAN_TYPE_CMD_OPCODE            0x0C47
#define HCI_RD_AFH_CH_ASSESS_MODE_CMD_OPCODE        0x0C48
#define HCI_WR_AFH_CH_ASSESS_MODE_CMD_OPCODE        0x0C49
#define HCI_RD_EXT_INQ_RSP_CMD_OPCODE               0x0C51
#define HCI_WR_EXT_INQ_RSP_CMD_OPCODE               0x0C52
#define HCI_REFRESH_ENC_KEY_CMD_OPCODE              0x0C53
#define HCI_RD_SP_MODE_CMD_OPCODE                   0x0C55
#define HCI_WR_SP_MODE_CMD_OPCODE                   0x0C56
#define HCI_RD_LOC_OOB_DATA_CMD_OPCODE              0x0C57
#define HCI_RD_INQ_RSP_TX_PWR_LVL_CMD_OPCODE        0x0C58
#define HCI_WR_INQ_TX_PWR_LVL_CMD_OPCODE            0x0C59
#define HCI_RD_DFT_ERR_DATA_REP_CMD_OPCODE          0x0C5A
#define HCI_WR_DFT_ERR_DATA_REP_CMD_OPCODE          0x0C5B
#define HCI_ENH_FLUSH_CMD_OPCODE                    0x0C5F
#define HCI_SEND_KEYPRESS_NOTIF_CMD_OPCODE          0x0C60
#define HCI_SET_EVT_MASK_PAGE_2_CMD_OPCODE          0x0C63
#define HCI_RD_FLOW_CNTL_MODE_CMD_OPCODE            0x0C66
#define HCI_WR_FLOW_CNTL_MODE_CMD_OPCODE            0x0C67
#define HCI_RD_ENH_TX_PWR_LVL_CMD_OPCODE            0x0C68
#define HCI_RD_LE_HOST_SUPP_CMD_OPCODE              0x0C6C
#define HCI_WR_LE_HOST_SUPP_CMD_OPCODE              0x0C6D
#define HCI_RD_AUTH_PAYL_TO_CMD_OPCODE              0x0C7B
#define HCI_WR_AUTH_PAYL_TO_CMD_OPCODE              0x0C7C
#define HCI_RD_EXT_PAGE_TO_CMD_OPCODE               0x0C7E
#define HCI_WR_EXT_PAGE_TO_CMD_OPCODE               0x0C7F
#define HCI_RD_EXT_INQ_LEN_CMD_OPCODE               0x0C80
#define HCI_WR_EXT_INQ_LEN_CMD_OPCODE               0x0C81

//Info Params
#define HCI_RD_LOCAL_VER_INFO_CMD_OPCODE            0x1001
#define HCI_RD_LOCAL_SUPP_CMDS_CMD_OPCODE           0x1002
#define HCI_RD_LOCAL_SUPP_FEATS_CMD_OPCODE          0x1003
#define HCI_RD_LOCAL_EXT_FEATS_CMD_OPCODE           0x1004
#define HCI_RD_BUFF_SIZE_CMD_OPCODE                 0x1005
#define HCI_RD_BD_ADDR_CMD_OPCODE                   0x1009
#define HCI_RD_LOCAL_SUPP_CODECS_CMD_OPCODE         0x100B

//Status Params
#define HCI_RD_FAIL_CONTACT_CNT_CMD_OPCODE          0x1401
#define HCI_RST_FAIL_CONTACT_CNT_CMD_OPCODE         0x1402
#define HCI_RD_LINK_QUAL_CMD_OPCODE                 0x1403
#define HCI_RD_RSSI_CMD_OPCODE                      0x1405
#define HCI_RD_AFH_CH_MAP_CMD_OPCODE                0x1406
#define HCI_RD_CLK_CMD_OPCODE                       0x1407
#define HCI_RD_ENC_KEY_SIZE_CMD_OPCODE              0x1408

//Testing Commands
#define HCI_RD_LOOP_BACK_MODE_CMD_OPCODE            0x1801
#define HCI_WR_LOOP_BACK_MODE_CMD_OPCODE            0x1802
#define HCI_EN_DUT_MODE_CMD_OPCODE                  0x1803
#define HCI_WR_SP_DBG_MODE_CMD_OPCODE               0x1804

/// LE Commands Opcodes
#define HCI_LE_SET_EVT_MASK_CMD_OPCODE              0x2001
#define HCI_LE_RD_BUFF_SIZE_CMD_OPCODE              0x2002
#define HCI_LE_RD_LOCAL_SUPP_FEATS_CMD_OPCODE       0x2003
#define HCI_LE_SET_RAND_ADDR_CMD_OPCODE             0x2005
#define HCI_LE_SET_ADV_PARAM_CMD_OPCODE             0x2006
#define HCI_LE_RD_ADV_CHNL_TX_PW_CMD_OPCODE         0x2007
#define HCI_LE_SET_ADV_DATA_CMD_OPCODE              0x2008
#define HCI_LE_SET_SCAN_RSP_DATA_CMD_OPCODE         0x2009
#define HCI_LE_SET_ADV_EN_CMD_OPCODE                0x200A
#define HCI_LE_SET_SCAN_PARAM_CMD_OPCODE            0x200B
#define HCI_LE_SET_SCAN_EN_CMD_OPCODE               0x200C
#define HCI_LE_CREATE_CON_CMD_OPCODE                0x200D
#define HCI_LE_CREATE_CON_CANCEL_CMD_OPCODE         0x200E
#define HCI_LE_RD_WLST_SIZE_CMD_OPCODE              0x200F
#define HCI_LE_CLEAR_WLST_CMD_OPCODE                0x2010
#define HCI_LE_ADD_DEV_TO_WLST_CMD_OPCODE           0x2011
#define HCI_LE_RMV_DEV_FROM_WLST_CMD_OPCODE         0x2012
#define HCI_LE_CON_UPDATE_CMD_OPCODE                0x2013
#define HCI_LE_SET_HOST_CH_CLASS_CMD_OPCODE         0x2014
#define HCI_LE_RD_CHNL_MAP_CMD_OPCODE               0x2015
#define HCI_LE_RD_REM_USED_FEATS_CMD_OPCODE         0x2016
#define HCI_LE_ENC_CMD_OPCODE                       0x2017
#define HCI_LE_RAND_CMD_OPCODE                      0x2018
#define HCI_LE_START_ENC_CMD_OPCODE                 0x2019
#define HCI_LE_LTK_REQ_REPLY_CMD_OPCODE             0x201A
#define HCI_LE_LTK_REQ_NEG_REPLY_CMD_OPCODE         0x201B
#define HCI_LE_RD_SUPP_STATES_CMD_OPCODE            0x201C
#define HCI_LE_RX_TEST_CMD_OPCODE                   0x201D
#define HCI_LE_TX_TEST_CMD_OPCODE                   0x201E
#define HCI_LE_TEST_END_CMD_OPCODE                  0x201F
#define HCI_LE_REM_CON_PARAM_REQ_REPLY_CMD_OPCODE   0x2020
#define HCI_LE_REM_CON_PARAM_REQ_NEG_REPLY_CMD_OPCODE 0x2021
#if (RWBLE_SW_VERSION_MAJOR >= 8)
#define HCI_LE_SET_DATA_LENGTH_CMD_OPCODE                     0x2022
#define HCI_LE_READ_SUGGESTED_DEFAULT_DATA_LENGTH_CMD_OPCODE  0x2023
#define HCI_LE_WRITE_SUGGESTED_DEFAULT_DATA_LENGTH_CMD_OPCODE 0x2024
#define HCI_LE_READ_MAX_DATA_LENGTH_CMD_OPCODE                0x202F
#define HCI_LE_READ_LOCAL_P256_PUBLIC_KEY_CMD_OPCODE          0x2025
#define HCI_LE_GENERATE_DHKEY_CMD_OPCODE                      0x2026
#define HCI_LE_ADD_DEV_TO_RSLV_LIST_CMD_OPCODE                0x2027
#define HCI_LE_RMV_DEV_FROM_RSLV_LIST_CMD_OPCODE              0x2028
#define HCI_LE_CLEAR_RSLV_LIST_CMD_OPCODE                     0x2029
#define HCI_LE_RD_RSLV_LIST_SIZE_CMD_OPCODE                   0x202A
#define HCI_LE_RD_PEER_RSLV_ADDR_CMD_OPCODE                   0x202B
#define HCI_LE_RD_LOCAL_RSLV_ADDR_CMD_OPCODE                  0x202C
#define HCI_LE_SET_ADDR_RESOL_EN_CMD_OPCODE                   0x202D
#define HCI_LE_SET_RSLV_PRIV_ADDR_TO_CMD_OPCODE               0x202E
#if (RWBLE_SW_VERSION_MINOR >= 1)
// ESR10
#define HCI_LE_SET_PRIVACY_MODE_CMD_OPCODE                    0x204E
#endif /* (RWBLE_SW_VERSION_MINOR >= 1) */
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */


///Debug commands - OGF = 0x3F (spec)
#define HCI_DBG_RD_MEM_CMD_OPCODE                     0xFC01
#define HCI_DBG_WR_MEM_CMD_OPCODE                     0xFC02
#define HCI_DBG_DEL_PAR_CMD_OPCODE                    0xFC03
#define HCI_DBG_ID_FLASH_CMD_OPCODE                   0xFC05
#define HCI_DBG_ER_FLASH_CMD_OPCODE                   0xFC06
#define HCI_DBG_WR_FLASH_CMD_OPCODE                   0xFC07
#define HCI_DBG_RD_FLASH_CMD_OPCODE                   0xFC08
#define HCI_DBG_RD_PAR_CMD_OPCODE                     0xFC09
#define HCI_DBG_WR_PAR_CMD_OPCODE                     0xFC0A
#define HCI_DBG_WLAN_COEX_CMD_OPCODE                  0xFC0B
#define HCI_DBG_WLAN_COEXTST_SCEN_CMD_OPCODE          0xFC0D
#define HCI_DBG_RD_KE_STATS_CMD_OPCODE                0xFC10
#define HCI_DBG_PLF_RESET_CMD_OPCODE                  0xFC11
#define HCI_DBG_RD_MEM_INFO_CMD_OPCODE                0xFC12
#define HCI_DBG_HW_REG_RD_CMD_OPCODE                  0xFC30
#define HCI_DBG_HW_REG_WR_CMD_OPCODE                  0xFC31
#define HCI_DBG_SET_BD_ADDR_CMD_OPCODE                0xFC32
#define HCI_DBG_SET_TYPE_PUB_CMD_OPCODE               0xFC33
#define HCI_DBG_SET_TYPE_RAND_CMD_OPCODE              0xFC34
#define HCI_DBG_SET_CRC_CMD_OPCODE                    0xFC35
#define HCI_DBG_LLCP_DISCARD_CMD_OPCODE               0xFC36
#define HCI_DBG_RESET_RX_CNT_CMD_OPCODE               0xFC37
#define HCI_DBG_RESET_TX_CNT_CMD_OPCODE               0xFC38
#define HCI_DBG_RF_REG_RD_CMD_OPCODE                  0xFC39
#define HCI_DBG_RF_REG_WR_CMD_OPCODE                  0xFC3A
#define HCI_DBG_SET_TX_PW_CMD_OPCODE                  0xFC3B
#define HCI_DBG_RF_SWITCH_CLK_CMD_OPCODE              0xFC3C
#define HCI_DBG_RF_WR_DATA_TX_CMD_OPCODE              0xFC3D
#define HCI_DBG_RF_RD_DATA_RX_CMD_OPCODE              0xFC3E
#define HCI_DBG_RF_CNTL_TX_CMD_OPCODE                 0xFC3F
#define HCI_DBG_RF_SYNC_P_CNTL_CMD_OPCODE             0xFC40
#define HCI_TESTER_SET_LE_PARAMS_CMD_OPCODE           0xFC40
#if (RWBLE_SW_VERSION_MAJOR >= 8)
#define HCI_DBG_WR_DLE_DFT_VALUE_CMD_OPCODE           0xFC41
#define HCI_DBG_WR_RL_SIZE_CMD_OPCODE                 0xFC42
#define HCI_DBG_WR_FILT_DUP_SIZE_CMD_OPCODE           0xFC43
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

#define HCI_DBG_SET_PWR_STATE_CMD_OPCODE              0xFC50

/**************************************************************************************
 **************                        HCI EVENTS                      ****************
 **************************************************************************************/

///Event Codes
#define HCI_INQ_CMP_EVT_CODE                       0x01
#define HCI_INQ_RES_EVT_CODE                       0x02
#define HCI_CON_CMP_EVT_CODE                       0x03
#define HCI_CON_REQ_EVT_CODE                       0x04
#define HCI_DISC_CMP_EVT_CODE                      0x05
#define HCI_AUTH_CMP_EVT_CODE                      0x06
#define HCI_REM_NAME_REQ_CMP_EVT_CODE              0x07
#define HCI_ENC_CHG_EVT_CODE                       0x08
#define HCI_CHG_CON_LK_CMP_EVT_CODE                0x09
#define HCI_MASTER_LK_CMP_EVT_CODE                 0x0A
#define HCI_RD_REM_SUPP_FEATS_CMP_EVT_CODE         0x0B
#define HCI_RD_REM_VER_INFO_CMP_EVT_CODE           0x0C
#define HCI_QOS_SETUP_CMP_EVT_CODE                 0x0D
#define HCI_CMD_CMP_EVT_CODE                       0x0E
#define HCI_CMD_STATUS_EVT_CODE                    0x0F
#define HCI_HW_ERR_EVT_CODE                        0x10
#define HCI_FLUSH_OCCURRED_EVT_CODE                0x11
#define HCI_ROLE_CHG_EVT_CODE                      0x12
#define HCI_NB_CMP_PKTS_EVT_CODE                   0x13
#define HCI_MODE_CHG_EVT_CODE                      0x14
#define HCI_RETURN_LINK_KEYS_EVT_CODE              0x15
#define HCI_PIN_CODE_REQ_EVT_CODE                  0x16
#define HCI_LK_REQ_EVT_CODE                        0x17
#define HCI_LK_NOTIF_EVT_CODE                      0x18
#define HCI_DATA_BUF_OVFLW_EVT_CODE                0x1A
#define HCI_MAX_SLOT_CHG_EVT_CODE                  0x1B
#define HCI_RD_CLK_OFF_CMP_EVT_CODE                0x1C
#define HCI_CON_PKT_TYPE_CHG_EVT_CODE              0x1D
#define HCI_QOS_VIOL_EVT_CODE                      0x1E
#define HCI_PAGE_SCAN_REPET_MODE_CHG_EVT_CODE      0x20
#define HCI_FLOW_SPEC_CMP_EVT_CODE                 0x21
#define HCI_INQ_RES_WITH_RSSI_EVT_CODE             0x22
#define HCI_RD_REM_EXT_FEATS_CMP_EVT_CODE          0x23
#define HCI_SYNC_CON_CMP_EVT_CODE                  0x2C
#define HCI_SYNC_CON_CHG_EVT_CODE                  0x2D
#define HCI_SNIFF_SUB_EVT_CODE                     0x2E
#define HCI_EXT_INQ_RES_EVT_CODE                   0x2F
#define HCI_ENC_KEY_REFRESH_CMP_EVT_CODE           0x30
#define HCI_IO_CAP_REQ_EVT_CODE                    0x31
#define HCI_IO_CAP_RSP_EVT_CODE                    0x32
#define HCI_USER_CFM_REQ_EVT_CODE                  0x33
#define HCI_USER_PASSKEY_REQ_EVT_CODE              0x34
#define HCI_REM_OOB_DATA_REQ_EVT_CODE              0x35
#define HCI_SP_CMP_EVT_CODE                        0x36
#define HCI_LINK_SUPV_TO_CHG_EVT_CODE              0x38
#define HCI_ENH_FLUSH_CMP_EVT_CODE                 0x39
#define HCI_USER_PASSKEY_NOTIF_EVT_CODE            0x3B
#define HCI_KEYPRESS_NOTIF_EVT_CODE                0x3C
#define HCI_REM_HOST_SUPP_FEATS_NOTIF_EVT_CODE     0x3D
#define HCI_LE_META_EVT_CODE                       0x3E
#define HCI_MAX_EVT_MSK_PAGE_1_CODE                0x40
#define HCI_AUTH_PAYL_TO_EXP_EVT_CODE              0x57
#define HCI_MAX_EVT_MSK_PAGE_2_CODE                0x58
#define HCI_DBG_EVT_CODE                           0xFF

/// LE Events Subcodes
#define HCI_LE_CON_CMP_EVT_SUBCODE                  0x01
#define HCI_LE_ADV_REPORT_EVT_SUBCODE               0x02
#define HCI_LE_CON_UPDATE_CMP_EVT_SUBCODE           0x03
#define HCI_LE_RD_REM_USED_FEATS_CMP_EVT_SUBCODE    0x04
#define HCI_LE_LTK_REQUEST_EVT_SUBCODE              0x05
#define HCI_LE_REM_CON_PARAM_REQ_EVT_SUBCODE        0x06
#if (RWBLE_SW_VERSION_MAJOR >= 8)
#define HCI_LE_DATA_LENGTH_CHANGE_EVT_SUBCODE       0x07
#define HCI_LE_READ_LOCAL_P256_PUBLIC_KEY_CMP_EVT_SUBCODE       0x08
#define HCI_LE_GENERATE_DHKEY_CMP_EVT_SUBCODE       0x09
#define HCI_LE_ENH_CON_CMP_EVT_SUBCODE              0x0A
#define HCI_LE_DIRECT_ADV_REPORT_EVT_SUBCODE        0x0B
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

/// Event mask page enum
enum hci_evt_mask_page
{
    /// page 0
    HCI_PAGE_0,
    /// page 1
    HCI_PAGE_1,
    /// page 2
    HCI_PAGE_2,
    /// Default
    HCI_PAGE_DFT
};
/// HCI ACL data RX  packet structure
struct hci_acl_data_rx
{
    /// connection handle
    uint16_t    conhdl;
    /// broadcast and packet boundary flag
    uint8_t     pb_bc_flag;
    /// length of the data
    uint16_t    length;

    #if (BLE_EMB_PRESENT)
    /// Handle of the descriptor containing RX Data
    uint8_t     rx_hdl;
    #else// (BLE_HOST_PRESENT)
    /// Pointer to the data buffer
    uint8_t* buffer;
    #endif // (BLE_EMB_PRESENT) / (BLE_HOST_PRESENT)
};

/// HCI ACL data TX packet structure
struct hci_acl_data_tx
{
    /// connection handle
    uint16_t    conhdl;
    /// broadcast and packet boundary flag
    uint8_t     pb_bc_flag;
    /// length of the data
    uint16_t    length;
    #if (BLE_EMB_PRESENT)
    /// Pointer to the first descriptor containing RX Data
    struct co_buf_tx_node *desc;
    #else // (BLE_HOST_PRESENT)
    /// Pointer to the data buffer
    uint8_t* buffer;
    #endif // (BLE_EMB_PRESENT) / (BLE_HOST_PRESENT)
};

#if (BT_EMB_PRESENT)
/// HCI ACL data packet structure
struct hci_bt_acl_data_tx
{
    /// Buffer element
    struct bt_em_acl_buf_elt* buf_elt;
};

/// HCI ACL data Rx packet structure
struct hci_bt_acl_data_rx
{
    /// EM buffer pointer
    uint16_t buf_ptr;
    /// Data length + Data Flags (PBF + BF)
    uint16_t data_len_flags;
};

#endif // (BT_EMB_PRESENT)


/*
 * HCI COMMANDS PARAMETERS (to classify)
 ****************************************************************************************
 */

/// HCI basic command structure with connection handle
struct hci_basic_conhdl_cmd
{
    /// connection handle
    uint16_t    conhdl;
};

/// HCI basic command structure with BD address
struct hci_basic_bd_addr_cmd
{
    ///BdAddr
    struct bd_addr  bd_addr;
};

/// HCI Accept connection request command structure
struct hci_accept_con_req_cmd
{
    ///BdAddr
    struct bd_addr  bd_addr;
    ///Page Scan Repetition Mode
    uint8_t         role;
};

/// HCI Accept synchronous connection request command structure
struct hci_accept_sync_con_req_cmd
{
    ///BdAddr
    struct bd_addr  bd_addr;
    ///Transmit bandwidth
    uint32_t    tx_bw;
    ///Receive bandwidth
    uint32_t    rx_bw;
    ///Max latency
    uint16_t    max_lat;
    ///Voice settings
    uint16_t    vx_set;
    ///Retransmission effort
    uint8_t      retx_eff;
    ///Packet type
    uint16_t     pkt_type  ;
};

/// HCI Enhanced Accept synchronous connection request command structure
struct hci_enh_accept_sync_con_cmd
{

    struct bd_addr    bd_addr;            // BD address
    uint32_t          tx_bw;              // Transmit Bandwidth (in B/sec)
    uint32_t          rx_bw;              // Receive Bandwidth (in B/sec)
    uint8_t           tx_cod_fmt[5];      // Transmit Coding Format
    uint8_t           rx_cod_fmt[5];      // Receive Coding Format
    uint16_t          tx_cod_fr_sz;       // Transmit Codec Frame Size (in B)
    uint16_t          rx_cod_fr_sz;       // Receive Codec Frame Size (in B)
    uint32_t          in_bw;              // Input Bandwidth (in B/sec)
    uint32_t          out_bw;             // Output Bandwidth (in B/sec)
    uint8_t           in_cod_fmt[5];      // Input Coding Format
    uint8_t           out_cod_fmt[5];     // Output Coding Format
    uint16_t          in_cod_data_sz;     // Input Coded Data Size (in bits)
    uint16_t          out_cod_data_sz;    // Output Coded Data Size (in bits)
    uint8_t           in_data_fmt;        // Input PCM Data Format
    uint8_t           out_data_fmt;       // Output PCM Data Format
    uint8_t           in_msb_pos;         // Input PCM Sample Payload MSB Position (in bits)
    uint8_t           out_msb_pos;        // Output PCM Sample Payload MSB Position (in bits)
    uint8_t           in_data_path;       // Input Data Path
    uint8_t           out_data_path;      // Output Data Path
    uint8_t           in_tr_unit_sz;      // Input Transport Unit Size (in bits)
    uint8_t           out_tr_unit_sz;     // Output Transport Unit Size (in bits)
    uint16_t          max_lat;            // Max Latency (in ms)
    uint16_t          packet_type;        // Packet Type
    uint8_t           retx_eff;           // Retransmission Effort


};

/// HCI reject connection request command structure
struct hci_reject_con_req_cmd
{
    ///BdAddr
    struct bd_addr  bd_addr;
    ///Reason
    uint8_t         reason;
};

/// HCI reject synchronous connection request command structure
struct hci_reject_sync_con_req_cmd
{
    ///BdAddr
    struct bd_addr  bd_addr;
    ///Reason
    uint8_t         reason;
};

/// HCI link key request reply command structure
struct hci_lk_req_reply_cmd
{
    ///BdAddr
    struct bd_addr  bd_addr;
    ///Key
    struct ltk         key;
};

/// HCI link key request reply command structure
struct hci_pin_code_req_reply_cmd
{
    ///BdAddr
    struct bd_addr  bd_addr;
    ///Pin code length
    uint8_t     pin_len;
    ///Key
    struct pin_code pin;
};

/// HCI switch role command structure
struct hci_switch_role_cmd
{
    ///BdAddr
    struct bd_addr  bd_addr;
    ///Read all flag
    uint8_t role;
};

/// HCI flow specification command parameters structure
struct hci_flow_spec_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Flags
    uint8_t flags;
    ///Flow direction
    uint8_t flow_dir;
    ///Service type
    uint8_t serv_type;
    ///Token rate
    uint32_t tk_rate;
    ///Token buffer size
    uint32_t tk_buf_sz;
    ///Peak bandwidth
    uint32_t pk_bw;
    ///Access latency
    uint32_t acc_lat;
};

/// HCI enhanced flush command parameters structure
struct hci_enh_flush_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Packet Type
    uint8_t pkt_type;
};

/// HCI command complete event structure for the read auto flush TO command
struct hci_rd_auto_flush_to_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Flush timeout
    uint16_t flush_to;
};

/// HCI write flush timeout command parameters structure
struct hci_wr_auto_flush_to_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Flush timeout
    uint16_t flush_to;
};

/// HCI change connection packet type command parameters structure
struct hci_chg_con_pkt_type_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Packet type
    uint16_t pkt_type;
};

/// HCI read link policy settings command parameters structure
struct hci_rd_link_pol_stg_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Link policy
    uint16_t lnk_policy;
};

/// HCI read link policy settings command parameters structure
struct hci_wr_link_pol_stg_cmd
{
    ///Connection handle
    uint16_t    conhdl;
    ///Link policy
    uint16_t lnk_policy;
};

/// HCI sniff mode request command parameters structure
struct hci_sniff_mode_cmd
{
    ///Connection handle
    uint16_t    conhdl;
    ///Sniff max interval
    uint16_t    max_int;
    ///Sniff min interval
    uint16_t    min_int;
    ///Sniff attempt
    uint16_t    attempt;
    ///Sniff timeout
    uint16_t    timeout;
};

/// HCI sniff subrating mode request command parameters structure
struct hci_sniff_sub_cmd
{
    ///Connection handle
    uint16_t    conhdl;
    ///Sniff max latency
    uint16_t    max_lat;
    ///Minimun remote TO
    uint16_t    min_rem_to;
    ///Minimun local TO
    uint16_t    min_loc_to;
};

/// HCI role discovery complete event parameters structure
struct hci_role_discovery_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Role
    uint8_t role;

};

/// HCI read failed contact counter command parameters structure
struct hci_rd_fail_contact_cnt_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Fail contact counter
    uint16_t fail_cnt;
};

/// HCI read link quality complete event parameters structure
struct hci_rd_link_qual_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Quality type
    uint8_t quality;
};

/// HCI read afh channel map complete event parameters structure
struct hci_rd_afh_ch_map_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///afh status
    uint8_t afh_stat;
    ///afh channel map
    struct chnl_map afh_map;
};

/// HCI read lmp handle complete event parameters structure
struct hci_rd_lmp_hdl_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///lmp handle
    uint8_t lmp_hdl;
    ///rsvd
    uint32_t rsvd;
};

/// HCI read remote extended features command parameters structure
struct hci_rd_rem_ext_feats_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///page number
    uint8_t pg_nb;
};

/// HCI read encryption key size complete event parameters structure
struct hci_rd_enc_key_size_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Key size
    uint8_t key_sz;
};

/// HCI read enhanced transmit power command parameters structure
struct hci_rd_enh_tx_pwr_lvl_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Type
    uint8_t type;
};

/// HCI read enhanced transmit power complete event parameters structure
struct hci_rd_enh_tx_pwr_lvl_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Transmit power GFSK
    uint8_t pw_gfsk;
    ///Transmit power DQPSK
    uint8_t pw_dqpsk;
    ///Transmit power 8DPSK
    uint8_t pw_8dpsk;
};


/*
 * HCI LINK CONTROL COMMANDS PARAMETERS
 ****************************************************************************************
 */

/// Format of the message of the Group: LINK_CONTROL_COMMANDS
/// HCI Inquiry command parameters structure
struct hci_inq_cmd
{
    ///Lap
    struct lap  lap;
    ///Inquiry Length
    uint8_t     inq_len;
    ///Number of response
    uint8_t     nb_rsp;
};
struct hci_per_inq_mode_cmd
{
    ///Maximum period length
    uint16_t max_per_len;
    ///Minimum period length
    uint16_t min_per_len;
    ///lap
    struct lap lap;
    ///Inquiry length
    uint8_t inq_len;
    ///Number of response
    uint8_t nb_rsp;
};
struct hci_create_con_cmd
{
    ///BdAddr
    struct bd_addr  bd_addr;
    ///Packet Type
    uint16_t        pkt_type;
    ///Page Scan Repetition Mode
    uint8_t         page_scan_rep_mode;
    ///Reserved
    uint8_t         rsvd;
    ///Clock Offset
    uint16_t        clk_off;
    ///Allow Switch
    uint8_t         switch_en;
};

/// HCI disconnect command structure
struct hci_disconnect_cmd
{
    /// connection handle
    uint16_t    conhdl;
    /// reason
    uint8_t     reason;
};

/// HCI master link key command structure
struct hci_master_lk_cmd
{
    ///Key flag
    uint8_t key_flag;
};
/// HCI authentication request command parameters structure
struct hci_set_con_enc_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Encryption mode
    uint8_t enc_en;
};

struct hci_rem_name_req_cmd
{
        ///BdAddr
        struct bd_addr  bd_addr;
        ///Page Scan Repetition Mode
        uint8_t         page_scan_rep_mode;
        ///Reserved
        uint8_t         rsvd;
        ///Clock Offset
        uint16_t        clk_off;
};

/// HCI remote name request complete event structure
struct hci_rem_name_req_cmp_evt
{
    /// Status
    uint8_t status;
    /// BD Addr
     struct bd_addr bd_addr;
     /// Name
     struct device_name name;
};

/// HCI setup synchronous connection command structure
struct hci_setup_sync_con_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Transmit bandwidth
    uint32_t tx_bw;
    ///Receive bandwidth
    uint32_t rx_bw;
    ///Max latency
    uint16_t max_lat;
    ///Voice setting
    uint16_t vx_set;
    ///Retransmission effort
    uint8_t retx_eff;
    ///Packet type
    uint16_t pkt_type;
};

/// HCI setup synchronous connection command structure
struct hci_enh_setup_sync_con_cmd
{
    uint16_t      conhdl;               // Connection Handle
    uint32_t      tx_bw;                // Transmit Bandwidth (in B/sec)
    uint32_t      rx_bw;                // Receive Bandwidth (in B/sec)
    uint8_t       tx_cod_fmt[5];        // Transmit Coding Format
    uint8_t       rx_cod_fmt[5];        // Receive Coding Format
    uint16_t      tx_cod_fr_sz;         // Transmit Codec Frame Size (in B)
    uint16_t      rx_cod_fr_sz;         // Receive Codec Frame Size (in B)
    uint32_t      in_bw;                // Input Bandwidth (in B/sec)
    uint32_t      out_bw;               // Output Bandwidth (in B/sec)
    uint8_t       in_cod_fmt[5];        // Input Coding Format
    uint8_t       out_cod_fmt[5];       // Output Coding Format
    uint16_t      in_cod_data_sz;       // Input Coded Data Size (in bits)
    uint16_t      out_cod_data_sz;      // Output Coded Data Size (in bits)
    uint8_t       in_data_fmt;          // Input PCM Data Format
    uint8_t       out_data_fmt;         // Output PCM Data Format
    uint8_t       in_msb_pos;           // Input PCM Sample Payload MSB Position (in bits)
    uint8_t       out_msb_pos;          // Output PCM Sample Payload MSB Position (in bits)
    uint8_t       in_data_path;         // Input Data Path
    uint8_t       out_data_path;        // Output Data Path
    uint8_t       in_tr_unit_sz;        // Input Transport Unit Size (in bits)
    uint8_t       out_tr_unit_sz;       // Output Transport Unit Size (in bits)
    uint16_t      max_lat;              // Max Latency (in ms)
    uint16_t      packet_type;          // Packet Type
    uint8_t       retx_eff;             // Retransmission Effort
};

/// HCI io capability request reply command structure
struct hci_io_cap_req_reply_cmd
{
    ///BdAddr
    struct bd_addr  bd_addr;
    ///IO capability
    uint8_t io_capa;
    ///OOB data present
    uint8_t oob_data_pres;
    ///Authentication requirements
    uint8_t auth_req;

};

/// HCI io capability request negative reply command structure
struct hci_io_cap_req_neg_reply_cmd
{
    ///BdAddr
    struct bd_addr  bd_addr;
    ///Reason
    uint8_t reason;
};

/// HCI user pass key request reply command structure
struct hci_user_passkey_req_reply_cmd
{
    ///BdAddr
    struct bd_addr  bd_addr;
    ///Numeric value
    uint32_t num_val;
};

/// HCI remote oob data request reply command structure
struct hci_rem_oob_data_req_reply_cmd
{
    ///BdAddr
    struct bd_addr  bd_addr;
    ///hash part
    struct hash oob_c;
    ///random part
    struct randomizer oob_r;
};

/// HCI send key press notification command structure
struct hci_send_keypress_notif_cmd
{
    ///BdAddr
    struct bd_addr  bd_addr;
    ///Notification type
    uint8_t notif_type;
};


/*
 * HCI LINK POLICY COMMANDS PARAMETERS
 ****************************************************************************************
 */

/// HCI setup quality of service command structure
struct hci_qos_setup_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Flags
    uint8_t flags;
    ///Service type
    uint8_t serv_type;
    ///Token rate
    uint32_t tok_rate;
    ///Peak bandwidth
    uint32_t pk_bw;
    ///Latency
    uint32_t lat;
    ///Delay variation
    uint32_t del_var;
};

/// HCI command complete event structure for read default link policy command structure
struct hci_rd_dft_link_pol_stg_cmd_cmp_evt
{
    ///Status of the command reception
    uint8_t     status;
    ///Link policy
    uint16_t    link_pol_stg;
};

struct hci_wr_dft_link_pol_stg_cmd
{
        ///Link policy
        uint16_t    link_pol_stg;
};

/*
 * HCI CONTROL & BASEBAND COMMANDS PARAMETERS
 ****************************************************************************************
 */

/// HCI set event mask command structure
struct hci_set_evt_mask_cmd
{
    ///Event Mask
    struct evt_mask    event_mask;
};

/// HCI set event filter command structure
struct hci_set_evt_filter_cmd
{
    /// Filter type
    uint8_t filter_type;

    /// Filters
    union hci_filter
    {
        uint8_t clear_all_filter_reserved;

        /// Inquiry Result Filter
        struct inq_res_filter
        {
            /// Filter Condition type
            uint8_t cond_type;

            /// Filter conditions
            union hci_inq_filter_cond
            {
                /// Reserved value (Inquiry Result Filter - condition type 0x00 has no condition)
                uint8_t cond_0_reserved;

                /// Inquiry Result Filter Condition - condition type 0x01
                struct inq_res_filter_cond_1
                {
                    /// Class_of_Device
                    struct devclass class_of_dev;
                    /// Class_of_Device_Mask
                    struct devclass class_of_dev_msk;
                } cond_1;

                /// Inquiry Result Filter Condition - condition type 0x02
                struct inq_res_filter_cond_2
                {
                    /// BD Address
                    struct bd_addr bd_addr;
                } cond_2;
            } cond;
        } inq_res;

        /// Connection Setup Filter
        struct con_set_filter
        {
            /// Filter Condition type
            uint8_t cond_type;

            /// Filter conditions
            union hci_con_filter_cond
            {
                /// Connection Setup Filter Condition - condition type 0x00
                struct con_set_filter_cond_0
                {
                    /// Auto_Accept_Flag
                    uint8_t auto_accept;
                } cond_0;

                /// Connection Setup Filter Condition - condition type 0x01
                struct con_set_filter_cond_1
                {
                    /// Class_of_Device
                    struct devclass class_of_dev;
                    /// Class_of_Device_Mask
                    struct devclass class_of_dev_msk;
                    /// Auto_Accept_Flag
                    uint8_t auto_accept;
                } cond_1;

                /// Connection Setup Filter Condition - condition type 0x02
                struct con_set_filter_cond_2
                {
                    /// BD Address
                    struct bd_addr bd_addr;
                    /// Auto_Accept_Flag
                    uint8_t auto_accept;
                } cond_2;
            } cond;

        } con_set;

    } filter;
};

/// HCI command completed event structure for the flush command
struct hci_flush_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
};

/// HCI command complete event structure for the Read pin type command
struct hci_rd_pin_type_cmd_cmp_evt
{
    ///Status of the command
    uint8_t     status;
    ///PIN type
    uint8_t   pin_type;
};

struct hci_wr_pin_type_cmd
{
    ///PIN type
    uint8_t pin_type;
};

struct hci_rd_stored_lk_cmd
{
    ///BdAddr
    struct bd_addr  bd_addr;
    ///Read all flag
    uint8_t rd_all_flag;
};

/// HCI command complete event structure for read stored link key command
struct hci_rd_stored_lk_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    ///Maximum number of key
    uint16_t num_key_max;
    ///Read number of key
    uint16_t num_key_rd;
};

#if BT_EMB_PRESENT
struct hci_wr_stored_lk_cmd
{
        /// Number of key to write
        uint8_t num_key_wr;

        /// BD Address + Key table
        struct bd_addr_plus_key link_keys[HCI_MAX_CMD_PARAM_SIZE / sizeof(struct bd_addr_plus_key)];
};
#endif //BT_EMB_PRESENT

/// HCI command complete event structure for write stored link key command
struct hci_wr_stored_lk_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    ///number of key written
    uint8_t    num_key_wr;
};

struct hci_del_stored_lk_cmd
{
        ///BdAddr
        struct bd_addr  bd_addr;
        ///Delete all flag
        uint8_t del_all_flag;
};

/// HCI command complete event structure for delete stored link key command
struct hci_del_stored_lk_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    ///Read number of key
    uint16_t num_key_del;
};

struct hci_wr_local_name_cmd
{
        ///Name
        struct device_name  name;
};

/// HCI command complete event structure for the read local name command
struct hci_rd_local_name_cmd_cmp_evt
{
    ///Status of the command
    uint8_t     status;
    ///Name
    uint8_t  name[BD_NAME_SIZE];
};

/// HCI command complete event structure for the Read connection accept to command
struct hci_rd_con_accept_to_cmd_cmp_evt
{
    ///Status of the command
    uint8_t     status;
    /// Connection accept timeout (in slots)
    uint16_t    con_acc_to;
};

struct hci_wr_con_accept_to_cmd
{
    /// Connection accept timeout (in slots)
    uint16_t    con_acc_to;
};

/// HCI command complete event structure for the Read page to command
struct hci_rd_page_to_cmd_cmp_evt
{
    ///Status of the command
    uint8_t     status;
    /// Page timeout (in slots)
    uint16_t    page_to;
};

struct hci_wr_page_to_cmd
{
    /// Page timeout (in slots)
    uint16_t    page_to;
};

/// HCI command complete event structure for the Read scan enable command
struct hci_rd_scan_en_cmd_cmp_evt
{
    ///Status of the command
    uint8_t     status;
    ///Status of the scan enable
    uint8_t     scan_en;
};

struct hci_wr_scan_en_cmd
{
        ///Status of the scan enable
        uint8_t scan_en;
};

/// HCI command complete event structure for the Read scan activity command
struct hci_rd_page_scan_act_cmd_cmp_evt
{
    ///Status of the command
    uint8_t     status;
    /// Page scan interval (in slots)
    uint16_t page_scan_intv;
    /// Page scan window (in slots)
    uint16_t page_scan_win;
};

struct hci_wr_page_scan_act_cmd
{
    /// Page scan interval (in slots)
    uint16_t page_scan_intv;
    /// Page scan window (in slots)
    uint16_t page_scan_win;
};

/// HCI command complete event structure for the Read inquiry scan activity command
struct hci_rd_inq_scan_act_cmd_cmp_evt
{
    /// Status of the command
    uint8_t  status;
    /// Inquiry scan interval (in slots)
    uint16_t inq_scan_intv;
    /// Inquiry scan window (in slots)
    uint16_t inq_scan_win;
};

struct hci_wr_inq_scan_act_cmd
{
    /// Inquiry scan interval (in slots)
    uint16_t inq_scan_intv;
    /// Inquiry scan window (in slots)
    uint16_t inq_scan_win;
};

/// HCI command complete event structure for the Read authentication command
struct hci_rd_auth_en_cmd_cmp_evt
{
    ///Status of the command
    uint8_t     status;
    ///Value of the authentication
    uint8_t    auth_en;
};

struct hci_wr_auth_en_cmd
{
        ///Value of the authentication
        uint8_t auth_en;
};

/// HCI command complete event structure for the read class of device command
struct hci_rd_class_of_dev_cmd_cmp_evt
{
    ///Status of the command
    uint8_t     status;
    ///Class of device
    struct devclass class_of_dev;
};

struct hci_wr_class_of_dev_cmd
{
        ///Class of device
        struct devclass class_of_dev;
};

/// HCI read voice settings complete event
struct hci_rd_voice_stg_cmd_cmp_evt
{
    ///Status of the command reception
    uint8_t     status;
    /// Voice setting
    uint16_t voice_stg;
};

struct hci_wr_voice_stg_cmd
{
    /// voice setting
    uint16_t voice_stg;
};

/// HCI command complete event structure for read number of broadcast retrans command
struct hci_rd_nb_bdcst_retx_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    ///Read number of broadcast retransmission
    uint8_t num_bcst_ret;
};

struct hci_wr_nb_bdcst_retx_cmd
{
        ///Read number of broadcast retransmission
        uint8_t num_bcst_ret;
};

/// HCI command complete event structure for the Read Synchronous Flow Control command
struct hci_rd_sync_flow_cntl_en_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    ///Synchronous flow control
    uint8_t     sync_flow;
};

struct hci_wr_sync_flow_cntl_en_cmd
{
        ///control mode
        uint8_t   cntl_en;
};

///HCI set controller to host flow control command
struct hci_set_ctrl_to_host_flow_ctrl_cmd
{
    ///Flow control enable for controller
    uint8_t flow_cntl;
};

///HCI host buffer size command
struct hci_host_buf_size_cmd
{
    ///Host ACL packet length
    uint16_t    acl_pkt_len;
    ///Host synchronous packet length
    uint8_t     sync_pkt_len;
    ///Host Total number of ACL packets allowed
    uint16_t    nb_acl_pkts;
    ///Host total number of synchronous packets allowed
    uint16_t    nb_sync_pkts;
};

#if BT_EMB_PRESENT
///HCI host number of completed packets command
struct hci_host_nb_cmp_pkts_cmd
{
        ///Number of handles for which the completed packets number is given
        uint8_t     nb_of_hdl;
        ///Array of connection handles
        uint16_t    con_hdl[MAX_NB_ACTIVE_ACL];
        ///Array of number of completed packets values for connection handles.
        uint16_t    nb_comp_pkt[MAX_NB_ACTIVE_ACL];
};
#elif BLE_EMB_PRESENT || BLE_HOST_PRESENT
///HCI host number of completed packets command
struct hci_host_nb_cmp_pkts_cmd
{
    ///Number of handles for which the completed packets number is given
    uint8_t     nb_of_hdl;
    ///Array of connection handles
    uint16_t    con_hdl[BLE_CONNECTION_MAX];
    ///Array of number of completed packets values for connection handles.
    uint16_t    nb_comp_pkt[BLE_CONNECTION_MAX];
};
#endif //BLE_EMB_PRESENT || BLE_HOST_PRESENT

/// HCI read link supervision timeout command parameters structure
struct hci_rd_link_supv_to_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Link supervision timeout
    uint16_t lsto_val;
};

/// HCI write link supervision timeout command parameters structure
struct hci_wr_link_supv_to_cmd
{
    ///Connection handle
    uint16_t conhdl;
    ///Link supervision timeout
    uint16_t lsto_val;
};

/// HCI command complete event structure for the nb of supported IAC command
struct hci_rd_nb_supp_iac_cmd_cmp_evt
{
    ///Status of the command
    uint8_t     status;
    ///nb_of iac
    uint8_t  nb_iac;
};

/// HCI command complete event structure for read current IAC LAP command
struct hci_rd_curr_iac_lap_cmd_cmp_evt
{
    ///Status of the command
    uint8_t     status;
    ///nb of current iac
    uint8_t  nb_curr_iac;
    ///lap
    struct lap iac_lap;
};

/// HCI write current IAC LAP command structure
struct hci_wr_curr_iac_lap_cmd
{
    /// Number of current iac laps
    uint8_t  nb_curr_iac;
    ///lap
    struct lap iac_lap[(HCI_MAX_CMD_PARAM_SIZE / BD_ADDR_LAP_LEN) - 1];
};

struct hci_set_afh_host_ch_class_cmd
{
        ///AFH channel map
        struct chnl_map   afh_ch;
};

/// HCI command complete event structure for write inquiry scan type command structure
struct hci_rd_inq_scan_type_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    /// Inquiry scan type
    uint8_t     inq_scan_type;
};

struct hci_wr_inq_scan_type_cmd
{
    /// Inquiry scan type
    uint8_t     inq_scan_type;
};

/// HCI command complete event structure for read inquiry mode command structure
struct hci_rd_inq_mode_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    /// Inquiry mode
    uint8_t     inq_mode;
};

struct hci_wr_inq_mode_cmd
{
    /// Inquiry mode
    uint8_t     inq_mode;
};

/// HCI command complete event structure for write page scan type command structure
struct hci_rd_page_scan_type_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    /// Page scan type
    uint8_t     page_scan_type;
};

struct hci_wr_page_scan_type_cmd
{
    /// Page scan type
    uint8_t     page_scan_type;
};

/// HCI command complete event structure for read assessment mode command structure
struct hci_rd_afh_ch_assess_mode_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    ///AFH channel assessment mode
    uint8_t     afh_ch_ass_mode;
};

struct hci_wr_afh_ch_assess_mode_cmd
{
        ///AFH channel assessment mode
        uint8_t     afh_ch_ass_mode;
};

/// HCI command complete event structure for remote name request cancel command
struct hci_rd_ext_inq_rsp_cmd_cmp_evt
{
    ///status
    uint8_t     status;
    ///FEC required
    uint8_t     fec_req;
    ///Extended inquiry response
    struct eir  eir;
};

struct hci_wr_ext_inq_rsp_cmd
{
        ///FEC required
        uint8_t     fec_req;
        ///Extended inquiry response
        struct eir  eir;
};

/// HCI command complete event structure for remote name request cancel command
struct hci_rd_sp_mode_cmd_cmp_evt
{
    ///status
    uint8_t     status;
    ///Simple pairing mode
    uint8_t     sp_mode;
};

struct hci_wr_sp_mode_cmd
{
        ///Simple pairing mode
        uint8_t     sp_mode;
};


/// HCI command complete event structure for read oob data command
struct hci_rd_loc_oob_data_cmd_cmp_evt
{
    ///status
    uint8_t     status;
    ///hash part
    struct hash oob_c;
    ///random part
    struct randomizer oob_r;
};

/// HCI command complete event structure for read inquiry response transmit power command
struct hci_rd_inq_rsp_tx_pwr_lvl_cmd_cmp_evt
{
    ///status
    uint8_t status;
    ///TX power
    uint8_t tx_pwr;
};

struct hci_wr_inq_tx_pwr_lvl_cmd
{
    ///TX power
    int8_t tx_pwr;
};

/// HCI command complete event structure for read erroneous data reporting command
struct hci_rd_dft_err_data_rep_cmd_cmp_evt
{
    ///status
    uint8_t     status;
    ///Erroneous data reporting
    uint8_t     data;
};

struct hci_wr_dft_err_data_rep_cmd
{
    ///Erroneous data reporting
    uint8_t data;
};

/// HCI read LE Host Supported complete event
struct  hci_rd_le_host_supp_cmd_cmp_evt
{
    ///Status
    uint8_t status;
    ///LE_Supported_Host
    uint8_t le_supported_host;
    ///Simultaneous_LE_Host
    uint8_t simultaneous_le_host;
};

/// HCI write LE Host Supported command
struct  hci_wr_le_host_supp_cmd
{
    ///LE_Supported_Host
    uint8_t le_supported_host;
    ///Simultaneous_LE_Host
    uint8_t simultaneous_le_host;
};

/// HCI read authenticated payload timeout command
struct hci_rd_auth_payl_to_cmd
{
    ///Connection handle
    uint16_t     conhdl;
};

/// HCI command complete event structure for the Read Authenticated Payload Timeout Command
struct hci_rd_auth_payl_to_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    ///Connection handle
    uint16_t     conhdl;
    ///Authenticated payload timeout
    uint16_t     auth_payl_to;
};

/// HCI read Extended Page Timeout CC event
struct hci_rd_ext_page_to_cmd_cmp_evt
{
    /// Status
    uint8_t  status;
    /**
     * Extended Page Timeout measured in Number of Baseband slots. Interval Length = N * 0.625 msec (1 Baseband slot)
     * Range for N: 0x0000 (default) � 0xFFFF
     * Time Range: 0 - 40.9 Seconds
     */
    uint16_t ext_page_to;
};

/// HCI write Extended Page Timeout
struct hci_wr_ext_page_to_cmd
{
    /**
     * Extended Page Timeout measured in Number of Baseband slots. Interval Length = N * 0.625 msec (1 Baseband slot)
     * Range for N: 0x0000 (default) � 0xFFFF
     * Time Range: 0 - 40.9 Seconds
     */
    uint16_t ext_page_to;
};

/// HCI read Extended Inquiry Length CC event
struct hci_rd_ext_inq_len_cmd_cmp_evt
{
    /// Status
    uint8_t  status;
    /// Extended Inquiry Length
    uint16_t ext_inq_len;
};

/// HCI write Extended Inquiry Length
struct hci_wr_ext_inq_len_cmd
{
    /// Extended Inquiry Length
    uint16_t ext_inq_len;
};

/*
 * HCI INFORMATIONAL PARAMETERS COMMANDS PARAMETERS
 ****************************************************************************************
 */

///HCI command complete event structure for read local version information
struct hci_rd_local_ver_info_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    ///HCI version number
    uint8_t     hci_ver;
    ///HCI revision number
    uint16_t    hci_rev;
    ///LMP version
    uint8_t     lmp_ver;
    ///manufacturer name
    uint16_t    manuf_name;
    ///LMP Subversion
    uint16_t    lmp_subver;
};

///HCI command complete event structure for read local supported commands
struct hci_rd_local_supp_cmds_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t             status;
    ///Supported Commands structure
    struct supp_cmds    local_cmds;
};

/// HCI command complete event structure for read local supported features command
struct hci_rd_local_supp_feats_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t         status;
    ///Local  supported features
    struct features feats;
};

struct hci_rd_local_ext_feats_cmd
{
        ///Page number
        uint8_t page_nb;
};

/// HCI command complete event structure for read local extended features command
struct hci_rd_local_ext_feats_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    ///Page number
    uint8_t     page_nb;
    ///Maximum page number
    uint8_t     page_nb_max;
    ///Extended LMP features
    struct features ext_feats;
};

///HCI command complete event structure for the Read Buffer Size Command
struct hci_rd_buff_size_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    ///ACL data packet length controller can receive from host
    uint16_t     hc_data_pk_len;
    ///Synchronous data packet length controller can receive from host
    uint8_t      hc_sync_pk_len;
    ///Total number of ACL data packets controller can receive from host
    uint16_t     hc_tot_nb_data_pkts;
    ///Total number of synchronous data packets controller can receive from host
    uint16_t     hc_tot_nb_sync_pkts;
};

///HCI command complete event structure for read bd address
struct hci_rd_bd_addr_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t             status;
    ///BD address
    struct bd_addr      local_addr;
};

/// HCI command complete event structure for read local supported codecs
struct hci_rd_local_supp_codecs_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t               status;
//    ///Supported Codecs structure
//    struct supp_codecs    local_codecs;
};

/*
 * HCI STATUS PARAMETERS COMMANDS PARAMETERS
 ****************************************************************************************
 */

/// HCI command complete event structure for read rssi
struct hci_rd_rssi_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///RSSI value
    uint8_t rssi;
};

struct hci_rd_clk_cmd
{
        ///Connection handle
        uint16_t conhdl;
        ///Which clock
        uint8_t clk_type;
};

/// HCI read clock command structure
struct hci_rd_clk_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    ///Connection handle
    uint16_t conhdl;
    ///clock
    uint32_t clk;
    ///Accuracy
    uint16_t clk_acc;
};


/*
 * HCI TESTING COMMANDS PARAMETERS
 ****************************************************************************************
 */

/// HCI command complete event structure for read loop back mode command
struct hci_rd_loop_back_mode_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    ///Local  supported features
    uint8_t     lb_mode;
};

struct hci_wr_loop_back_mode_cmd
{
        ///Local  supported features
        uint8_t     lb_mode;
};
struct hci_wr_sp_dbg_mode_cmd
{
        ///Simple pairing mode
        uint8_t     sp_mode;
};


/*
 * HCI LE CONTROLLER COMMANDS PARAMETERS
 ****************************************************************************************
 */

///HCI LE Set Event Mask Command parameters structure
struct hci_le_set_evt_mask_cmd
{
    ///LE Event Mask
    struct evt_mask le_mask;
};

///HCI LE Set Random Address Command parameters structure
struct hci_le_set_rand_addr_cmd
{
    ///Random address to set
    struct bd_addr rand_addr;
};

///HCI LE Set Advertising Parameters Command parameters structure
struct hci_le_set_adv_param_cmd
{
    ///Minimum interval for advertising
    uint16_t       adv_intv_min;
    ///Maximum interval for advertising
    uint16_t       adv_intv_max;
    ///Advertising type
    uint8_t        adv_type;
    ///Own address type: public=0x00 /random = 0x01
    uint8_t        own_addr_type;
    ///Direct address type: public=0x00 /random = 0x01
    uint8_t        direct_addr_type;
    ///Direct Bluetooth device address
    struct bd_addr direct_addr;
    ///Advertising channel map
    uint8_t        adv_chnl_map;
    ///Advertising filter policy
    uint8_t        adv_filt_policy;
};

///HCI LE Set Advertising Data Command parameters structure
struct hci_le_set_adv_data_cmd
{
    ///Advertising data length
    uint8_t         adv_data_len;
    ///Advertising data - maximum 31 bytes
    struct adv_data data;
};

///HCI LE Set Scan Response Data Command parameters structure
struct hci_le_set_scan_rsp_data_cmd
{
    ///Scan response data length
    uint8_t              scan_rsp_data_len;
    ///Scan response data - maximum 31 bytes
    struct scan_rsp_data data;
};

///HCI LE Set Advertise Enable Command parameters structure
struct hci_le_set_adv_en_cmd
{
    ///Advertising enable - 0=disabled, 1=enabled
    uint8_t        adv_en;
};

///HCI LE Set Scan Parameters Command parameters structure
struct hci_le_set_scan_param_cmd
{
    ///Scan type - 0=passive / 1=active
    uint8_t        scan_type;
    ///Scan interval
    uint16_t       scan_intv;
    ///Scan window size
    uint16_t       scan_window;
    ///Own address type - 0=public, 1=random
    uint8_t        own_addr_type;
    ///Scan filter policy
    uint8_t        scan_filt_policy;
};

///HCI LE Set Scan Enable Command parameters structure
struct hci_le_set_scan_en_cmd
{
    ///Scan enable - 0=disabled, 1=enabled
    uint8_t        scan_en;
    ///Enable for duplicates filtering - 0 =disabled/ 1=enabled
    uint8_t        filter_duplic_en;
};

///HCI LE Create Connection Command parameters structure
struct hci_le_create_con_cmd
{
    ///Scan interval
    uint16_t       scan_intv;
    ///Scan window size
    uint16_t       scan_window;
    ///Initiator filter policy
    uint8_t        init_filt_policy;
    ///Peer address type - 0=public/1=random
    uint8_t        peer_addr_type;
    ///Peer BD address
    struct bd_addr peer_addr;
    ///Own address type - 0=public/1=random
    uint8_t        own_addr_type;
    ///Minimum of connection interval
    uint16_t       con_intv_min;
    ///Maximum of connection interval
    uint16_t       con_intv_max;
    ///Connection latency
    uint16_t       con_latency;
    ///Link supervision timeout
    uint16_t       superv_to;
    ///Minimum CE length
    uint16_t       ce_len_min;
    ///Maximum CE length
    uint16_t       ce_len_max;
};

///HCI LE Add Device to White List Command parameters structure
struct hci_le_add_dev_to_wlst_cmd
{
    ///Type of address of the device to be added to the White List - 0=public/1=random
    uint8_t        dev_addr_type;
    ///Address of device to be added to White List
    struct bd_addr dev_addr;
};

///HCI LE Remove Device from White List Command parameters structure
struct hci_le_rmv_dev_from_wlst_cmd
{
    ///Type of address of the device to be removed from the White List - 0=public/1=random
    uint8_t        dev_addr_type;
    ///Address of device to be removed from White List
    struct bd_addr dev_addr;
};


///HCI LE Set Host Channel Classification Command parameters structure
struct hci_le_set_host_ch_class_cmd
{
    ///Channel map
    struct le_chnl_map chmap;
};


///HCI LE Receiver Test Command parameters structure
struct hci_le_rx_test_cmd
{
    ///RX frequency for Rx test
    uint8_t        rx_freq;
};

///HCI LE Transmitter Test Command parameters structure
struct hci_le_tx_test_cmd
{
    ///TX frequency for Tx test
    uint8_t        tx_freq;
    ///TX test data length
    uint8_t        test_data_len;
    ///TX test payload type - see enum
    uint8_t        pk_payload_type;
};

///HCI LE Encrypt Command parameters structure
struct hci_le_enc_cmd
{
    ///Long term key structure
    struct ltk     key;
    ///Pointer to buffer with plain data to encrypt - 16 bytes
    uint8_t        plain_data[16];
};

/// HCI LE Connection Update Command parameters structure
struct hci_le_con_update_cmd
{
    ///Connection Handle
    uint16_t       conhdl;
    ///Minimum of connection interval
    uint16_t       con_intv_min;
    ///Maximum of connection interval
    uint16_t       con_intv_max;
    ///Connection latency
    uint16_t       con_latency;
    ///Link supervision timeout
    uint16_t       superv_to;
    ///Minimum of CE length
    uint16_t       ce_len_min;
    ///Maximum of CE length
    uint16_t       ce_len_max;
};

/// HCI LE Start Encryption Command parameters structure
struct hci_le_start_enc_cmd
{
    ///Connection handle
    uint16_t        conhdl;
    ///Random number - 8B
    struct rand_nb  nb;
    ///Encryption Diversifier
    uint16_t       enc_div;
    ///Long term key
    struct ltk     ltk;
};

/// HCI long term key request reply command parameters structure
struct hci_le_ltk_req_reply_cmd
{
    ///Connection handle
    uint16_t        conhdl;
    ///Long term key
    struct ltk      ltk;
};

/// HCI LE remote connection parameter request reply command parameters structure
struct hci_le_rem_con_param_req_reply_cmd
{
    ///Connection handle
    uint16_t        conhdl;
    ///Interval_Min
    uint16_t        interval_min;
    ///Interval_Max
    uint16_t        interval_max;
    ///Latency
    uint16_t        latency;
    ///Timeout
    uint16_t        timeout;
    ///Minimum_CE_Length
    uint16_t        min_ce_len;
    ///Maximum_CE_Length
    uint16_t        max_ce_len;
};

/// HCI LE remote connection parameter request negative reply command parameters structure
struct hci_le_rem_con_param_req_neg_reply_cmd
{
    ///Connection handle
    uint16_t        conhdl;
    ///Reason
    uint8_t         reason;
};

#if (RWBLE_SW_VERSION_MAJOR >= 8)
/// HCI LE set data length
struct hci_le_set_data_length_cmd
{
    ///Connection handle
    uint16_t        conhdl;
    ///tx octets
    uint16_t        tx_octets;	
    ///tx time
    uint16_t        tx_time;	
};

/// HCI LE write suggested default data length
struct hci_le_write_suggested_default_data_length_cmd
{
    ///tx octets
    uint16_t        suggested_tx_octets;	
    ///tx time
    uint16_t        suggested_tx_time;	
};

/// HCI LE add address to resolving list
struct hci_le_add_dev_to_rslv_list_cmd
{
    ///identity address type
    uint8_t        identity_address_type;
    ///identity address
    uint8_t        identity_address[BD_ADDR_LEN];
    ///peer irk
    uint8_t        peer_irk[KEY_LEN];
    ///local irk
    uint8_t        local_irk[KEY_LEN];
};

/// HCI LE address from resolving list
struct hci_le_dev_from_rslv_list_cmd
{
    ///identity address type
    uint8_t        identity_address_type;
    ///identity address
    uint8_t        identity_address[BD_ADDR_LEN];
};

/// HCI LE set address resolve enable
struct hci_le_set_addr_resol_en_cmd
{
    ///enable/disable
    uint8_t        address_resolution_enable;	
};

/// HCI LE set resolvable private address timeout
struct hci_le_set_rslv_priv_addr_to_cmd
{
    ///RPA_timeout
    uint16_t        rpa_timeout;	
};

#if (RWBLE_SW_VERSION_MINOR >= 1)
/// HCI LE set privacy mode (ESR10)
struct hci_le_set_privacy_mode_cmd
{
    ///identity address type
    uint8_t        identity_address_type;
    ///identity address
    uint8_t        identity_address[BD_ADDR_LEN];
    ///privacy mode, 0=Network Privacy, 1=Device Privacy
    uint8_t        privacy_mode;
};
#endif /* (RWBLE_SW_VERSION_MINOR >= 1) */
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

/*
 * HCI EVENTS PARAMETERS
 ****************************************************************************************
 */

/// HCI inquiry complete event structure
struct hci_inq_cmp_evt
{
    ///Status of the procedure
    uint8_t status;
};


/// HCI Inquiry result event structure (with only 1 result)
struct hci_inq_res_evt
{

    ///Number of response
    uint8_t     nb_rsp;
    ///BdAddr
    struct bd_addr bd_addr;
    ///Page Scan Repetition Mode
    uint8_t     page_scan_rep_mode;
    ///Reserved
    uint8_t     reserved1;
    ///Reserved
    uint8_t     reserved2;
    ///class of device
    struct devclass class_of_dev;
    ///Clock Offset
    uint16_t        clk_off;

};

/// HCI Inquiry result with rssi event structure (with only 1 result)
struct hci_inq_res_with_rssi_evt
{
    ///Number of response
    uint8_t     nb_rsp;
    ///BdAddr
    struct bd_addr  bd_addr;
    ///Page Scan Repetition Mode
    uint8_t     page_scan_rep_mode;
    ///Reserved
    uint8_t     reserved1;
    ///class of device
    struct devclass class_of_dev;
    ///Clock Offset
    uint16_t     clk_off;
    ///Rssi
    uint8_t      rssi;

};

/// HCI Extended inquiry result indication structure (with only 1 result)
struct hci_ext_inq_res_evt
{
    ///Number of response
    uint8_t     nb_rsp;
    ///BdAddr
    struct bd_addr  bd_addr;
    ///Page Scan Repetition Mode
    uint8_t     page_scan_rep_mode;
    ///Reserved
    uint8_t     reserved1;
    ///class of device
    struct devclass class_of_dev;
    ///Clock Offset
    uint16_t        clk_off;
    ///RSSi
    uint8_t         rssi;
    ///Extended inquiry response data
    struct eir      eir;
};

/// HCI disconnect complete event structure
struct hci_disc_cmp_evt
{
    ///Status of received command
    uint8_t     status;
    ///Connection Handle
    uint16_t    conhdl;
    ///Reason for disconnection
    uint8_t     reason;
};

/// HCI basic command complete event structure
struct hci_basic_cmd_cmp_evt
{
    ///Status of the command reception
    uint8_t status;
};

/// HCI basic command complete event structure with connection handle
struct hci_basic_conhdl_cmd_cmp_evt
{
    /// status
    uint8_t     status;
    /// connection handle
    uint16_t    conhdl;
};

/// HCI basic command complete event structure with BD address
struct hci_basic_bd_addr_cmd_cmp_evt
{
    ///status
   uint8_t         status;
   ///BdAddr
    struct bd_addr bd_addr;
};

/// HCI basic event including a connection handle as parameter
struct hci_basic_conhdl_evt
{
    ///Connection handle
    uint16_t     conhdl;
};

/// HCI complete event with status only.
struct hci_cmd_stat_event
{
    /// Status of the command reception
    uint8_t status;
};

/// HCI number of packet complete event structure
struct hci_nb_cmp_pkts_evt
{
    /// number of handles
    uint8_t     nb_of_hdl;
    /// connection handle
    uint16_t    conhdl[1];
    /// number of completed packets
    uint16_t    nb_comp_pkt[1];
};

/// HCI data buffer overflow event structure
struct hci_data_buf_ovflw_evt
{
    ///Link type
    uint8_t link_type;
};

/// HCI Hardware Error Event parameters structure
struct hci_hw_err_evt
{
    /// HW error code
    uint8_t hw_code;
};

/// HCI encryption change event structure
struct hci_enc_change_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Encryption enabled information
    uint8_t     enc_stat;
};

/// HCI encryption key refresh complete event structure
struct hci_enc_key_ref_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
};

/// HCI Authenticated Payload Timeout Expired Event structure
struct hci_auth_payl_to_exp_evt
{
    ///Connection handle
    uint16_t     conhdl;
};

/// HCI command complete event structure for create connection
struct hci_con_cmp_evt
{
    /// Status
    uint8_t             status;
    ///Connection handle
    uint16_t            conhdl;
    ///Bluetooth Device address
    struct bd_addr      bd_addr;
    ///Link type
    uint8_t             link_type;
    ///Encryption state
    uint8_t             enc_en;
};

/// HCI command complete event structure for qos setup
struct hci_qos_setup_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Flags
    uint8_t flags;
    ///Service type
    uint8_t serv_type;
    ///Token rate
    uint32_t tok_rate;
    ///Peak bandwidth
    uint32_t pk_bw;
    ///Latency
    uint32_t lat;
    ///Delay variation
    uint32_t del_var;
};

/// HCI flow specification complete event parameters structure
struct hci_flow_spec_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Flags
    uint8_t flags;
    ///Flow direction
    uint8_t flow_dir;
    ///Service type
    uint8_t serv_type;
    ///Token rate
    uint32_t tk_rate;
    ///Token buffer size
    uint32_t tk_buf_sz;
    ///Peak bandwidth
    uint32_t pk_bw;
    ///Access latency
    uint32_t acc_lat;
};

/// HCI role change event parameters structure
struct hci_role_chg_evt
{
    ///Status
    uint8_t status;
    ///BD address
    struct bd_addr bd_addr;
    ///New role
    uint8_t new_role;
};

/// HCI complete event structure for the read clock offset command
struct hci_rd_clk_off_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Clock offset
    uint16_t clk_off_val;
};

/// HCI event structure for the flush occurred event
struct hci_flush_occurred_evt
{
    ///Connection handle
    uint16_t conhdl;
};

/// HCI max slot change event structure
struct hci_max_slot_chg_evt
{
    ///Connection handle
    uint16_t conhdl;
    ///Max slot
    uint8_t max_slot;
};

/// HCI sniff subrating event parameters structure
struct hci_sniff_sub_evt
{
    ///Status.
    uint8_t     status;
    ///Connection handle
    uint16_t    conhdl;
    ///Maximum transmit latency
    uint16_t    max_lat_tx;
    ///Maximum receive latency
    uint16_t    max_lat_rx;
    ///Minimum remote TO
    uint16_t    min_rem_to;
    ///Minimum local TO
    uint16_t    min_loc_to;
};

/// HCI read remote extended features complete event parameters structure
struct hci_rd_rem_ext_feats_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///page number
    uint8_t pg_nb;
    ///page number max
    uint8_t pg_nb_max;
    ///ext LMP features
    struct features ext_feats;
};

/// HCI read remote extended features complete event parameters structure
struct hci_rem_host_supp_feats_notif_evt
{
    ///BD address
    struct bd_addr bd_addr;
    ///ext lmp features
    struct features ext_feats;
};

/// HCI command complete event structure for the read remote supported features command
struct hci_rd_rem_supp_feats_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Remote features
    struct features rem_feats;
};

/// HCI command complete event structure for the read remote information version command
struct hci_rd_rem_ver_info_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///LMP version
    uint8_t     vers;
    ///Manufacturer name
    uint16_t    compid;
    ///LMP subversion
    uint16_t    subvers;
};

/// HCI encryption change event structure
struct hci_enc_chg_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Encryption enabled information
    uint8_t     enc_stat;
};

/// HCI mode change event structure
struct hci_mode_chg_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t    conhdl;
    ///Current mode
    uint8_t    cur_mode;
    /// Interval
    uint16_t    interv;
};

/// HCI simple pairing complete event structure
struct hci_sp_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Bluetooth Device address
    struct bd_addr      bd_addr;
};

/// HCI Authentication complete event structure
struct hci_auth_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t    conhdl;
};

/// HCI change connection link key complete event structure
struct hci_chg_con_lk_cmp_evt
{
    ///Status
    uint8_t status;
    ///Connection handle
    uint16_t    conhdl;
};

/// HCI encryption key refresh complete event structure
struct hci_enc_key_refresh_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t    conhdl;
};

/// HCI master link key complete event structure
struct hci_master_lk_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t    conhdl;
    ///Key flag
    uint8_t key_flag;
};
/// HCI synchronous link connection complete event structure
struct hci_sync_con_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t    conhdl;
    ///BD address
    struct bd_addr bd_addr;
    ///Link type
    uint8_t lk_type;
    ///Transmit interval
    uint8_t tx_int;
    ///Retransmission window
    uint8_t ret_win;
    ///rx packet length
    uint16_t rx_pkt_len;
    ///tx packet length
    uint16_t tx_pkt_len;
    ///Air mode
    uint8_t air_mode;

};

/// HCI synchronous connection change event structure
struct hci_sync_con_chg_evt
{
    ///Status for command reception
    uint8_t status;
    ///Synchronous Connection handle
    uint16_t    sync_conhdl;
    ///Transmit interval
    uint8_t tx_int;
    ///Retransmission window
    uint8_t ret_win;
    ///rx packet length
    uint16_t rx_pkt_len;
    ///tx packet length
    uint16_t tx_pkt_len;
};

/// HCI connection packet type change event structure
struct hci_con_pkt_type_chg_evt
{
    ///Status for command reception
    uint8_t status;
    ///Synchronous Connection handle
    uint16_t    sync_conhdl;
    ///Synchronous packet type
    uint16_t    pkt_type;
};

/// HCI link supervision timeout change event structure
struct hci_link_supv_to_chg_evt
{
    ///Connection handle
    uint16_t    conhdl;
    ///Link supervision timeout
    uint16_t    lsto_val;
};

/// HCI link key request event structure
struct hci_lk_req_evt
{
    ///BD address
    struct bd_addr bd_addr;
};

/// HCI encryption key refresh event structure
struct hci_enc_key_refresh_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t    conhdl;
};

/// HCI connection request event structure
struct hci_con_req_evt
{
    ///BD address
    struct bd_addr bd_addr;
    ///Class of device
    struct devclass classofdev;
    ///link type
    uint8_t lk_type;
};

/// HCI quality of service violation event structure
struct hci_qos_viol_evt
{
    ///Connection handle
    uint16_t conhdl;
};

/// HCI io capability response event structure
struct hci_io_cap_rsp_evt
{
    ///BdAddr
    struct bd_addr  bd_addr;
    ///IO capability
    uint8_t io_capa;
    ///OOB data present
    uint8_t oob_data_pres;
    ///Authentication requirements
    uint8_t auth_req;

};

/// HCI IO capability response event structure
struct hci_io_cap_req_evt
{
    ///BdAddr
    struct bd_addr  bd_addr;
};

/// HCI Return link keys event structure
struct hci_return_link_keys_evt
{
    ///Number of Keys
    uint8_t num_keys;
    ///BdAddr
    struct bd_addr  bd_addr;
    ///Key
    struct ltk      key;
};

/// HCI pin code request event structure
struct hci_pin_code_req_evt
{
    ///BdAddr
    struct bd_addr  bd_addr;
};

/// HCI user passkey request event structure
struct hci_user_passkey_req_evt
{
    ///BdAddr
    struct bd_addr  bd_addr;
};

/// HCI user passkey notification event structure
struct hci_user_passkey_notif_evt
{
    ///BdAddr
    struct bd_addr  bd_addr;
    ///Passkey
    uint32_t passkey;
};

/// HCI remote OOB data request event structure
struct hci_rem_oob_data_req_evt
{
    ///BdAddr
    struct bd_addr  bd_addr;
};

/// HCI user confirmation request event structure
struct hci_user_cfm_req_evt
{
    ///BdAddr
    struct bd_addr  bd_addr;
    ///Passkey
    uint32_t passkey;
};

/// HCI keypress notification event structure
struct hci_keypress_notif_evt
{
    ///BdAddr
    struct bd_addr  bd_addr;
    ///type
    uint8_t type;
};

/// HCI link key notification event structure
struct hci_lk_notif_evt
{
    ///BdAddr
    struct bd_addr  bd_addr;
    ///Key
    struct ltk  key;
    ///type
    uint8_t key_type;
};


/*
 * HCI LE META EVENTS PARAMETERS
 ****************************************************************************************
 */


// LE event structures

/// HCI command complete event structure for the Read Local Supported Features
struct hci_le_rd_local_supp_feats_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t             status;
    ///Local LE supported features
    struct le_features  feats;
};

/// HCI command complete event structure for the Read Advertising Channel Tx Power Command
struct hci_rd_adv_chnl_tx_pw_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    ///Advertising channel Tx power level
    int8_t     adv_tx_pw_lvl;
};

///HCI command complete event structure for the Read White List Size Command
struct hci_rd_wlst_size_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    ///White List size
    uint8_t     wlst_size;
};

///HCI command complete event structure for the Read Buffer Size Command
struct hci_le_rd_buff_size_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    ///ACL data packet length that can be sent from host to controller
    uint16_t    hc_data_pk_len;
    ///Total number of ACL data packets that can be sent from host to controller.
    uint8_t     hc_tot_nb_data_pkts;
};

///HCI command complete event structure for LE Rand Command
struct hci_le_rand_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t             status;
    ///Random number
    struct rand_nb      nb;
};

///HCI command complete event structure for Read Supported States Command
struct hci_rd_supp_states_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t             status;
    ///LE supported states response
    struct le_states    states;
};

///HCI command complete event structure for Test End
struct hci_test_end_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t             status;
    ///Number of RX packets - null if TX test was the ended one
    uint16_t            nb_packet_received;
};

///HCI LE Encrypt complete event structure
struct hci_le_enc_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t status;
    ///Encrypted data to return to command source.
    uint8_t encrypted_data[ENC_DATA_LEN];
};

#if BLE_EMB_PRESENT || BLE_HOST_PRESENT
///HCI LE advertising report event structure
struct hci_le_adv_report_evt
{
    ///LE Subevent code
    uint8_t             subcode;
    ///Number of advertising reports in this event
    uint8_t             nb_reports;
    ///Advertising reports structures array
    struct adv_report   adv_rep[BLE_ADV_REPORTS_MAX];
};

#if (RWBLE_SW_VERSION_MAJOR >= 8)
///HCI LE advertising report event structure
struct hci_le_direct_adv_report_evt
{
    ///LE Subevent code
    uint8_t             subcode;
    ///Number of advertising reports in this event
    uint8_t             nb_reports;
    ///Advertising reports structures array
    struct direct_adv_report   adv_rep[BLE_ADV_REPORTS_MAX];
};
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */
#endif //BLE_EMB_PRESENT || BLE_HOST_PRESENT

/// HCI command complete event structure for Read Channel Map Command
struct hci_le_rd_chnl_map_cmd_cmp_evt
{
    ///Status of command reception
    uint8_t            status;
    ///Connection handle
    uint16_t           conhdl;
    ///Channel map
    struct le_chnl_map ch_map;
};

/// HCI command complete event structure for Long Term Key Request Reply Command
struct hci_le_ltk_req_reply_cmd_cmp_evt
{
    ///Status of command reception
    uint8_t        status;
    ///Connection handle
    uint16_t       conhdl;
};

/// HCI command complete event structure for Long Term Key Request Negative Reply Command
struct hci_le_ltk_req_neg_reply_cmd_cmp_evt
{
    ///Status of command reception
    uint8_t        status;
    ///Connection handle
    uint16_t       conhdl;
};

/// HCI write authenticated payload timeout command
struct hci_wr_auth_payl_to_cmd
{
    ///Connection handle
    uint16_t     conhdl;
    ///Authenticated payload timeout
    uint16_t     auth_payl_to;
};

/// HCI command complete event structure for the Write Authenticated Payload Timeout Command
struct hci_wr_auth_payl_to_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    ///Connection handle
    uint16_t     conhdl;
};

/// HCI command complete event structure for HCI LE Connection Update Command
struct hci_le_con_update_cmp_evt
{
    ///LE Subevent code
    uint8_t             subcode;
    ///Status of received command
    uint8_t             status;
    ///Connection handle
    uint16_t            conhdl;
    ///Connection interval value
    uint16_t            con_interval;
    ///Connection latency value
    uint16_t            con_latency;
    ///Supervision timeout
    uint16_t            sup_to;
};

/// HCI command complete event structure for create connection
struct hci_le_con_cmp_evt
{
    ///LE Subevent code
    uint8_t             subcode;
    ///Status of received command
    uint8_t             status;
    ///Connection handle
    uint16_t            conhdl;
    ///Device role - 0=Master/ 1=Slave
    uint8_t             role;
    ///Peer address type - 0=public/1=random
    uint8_t             peer_addr_type;
    ///Peer address
    struct bd_addr      peer_addr;
    ///Connection interval
    uint16_t            con_interval;
    ///Connection latency
    uint16_t            con_latency;
    ///Link supervision timeout
    uint16_t            sup_to;
    ///Master clock accuracy
    uint8_t             clk_accuracy;
};

/// HCI LE read remote used feature command parameters structure
struct hci_le_rd_rem_used_feats_cmd
{
    ///Connection handle
    uint16_t            conhdl;
};

/// HCI command complete event structure for HCI LE read remote used feature Command
struct hci_le_rd_rem_used_feats_cmd_cmp_evt
{
    ///LE Subevent code
    uint8_t             subcode;
    ///Status of received command
    uint8_t             status;
    ///Connection handle
    uint16_t            conhdl;
    ///Le Features used
    struct le_features  feats_used;
};

/// HCI command structure for the read transmit power level command
struct hci_rd_tx_pwr_lvl_cmd
{
    ///Connection handle
    uint16_t    conhdl;
    ///Power Level type: current or maximum
    uint8_t     type;
};

/// HCI command complete event structure for the read transmit power level command
struct hci_rd_tx_pwr_lvl_cmd_cmp_evt
{
    ///Status for command reception
    uint8_t status;
    ///Connection handle
    uint16_t conhdl;
    ///Value of TX power level
    uint8_t     tx_pow_lvl;
};

/// HCI read remote information version command parameters structure
struct hci_rd_rem_ver_info_cmd
{
    ///Connection handle
    uint16_t    conhdl;
};

/// HCI LE remote connection parameter request event
struct hci_le_rem_con_param_req_evt
{
    ///LE Subevent code
    uint8_t             subcode;
    ///Connection handle
    uint16_t            conhdl;
    ///Interval_Min
    uint16_t            interval_min;
    ///Interval_Max
    uint16_t            interval_max;
    ///Latency
    uint16_t            latency;
    ///Timeout
    uint16_t            timeout;
};

/// HCI command complete event structure for HCI LE read remote used feature Command
struct hci_le_ltk_request_evt
{
    ///LE Subevent code
    uint8_t             subcode;
    ///Connection handle
    uint16_t            conhdl;
    ///Random number
    struct rand_nb      rand;
    ///Encryption diversifier
    uint16_t            ediv;
};

#if (RWBLE_SW_VERSION_MAJOR >= 8)
/// HCI LE data length change event
struct hci_le_data_length_change_evt
{
    ///LE Subevent code
    uint8_t             subcode;
    ///Connection handle
    uint16_t            conhdl;
    ///MaxTxOctets
    uint16_t            max_txoctets;
    ///MaxTxTime
    uint16_t            max_txtime;
    ///MaxRxOctets
    uint16_t            max_rxoctets;
    ///MaxRxTime
    uint16_t            max_rxtime;
};

/// HCI enhanced command complete event structure for create connection
struct hci_le_enh_con_cmp_evt
{
    ///LE Subevent code
    uint8_t             subcode;
    ///Status of received command
    uint8_t             status;
    ///Connection handle
    uint16_t            conhdl;
    ///Device role - 0=Master/ 1=Slave
    uint8_t             role;
    ///Peer address type - 0=public/1=random/2=Public Identity/3=Random Identity
    uint8_t             peer_addr_type;
    ///Peer address
    struct bd_addr      peer_addr;
    ///Local RPA address, valid if own_address_type == 2 or 3 otherwise all zeros
    struct bd_addr      local_rpa_addr;
    ///Peer RPA address, valid if peer_address_type == 2 or 3 otherwise all zeros
    struct bd_addr      peer_rpa_addr;
    ///Connection interval
    uint16_t            con_interval;
    ///Connection latency
    uint16_t            con_latency;
    ///Link supervision timeout
    uint16_t            sup_to;
    ///Master clock accuracy
    uint8_t             clk_accuracy;
};

///HCI command complete event structure for the Read Suggested Default Data Length Command
struct hci_le_read_suggested_default_data_length_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    ///suggested max tx octets
    uint16_t    suggested_tx_octets;
    ///suggested max tx time
    uint16_t    suggested_tx_time;
};

///HCI command complete event structure for the Read Max Data Length Command
struct hci_le_read_max_data_length_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    ///max tx octets
    uint16_t    max_tx_octets;
    ///max tx time
    uint16_t    max_tx_time;
    ///max rx octets
    uint16_t    max_rx_octets;
    ///max rx time
    uint16_t    max_rx_time;
};

///HCI command complete event structure for the Read Peer/Local Resolvable AddressCommand
struct hci_le_dev_from_rslv_list_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    ///identity address
    uint8_t        address[BD_ADDR_LEN];
};

///HCI command complete event structure for the Read Resolving List Size Command
struct hci_le_rd_rslv_list_size_cmd_cmp_evt
{
    /// Status of the command reception
    uint8_t     status;
    ///resolving list size
    uint8_t    resolving_list_size;
};
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

/*
 * HCI VENDOR SPECIFIC COMMANDS PARAMETERS
 ****************************************************************************************
 */

/// Buffer structure
struct buffer_tag
{
    /// length of buffer
    uint8_t length;
    /// data of 128 bytes length
    uint8_t data[128];
};

/// Common structure for Command Complete Event of HCI Debug Read Memory/Flash/Param complete event parameters - vendor specific
struct hci_dbg_basic_rd_data_cmd_cmp_evt
{
    ///Status
    uint8_t status;
    ///buffer structure to return
    struct buffer_tag buf;
};

///HCI Debug read memory variable command parameters - vendor specific
struct hci_dbg_rd_mem_cmd
{
    ///Start address to read
    uint32_t start_addr;
    ///Access size
    uint8_t type;
    ///Length to read
    uint8_t length;
};

///HCI Debug write memory variable command parameters - vendor specific
struct hci_dbg_wr_mem_cmd
{
    ///Start address to read
    uint32_t start_addr;
    ///Access size
    uint8_t type;
    ///buffer structure to return
    struct buffer_tag buf;
};

///HCI Debug delete parameter command parameters - vendor specific
struct hci_dbg_del_param_cmd
{
    ///Parameter tag
    uint16_t param_tag;
};

///HCI Debug erase flash command parameters - vendor specific
struct hci_dbg_er_flash_cmd
{
    ///Flash type
    uint8_t flashtype;
    ///Start offset address
    uint32_t startoffset;
    ///Size to erase
    uint32_t size;
};

///HCI Debug write flash command parameters - vendor specific
struct hci_dbg_wr_flash_cmd
{
    ///Flash type
    uint8_t flashtype;
    ///Start offset address
    uint32_t startoffset;
    ///buffer structure
    struct buffer_tag buf;
};

///HCI Debug read flash command parameters - vendor specific
struct hci_dbg_rd_flash_cmd
{
    ///Flash type
    uint8_t flashtype;
    ///Start offset address
    uint32_t startoffset;
    ///Size to read
    uint8_t size;
};

///HCI Debug read parameter command parameters - vendor specific
struct hci_dbg_rd_par_cmd
{
    ///Parameter tag
    uint16_t param_tag;
};

///HCI Debug read parameters command parameters - vendor specific
struct hci_dbg_wr_par_cmd
{
    ///Parameter tag
    uint16_t param_tag;
    ///Structure buffer
    struct buffer_tag buf;
};

///HCI Debug Read Kernel Statistics complete event parameters - vendor specific
struct hci_dbg_rd_ke_stats_cmd_cmp_evt
{
    ///Status
    uint8_t status;
    ///Max message sent
    uint8_t max_msg_sent;
    ///Max message saved
    uint8_t max_msg_saved;
    ///Max timer used
    uint8_t max_timer_used;
    ///Max heap used
    uint16_t max_heap_used;
    ///Max stack used
    uint16_t max_stack_used;
};


/// HCI Debug Read information about memory usage. - vendor specific
struct hci_dbg_rd_mem_info_cmd_cmp_evt
{
    ///Status
    uint8_t status;
    /// memory size currently used into each heaps.
    uint16_t mem_used[KE_MEM_BLOCK_MAX];
    /// peak of memory usage measured
    uint32_t max_mem_used;
};

///HCI Debug identify Flash command complete event parameters - vendor specific
struct hci_dbg_id_flash_cmd_cmp_evt
{
    ///Status
    uint8_t status;
    ///Flash identity
    uint8_t flash_id;
};

///HCI Debug RF Register read command
struct hci_dbg_rf_reg_rd_cmd
{
    /// register address
    uint16_t addr;
};

///HCI Debug RF Register read command complete event
struct hci_dbg_rf_reg_rd_cmd_cmp_evt
{
    /// status
    uint8_t status;
    /// register address
    uint16_t addr;
    /// register value
    uint32_t value;
};

///HCI Debug RF Register write command
struct hci_dbg_rf_reg_wr_cmd
{
    /// register address
    uint16_t addr;
    /// register value
    uint32_t value;
};

///HCI Debug RF Register write command complete event
struct hci_dbg_rf_reg_wr_cmd_cmp_evt
{
    /// status
    uint8_t status;
    /// address
    uint16_t addr;
};

///HCI Debug platform reset command parameters - vendor specific
struct hci_dbg_plf_reset_cmd
{
    /// reason
    uint8_t reason;
};

#if (RW_WLAN_COEX)
///HCI Debug wlan coexistence command parameters - vendor specific
struct hci_dbg_wlan_coex_cmd
{
    /// State
    uint8_t state;
};
#if (RW_WLAN_COEX_TEST)
///HCI Debug wlan coexistence test scenario command parameters - vendor specific
struct hci_dbg_wlan_coextst_scen_cmd
{
    /// Scenario
    uint32_t scenario;
};
#endif //RW_WLAN_COEX_TEST
#endif //RW_WLAN_COEX

///HCI Debug HW Register Read command parameters - vendor specific
struct hci_dbg_hw_reg_rd_cmd
{
    /// register address
    uint16_t reg_addr;
};


///HCI Debug HW Register write command parameters - vendor specific
struct hci_dbg_hw_reg_wr_cmd
{
    /// register address
    uint16_t reg_addr;
    /// extra parameter
    uint16_t reserved;
    /// register value
    uint32_t reg_value;
};

///HCI Debug HW Register Read Complete event parameters - vendor specific
struct hci_dbg_hw_reg_rd_cmd_cmp_evt
{
    /// status
    uint8_t  status;
    /// register address
    uint16_t reg_addr;
    /// register value
    uint32_t reg_value;
};

///HCI Debug HW Register Write Complete event parameters - vendor specific
struct hci_dbg_hw_reg_wr_cmd_cmp_evt
{
    /// status
    uint8_t  status;
    /// register address
    uint16_t reg_addr;
};


#if (BLE_EMB_PRESENT || BLE_HOST_PRESENT)
///HCI Debug bd address write command parameters - vendor specific
struct hci_dbg_set_bd_addr_cmd
{
    ///bd address to set
    struct bd_addr addr;
};

///HCI Debug crc write command parameters - vendor specific
struct hci_dbg_set_crc_cmd
{
    /// Handle pointing to the connection for which CRC has to be modified
    uint16_t conhdl;
    /// CRC to set
    struct crc_init crc;
};

///HCI Debug LLC discard command parameters - vendor specific
struct hci_dbg_llcp_discard_cmd
{
    /// Handle pointing to the connection for which LLCP commands have to be discarded
    uint16_t conhdl;
    /// Flag indicating if the discarding has to be enabled or disabled
    uint8_t enable;
};

///HCI Debug reset RX counter command parameters - vendor specific
struct hci_dbg_reset_rx_cnt_cmd
{
    /// Handle pointing to the connection for which the counter have to be reseted
    uint16_t conhdl;
};

///HCI Debug reset TX counter command parameters - vendor specific
struct hci_dbg_reset_tx_cnt_cmd
{
    /// Handle pointing to the connection for which the counter have to be reseted
    uint16_t conhdl;
};

///HCI Debug Set TX Power Level Command parameters
struct hci_dbg_set_tx_pw_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// Power level
    uint8_t  pw_lvl;
};

#if (BLE_TESTER)
///HCI Tester set LE parameters
struct hci_tester_set_le_params_cmd
{
    /// Connection handle
    uint16_t conhdl;
    /// Tester features
    uint8_t  tester_feats;
    /// Preferred periodicity
    uint8_t  pref_period;
    /// Offset0
    uint16_t  offset0;
    /// Offset1
    uint16_t  offset1;
    /// Offset2
    uint16_t  offset2;
    /// Offset3
    uint16_t  offset3;
    /// Offset4
    uint16_t  offset4;
    /// Offset5
    uint16_t  offset5;
};
#endif //(BLE_TESTER)

#if (RWBLE_SW_VERSION_MAJOR >= 8)
///HCI Debug Data Length Extention write command
struct hci_dbg_wr_dle_dft_value_cmd
{
    ///supported max tx octets
    uint16_t    supported_tx_octets;
    ///supported max tx time
    uint16_t    supported_tx_time;
    ///supported max rx octets
    uint16_t    supported_rx_octets;
    ///supported max rx time
    uint16_t    supported_rx_time;
};

///HCI Debug Data Length Extention write command complete event
struct hci_dbg_wr_dle_dft_value_cmd_cmp_evt
{
    /// status
    uint8_t status;
};

///HCI Debug Resolving List Size write command
struct hci_dbg_wr_rl_size_cmd
{
    ///max resolving list size
    uint8_t    rl_size;
};

///HCI Debug Resolving List Size write command complete event
struct hci_dbg_wr_rl_size_cmd_cmp_evt
{
    /// status
    uint8_t status;
};

///HCI Debug Filter Duplicate List Size write command
struct hci_dbg_wr_filt_dup_size_cmd
{
    ///max filter duplicate list size
    uint8_t    filt_dup_size;
};

///HCI Debug Filter Duplicate List Size write command complete event
struct hci_dbg_wr_filt_dup_size_cmd_cmp_evt
{
    /// status
    uint8_t status;
};
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

#endif //BLE_EMB_PRESENT || BLE_HOST_PRESENT

#if (RWBLE_SW_VERSION_MAJOR >= 8)
/// HCI LE generate DHKey
struct hci_le_generate_dhkey_cmd
{
    ///remote P-256 Public Key
    uint8_t        public_key[ECDH_KEY_LEN*2];
};

/// HCI command complete event structure for the Read P-256 Public Key Command
struct hci_rd_p256_public_key_cmd_cmp_evt
{
    ///LE Subevent code
    uint8_t     subcode;
    /// Status of the command reception
    uint8_t     status;
    ///P256 Public Key
    uint8_t     public_key[ECDH_KEY_LEN*2];
};

/// HCI command complete event structure for the Generate DHKey Command
struct hci_generate_dhkey_cmd_cmp_evt
{
    ///LE Subevent code
    uint8_t     subcode;
    /// Status of the command reception
    uint8_t     status;
    ///P256 Public Key
    uint8_t     dhkey[ECDH_KEY_LEN];
};
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

/// @} CO_BT
#endif // CO_HCI_H_
