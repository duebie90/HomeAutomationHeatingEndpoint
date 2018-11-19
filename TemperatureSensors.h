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
    float temp_from_pt100_resistance(float);
    //ToDo: set correct values of sensor frontend
    const float sensor_gain = 31.5;
    const float measure_current_pt100 = 0.00261; //A
    // resistance coefficient for PT100 sensor
    const float u_ref = 0.339;
    const float V_uref = 49.5294;
    const float V_pt100 = 48.5294;
    const float r_koeff=2.5974;
    const float offset_v_mess = 0.0203; // difference though signal follow opv (first stage)

};

} /* namespace std */

#endif /* TEMPERATURESENSORS_H_ */
