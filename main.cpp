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

void try_connect_to_ap();

//int main(void) {
//    initSystem();
//    uart_init();
//
//    //homeAutomation_init(&esp);
//
//    switchLed(true);
//
//    MixerController mc;
//    //TEST OUTPUT
//    mc.test_relay();
//
//
//    mc.control_temperature();
//    while(1){;}
//	return 0;
//}


// HomeAutomation Standard-main ToDo: replace
int main(void) {
    initSystem();
    uart_init();

    homeAutomation_init(&esp);

    //
    MixerController mc;
    //TEST OUTPUT
    mc.test_relay();
    //RELAY_SOCKET_ON;

    espSetHardReset(true);
    wait_ms(20);
    espSetHardReset(false);
    wait_ms(2000);

    //  //Konfigure UART1 for 115200Baud (ESP8266 default Baudrate)
 //   while(true){
    initUart115KBaud();
//    wait_ms(500);
//    send_config_baudrate(9600);
//    wait_ms(1000);
    //initUart38_4KBaud();
    //wait_ms(500);
    //send_config_baudrate(9600);
    //wait_ms(1000);
  //  }
    //  switch own Baudrate of UART1 to 9600Baud again
    //initUart0();


    if ( !esp_init(&esp) ){
            return -1;
            //init failed
        }
    try_connect_to_ap();

    //if not TCP-socket can be opened sometimes it helps to reconnect to wifi
        wlan_connect_to_tcp_server(&esp, 1,SERVER_IP_1, SERVER_PORT);
        current_tcp_server = 1;
        wait_ms(2000);
        while(wait_ready(&esp)!=WLAN_OK) {


            try_connect_to_ap();

            wait_ms(5000);
            //retry to connect to tcp-server
            wlan_connect_to_tcp_server(&esp, 1,SERVER_IP_2, SERVER_PORT);
            current_tcp_server = 2;
            wait_ms(2000);
        }
        wait_ms(500);

        //register at homeAutomation server
        sendIdentMessage(&esp, DEFAULT_ALIAS, MAC, TYPE);
        //sendMessage(&esp, MESSAGETYPE_ENDPOINT_IDENT, identMessagePayload);

        if (wait_ready(&esp) != WLAN_OK) {
            disconnectedTcp();
            //TCP sockets seems to be broken

            do {
                //try to reestablish it
                wlan_connect_to_tcp_server(&esp, 1,SERVER_IP_1, SERVER_PORT);
                current_tcp_server = 1;
                wait_ms(2000);
                while(wait_ready(&esp)!=WLAN_OK) {
                    //WIFI Connection seems to be lost too
                    try_connect_to_ap();

                    wait_ms(5000);
                    //retry to connect to tcp-server
                    wlan_connect_to_tcp_server(&esp, 1,SERVER_IP_2, SERVER_PORT);
                    current_tcp_server = 2;
                    wait_ms(5000);
                }
                sendIdentMessage(&esp, DEFAULT_ALIAS, MAC, TYPE);
                //sendMessage(&esp, MESSAGETYPE_ENDPOINT_IDENT, identMessagePayload);
            } while(wait_ready(&esp) == WLAN_OK);
        }

        //uart_send_string("AT+UART_DEF=9600,8,1,0,0\r\n");

        // "Operating System": distributing tasks/ time-slices
        bootedOnce = true;
        switchLed(true);
        while(1){
            //parsing received data
            //eventually callback HomeAutomation parseReceivedData()
            wlan_update(&esp);
            esp_update(&esp);
            char ipd_data[UART_BUFFER_MAX_STRINGLENGTH] = {NULL};
            if (uart_buffer_find_delete_tcp_payload(ipd_data)) {
                parseReceivedData(ipd_data);
            }
            homeAutomation_update();

        }

    return 0;
}

void try_connect_to_ap() {
    wlan_connect_to_ap(&esp, SSID, PASSWORD);
    //wait_ms(5000);
    while(wait_ready(&esp)!=WLAN_OK) {
        //Connecting failed: try once again
        wlan_connect_to_ap(&esp, SSID, PASSWORD);
        wait_ms(1000);
        //wait_ms(10000);
        if (wait_ready(&esp)==WLAN_OK) {
            break;
        }
        //Connecting failed: Soft-Reset ESP, wait for ACK and try again
        //switchLed(0, false);
        if ( !esp_init(&esp) ){
            return;
            //init failed
        }

        wlan_connect_to_ap(&esp, SSID, PASSWORD);
        wait_ms(2000);
        //wait_ms(10000);
    }
}

//callled ###
#pragma vector=TIMERA0_VECTOR
__interrupt void timerA_CCR0(void) {
    if(bootedOnce == true) {
        if (esp_tcp_state_update_timer ==0 ) {
            if(esp.activeCommand == WLAN_COMMAND_NONE) {
                sendStateChangeNotification();
                esp.activeCommand = WLAN_COMMAND_NONE;
                esp_tcp_state_update_timer = 30;
                       }
             } else {
                esp_tcp_state_update_timer--;
            }
        }else{
            STATUS_LED_TOGGLE;
        }
}
