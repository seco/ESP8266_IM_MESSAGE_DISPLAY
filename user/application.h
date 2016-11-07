#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <osapi.h>
#include <ets_sys.h>
#include "user_interface.h"
#include "mem.h"
#include "gpio16.h"
#include "LCD_NOKIA_C100.h"
#include "FONT_courier_new_10pt_bold.h"
#include "FONT_rolande_28pt.h"
#include "ESP8266_UDP.h"

#define APPLICATION_COLOR_TIME LCD_NOKIA_C100_COLOR_GREEN
#define APPLICATION_COLOR_DATE LCD_NOKIA_C100_COLOR_ORANGE
#define APPLICATION_COLOR_SMARTCONFIG LCD_NOKIA_C100_COLOR_RED
#define APPLICATION_COLOR_IP LCD_NOKIA_C100_COLOR_RED

struct time
{
	uint8_t hour;
	uint8_t min;
	uint8_t sec;
	uint8_t date;
	uint8_t day;
	uint8_t month;
	uint16_t year;
	uint32_t time_tick;
};

struct message_box
{
	uint8_t box_0;
	uint8_t box_1;
	uint8_t box_2;
	uint8_t box_3;
};

struct time* NTP_TIME;
struct message_box* IM_MESSAGE_BOX;

void application_init(void);

void application_set_im_udp_listener(uint16_t port);
void application_im_udp_listener_cb(void* arg, char* pdata, uint16_t len);

void application_setup_push_button_interrupt(void);
void application_push_button_interrupt_cb(void* arg);

void application_get_time_ntp();
void application_start_timer_tick();

void application_print_time(uint8_t h, uint8_t m, uint8_t month, uint8_t date, uint16_t y, uint8_t day);
void application_print_time_dots(void);
void application_print_time_hour(uint8_t hour);
void application_print_time_min(uint8_t min);
void application_print_time_month(uint8_t month);
void application_print_time_date(uint8_t date);
void application_print_time_year(uint16_t year);
void application_print_time_day(uint8_t day);


void application_draw_logo_bitmap(void);
void application_draw_smartconfig_message(void);
void application_draw_ip_address(void);
void application_draw_im_notification_box(uint8_t box_num);
void application_clear_im_notification_box();

uint8_t application_get_ip_address_string(char* str);

#endif
