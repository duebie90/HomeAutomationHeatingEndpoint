

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
}

using std::string;
using namespace std;

enum MixerCommand {forward_short, backward_short, forward_long, backward_long };

class MixerController  {
    public:
        MixerController();
        void control_temperature(void);
        void test_relay(void);
    private:
        int current_temp;
        void move_mixer(MixerCommand direction);
        TemperatureSensors* temperatureSensors;

};

#endif



