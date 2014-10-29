/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <ch.h>
#include <hal.h>

#include "crc32.h"

void crcUpdateN(const uint8_t *buf, const size_t n) {
	size_t i = 0;
	for (i = 0; i < n; i++) {
		crcUpdate(buf[i]);
	}
}
