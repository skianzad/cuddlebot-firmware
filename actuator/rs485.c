/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <ch.h>
#include <hal.h>

#include "rs485.h"

void rsdInit(void) {
}

void rsdStart(void) {
	// start serial driver 3
	sdStart(&SD3, NULL);
	// enable RS-485 driver
	palSetPad(GPIOB, GPIOB_RS485_TXEN);
}

void rsdStop(void) {
	// disable RS-485 driver
	palClearPad(GPIOB, GPIOB_RS485_TXEN);
	// stop USART driver
	sdStop(&SD3);
}
