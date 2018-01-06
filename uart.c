#include "uart.h"
#include <string.h>
#include <stdbool.h>
/*
 * uart.c
 *
 *  Created on: 13.02.2015
 *      Author: Manuel du Bois
 */
#define uart_rx	BIT1   			//P1.1
#define uart_tx	BIT2			//P1.2

char  received_string[UART_BUFFER_MAX_STRINGLENGTH] = {0x00};

unsigned int charPosition=0;
char lastCharacter=0;

char  received_string_usb[UART_BUFFER_MAX_STRINGLENGTH] = {0x00};

unsigned int charPosition_usb=0;
char lastCharacter_usb=0;


unsigned int uartBufferLength = 0;
char uartBuffer[UART_BUFFER_MAX_SIZE][UART_BUFFER_MAX_STRINGLENGTH] = {0x00};

unsigned int uartBufferLength_usb = 0;
char uartBuffer_usb[UART_BUFFER_MAX_SIZE][UART_BUFFER_MAX_STRINGLENGTH] = {0x00};

char uart_init(void){
	//Konfigureren der Komunikation als UART
	/*UCA0CTL0 = UCPEN; //Asynchroner Modus, erst LSB dann MSB, 1 Stop, PArity aktiv
	UCA0CTL1 = UCSSEL1 + UCSSEL0; //SMCLK als Taktquelle
	UCA0BR0  = 52;						//Baudrate wahl: 9600 Baud
	UCA0MCTL |= UCBRS0;// UCBRS=1
	UCA0CTL0 &= ~UCSWRST ;	//Zustandsautomaten initialisieren
	IE2 |= UCA0RXIE; //Receive interupt aktivieren
	P1SEL  |= uart_tx + uart_rx;
	P1SEL2 |= uart_tx + uart_rx;*/
	uart_buffer_clear();
	return 0;
}

void uart_send(char data){	//Byte senden
#if defined (__MSP430F149__)
	while (!(IFG1 & UTXIFG0 )); //Warten bis bereit
		U1TXBUF=data;
#elif defined (__MSP430F5510__)
		//using UCA0
	while (!(UCA0IFG & UCTXIFG )); //Warten bis bereit
		UCA0TXBUF=data;
#else
	#error "Target configuration wrong choose either ''F149 or ''F5510"
#endif

}
void uart_send_string(char* data){	//Byte senden
	int i=0;
	char tx_data;
	int count = strlen(data);
	for(i=0; i<count; i++){
		uartWaitReadyToSend();
		tx_data = data[i];
		//uart_send(tx_data);
		// has been inlined
		while (!(IFG1 & UTXIFG0 )); //Warten bis bereit
		        U1TXBUF=tx_data;
		// end inline block
		//UCA0TXBUF=tx_data;
	}
}

void uartWaitReadyToSend() {
#if defined (__MSP430F149__)
	while (!(IFG2 & UTXIFG1 )); //Warten bis bereit
#elif defined (__MSP430F5510__)
	while (!(UCA0IFG & UCTXIFG )); //Warten bis bereit
#else
#error "Target configuration wrong choose either ''F149 or ''F5510"
#endif
}

inline void uart_receive_string() {
	memcpy(uartBuffer[uartBufferLength], received_string, charPosition);
	uartBufferLength++;
	//
	if(uartBufferLength >= UART_BUFFER_MAX_SIZE) {
		uartBufferLength=0;
		//	uart_buffer_clear();
	}
}
inline void uart_receive_string_usb() {
	//memcpy(uartBuffer_usb[uartBufferLength_usb], received_string_usb, charPosition_usb);
	//uartBufferLength++;
	//
	//if(uartBufferLength_usb >= UART_BUFFER_MAX_SIZE) {
	//	uartBufferLength_usb=0;
		//	uart_buffer_clear();
	//}

	//received_string_usb auswerten
	char wifiCredentialsDelimiter[2];
	wifiCredentialsDelimiter[0] = ';';
	wifiCredentialsDelimiter[1] = 0x00;
	char* ssid = strtok(received_string_usb, wifiCredentialsDelimiter );
	char* wpa_pass = strtok(NULL, wifiCredentialsDelimiter );
}


void uart_buffer_clear() {
	memset(received_string, 0, sizeof(received_string));
	int i=0;
	for(i=0; i<UART_BUFFER_MAX_SIZE; i++ ){
		memset(uartBuffer[i], 0, sizeof(uartBuffer[i]));

	}
	charPosition=0;
	uartBufferLength=0;
}
//if successfull returns row number starting with 1 !!
unsigned int uart_buffer_find(char* substring) {
	int i=0;
	for(i=0; i<UART_BUFFER_MAX_SIZE; i++ ){
		if( strstr(uartBuffer[i], substring)!=NULL ) {
			return i+1;
		}
	}
	return 0;
}
void uart_buffer_delete_row(unsigned int row) {
	//row number start with 1 (not 0)
	if(row > 0) {
		memset(uartBuffer[row-1], 0, UART_BUFFER_MAX_STRINGLENGTH);
	}
}
bool uart_buffer_find_delete(char* substring) {
	unsigned int row =  uart_buffer_find(substring);
	if (row != 0) {
		uart_buffer_delete_row(row);
		return true;
	} else {
		return false;
	}
}



bool uart_buffer_find_tcp_payload(char payload[]) {
	unsigned int i=0;
	for(i=0; i<UART_BUFFER_MAX_SIZE; i++ ){
		if( strstr(uartBuffer[i], "+IPD")!=NULL ) {
			//+IPD,15:blabla
			strcpy(payload, strstr(uartBuffer[i], ":") +1);
			if(payload != NULL) {
				return true;
			}
		}
	}
	return false;
}

bool uart_buffer_find_delete_tcp_payload(char payload[]) {
	unsigned int i=0;
	for(i=0; i<UART_BUFFER_MAX_SIZE; i++ ){
		if( strstr(uartBuffer[i], "+IPD")!=NULL ) {
			//+IPD,15:blabla
			strcpy(payload, strstr(uartBuffer[i], ":") +1);
			//delete this message because we don't want to find it again
			memset(uartBuffer[i], 0, UART_BUFFER_MAX_STRINGLENGTH);
			if(payload != NULL) {
				return true;
			}
		}
	}
	return false;
}

void clear_received_string(void) {
	//unsigned int i=(UART_BUFFER_MAX_STRINGLENGTH-1);
	//for(i = (UART_BUFFER_MAX_STRINGLENGTH-1); i==0; i--){
	//	received_string[i] = 0;
	//}
	memset(received_string, 0, UART_BUFFER_MAX_STRINGLENGTH);
	charPosition = 0;
}


//ISR UART0 RX IRG MCP2200 USB UART Interface
#pragma vector=USART0RX_VECTOR
__interrupt void uart0_received (void)
{
#if defined (__MSP430F149__)
	if (IFG1 & URXIFG0) {
		const char newCharacter= U0RXBUF;
		received_string_usb[charPosition_usb] = newCharacter;
		charPosition_usb++;
		if(charPosition_usb >= UART_BUFFER_MAX_STRINGLENGTH)
			charPosition_usb = 0;
		if(newCharacter=='\n') {
			if(lastCharacter=='\r' ) {
				if(charPosition_usb >2) {
					//pass message for further processing
					uart_receive_string_usb();
					//memset(received_string, 0, sizeof(received_string));
					charPosition_usb =0;
				}else {
					//ESP has sent a senseless CRLF before the next message  -->ignore
				}
			}
		}
		lastCharacter = newCharacter;
		IFG1 &= ~URXIFG0;
	}
#elif defined (__MSP430F5510__)
//	if (UCA0IFG & UCRXIFG) {
//		const char newCharacter= UCA0RXBUF;
//		received_string[charPosition] = newCharacter;
//		charPosition++;
//		if(charPosition >= UART_BUFFER_MAX_STRINGLENGTH)
//			charPosition = 0;
//		if(newCharacter=='\n') {
//			if(lastCharacter=='\r' ) {
//				if(charPosition >2) {
//					//pass message for further processing
//					uart_receive_string();
//					//memset(received_string, 0, sizeof(received_string));
//					charPosition =0;
//				}else {
//					//ESP has sent a senseless CRLF before the next message  -->ignore
//				}
//			}
//		}
//		lastCharacter = newCharacter;
//		UCA0IFG &= ~UCRXIFG;
//	}
#else
#error "Target configuration wrong choose either ''F149 or ''F5510"
#endif
}



//ISR UART1 RX IRQ = WLAN Modul ESP8266
#pragma vector=USART1RX_VECTOR
__interrupt void uart1_received (void)

{
#if defined (__MSP430F149__)
	//if (IFG2 & URXIFG1) {
		const char newCharacter= U1RXBUF;
		received_string[charPosition] = newCharacter;
		charPosition++;
		if(charPosition >= UART_BUFFER_MAX_STRINGLENGTH)
			charPosition = 0;
		if(newCharacter=='\n') {
			if(lastCharacter=='\r' ) {
				if(charPosition >2) {
					//pass message for further processing
					uart_receive_string();
					//memset(received_string, 0, sizeof(received_string));
					charPosition =0;
				}else {
					//ESP has sent a senseless CRLF before the next message  -->ignore
				}
			}
		}
		lastCharacter = newCharacter;
		IFG2 &= ~URXIFG1;
	//}
#elif defined (__MSP430F5510__)
	if (UCA0IFG & UCRXIFG) {
		const char newCharacter= UCA0RXBUF;
		received_string[charPosition] = newCharacter;
		charPosition++;
		if(charPosition >= UART_BUFFER_MAX_STRINGLENGTH)
			charPosition = 0;
		if(newCharacter=='\n') {
			if(lastCharacter=='\r' ) {
				if(charPosition >2) {
					//pass message for further processing
					uart_receive_string();
					//memset(received_string, 0, sizeof(received_string));
					charPosition =0;
				}else {
					//ESP has sent a senseless CRLF before the next message  -->ignore
				}
			}
		}
		lastCharacter = newCharacter;
		UCA0IFG &= ~UCRXIFG;
	}
#else
#error "Target configuration wrong choose either ''F149 or ''F5510"
#endif
}

