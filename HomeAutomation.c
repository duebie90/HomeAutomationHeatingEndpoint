/*
 * HomeAutomation.c
 *
 *  Created on: 02.05.2016
 *      Author: Manuel
 *      Specific functions to communicate with HomeAutomationServer are provided
 */
#include "HomeAutomation.h"


const static char termination[5] = {0x03, 0x04, 0x0D, 0x0A, 0x00};

static bool isSwitchedOn = false;

static char* endpointAlias = 0x00;

static ESP8266_t* esp_ref = NULL;

static bool registeredAtServer = false;

static bool switchedState = false;

//char identMessagePayload[50] = {0x00 };

int current_tcp_server = 0;

void homeAutomation_init(ESP8266_t* esp_ptr) {
	isSwitchedOn = false;
	//save pointer on ESP structure
	esp_ref = esp_ptr;
	endpointAlias = DEFAULT_ALIAS;

}

void disconnectedTcp() {
	registeredAtServer = false;
	//switchLed(3, false);
}

bool isRegisteredAtServer() {
	return registeredAtServer;
}

void sendMessage(ESP8266_t* esp_ptr, MessageType type, char* payload) {
	//build message header around payload
	char message[100];
	//send message to server via TCP
	const uint16_t payloadLength = strlen(payload);
	message[0] = 0x01;
	message[1] = (char)type;

	if( (payloadLength >> 8) == 0) {
		message[2] = 0xFF;
	} else {
		message[2] = 	(char)(payloadLength >>8); 	//upper byte
	}

	message[3] =	(char)payloadLength;		//lower byte
	message[4] = 0x02;
	message[5] = 0;
	strcat(message, payload);
	strcat(message, termination);
	wlan_tcp_send_string(esp_ptr, message);
}

void sendIdentMessage(ESP8266_t* esp_ptr, char alias[], char mac[], char type[]) {
	char identMessagePayload[50] = {0x00 };

	memset(identMessagePayload, 0, 50);
	identMessagePayload[0] = 0x00;

	strcat(identMessagePayload, alias);

	identMessagePayload[strlen(identMessagePayload)] = PDU_DELIMITER;
	identMessagePayload[strlen(identMessagePayload)+1] = '\0';

	strcat(identMessagePayload, MAC);

	identMessagePayload[strlen(identMessagePayload)] = PDU_DELIMITER;
	identMessagePayload[strlen(identMessagePayload)+1] = '\0';

	strcat(identMessagePayload, type);

	sendMessage(esp_ptr, MESSAGETYPE_ENDPOINT_IDENT, identMessagePayload);
}


void parseReceivedData(char* received) {
	char returnPayload[50] = {0x00};
	MessageType type;
	type = parseProtokollHeader(received, returnPayload);
	if (type !=NULL) {
			parseMessage(type, returnPayload);
	}
}

MessageType parseProtokollHeader(char* received, char returnPayload[]) {
	uint16_t payloadLength = 0;
	if (received[0] != 0x01) {
		return NULL;
	}
	MessageType type = received[1];
	if (type == 0x00) {
		return NULL;
	}
	uint8_t payloadLengthUpper = *(received + 2);
	uint8_t payloadLengthLower = *(received + 3);
	if (payloadLengthUpper != 0xFF) {
		payloadLength  |= payloadLengthUpper;
		payloadLength   = payloadLength << 8;
	}
	payloadLength |= payloadLengthLower;

	char splitOfPayload[50];
	strcpy(splitOfPayload, (received + HEADER_LENGTH) );


	char payloadEndDelimiter[1];
	payloadEndDelimiter[0] = 0x03;

	strcpy(returnPayload, strtok(splitOfPayload,payloadEndDelimiter));

	unsigned int measuredPayloadLength = strlen(returnPayload);
	if(measuredPayloadLength != (unsigned int)payloadLength) {
		//invalid payload section
		return NULL;
	}
	return type;
}

void parseMessage(MessageType type, char payload[]) {
	  //char *mac, *state;
	    //make a copy of payload, because strtok will manipulate it
	    char payloadCopy[50] = {0x00};
	    strcpy(payloadCopy, payload);
	    char payloadInnerDelimiter[2];
	    payloadInnerDelimiter[0] = PDU_DELIMITER;
	    payloadInnerDelimiter[1] = 0x00;

	    switch(type) {
	    case MESSAGETYPE_ENDPOINT_IDENT_ACK: {
	    	char* mac = strtok(payloadCopy, payloadInnerDelimiter);
	    	if( strstr(mac, MAC )!=NULL && mac != NULL)  {
	    		registeredAtServer = true;
	    		//switchLed(3, true);
	    	}
	    }
	        break;
	    case MESSAGETYPE_SERVER_ENDPOINT_STATE_REQUEST:{
	        char * mac     =   strtok(payloadCopy, payloadInnerDelimiter);
	        char * state   =   strtok(NULL, payloadInnerDelimiter);
	        if(mac != NULL && state != NULL) {
	            //ToDo: compare mac address
	            if(state[0] == '1') {
	                switchState(true);
	            }
	            else {
	            	switchState(false);
	            }
	        }
	    }
	        break;
	    default:
	        break;
	    }
}


void homeAutomation_update() {
	//ToDo update state machine
	//update switched state etc
}


void esp_update(ESP8266_t* esp_ptr) {
    switch(esp_ptr->state) {
    case WLAN_STATE_READY: {
        //repeatedly try to connect until it was acknowledged
        unsigned int reconnectCounter = 3;
        do {
            reconnectCounter--;
            if (reconnectCounter == 0) {
                //Reboot ESP until it ACKs
                while(esp_init(esp_ptr) != WLAN_OK) {
                    wait_ms(2000);
                }
                reconnectCounter = 3;
            }
            wlan_connect_to_ap(esp_ptr, SSID, PASSWORD);
            wait_ms(10000);
        } while (wait_ready(esp_ptr)!=WLAN_OK);
        wait_ms(5000);
    }
    break;
    case WLAN_STATE_CONNECTED_AP:
        //nothing to do: wait for IP
        break;
    case WLAN_STATE_CONNECTED_AP_GOT_IP:
        wait_ms(5000);
        if(current_tcp_server == 1){
            wlan_connect_to_tcp_server(esp_ptr, 1,SERVER_IP_1, SERVER_PORT);
        }
        else{
                wlan_connect_to_tcp_server(esp_ptr, 1,SERVER_IP_2, SERVER_PORT);
        }

        wait_ms(2000);
        if (wait_ready(esp_ptr) != WLAN_OK) {
            //eventually we also lost Wifi connection
            esp_ptr->state = WLAN_STATE_READY;
        }
        break;
    case WLAN_STATE_CONNECTED_TCP:
        if (isRegisteredAtServer() == false) {
             sendIdentMessage(esp_ptr, DEFAULT_ALIAS, MAC, TYPE);
             //sendMessage(esp_ptr, MESSAGETYPE_ENDPOINT_IDENT, identMessagePayload);

            if(wait_ready(esp_ptr) != WLAN_OK) {
                esp_ptr->state = WLAN_STATE_CONNECTED_AP_GOT_IP;
                //switchLed(2, false);
            }
            //ToDo: The message was successfully sent, now go asleep for a certain time and retry if still no response
            wait_ms(1000);
        }
        break;
    default:
        break;
    }
}

// Preliminary this is also built in here ToDo: Remove
void switchState(bool state) {
    esp_tcp_state_update_timer = 0;
//    if (state == AC_SENSE_CHECK) {
//        //state != current state
//        RELAY_LIGHT_TOGGLE;
//        switchLed(4, state);
//        switchedState = state;
//    }
//    wait_ms(200);
    //wait until new state is stedy
    //sendStateChangeNotification();
  if (state != isSwitchedOn) {
      if(state == true) {
          //switch on
          RELAY_LIGHT_ON;
          isSwitchedOn = true;

      } else {
          //switch off
          RELAY_LIGHT_OFF;
          isSwitchedOn = false;
      }
      sendStateChangeNotification();
  }
}

bool getSwitchState() {
    return switchedState;
}

void sendStateChangeNotification() {
    char notificationMessage[20] = {0x00};

    strcat(notificationMessage, MAC);

    notificationMessage[strlen(notificationMessage)] = PDU_DELIMITER;
    notificationMessage[strlen(notificationMessage)+1] = '\0';

    char stateChar[2] = {'0',0};
    //if (isSwitchedOn) {
//    if (!AC_SENSE_CHECK) {
//        stateChar[0] = '1';
//    }
    if(isSwitchedOn){
        stateChar[0] = '1';
    }
    strcat(notificationMessage, stateChar);

    sendMessage(esp_ref, MESSAGETYPE_ENDPOINT_STATE, notificationMessage);
}



