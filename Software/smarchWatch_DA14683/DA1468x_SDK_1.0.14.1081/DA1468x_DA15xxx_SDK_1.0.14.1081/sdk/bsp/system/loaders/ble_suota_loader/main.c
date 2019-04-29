/**
 ****************************************************************************************
 *
 * @file main.c
 *
 * @brief BLE SUOTA loader
 *
 * Copyright (C) 2016-2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
/* Standard includes. */
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "osal.h"
#include "ad_ble.h"
#include "hw_gpio.h"
#include "hw_uart.h"
#include "hw_timer1.h"
#include "hw_qspi.h"
#include "ble_mgr.h"
#include "sys_clock_mgr.h"
#include "sys_power_mgr.h"
#include "suota.h"
#include "dlg_suota.h"
#include "ad_nvms.h"
#include "ad_gpadc.h"
#include "sys_watchdog.h"
#include "flash_partitions.h"
#include "bootloaders_common.h"

/*
 * Buffer for sector needed during copy from one partition to the other.
 */
static uint8_t sector_buffer[FLASH_SECTOR_SIZE];

/* Task priorities */
#define mainBLE_SUOTA_TASK_PRIORITY              ( tskIDLE_PRIORITY + 1 )

#define NVMS_MINIMUM_STACK      768

static bool force_suota = false;

#if dg_configUSE_WDOG
INITIALISED_PRIVILEGED_DATA int8_t idle_task_wdog_id = -1;
#endif

/*
 * Perform any application specific hardware configuration.  The clocks,
 * memory, etc. are configured before main() is called.
 */
static void prvSetupHardware( void );
/*
 * Task functions .
 */
void ble_suota_task(void *params);

static bool valid_image(suota_1_1_image_header_t *header, nvms_t exec_part, uint32_t exec_location,
                                                                        bool force_crc_check)
{
        const uint8_t *mapped_ptr;
        uint32_t crc;

        if (header == NULL || exec_part == NULL) {
                return false;
        }

        /*
         * Check CRC can be forced by image (then on every start CRC will be checked)
         * If it is not forced it will be checked anyway before image is copied to executable
         * partition.
         */
        if (!force_crc_check && (0 == (header->flags & SUOTA_1_1_IMAGE_FLAG_FORCE_CRC))) {
                return true;
        }

        crc = ~0; /* Initial value of CRC prepared by mkimage */
        /*
         * Utilize QSPI memory mapping for CRC check, this way no additional buffer is needed.
         */
        if (header->code_size != ad_nvms_get_pointer(exec_part, exec_location, header->code_size,
                                                                (const void **) &mapped_ptr)) {
                return false;
        }
        TRACE("Checking image CRC.\r\n");
        crc = suota_update_crc(crc, mapped_ptr, header->code_size);
        crc ^= ~0; /* Final XOR */

        return crc == header->crc;
}

static bool update_image(suota_1_1_image_header_t *new_header, nvms_t update_part,
                                                        nvms_t exec_part, nvms_t header_part)
{
        size_t left;
        size_t src_offset;
        size_t dst_offset;
        suota_1_1_image_header_t header;
        bool exec_image_valid = false;

        /*
         * Erase header partition. New header will be written after executable is copied.
         */
        if (!ad_nvms_erase_region(header_part, 0, sizeof(suota_1_1_image_header_t))) {
                return false;
        }

        /*
         * Erase executable partition.
         */
        if (!ad_nvms_erase_region(exec_part, 0, new_header->code_size)) {
                return false;
        }

        left = new_header->code_size;   /* Whole image to copy */
        dst_offset = 0;                 /* Write from the beginning of executable partition */
        src_offset = SUOTA_IMAGE_HEADER_OFFSET + new_header->exec_location;

        while (left > 0) {
                size_t chunk = left > FLASH_SECTOR_SIZE ? FLASH_SECTOR_SIZE : left;

                ad_nvms_read(update_part, src_offset, sector_buffer, chunk);
                ad_nvms_write(exec_part, dst_offset, sector_buffer, chunk);

                left -= chunk;
                src_offset += chunk;
                dst_offset += chunk;
        }

        /*
         * Header is in different partition than executable.
         * Executable is a the beginning of partition, change location to 0.
         */
        header = *new_header;

        if (new_header->flags & SUOTA_1_1_IMAGE_FLAG_RETRY2) {
                new_header->flags ^= SUOTA_1_1_IMAGE_FLAG_RETRY2;
        } else if (new_header->flags & SUOTA_1_1_IMAGE_FLAG_RETRY1) {
                new_header->flags ^= SUOTA_1_1_IMAGE_FLAG_RETRY1;
        } else {
                new_header->signature[0] = 0;
                new_header->signature[0] = 1;
                new_header->flags &= ~SUOTA_1_1_IMAGE_FLAG_VALID;
        }

        exec_image_valid = valid_image(&header, exec_part, 0, true);
        if (exec_image_valid) {
                /*
                 * Write image header, so it can be used later and in subsequent reboots.
                 */
                ad_nvms_write(header_part, 0, (uint8_t *) &header, sizeof(header));
                /*
                 * Mark header from update partition as invalid since it will not be used any more.
                 */
                new_header->signature[0] = 0;
                new_header->signature[0] = 1;
                new_header->flags &= ~SUOTA_1_1_IMAGE_FLAG_VALID;
        }
        /*
         * Write header to update partition. It can be invalid header if update was ok.
         * If number of retries run out, this header will also be written so no further tries
         * with image in update partition will be performed.
         */
        ad_nvms_write(update_part, SUOTA_IMAGE_HEADER_OFFSET, (uint8_t *) new_header,
                                                                        sizeof(*new_header));

        if (!exec_image_valid) {
                /*
                 * New image is not valid. Reboot it can result in yet another try or with SUOTA.
                 */
                reboot();
        }
        return true;
}

void boot_application(void)
{
        nvms_t update_part;
        nvms_t exec_part;
        nvms_t header_part;
        int32_t *int_vector_table = (int32_t *) 0;
        int32_t *image_address;
        suota_1_1_image_header_t new_header = { {0} };
        suota_1_1_image_header_t current_header = { {0} };

        TRACE("\r\nBootloader started.\r\n");

        if (force_suota) {
                return;
        }

#if (CFG_FORCE_SUOTA_GPIO == 1)
        TRACE("Checking status of K1 Button..\r\n");
        if (!hw_gpio_get_pin_status(CFG_FORCE_SUOTA_GPIO_PORT, CFG_FORCE_SUOTA_GPIO_PIN)) {
                TRACE("K1 Button is pressed, starting SUOTA service without booting application.\r\n");
                return;
        }
#endif /* (CFG_FORCE_SUOTA_GPIO == 1) */

        update_part = ad_nvms_open(NVMS_FW_UPDATE_PART);
        exec_part = ad_nvms_open(NVMS_FW_EXEC_PART);
        header_part = ad_nvms_open(NVMS_IMAGE_HEADER_PART);

        TRACE("Checking for update image.\r\n");
        read_image_header(update_part, SUOTA_IMAGE_HEADER_OFFSET, &new_header);

        if (image_ready(&new_header)) {
                /* Check if there is valid image for update, check CRC */
                if (valid_image(&new_header, update_part, new_header.exec_location, true)) {
                        TRACE("Updating image.\r\n");
                        update_image(&new_header, update_part, exec_part, header_part);
                } else {
                        TRACE("New image invalid, erasing.\r\n");
                        /* Update image not good, just erase it and start whatever is there */
                        new_header.signature[0] = 0;
                        new_header.signature[1] = 0;
                        ad_nvms_write(update_part, SUOTA_IMAGE_HEADER_OFFSET,
                                                (uint8_t *) &new_header, sizeof(new_header));
                }
        }

        /*
         * Check if current image is valid, CRC can be forced by image header but it is not
         * forced here.
         */
        read_image_header(header_part, 0, &current_header);
        TRACE("Validating current image.\r\n");
        if (!valid_image(&current_header, exec_part, 0, false)) {
                TRACE("Current image invalid, starting SUOTA.\r\n");
                return;
        }

        /*
         * The following code assumes that code will be executed in QSPI cached mode.
         *
         * The binary image that is stored in the QSPI flash must be compiled for a specific
         * address, other than address 0x0 (or 0x8000000) since this is where the boot loader is
         * stored.
         * The binary images that are stored in the QSPI Flash, except for the boot loader image,
         * must not be modified in any way before flashed. No image header must be preceded. The
         * images start with the initial stack pointer and the reset handler and the rest of the
         * vector table and image code and data follow.
         * The complete vector table of the application image is copied from the image location
         * to the RAM.
         */
        if (256 != ad_nvms_get_pointer(exec_part, 0, 256, (const void **) &image_address)) {
                return;
        }

        /* Check sanity of image */
        if (!image_sanity_check(image_address)) {
                TRACE("Current executable insane, starting SUOTA.\r\n");
                return;
        }

        TRACE("Starting image at 0x%X, reset vector 0x%X.\r\n", (unsigned int) image_address,
                                                                (unsigned int) image_address[1]);
        /*
         * In OS environment some interrupt could already be enabled, disable all before
         * interrupt vectors are changed.
         */
        __disable_irq();

        /*
         * Copy interrupt vector table from image. We only care to copy the
         * address of the reset handler here, to perform software reset properly.
         * The actual VT copy using the correct shuffling value will be done
         * by the reset handler.
         * */
        memcpy(int_vector_table, image_address, 0x100);

        /*
         * If bootloader changed any configuration (GPIO, clocks) it should be uninitialized here
         */
        periph_deinit();

        /*
         * Reset platform
         */
        reboot();
        for (;;) {
        }
}

/**
 * @brief System Initialization and creation of the BLE task
 */
static void system_init( void *pvParameters )
{
        OS_TASK handle;

#if defined CONFIG_RETARGET
        extern void retarget_init(void);
#endif

        /* Prepare clocks. Note: cm_cpu_clk_set() and cm_sys_clk_set() can be called only from a
         * task since they will suspend the task until the XTAL16M has settled and, maybe, the PLL
         * is locked.
         */
        cm_sys_clk_init(sysclk_XTAL16M);
        cm_apb_set_clock_divider(apb_div1);
        cm_ahb_set_clock_divider(ahb_div1);
        cm_lp_clk_init();

        /*
         * Initialize platform watchdog
         */
        sys_watchdog_init();

#if dg_configUSE_WDOG
        // Register the Idle task first.
        idle_task_wdog_id = sys_watchdog_register(false);
        ASSERT_WARNING(idle_task_wdog_id != -1);
        sys_watchdog_configure_idle_id(idle_task_wdog_id);
#endif

        /* Set system clock */
        cm_sys_clk_set(sysclk_XTAL16M);

        /* Prepare the hardware to run this demo. */
        prvSetupHardware();

        /* init resources */
        resource_init();

#if defined CONFIG_RETARGET
        retarget_init();
#endif

        /* If this function returns, current executable is not valid, start SUOTA part */
        boot_application();

        /* Set the desired sleep mode. */
        pm_set_wakeup_mode(true);
        pm_set_sleep_mode(pm_mode_extended_sleep);

        /* Initialize BLE Manager */
        ble_mgr_init();

        /* Start the BLE SUOTA application task. */
        OS_TASK_CREATE("SUOTA",                         /* The text name assigned to the task, for
                                                           debug only; not used by the kernel. */
                       ble_suota_task,                  /* The function that implements the task. */
                       NULL,                            /* The parameter passed to the task. */
                       768,                             /* The number of bytes to allocate to the
                                                           stack of the task. */
                       mainBLE_SUOTA_TASK_PRIORITY,     /* The priority assigned to the task. */
                       handle);                         /* The task handle. */
        OS_ASSERT(handle);

        /* SysInit task is no longer needed */
        OS_TASK_DELETE(OS_GET_CURRENT_TASK());
}
/*-----------------------------------------------------------*/

/**
 * @brief Basic initialization and creation of the system initialization task.
 */
int main( void )
{
        OS_TASK handle;
        BaseType_t status;

        hw_qspi_set_div(HW_QSPI_DIV_1);
        cm_clk_init_low_level();                            /* Basic clock initializations. */

        /* Start SysInit task. */
        status = OS_TASK_CREATE("SysInit",                /* The text name assigned to the task, for
                                                             debug only; not used by the kernel. */
                                system_init,              /* The System Initialization task. */
                                ( void * ) 0,             /* The parameter passed to the task. */
                                1024,                     /* The number of bytes to allocate to the
                                                             stack of the task. */
                                configMAX_PRIORITIES - 1, /* The priority assigned to the task. */
                                handle );                 /* The task handle */
        configASSERT(status == OS_TASK_CREATE_SUCCESS);

        /* Start the tasks and timer running. */
        vTaskStartScheduler();

        /* If all is well, the scheduler will now be running, and the following
        line will never be reached.  If the following line does execute, then
        there was insufficient FreeRTOS heap memory available for the idle and/or
        timer tasks     to be created.  See the memory management section on the
        FreeRTOS web site for more details. */
        for ( ;; );
}

static void prvSetupHardware( void )
{
        /* Init hardware */
        pm_system_init(periph_init);
}

/**
 * @brief Malloc fail hook
 */
void vApplicationMallocFailedHook( void )
{
        /* vApplicationMallocFailedHook() will only be called if
        configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
        function that will get called if a call to pvPortMalloc() fails.
        pvPortMalloc() is called internally by the kernel whenever a task, queue,
        timer or semaphore is created.  It is also called by various parts of the
        demo application.  If heap_1.c or heap_2.c are used, then the size of the
        heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
        FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
        to query the size of free heap space that remains (although it does not
        provide information on how the remaining heap might be fragmented). */
        taskDISABLE_INTERRUPTS();
        for ( ;; );
}

/**
 * @brief Application idle task hook
 */
void vApplicationIdleHook( void )
{
        /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
           to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
           task. It is essential that code added to this hook function never attempts
           to block in any way (for example, call xQueueReceive() with a block time
           specified, or call vTaskDelay()).  If the application makes use of the
           vTaskDelete() API function (as this demo application does) then it is also
           important that vApplicationIdleHook() is permitted to return to its calling
           function, because it is the responsibility of the idle task to clean up
           memory allocated by the kernel to any task that has since been deleted. */

#if dg_configUSE_WDOG
        sys_watchdog_notify(idle_task_wdog_id);
#endif
}

/**
 * @brief Application stack overflow hook
 */
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
        ( void ) pcTaskName;
        ( void ) pxTask;

        /* Run time stack overflow checking is performed if
        configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
        function is called if a stack overflow is detected. */
        taskDISABLE_INTERRUPTS();
        for ( ;; );
}

/**
 * @brief Application tick hook
 */
void vApplicationTickHook( void )
{

        OS_POISON_AREA_CHECK( OS_POISON_ON_ERROR_HALT, result );

}
