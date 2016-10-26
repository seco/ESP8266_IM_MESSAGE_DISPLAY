/*************************************************
* ESP8266 HARDWARE SPI (HSPI) LIBRARY
*
* OCTOBER 26 2016
* ANKIT BHATNAGAR
* ANKIT.BHATNAGARINDIA@GMAIL.COM
* ***********************************************/


#include "spi.h"


#define ESP8266_SPI_cs_high GPIO_OUTPUT_SET(15, 1)
#define ESP8266_SPI_cs_low  GPIO_OUTPUT_SET(15, 0)

void ESP8266_SPI_init_pins(void)
{
	//INITIALIZE THE GPIO PINS USED FOR SPI
	//GPIO12 - MTDI - HSPI MISO
	//GPIO13 - MTCK - HSPI MOSI
	//GPIO14 - MTMS - HSPI CLK
	//GPIO15 - MTDO - HSPI CS

	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, 2);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, 2);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, 2);
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, 2);
}

void ESP8266_SPI_set_params(void)
{

}

void ESP8266_SPI_send(uint8_t data)
{

}

void ESP8266_SPI_get(uint8_t* data)
{

}
