/*
 * ESP8266.c
 *
 *  Created on: 21.03.2016
 *      Author: admin
 */
#include "ESP8266.h"

#define COMMAND_TIMEOUT_COUNTER_THRESHOLD 40000//80000

bool connected_to_ap = false;

unsigned int tcp_reconnect_counter = 0;
char command[50]={0x00};

void espSetHardReset(bool reset) {
	if (reset == false) {
		//set reset state
		P4OUT |= ESP_RST;
	} else {
		//reset reset state
		P4OUT &= ~ESP_RST;
	}
}

bool esp_init(ESP8266_t* esp) {
	unsigned int askReadyCounter = 0;
	uart_send_string("ATE0\r\n");


	connected_to_ap = false;
	esp->state = WLAN_STATE_IDLE;
	char command[15] = {0x00};
	strcat(command, "AT+RST\r\n");
	uart_send_string(command);
	//wait untill ESP8266 has booted
	wait_ms(5000);
	uart_buffer_clear();
	//ask for readyness repeatedly
	strcpy(command,"AT\r\n");
	do {
		esp->activeCommand = WLAN_COMMAND_ASK_READY;
		uart_send_string(command);
		askReadyCounter++;
		if(askReadyCounter >= 5) {
				askReadyCounter = 0;
				strcpy(command, "AT+RST\r\n");
				uart_send_string(command);
				//wait untill ESP8266 has booted
				wait_ms(10000);
				uart_buffer_clear();
				strcpy(command,"AT\r\n");
				uart_send_string(command);
				esp->activeCommand = WLAN_COMMAND_ASK_READY;
		}
	} while (wait_ready(esp) != WLAN_OK);

	wait_ms(10);
	//turn of command echoing
	uart_send_string("ATE0\r\n");
	wait_ms(10);

	uart_buffer_clear();

//	char qcommand[17]="AT+CWMODE?";
//	strcat(qcommand,"\r\n");
//	uart_send_string(qcommand);
//	wait_ms(100);
//	uart_buffer_clear();
	/////
	wlan_switch_mode(esp, 1);
	while(wait_ready(esp)!=WLAN_OK) {
		//wait
	}
	wait_ms(100);
	wlan_set_default_mode(esp, 1);
	while(wait_ready(esp)!=WLAN_OK) {
			//wait
	}
//	wait_ms(100);
//	char qcommand2[17]="AT+CWMODE?";
//	strcat(qcommand2,"\r\n");
//	uart_send_string(qcommand2);
//	wait_ms(100);
//	uart_buffer_clear();

	return true;
}

bool esp_init_baudrates(ESP8266_t* esp){
    uart_send_string("ATE0\r\n");
    connected_to_ap = false;
    esp->state = WLAN_STATE_IDLE;
    esp->activeCommand = WLAN_COMMAND_ASK_READY;
    char reset_command[15] = {0x00};
    char ask_ready_command[15] = {0x00};
    strcat(reset_command, "AT+RST\r\n");
    strcpy(ask_ready_command,"AT\r\n");
    uart_send_string(reset_command);
    wait_ms(1000);
    uart_buffer_clear();
    //wait untill ESP8266 has booted

    unsigned short attempts_counter = 10;

    while(attempts_counter-- > 0){
        //ask for readyness repeatedly
    	esp->activeCommand = WLAN_COMMAND_ASK_READY;
        uart_send_string(ask_ready_command);
        wait_ms(100);
        if(wait_ready(esp)==WLAN_OK){
            return true;
        }

        //try to communicate using the next baudrate
        initUart38_4KBaud();
        wait_ms(100);
        uart_send_string(reset_command);
        wait_ms(1000);
        uart_buffer_clear();
        //wait untill ESP8266 has booted
        //ask for readyness repeatedly
        esp->activeCommand = WLAN_COMMAND_ASK_READY;
        uart_send_string(ask_ready_command);
        wait_ms(100);
        if(wait_ready(esp)==WLAN_OK){
            return true;
        }
        //try to communicate using the next baudrate
        initUart76_8Baud();
        wait_ms(100);
        uart_send_string(reset_command);
        wait_ms(1000);
        uart_buffer_clear();
        //wait untill ESP8266 has booted
        //ask for readyness repeatedly
        esp->activeCommand = WLAN_COMMAND_ASK_READY;
        uart_send_string(ask_ready_command);
        wait_ms(100);
        if(wait_ready(esp)==WLAN_OK){
            return true;
        }

        //try to communicate using the next baudrate
        initUart115KBaud();
        wait_ms(100);
        uart_send_string(reset_command);
        wait_ms(1000);
        uart_buffer_clear();
        //wait untill ESP8266 has booted
        //ask for readyness repeatedly
        esp->activeCommand = WLAN_COMMAND_ASK_READY;
        uart_send_string(ask_ready_command);
        wait_ms(100);
        if(wait_ready(esp)==WLAN_OK){
            return true;
        }
    }
    return false;
}

void wlan_switch_mode(ESP8266_t* esp, char mode) {
	esp->activeCommand = WLAN_COMMAND_SWITCH_MODE;
	//switch mode 1=station, 2=AccessPoint, 3=both
	if(mode==0 || mode >3)
		mode = 1;
	char buffer[2] = {0x00,0x00};
	char command[17]="AT+CWMODE_CUR=";
	sprintf(buffer, "%d", mode);
	strcat(command, buffer);
	strcat(command,"\r\n");
	uart_send_string(command);
}
void wlan_set_default_mode(ESP8266_t* esp, char mode) {
	//switch mode 1=station, 2=AccessPoint, 3=both
	char buffer[2] = {0x00,0x00};
	char command[17]="AT+CWMODE_DEF=";
	sprintf(buffer, "%d", mode);
	strcat(command, buffer);
	strcat(command,"\r\n");
	uart_send_string(command);
}

void send_config_baudrate(int baudrate) {
    //  //try to configure ESPs Baudrate to 9600
        char baudrate_string[50]={0x00};
        char buffer[5];
        sprintf(baudrate_string, "%d", baudrate);
        strcat(command, "AT+UART_CUR=");
        strcat(command, baudrate_string);
        strcat(command, "9600,8,1,0,0");
        strcat(command, "\r\n");
        uart_send_string(command);
}

void wlan_set_static_ip(ESP8266_t* esp, char* ip, char* gateway, char* netmask){
	esp->activeCommand = WLAN_COMMAND_SET_STATIC_IP;
	char command[100]={0x00};
	strcat(command, "AT+CIPSTA_CUR=\"");
	strcat(command, ip);
	strcat(command, "\",\"");
	strcat(command, gateway);
	strcat(command, "\",\"");
	strcat(command, netmask);
	strcat(command, "\"\r\n");
	uart_send_string(command);
}


void wlan_connect_to_ap(ESP8266_t* esp, char* ssid, char* pwd) {
	//set active command
	esp->activeCommand = WLAN_COMMAND_JOIN_ACCESSPOINT;
	//Do it
	command[0] = 0;
	strcat(command, "AT+CWJAP_CUR=\"");
	strcat(command, ssid);
	strcat(command, "\",\"");
	strcat(command, pwd);
	strcat(command, "\"\r\n");
	uart_send_string(command);
}


void wlan_set_default_ap(ESP8266_t* esp, char* ssid, char* pwd) {
	char command[50]={0x00};
	strcat(command, "AT+CWJAP_DEF=\"");
	strcat(command, ssid);
	strcat(command, "\",\"");
	strcat(command, pwd);
	strcat(command, "\"\r\n");
	uart_send_string(command);
}

char wlan_is_connected_to_ap() {
	return connected_to_ap;
}


void disconnect_from_ap(ESP8266_t* esp) {
	esp->activeCommand = WLAN_COMMAND_DISCONNECT_ACCESSPOINT;
	char command[10]={0x00};
	strcat(command, "AT+CWQAP");
	uart_send_string(command);
}

//tries to connect to an TCP/UDP Server
//Type 1=TCP, other=UDP
void wlan_connect_to_tcp_server(ESP8266_t* esp, unsigned int type, char* addr, unsigned int port) {
	//set active command
	esp->activeCommand = WLAN_COMMAND_CONNECT_TO_TCP;
	//Do it
	//char command[45]={0x00};
	command[0] = 0;
	strcat(command, "AT+CIPSTART=");
	char buffer[8];
	if(type==1)
		strcat(command, "\"TCP\",\"");
	else
		strcat(command, "\"UDP\", \"");
	strcat(command, addr);
	strcat(command, "\",");

	buffer[0] = '0' + (int)(port/1000);
	buffer[1] = '0' + (int)( (port % 1000) / 100);
	buffer[2] = '0' + (int)( (port %100) / 10);
	buffer[3] = '0' + (int)(port % 10);
	buffer[4] = 0x0D;
	buffer[5] = 0x0A;
	buffer[6] = 0x00;

	if (port > 999) {
		//has four digits (standard)

	} else if (port > 99) {
		//has three digits
	} else if (port > 9){
		//has two digits
	} else {
		//only one digit

	}
	strcat(command, buffer);
	uart_send_string(command);
	//wait_ms(5000);
}
void wlan_disconnect_from_tcp_server(ESP8266_t* esp) {
	//set active command
	esp->activeCommand = WLAN_COMMAND_DISCONNECT_FROM_TCP;
	//Do it
	char command[15]="AT+CIPCLOSE\r\n";
	uart_send_string(command);
}

void wlan_enable_multiple_sessions(ESP8266_t* esp, unsigned int enabled) {
	if(enabled!=0 && enabled!=1)
		enabled=0;
	char buffer[3];
	char command[13]="AT+CIPMUX=";
	sprintf(buffer, "%d\r\n", enabled);
	strcat(command, buffer);
	uart_send_string(command);
}



void wlan_tcp_send_string(ESP8266_t* esp_ptr, char data[]) {

	if (esp_ptr->state == WLAN_STATE_CONNECTED_TCP) {
		//set active command
		esp_ptr->activeCommand = WLAN_COMMAND_SEND_TCP;
		char command[20];
		command[0] = 0x00;
		strcat(command, "AT+CIPSEND=");
		unsigned int length= strlen(data);
		char buffer[2] = {0x00};
		if (length < 100) {
			if (length >9) {
				//two digits
				buffer[0] = '0' + (int)(length/10);
				strcat(command, buffer );
			}
			buffer[0] = '0' + (int)(length % 10) ;
			strcat(command, buffer);
		}
		strcat(command, "\r\n");
		//announce sending data over tcp
		uart_send_string(command);
		wait_ms(100);
		//send actual data
		uart_send_string(data);
		//return 0;
	}
	//else {
	//	return -1;
	//}
}

	volatile long counter 	= 0;
COMMAND_EXEC_RETURN wait_ready(ESP8266_t* esp) {
	volatile long timeOut = COMMAND_TIMEOUT_COUNTER_THRESHOLD;
//	if(esp->activeCommand == WLAN_COMMAND_CONNECT_TO_TCP || esp->activeCommand == WLAN_COMMAND_JOIN_ACCESSPOINT) {
		//those command take longer
//		timeOut = 50000;//100000;
//	}
	COMMAND_EXEC_RETURN exec_ret = COMMAND_TIMEOUT;
	while ( esp->activeCommand != WLAN_COMMAND_NONE && (counter < timeOut ) ) {

		switch(esp->activeCommand) {
		case WLAN_COMMAND_ASK_READY:
			if (uart_buffer_find_delete("OK")) {
				esp->state = WLAN_STATE_READY;
				esp->activeCommand = WLAN_COMMAND_NONE;
				//switchLed(0, true);
				exec_ret = WLAN_OK;
				break;
			}
		break;
		case WLAN_COMMAND_SWITCH_MODE:
			if (uart_buffer_find_delete("OK")) {
				esp->state = WLAN_STATE_READY;
				esp->activeCommand = WLAN_COMMAND_NONE;
				exec_ret = WLAN_OK;
				break;
			}
			break;
		case WLAN_COMMAND_SET_STATIC_IP:
			if (uart_buffer_find_delete("OK")) {
				esp->state = WLAN_STATE_READY;
				esp->activeCommand = WLAN_COMMAND_NONE;
				exec_ret = WLAN_OK;
				break;
			}
			break;
		case WLAN_COMMAND_JOIN_ACCESSPOINT:
			if (uart_buffer_find_delete("WIFI CONNECTED")) {
				esp->state = WLAN_STATE_CONNECTED_AP;
				//esp->activeCommand stays JOIN_ACCESSPOINT as we still wait for the "got IP " message
			}
			//check ACK
			if (uart_buffer_find_delete("GOT IP")) {
					uart_buffer_find_delete("OK");
					esp->state = WLAN_STATE_CONNECTED_AP_GOT_IP;
					esp->activeCommand = WLAN_COMMAND_NONE;
					//switchLed(1, true);
					exec_ret = WLAN_OK;
					break;
			}
			if( uart_buffer_find_delete("FAIL" ) ) {
				uart_buffer_find_delete("+CWJAP:1");
				esp->state = WLAN_STATE_READY;
				esp->activeCommand = WLAN_COMMAND_NONE;
				//switchLed(1, false);
				exec_ret = AP_CONNECTION_FAILURE;
				break;
			}
			if( uart_buffer_find_delete("ERROR" )  ) {
				esp->state = WLAN_STATE_READY;
				esp->activeCommand = WLAN_COMMAND_NONE;
				//switchLed(1, false);
				exec_ret = AP_CONNECTION_FAILURE;
				break;
			}
			break;
		case WLAN_COMMAND_CONNECT_TO_TCP:
			//check for ACK
			//if (uart_buffer_find("OK") != 0) {
			if (uart_buffer_find_delete("ALREADY CONNECTED") ) {
				esp->state = WLAN_STATE_CONNECTED_TCP;
									esp->activeCommand = WLAN_COMMAND_NONE;
									//switchLed(2, true);
									exec_ret = WLAN_OK;
									break;
			}
			if (uart_buffer_find_delete("CONNECT") ) {
					uart_buffer_find_delete("OK");
					esp->state = WLAN_STATE_CONNECTED_TCP;
					esp->activeCommand = WLAN_COMMAND_NONE;
					//switchLed(2, true);
					exec_ret = WLAN_OK;
					break;
				}
			//}
			//check for NACK
			if (uart_buffer_find_delete("ERROR")) {
				uart_buffer_find_delete("CLOSED");
				esp->state = WLAN_STATE_CONNECTED_AP_GOT_IP;
				esp->activeCommand = WLAN_COMMAND_NONE;
				//switchLed(2, false);
				exec_ret = TCP_CONNECTION_FAILURE;
				break;
			}
			break;
		case WLAN_COMMAND_SEND_TCP:
			if (uart_buffer_find_delete("SEND OK")) {
				uart_buffer_find_delete("Recv");
				uart_buffer_find_delete("SEND OK");
				esp->activeCommand = WLAN_COMMAND_NONE;
				esp->state		   = WLAN_STATE_CONNECTED_TCP;
				return WLAN_OK;
			}
			if (uart_buffer_find_delete("link")) {
				uart_buffer_find_delete("ERROR");
				esp->activeCommand = WLAN_COMMAND_NONE;
				esp->state		   = WLAN_STATE_CONNECTED_AP_GOT_IP;
				//switchLed(2, false);
				exec_ret = TCP_CONNECTION_LOSS;
				break;
			}
			break;
		case WLAN_COMMAND_DISCONNECT_ACCESSPOINT:
			if (uart_buffer_find_delete("CLOSED")) {
				esp->state = WLAN_STATE_READY;
				esp->activeCommand = WLAN_COMMAND_NONE;
				//switchLed(1, false);
				exec_ret = WLAN_OK;
				break;
			}
			break;
		case WLAN_COMMAND_CHECK_TCP_STATUS:
			if (uart_buffer_find_delete("STATUS:2")) {
				//WIFI Connected and got Ip
				esp->state = WLAN_STATE_CONNECTED_AP_GOT_IP;
				esp->activeCommand = WLAN_COMMAND_NONE;
				exec_ret = WLAN_OK;
				break;
			} else if(uart_buffer_find_delete("STATUS:3")) {
				//TCP connected
				esp->state = WLAN_STATE_CONNECTED_TCP;
				esp->activeCommand = WLAN_COMMAND_NONE;
				//switchLed(2, true);
				exec_ret = WLAN_OK;
				break;
			} else if(uart_buffer_find_delete("STATUS:4")) {
				//TCP connection lost
				esp->state = WLAN_STATE_READY;
				esp->activeCommand = WLAN_COMMAND_NONE;
				//switchLed(1, false);
				//switchLed(2, false);
				exec_ret = WLAN_OK;
				break;

			}
		default:
			break;
		}
		counter ++;
	}
	counter = 0;
	return exec_ret;
}

void wlan_update(ESP8266_t* esp) {
	//ToDo check
	//if connection to AP was lost 	OR recovered
	//if TCP Connection was lost	OR recovered
	if (uart_buffer_find_delete("WIFI CONNECTED")) {
		esp->state = WLAN_STATE_CONNECTED_AP;
	}
	if (uart_buffer_find_delete("CLOSED")) {
		//lost connection to TCP-Server
		esp->state = WLAN_STATE_CONNECTED_AP_GOT_IP;
		//switchLed(2, false);
		disconnectedTcp();
	}
	if (uart_buffer_find_delete("WIFI GOT IP")) {
		esp->state = WLAN_STATE_CONNECTED_AP_GOT_IP;
	}
	if (uart_buffer_find_delete("WIFI DISCONNECTED")) {
		esp->state = WLAN_STATE_READY;
		//switchLed(1, false);
		//switchLed(2, false);
		disconnectedTcp();
	}
	if (uart_buffer_find_delete("SEND OK")) {
		uart_buffer_find_delete("Recv");
		uart_buffer_find_delete("SEND OK");
		esp->activeCommand = WLAN_COMMAND_NONE;
		esp->state		   = WLAN_STATE_CONNECTED_TCP;

	}
	if (uart_buffer_find_delete("link")) {
		uart_buffer_find_delete("ERROR");
		esp->activeCommand = WLAN_COMMAND_NONE;
		esp->state		   = WLAN_STATE_CONNECTED_AP_GOT_IP;
		//switchLed(2, false);

	}

	//ToDo check for other command returns
	//occasionally reset active command
}

void esp_update_tcp_state(ESP8266_t* esp_ptr) {
	char command[15]={0x00};
	strcat(command, "AT+CIPSTATUS");
	strcat(command,"\r\n");
	uart_send_string(command);
	esp_ptr->activeCommand = WLAN_COMMAND_CHECK_TCP_STATUS;
	wait_ms(100);
	wait_ready(esp_ptr);

}


