/*
 * general.c
 *
 *  Created on: 03.06.2016
 *      Author: Manuel
 */

#include "general.h"

unsigned int esp_tcp_state_update_timer = 2;

void wait_ms(const int ms) {
    volatile uint16_t ms_counter = 0;
	while(ms_counter < ms) {
		//1ms delay
		__delay_cycles(CLOCKCYCLES_ONE_MS);
		ms_counter++;
	}
}


void fillStackWithPattern() {
	extern unsigned int _stack;
	extern unsigned int __STACK_END;
	unsigned int* p;
	//_stack = 0x0342;

	//p = &_stack;
	p = (void*)0x0312;
	while (p < (&__STACK_END -3))
	{
	    *p = 0xCAFE;
	    p++;
	}
}


