#include "application.h"

//////////////////////////////////
//GLOBAL VARIABLES
//////////////////////////////////
const char days_in_month[] = {
								31, //MARCH
								30, //APRIL
								31, //MAY
								30, //JUNE
								31, //JULY
								31, //AUGUST
								30, //SEPT
								31, //OCT
								30, //NOV
								31, //DEC
								31, //JAN
								29	//FEB
							};

const uint8_t* month_names[12] = {
									"March",
									"April",
									"May",
									"June",
									"July",
									"August",
									"September",
									"October",
									"November",
									"December",
									"January",
									"February"
								};

const uint8_t month_names_length[12] = {
										 5,	//MARCH
										 5,	//APRIL
										 3,	//MAY
										 4,	//JUNE
										 4, //JULY
										 6,	//AUG
										 9,	//SEPT
										 7,	//OCT
										 8,	//NOV
										 8,	//DEC
										 7,	//JAN
										 8	//FEB
									};

const uint8_t ntp_server_list[3][4] = {
										{128, 138, 140, 044},
										{216, 229, 000, 179},
										{066, 199, 022, 067}
									};

uint8_t ntp_current_server_index = 0;
uint8_t ntp_switch_ip_timer_started = 0;
os_timer_t ntp_server_timer;
uint8_t ntp_successfull = 0;

struct time* NTP_TIME;
struct message_box* IM_MESSAGE_BOX;
os_timer_t timer_tick_timer;
uint32_t timer_tick_count;
uint8_t second_dash_x_count;
struct ESP8266_UDP_HANDLE* ntp_handle;

void ICACHE_FLASH_ATTR application_init(void)
{
	//INITIALIZE APPLICATION VARIABLES
	NTP_TIME = (struct time*)os_zalloc(sizeof(struct time));
	IM_MESSAGE_BOX = (struct message_box*)os_zalloc(sizeof(struct message_box));
	timer_tick_count = 0;
	second_dash_x_count = 1;
	ntp_handle = NULL;
}

void ICACHE_FLASH_ATTR application_set_im_udp_listener(uint16_t port)
{
	//SETUP THE UDP LISTENER FOR THE IM BOX MESSAGES

	struct ESP8266_UDP_HANDLE *h = (struct ESP8266_UDP_HANDLE*)os_zalloc(sizeof(struct ESP8266_UDP_HANDLE));
	ESP8266_UDP_create_listener(port, &application_im_udp_listener_cb, h);
	os_printf("created udp listener on port %d\n", port);
}

void ICACHE_FLASH_ATTR application_im_udp_listener_cb(void* arg, char* pdata, uint16_t len)
{
	//CALLBACK FUNCTION FOR UDP IM MESSAGES
	//BYTE 0 : NAME (LETTER TO PRINT INSIDE BOX)
	//BYTE 1 : LOCATION OF BOX (0-3)

	os_printf("received im message of len %d\n", len);
	os_printf("drawing box at location %d with letter %c\n", pdata[1], pdata[0]);
	application_draw_im_notification_box(pdata[1], pdata[0]);
}

void ICACHE_FLASH_ATTR application_setup_push_button_interrupt(void)
{
	//SETUP THE PUSH BUTTON INTERRUPT FOR BUTTON
	//CONNECTED TO GPIO 5

	//ENABLE INTERRUPT ON PUSH BUTTON PIN (GPIO5)
	//INTERRUPT ON LOW TO HIGH
	ETS_GPIO_INTR_DISABLE();
	ETS_GPIO_INTR_ATTACH(application_push_button_interrupt_cb, 5);
	gpio_pin_intr_state_set(GPIO_ID_PIN(5), GPIO_PIN_INTR_NEGEDGE);
	ETS_GPIO_INTR_ENABLE();
}

void ICACHE_FLASH_ATTR application_push_button_interrupt_cb(void* arg)
{
	//PUSH BUTTON INTERRUPT ISR

	uint32 status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	if(status & BIT(5))
	{
		os_printf("push btn interrupt\n");
		application_clear_im_notification_box();

		// Clear interrupt
		 GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, status & BIT(5));
	}
}

void ICACHE_FLASH_ATTR application_send_ntp_request()
{
	//SEND A NTP TIME REQUEST PACKET TO NTP SERVER PORT 123 FROM LOCAL PORT 55555
	//AND GET THE REPLY

	ntp_successfull = 0;

	//setup ntp server switch timer
	//1 second ntp server switch
	if(ntp_switch_ip_timer_started == 0)
	{
		os_timer_setfn(&ntp_server_timer, application_switch_ntp_server_cb, NULL);
	}

	uint8_t packet[48] = {0};
	packet[0] = 0x0B;

	//CREATE NEW NTP UDP HANDLE IF NOT DONE YET
	if(ntp_handle == NULL)
	{
		ntp_handle = (struct ESP8266_UDP_HANDLE*)os_zalloc(sizeof(struct ESP8266_UDP_HANDLE));

	}
	os_printf("********\n");
	ESP8266_UDP_send_receive_data_ip(ntp_server_list[ntp_current_server_index][0], ntp_server_list[ntp_current_server_index][1],
									ntp_server_list[ntp_current_server_index][2], ntp_server_list[ntp_current_server_index][3],
									123 , 55555, packet, 48, application_ntp_udp_listener_cb, ntp_handle);

	os_printf("ntp request sent to ntp server %d. waiting for reply\n", ntp_current_server_index);

	//start ntp server switch timer, only if not already started
	if(ntp_switch_ip_timer_started == 0)
	{
		os_timer_arm(&ntp_server_timer, 1000, 1);
		ntp_switch_ip_timer_started = 1;
	}
}

void ICACHE_FLASH_ATTR application_switch_ntp_server_cb(void)
{
	//ntp server reply timed out.
	//switch the global ntp server list index variable
	//so that the next ntp server can be tried

	if(ntp_switch_ip_timer_started == 1)
	{
		ntp_current_server_index++;
		os_printf("switching ntp server index to %d\n", ntp_current_server_index);

		if(ntp_current_server_index == 3)
		{
			//all 3 ntp servers have been tried
			//and exhausted
			os_printf("all ntp servers tried and exhausted. no reply !\n");
			LCD_NOKIA_C100_draw_text(10, 15, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "NTP Error" , 9, LCD_NOKIA_C100_COLOR_RED, LCD_NOKIA_C100_COLOR_BLACK);

			ntp_switch_ip_timer_started = 0;
			ntp_current_server_index = 0;

			//disable and turn off timer
			os_timer_disarm(ntp_server_timer);

			ntp_successfull = 0;
		}
		else
		{
			//send a new ntp request with the new server
			application_send_ntp_request();
		}
	}
}

void ICACHE_FLASH_ATTR application_ntp_udp_listener_cb(void* arg, char* pdata, uint16_t len)
{
	//CALLBACK FUNCTION FOR NTP REPLY

	//received reply from ntp server
	//stop the ntp server switch timer
	//ntp successfull
	os_timer_disarm(&ntp_server_timer);
	ntp_switch_ip_timer_started = 0;
	ntp_current_server_index = 0;
	ntp_successfull = 1;

	os_printf("received NTP message of len %d\n", len);
	//extract the 32 bit timestamp from ntp reply
	uint32_t a = pdata[40];
	uint32_t b = pdata[41];
	uint32_t c = pdata[42];
	uint32_t d = pdata[43];

	NTP_TIME->time_tick = (a << 24) | (b << 16) | (c << 8) | d;

	os_printf("ntp timestamp %d\n", NTP_TIME->time_tick);

	//reset timer tick count
	timer_tick_count = 0;

	//populate time parameters from the timestamp
	application_get_time_components_from_ntp_timestamp();

	application_print_time();
}

void ICACHE_FLASH_ATTR application_get_time_ntp(void)
{
	//GET THE TIME FROM NTP SERVER
	//SEND NTP REQUEST AND CREATE A LISTENER FOR REPLY
	application_send_ntp_request();
}

void ICACHE_FLASH_ATTR application_get_time_components_from_ntp_timestamp(void)
{
	//CALCULATE THE TIME COMPONENTS FROM THE 32 BIT TIMESTAMP
	//AND SAVE IN GLOBAL TIME STRUCTURE

	//CONVERT TIMESTAMP TO DATE/TIME VALUES
	//REFERENCE : http://git.musl-libc.org/cgit/musl/plain/src/time/__secs_to_tm.c?h=v0.9.15

	//	YEAR 		(BASE 2000)
	//	MONTH 		(JANUARY = 1)
	//	DATE 		(1 BASED)
	//	WEEKDAY		(MONDAY = 1)
	//	HOUR
	//	MINUTE
	//	SECOND

	//UTC OFFSET INDIA (GMT + 5:30)
	uint32_t india_utc_offset_seconds = (5*3600) + (30*60);

	uint64_t secs = NTP_TIME->time_tick;
	secs += india_utc_offset_seconds;
	secs -= LEAPOCH;

	uint32_t days = secs / 86400;
	uint32_t rem_secs = secs % 86400;

	if(rem_secs < 0)
	{
		rem_secs += 86400;
		days--;
	}

	uint8_t wday = (days % 7) + 3;
	if(wday > 7)
	{
		wday -= 7;
	}

	uint32_t _400y_cycles = days / DAYS_PER_400Y;
	uint32_t rem_days = days % DAYS_PER_400Y;

	if(rem_days < 0)
	{
		rem_days += DAYS_PER_400Y;
		_400y_cycles--;
	}

	uint32_t _100y_cycles = rem_days / DAYS_PER_100Y;
	if(_100y_cycles == 4)
	{
		_100y_cycles--;
	}
	rem_days -= (_100y_cycles * DAYS_PER_100Y);

	uint32_t _4y_cycles = rem_days / DAYS_PER_4Y;
	if(_4y_cycles == 25)
	{
		_4y_cycles--;
	}
	rem_days -= (_4y_cycles * DAYS_PER_4Y);

	uint32_t rem_years = rem_days / 365;
	if(rem_years == 4)
	{
		rem_years --;
	}
	rem_days -= (rem_years * 365);

	uint16_t leap = !rem_years && (_4y_cycles || !_100y_cycles);
	uint16_t yday = rem_days + 31 + 28 + leap;

	if(yday > (365 + leap))
	{
		yday -= (365 + leap);
	}

	uint32_t years = rem_years + (4 * _4y_cycles) + (100 * _100y_cycles) + (400 * _400y_cycles);

	uint8_t months;
	for(months = 0; days_in_month[months] <= rem_days; months++)
	{
		rem_days -= days_in_month[months];
	}

	NTP_TIME->year = years;
	NTP_TIME->month = months + 3;
	if(NTP_TIME->month > 12)
	{
		NTP_TIME->month -= 12;
		NTP_TIME->year++;
	}
	NTP_TIME->year += 2000; //since we calculated from year 2000

	NTP_TIME->date = rem_days + 1;
	NTP_TIME->day = wday;

	NTP_TIME->hour = rem_secs / 3600;
	NTP_TIME->min = (rem_secs / 60) % 60;
	NTP_TIME->sec = rem_secs % 60;

	/////////////////////////////////////////////////////////

	//set to redraw all the time components
	NTP_TIME->draw_update_flag = 0x7F;
}

void ICACHE_FLASH_ATTR application_increament_time_components(void)
{
	//INCREMENT TIME COMPONENTS BASED ON THE LAST VALUE
	//AND THE 1 SECOND TIMER TICK. THIS IS USED TO INCREAMENT
	//THE TIME WHEN THE TIME IS NOT RECEIVED FROM NTP

	NTP_TIME->sec++;
	NTP_TIME->draw_update_flag |= 0b00000001;

	if(NTP_TIME->sec == 60)
	{
		NTP_TIME->sec = 0;
		NTP_TIME->min++;
		NTP_TIME->draw_update_flag |= 0b00000010;
	}

	if(NTP_TIME->min == 60)
	{
		NTP_TIME->min = 0;
		NTP_TIME->hour++;
		NTP_TIME->draw_update_flag |= 0b00000100;
	}

	if(NTP_TIME->hour == 24)
	{
		NTP_TIME->hour = 0;
		NTP_TIME->date++;
		NTP_TIME->day++;
		if(NTP_TIME->day == 8)
		{
			NTP_TIME->day = 1;
		}
		NTP_TIME->draw_update_flag |= 0b00001000;
		NTP_TIME->draw_update_flag |= 0b01000000;
	}

	//date (number of days in month) depend on
	//month and leap year(february)
	if(NTP_TIME->date > days_in_month[(NTP_TIME->month+10)%12])
	{
		//check if its february in a leap year
		if(NTP_TIME->date == 29 && NTP_TIME->month == 2 && application_check_leap_year(NTP_TIME->year)==1)
		{
			//its february 29 in a leap year. do nothing
		}
		else
		{
			NTP_TIME->date = 1;
			NTP_TIME->month++;
			NTP_TIME->draw_update_flag |= 0b00010000;
		}
	}

	if(NTP_TIME->month == 13)
	{
		NTP_TIME->month = 1;
		NTP_TIME->year++;
		NTP_TIME->draw_update_flag |= 0b00100000;
	}
}

void ICACHE_FLASH_ATTR application_start_timer_tick(void)
{
	//START AND MANAGE THE 1 SECOND TIMER TICK
	//EVERY 120 SECONDS (2 HOURS) DO AN NTP TIME UPDATE

	os_timer_setfn(&timer_tick_timer, application_timer_timer_tick_cb);
	os_timer_arm(&timer_tick_timer, 1000, 1);
}

void ICACHE_FLASH_ATTR application_timer_timer_tick_cb(void)
{
	//TIME TIMER CALLBACK FUNCTION

	NTP_TIME->time_tick++;
	timer_tick_count++;

	second_dash_x_count++;

	if(timer_tick_count == 7200)
	{
		//2 hours since last ntp time check
		//call ntp
		os_printf("calling ntp\n");
		timer_tick_count = 0;
		application_get_time_ntp();
	}
	else
	{
		application_increament_time_components();
	}

	//print time (whatever needs to be redrawn)
	application_print_time();
}

uint8_t ICACHE_FLASH_ATTR application_check_leap_year(uint16_t year)
{
	//CHECK IF THE SUPPLIED YEAR IS LEAP YEAR
	//0: NO LEAP
	//1: LEAP

	if((year % 4) == 0)
	{
		if((year % 100) == 0)
		{
			if((year % 400) == 0)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			return 0;
		}
	}
	return 0;
}

void ICACHE_FLASH_ATTR application_print_time(void)
{
	//PRINT COMPLETE TIME BASED ON TIMER TIME VALUES
	//TIME STRUCTURE HAS A BYTE FLAG TO (draw_update_flag) TO SIGNAL
	//WHAT ALL TIME COMPONENTS TO REDRAW
	//1: REDRAW
	//0: NO REDRAW
	//BYTE 0 : SECOND DOTS
	//BYTE 1 : MIN
	//BYTE 2 : HR
	//BYTE 3 : DATE
	//BYTE 4 : MONTH
	//BYTE 5 : YEAR
	//BYTE 6 : DAY

	if(NTP_TIME->draw_update_flag & (1<<0))
	{
		//redraw second dots
		application_print_time_dots();
		//clear draw bit
		NTP_TIME->draw_update_flag &=(~(1<<0));
	}

	if(NTP_TIME->draw_update_flag & (1<<1))
	{
		//redraw min
		application_print_time_min(NTP_TIME->min);
		//clear draw bit
		NTP_TIME->draw_update_flag &=(~(1<<1));
	}

	if(NTP_TIME->draw_update_flag & (1<<2))
	{
		//redraw hr
		application_print_time_hour(NTP_TIME->hour);
		//clear draw bit
		NTP_TIME->draw_update_flag &=(~(1<<2));
	}

	if(NTP_TIME->draw_update_flag & (1<<3))
	{
		//redraw date
		application_print_time_date(NTP_TIME->date);
		//clear draw bit
		NTP_TIME->draw_update_flag &=(~(1<<3));
	}

	if(NTP_TIME->draw_update_flag & (1<<4))
	{
		//redraw month
		application_print_time_month(NTP_TIME->month);
		//clear draw bit
		NTP_TIME->draw_update_flag &=(~(1<<4));
	}

	if(NTP_TIME->draw_update_flag & (1<<5))
	{
		//redraw year
		application_print_time_year(NTP_TIME->year);
		//clear draw bit
		NTP_TIME->draw_update_flag &=(~(1<<5));
	}

	if(NTP_TIME->draw_update_flag & (1<<6))
	{
		//redraw day
		application_print_time_day(NTP_TIME->day);
		//clear draw bit
		NTP_TIME->draw_update_flag &=(~(1<<6));
	}
}

void ICACHE_FLASH_ATTR application_print_time_hour(uint8_t hour)
{
	//PRINT THE HOUR STRING
	//HOUR: 0 - 23

	uint8_t i;
	uint8_t x_start = 10;
	uint8_t y = 5;

	for(i=0; i<=5; i++)
	{
		if(hour & (1 << (5-i)))
		{
			LCD_NOKIA_C100_draw_filled_box(x_start, x_start + 17 - 1, y, y + 17 - 1, APPLICATION_COLOR_TIME);
		}
		else
		{
			LCD_NOKIA_C100_draw_outline_box(x_start, x_start + 17 - 1, y, y + 17 - 1, 1, APPLICATION_COLOR_TIME);
		}
		x_start += 19;
	}

}

void ICACHE_FLASH_ATTR application_print_time_dots(void)
{
	//PRINT TIME DOTS
	//-

	if(second_dash_x_count < 7)
	{
		LCD_NOKIA_C100_draw_line_horizontal(12 + ((second_dash_x_count - 1) * 19), 12 + ((second_dash_x_count - 1) * 19) + 12 - 1, 30, 2, APPLICATION_COLOR_TIME);
	}
	else
	{
		//time to clear the second dashes
		LCD_NOKIA_C100_draw_filled_box(10, 120, 30, 36, LCD_NOKIA_C100_COLOR_BLACK);
		second_dash_x_count = 0;
	}
}

void ICACHE_FLASH_ATTR application_print_time_min(uint8_t min)
{
	//PRINT TIME MIN
	//MIN: 0 - 59

	uint8_t i;
	uint8_t x_start = 10, y = 40;
	for(i=0; i<=5; i++)
	{
		if(min & (1 << (5-i)))
		{
			LCD_NOKIA_C100_draw_filled_box(x_start, x_start + 17 - 1, y, y + 17 - 1, APPLICATION_COLOR_TIME);
		}
		else
		{
			LCD_NOKIA_C100_draw_outline_box(x_start, x_start + 17 - 1, y, y + 17 - 1, 1, APPLICATION_COLOR_TIME);
		}
		x_start += 19;
	}

}

void ICACHE_FLASH_ATTR application_print_time_month(uint8_t month)
{
	//PRINT THE SPECIFIED MONTH
	//MONTH: 1(JANURARY) - 12(DECEMBER)

	switch(month)
	{
	case 1:
		LCD_NOKIA_C100_draw_text(10, 63, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "January" , 11, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 2:
		LCD_NOKIA_C100_draw_text(10, 63, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "February" , 8, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 3:
		LCD_NOKIA_C100_draw_text(10, 63, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "March" , 5, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 4:
		LCD_NOKIA_C100_draw_text(10, 63, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "April" , 5, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 5:
		LCD_NOKIA_C100_draw_text(10, 63, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "May" , 3, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 6:
		LCD_NOKIA_C100_draw_text(10, 63, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "June" , 4, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 7:
		LCD_NOKIA_C100_draw_text(10, 63, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "July" , 4, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 8:
		LCD_NOKIA_C100_draw_text(10, 63, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "August" ,6, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 9:
		LCD_NOKIA_C100_draw_text(10, 63, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "September" , 9, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 10:
		LCD_NOKIA_C100_draw_text(10, 63, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "October" , 7, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 11:
		LCD_NOKIA_C100_draw_text(10, 63, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "November" , 8, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 12:
		LCD_NOKIA_C100_draw_text(10, 63, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "December" , 8, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	}
}

void ICACHE_FLASH_ATTR application_print_time_date(uint8_t date)
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

	LCD_NOKIA_C100_draw_text(100, 63, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, date_string, len, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
}

void ICACHE_FLASH_ATTR application_print_time_year(uint16_t year)
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

	LCD_NOKIA_C100_draw_text(10, 78, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, year_string , 4, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
}

void ICACHE_FLASH_ATTR application_print_time_day(uint8_t day)
{
	//PRINT THE DAY STRING
	//DAY : 0(SUNDAY) TO 6(SATURDAY)

	switch(day)
	{
	case 1:
		LCD_NOKIA_C100_draw_text(10, 93, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "Monday" , 6, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 2:
		LCD_NOKIA_C100_draw_text(10, 93, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "Tuesday" , 7, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 3:
		LCD_NOKIA_C100_draw_text(10, 93, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "Wednesday" , 9, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 4:
		LCD_NOKIA_C100_draw_text(10, 93, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "Thursday" , 8, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 5:
		LCD_NOKIA_C100_draw_text(10, 93, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "Friday" , 6, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 6:
		LCD_NOKIA_C100_draw_text(10, 93, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "Saturday" , 8, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	case 7:
		LCD_NOKIA_C100_draw_text(10, 93, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "Sunday" , 8, APPLICATION_COLOR_DATE, LCD_NOKIA_C100_COLOR_BLACK);
		break;
	}
}

void ICACHE_FLASH_ATTR application_draw_logo_bitmap(void)
{
	//DRAW THE APPLICATION LOGO ON NOKIA LCD
	LCD_NOKIA_C100_draw_bitmap(0, 131, 0, 161, 0xD0000, 42768);
}

void ICACHE_FLASH_ATTR application_draw_smartconfig_message(void)
{
	//DRAW THE SMART CONFIG MESSAGE

	LCD_NOKIA_C100_draw_text(5, 110, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "SmartConfig" , 11, APPLICATION_COLOR_SMARTCONFIG, LCD_NOKIA_C100_COLOR_WHITE);
	LCD_NOKIA_C100_draw_text(7, 125, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, "Mode" , 4, APPLICATION_COLOR_SMARTCONFIG, LCD_NOKIA_C100_COLOR_WHITE);
}

void ICACHE_FLASH_ATTR application_draw_ip_address(void)
{
	//DRAW THE IP ADDRESS STRING

	uint8_t ip_addr[15];
	uint8_t len = application_get_ip_address_string(ip_addr);
	LCD_NOKIA_C100_draw_text(5, 140, courierNew_10ptBitmaps, courierNew_10ptDescriptors, 2, 13, ip_addr , len, APPLICATION_COLOR_IP, LCD_NOKIA_C100_COLOR_WHITE);
}

void ICACHE_FLASH_ATTR application_draw_im_notification_box(uint8_t box_num, uint8_t letter)
{
	//DRAW THE NOTIFICATION BOX
	//BOX_NUM : 0 TO 3
	//DRAW A BOX IF NOT ALREADY DRAWN
	//DRAW THE SPECIFIED LETTER INSIDE IT

	switch(box_num)
	{
		case 0:
			if(IM_MESSAGE_BOX->box_0 == 0)
			{
				LCD_NOKIA_C100_draw_filled_box(3, 31, 112, 160, LCD_NOKIA_C100_COLOR_CYAN);
				LCD_NOKIA_C100_draw_text(3 + 5, 112 + 14, courierNew_18ptBitmaps, courierNew_18ptDescriptors, 3, 24, &letter, 1, LCD_NOKIA_C100_COLOR_BLACK, LCD_NOKIA_C100_COLOR_CYAN);
				IM_MESSAGE_BOX->box_0 = 1;
			}
			break;
		case 1:
			if(IM_MESSAGE_BOX->box_1 == 0)
			{
				LCD_NOKIA_C100_draw_filled_box(36, 64, 112, 160, LCD_NOKIA_C100_COLOR_RED);
				LCD_NOKIA_C100_draw_text(36 + 5, 112 + 14, courierNew_18ptBitmaps, courierNew_18ptDescriptors, 3, 24, &letter, 1, LCD_NOKIA_C100_COLOR_BLACK, LCD_NOKIA_C100_COLOR_RED);
				IM_MESSAGE_BOX->box_1 = 1;
			}
			break;
		case 2:
			if(IM_MESSAGE_BOX->box_2 == 0)
			{
				LCD_NOKIA_C100_draw_filled_box(69, 97, 112, 160, LCD_NOKIA_C100_COLOR_YELLOW);
				LCD_NOKIA_C100_draw_text(69 + 5, 112 + 14, courierNew_18ptBitmaps, courierNew_18ptDescriptors, 3, 24, &letter, 1, LCD_NOKIA_C100_COLOR_BLACK, LCD_NOKIA_C100_COLOR_YELLOW);
				IM_MESSAGE_BOX->box_2 = 1;
			}
			break;
		case 3:
			if(IM_MESSAGE_BOX->box_3 == 0)
			{
				LCD_NOKIA_C100_draw_filled_box(102, 130, 112, 160, LCD_NOKIA_C100_COLOR_MAGENTA);
				LCD_NOKIA_C100_draw_text(102 + 5, 112 + 14, courierNew_18ptBitmaps, courierNew_18ptDescriptors, 3, 24, &letter, 1, LCD_NOKIA_C100_COLOR_BLACK, LCD_NOKIA_C100_COLOR_MAGENTA);
				IM_MESSAGE_BOX->box_3 = 1;
			}
	}
}

void ICACHE_FLASH_ATTR application_clear_im_notification_box()
{
	//CLEAR THE WHOLE NOTIFICATION BOX AREA

	LCD_NOKIA_C100_draw_outline_box(3, 130, 112, 160, 3, LCD_NOKIA_C100_COLOR_BLACK);
	//RESET BOX VARIABLE
	IM_MESSAGE_BOX->box_0 = 0;
	IM_MESSAGE_BOX->box_1 = 0;
	IM_MESSAGE_BOX->box_2 = 0;
	IM_MESSAGE_BOX->box_3 = 0;
}

uint8_t  ICACHE_FLASH_ATTR application_get_ip_address_string(char* str)
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
