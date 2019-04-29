/**
 * \addtogroup BSP
 * \{
 * \addtogroup DEVICES
 * \{
 * \addtogroup USB_Charger
 * \{
 * \brief USB Charger
 */

/**
 ****************************************************************************************
 *
 * @file hw_usb_charger.h
 *
 * @brief Definition of API for the USB Charger.
 *
 * Copyright (C) 2015-2016 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */

#ifndef HW_USB_CHARGER_H_
#define HW_USB_CHARGER_H_

#if dg_configUSE_HW_USB_CHARGER

#include <stdbool.h>
#include <stdint.h>
#include <sdk_defs.h>

#include "sys_tcs.h"


/**
 * \brief Get the mask of a field of a USB_CHARGER register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to access
 *
 */
#define HW_USB_CHARGER_REG_FIELD_MASK(reg, field) \
        (USB_USB_CHARGER_##reg##_REG_##field##_Msk)

/**
 * \brief Get the bit position of a field of a USB_CHARGER register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to access
 *
 */
#define HW_USB_CHARGER_REG_FIELD_POS(reg, field) \
        (USB_USB_CHARGER_##reg##_REG_##field##_Pos)

/**
 * \brief Get the value of a field of a USB_CHARGER register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 *
 * \return the value of the register field
 *
 */
#define HW_USB_CHARGER_REG_GETF(reg, field) \
        ((USB->USB_CHARGER_##reg##_REG & (USB_USB_CHARGER_##reg##_REG_##field##_Msk)) >> (USB_USB_CHARGER_##reg##_REG_##field##_Pos))

/**
 * \brief Set the value of a field of a USB_CHARGER register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 * \param [in] new_val is the value to write
 *
 */
#define HW_USB_CHARGER_REG_SETF(reg, field, new_val) \
        USB->USB_CHARGER_##reg##_REG = ((USB->USB_CHARGER_##reg##_REG & ~(USB_USB_CHARGER_##reg##_REG_##field##_Msk)) | \
        ((USB_USB_CHARGER_##reg##_REG_##field##_Msk) & ((new_val) << (USB_USB_CHARGER_##reg##_REG_##field##_Pos))))

/**
 * \brief Get the mask of a field of a CHARGER register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to access
 *
 */
#define HW_CHARGER_REG_FIELD_MASK(reg, field) \
        (ANAMISC_CHARGER_##reg##_REG_##field##_Msk)

/**
 * \brief Get the bit position of a field of a CHARGER register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to access
 *
 */
#define HW_CHARGER_REG_FIELD_POS(reg, field) \
        (ANAMISC_CHARGER_##reg##_REG_##field##_Pos)

/**
 * \brief Get the value of a field of a CHARGER register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 *
 * \return the value of the register field
 *
 */
#define HW_CHARGER_REG_GETF(reg, field) \
        ((ANAMISC->CHARGER_##reg##_REG & (ANAMISC_CHARGER_##reg##_REG_##field##_Msk)) >> (ANAMISC_CHARGER_##reg##_REG_##field##_Pos))

/**
 * \brief Set the value of a field of a CHARGER register.
 *
 * \param [in] reg is the register to access
 * \param [in] field is the register field to write
 * \param [in] new_val is the value to write
 *
 */
#define HW_CHARGER_REG_SETF(reg, field, new_val) \
        ANAMISC->CHARGER_##reg##_REG = ((ANAMISC->CHARGER_##reg##_REG & ~(ANAMISC_CHARGER_##reg##_REG_##field##_Msk)) | \
        ((ANAMISC_CHARGER_##reg##_REG_##field##_Msk) & ((new_val) << (ANAMISC_CHARGER_##reg##_REG_##field##_Pos))))


/**
 * \brief Check if VBUS power is available.
 *
 * \return True, if a source is connected to VBUS, else false.
 *
 */
bool hw_charger_check_vbus(void);

 /**
  * \brief Check if VBAT power is available.
  *
  * \return True, if a battery is connected, else false.
  *
  */
__STATIC_INLINE bool hw_charger_check_vbat(void)
{
        return (REG_GETF(CRG_TOP, ANA_STATUS_REG, COMP_VBAT_OK) == 1);
}

/**
 * \brief Program VBUS IRQ to hit when the VBUS level goes from low to high.
 *
 */
__STATIC_INLINE void hw_charger_set_vbus_irq_high(void)
{
        CRG_TOP->VBUS_IRQ_MASK_REG = CRG_TOP_VBUS_IRQ_MASK_REG_VBUS_IRQ_EN_RISE_Msk;
}

/**
 * \brief Program VBUS IRQ to hit when the VBUS level goes from high to low.
 *
 */
__STATIC_INLINE void hw_charger_set_vbus_irq_low(void)
{
        CRG_TOP->VBUS_IRQ_MASK_REG = CRG_TOP_VBUS_IRQ_MASK_REG_VBUS_IRQ_EN_FALL_Msk;
}

/**
 * \brief Mask VBUS IRQ so that it does not hit when the VBUS level changes.
 *
 */
__STATIC_INLINE void hw_charger_mask_vbus_irq(void)
{
        CRG_TOP->VBUS_IRQ_MASK_REG = 0;
}

/**
 * \brief Enable VBUS IRQ.
 *
 */
void hw_charger_enable_vbus_irq(void);

/**
 * \brief Disable VBUS IRQ.
 *
 */
__STATIC_INLINE void hw_charger_disable_vbus_irq(void)
{
        NVIC_DisableIRQ(VBUS_IRQn);
}

/**
 * \brief Clear VBUS IRQ.
 *
 */
__STATIC_INLINE void hw_charger_clear_vbus_irq(void)
{
        CRG_TOP->VBUS_IRQ_CLEAR_REG = 0x3;      /* Clear both interrupts */
}

/**
 * \brief Configure USB GPIOs.
 *
 */
void hw_charger_configure_usb_pins(void);

/**
 * \brief Enable Charger IRQ.
 *
 */
void hw_charger_enable_charger_irq(void);

/**
 * \brief Disable Charger IRQ.
 *
 */
__STATIC_INLINE void hw_charger_disable_charger_irq(void)
{
        REG_CLR_BIT(USB, USB_MAMSK_REG, USB_M_CH_EV);
}

/**
 * \brief Get the status of the Charger IRQ and ACK it.
 *
 */
__STATIC_INLINE uint16_t hw_charger_get_charger_irq_status(void)
{
        return USB->USB_MAEV_REG;
}

/**
 * \brief Enables the USB pads without activating the pull-up.
 *
 */
__STATIC_INLINE void hw_charger_enable_usb_pads_passive(void)
{
        ASSERT_WARNING(GPIO->P11_MODE_REG == 0x026);       // Must be configured as a USB pad.
        ASSERT_WARNING(GPIO->P22_MODE_REG == 0x026);       // Must be configured as a USB pad.

        CRG_PER->USBPAD_REG = (CRG_PER_USBPAD_REG_USBPAD_EN_Msk |
                               CRG_PER_USBPAD_REG_USBPHY_FORCE_SW1_OFF_Msk);
}

/**
 * \brief Enables the USB pull-up on D+.
 *
 */
__STATIC_INLINE void hw_charger_enable_usb_pullup(void)
{
        CRG_PER->USBPAD_REG = CRG_PER_USBPAD_REG_USBPAD_EN_Msk;
}

/**
 * \brief Disables the USB pads.
 *
 */
__STATIC_INLINE void hw_charger_disable_usb_pads(void)
{
        CRG_PER->USBPAD_REG = 0;
}

/**
 * \brief Set the USB clock to PLL/2.
 *
 */
__STATIC_INLINE void hw_charger_setclk_pll(void)
{
        GLOBAL_INT_DISABLE();
        REG_CLR_BIT(CRG_TOP, CLK_CTRL_REG, USB_CLK_SRC);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Set the USB clock to AHB clock.
 *
 */
__STATIC_INLINE void hw_charger_setclk_ahb(void)
{
        GLOBAL_INT_DISABLE();
        REG_SET_BIT(CRG_TOP, CLK_CTRL_REG, USB_CLK_SRC);
        GLOBAL_INT_RESTORE();
}

/**
 * \brief Enables the USB port.
 *
 */
__STATIC_INLINE void hw_charger_enable_usb_node(void)
{
        hw_charger_setclk_ahb();

        USB->USB_MCTRL_REG = USB_USB_MCTRL_REG_USBEN_Msk;
        REG_SET_BIT(USB, USB_MCTRL_REG, USB_NAT);
}

/**
 * \brief Disables the USB port.
 *
 */
__STATIC_INLINE void hw_charger_disable_usb_node(void)
{
        USB->USB_MCTRL_REG = 0;

        hw_charger_setclk_pll();
}

/**
 * \brief Enable USB Charger detection circuit.
 *
 */
__STATIC_INLINE void hw_charger_enable_detection(void)
{
        REG_SET_BIT(USB, USB_CHARGER_CTRL_REG, USB_CHARGE_ON);
}

/**
 * \brief Disable USB Charger detection circuit.
 *
 */
__STATIC_INLINE void hw_charger_disable_detection(void)
{
        USB->USB_CHARGER_CTRL_REG = 0;
}

/**
 * \brief Enable USB Charger detection circuit and start contact detection.
 *
 */
__STATIC_INLINE void hw_charger_start_contact_detection(void)
{
        USB->USB_CHARGER_CTRL_REG = (USB_USB_CHARGER_CTRL_REG_USB_CHARGE_ON_Msk | 
                                     USB_USB_CHARGER_CTRL_REG_IDP_SRC_ON_Msk);
}

/**
 * \brief Enable USB Charger detection circuit and start primary detection.
 *
 */
__STATIC_INLINE void hw_charger_start_primary_detection(void)
{
        USB->USB_CHARGER_CTRL_REG = (USB_USB_CHARGER_CTRL_REG_USB_CHARGE_ON_Msk |
                                     USB_USB_CHARGER_CTRL_REG_VDP_SRC_ON_Msk |
                                     USB_USB_CHARGER_CTRL_REG_IDM_SINK_ON_Msk);
}

/**
 * \brief Enable USB Charger detection circuit and start secondary detection.
 *
 */
__STATIC_INLINE void hw_charger_start_secondary_detection(void)
{
        USB->USB_CHARGER_CTRL_REG = (USB_USB_CHARGER_CTRL_REG_USB_CHARGE_ON_Msk |
                                     USB_USB_CHARGER_CTRL_REG_VDM_SRC_ON_Msk |
                                     USB_USB_CHARGER_CTRL_REG_IDP_SINK_ON_Msk);
}

/**
 * \brief Enable USB Charger detection circuit and pull D+ high.
 *
 */
__STATIC_INLINE void hw_charger_set_dp_high(void)
{
        USB->USB_CHARGER_CTRL_REG = (USB_USB_CHARGER_CTRL_REG_USB_CHARGE_ON_Msk | 
                                     USB_USB_CHARGER_CTRL_REG_VDP_SRC_ON_Msk);
}

/**
 * \brief Enable USB Charger detection circuit but stop any detection (contact, primary or
 *        secondary).
 *
 */
__STATIC_INLINE void hw_charger_stop_any_detection(void)
{
        USB->USB_CHARGER_CTRL_REG = USB_USB_CHARGER_CTRL_REG_USB_CHARGE_ON_Msk;
}

/**
 * \brief Get USB Charger status and clear the USB_IRQn interrupt.
 *
 */
__STATIC_INLINE uint16_t hw_charger_get_status(void)
{
        return USB->USB_CHARGER_STAT_REG;
}

/**
 * \brief Check USB contact.
 *
 * \param[in] status charger status
 *
 * \return true if there is something connected, else false.
 *
 * \warning Must be called from the USB interrupt's call-back function only.
 *
 */
__STATIC_INLINE bool hw_charger_check_contact(uint16_t status)
{
        return !(status & USB_USB_CHARGER_STAT_REG_USB_DP_VAL_Msk);
}

/**
 * \brief Get USB Charger primary detection result.
 *
 * \return true if it is a Charging Downstream Port (CDP) or a Dedicated Charging Port (DCP), else
 *         false.
 *
 */
__STATIC_INLINE bool hw_charger_check_primary(void)
{
        return HW_USB_CHARGER_REG_GETF(STAT, USB_CHG_DET);
}

/**
 * \brief Get USB Charger secondary detection result.
 *
 * \return true if it is a Dedicated Charging Port (DCP), false if it is a Charging Downstream Port
 *         (CDP).
 *
 */
__STATIC_INLINE bool hw_charger_check_secondary(void)
{
        return HW_USB_CHARGER_REG_GETF(STAT, USB_DCP_DET);
}

/**
 * \brief Enable ext scale charging.
 *
 */
__STATIC_INLINE void hw_charger_enable_ext_charging(void)
{
        if (dg_configUSE_USB_CHARGER == 1) {
                HW_CHARGER_REG_SETF(CTRL2, CHARGER_TEST, 6);
        }
}

/**
 * \brief Enable normal charging.
 *
 */
__STATIC_INLINE void hw_charger_enable_normal_charging(void)
{
        if (dg_configUSE_USB_CHARGER == 1) {
                HW_CHARGER_REG_SETF(CTRL2, CHARGER_TEST, 0);
        }
}

/**
 * \brief Configures the Charger how to charge the battery.
 *
 * \details The default configuration is:
 *          -                    Charger status: disabled
 *          -                              NTC : user defined
 *          -                          NTC_LOW : stop at 7/8 VDD
 *          - Die temperature protection level : 80C
 *          -       Die temperature protection : enabled
 *          -                   Charge current : user defined
 *          -                   Charge voltage
 *                                    2 x NiMh : 3.40V
 *                                Li-phosphate : 3.60V
 *                           LiCo / LiMn / NMC : 4.20V
 *                                    3 x NiMh : 4.90V
 *                                      Custom : user defined
 *
 */
__STATIC_INLINE void hw_charger_configure(void)
{
        /* Make sure a rechargeable battery is selected */
        ASSERT_WARNING( (dg_configBATTERY_TYPE != BATTERY_TYPE_NO_RECHARGE) &&
                        (dg_configBATTERY_TYPE != BATTERY_TYPE_NO_BATTERY) );

        uint16_t reg;

#if (dg_configBATTERY_TYPE == BATTERY_TYPE_2xNIMH)
        reg = 0x1 << ANAMISC_CHARGER_CTRL1_REG_CHARGE_LEVEL_Pos;        // 3.40V
#elif (dg_configBATTERY_TYPE == BATTERY_TYPE_LIFEPO4)
        reg = 0x3 << ANAMISC_CHARGER_CTRL1_REG_CHARGE_LEVEL_Pos;        // 3.60V
#elif (dg_configBATTERY_TYPE == BATTERY_TYPE_LICOO2)
        reg = 0xA << ANAMISC_CHARGER_CTRL1_REG_CHARGE_LEVEL_Pos;        // 4.20V
#elif (dg_configBATTERY_TYPE == BATTERY_TYPE_LIMN2O4)
        reg = 0xA << ANAMISC_CHARGER_CTRL1_REG_CHARGE_LEVEL_Pos;        // 4.20V
#elif (dg_configBATTERY_TYPE == BATTERY_TYPE_NMC)
        reg = 0xA << ANAMISC_CHARGER_CTRL1_REG_CHARGE_LEVEL_Pos;        // 4.20V
#elif (dg_configBATTERY_TYPE == BATTERY_TYPE_LINICOAIO2)
        reg = 0xA << ANAMISC_CHARGER_CTRL1_REG_CHARGE_LEVEL_Pos;        // 4.20V
#elif (dg_configBATTERY_TYPE == BATTERY_TYPE_3xNIMH)
        reg = 0x11 << ANAMISC_CHARGER_CTRL1_REG_CHARGE_LEVEL_Pos;       // 4.90V
#elif (dg_configBATTERY_TYPE == BATTERY_TYPE_CUSTOM)
        reg = dg_configBATTERY_CHARGE_VOLTAGE << ANAMISC_CHARGER_CTRL1_REG_CHARGE_LEVEL_Pos;
#else
        reg = 0x0 << ANAMISC_CHARGER_CTRL1_REG_CHARGE_LEVEL_Pos;        // 3V
#endif
        reg |= (dg_configBATTERY_CHARGE_NTC << ANAMISC_CHARGER_CTRL1_REG_NTC_DISABLE_Pos);
        reg |= (dg_configBATTERY_CHARGE_CURRENT << ANAMISC_CHARGER_CTRL1_REG_CHARGE_CUR_Pos);
        reg |= (2 << ANAMISC_CHARGER_CTRL1_REG_DIE_TEMP_SET_Pos);

        ANAMISC->CHARGER_CTRL1_REG = reg;

        sys_tcs_apply(tcs_charger);

        if (dg_configBATTERY_CHARGE_NTC == 0) {
                /* Set P14 to output high (3.3V). */
                GPIO->P1_PADPWR_CTRL_REG &= ~(1 << 4);
                GPIO->P1_SET_DATA_REG = (1 << 4);
                GPIO->P14_MODE_REG = 0x300;

                /* Set P16 to input, no pull-up or pull-down. */
                GPIO->P16_MODE_REG = 0x0;
        }
}

/**
 * \brief Set battery charging current (if different from dg_configBATTERY_CHARGE_CURRENT).
 *
 * \param[in] current The pre-charging current level.
 *
 */
__STATIC_INLINE void hw_charger_set_charge_current(uint8_t current)
{
        /* The current cannot exceed 30 */
        ASSERT_WARNING(current < 30);

        if (dg_configUSE_USB_CHARGER == 1) {
                if (current > 15) {
                        HW_CHARGER_REG_SETF(CTRL1, CHARGE_CUR, current - 16);
                        hw_charger_enable_ext_charging();
                }
                else {
                        HW_CHARGER_REG_SETF(CTRL1, CHARGE_CUR, current);
                        hw_charger_enable_normal_charging();
                }
        }
}

/**
 * \brief Start battery charging.
 *
 */
__STATIC_INLINE void hw_charger_start_charging(void)
{
        if (dg_configUSE_USB_CHARGER == 1) {
                REG_SET_BIT(ANAMISC, CHARGER_CTRL1_REG, CHARGE_ON);
        }
}

/**
 * \brief Stop battery charging.
 *
 */
__STATIC_INLINE void hw_charger_stop_charging(void)
{
        if (dg_configUSE_USB_CHARGER == 1) {
                REG_CLR_BIT(ANAMISC, CHARGER_CTRL1_REG, CHARGE_ON);
        }
}

/**
 * \brief Get battery charging status.
 *
 */
__STATIC_INLINE bool hw_charger_is_charging(void)
{
        return HW_CHARGER_REG_GETF(CTRL1, CHARGE_ON) == 1;
}

/**
 * \brief Check if charger is in CC mode.
 *
 */
__STATIC_INLINE bool hw_charger_in_cc_mode(void)
{
        return (HW_CHARGER_REG_GETF(STATUS, CHARGER_CC_MODE) == 1);
}

/**
 * \brief Check if charger is in CV mode.
 *
 */
__STATIC_INLINE bool hw_charger_in_cv_mode(void)
{
        return (HW_CHARGER_REG_GETF(STATUS, CHARGER_CV_MODE) == 1);
}

/**
 * \brief Enable trickle charging.
 *
 */
__STATIC_INLINE void hw_charger_enable_trickle_charging(void)
{
        if (dg_configUSE_USB_CHARGER == 1) {
                REG_SET_BIT(ANAMISC, CHARGER_CTRL1_REG, NTC_LOW_DISABLE);
        }
}

/**
 * \brief Disable trickle charging.
 *
 */
__STATIC_INLINE void hw_charger_disable_trickle_charging(void)
{
        if (dg_configUSE_USB_CHARGER == 1) {
                HW_CHARGER_REG_SETF(CTRL1, NTC_LOW_DISABLE, 0);
                REG_CLR_BIT(ANAMISC, CHARGER_CTRL1_REG, NTC_LOW_DISABLE);
        }
}

/**
 * \brief Check end-of-charge (Li-ion).
 *
 * \return bool True if end-of-charge, else false.
 *
 * \warning It returns true even when the USB cable is attached. Thus, in order to reach to a safe
 *        conclusion about the battery charge status, the USB state (attached / detached) must also
 *        be taken into account.
 */
__STATIC_INLINE bool hw_charger_end_of_charge(void)
{
        bool ret = false;

        // Check only in normal charging mode.
        if (HW_CHARGER_REG_GETF(CTRL2, CHARGER_TEST) == 0) {
                ret = HW_CHARGER_REG_GETF(STATUS, END_OF_CHARGE);
        }

        return ret;
}

/**
 * \brief Check battery low-temperature.
 *
 * \return bool True if battery temp is too low, else false.
 *
 */
__STATIC_INLINE bool hw_charger_temp_low(void)
{
        bool ret;

        if (dg_configBATTERY_CHARGE_NTC == 1) {
                ret = false;
        }
        else {
                ret = HW_CHARGER_REG_GETF(STATUS, CHARGER_BATTEMP_LOW);
        }

        return ret;
}

/**
 * \brief Check battery high-temperature.
 *
 * \return bool True if battery temp is too high, else false.
 *
 */
__STATIC_INLINE bool hw_charger_temp_high(void)
{
        bool ret;

        if (dg_configBATTERY_CHARGE_NTC == 1) {
                ret = false;
        }
        else {
                ret = HW_CHARGER_REG_GETF(STATUS, CHARGER_BATTEMP_HIGH);
        }

        return ret;
}

/**
 * \brief Check battery temperature.
 *
 * \return bool True if battery temp is ok, else false.
 *
 */
__STATIC_INLINE bool hw_charger_temp_ok(void)
{
        bool ret;

        if (dg_configBATTERY_CHARGE_NTC == 1) {
                ret = true;
        }
        else {
                ret = HW_CHARGER_REG_GETF(STATUS, CHARGER_BATTEMP_OK);
        }

        return ret;
}

/**
 * \brief Check if new battery has been inserted.
 *
 * \return bool True if the battery has been replaced, else false.
 *
 */
__STATIC_INLINE bool hw_charger_new_battery_detected(void)
{
        return REG_GETF(CRG_TOP, ANA_STATUS_REG, NEWBAT);
}

#endif /* dg_configUSE_HW_USB_CHARGER */

#endif /* HW_USB_CHARGER_H_ */
/**
\}
\}
\}
*/
