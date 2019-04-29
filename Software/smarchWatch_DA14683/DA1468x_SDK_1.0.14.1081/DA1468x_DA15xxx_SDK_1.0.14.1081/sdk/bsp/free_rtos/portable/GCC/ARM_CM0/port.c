/*
    FreeRTOS V8.2.0 - Copyright (C) 2015 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 8 spaces!
*/

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the ARM CM0 port.
 *----------------------------------------------------------*/

#include "sdk_defs.h"

/* Scheduler includes. */
#include "../../../../free_rtos/include/FreeRTOS.h"
#include "../../../../free_rtos/include/task.h"

#include "hw_timer1.h"
#include "hw_otpc.h"
#include "sys_clock_mgr.h"
#include "sys_power_mgr.h"


/* Constants required to manipulate the NVIC. */
#define portNVIC_SYSTICK_CTRL           ( ( volatile uint32_t *) 0xe000e010 )
#define portNVIC_SYSTICK_LOAD           ( ( volatile uint32_t *) 0xe000e014 )
#define portNVIC_INT_CTRL               ( ( volatile uint32_t *) 0xe000ed04 )
#define portNVIC_SYSPRI2                ( ( volatile uint32_t *) 0xe000ed20 )
#define portNVIC_SYSTICK_CLK            0x00000004
#define portNVIC_SYSTICK_INT            0x00000002
#define portNVIC_SYSTICK_ENABLE         0x00000001
#define portNVIC_PENDSVSET              0x10000000
#define portMIN_INTERRUPT_PRIORITY      ( 255UL )
#define portNVIC_PENDSV_PRI             ( portMIN_INTERRUPT_PRIORITY << 16UL )
#define portNVIC_SYSTICK_PRI            ( portMIN_INTERRUPT_PRIORITY << 24UL )

/* Constants required to set up the initial stack. */
#define portINITIAL_XPSR                ( 0x01000000 )

/* Let the user override the pre-loading of the initial LR with the address of
prvTaskExitError() in case is messes up unwinding of the stack in the
debugger. */
#ifdef configTASK_RETURN_ADDRESS
	#define portTASK_RETURN_ADDRESS	configTASK_RETURN_ADDRESS
#else
	#define portTASK_RETURN_ADDRESS	prvTaskExitError
#endif

/* Each task maintains its own interrupt status in the critical nesting
variable. */
static UBaseType_t uxCriticalNesting INITIALISED_PRIVILEGED_DATA = 0xaaaaaaaa;

/*
 * When calling vPortEnterCritical(), the interrupt enable/disable state (PRIMARK) is saved
 * here and restored when vPortExitCritical() is called.
 * Nested calls to vPortEnterCritical()/vPortExitCritical() don't touch it.
 */
static uint32_t prev_PRIMASK PRIVILEGED_DATA/* = 0*/;

/*
 * Setup the timer to generate the tick interrupts.
 */
static void prvSetupTimerInterrupt( void );

/*
 * Exception handlers.
 */
void xPortPendSVHandler( void ) __attribute__ (( naked ));
void xPortSysTickHandler( void );
void vPortSVCHandler( void );

/*
 * Start first task is a separate function so it can be tested in isolation.
 */
static void vPortStartFirstTask( void ) __attribute__ (( naked ));

/*
 * Used to catch tasks that attempt to return from their implementing function.
 */
static void prvTaskExitError( void );

/*-----------------------------------------------------------*/


/*
 * See header file for description.
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
        /* Simulate the stack frame as it would be created by a context switch
           interrupt. */
        pxTopOfStack--; /* Offset added to account for the way the MCU uses the stack on entry/exit of interrupts. */
        *pxTopOfStack = portINITIAL_XPSR;                       /* xPSR */
        pxTopOfStack--;
        *pxTopOfStack = ( StackType_t ) pxCode;                 /* PC */
        pxTopOfStack--;
        *pxTopOfStack = ( StackType_t ) portTASK_RETURN_ADDRESS;/* LR */
        pxTopOfStack -= 5;                                      /* R12, R3, R2 and R1. */
        *pxTopOfStack = ( StackType_t ) pvParameters;	        /* R0 */
        pxTopOfStack -= 8;                                      /* R11..R4. */

        return pxTopOfStack;
}
/*-----------------------------------------------------------*/

static void prvTaskExitError( void )
{
        /* A function that implements a task must not exit or attempt to return to
           its caller as there is nothing to return to.  If a task wants to exit it
           should instead call vTaskDelete( NULL ).

           Artificially force an assert() to be triggered if configASSERT() is
           defined, then stop here so application writers can catch the error. */
        configASSERT( uxCriticalNesting == ~0UL );
        portDISABLE_INTERRUPTS();
        for( ;; );
}
/*-----------------------------------------------------------*/

void vPortSVCHandler( void )
{
	/* This function is no longer used, but retained for backward
	compatibility. */
}
/*-----------------------------------------------------------*/

void vPortStartFirstTask( void )
{
        /* The MSP stack is not reset as, unlike on M3/4 parts, there is no vector
           table offset register that can be used to locate the initial stack value.
           Not all M0 parts have the application vector table at address 0. */
        __asm volatile(
        " ldr r2, pxCurrentTCBConst2      \n" /* Obtain location of pxCurrentTCB. */
        " ldr r3, [r2]                    \n"
        " ldr r0, [r3]                    \n" /* The first item in pxCurrentTCB is the task top of stack. */
        " add r0, #32                     \n" /* Discard everything up to r0. */
        " msr psp, r0                     \n" /* This is now the new top of stack to use in the task. */
        " movs r0, #2                     \n" /* Switch to the psp stack. */
        " msr CONTROL, r0                 \n"
        " pop {r0-r5}                     \n" /* Pop the registers that are saved automatically. */
        " mov lr, r5                      \n" /* lr is now in r5. */
        " cpsie i                         \n" /* The first task has its context and interrupts can be enabled. */
        " pop {pc}                        \n" /* Finally, pop the PC to jump to the user defined task code. */
        "                                 \n"
        " .align 2                        \n"
        "pxCurrentTCBConst2: .word pxCurrentTCB	  "
        );
}
/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
BaseType_t xPortStartScheduler( void )
{
        /* Make PendSV, CallSV and SysTick the same priority as the kernel. */
        *(portNVIC_SYSPRI2) |= portNVIC_PENDSV_PRI;
        *(portNVIC_SYSPRI2) |= portNVIC_SYSTICK_PRI;

        /* Start the timer that generates the tick ISR.  Interrupts are disabled
           here already. */
        prvSetupTimerInterrupt();

        /* Initialise the critical nesting count ready for the first task. */
        uxCriticalNesting = 0;

        /* Start the first task. */
        vPortStartFirstTask();

        /* Should never get here as the tasks will now be executing!  Call the task
           exit error function to prevent compiler warnings about a static function
           not being called in the case that the application writer overrides this
           functionality by defining configTASK_RETURN_ADDRESS. */
        prvTaskExitError();

        /* Should not get here! */
        return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
        /* Not implemented in ports where there is nothing to return to.
           Artificially force an assert. */
        configASSERT( uxCriticalNesting == 1000UL );
}
/*-----------------------------------------------------------*/

void vPortYield( void )
{
        /* Set a PendSV to request a context switch. */
        *( portNVIC_INT_CTRL ) = portNVIC_PENDSVSET;

        /* Barriers are normally not required but do ensure the code is completely
           within the specified behaviour for the architecture. */
        __asm volatile( "dsb" );
        __asm volatile( "isb" );
}
/*-----------------------------------------------------------*/

void vPortEnterCritical( void )
{
        if( uxCriticalNesting == 0 )
        {
                prev_PRIMASK = __get_PRIMASK();
        }
        portDISABLE_INTERRUPTS();
        uxCriticalNesting++;
        __asm volatile( "dsb" );
        __asm volatile( "isb" );
}
/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{
        configASSERT( uxCriticalNesting );
        uxCriticalNesting--;
        if( uxCriticalNesting == 0 )
        {
                if( prev_PRIMASK == 0 )
                {
                        portENABLE_INTERRUPTS();
                }
        }
}
/*-----------------------------------------------------------*/

#if (CMN_TIMING_DEBUG == 0)
uint32_t ulSetInterruptMaskFromISR( void )
{
        __asm volatile(
        " mrs r0, PRIMASK       \n"
        " cpsid i               \n"
        " bx lr                   "
        );

        /* To avoid compiler warnings.  This line will never be reached. */
        return 0;
}
/*-----------------------------------------------------------*/

void vClearInterruptMaskFromISR( uint32_t ulMask )
{
        __asm volatile(
        " msr PRIMASK, r0       \n"
        " bx lr                   "
        );

        /* Just to avoid compiler warning. */
        ( void ) ulMask;
}
#else
uint32_t ulSetInterruptMaskFromISR( void )
{
        uint32 primask = __get_PRIMASK();
        __set_PRIMASK(1);
        DBG_CONFIGURE_HIGH(CMN_TIMING_DEBUG, CMNDBG_CRITICAL_SECTION);
        return primask;
}
/*-----------------------------------------------------------*/

void vClearInterruptMaskFromISR( uint32_t ulMask )
{
        if (!ulMask) {
                DBG_CONFIGURE_LOW(CMN_TIMING_DEBUG, CMNDBG_CRITICAL_SECTION);
        }
        __set_PRIMASK(ulMask);
}
#endif
/*-----------------------------------------------------------*/

void xPortPendSVHandler( void )
{
	/* This is a naked function. */

        __asm volatile
        (
        " mrs r0, psp                           \n"
        "                                       \n"
        " ldr r3, pxCurrentTCBConst             \n" /* Get the location of the current TCB. */
        " ldr r2, [r3]                          \n"
        "                                       \n"
        " sub r0, r0, #32                       \n" /* Make space for the remaining low registers. */
        " str r0, [r2]                          \n" /* Save the new top of stack. */
        " stmia r0!, {r4-r7}                    \n" /* Store the low registers that are not saved automatically. */
        " mov r4, r8                            \n" /* Store the high registers. */
        " mov r5, r9                            \n"
        " mov r6, r10                           \n"
        " mov r7, r11                           \n"
        " stmia r0!, {r4-r7}                    \n"
        "                                       \n"
        " push {r3, r14}                        \n"
        " cpsid i                               \n"
        " bl vTaskSwitchContext                 \n"
        " cpsie i                               \n"
        " pop {r2, r3}                          \n" /* lr goes in r3. r2 now holds tcb pointer. */
        "                                       \n"
        " ldr r1, [r2]                          \n"
        " ldr r0, [r1]                          \n" /* The first item in pxCurrentTCB is the task top of stack. */
        " add r0, r0, #16                       \n" /* Move to the high registers. */
        " ldmia r0!, {r4-r7}                    \n" /* Pop the high registers. */
        " mov r8, r4                            \n"
        " mov r9, r5                            \n"
        " mov r10, r6                           \n"
        " mov r11, r7                           \n"
        "                                       \n"
        " msr psp, r0                           \n" /* Remember the new top of stack for the task. */
        "                                       \n"
        " sub r0, r0, #32                       \n" /* Go back for the low registers that are not automatically restored. */
        " ldmia r0!, {r4-r7}                    \n" /* Pop low registers.  */
        "                                       \n"
        " bx r3                                 \n"
        "                                       \n"
        " .align 2                              \n"
        " pxCurrentTCBConst: .word pxCurrentTCB	  "
        );
}
/*-----------------------------------------------------------*/

void xPortTickAdvance( void )
{
        /* Increment the RTOS tick. */
        if( xTaskIncrementTick() != pdFALSE )
        {
                /* Pend a context switch. */
                *(portNVIC_INT_CTRL) = portNVIC_PENDSVSET;
        }
}
/*-----------------------------------------------------------*/

/*
 * Setup the timer1 to generate the tick interrupts at the required frequency.
 */
void prvSetupTimerInterrupt( void )
{
        /* Configure Timer1 */
        NVIC_SetPriority(SWTIM1_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);
        hw_timer1_lp_clk_init();
        HW_TIMER1_SET_TRIGGER(TICK_PERIOD - 1, lp_last_trigger); // Set initial reload value
        lp_last_trigger = LP_CNT_MAX_VALUE;             // initial trigger is -1...
        hw_timer1_int_enable();                         // Enable interrupt
        hw_timer1_enable();                             // Start running
}
/*-----------------------------------------------------------*/

void prvSystemSleep( TickType_t xExpectedIdleTime )
{
        uint32_t ulSleepTime;
        eSleepModeStatus eSleepStatus;

        /* A simple WFI() is executed in any of the cases below:
         * 1. the system has just booted and the dg_configINITIAL_SLEEP_DELAY_TIME has not yet
         *    passed
         * 2. the XTAL32K is used as the LP clock, the system has just woke up after clockless
         *    sleep and the LP clock has not yet settled.
         */
        if( !cm_lp_clk_is_avail() ) {
                taskDISABLE_INTERRUPTS();

                /* Ensure it is still ok to enter the sleep mode. */
                eSleepStatus = eTaskConfirmSleepModeStatus();

                if( eSleepStatus == eAbortSleep ) {
                        taskENABLE_INTERRUPTS();
                        return;
                }

#if (dg_configDISABLE_BACKGROUND_FLASH_OPS == 1)
                /* Wait for an interrupt
                 *
                 * Any interrupt will cause an exit from WFI(). This is not a problem since even
                 * if an interrupt other that the tick interrupt occurs before the next tick comes,
                 * the only thing that should be done is to resume the scheduler. Since no tick has
                 * occurred, the OS time will be the same.
                 */
                __WFI();
                taskENABLE_INTERRUPTS();
#else
                pm_execute_active_wfi();
                taskENABLE_INTERRUPTS();

                /* Notify blocked tasks, if appropriate. */
                pm_process_completed_qspi_operations();
#endif

                return;
        }

        if (dg_configUSE_LP_CLK == LP_CLK_RCX) {
                // Update if a previous calibration was running and is finished.
                if (cm_rcx_calibration_is_on) {
                        if (cm_calibrate_rcx_update()) {
                                return;
                        }
                }
        }

        /*
         * Calculate the sleep time
         */
        ulSleepTime = pm_conv_ticks_2_prescaled_lpcycles(xExpectedIdleTime);

        /* Enter a critical section that will not effect interrupts bringing the MCU
         * out of sleep mode.
         */
        taskDISABLE_INTERRUPTS();

        DBG_CONFIGURE_LOW(CMN_TIMING_DEBUG, CMNDBG_CRITICAL_SECTION);

        DBG_SET_HIGH(CPM_USE_TIMING_DEBUG, CPMDBG_SLEEP_ENTER);

        /* Ensure it is still ok to enter the sleep mode. */
        eSleepStatus = eTaskConfirmSleepModeStatus();

        if( eSleepStatus == eAbortSleep ) {
                DBG_SET_LOW(CPM_USE_TIMING_DEBUG, CPMDBG_SLEEP_ENTER);

                /* A task has been moved out of the Blocked state since this macro was
                 * executed, or a context switch is being held pending. Do not enter a
                 * sleep state. Restart the tick and exit the critical section.
                 */
                taskENABLE_INTERRUPTS();
        }
        else {
#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
                uint32_t primask;
#endif

                if( eSleepStatus == eNoTasksWaitingTimeout )
                {
                        /* It is not necessary to configure an interrupt to bring the
                         * microcontroller out of its low power state at a fixed time in the
                         * future.
                         * Enter the low power state.
                         */
                        pm_sleep_enter( 0 );
                }
                else
                {
                        /* Configure an interrupt to bring the microcontroller out of its low
                         * power state at the time the kernel next needs to execute.
                         * Enter the low power state.
                         */
                        pm_sleep_enter( ulSleepTime );
                }

#if (dg_configIMAGE_SETUP == DEVELOPMENT_MODE)
                /* If the code stops at this point then the interrupts were enabled while they
                 * shouldn't be so.
                 */
                primask = __get_PRIMASK();
                ASSERT_WARNING(primask == 1);
#endif

                /* Wake-up! */
                pm_system_wake_up();

#if (dg_configDISABLE_BACKGROUND_FLASH_OPS == 0)
                /* Notify blocked tasks, if appropriate. */
                pm_process_completed_qspi_operations();
#endif
        }
}
/*-----------------------------------------------------------*/
