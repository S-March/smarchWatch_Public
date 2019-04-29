
/*
 * Xtal_TRIM.h
 *
 *  Created on: Oct 02 , 2015
 *      Author: hmoons
 */

#ifndef XTAL_TRIM_H_
#define XTAL_TRIM_H_

	void delay(unsigned long dd);
	void led(char flag_ON);								// led P1_0
	void led_burst(char number);
	void print_dec_numbers(char Amount, long numbers);	// just for displaying numbers ... max 9,999,999
	void Setting_Trim(unsigned int Trim_Value);			// program new Trim_Value
	void nop(void);
	long calculations(long xx_input, long yy_input);
	long Clock_Read(char port_number);
	//long data_simulation(long x1);
	void print_dec_numbers(char Amount, long numbers);
	long Simulation(volatile long x);					// simulation graph: input = x (256 ... 2047)

	int auto_trim(char port_number);
	int main_part(void);								// main routine

	void enable_pll(void);								// setting of PLL

	long pulse_counter(void); 							// counting pulses during 500 msec

	int mpulse(int datareg, int shift_bit); 			// forward declaration

	int Pulse(void);	// assembly routine

#endif /* XTAL_TRIM_H_ */
