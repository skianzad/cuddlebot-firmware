/*

Cuddlemaster - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <stdbool.h>
#include <stdint.h>

#include <ch.h>
#include <hal.h>

#include "addresscfg.h"

// Board address.
cm_address_t cm_address = ADDRESS_INVALID;

/*
Read the board address and save to `cm_address`.

The actuator boards are arranged from rear to head of the CuddleBot
in the following order:

		Conn	ADDR0/1		Slave				H1,H0,L1,L0	ADDR
Rear	CN105	VCC HIZ		ADDRESS_RIBS		1,1,1,0		0x01
		CN107	VCC VCC		ADDRESS_PURR		1,1,1,1		0x02
		CN103	VCC GND		ADDRESS_HEAD_PITCH	1,0,1,0		0x04
		CN102	GND VCC		ADDRESS_SPINE		0,1,0,1		0x08
Head	CN101	GND GND		ADDRESS_HEAD_YAW	0,0,0,0		0x10
*/
void cm_address_init(void) {
	uint32_t addr = 0;

	// set addrout to vdd
	palSetPadMode(GPIOC, GPIOC_ADDROUT, PAL_MODE_INPUT_PULLDOWN);
	// wait to stabilize
	chThdSleepMicroseconds(10);
	// read L0
	if (palReadPad(GPIOC, GPIOC_ADDR0)) {
		addr |= 1 << 0;
	}
	// read L1
	if (palReadPad(GPIOC, GPIOC_ADDR1)) {
		addr |= 1 << 1;
	}

	// set addrout to vcc
	palSetPadMode(GPIOC, GPIOC_ADDROUT, PAL_MODE_INPUT_PULLUP);
	// wait to stabilize
	chThdSleepMicroseconds(10);
	// read H0
	if (palReadPad(GPIOC, GPIOC_ADDR0)) {
		addr |= 1 << 2;
	}
	// read H1
	if (palReadPad(GPIOC, GPIOC_ADDR1)) {
		addr |= 1 << 3;
	}

	// disable addrout port
	palSetPadMode(GPIOC, GPIOC_ADDROUT, PAL_MODE_INPUT);

	// map addresses
	switch (addr) {
		case 0b1110: cm_address = ADDRESS_RIBS; break;
		case 0b1111: cm_address = ADDRESS_PURR; break;
		case 0b1010: cm_address = ADDRESS_HEAD_PITCH; break;
		case 0b0101: cm_address = ADDRESS_SPINE; break;
		case 0b0000: cm_address = ADDRESS_HEAD_YAW; break;
		default: cm_address = ADDRESS_INVALID;
	}
}

/*
Check if the address is address to the board.

The input address
*/
bool cm_address_is_self(cm_address_t addr) {
	return (addr & cm_address) != 0 ? true : false;
}
