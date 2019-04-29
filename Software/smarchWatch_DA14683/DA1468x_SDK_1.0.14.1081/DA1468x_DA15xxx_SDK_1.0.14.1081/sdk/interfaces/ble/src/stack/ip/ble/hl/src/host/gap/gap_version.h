/**
 ****************************************************************************************
 *
 * @file gap_version.h
 *
 * @brief Definition of BLE Host version macros.
 *
 * Copyright (C) RivieraWaves 2009-2014
 *
 *
 ****************************************************************************************
 */

#ifndef GAP_VERSION_H_
#define GAP_VERSION_H_

// Version has the form Major.minor.release
#if ((dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A) \
                && (dg_configBLACK_ORCA_IC_STEP == BLACK_ORCA_IC_STEP_E))
/// Major version. This value is coming from the assigned numbers and corresponds to the
/// version of specification that the host is following.
/// For more information, go to https://www.bluetooth.org/Technical/AssignedNumbers/link_manager.htm
#define BLE_HOST_VERSION_MAJ       8  // From Assigned Numbers

/// Minor version. This value is incremented when new features are added to the host
#define BLE_HOST_VERSION_MIN       0

/// Release number. This value is incremented for every delivery done to the validation
/// team prior to release to customer
#define BLE_HOST_VERSION_REL       14
#elif ((dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_B) \
                && (dg_configBLACK_ORCA_IC_STEP == BLACK_ORCA_IC_STEP_B))
/// Major version. This value is coming from the assigned numbers and corresponds to the
/// version of specification that the host is following.
/// For more information, go to https://www.bluetooth.org/Technical/AssignedNumbers/link_manager.htm
#define BLE_HOST_VERSION_MAJ       8  // From Assigned Numbers

/// Minor version. This value is incremented when new features are added to the host
#define BLE_HOST_VERSION_MIN       1

/// Release number. This value is incremented for every delivery done to the validation
/// team prior to release to customer
#define BLE_HOST_VERSION_REL       14
#else
#error "Unsupported chip version"
#endif

#endif // GAP_VERSION_H_
