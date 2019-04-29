/**
 ****************************************************************************************
 *
 * @file lld_util.h
 *
 * @brief Link layer driver utilities definitions
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */
#ifndef LLD_UTIL_H_
#define LLD_UTIL_H_
/**
 ****************************************************************************************
 * @addtogroup LLDUTIL
 * @ingroup LLD
 * @brief Link layer driver utilities definitions
 *
 * full description
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include "co_bt.h"
#include "ea.h"

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief  Get the BD address
 *
 * @param[out]    Pointer to BD address buffer
 ****************************************************************************************
 */
void lld_util_get_bd_address(struct bd_addr *bd_addr);

/**
 ****************************************************************************************
 * @brief  Set the BD address
 *
 * @param[in]    bd_addr    Pointer to BD address buffer
 * @param[in]    type       Address type (0:public, 1:random)
 ****************************************************************************************
 */
void lld_util_set_bd_address(struct bd_addr *bd_addr, uint8_t type);

/**
 ****************************************************************************************
 * @brief  Convert frequecny in channel
 *
 * @param[in]    freq        Frequency to convert
 *
 * @return return the channel used
 *
 ****************************************************************************************
 */
uint8_t lld_util_freq2chnl(uint8_t freq);

/**
 ****************************************************************************************
 * @brief  Check if the element is in the programmed queue
 *
 * @param[in]    elt        element to be checked
 *
 * @return return true if the element has been found
 *
 ****************************************************************************************
 */
bool lld_util_elt_is_prog(struct ea_elt_tag *elt);


/**
 ****************************************************************************************
 * @brief This function returns the local offset (with respect to CLKN).
 *
 * @param[in] PeerOffset     Peer offset
 * @param[in] Interval       Interval
 * @param[in] AnchorPoint    Anchor point (with respect to CLKN)
 *
 * @return Offset value
 *
 ****************************************************************************************
 */
uint16_t lld_util_get_local_offset(uint16_t PeerOffset, uint16_t Interval, uint32_t AnchorPoint);

/**
 ****************************************************************************************
 * @brief This function returns the peer offset (with respect to CLKE).
 *
 * @param[in] LocalOffset      Local offset
 * @param[in] Interval         Interval
 * @param[in] AnchorPoint      Anchor point (with respect to CLKN)
 *
 * @return Offset value
 *
 ****************************************************************************************
 */
uint16_t lld_util_get_peer_offset(uint16_t LocalOffset, uint16_t Interval, uint32_t AnchorPoint);

/**
 ****************************************************************************************
 * @brief Function to set the duration and the start of the event
 *
 * @param[in] elt       element to update
 *
 * @param[in] param     parameter to compute the start of the event
 *
 ****************************************************************************************
 */
void lld_util_connection_param_set(struct ea_elt_tag *elt, struct ea_param_output* param);


/// @} LLDUTIL

#endif // LLD_UTIL_H_
