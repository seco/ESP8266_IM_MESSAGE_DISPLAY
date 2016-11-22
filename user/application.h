#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <osapi.h>
#include <ets_sys.h>
#include "user_interface.h"
#include "mem.h"
#include "gpio16.h"
#include "LCD_NOKIA_C100.h"
#include "FONT_courier_new_10pt_bold.h"
#include "FONT_courier_new_18pt.h"
#include "ESP8266_UDP.h"
#include "FONT_white_rabbit_36pt_bold.h"


#define APPLICATION_COLOR_TIME LCD_NOKIA_C100_COLOR_TEAL_BLUE
#define APPLICATION_COLOR_DATE LCD_NOKIA_C100_COLOR_ORANGE
#define APPLICATION_COLOR_SMARTCONFIG LCD_NOKIA_C100_COLOR_RED
#define APPLICATION_COLOR_IP LCD_NOKIA_C100_COLOR_RED

//2000-03-01 : new epoch - mod 400 year, immidately after feb 29
//seconds since jan 1 1900 to 2000-03-01
#define LEAPOCH (36584UL) * 86400
#define DAYS_PER_4Y (365*4 +1)
#define DAYS_PER_100Y (365*100UL + 24)
#define DAYS_PER_400Y (365*400UL + 97)


struct time
{
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint8_t date;
	uint8_t day;
	uint8_t month;
	uint16_t year;
	uint64_t time_tick;
	uint8_t draw_update_flag;
};

struct message_box
{
	uint8_t box_0;
	uint8_t box_1;
	uint8_t box_2;
	uint8_t box_3;
};

const uint8_t ntp_server_list[3][4];
uint8_t ntp_current_server_index;
os_timer_t ntp_server_timer;
uint8_t ntp_switch_ip_timer_started;
uint8_t ntp_successfull;

struct time* NTP_TIME;
struct message_box* IM_MESSAGE_BOX;
os_timer_t timer_tick_timer;
uint32_t timer_tick_count;
struct ESP8266_UDP_HANDLE* ntp_handle;
uint8_t second_dash_x_count;

void application_init(void);

void ICACHE_FLASH_ATTR application_set_im_udp_listener(uint16_t port);
void ICACHE_FLASH_ATTR application_im_udp_listener_cb(void* arg, char* pdata, uint16_t len);

void ICACHE_FLASH_ATTR application_setup_push_button_interrupt(void);
void ICACHE_FLASH_ATTR application_push_button_interrupt_cb(void* arg);

void ICACHE_FLASH_ATTR application_send_ntp_request(void);
void ICACHE_FLASH_ATTR application_ntp_udp_listener_cb(void* arg, char* pdata, uint16_t len);
void ICACHE_FLASH_ATTR application_switch_ntp_server_cb(void);

void ICACHE_FLASH_ATTR application_get_time_ntp(void);
void ICACHE_FLASH_ATTR application_get_time_components_from_ntp_timestamp(void);
void ICACHE_FLASH_ATTR application_increament_time_components(void);
void ICACHE_FLASH_ATTR application_start_timer_tick(void);
void ICACHE_FLASH_ATTR application_timer_timer_tick_cb(void);
uint8_t ICACHE_FLASH_ATTR application_check_leap_year(uint16_t year);

void ICACHE_FLASH_ATTR application_print_time(void);
void ICACHE_FLASH_ATTR application_print_time_dots(void);
void ICACHE_FLASH_ATTR application_print_time_hour(uint8_t hour);
void ICACHE_FLASH_ATTR application_print_time_min(uint8_t min);
void ICACHE_FLASH_ATTR application_print_time_month(uint8_t month);
void ICACHE_FLASH_ATTR application_print_time_date(uint8_t date);
void ICACHE_FLASH_ATTR application_print_time_year(uint16_t year);
void ICACHE_FLASH_ATTR application_print_time_day(uint8_t day);


void ICACHE_FLASH_ATTR application_draw_logo_bitmap(void);
void ICACHE_FLASH_ATTR application_draw_smartconfig_message(void);
void ICACHE_FLASH_ATTR application_draw_ip_address(void);
void ICACHE_FLASH_ATTR application_draw_im_notification_box(uint8_t box_num, uint8_t letter);
void ICACHE_FLASH_ATTR application_clear_im_notification_box(void);

uint8_t ICACHE_FLASH_ATTR application_get_ip_address_string(char* str);

#endif
