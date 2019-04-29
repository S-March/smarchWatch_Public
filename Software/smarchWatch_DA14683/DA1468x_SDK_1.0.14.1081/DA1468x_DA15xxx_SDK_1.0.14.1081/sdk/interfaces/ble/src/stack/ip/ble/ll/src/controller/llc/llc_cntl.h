/**
 ****************************************************************************************
 *
 * @file llc_cntl.h
 *
 * @brief Functions for control pdu transmission/reception handling
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef LLC_CNTL_H_
#define LLC_CNTL_H_

/**
 ****************************************************************************************
 * @addtogroup LLCCNTL LLCCNTL
 * @ingroup LLC
 * @brief Functions for control pdu transmission/reception handling
 *
 * This module implements the functions allowing the handling of the transmission and
 * reception of the control pdu.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "co_buf.h"
#include "co_version.h"
#include "llc_task.h"

#if (BLE_PERIPHERAL || BLE_CENTRAL)
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
/// LLCP PDU unpack function pointer type definition
typedef void (*llcp_pdu_unpk_func_t)(uint8_t *pdu, uint8_t parlen, uint8_t *param);

/// LLCP PDU unpacking details structure
struct llcp_pdu_unpk_util
{
    /// PDU unpacking handler
    llcp_pdu_unpk_func_t func;
    /// PDU opcode
    ke_msg_id_t msg_id;
    /// PDU length as defined in the standard (including opcode)
    uint8_t pdu_len;
    /// Parameter structure length
    uint8_t msg_len;
};

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Handles the reception of the control packet.
 *
 * This function checks the type of control packet and processes it .
 *
 * @param[in] conhdl        Connection handle on which the pdu is received.
 * @param[in] rxdesc        Pointer oh the reception descriptor where the parameters are
 *                          available.
 ****************************************************************************************
 */
void llc_cntl_rcv(uint16_t conhdl, uint8_t hdl);

/**
 ****************************************************************************************
 * @brief Sends the (extended) reject indication pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the PDU will be sent.
 * @param[in] reason        The reason to be put in the Reject Indication PDU
 ****************************************************************************************
 */
void llc_reject_ind_pdu_send(uint16_t conhdl, uint8_t rej_opcode, uint8_t reason);

/**
 ****************************************************************************************
 * @brief Sends the read remote information version pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the PDU will be sent.
 * @param[in] state         State to which the LLC has to be put to wait for the ACK
 ****************************************************************************************
 */
void llc_version_ind_pdu_send(uint16_t conhdl, ke_state_t state);

/**
 ****************************************************************************************
 * @brief Sends the set host channel classification pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_ch_map_update_pdu_send(uint16_t conhdl, uint16_t instant);

/**
 ****************************************************************************************
 * @brief Sends the pause encryption request pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_pause_enc_req_pdu_send(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the pause encryption response pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 * @param[in] state         State to which we have to put the LLC after PDU acknowledgment
 ****************************************************************************************
 */
void llc_pause_enc_rsp_pdu_send(uint16_t conhdl, ke_state_t state);

/**
 ****************************************************************************************
 * @brief Sends the encryption request pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 * @param[in] param         The parameters to be put in the encryption request
 ****************************************************************************************
 */
void llc_enc_req_pdu_send(uint16_t conhdl, struct hci_le_start_enc_cmd const *param);

/**
 ****************************************************************************************
 * @brief Sends the encryption response pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_enc_rsp_pdu_send(uint16_t conhdl, struct llcp_enc_req const *param);

/**
 ****************************************************************************************
 * @brief Sends the start encryption response pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_start_enc_rsp_pdu_send(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the connection update request pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 * @param[in] param         Pointer on the structure which contains all the updated
 *                          parameters useful for the link.
 ****************************************************************************************
 */
void llc_con_update_pdu_send(uint16_t conhdl, struct llcp_con_up_req *param);

/**
 ****************************************************************************************
 * @brief Sends the connection parameters request pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 * @param[in] param         Pointer on the structure which contains all the updated
 *                          parameters useful for the link.
 ****************************************************************************************
 */
void llc_con_param_req_pdu_send(uint16_t conhdl, struct llcp_con_param_req *param);

/**
 ****************************************************************************************
 * @brief Sends the connection parameters response pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 * @param[in] param         Pointer on the structure which contains all the updated
 *                          parameters useful for the link.
 ****************************************************************************************
 */
void llc_con_param_rsp_pdu_send(uint16_t conhdl, struct llcp_con_param_rsp *param);

/**
 ****************************************************************************************
 * @brief Sends the features request pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_feats_req_pdu_send(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the start encryption request pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_start_enc_req_pdu_send(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Sends the terminate indication pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 * @param[in] err_code      Reason of the termination.
 ****************************************************************************************
 */
void llc_terminate_ind_pdu_send(uint16_t conhdl, uint8_t err_code);
/**
 ****************************************************************************************
 * @brief Sends the unknown response pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 * @param[in] unk_type      Opcode of the unknown pdu type .
 ****************************************************************************************
 */
void llc_unknown_rsp_send_pdu(uint16_t conhdl, uint8_t unk_type);

/**
 ****************************************************************************************
 * @brief Sends the ping request pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_ping_req_pdu_send(uint16_t conhdl);
/**
 ****************************************************************************************
 * @brief Sends the ping response pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_ping_rsp_pdu_send(uint16_t conhdl);

#if (RWBLE_SW_VERSION_MAJOR >= 8)
/**
 ****************************************************************************************
 * @brief Sends the data length request pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_length_req_pdu_send(uint16_t conhdl);
/**
 ****************************************************************************************
 * @brief Sends the data length response pdu.
 *
 * This function allocates an sets header and parameters of the pdu before pushing it in
 * the tx queue.
 *
 * @param[in] conhdl        Connection handle on which the pdu will be sent.
 ****************************************************************************************
 */
void llc_length_rsp_pdu_send(uint16_t conhdl);

void llc_length_ind(uint16_t conhdl, struct llcp_length_req const *param);
#endif /* (RWBLE_SW_VERSION_MAJOR >= 8) */

/**
 ****************************************************************************************
 * @brief Handles the connection update request pdu.
 *
 * This function extracts the parameters from the control packet received and takes the
 * associated actions (new pdu generation, new state , parameters update, etc...).
 *
 * @param[in] conhdl        Connection handle on which the pdu has been received.
 * @param[in] param         Pointer on the received pdu parameters.
 ****************************************************************************************
 */
void llc_con_update_req_ind(uint16_t conhdl, struct llcp_con_up_req const *param);

/**
 ****************************************************************************************
 * @brief Handles the features request pdu.
 *
 * This function extracts the parameters from the control packet received and takes the
 * associated actions (new pdu generation, new state , parameters update, etc...).
 *
 * @param[in] conhdl        Connection handle on which the pdu has been received.
 * @param[in] param         Pointer on the received pdu parameters.
 ****************************************************************************************
 */
void llc_feats_req_ind(uint16_t conhdl, struct llcp_feats_req const *param);

/**
 ****************************************************************************************
 * @brief Handles the features response pdu.
 *
 * This function extracts the parameters from the control packet received and takes the
 * associated actions (new pdu generation, new state , parameters update, etc...).
 *
 * @param[in] conhdl        Connection handle on which the pdu has been received.
 * @param[in] param         Pointer on the received pdu parameters.
 ****************************************************************************************
 */
void llc_feats_rsp_ind(uint16_t conhdl, struct llcp_feats_rsp const *param);

/**
 ****************************************************************************************
 * @brief Handles the unknown response pdu.
 *
 * This function extracts the parameters from the control packet received and takes the
 * associated actions (new pdu generation, new state , parameters update, etc...).
 *
 * @param[in] conhdl        Connection handle on which the pdu has been received.
 * @param[in] rxbuf         Pointer on the received pdu parameters.
 ****************************************************************************************
 */
void llc_unknown_rsp_ind(uint16_t conhdl, uint8_t *rxbuf);

/**
 ****************************************************************************************
 * @brief Handles the version indication pdu.
 *
 * This function extracts the parameters from the control packet received and takes the
 * associated actions (new pdu generation, new state , parameters update, etc...).
 *
 * @param[in] conhdl        Connection handle on which the pdu has been received.
 * @param[in] param         Pointer on the received pdu parameters.
 ****************************************************************************************
 */
void llc_vers_ind_ind(uint16_t conhdl, struct llcp_vers_ind const *param);

/**
 ****************************************************************************************
 * @brief Handles the channel mapping request pdu.
 *
 * This function extracts the parameters from the control packet received and takes the
 * associated actions (new pdu generation, new state , parameters update, etc...).
 *
 * @param[in] conhdl        Connection handle on which the pdu has been received.
 * @param[in] rxbuf         Pointer on the received pdu parameters.
 ****************************************************************************************
 */
void llc_ch_map_req_ind (uint16_t conhdl, struct llcp_channel_map_req const *param);

/**
 ****************************************************************************************
 * @brief Handles the terminate indication pdu.
 *
 * This function extracts the parameters from the control packet received and takes the
 * associated actions (new pdu generation, new state , parameters update, etc...).
 *
 * @param[in] conhdl        Connection handle on which the pdu has been received.
 * @param[in] param         Pointer on the received pdu parameters.
 ****************************************************************************************
 */
void llc_terminate_ind (uint16_t conhdl, struct llcp_terminate_ind const *param);

/**
 ****************************************************************************************
 * @brief Handles the pause encryption request pdu.
 *
 * This function extracts the parameters from the control packet received and takes the
 * associated actions (new pdu generation, new state , parameters update, etc...).
 *
 * @param[in] conhdl        Connection handle on which the pdu has been received.
 ****************************************************************************************
 */
void llc_pause_enc_req_ind(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Handles the pause encryption response pdu.
 *
 * This function extracts the parameters from the control packet received and takes the
 * associated actions (new pdu generation, new state , parameters update, etc...).
 *
 * @param[in] conhdl        Connection handle on which the pdu has been received.
 ****************************************************************************************
 */
void llc_pause_enc_rsp_ind(uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Handles the encryption request pdu.
 *
 * This function extracts the parameters from the control packet received and takes the
 * associated actions (new pdu generation, new state , parameters update, etc...).
 *
 * @param[in] conhdl        Connection handle on which the pdu has been received.
 * @param[in] param         Pointer on the received pdu parameters.
 ****************************************************************************************
 */
void llc_enc_req_ind(uint16_t conhdl, struct llcp_enc_req const *param);

/**
 ****************************************************************************************
 * @brief Handles the encryption response pdu.
 *
 * This function extracts the parameters from the control packet received and takes the
 * associated actions (new pdu generation, new state , parameters update, etc...).
 *
 * @param[in] conhdl        Connection handle on which the pdu has been received.
 * @param[in] param         Pointer on the received pdu parameters.
 ****************************************************************************************
 */
void llc_enc_rsp_ind(uint16_t conhdl, struct llcp_enc_rsp const *param);

/**
 ****************************************************************************************
 * @brief Handles the start encryption request pdu.
 *
 * This function extracts the parameters from the control packet received and takes the
 * associated actions (new pdu generation, new state , parameters update, etc...).
 *
 * @param[in] conhdl        Connection handle on which the pdu has been received.
 * @param[in] rxbuf         Pointer on the received pdu parameters.
 ****************************************************************************************
 */
void llc_start_enc_req_ind (uint16_t conhdl);

/**
 ****************************************************************************************
 * @brief Handles the start encryption response pdu.
 *
 * This function extracts the parameters from the control packet received and takes the
 * associated actions (new pdu generation, new state , parameters update, etc...).
 *
 * @param[in] conhdl        Connection handle on which the pdu has been received.
 * @param[in] rxbuf         Pointer on the received pdu parameters.
 ****************************************************************************************
 */
void llc_start_enc_rsp_ind (uint16_t conhdl);

/**
******************************************************************************************
* @brief GENERAL COMMENT FOR llcp_..._pdu_unpk : LMP PDU param extraction function
*
* @param[in] pdu      Pointer to PDU buffer, without the 1 or two opcode bytes.
* @param[in] parlen   Length of left over pdu params.
* @param[in] param    Pointer to kernel message param position for direct copy of pdu params
*
******************************************************************************************
*/

void llcp_con_param_req_pdu_unpk(uint8_t * pdu, uint8_t parlen, uint8_t * param);
void llcp_con_param_rsp_pdu_unpk(uint8_t * pdu, uint8_t parlen, uint8_t * param);


#endif // #if (BLE_PERIPHERAL || BLE_CENTRAL)
/// @} LLCCNTL

#endif // LLC_CNTL_H_
