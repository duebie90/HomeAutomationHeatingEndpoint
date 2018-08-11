/*
 * System.c
 *
 *  Created on: 08.07.2016
 *      Author: Manuel
 *
 *      Used to initialize internals of the MSP430F149 µC
 *
 */
#include "System.h"


void initSystem() {
	/* Stop watchdog timer from timing out during initial start-up. */
	WDTCTL = WDTPW | WDTHOLD;

	initClockSystem();

	initGPIO();

	init_adc_12();

	initUart0();

	initUart1();

	initTimer0();
	//global interrupt enable
    //__bis_SR_register(GIE);
	_EINT();

}

void initClockSystem() {
#if defined (__MSP430F149__)
	//BCSCTL2/////////////////////////////////
	//SELM_0 -> MainClock source is internal DCO
	//DIVM_0 -> MainClock devider = 1
	//SSEL0  -> SubMainClock source is internal DCO
	//DIVS_0 -> SubMainClock devider is 1
	BCSCTL2 = 0x00;
	//BCSCTL2 = SELM_0 | DIVM_0 | DIVS_0; // &~SELS

	//TEST//
	BCSCTL2 |= SELM_0 | DIVM_0;
	//SMCLK ~ 8MHz
	//TEST END


	//ergo BCSCTL2 = 0x00

	// Follow recommended flow. First, clear all DCOx and MODx bits. Then
	// apply new RSELx values. Finally, apply new DCOx and MODx bit values.
	DCOCTL = 0x00;
	//BCSCTL1:///////////////////////////////
	//XT2 off = 1
	//LFXT1: low frequency mode, not used = 0
	//DIVA_0 AuxCLK divider 1
	//RSEL: DCO frequency range selected 7 -> 5..8MHz
	BCSCTL1 = XT2OFF | DIVA_0 |  RSEL2 | RSEL1 | RSEL0;

	//DCO = 7 --> max. frequency 8Mhz
	DCOCTL  = DCO2 | DCO1 | DCO0; // no Modulation
#elif defined (__MSP430F5510__)
	//ToDo create configuration for F5510 (e.g. Olimexino)
	//UCA0CTL0 ...
//	UCSCTL1 |= DCORSEL2 | DCORSEL1 | DCORSEL0
//	UCSCTL1 &= ~DISMOD;
	//UCSCTL4 |=
//  UCSCTL6 |= XT2OFF;


#else
#error "Target configuration wrong choose either ''F149 or ''F5510"
#endif

}

void initGPIO() {
#if defined (__MSP430F149__)
	//LED Pins auf o/p und low
	P1DIR =  STATUS_LED | ALARM_OUT;
	//all Outputs off
	P1OUT &= ~(STATUS_LED + ALARM_OUT);
	//Relais Pin o/p low
	P6DIR |= RELAY_BW_OUT + RELAY_FW_OUT;
	P6OUT &= ~(RELAY_BW_OUT + RELAY_FW_OUT);

    //disable Port IO Buffer for bits of analog input (both sensors)
	P6SEL |= HEATING_TEMP_SENSOR_PIN + BOILER_TEMP_SENSOR_PIN;


	//ESP Reset line P4.0
	P4DIR |= ESP_RST;
	//Reset the device = PULL DOWN
	P4OUT &= ~ESP_RST;

	//uart0 + uart1 pins
	//UART0 TX P3.4 RX P3.5
	//UART1 TX P3.6 RX P3.7


	P3SEL = BIT6 | BIT7;

	P3DIR = 0x00;
	P3OUT = 0x00;
	//write out SMCLK on P1.4
	//P1DIR |= BIT1;
	//P1SEL |=  BIT1;

	//input toggle switch
#elif defined (__MSP430F5510__)
	P4SEL |= BIT4 | BIT5;
	P4DIR |= BIT2 + BIT3;
	P4OUT |= BIT2; //VCC für WLAN

#else
#error "Target configuration wrong choose either ''F149 or ''F5510"
#endif

}

void initUart1() {

#if defined (__MSP430F149__)
	/*
	 * Control Register 1
	 *
	 * UCSSEL_2 -- SMCLK
	 * ~UCRXEIE -- Erroneous characters rejected and UCAxRXIFG is not set
	 * ~UCBRKIE -- Received break characters do not set UCAxRXIFG
	 * ~UCDORM -- Not dormant. All received characters will set UCAxRXIFG
	 * ~UCTXADDR -- Next frame transmitted is data
	 * ~UCTXBRK -- Next frame transmitted is not a break
	 * UCSWRST -- Enabled. USCI logic held in reset state
	 *
	 * Note: ~<BIT> indicates that <BIT> has value zero
	 */
	/* Disable USCI */
	U0CTL |= SWRST;

	U0CTL |= CHAR;
	//enable RX and TX Module
	ME1 |=	UTXE0 + URXE0;

	//Clock source for transmit is SMCLK
	U0TCTL = SSEL1;

	//on MSP430G2553:
	//U0MCTL = UBRF_0 | UCBRS_2;
	//seems to be equivalent to:

	//Modululation Control-register
	//CHAR = 8 Bit messages

	//EIGENTLICH FEHLERHAFT!!!
	//U0MCTL = 0x04 + CHAR;

	U0MCTL = 0x04;

	//UBR = 833
	/* Baud rate control register 0 */
	U0BR0 = 65;

	/* Baud rate control register 1 */
	U0BR1 = 3;

	//enable transmitter and receiver 0 for ESP8266 Module Communication
	ME1 = UTXE0 | URXE0;

	/* Enable USCI */
	U0CTL &= ~SWRST;

	//enable receive interrupt
	IE1 |= URXIE0;
#elif defined (__MSP430F5510__)
	//ToDo create configuration for F5510 (e.g. Olimexino)
	//UCA0CTL0 ...
	UCA0CTL1 |= UCSWRST;

	//source is ACLK = 32KHZ
	UCA0CTL1 |= UCSSEL_1;


	 /* Baud rate control register 0 */
	UCA0BR0 = 3;

	    /* Baud rate control register 1 */
	UCA0BR1 = 0;

	UCA0MCTL = UCBRS0 | UCBRS1 | UCBRS2;


	    /* Enable USCI */
	UCA0CTL1 &= ~UCSWRST;


#else
#error "Target configuration wrong choose either ''F149 or ''F5510"
#endif
}

void initUart115KBaud() {
	U1CTL |= SWRST;
	U1CTL |= CHAR;
	//enable RX and TX Module
	ME2 |=	UTXE1 + URXE1;

	//Clock source for transmit is SMCLK
	U1TCTL = SSEL1;

	//evaluated setting///////////////////////
	//U1MCTL = 0xB5;							//
											//
	//UBR = 833
	/* Baud rate control register 0 */
	//U1BR0 = 29;

	/* Baud rate control register 1 */		//
	//U1BR1 = 0;							//
	//////////////////////////////////////////
	//calculated actual setting///////////////
											//
											//
	U1MCTL = 0x03;							//
												//
	//UBR = 833
	/* Baud rate control register 0 */
	U1BR0 = 40;

	/* Baud rate control register 1 */		//
	U1BR1 = 0;								//

	/* Enable USCI */
	U1CTL &= ~SWRST;

	//enable receive interrupt
	IE2 |=  URXIE1;
}

void initUart38_4KBaud() {
    U1CTL |= SWRST;
    U1CTL |= CHAR;
    //enable RX and TX Module
    ME2 |=  UTXE1 + URXE1;

    //Clock source for transmit is SMCLK
    U1TCTL = SSEL1;

    //evaluated setting///////////////////////
    //U1MCTL = 0xB5;                            //
                                            //
    //UBR = 833
    /* Baud rate control register 0 */
    //U1BR0 = 29;

    /* Baud rate control register 1 */      //
    //U1BR1 = 0;                            //
    //////////////////////////////////////////
    //calculated actual setting///////////////
                                            //
                                            //

    // Somehow SMCLK seems to  be 7.705MHz

    // Config for 38_4KBaud
    //U1MCTL = 0x00;                          //
                                            //
    //UBR = 833
    /* Baud rate control register 0 */
    //U1BR0 = 0xE0; //200 decimal

    /* Baud rate control register 1 */      //
    //U1BR1 = 0;                              //
    //////////////////////////////////////
    //Konfig for 76800Baud
    U1MCTL = 0x00;                          //
    /* Baud rate control register 0 */
    U1BR0 = 0x40; //100 decimal
   /* Baud rate control register 1 */      //
    U1BR1 = 0;                              //


    /* Enable USCI */
    U1CTL &= ~SWRST;

    //enable receive interrupt
    IE2 |=  URXIE1;
}

void initUart76_8Baud(){
    U1CTL |= SWRST;
    U1CTL |= CHAR;
    //enable RX and TX Module
    ME2 |=  UTXE1 + URXE1;

    //Clock source for transmit is SMCLK
    U1TCTL = SSEL1;

    //evaluated setting///////////////////////
    //U1MCTL = 0xB5;                            //
                                            //
    //UBR = 833
    /* Baud rate control register 0 */
    //U1BR0 = 29;

    /* Baud rate control register 1 */      //
    //U1BR1 = 0;                            //
    //////////////////////////////////////////
    //calculated actual setting///////////////
                                            //
                                            //
    U1MCTL = 0x03;                          //
                                                //
    //UBR = 833
    /* Baud rate control register 0 */
    U1BR0 = 61;

    /* Baud rate control register 1 */      //
    U1BR1 = 0;                              //

    /* Enable USCI */
    U1CTL &= ~SWRST;

    //enable receive interrupt
    IE2 |=  URXIE1;


}


void initTimer0() {
	//Select SMCLK as clk source
	//divide clk freq. by 8 --> 1MHz
	//MC_1 count up until TACCR0
	//enable the interrup
	TACTL |= TASSEL_2 | ID_3 | MC_1 ;
	TACTL &= ~TAIE;
	//16 times per second
	TACCR0 = 62500;
	TACCTL0 |= CCIE;

}


void initUart0() {

#if defined (__MSP430F149__)
	/*
	 * Control Register 1
	 *
	 * UCSSEL_2 -- SMCLK
	 * ~UCRXEIE -- Erroneous characters rejected and UCAxRXIFG is not set
	 * ~UCBRKIE -- Received break characters do not set UCAxRXIFG
	 * ~UCDORM -- Not dormant. All received characters will set UCAxRXIFG
	 * ~UCTXADDR -- Next frame transmitted is data
	 * ~UCTXBRK -- Next frame transmitted is not a break
	 * UCSWRST -- Enabled. USCI logic held in reset state
	 *
	 * Note: ~<BIT> indicates that <BIT> has value zero
	 */
	/* Disable USCI */
	U1CTL |= SWRST;
	U1CTL |= CHAR;
	//enable RX and TX Module
	ME2 |=	UTXE1 + URXE1;

	//Clock source for transmit is SMCLK
	U1TCTL |= SSEL1;

	//on MSP430G2553:
	//U0MCTL = UBRF_0 | UCBRS_2;
	//seems to be equivalent to:
	//U1MCTL = 0x04;
	U1MCTL = 0x01;


	/* Baud rate control register 0 */
	//U1BR0 = 65;
	U1BR0 = 0xF1;

	/* Baud rate control register 1 */
	//U1BR1 = 3;
	U1BR1	= 0x01;


	//enable transmitter and receiver 0
	ME2 = UTXE1 | URXE1;

	/* Enable USCI */
	U1CTL &= ~SWRST;

	//enable receive interrupt
	IE2 |=  URXIE1;
#elif defined (__MSP430F5510__)
	//ToDo create configuration for F5510 (e.g. Olimexino)
	//UCA0CTL0 ...

#else
#error "Target configuration wrong choose either ''F149 or ''F5510"
#endif

}


void init_adc_12(){
    //Select internal voltage references (2.5V)
    //ADC12CTL0 = ??

    //configure for maximum sample & hold time (1024 cycles)
    //ADC12CTL0 = SHT10 | SHT11 | SHT12 | SHT13 | SHT00 | SHT01 | SHT02 | SHT03;
    ADC12CTL0 = SHT0_8;
    ADC12CTL0 |=  REF2_5V | REFON | ADC12ON ;
    ADC12CTL0 &= ~(MSC | ADC12OVIE |  ADC12TOVIE | ENC  | ADC12SC);

    //select clock source and single channel mode
    ADC12CTL1 = CSTARTADD_0 | SHS_0 | ADC12DIV0 | ADC12SSEL_0 | CONSEQ_0 | SHP;
    ADC12CTL1 &= ~(ISSH);
    // select reference +/- for first conversion memmory (16 available)
    ADC12MCTL0 |= SREF_1;


    //ENC = 1
    //ADC12IFG

    //start conversion
    //ADC12SC


}

void switchLed(bool state) {
	int bit = STATUS_LED;
	if (state == true) {
	    P1OUT |= bit;
	} else {
	    P1OUT &= ~bit;
	}
}






