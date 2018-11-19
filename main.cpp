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
bool bootFailed = false;
ESP8266_t esp = { WLAN_STATE_IDLE, WLAN_COMMAND_NONE };
MixerController* mc;

void try_connect_to_ap();

extern "C" void proxy_set_target_influx_mixer_controller(float target_temp){
	mc->set_target_temp(target_temp);
}

// HomeAutomation Standard-main ToDo: replace
int main(void) {
	initSystem();
	uart_init();
	//switch own Baudrate of UART1 to 9600Baud
	initUart0();
	homeAutomation_init(&esp);

	//
	mc = new MixerController();

	//callback_target_influx_temp_received = &mc->set_target_temp;
	set_callback_target_influx_temp(proxy_set_target_influx_mixer_controller);


	//TEST OUTPUT
	//mc->test_relay();
	//RELAY_SOCKET_ON;
	espSetHardReset(true);
	wait_ms(100);
	espSetHardReset(false);
	wait_ms(500);

	if (!esp_init_baudrates(&esp)) {
		return -1;
		//init failed
	} else {
		uart_send_string("ATE0\r\n");
		wait_ms(10);
		uart_buffer_clear();
		wlan_set_default_mode(&esp, 1);
		uart_buffer_clear();
		wait_ms(10);
	}
	uart_buffer_clear();
	try_connect_to_ap();
	bootedOnce = true;
	//if not TCP-socket can be opened sometimes it helps to reconnect to wifi
	wlan_connect_to_tcp_server(&esp, 1, SERVER_IP_1, SERVER_PORT);
	current_tcp_server = 1;
	wait_ms(200);
	while (wait_ready(&esp) != WLAN_OK) {
		bootedOnce = false;
		uart_buffer_clear();
		//try_connect_to_ap();

		//wait_ms(5000);
		//retry to connect to tcp-server
		wlan_connect_to_tcp_server(&esp, 1, SERVER_IP_1, SERVER_PORT);
		current_tcp_server = 2;
		wait_ms(200);
	}
	uart_buffer_clear();

	//register at homeAutomation server
	sendIdentMessage(&esp, DEFAULT_ALIAS, MAC, TYPE);
	//sendMessage(&esp, MESSAGETYPE_ENDPOINT_IDENT, identMessagePayload);
	wait_ms(1000);
	if (wait_ready(&esp) != WLAN_OK) {
		bootedOnce = false;
		disconnectedTcp();
		uart_buffer_clear();
		//TCP sockets seems to be broken

		do {
			//try to reestablish it
			wlan_disconnect_from_tcp_server(&esp);
			wait_ms(1000);
			uart_buffer_clear();
			wlan_connect_to_tcp_server(&esp, 1, SERVER_IP_1, SERVER_PORT);
			current_tcp_server = 1;
			wait_ms(2000);
			while (wait_ready(&esp) != WLAN_OK) {
				//WIFI Connection seems to be lost too
				try_connect_to_ap();

				wait_ms(5000);
				//retry to connect to tcp-server
				wlan_connect_to_tcp_server(&esp, 1, SERVER_IP_1, SERVER_PORT);
				current_tcp_server = 2;
				wait_ms(5000);
			}
			uart_buffer_clear();
			bootedOnce = true;
			sendIdentMessage(&esp, DEFAULT_ALIAS, MAC, TYPE);
			wait_ms(1000);
			//sendMessage(&esp, MESSAGETYPE_ENDPOINT_IDENT, identMessagePayload);
		} while (wait_ready(&esp) != WLAN_OK);
	}

	//uart_send_string("AT+UART_DEF=9600,8,1,0,0\r\n");

	// "Operating System": distributing tasks/ time-slices
	bootedOnce = true;
	switchLed(true);
	while (1) {
		//parsing received data
		//eventually callback HomeAutomation parseReceivedData()
		wlan_update(&esp);
		esp_update(&esp);
		char ipd_data[UART_BUFFER_MAX_STRINGLENGTH] = { NULL };
		if (uart_buffer_find_delete_tcp_payload(ipd_data)) {
			parseReceivedData(ipd_data);
		}
		homeAutomation_update();
		mc->control_temperature();

	}

	return 0;
}

void try_connect_to_ap() {
	wlan_connect_to_ap(&esp, SSID, PASSWORD);
	wait_ms(5000);
	while (wait_ready(&esp) != WLAN_OK) {
		//Connecting failed: try once again
		wlan_connect_to_ap(&esp, SSID, PASSWORD);
		wait_ms(1000);
		//wait_ms(10000);
		if (wait_ready(&esp) == WLAN_OK) {
			break;
		}
		wlan_disconnect_from_tcp_server(&esp);
		wait_ms(100);
		wlan_connect_to_ap(&esp, SSID, PASSWORD);
		wait_ms(100);
		if (wait_ready(&esp) == WLAN_OK) {
			break;
		}
		//Connecting failed: Soft-Reset ESP, wait for ACK and try again
		//switchLed(0, false);
		if (!esp_init(&esp)) {
			return;
			//init failed
		}

		wlan_connect_to_ap(&esp, SSID, PASSWORD);
		wait_ms(2000);
		//wait_ms(10000);
	}
	uart_buffer_clear();
}

//callled ###
#pragma vector=TIMERA0_VECTOR
__interrupt void timerA_CCR0(void) {
	if (isRegisteredAtServer() == true
			&& esp.state == WLAN_STATE_CONNECTED_TCP) {
		if (esp_tcp_state_update_timer == 0) {
			mc->update_temperatures();
			mc->send_temp_update();
			//wait_ready(&esp);
			sendStateChangeNotification(true);
			esp_tcp_state_update_timer = 20;
		} else {
			esp_tcp_state_update_timer--;
		}
	} else if (bootFailed == false) {
		STATUS_LED_TOGGLE;
	} else {
		STATUS_LED_OFF;
	}

}
