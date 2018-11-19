

/*
 * MixerControl.c
 *
 *  Created on: 28.12.2017
 *      Author: admin
 */
#ifndef MIXERCONTROLLER_H
#define MIXERCONTROLLER_H
#include <string>
#include "TemperatureSensors.h"
extern "C" {
#include "general.h"
#include "HomeAutomation.h"
}

using std::string;
using namespace std;

enum MixerCommand {forward_short, backward_short, forward_long, backward_long };

class MixerController  {
    public:
        MixerController();
        void update_temperatures(void);
        void control_temperature(void);
        void test_relay(void);
        void send_temp_update();
        void set_target_temp(float);
        void set_temp_threshold(float);
    private:
        void move_mixer(MixerCommand direction);
        float current_temp_heating;
        float current_temp_boiler;
        float target_temp_heating;
        float temp_threshold_boiler;
        TemperatureSensors* temperatureSensors;
        const signed hysteresis1 = 1; // in either direction
        const signed hysteresis2 = 3;

};

#endif



