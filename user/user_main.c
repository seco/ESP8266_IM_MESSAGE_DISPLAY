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
#include "mem.h"
#include "smartconfig.h"
#include "ESP8266_SPI.h"
#include "LCD_NOKIA_C100.h"
#include "ESP8266_UDP.h"
#include <spi_flash.h>
#include "FONT_courier_new_10pt_bold.h"
#include "FONT_rolande_28pt.h"


//////////////////////////////////
//FUNCTION PROTOTYPES
//////////////////////////////////
void esp8266_init_complete(void);
//void set_wifi_parameters(void);
void setup_gpio_pins(void);
void timer_led_callback(void *pArg);
void wifi_event_handler_function(System_Event_t* event);
void smartconfig_done_function(sc_status status, void* pdata);
void udp_listener_cb(void* arg, char* pdata, uint16_t len);
void sntp_done_cb(uint32_t val);
uint8_t get_ip_address(char* str);

//////////////////////////////////
//GLOBAL VARIABLES
//////////////////////////////////
os_timer_t timer_led;
uint32_t soft_reset_done = 1;
uint32_t soft_reset_not_done = 0;

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

void ICACHE_FLASH_ATTR setup_gpio_pins(void)
{
	//INITIAL SETUP FOR GPIO PINSs
	//GPIO-D3 : OUTPUT : LED
	//GPIO-D5 : INPUT  : MODE SELECT PUSH BUTTON (NORMAL / SMARTCONFIG)
	//GPIO-D0  : INPUT  : CLEAR MESSAGE PUSH BUTTON

	WRITE_PERI_REG(PERIPHS_IO_MUX, 0x105);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
	GPIO_DIS_OUTPUT(5);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);

	//INITIALIZA SPI
	ESP8266_SPI_init_pins();

	//INITIALIZE NOKIA LCD RESET PIN
	LCD_NOKIA_C100_set_pins();
}

void ICACHE_FLASH_ATTR esp8266_init_complete(void)
{
	os_printf("ESP8266 module init complete\n");

	uint32_t id = spi_flash_get_id();
	os_printf("flash id %X\n", id);
	enum flash_size_map m = system_get_flash_size_map();
	switch(m)
	{
		case FLASH_SIZE_4M_MAP_256_256:
			os_printf("FLASH_SIZE_4Mbit_MAP_256_256\n");
			break;
		case FLASH_SIZE_2M:
			os_printf("FLASH_SIZE_2Mbit\n");
			break;
		case FLASH_SIZE_8M_MAP_512_512:
			os_printf("FLASH_SIZE_8Mbit_MAP_512_512\n");
			break;
		case FLASH_SIZE_16M_MAP_512_512:
			os_printf("FLASH_SIZE_16Mbit_MAP_512_512\n");
			break;
		case FLASH_SIZE_32M_MAP_512_512:
			os_printf("FLASH_SIZE_32Mbit_MAP_512_512\n");
			break;
		case FLASH_SIZE_16M_MAP_1024_1024:
			os_printf("FLASH_SIZE_16Mbit_MAP_1024_1024\n");
			break;
		case FLASH_SIZE_32M_MAP_1024_1024:
			os_printf("FLASH_SIZE_32Mbit_MAP_1024_1024\n");
			break;
	}
	os_printf("testing spi + nokia\n");

	//SET ADDRESS LEN = 1 & DATA LEN = 8
	//SPI CLK = 5Mhz (75% duty)
	ESP8266_SPI_set_params(1,8, 8, 2, 3, 0);
	LCD_NOKIA_C100_init();
	LCD_NOKIA_C100_clear_screen(LCD_NOKIA_C100_COLOR_RED);

	//NEED TO INIT THE LCD AGAIN TO MAKE IT WORK AS ON
	//POWER ON ESP SAMPLES/DRIVES THE GPIO WHICH I AM NOT
	//SURE ABOUT. THIS CAUSES LCD TO NOT INIT PROPERLY.
	//TO MAKE IT WORK, NEED TO INIT AGAIN
	LCD_NOKIA_C100_init();
	LCD_NOKIA_C100_clear_screen(LCD_NOKIA_C100_COLOR_RED);

	//DRAW NOMADIC LOGO ON LCD
	LCD_NOKIA_C100_draw_bitmap(0, 131, 0, 161, 0xD0000, 42768);

	//PRINT SOME BASIC INFORMATION ABOUT THE MODULE ON DEBUG
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

		//PRINT ON LCD
		LCD_NOKIA_C100_draw_text(5, 100, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "SmartConfig" , 11, LCD_NOKIA_C100_COLOR_RED, LCD_NOKIA_C100_COLOR_WHITE);
		LCD_NOKIA_C100_draw_text(5, 115, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "Mode" , 4, LCD_NOKIA_C100_COLOR_RED, LCD_NOKIA_C100_COLOR_WHITE);

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
	}
}

void ICACHE_FLASH_ATTR wifi_event_handler_function(System_Event_t* event)
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
			os_printf("*** COMPLETE INIT DONE ***\n");

			uint8_t ip_addr[15];
			uint8_t len = get_ip_address(ip_addr);
			LCD_NOKIA_C100_draw_text(5, 130, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, ip_addr , len, LCD_NOKIA_C100_COLOR_RED, LCD_NOKIA_C100_COLOR_WHITE);
			uint8_t i = 0;
			for(i=0; i<21; i++)
			{
				os_delay_us(50000);
			}
			LCD_NOKIA_C100_clear_screen(LCD_NOKIA_C100_COLOR_BLACK);

			LCD_NOKIA_C100_draw_text(10, 10, rolande_36ptBitmaps, rolande_36ptDescriptors, 3, 39, "23:59" , 7, LCD_NOKIA_C100_COLOR_GREEN, LCD_NOKIA_C100_COLOR_BLACK);
			LCD_NOKIA_C100_draw_text(10, 55, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "September30" , 11, LCD_NOKIA_C100_COLOR_ORANGE, LCD_NOKIA_C100_COLOR_BLACK);
			LCD_NOKIA_C100_draw_text(10, 70, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "2016" , 4, LCD_NOKIA_C100_COLOR_ORANGE, LCD_NOKIA_C100_COLOR_BLACK);
			LCD_NOKIA_C100_draw_text(10, 85, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "Tuesday" , 7, LCD_NOKIA_C100_COLOR_ORANGE, LCD_NOKIA_C100_COLOR_BLACK);

			LCD_NOKIA_C100_draw_filled_box(3, 31, 112, 160, LCD_NOKIA_C100_COLOR_CYAN);
			LCD_NOKIA_C100_draw_filled_box(36, 64, 112, 160, LCD_NOKIA_C100_COLOR_RED);
			LCD_NOKIA_C100_draw_filled_box(69, 97, 112, 160, LCD_NOKIA_C100_COLOR_YELLOW);
			LCD_NOKIA_C100_draw_outline_box(102, 130, 112, 160, 3, LCD_NOKIA_C100_COLOR_MAGENTA);

			//struct ESP8266_UDP_HANDLE *h = (struct ESP8266_UDP_HANDLE*)os_zalloc(sizeof(struct ESP8266_UDP_HANDLE));
			//ESP8266_UDP_create_listener(25867, &udp_listener_cb, h);
			//os_printf("created udp listener on port 25867\n");

			os_printf("setting up ntp listener\n");
			struct ESP8266_UDP_HANDLE *h2 = (struct ESP8266_UDP_HANDLE*)os_zalloc(sizeof(struct ESP8266_UDP_HANDLE));
			ESP8266_UDP_create_listener(123, &udp_listener_cb, h2);
			os_printf("created udp listener on port 123\n");

			/*
			os_printf("doing sntp\n");
			struct ip_addr* ipp = (struct ip_addr*)os_zalloc(sizeof(struct ip_addr));
			ipp->addr = 1655688136;
			ESP8266_SNTP_set_server(0, ipp);
			//ESP8266_SNTP_set_server_name(0, "pool.ntp.org");
			ESP8266_SNTP_set_timezone(5);
			ESP8266_SNTP_set_callback(&sntp_done_cb);
			ESP8266_SNTP_start();
			ESP8266_SNTP_get_current_timestamp();
			*/

			os_printf("testing udp ntp\n");
			struct ESP8266_UDP_HANDLE *h1 = (struct ESP8266_UDP_HANDLE*)os_zalloc(sizeof(struct ESP8266_UDP_HANDLE));
			uint8_t data[48] = {0};
			data[0] = 0x0B;
			ESP8266_UDP_send_data_ip(128, 138, 141, 172, 123, data, 48, h1);
			os_printf("udp data sent\n");
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

void ICACHE_FLASH_ATTR timer_led_callback(void *pArg)
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

void ICACHE_FLASH_ATTR smartconfig_done_function(sc_status status, void* pdata)
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
			os_printf("*** smartconfig done\n");
			//STOP THE LED TOGGLING TIMER
			os_timer_disarm(&timer_led);
			break;
	}
}

void udp_listener_cb(void* arg, char* pdata, uint16_t len)
{
	os_printf("received udp data of length = %d\n", len);
}

void sntp_done_cb(uint32_t val)
{
	os_printf("sntp timestamp %d\n", val);
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

uint8_t get_ip_address(char* str)
{
	//RETURN THE IP ADDRESS IN THE PROVIDED
	//CHARACTER STRING. RETURN VALUE IS THE LENGTH
	//OF THE IP ADDRESS STRING

	struct ip_info i;
	wifi_get_ip_info(0, &i);

	uint32_t ip = i.ip.addr;

	uint8_t ip4 = (uint8_t)((ip & 0xFF000000) >> 24);
	uint8_t ip3 = (uint8_t)((ip & 0x00FF0000) >> 16);
	uint8_t ip2 = (uint8_t)((ip & 0x0000FF00) >> 8);
	uint8_t ip1 = (uint8_t)(ip & 0x000000FF);

	uint8_t len = 0;

	if(ip1 > 99)
	{
		str[len] = (ip1/100) + 48; len++;
		ip1 = ip1 % 100;
	}
	if(ip1 > 9)
	{
		str[len] = (ip1/10) + 48; len++;
		ip1 = ip1 % 10;
	}
	str[len] = (ip1 % 10) + 48; len++;

	str[len] = '.'; len++;

	if(ip2 > 99)
	{
		str[len] = (ip2/100) + 48; len++;
		ip2 = ip2 % 100;
	}
	if(ip2 > 9)
	{
		str[len] = (ip2/10) + 48; len++;
		ip2 = ip2 % 10;
	}
	str[len] = (ip2 % 10) + 48; len++;

	str[len] = '.'; len++;

	if(ip3 > 99)
	{
		str[len] = (ip3/100) + 48; len++;
		ip3 = ip3 % 100;
	}
	if(ip3 > 9)
	{
		str[len] = (ip3/10) + 48; len++;
		ip3 = ip3 % 10;
	}
	str[len] = (ip3 % 10) + 48; len++;

	str[len] = '.'; len++;

	if(ip4 > 99)
	{
		str[len] = (ip4/100) + 48; len++;
		ip4 = ip4 % 100;
	}
	if(ip4 > 9)
	{
		str[len] = (ip4/10) + 48; len++;
		ip4 = ip4 % 10;
	}
	str[len] = (ip4 % 10) + 48; len++;

	return len;
}
