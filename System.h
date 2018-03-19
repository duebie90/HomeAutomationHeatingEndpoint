/*
 * System.h
 *
 *  Created on: 08.07.2016
 *      Author: admin
 */

#ifndef SYSTEM_H_
#define SYSTEM_H_
#include <msp430.h>
#include <stdbool.h>

//PORT 1
#define STATUS_LED	BIT4
#define STATUS_LED_STATE ((P1IN & BIT4) == BIT4)
#define STATUS_LED_TOGGLE P1OUT ^= BIT4
#define ALARM_OUT BIT5

//PORT 4
#define ESP_RST	BIT3


//Port 6
#define RELAY_FW_OUT	    BIT1
#define RELAY_FW_ON	 	    P6OUT |= BIT1
#define RELAY_FW_OFF		P6OUT &= ~BIT1
#define RELAY_FW_TOGGLE	    P6OUT ^= BIT1

#define RELAY_BW_OUT  	    BIT2
#define RELAY_BW_ON			P6OUT |= BIT2
#define RELAY_BW_OFF		P6OUT &= ~BIT2
#define RELAY_BW_TOGGLE		P6OUT ^= BIT2

//Temperatur Sensors: ADC Input numnbers
// BOILER SENSOR ADC INPUT 4 (P6.4)
#define BOILER_TEMP_SENSOR 4
#define BOILER_TEMP_SENSOR_PIN BIT4

// HEATING  FEED LINE SENSOR ADC INPUT 3 (P6.3)
#define HEATING_TEMP_SENSOR 3
#define HEATING_TEMP_SENSOR_PIN BIT3

// Endpoint Pins
#define RELAY_SOCKET_OUT        BIT1
#define RELAY_SOCKET_ON         P6OUT |= BIT1
#define RELAY_SOCKET_OFF        P6OUT &= ~BIT1
#define RELAY_SOCKET_TOGGLE     P6OUT ^= BIT1

#define RELAY_LIGHT_OUT         BIT2
#define RELAY_LIGHT_ON          P6OUT |= BIT2
#define RELAY_LIGHT_OFF         P6OUT &= ~BIT2
#define RELAY_LIGHT_TOGGLE      P6OUT ^= BIT2






//superior init routine
void initSystem();

void initClockSystem();

//setting GPIOs initial state and function
void initGPIO();

//UART0 is connected to the WLAN-Module
void initUart0();

//UART1 is connected to the FTDI USB bridge
void initUart1();

void initUart115KBaud();
void initUart38_4KBaud();

void initTimer0();

void init_adc_12();

//STATUS LED
void switchLed(bool state);


#endif /* SYSTEM_H_ */
