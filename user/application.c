#include "application.h"

struct time* NTP_TIME;
struct message_box* IM_MESSAGE_BOX;

void application_init(void)
{
	//INITIALIZE APPLICATION VARIABLES
	NTP_TIME = (struct time*)os_zalloc(sizeof(struct time));
	IM_MESSAGE_BOX = (struct message_box*)os_zalloc(sizeof(struct message_box));
}

void application_get_time_ntp()
{
	//GET THE TIME FROM NTP SERVER

}

void application_start_timer_tick()
{
	//START AND MANAGE THE 1 SECOND TIMER TICK
	//EVERY 120 SECONDS (2 HOURS) DO AN NTP TIME UPDATE


}

void application_print_time(uint8_t h, uint8_t m, uint8_t month, uint8_t date, uint16_t y, uint8_t day)
{
	//PRINT COMPLETE TIME BASED ON TIMER TIME VALUES
}

void application_print_time_hour(uint8_t hour)
{
	//PRINT THE HOUR STRING
	//HOUR: 0 - 23

	uint8_t hr_string[2];
	uint8_t len = 0;

	if(hour > 9)
	{
		hr_string[0] = (hour/10) + 48; len++;
		hour = hour % 10;
	}
	else
	{
		hr_string[0] = 48;
		len++;
	}
	hr_string[len] = (hour % 10) + 48; len++;

	LCD_NOKIA_C100_draw_text(10, 10, rolande_36ptBitmaps, rolande_36ptDescriptors, 3, 39, hr_string , len, APPLICATION_COLOR_TIME, LCD_NOKIA_C100_COLOR_BLACK);

}

void application_print_time_dots(void)
{
	//PRINT TIME DOTS
	//:

	LCD_NOKIA_C100_draw_text(60, 10, rolande_36ptBitmaps, rolande_36ptDescriptors, 3, 39, ":" , 1, APPLICATION_COLOR_TIME, LCD_NOKIA_C100_COLOR_BLACK);
}

void application_print_time_min(uint8_t min)
{
	//PRINT TIME MIN
	//MIN: 0 - 59

	uint8_t min_string[2];
	uint8_t len = 0;

	if(min > 9)
	{
		min_string[0] = (min/10) + 48; len++;
		min = min % 10;
	}
	else
	{
		min_string[0] = 48;
		len++;
	}
	min_string[len] = (min % 10) + 48; len++;

	LCD_NOKIA_C100_draw_text(80, 10, rolande_36ptBitmaps, rolande_36ptDescriptors, 3, 39, min_string , len, APPLICATION_COLOR_TIME, LCD_NOKIA_C100_COLOR_BLACK);
}

void application_print_time_month(uint8_t month)
{
	//PRINT THE SPECIFIED MONTH
	//MONTH: 1(JANURARY) - 12(DECEMBER)

	switch(month)
	{
	case 1:
		LCD_NOKIA_C100_draw_text(10, 55, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "January" , 11, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 2:
		LCD_NOKIA_C100_draw_text(10, 55, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "February" , 8, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 3:
		LCD_NOKIA_C100_draw_text(10, 55, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "March" , 5, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 4:
		LCD_NOKIA_C100_draw_text(10, 55, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "April" , 5, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 5:
		LCD_NOKIA_C100_draw_text(10, 55, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "May" , 3, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 6:
		LCD_NOKIA_C100_draw_text(10, 55, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "June" , 4, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 7:
		LCD_NOKIA_C100_draw_text(10, 55, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "July" , 4, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 8:
		LCD_NOKIA_C100_draw_text(10, 55, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "August" ,6, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 9:
		LCD_NOKIA_C100_draw_text(10, 55, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "September" , 9, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 10:
		LCD_NOKIA_C100_draw_text(10, 55, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "October" , 7, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 11:
		LCD_NOKIA_C100_draw_text(10, 55, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "November" , 8, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 12:
		LCD_NOKIA_C100_draw_text(10, 55, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "December" , 8, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	}
}

void application_print_time_date(uint8_t date)
{
	//PRINT THE SPECIFIED DATE
	//DATE : 1 TO 31

	uint8_t date_string[2];
	uint8_t len = 0;

	if(date > 9)
	{
		date_string[0] = (date/10) + 48; len++;
		date = date % 10;
	}
	date_string[len] = (date % 10) + 48; len++;

	LCD_NOKIA_C100_draw_text(100, 55, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, date_string, len, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
}

void application_print_time_year(uint16_t year)
{
	//PRINT THE SPECIFIED YEAR
	uint8_t year_string[4];

	year_string[0] = (year / 1000) + 48;
	year = year % 1000;
	year_string[1] = (year / 100) + 48;
	year = year % 100;
	year_string[2] = (year / 10) + 48;
	year = year % 10;
	year_string[3] = year + 48;

	LCD_NOKIA_C100_draw_text(10, 70, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, year_string , 4, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
}

void application_print_time_day(uint8_t day)
{
	//PRINT THE DAY STRING
	//DAY : 0(SUNDAY) TO 6(SATURDAY)

	switch(day)
	{
	case 0:
		LCD_NOKIA_C100_draw_text(10, 85, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "Sunday" , 6, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 1:
		LCD_NOKIA_C100_draw_text(10, 85, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "Monday" , 6, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 2:
		LCD_NOKIA_C100_draw_text(10, 85, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "Tuesday" , 7, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 3:
		LCD_NOKIA_C100_draw_text(10, 85, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "Wednesday" , 9, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 4:
		LCD_NOKIA_C100_draw_text(10, 85, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "Thursday" , 8, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 5:
		LCD_NOKIA_C100_draw_text(10, 85, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "Friday" , 6, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 6:
		LCD_NOKIA_C100_draw_text(10, 85, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "Saturday" , 8, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	}
}

void application_draw_logo_bitmap(void)
{
	//DRAW THE APPLICATION LOGO ON NOKIA LCD
	LCD_NOKIA_C100_draw_bitmap(0, 131, 0, 161, 0xD0000, 42768);
}

void application_draw_smartconfig_message(void)
{
	//DRAW THE SMART CONFIG MESSAGE

	LCD_NOKIA_C100_draw_text(5, 100, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "SmartConfig" , 11, APPLICATION_COLOR_SMARTCONFIG, LCD_NOKIA_C100_COLOR_WHITE);
	LCD_NOKIA_C100_draw_text(5, 115, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "Mode" , 4, APPLICATION_COLOR_SMARTCONFIG, LCD_NOKIA_C100_COLOR_WHITE);
}

void application_draw_ip_address(void)
{
	//DRAW THE IP ADDRESS STRING

	uint8_t ip_addr[15];
	uint8_t len = application_get_ip_address_string(ip_addr);
	LCD_NOKIA_C100_draw_text(5, 130, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, ip_addr , len, APPLICATION_COLOR_IP, LCD_NOKIA_C100_COLOR_WHITE);
}

void application_draw_im_notification_box(uint8_t box_num)
{
	//DRAW THE NOTIFICATION BOX
	//BOX_NUM : 0 TO 3
	//DRAW A BOX IF NOT ALREADY DRAWN

	switch(box_num)
	{
		case 0:
			if(IM_MESSAGE_BOX->box_0 == 0)
			{
				LCD_NOKIA_C100_draw_filled_box(3, 31, 112, 160, LCD_NOKIA_C100_COLOR_CYAN);
				IM_MESSAGE_BOX->box_0 = 1;
			}
			break;
		case 1:
			if(IM_MESSAGE_BOX->box_1 == 0)
			{
				LCD_NOKIA_C100_draw_filled_box(36, 64, 112, 160, LCD_NOKIA_C100_COLOR_RED);
				IM_MESSAGE_BOX->box_1 = 1;
			}
			break;
		case 2:
			if(IM_MESSAGE_BOX->box_2 == 0)
			{
				LCD_NOKIA_C100_draw_filled_box(69, 97, 112, 160, LCD_NOKIA_C100_COLOR_YELLOW);
				IM_MESSAGE_BOX->box_2 = 1;
			}
			break;
		case 3:
			if(IM_MESSAGE_BOX->box_3 == 0)
			{
				LCD_NOKIA_C100_draw_outline_box(102, 130, 112, 160, 3, LCD_NOKIA_C100_COLOR_MAGENTA);
				IM_MESSAGE_BOX->box_3 = 1;
			}
	}
}

void application_clear_im_notification_box()
{
	//CLEAR THE WHOLE NOTIFICATION BOX AREA

	LCD_NOKIA_C100_draw_outline_box(3, 130, 112, 160, 3, LCD_NOKIA_C100_COLOR_BLACK);
	//RESET BOX VARIABLE
	IM_MESSAGE_BOX->box_0 = 0;
	IM_MESSAGE_BOX->box_1 = 0;
	IM_MESSAGE_BOX->box_2 = 0;
	IM_MESSAGE_BOX->box_3 = 0;
}

uint8_t application_get_ip_address_string(char* str)
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
