/*
 * Xtal_TRIM.c
 *
 *  Created on: update 20151020
 *      Author: hmoons
 */

#include <stdio.h>
#include "sdk_defs.h"
#include <hw_gpio.h>
#include <stdbool.h>
#include <stdint.h>
#include "Xtal_TRIM.h"
#include "sys_clock_mgr.h"
#include "sys_power_mgr.h"

// ***

// Remark: {***} = can be deleted when no debugging is needed

#define debugging                       0                                                       // for testing purposes

#define TEMP_OFFSET             0                                       // 40 = 5 ppm {8M}
#define IDEAL_CNT                       8000000 - 4 + TEMP_OFFSET       // 500 msec  x = 1148
//#define IDEAL_CNT             4800000-4                                       // 300 msec  x = 1148

#define DEFAULT_TRIM            1148                                            // ideal TRIM value (1148)
#define ACCURACY_STEP           9                                                       // using the SYSTICK: accuracy is 7 clocks

#define DELAY_1MSEC                     1777                                            // delay x * 1 msec
#define MAX_LOOP                        10                                                      // loop_counter

#define TRIM_MIN                        256                                                     // minimum TRIM value
#define TRIM_MAX                        2047                                            // maximum TRIM value

#define PPM_1                           10                                                      //      1 ppm {8M}
#define PPM_2                           20                                                      //      2 ppm {8M}

#define PPM_BOUNDARY            40                                                      //      5 ppm {8M}

#define Number_loop_calc        3                                                       // calculations of TRIM to XTAL-clock
#define Max_loop_calc           1791                                            // 2047 - 256

// variables

long x_calc, y_input, x_output;
long delta_x_input;
long ccc;

int loop_calc;
int loop_counter;

#if debugging
long log_loop[11];      // for testing {***}
long log_calc[11];
#endif

//( x - 4 ) -9 <= c <=  ( x - 4 ) +9
//( 8,000,000 - 4 ) -9 <= c <=  ( 8,000,000 - 4 ) +9
// ideal = 8,000,000 - 4 = 7,999,996 + temp-offset

volatile long actual_trimming_value;
long new_trimming_value;

extern uint32_t MEASURE_PULSE(uint32_t, char);

// *** delay routine x * 1 msec
void delay(unsigned long dd)            //
{
        unsigned long j, jj;
        jj = dd * DELAY_1MSEC;
        for (j = 1; j <= jj; j++)
        {
                __NOP();
                __NOP();
        }
}

#if debugging

void led(char flag_ON)  // test-output P1_2
{
        if (flag_ON == 1)       hw_gpio_set_active(1, 2);
        else                            hw_gpio_set_inactive(1, 2);
}

#endif

#if debugging

void led_burst(char number)
{
        char t;

        for (t=1; t<=number; t++)
        {
                led(1);
                led(0);
        }
}

#endif

// *** boundary testing new TRIM-value and storing in CLK_FREQ_TRIM_REG
void Setting_Trim(unsigned int Trim_Value)      // program new Trim_Value
{
        if (Trim_Value < TRIM_MIN && Trim_Value != 0)   Trim_Value = TRIM_MIN;
        if (Trim_Value > TRIM_MAX)                                              Trim_Value = TRIM_MAX;

        CRG_TOP->CLK_FREQ_TRIM_REG = Trim_Value; //Trim_Value;

        delay(2);       // delay = 2msec
}

/**
 ******************************************************************************************
 * @brief Measures the high duration of an externally applied square pulse in system ticks.
 *
 * @param[in] port_number    GPIO where the square pulse is applied on.
 * @return   Zero if no external pulse is detected on the given GPIO.
 *           Otherwise the pulse high duration in system ticks is returned.
 ******************************************************************************************
 */
long Clock_Read(char port_number) // testing block wave via input e.g. P0_6 ... port can be selected
{
        long ccc1 = 0;
        char port_number_10, port_number_1; // 10th and 1th e.g. 2 and 3 => port P2_3

        char shift_bit;
        uint32_t datareg = 0xFFFFFFFF; // initialize to an invalid address

        uint32_t tick_counter = 0;

        port_number_10 = port_number / 10;
        port_number_1 = port_number % 10;

        shift_bit = (1 << port_number_1);

        switch (port_number_10)
        {
                case 0:
                        datareg = (uint32_t) (&(GPIO->P0_DATA_REG));
                        break;
                case 1:
                        datareg = (uint32_t) (&(GPIO->P1_DATA_REG));
                        break;
                case 2:
                        datareg = (uint32_t) (&(GPIO->P2_DATA_REG));
                        break;
                case 3:
                        datareg = (uint32_t) (&(GPIO->P3_DATA_REG));
                        break;
                case 4:
                        datareg = (uint32_t) (&(GPIO->P4_DATA_REG));
                        break;
        }

        // configure systick timer
        SysTick->LOAD = 0xFFFFFF;
        SysTick->VAL = 0;

        SysTick->CTRL |= 0x04;  // use processor-clock

#if debugging
        led_burst(10);  // testing
#endif

        // during counting, no interrupts should appear
        GLOBAL_INT_DISABLE(); // disable interrupts

        // tick_counter is result after 500 msec pulse (appr 8 000 000 counts)
        // tick_counter =  MEASURE_PULSE(&(GPIO->P4_DATA_REG), 4);      // *AK*  bit-mask
        tick_counter =  MEASURE_PULSE(datareg, shift_bit);

        SysTick->CTRL &= ~(0x01);               // stop systick timer ... bit 0: ENABLE

        GLOBAL_INT_RESTORE(); // enable interrupts

#if debugging
        led_burst(5);   // testing
#endif

        if (tick_counter == 0xFFFFFF)
                ccc1 = 0;
        else
                ccc1 = 0xFFFFFF - tick_counter; //ccc1 = 0xFFFFFF - tick_counter;

        return ccc1;
}

// *** algorithm for standard graph
long Simulation(volatile long x)// simulation graph: input = x (256 ... 2047)
{
        volatile float y_calc;
        volatile float aa, bb, cc;

        // y = 1.57235707E-07x3 - 3.81812218E-04x2 + 4.83639200E-01x + 4.79971690E+06 at 300 msec
        // to be defined

        // y = 1.64115303E-07x3 - 2.82201453E-04x2 + 3.65408640E-01x + 7.99970196E+06 at 500 msec
        aa = x * x * 1.64115303 / 10000;
        aa = aa * x / 1000;
        bb = x * x * 2.82201453 / 10000;
        cc = x * 3.65408640 / 10;
        y_calc = aa - bb + cc + 7999702;        //      ideal: y_calc = 8000000 at x = 1148
                                                                                //      y = f(x) => y = f(1148) = 8000000
        return ((long) y_calc);
}

// *** calculation for new best fit of TRIM
long calculations(long xx_input, long yy_input)
{
        long hy, dy;
        bool plus = false, minus = false;

        x_calc = xx_input;                      // e.g. 1148 at 8000000 counts
        y_input = yy_input;     // e.g. ideal value 4800000 at 300msec pulse or 8000000 at 500msec pulse

        hy = Simulation(x_calc);        // create start value
        dy = y_input - hy;                      // y = f(x) + dy

        loop_calc = 0;

        // estimated time to do calculation: appr. 49 msec for 90 iterations
        // iteration loops: changing step by step x_calc and re-calculate the result.
        // when the zero-line (= e.g. 8000000-4 counts) is crossed, system has new x_calc value

#if debugging
        led(1);
#endif

        do
        {
                loop_calc += Number_loop_calc;          // loop steps

                hy = Simulation(x_calc);
                hy = hy + dy;

                if (hy >= IDEAL_CNT)
                {
                        x_calc -= Number_loop_calc;             // e.g. steps of 3
                        minus = true;
                }

                if (hy <= IDEAL_CNT)
                {
                        x_calc += Number_loop_calc;             // e.g. steps of 3
                        plus = true;
                }
        }
        while (!(plus == true && minus == true) && loop_calc < Max_loop_calc);  //crossing the 0-line

#if debugging
        led(0);
#endif

#if debugging
        // testing log
        log_calc[loop_counter] = loop_calc / Number_loop_calc;
#endif

        if (loop_calc >= Max_loop_calc)
                return (-1);

        // 256 ... 2047

        if (x_calc < 256)
                x_calc = 256;
        if (x_calc > 2047)
                x_calc = 2047;

        return (x_calc);        // new TRIM-value is replied
}

// *** main function: port_number = input of 500 msec pulse
int auto_trim(char port_number)
{
        char pn10, pn01;

#if debugging
        char a;
        long ccc2;              // testing
        long ccc3;      // testing
        long ccc_temp;  // for testing
#endif

        pn10 = port_number / 10;
        pn01 = port_number % 10;

        // define port-number
        hw_gpio_set_pin_function(pn10, pn01, HW_GPIO_MODE_INPUT, HW_GPIO_FUNC_GPIO);    // *AK*

#if debugging
        hw_gpio_set_pin_function(1, 2, HW_GPIO_MODE_OUTPUT, HW_GPIO_FUNC_GPIO);                 // test-pin P1.2
#endif

#if debugging
jump:
#endif


#if debugging
        // for testing ... empty variables
        for (a=0; a<=10;a++)
        {
                log_loop[a] = 0;
                log_calc[a] = 0;
        }
#endif

        Setting_Trim(DEFAULT_TRIM);             // start TRIMming value

        ccc = Clock_Read(port_number);  // reading speed of DA14580

        if (ccc <= 7000000)                             // clock-count check ... no correct count was done
        {
                Setting_Trim(0);    // if no square pulse was detected, then TRIM = 0
                return (-2);        // no square pulse detected
        }

        loop_counter = 0;

        // iteration of calculating new TRIM, setting TRIM, Clock_Read and comparing ... {hm}
        do              // main loop
        {
                loop_counter++;

                // read TRIM register
                actual_trimming_value = CRG_TOP->CLK_FREQ_TRIM_REG;
                new_trimming_value = calculations(actual_trimming_value, ccc);  // calculate new calibration value

                if (new_trimming_value == -1)
                {
                        Setting_Trim(0);   // if no square pulse was detected, then TRIM = 0
                        return (-1);        // out of range
                }

                Setting_Trim(new_trimming_value);       // loading TRIM in register

                actual_trimming_value = CRG_TOP->CLK_FREQ_TRIM_REG;
                ccc = Clock_Read(port_number);  // reading speed of DA14580

                if (ccc == 0)
                {
                        Setting_Trim(0);   // if no square pulse was detected, then TRIM = 0
                        return (-2);        // no square pulse detected
                }

#if debugging
                // testing log
                log_loop[loop_counter] = ccc;
#endif

        }
        while (loop_counter <= MAX_LOOP && (ccc < (IDEAL_CNT - PPM_2) || ccc > (IDEAL_CNT + PPM_2))); // +- 20 {8M} = 2.5 ppm

        // when loop_counter overflow ... PPM_BOUNDARY = 5 ppm = 5 * 8 {8M} = 40 clock's
        if (ccc < (IDEAL_CNT - PPM_BOUNDARY) || ccc > (IDEAL_CNT + PPM_BOUNDARY))
        {
                Setting_Trim(0);        // if out of limits, then TRIM = 0
                return (-1);            // out of limits 5 ppm
        }

        // read TRIM register
        actual_trimming_value = CRG_TOP->CLK_FREQ_TRIM_REG;

//      goto jump;

#if debugging

        printf("\n\r * TRIM = %d", actual_trimming_value);
        printf(" * Freq = %d", ccc * 2);

        ccc_temp = ccc - IDEAL_CNT;     // calculate ppm-value
        ccc_temp = ccc_temp * 2;

        printf(" * Diff Freq = %d", ccc_temp);

        ccc_temp = ccc_temp / 16;

        printf(" * PPM = %d", ccc_temp);
        printf(" * loops = %d", loop_counter);
        printf(" * ");

#endif

#if debugging
goto jump;
#endif

        return (actual_trimming_value);
}
