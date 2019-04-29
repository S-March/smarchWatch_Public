/**
 ****************************************************************************************
 *
 * @file main.c
 *
 * @brief ANCS application
 *
 * Copyright (C) 2015-2018 Dialog Semiconductor.
 * This computer program includes Confidential, Proprietary Information
 * of Dialog Semiconductor. All Rights Reserved.
 *
 ****************************************************************************************
 */
/* Standard includes. */
#include <string.h>
#include <stdbool.h>

#include "osal.h"
#include "resmgmt.h"
#include "ad_ble.h"
#include "ad_spi.h"
#include "ad_nvms.h"
#include "ble_mgr.h"
#include "hw_gpio.h"
#include "hw_wkup.h"
#include "sys_clock_mgr.h"
#include "sys_power_mgr.h"
#include "sys_watchdog.h"
#include "ancs_config.h"
#include "platform_devices.h"

/* Task priorities */
#define mainBLE_ANCS_TASK_PRIORITY              ( OS_TASK_PRIORITY_NORMAL )
#define mainDISPLAY_TASK_PRIORITY               ( OS_TASK_PRIORITY_NORMAL )
#define mainFLASH_TASK_PRIORITY                 ( OS_TASK_PRIORITY_NORMAL )

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
#if LOAD_NEW_IMAGES
        void storeInFlash_task(void *params);
#else
        void ancs_task(void *params);
        void display_task(void *params);
#endif

static OS_TASK handle;

/*
 * Wakeup handler
 */
void ancs_client_wkup_handler(void);

static void wkup_handler()
{
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
        hw_wkup_reset_counter();
#endif
        hw_wkup_reset_interrupt();

        ancs_client_wkup_handler();
}

static void init_wakeup(void)
{
        hw_wkup_init(NULL);
        hw_wkup_configure_pin(CFG_TRIGGER_PERFORM_NOTIF_ACTION_GPIO_PORT,
                                                        CFG_TRIGGER_PERFORM_NOTIF_ACTION_GPIO_PIN, 1,
                                                        HW_WKUP_PIN_STATE_LOW);
#if dg_configBLACK_ORCA_IC_REV == BLACK_ORCA_IC_REV_A
        hw_wkup_set_counter_threshold(1);
#endif
        hw_wkup_set_debounce_time(10);

        hw_wkup_register_interrupt(wkup_handler, 1);
}

/**
 * @brief System Initialization and creation of the BLE task
 */
static void system_init( void *pvParameters )
{
#if defined CONFIG_RETARGET
        extern void retarget_init(void);
#endif

        /* Prepare clocks. Note: cm_cpu_clk_set() and cm_sys_clk_set() can be called only from a
         * task since they will suspend the task until the XTAL16M has settled and, maybe, the PLL
         * is locked.
         */
        cm_sys_clk_init(sysclk_XTAL16M);
//        cm_sys_clk_init(sysclk_PLL96);
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
//        cm_sys_clk_set(sysclk_XTAL16M);
        cm_sys_clk_set(sysclk_PLL96);

        /* Prepare the hardware to run this demo. */
        prvSetupHardware();

        /* init resources */
        resource_init();

#if defined CONFIG_RETARGET
        retarget_init();
#endif

        /* Initialize wakeup GPIO pins used in demo */
        init_wakeup();

        /* Set the desired sleep mode. */
        pm_set_wakeup_mode(true);

        #if LOAD_NEW_IMAGES
                pm_set_sleep_mode(pm_mode_active);
                /* Start the Flash Storage application task. */
                OS_TASK_CREATE("Flash Storage Task",                     /* The text name assigned to the task, for
                                                                      debug only; not used by the kernel. */
                               storeInFlash_task,                          /* The function that implements the task. */
                               NULL,                               /* The parameter passed to the task. */
                               4096,                               /* The number of bytes to allocate to the
                                                                      stack of the task. */
                               mainFLASH_TASK_PRIORITY,         /* The priority assigned to the task. */
                               handle);                            /* The task handle. */
                OS_ASSERT(handle);
        #else
                pm_set_sleep_mode(pm_mode_extended_sleep);
                /* Initialize BLE Manager */
                ble_mgr_init();

                /* Start the Display application task. */
                OS_TASK_CREATE("Display Task",                     /* The text name assigned to the task, for
                                                                      debug only; not used by the kernel. */
                               display_task,                          /* The function that implements the task. */
                               NULL,                               /* The parameter passed to the task. */
                               4096,                               /* The number of bytes to allocate to the
                                                                      stack of the task. */
                               mainDISPLAY_TASK_PRIORITY,         /* The priority assigned to the task. */
                               handle);                            /* The task handle. */
                OS_ASSERT(handle);

                /* Start the ANCS Profile application task. */
                OS_TASK_CREATE("ANCS Profile",                     /* The text name assigned to the task, for
                                                                      debug only; not used by the kernel. */
                               ancs_task,                          /* The function that implements the task. */
                               NULL,                               /* The parameter passed to the task. */
                               1024,                               /* The number of bytes to allocate to the
                                                                      stack of the task. */
                               mainBLE_ANCS_TASK_PRIORITY,         /* The priority assigned to the task. */
                               handle);                            /* The task handle. */
                OS_ASSERT(handle);
        #endif


        /* the work of the SysInit task is done */
        OS_TASK_DELETE(OS_GET_CURRENT_TASK());
}
/*-----------------------------------------------------------*/

/**
 * @brief Basic initialization and creation of the system initialization task.
 */
int main( void )
{
        OS_BASE_TYPE status;
        cm_clk_init_low_level();                            /* Basic clock initializations. */

        /* Start SysInit task. */
        status = OS_TASK_CREATE("SysInit",                /* The text name assigned to the task, for
                                                             debug only; not used by the kernel. */
                                system_init,              /* The System Initialization task. */
                                ( void * ) 0,             /* The parameter passed to the task. */
                                1024,                     /* The number of bytes to allocate to the
                                                             stack of the task. */
                                OS_TASK_PRIORITY_HIGHEST, /* The priority assigned to the task. */
                                handle );                 /* The task handle */
        OS_ASSERT(status == OS_TASK_CREATE_SUCCESS);

        /* Start the tasks and timer running. */
        vTaskStartScheduler();

        /* If all is well, the scheduler will now be running, and the following
        line will never be reached.  If the following line does execute, then
        there was insufficient FreeRTOS heap memory available for the idle and/or
        timer tasks     to be created.  See the memory management section on the
        FreeRTOS web site for more details. */
        for( ;; );
}

/**
 * \brief Initialize the peripherals domain after power-up.
 *
 */
static void periph_init(void)
{
        //UART
        hw_gpio_set_pin_function(HW_GPIO_PORT_1, HW_GPIO_PIN_3, HW_GPIO_MODE_OUTPUT,
                HW_GPIO_FUNC_UART2_TX);
        hw_gpio_set_pin_function(HW_GPIO_PORT_2, HW_GPIO_PIN_3, HW_GPIO_MODE_OUTPUT,
                HW_GPIO_FUNC_UART2_RX);

        //SPI
        hw_gpio_set_pin_function(HW_GPIO_PORT_3, HW_GPIO_PIN_3, HW_GPIO_MODE_OUTPUT,
                HW_GPIO_FUNC_SPI_CLK);
        hw_gpio_set_pin_function(HW_GPIO_PORT_3, HW_GPIO_PIN_0, HW_GPIO_MODE_INPUT,
                HW_GPIO_FUNC_SPI_DI);
        hw_gpio_set_pin_function(HW_GPIO_PORT_4, HW_GPIO_PIN_6, HW_GPIO_MODE_OUTPUT,
                HW_GPIO_FUNC_SPI_DO);
        hw_gpio_set_pin_function(HW_GPIO_PORT_4, HW_GPIO_PIN_5, HW_GPIO_MODE_OUTPUT,
                HW_GPIO_FUNC_SPI_EN);
        hw_gpio_set_active(HW_GPIO_PORT_4,HW_GPIO_PIN_5);

        //Display Reset
        hw_gpio_set_pin_function(HW_GPIO_PORT_4, HW_GPIO_PIN_7, HW_GPIO_MODE_OUTPUT,
                HW_GPIO_FUNC_GPIO);
        hw_gpio_set_active(HW_GPIO_PORT_4,HW_GPIO_PIN_7);

        //BLE TURN DOWN CALL
        hw_gpio_configure_pin(CFG_TRIGGER_PERFORM_NOTIF_ACTION_GPIO_PORT,
                        CFG_TRIGGER_PERFORM_NOTIF_ACTION_GPIO_PIN, HW_GPIO_MODE_INPUT_PULLUP,
                        HW_GPIO_FUNC_GPIO, 1);
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
        for( ;; );
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
void vApplicationStackOverflowHook( OS_TASK pxTask, char *pcTaskName )
{
        ( void ) pcTaskName;
        ( void ) pxTask;

        /* Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
        taskDISABLE_INTERRUPTS();
        for( ;; );
}

/**
 * @brief Application tick hook
 */
void vApplicationTickHook( void )
{

        OS_POISON_AREA_CHECK( OS_POISON_ON_ERROR_HALT, result );

}
