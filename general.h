/*
 * general.h
 *
 *  Created on: 27.05.2016
 *      Author: admin
 */

#ifndef GENERAL_H_
#define GENERAL_H_

//CPU configuration
#define CLOCKRATE 8000000 //8Mhz
#define CLOCKCYCLES_ONE_MS 8000

extern unsigned int esp_tcp_state_update_timer;

void wait_ms(const int ms);
void fillStackWithPattern();

#endif /* GENERAL_H_ */
