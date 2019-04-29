/*
 * Pulse.s
 *
 *  Created on: Sep 2, 2015
 *      Author: hmoons
 */


/*
 * systick.s
 *
 *  Created on: Aug 26, 2015
 *      Author: pp
 */

.syntax	unified
.arch	armv6-m

.equ GUARD_COUNTER_INIT, 3000000 	@ 2000000 iterations x 9 cycles/iteration * 62.5 ns/cycle = 1.125 seconds
//GUARD_COUNTER_INIT	EQU 3000000 	@ 2000000 iterations x 9 cycles/iteration * 62.5 ns/cycle = 1.125 seconds

//SYSTICK_CTRL_ADDR   EQU 0xE000E010 @ (R/W)  SysTick Control and Status Register
//SYSTICK_LOAD_ADDR   EQU 0xE000E014 @ (R/W)  SysTick Reload Value Register
//SYSTICK_VAL_ADDR    EQU 0xE000E018 @ (R/W)  SysTick Current Value Register
//SYSTICK_CALIB_ADDR  EQU 0xE000E01C @ (R/ )  SysTick Calibration Register

.equ SYSTICK_CTRL_ADDR, 0xE000E010 	@ (R/W)  SysTick Control and Status Register
.equ SYSTICK_LOAD_ADDR, 0xE000E014 	@ (R/W)  SysTick Reload Value Register
.equ SYSTICK_VAL_ADDR, 0xE000E018 	@ (R/W)  SysTick Current Value Register
.equ SYSTICK_CALIB_ADDR, 0xE000E01C @ (R/ )  SysTick Calibration Register

	.thumb
                .text

.global MEASURE_PULSE
.type MEASURE_PULSE,function
.thumb_func
@ measure_pulse   PROC
@                 EXPORT  measure_pulse
MEASURE_PULSE:
                push {r1-r5, lr}

                LDR      r5, =GUARD_COUNTER_INIT @ guard counter initialization
wait_low:
                SUBS     r5, r5, #1              @ decrement guard counter -> Z=1 at zero crossing
                BEQ      abort                   @ abort if guard counter has reached zero
                LDRH     r2,[r0]
                TST      r2, r1
                BNE      wait_low

                @ detect positive edge
wait_high:
                SUBS     r5, r5, #1              @ decrement guard counter (gc--)
                BEQ      abort                   @ abort if guard counter has reached zero
                LDRH     r2, [r0]
                TST      r2, r1
                BEQ      wait_high

                @ start systick timer
                LDR      r2, =SYSTICK_CTRL_ADDR
                LDR      r3, [r2]
                MOVS     r4, #1
                ORRS     r3,r3,r4
                STR      r3, [r2]

                @ detect negative edge
while_high:
                SUBS     r5, r5, #1              @ decrement guard counter
                BEQ      abort                   @ abort if guard counter has reached zero
                LDRH     r2,[r0]
                TST      r2, r1
                BNE      while_high

                @ read and return current systick value
                LDR      r2, =SYSTICK_VAL_ADDR
                LDR      r0, [r2]
                POP      {r1-r5, pc}

abort:
                MOVS    r0, #0
                POP     {r1-r5, pc}

