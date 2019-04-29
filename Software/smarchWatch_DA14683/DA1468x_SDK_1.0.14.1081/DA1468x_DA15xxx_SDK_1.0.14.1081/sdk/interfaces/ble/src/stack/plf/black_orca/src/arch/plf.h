/**
 ****************************************************************************************
 *
 * @file plf.h
 *
 * @brief This file contains the definitions of the macros and functions that are
 * platform dependent.  The implementation of those is implemented in the
 * appropriate platform directory.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */
#ifndef _PLF_H_
#define _PLF_H_

/**
 ****************************************************************************************
 * @defgroup PLF
 * @ingroup DRIVERS
 *
 * @brief Platform register driver
 *
 * @{
 *
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

#define plf_read_rf_board_id()      0
#define plf_rf_switch()             1

/// @} PLF

#endif // _PLF_H_
