/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <math.h>

#include <ch.h>
#include <hal.h>

#include <chprintf.h>

#include "addr.h"
#include "comm.h"
#include "crc32.h"
#include "motor.h"
#include "msgtype.h"
#include "rs485.h"

msg_t commReceive(BaseChannel *chnp, msgtype_header_t *header,
                  char *buf, size_t len) {

	msg_t ret;

	BaseSequentialStream *chp = (BaseSequentialStream *)chnp;

	for (;;) {

		// read header
		chSequentialStreamRead(chnp, &header->addr, 1);

		// handle human-testable commands
		switch (header->addr) {
		case MSGTYPE_PING:
		case MSGTYPE_TEST:
		case MSGTYPE_VALUE:
			header->type = header->addr;
			header->size = 0;
			return RDY_OK;
		}

		// read message type and message size
		uint8_t *bp = ((uint8_t *)&header->type);
		size_t n = chnReadTimeout(chnp, bp,
		                          sizeof(*header) - sizeof(header->addr),
		                          MS2ST(10));

		// check header and buffer should be large enough to hold data
		if (n != sizeof(*header) || header->size > len) {
			return RDY_RESET;
		}

		if (header->size) {
			// read with a timeout long enough to accept all data, plus 10ms slack
			ret = chnReadTimeout(chnp, (uint8_t *)buf, header->size,
			                     MS2ST((len * 1000 / 11520) + 11));
			if (ret < RDY_OK) {
				return ret;
			}
		}

		// read footer
		msgtype_footer_t footer;
		ret = chnReadTimeout(chnp, (uint8_t *)&footer,
		                     sizeof(msgtype_footer_t), MS2ST(10));
		if (ret < RDY_OK) {
			return ret;
		}

		rs485Wait((RS485Driver *)chnp);
		chprintf(chp, "\1\2\0\0");

		// ignore messages not addressed to self
		if (!addrIsSelf(header->addr)) {
			continue;
		}

		// verify checksum
		uint16_t checksum = crc32(buf, header->size);
		if (checksum != footer.crc16) {
			return RDY_RESET;
		}

		// all ok
		break;
	}

	return RDY_OK;
}

msg_t commService(BaseChannel *chnp, const msgtype_header_t *header,
                  const void *dp) {

	msg_t ret = RDY_OK;
	float p;
	(void)dp;

	BaseSequentialStream *chp = (BaseSequentialStream *)chnp;

	switch (header->type) {

	// human-testable commands

	case MSGTYPE_PING:
		chprintf(chp, "%c\r\n", MSGTYPE_PONG);
		break;
	case MSGTYPE_TEST:
		chprintf(chp, "Hello World!\r\n");
		break;
	case MSGTYPE_VALUE:
		p = motorCGet();
		chprintf(chp, "%d.%03d\r\n", (int)(p),
		         (int)(1000 * fmod(copysign(p, 1.0), 1.0)));
		break;

	// computer commands

	case MSGTYPE_SETPID:
	case MSGTYPE_SETPOINT:
		break;

	// invalid commands

	default:
		return RDY_RESET;
	}

	return ret;
}
