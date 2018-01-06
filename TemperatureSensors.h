/*
 * TemperatureSensors.h
 *
 *  Created on: 28.12.2017
 *      Author: admin
 */

#ifndef TEMPERATURESENSORS_H_
#define TEMPERATURESENSORS_H_

#include "System.h"
#include <math.h>
#include "MSP430F149.h"
namespace std {

class TemperatureSensors {
public:
    TemperatureSensors();
    virtual ~TemperatureSensors();
    float get_temp_value(int);
private:
    float temp_from_adc_value(unsigned int);
    unsigned int read_adc_value(unsigned int);
    //ToDo: set correct values of sensor frontend
    const float sensor_gain = 0.0;
    const float measure_current_pt100 = 0.0;

};

} /* namespace std */

#endif /* TEMPERATURESENSORS_H_ */
