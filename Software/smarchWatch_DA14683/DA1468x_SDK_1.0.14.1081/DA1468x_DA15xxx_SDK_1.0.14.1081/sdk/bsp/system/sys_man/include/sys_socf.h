/**
 * \addtogroup BSP
 * \{
 * \addtogroup SYSTEM
 * \{
 * \addtogroup State_Of_Charge
 *
 * \brief SOCF functions
 *
 * \{
 */
/**
 ****************************************************************************************
 *
 * @file sys_socf.h
 *
 * @brief external functions of SOCF driver
 *
 * Copyright (C) 2016-2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef SYS_SOCF_H_
#define SYS_SOCF_H_

/**
 * \brief Get SOC value.
 *
 * \details Application can get SOC from this function any time.
 *        Return value is from 0(0%) to 1000(100.0%).
 *        So proper SOC(0 ~ 100%) for BAS profile would be (soc_get_soc() + 5) / 10.
 *
 * \return SOC with range from 0 to 1000
 *
 */
int16_t socf_get_soc(void);

/**
 * \brief Set SOC value.
 *
 * \param [in] soc value.
 *
 * \details Application can set SOC using this function any time.
 *        Input value is from 0(0%) to 1000(100.0%).
 *
 */
void socf_set_init_soc(int16_t soc);

/**
 * \brief Initialization of SOC value.
 *
 * \warning This function must be called before USB charging is enabled.
 *
 */
void socf_init_soc(void);

/**
 * \brief Initialization of SOCF driver.
 *
 * \details This function is called in pm_system_init.
 *
 */
void socf_init(void);

/**
 * \brief Starts a timer for measuring the coulomb count.
 *
 * \details The coulomb count is measured every second if active mode stays over one second.
 *
 */
void socf_start_timer(void);

/**
 * \brief Give a notification of full charging from charger driver to socf driver.
 *
 * \details The capacitance is updated in this function if the capacitance is reduced by aging.
 *
 */
void socf_full_charged_notification(void);

/**
 * \brief Set a capacitance manually.
 *
 * \param [in] capacitance value.
 *
 * \details If the capacitance value must be sustained after reboot, this function can be used
 * with soc_get_capacity. This function must be called before socf_init_soc.
 *
 */
void socf_set_capacity(int16_t capacitance);

/**
 * \brief Get a capacitance.
 *
 * \details Get the current capacitance. If you want to reuse this capacitance after reboot,
 * this value must be saved to some area like NVMS.
 *
 * \return capacitance value
 *
 */
int16_t socf_get_capacity(void);

#endif /* SYS_SOCF_H_ */
/**
 \}
 \}
 \}
 */
