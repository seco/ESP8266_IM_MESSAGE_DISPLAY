
#include <osapi.h>
#include <ets_sys.h>			//NEEDS TO BE THERE IN EVERY ESP8266 PROGRAM
#include "user_interface.h"		//NEEDS TO BE THERE IN EVERY ESP8266 PROGRAM
#include "uart.h"

void esp8266_init_complete(void);
//void set_wifi_parameters(void);
//void wifi_event_handler_function(System_Event_t* event);
void setupPins(void);

void user_init(void)
{
	//set usart0 to 115200bps
	//no need to set pin functionality as GPIO1
	//by default is set to TXD0
	uart_div_modify(0, UART_CLK_FREQ / 115200);

	//register callback function for when environment
	//100% functional
	system_init_done_cb(esp8266_init_complete);
}

void setupPins(void)
{
	//set pins for gpio and peripherals
}

void esp8266_init_complete(void)
{
	os_printf("ESP8266 module init complete\n");

	//print some basic information about the module
	char system_mac[6];
	wifi_get_macaddr(STATION_IF, system_mac);

	os_printf("Module MAC ADDRESS : %02x:%02x:%02x:%02x:%02x:%02x\n", system_mac[0], system_mac[1], system_mac[2], system_mac[3], system_mac[4], system_mac[5]);
}

/*
void module_init_complete(void)
{
	//at this point the module initialization
	//is 100% done


	//setup UART0
	//UART_SetBaudrate(0, BIT_RATE_9600);
	//UART_SetParity(0, STICK_PARITY_DIS);
	//UART_SetStopBits(0, ONE_STOP_BIT);
	//UART_SetWordLength(0, EIGHT_BITS);
	//uart_init(BIT_RATE_9600, BIT_RATE_9600);
	//UART_SetPrintPort(0);

	os_printf("system init complete\n");




	//set a custom wifi event handler function
	wifi_set_event_handler_cb(wifi_event_handler_function);

	set_wifi_parameters();
}

void wifi_event_handler_function(System_Event_t* event)
{
	//custom wifi event handler function
	//simple print the event information on debug uart1

	switch(event->event)
	{
		case EVENT_STAMODE_CONNECTED:
			os_printf("EVENT : EVENT_STAMODE_CONNECTED\n");
			break;

		case EVENT_STAMODE_DISCONNECTED:
			os_printf("EVENT : EVENT_STAMODE_DISCONNECTED\n");
			break;

		case EVENT_STAMODE_AUTHMODE_CHANGE:
			os_printf("EVENT : EVENT_STAMODE_AUTHMODE_CHANGE\n");
			break;

		case EVENT_STAMODE_GOT_IP:
			os_printf("EVENT : EVENT_STAMODE_GOT_IP\n");

			break;

		case EVENT_SOFTAPMODE_STACONNECTED:
			os_printf("EVENT : EVENT_SOFTAPMODE_STACONNECTED\n");
			break;

		case EVENT_SOFTAPMODE_STADISCONNECTED:
			os_printf("EVENT : EVENT_SOFTAPMODE_STADISCONNECTED\n");
			break;

		default:
			os_printf("Unknown EVENT %d\n", event->event);
	}
}

void set_wifi_parameters(void)
{
	//set the wifi parameters

	wifi_set_opmode(STATION_MODE);
	struct station_config stationConfig;
	strncpy(stationConfig.ssid, "ankit", 32);
	strncpy(stationConfig.password, "casa2016", 64);
	wifi_station_set_config(&stationConfig);

	//disable auto connect
	wifi_station_set_auto_connect(FALSE);

	//connect to the access point
	wifi_station_connect();
}
*/

uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}


