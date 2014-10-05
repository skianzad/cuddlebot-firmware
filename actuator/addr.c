/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <ch.h>
#include <hal.h>

#include "addr.h"

cm_address_t local_addr = ADDRESS_INVALID;

void addrRead(void) {
	uint32_t addr = 0;

	// set addrout to output push-pull
	palSetPadMode(GPIOC, GPIOC_ADDROUT, PAL_MODE_OUTPUT_PUSHPULL);

	// set to low
	palClearPad(GPIOC, GPIOC_ADDROUT);
	// wait to stabilize
	chThdSleepMicroseconds(10);
	// read L0
	if (palReadPad(GPIOC, GPIOC_ADDR0)) {
		addr |= 0b0001;
	}
	// read L1
	if (palReadPad(GPIOC, GPIOC_ADDR1)) {
		addr |= 0b0010;
	}

	// set addrout to high
	palSetPad(GPIOC, GPIOC_ADDROUT);
	// wait to stabilize
	chThdSleepMicroseconds(10);
	// read H0
	if (palReadPad(GPIOC, GPIOC_ADDR0)) {
		addr |= 0b0100;
	}
	// read H1
	if (palReadPad(GPIOC, GPIOC_ADDR1)) {
		addr |= 0b1000;
	}

	// disable addrout port
	palSetPadMode(GPIOC, GPIOC_ADDROUT, PAL_MODE_INPUT);

	// map addresses
	switch (addr) {
	case 0b0000: local_addr = ADDRESS_HEAD_PITCH; break;
	case 0b1010: local_addr = ADDRESS_HEAD_YAW; break;
	case 0b0101: local_addr = ADDRESS_SPINE; break;
	case 0b1111: local_addr = ADDRESS_PURR; break;
	case 0b1101: local_addr = ADDRESS_RIBS; break;
	default: local_addr = ADDRESS_INVALID;
	}
}

bool addrIsSelf(cm_address_t addr) {
	return (addr & local_addr) != 0 ? true : false;
}
