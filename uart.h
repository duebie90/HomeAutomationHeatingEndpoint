#ifndef UART_BLUETOOTH_H_
#define UART_BLUETOOTH_H_

#include <msp430.h>
#include <stdbool.h>
//
//
//author: Manuel du Bois
//
//Defines Methods for communication via UART with HC-05 Bluetooth device
#define UART_BUFFER_MAX_SIZE 3
#define UART_BUFFER_MAX_STRINGLENGTH 50

//UART
char uart_init(void);
void uart_buffer_clear();

void uartWaitReadyToSend();
void uart_send(char data);
void uart_send_string(char* data);

extern void uart_receive_string();
extern void uart_receive_string_usb();
__interrupt void uart0_received(void);
__interrupt void uart1_received(void);
void clear_received_string(void);

unsigned int uart_buffer_find(char* substring);
void uart_buffer_delete(unsigned int row);
bool uart_buffer_find_delete(char* substring);

bool uart_buffer_find_tcp_payload(char payload[]);

bool uart_buffer_find_delete_tcp_payload(char payload[]);

extern unsigned int charPosition;
extern char received_string[UART_BUFFER_MAX_STRINGLENGTH];
extern char lastCharacter;

extern unsigned int uartBufferLength;
extern char uartBuffer[UART_BUFFER_MAX_SIZE][UART_BUFFER_MAX_STRINGLENGTH];

extern char initd;

//Interrupt bei Leerem Sende- und vollem Empfangspuffer

//Methoden für Uart RX, TX (TX aufgerufen durch interrupt)

//Methoden für Bluetooth initialisierung
//nach initialisierung ist BT-Gerät transparent

//Methoden für Datenverkehrssteuerung & Queuing

// Kommunikation mit Android //

#endif /* ULTRASONIC_H_ACC_H_ */
