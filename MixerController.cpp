#include "MixerController.h"
/*
 * MixerController.cpp
 *
 *  Created on: 28.12.2017
 *      Author: Manuel
 *
 */

MixerController::MixerController():
current_temp(0) {
    temperatureSensors = new TemperatureSensors();
}


void MixerController::control_temperature(void) {
    temperatureSensors->get_temp_value((unsigned int)HEATING_TEMP_SENSOR);

    // can be used as callback from timer ISR
        // read adc value
        // create control output

    //example
    MixerCommand command = forward_short;
    this->move_mixer(command);
    }

void MixerController::test_relay(void){
    move_mixer(forward_long);
    wait_ms(500);
    move_mixer(backward_long);
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






