extern "C" {
    #include <msp430f149.h>
    #include <stdbool.h>

    #include "System.h"
    #include "uart.h"
    #include "HomeAutomation.h"
    #include "ESP8266.h"
    #include <string.h>
    #include "general.h"
}
#include "MixerController.h"

/*
 * main.c
 */

__interrupt void timerA_CCR0(void);
bool bootedOnce = false;
ESP8266_t esp = {.activeCommand = WLAN_COMMAND_NONE, .state = WLAN_STATE_IDLE };

int main(void) {
    initSystem();
    uart_init();

    //homeAutomation_init(&esp);

    switchLed(true);

    MixerController mc;
    //TEST OUTPUT
    mc.test_relay();


    mc.control_temperature();
    while(1){;}
	return 0;
}


//callled ###
#pragma vector=TIMERA0_VECTOR
__interrupt void timerA_CCR0(void) {
    if(bootedOnce == true) {


    }
}
