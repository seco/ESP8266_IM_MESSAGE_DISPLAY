/*************************************************
* ESP8266 IM MESSAGE DISPLAY
*
* OCTOBER 21 2016
* ANKIT BHATNAGAR
* ANKIT.BHATNAGARINDIA@GMAIL.COM
* ***********************************************/

//////////////////////////////////
//INCLUDES
/////////////////////////////////
#include <osapi.h>
#include <ets_sys.h>			//NEEDS TO BE THERE IN EVERY ESP8266 PROGRAM
#include "user_interface.h"		//NEEDS TO BE THERE IN EVERY ESP8266 PROGRAM
#include "uart.h"
#include "smartconfig.h"
#include "ESP8266_SPI.h"

//////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////
void esp8266_init_complete(void);
//void set_wifi_parameters(void);
void setup_gpio_pins(void);
void timer_led_callback(void *pArg);
void wifi_event_handler_function(System_Event_t* event);
void smartconfig_done_function(sc_status status, void* pdata);

//////////////////////////////////
//GLOBAL VARIABLES
//////////////////////////////////
os_timer_t timer_led;

void user_init(void)
{
	//SET USART0 BAUD RATE TO 115200 BPS
	//BY DEFAULT PIN GPIO1 SET TO UART0 TXD FUNCTIONALITY
	uart_div_modify(0, UART_CLK_FREQ / 115200);

	//SET UP GPIO PONS
	setup_gpio_pins();

	//REGISTER CALLBACK FUNCTION FOR ENVIRONMENT INITIALIZATION DONE
	system_init_done_cb(esp8266_init_complete);
}

void setup_gpio_pins(void)
{
	//INITIAL SETUP FOR GPIO PINSs
	//GPIO-D3 : OUTPUT : LED
	//GPIO-D5 : INPUT : MODE SELECT BUTTON (NORMAL / SMARTCONFIG)

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
}

void esp8266_init_complete(void)
{
	os_printf("ESP8266 module init complete\n");

	//PRINT SOME BADIC INFORMATION ABOUT THE MODULE
	char system_mac[6];
	wifi_get_macaddr(STATION_IF, system_mac);
	os_printf("Module MAC ADDRESS : %02x:%02x:%02x:%02x:%02x:%02x\n", system_mac[0], system_mac[1], system_mac[2], system_mac[3], system_mac[4], system_mac[5]);

	//SET CUSTOM WIFI EVENT HANDLER FUNCTION
	wifi_set_event_handler_cb(wifi_event_handler_function);

	//DETERMINE WEATHER TO START IN STATION SMARTCONFIG MODE
	if(GPIO_INPUT_GET(5) == 1)
	{
		//MODE SELECTION BUTTON PRESSED
		//SMARTCONFIG MODE
		os_printf("smartconfig mode\n");
		//START TIMER TO TOGGLE LED @ 2Hz
		os_timer_setfn(&timer_led, timer_led_callback, NULL);
		os_timer_arm(&timer_led, 500, 1);

		//START SMARTCONFIG
		//NEED THE MODULE TO BE IN STATION MODE FIRST FOR SMARTCONFIG TO WORK
		wifi_set_opmode(STATION_MODE);
		smartconfig_start(smartconfig_done_function, SC_TYPE_ESPTOUCH);
	}
	else
	{
		//NORMAL MODE
		os_printf("normal operation mode\n");

		//CONNECT TO WIFI NETWORK
		//USED INTERNALLY SAVED CONFIGURATION
		wifi_set_opmode(STATION_MODE);
		wifi_station_connect();

		//INIT SPI PINS
		ESP8266_SPI_init_pins();
		//ESP8266_SPI_set_params();
		os_printf("sending spi data\n");
		ESP8266_SPI_send(0xAB, 0x86);
	}
}

void wifi_event_handler_function(System_Event_t* event)
{
	//WIFI EVENT HANDLER FUNCTION
	//SIMPLY PRINTS THE NAME OF THE WIFI EVENT FOR DEBUGGING

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

/*
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

void timer_led_callback(void *pArg)
{
	LOCAL uint8_t led_on = 0;

	os_printf("timer led called ... ");
	os_printf("time : %d\n", system_get_time());

	if(!led_on)
	{
		GPIO_OUTPUT_SET(3, 1);
		led_on = 1;
	}
	else
	{
		GPIO_OUTPUT_SET(3,0);
		led_on = 0;
	}
}

void smartconfig_done_function(sc_status status, void* pdata)
{
	switch(status)
	{
		case SC_STATUS_WAIT:
			os_printf("SMARTCONFIG - SC_STATUS_WAIT\n");
			break;

		case SC_STATUS_FIND_CHANNEL:
			os_printf("SMARTCONFIG - SC_STATUS_FIND_CHANNEL\n");
			break;

		case SC_STATUS_GETTING_SSID_PSWD:
			os_printf("SMARTCONFIG - SC_STATUS_GETTING_SSID_PSWD\n");
			sc_type* type = pdata;
			if(*type == SC_TYPE_ESPTOUCH)
			{
				os_printf("SMARTCONFIG - SC_TYPE_ESPTOUCH\n");
			}
			else
			{
				os_printf("SMARTCONFIG - SC_TYPE_AIRKISS\n");
			}
			break;

		case SC_STATUS_LINK:
			os_printf("SMARTCONFIG - SC_STATUS_LINK\n");
			struct station_config *sta_conf = pdata;
			wifi_station_set_config(sta_conf);
			wifi_station_disconnect();
			wifi_station_connect();

			//SET AUTO CONNECT = ON
			wifi_station_set_auto_connect(1);
			break;

		case SC_STATUS_LINK_OVER:
			os_printf("SMARTCONFIG - SC_STATUS_LINK_OVER\n");
			if(pdata != NULL)
			{
				//ESP_TOUCH TYPE
				uint8_t phone_ip[4] = {0,0,0,0};
				os_memcpy(phone_ip, (uint8_t*)pdata, 4);
				os_printf("SMARTCONFIG - PHONE IP : %d.%d.%d.%d\n", phone_ip[0], phone_ip[1], phone_ip[2], phone_ip[3]);
			}
			else
			{
				//ESP_AIRKISS TYPE
				os_printf("SMARTCONFIG - THIS FIRMEARE DOES NOT SUPPORT AIRKISS\n");
			}
			smartconfig_stop();

			//SMARTCONFIG DONE
			//STOP THE LED TOGGLING TIMER
			os_timer_disarm(&timer_led);
			break;
	}
}


//THIS FUNCTION IS REQUIRED TO BE IN USER_MAIN.C BY ESP8266 SDK
//COPIED FROM SDK EXAMPLES
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


