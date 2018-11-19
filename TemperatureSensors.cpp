/*
 * TemperatureSensors.cpp
 *
 *  Created on: 28.12.2017
 *      Author: admin
 */

#include "TemperatureSensors.h"
namespace std {

TemperatureSensors::TemperatureSensors() {


}

float TemperatureSensors::get_temp_value(int sensor_number){
    float adc_value = 0.0;
    if (sensor_number == BOILER_TEMP_SENSOR)  {
        adc_value = read_adc_value(sensor_number);
        return temp_from_adc_value(adc_value);
    }else if (sensor_number == HEATING_TEMP_SENSOR) {
        adc_value = read_adc_value(sensor_number);
        float temp = temp_from_adc_value(adc_value);
        return temp;
    } else{
        // unknown sensor number
        return 0.0;
    }
    return 0.0;
}

unsigned int TemperatureSensors::read_adc_value(unsigned int sensor_number){
    // select the proper input channel to store into 1st conversional memory (of 16)
    if (sensor_number == BOILER_TEMP_SENSOR){
    	//channel 4
            ADC12MCTL0 &= ~(INCH0 + INCH1 + INCH2 + INCH3);
            ADC12MCTL0 |=  INCH2;
    }
    else if (sensor_number == HEATING_TEMP_SENSOR)  {
		// channel 3
    	ADC12MCTL0 &= ~(INCH0 + INCH1 + INCH2 + INCH3);
    	ADC12MCTL0 |= INCH0 + INCH1;

    } else{
        //do the same
        ADC12MCTL0 |= HEATING_TEMP_SENSOR;
    }
    //enable interrupt of first conversion memmory
    ADC12IE |= BIT0;

    //enable ADC12
    ADC12CTL0 |= ENC;

    _delay_cycles(0xFF);

    __bis_SR_register(GIE);
    // start conversion
    ADC12CTL0 |= ADC12SC;
    // wait until finished (interrupt flag)

    while((ADC12IFG & BIT0)==0){
        __no_operation();
    }

    //disable ADC12
    ADC12CTL0 &= ~ENC;

    return ADC12MEM0;


    //    //ToDo utilize adc number
//    //TA0CCTL0 &= ~CCIFG;
//    ADC10CTL0       =       SREF_0+ADC10SHT_2+ REF2_5V + REFON + ADC10ON;// + ADC10IE;
//    ADC10CTL1      &=       ~(INCH3 + INCH2 + INCH1 + INCH0); //Auswahl Channel 0
//    ADC10CTL1      |=        INCH_1; //Auswahl Channel 0
//    //ADC10CTL1      |=      (INCH3           INCH1        ); //Wählt den internen  Temperatursensor aus
//    TACTL &= ~MC0; //Timer anhalten
//    TAR=0;
//    ADC10CTL0       |=      ENC + ADC10SC; //Startet die Wandlung
//    //AUF ADC10 Warten
//    while((ADC10CTL0 & ADC10IFG)==0){;}
//    return ADC10MEM;
}

float TemperatureSensors::temp_from_adc_value(unsigned int adc_value){
    float R_PT100;
    float messspannung_temp=0;
    messspannung_temp=(float)adc_value/1638.4;//Spannungswert aus ADC Wert ausrechnen Referenzspannung 3,23V
    //Via Messspannung R_PT100 berechnenb
    //1. von der Messspannung zur Sensorspannung (Differenz OPV)
    messspannung_temp = u_ref*V_uref - messspannung_temp;
    messspannung_temp /= V_pt100;
    messspannung_temp -= offset_v_mess;
    //2. Zum Widerstand des PT100
    R_PT100 = messspannung_temp/ measure_current_pt100;
    //3. aus RPT100 Temperatur berechnen
    return temp_from_pt100_resistance(R_PT100);
}

float TemperatureSensors::temp_from_pt100_resistance(float pt100_resistance){
    float temp_celsius=(pt100_resistance-100.0);
    temp_celsius *= this->r_koeff;
    return temp_celsius;
}

#pragma vector=ADC12_VECTOR
__interrupt void ADC10_ISR(void){
    __bic_SR_register_on_exit(GIE);
}

// Port 1 interrupt service routine
//#pragma vector=PORT1_VECTOR
//__interrupt void Port_1(void)
//{
//    char sollwert_temp=0;
//    sollwert_temp=round(calc_temp_value(start_conv()));
//
//      FCTL3 = FWKEY;                            // Clear Lock bit
//      FCTL1 = FWKEY + WRT;
//      char * ptr_sollwert = (char *)0x0F000; //Anfang des Flash speichers
//      *ptr_sollwert = sollwert_temp;
//      P1IFG &= ~0x10;                           // P1.4 IFG cleared
//}

TemperatureSensors::~TemperatureSensors() {
    // TODO Auto-generated destructor stub
}

} /* namespace std */
