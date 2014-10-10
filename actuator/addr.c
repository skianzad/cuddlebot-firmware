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

uint8_t _local_addr = ADDR_INVALID;

void addrLoad(void) {
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
	case 0b0000: addrGet() = ADDR_HEAD_PITCH; break;
	case 0b1010: addrGet() = ADDR_HEAD_YAW; break;
	case 0b0101: addrGet() = ADDR_SPINE; break;
	case 0b1111: addrGet() = ADDR_PURR; break;
	case 0b1101: addrGet() = ADDR_RIBS; break;
	default: addrGet() = ADDR_INVALID;
	}
}

bool addrIsSelf(uint8_t addr) {
	return (addr & _local_addr) != 0 ? true : false;
}
