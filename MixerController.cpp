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
current_temp_heating(0),
temp_threshold_boiler(80),
target_temp_heating(45)
{
    temperatureSensors = new TemperatureSensors();
}


void MixerController::update_temperatures(void){
	//read the temperatures and store them for later
	this->current_temp_heating = temperatureSensors->get_temp_value((unsigned int)HEATING_TEMP_SENSOR);
	this->current_temp_boiler = temperatureSensors->get_temp_value((unsigned int)BOILER_TEMP_SENSOR);
}

void MixerController::control_temperature(void) {
	//control heater influx temperature
	float delta_t = this->current_temp_heating - this->target_temp_heating;
	MixerCommand command;
	bool action_required = false;
	if(delta_t > hysteresis2) {//Temperatur deutlich zu hoch
		command = forward_long;
		action_required = true;
	} else if (delta_t < (-hysteresis2)){//Temperatur deutlich zu niedrig
		command = backward_long;
		action_required = true;
	} else if(delta_t > hysteresis1) {//Temperatur liegt innerhalb der groﬂen Hysterese
		//Temperatur etwas zu hoch
		command = forward_short;
		action_required = true;
	}else if (delta_t < (-hysteresis1)){
		//Temperatur etwas zu niedrig
		command = backward_short;
		action_required = true;
	}

    //example
	if(action_required){
		this->move_mixer(command);
	}
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
    uint16_t current_temp_heating_fixed_point = (uint16_t)(this->current_temp_heating*100);

    if( (current_temp_heating_fixed_point >> 8) == 0) {
    	payload[strlen(payload)] = 0xFF;
    } else {
    	payload[strlen(payload)] = 	(char)(current_temp_heating_fixed_point >>8); 	//upper byte
    }
    payload[strlen(payload)] =	(char)current_temp_heating_fixed_point;	//lower byte

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

extern "C" void MixerController::set_target_temp(float target_temp){
	this->target_temp_heating = target_temp;
}

void MixerController::set_temp_threshold(float th_temp){
	this->temp_threshold_boiler = th_temp;
}






