/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <ch.h>
#include <hal.h>

// #include <chprintf.h>

#include "addr.h"
#include "comm.h"
#include "crc32.h"
#include "msgtype.h"
#include "rs485.h"

void commStart(RS485Driver *sdp) {
	(void)sdp;
}

void commStop(RS485Driver *sdp) {
	(void)sdp;
}

void commRestart(RS485Driver *sdp) {
	(void)sdp;
}

msg_t commReceive(RS485Driver *sdp, msgtype_header_t *header,
                  char *buf, size_t len) {

	// BaseSequentialStream *chp = (BaseSequentialStream *)sdp;

	(void)sdp;
	(void)header;
	(void)buf;
	(void)len;

	msg_t ret;

	for (;;) {

		// receive address, message type, and message size
		chnRead(sdp, (uint8_t *)header, sizeof(msgtype_header_t));

		// buffer should be large enough to hold data
		if (header->size > len) {
			return RDY_RESET;
		}

		// ignore messages not addressed to self
		if (!addrIsSelf(header->addr)) {
			// also ignore crc16 checksum
			header->size += sizeof(msgtype_footer_t);
			// take bytes off queue
			while (header->size--) {
				ret = chnGetTimeout(sdp, MS2ST(10));
				if (ret < RDY_OK) {
					return ret;
				}
			}
			continue;
		}

		// read with a timeout long enough to accept all data, plus 10ms slack
		ret = chnReadTimeout(sdp, (uint8_t *)buf, header->size,
		                     MS2ST((10240000 / 11520) + 11));
		if (ret < RDY_OK) {
			return ret;
		}

		// read footer
		msgtype_footer_t footer;
		ret = chnReadTimeout(sdp, (uint8_t *)&footer,
		                     sizeof(msgtype_footer_t), MS2ST(10));
		if (ret < RDY_OK) {
			return ret;
		}

		// verify checksum
		uint16_t checksum = crc32(buf, header->size);
		if (checksum != footer.crc16) {
			return RDY_RESET;
		}

		// all ok
		return RDY_OK;
	}

	return RDY_OK;
}
