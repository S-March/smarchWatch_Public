/**
 ****************************************************************************************
 *
 * @file bootloaders_common.c
 *
 * @brief Common part for both bootloaders (BLE SUOTA Loader and Secure Boot)
 *
 * Copyright (C) 2017 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
#include "ad_nvms.h"
#include "hw_gpio.h"
#include "hw_uart.h"
#include "hw_timer1.h"
#include "sys_clock_mgr.h"
#include "suota.h"
#include "flash_partitions.h"
#include "bootloaders_common.h"

void periph_init(void)
{
#if   dg_configBLACK_ORCA_MB_REV == BLACK_ORCA_MB_REV_D
#       define UART_TX_PORT    HW_GPIO_PORT_1
#       define UART_TX_PIN     HW_GPIO_PIN_3
#       define UART_RX_PORT    HW_GPIO_PORT_2
#       define UART_RX_PIN     HW_GPIO_PIN_3
# else
#       error "Unknown value for dg_configBLACK_ORCA_MB_REV!"
#endif


#if LOADER_UART
#if LOADER_UART == 2
        hw_gpio_set_pin_function(UART_TX_PORT, UART_TX_PIN, HW_GPIO_MODE_OUTPUT,
                                                                        HW_GPIO_FUNC_UART2_TX);
        hw_gpio_set_pin_function(UART_RX_PORT, UART_RX_PIN, HW_GPIO_MODE_INPUT,
                                                                        HW_GPIO_FUNC_UART2_RX);
#else
        hw_gpio_set_pin_function(UART_TX_PORT, UART_TX_PIN, HW_GPIO_MODE_OUTPUT,
                                                                        HW_GPIO_FUNC_UART_TX);
        hw_gpio_set_pin_function(UART_RX_PORT, UART_RX_PIN, HW_GPIO_MODE_INPUT,
                                                                        HW_GPIO_FUNC_UART_RX);
#endif
#endif

#if (CFG_FORCE_SUOTA_GPIO == 1)
        hw_gpio_configure_pin(CFG_FORCE_SUOTA_GPIO_PORT, CFG_FORCE_SUOTA_GPIO_PIN,
                                                HW_GPIO_MODE_INPUT_PULLUP, HW_GPIO_FUNC_GPIO, 1);
#endif /* (CFG_FORCE_SUOTA_GPIO == 1) */
}

void periph_deinit(void)
{
#if LOADER_UART
        while (!hw_uart_is_tx_fifo_empty(CONFIG_RETARGET_UART)) {}

        /* Configure pins used for UART as input, since UART can be used on other pins in app */
        hw_gpio_set_pin_function(UART_TX_PORT, UART_TX_PIN, HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_GPIO);
        hw_gpio_set_pin_function(UART_RX_PORT, UART_RX_PIN, HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_GPIO);
#endif

#if (CFG_FORCE_SUOTA_GPIO == 1)
        hw_gpio_set_pin_function(CFG_FORCE_SUOTA_GPIO_PORT, CFG_FORCE_SUOTA_GPIO_PIN,
                                                                HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_GPIO);
#endif /* (CFG_FORCE_SUOTA_GPIO == 1) */

#ifdef OS_FREERTOS
        /* Timer1 is used for ticks in OS disable it for now */
        hw_timer1_disable();
        /* Restore initial clock settings */
        cm_sys_clk_set(sysclk_XTAL16M);
        hw_cpm_pll_sys_off();
#endif
}

void reboot(void)
{
        /* Reset platform */
        __disable_irq();
        REG_SETF(CRG_TOP, SYS_CTRL_REG, SW_RESET, 1);
}

bool image_ready(suota_1_1_image_header_t *header)
{
        /* Is header ready for update */
        if ((header->flags & SUOTA_1_1_IMAGE_FLAG_VALID) &&
                                header->signature[0] == SUOTA_1_1_IMAGE_HEADER_SIGNATURE_B1 &&
                                header->signature[1] == SUOTA_1_1_IMAGE_HEADER_SIGNATURE_B2) {
                return true;
        }
        return false;
}

bool image_sanity_check(const int32_t *image_address)
{
        /*
         * Test reset vector for sanity:
         * - greater than image address
         * - address is odd for THUMB instruction
         */
        if (image_address[1] < (int32_t) image_address || (image_address[1] & 1) == 0) {
                return false;
        }

        return true;
}
