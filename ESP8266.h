/*
 * ESP8266.h
 *
 *  Created on: 21.03.2016
 *      Author: Manuel
 *      This Header provides function for konfiguration and usage of
 *      ESP8266 UART WLAN Module
 *
 */
#ifndef ESP8266_H_
#define ESP8266_H_

#include "uart.h"
//this include will automatically select the right target
#include <msp430.h>
#include <stdint.h>
#include "System.h"
#include <string.h>
#include "general.h"
//#include "HomeAutomation.h"
#include <stdio.h>

//ESP reset line P4.0
//#define ESP_RST BIT0;

#define MAX_SSID_LENGTH 20

//Defines Methods for communication via UART with HC-05 Bluetooth device
#define WLAN_BUFFER_MAX_SIZE 5
#define WLAN_BUFFER_MAX_STRINGLENGTH 50

typedef enum {
	WLAN_COMMAND_NONE = 0x00,
	WLAN_COMMAND_ASK_READY = 0x01,
	WLAN_COMMAND_ENABLE_MULITPLE_SESSIONS,
	WLAN_COMMAND_DISABLE_MULTIPLE_SESSIONS,
	WLAN_COMMAND_SWITCH_MODE,				//CWMODE
	WLAN_COMMAND_LIST_ACCESSPOINTS,				//CWLAP
	WLAN_COMMAND_SET_STATIC_IP,
	WLAN_COMMAND_JOIN_ACCESSPOINT,				//CWJAP
	WLAN_COMMAND_DISCONNECT_ACCESSPOINT,		//CQQAP
	WLAN_COMMAND_GET_OWN_MAC,					//CIPSTAMAC
	WLAN_COMMAND_SET_OWN_MAC,
	WLAN_COMMAND_GET_TCP_STATUS,				//CIPSTATUS
	WLAN_COMMAND_CONNECT_TO_TCP,				//CIPSTART
	WLAN_COMMAND_GET_OWN_IP,					//CIFSR
	WLAN_COMMAND_DISCONNECT_FROM_TCP,			//CIPCLOSE
	WLAN_COMMAND_SEND_TCP,						//CIPSEND
	WLAN_COMMAND_CHECK_TCP_STATUS				//CIPSTATUS
} ACTIVE_COMMAND;

typedef enum {
	WLAN_OK = 0x01,
	UNDEFINED_ERROR = 0x02,
	AP_CONNECTION_FAILURE = 0x03,
	TCP_CONNECTION_FAILURE = 0x04,
	TCP_CONNECTION_LOSS = 0x05,
	COMMAND_TIMEOUT = 0x06
	//ToDo add more error codes
} COMMAND_EXEC_RETURN;

typedef enum {
	WLAN_STATE_IDLE = 0x00,	//we don't know what it is doing
	WLAN_STATE_READY =0x01,	//received ready message at startup
	WLAN_STATE_CONNECTED_AP = 0x02,
	WLAN_STATE_CONNECTED_AP_GOT_IP = 0x03,
	WLAN_STATE_CONNECTED_TCP = 0x04	//required a WIFI Connection
} ESP8266_STATE;


/**
 * \brief  List of AP stations found on network search
 */
typedef enum {
	ESP8266_SleepMode_Disable = 0x00, /*!< Sleep mode disabled */
	ESP8266_SleepMode_Light = 0x01,   /*!< Light sleep mode */
	ESP8266_SleepMode_Modem = 0x02    /*!< Model sleep mode */
} ESP8266_SleepMode_t;

typedef struct{
	ESP8266_STATE state;
	//ESP8266_ConnectedWifi_t ConnectedWifi;                    /*!< Informations about currently connected wifi network */
	ACTIVE_COMMAND activeCommand;
	//ESP8266_WifiConnectError_t WifiConnectError;              /*!< Error code for connection to wifi network. This parameter can be a value of \ref ESP8266_WifiConnectError_t enumeration */
	//active Command
	//command timout timer
} ESP8266_t;
void espSetHardReset(bool reset);

bool esp_init(ESP8266_t* esp);

bool esp_init_baudrates(ESP8266_t* esp);

void send_config_baudrate(int baudrate);

void esp_update_tcp_state(ESP8266_t* esp_ptr);

void wlan_update(ESP8266_t* esp);

void processSendBuffer(ESP8266_t* esp);

void enqueueTcpMessage(char*);

//switch mode 1=station, 2=AccessPoint, 3=both
void wlan_switch_mode(ESP8266_t* esp, char mode);
//same but set permanent default value
void wlan_set_default_mode(ESP8266_t* esp, char mode);

///Connection:
//connects to a given network
void wlan_connect_to_ap(ESP8266_t* esp, char* ssid, char* pwd);
//same, but sets default network connection
void wlan_set_default_ap(ESP8266_t* esp, char* ssid, char* pwd);

void disconnect_from_ap(ESP8266_t* esp);

////TCP Connections:

void wlan_set_static_ip(ESP8266_t* esp, char* ip, char* gateway, char* netmask);

void wlan_connect_to_tcp_server(ESP8266_t* esp, unsigned int type, char* addr, unsigned int port);
void wlan_disconnect_from_tcp_server(ESP8266_t* esp) ;


void wlan_enable_multiple_sessions(ESP8266_t* esp, unsigned int enabled);
void wlan_tcp_send_string(ESP8266_t* esp, char data[]);
//wait for acknowledge of the currently active command
//it will return != 0 if an error occured
COMMAND_EXEC_RETURN wait_ready(ESP8266_t* esp8266);

extern unsigned int wlanSendBufferLength;
extern char wlanSendBuffer[WLAN_BUFFER_MAX_SIZE][WLAN_BUFFER_MAX_STRINGLENGTH];



#endif /* ESP8266_H_ */
