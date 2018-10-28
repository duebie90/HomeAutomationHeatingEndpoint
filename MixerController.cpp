#include "MixerController.h"
/*
 * MixerController.cpp
 *
 *  Created on: 28.12.2017
 *      Author: Manuel
 *
 */

MixerController::MixerController():
current_temp_boiler(0),
current_temp_heating(0){
    temperatureSensors = new TemperatureSensors();
}


void MixerController::control_temperature(void) {
	this->current_temp_heating = temperatureSensors->get_temp_value((unsigned int)HEATING_TEMP_SENSOR);
	this->current_temp_boiler = temperatureSensors->get_temp_value((unsigned int)BOILER_TEMP_SENSOR);


    // can be used as callback from timer ISR
        // read adc value
        // create control output

    //example
    MixerCommand command = forward_short;
	//this->move_mixer(command);
    }

void MixerController::test_relay(void){
    move_mixer(forward_long);
    wait_ms(500);
    move_mixer(backward_long);
}

void MixerController::send_temp_update(){
    char payload[50] = {0x00 };
    memset(payload, 0, 50);
    strcat(payload, MAC);
    payload[strlen(payload)] = PDU_DELIMITER;
    payload[strlen(payload)] = (char)this->current_temp_heating;
    payload[strlen(payload)+1] = '\0';

    sendMessage(MESSAGETYPE_ENDPOINT_SERVER_HEATING_TEMP, payload, true);
}

void MixerController::move_mixer(MixerCommand direction){
    switch(direction){
        case forward_short:
            RELAY_FW_ON;
            wait_ms(500);
            RELAY_FW_OFF;
            break;
        case forward_long:
            RELAY_FW_ON;
            wait_ms(1000);
            RELAY_FW_OFF;
            break;
        case backward_short:
            RELAY_BW_ON;
            wait_ms(500);
            RELAY_BW_OFF;
            break;
        case backward_long:
            RELAY_BW_ON;
            wait_ms(1000);
            RELAY_BW_OFF;
            break;

    }


}






