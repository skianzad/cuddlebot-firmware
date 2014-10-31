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
#include "motion.h"
#include "msgtype.h"
#include "rs485.h"

CommDriver COMM1;

void commInit(void) {
	commObjectInit(&COMM1);
}

void commObjectInit(CommDriver *comm) {
	comm->state = COMM_STOP;
}

void commStart(CommDriver *comm, CommConfig *config) {
	if (comm->state == COMM_STOP) {
		comm->config = *config;
	}
	comm->state = COMM_READY;
}

/*

Receive master commands, ignoring messages not addressed to self.

@param chnp The comm channel
@param header Pointer to header struct to save header data
@param buf Buffer for data, should be at least 1KB

*/
msg_t comm_lld_receive(CommDriver *comm, msgtype_header_t *header,
                       void **buf) {

	msg_t ret;
	BaseChannel *chnp = comm->config.io.chnp;
	const size_t len = comm->config.object_size;
	*buf = NULL;

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

		// read rest of header
		uint8_t *bp = &header->type;
		const size_t readlen = sizeof(*header) - sizeof(header->addr);
		size_t n = chnReadTimeout(chnp, bp, readlen, MS2ST(10));

		// check header and buffer should be large enough to hold data
		if (n != readlen || header->size > len) {
			goto error;
		}

		// receive data
		if (header->size) {
			// allocate memory
			*buf = chPoolAlloc(comm->config.pool);
			if (*buf == NULL) {
				return RDY_RESET;
			}
			// read with a timeout long enough to accept all data, plus 10ms slack
			const systime_t timeout =
			  MS2ST((len * 1000 / SERIAL_DEFAULT_BITRATE / 10) + 11);
			ret = chnReadTimeout(chnp, *buf, header->size, timeout);
			if (ret < RDY_OK) {
				goto error;
			}
		}

		// read footer
		msgtype_footer_t footer;
		ret = chnReadTimeout(chnp, (uint8_t *)&footer,
		                     sizeof(msgtype_footer_t), MS2ST(10));
		if (ret < RDY_OK) {
			goto error;
		}

		// ignore messages not addressed to self
		if (!addrIsSelf(header->addr)) {
			continue;
		}

		// calculate checksum
		// NOTE: Do NOT use CRC calculation unit elsewhere!
		crcReset();
		crcUpdateN((uint8_t *)header, sizeof(*header));
		if (*buf != NULL) {
			crcUpdateN(*buf, header->size);
		}
		uint16_t crc16 = crcValue();

		// verify checksum
		if (crc16 != footer.crc16) {
			goto error;
		}

		// all ok
		break;
	}

	return RDY_OK;

error:
	if (*buf != NULL) {
		chPoolFree(comm->config.pool, *buf);
		*buf = NULL;
	}
	return RDY_RESET;
}

/*

Service messages from master.

The following human-testable commands are implemented:

  ?   ping: the actuator transmits a "."
  t   test: the actuator runs local tests and prints the results
  v   value: the actuator prints the value of the position sensor

The commands, when not addressed via an envelope, will only work with
one actuator connected to the RS-485 bus.

@param chnp The comm channel
@param header The message header
@param dp The message data buffer

*/
msg_t comm_lld_service(CommDriver *comm,
                       const msgtype_header_t *header,
                       void **dp) {

	PIDConfig pidcfg = {0, 0, 0, 0, 0};
	msgtype_setpid_t *coeff = NULL;

	float p;

	BaseSequentialStream *chp = comm->config.io.chp;

	switch (header->type) {

	// human-testable commands

	case MSGTYPE_PING:
		chprintf(chp, "%c\r\n", MSGTYPE_PONG);
		break;
	case MSGTYPE_TEST:
		chprintf(chp, "Hello World!\r\n");
		break;
	case MSGTYPE_VALUE:
		p = motionGetPosition(&MOTION2);
		chprintf(chp, "%d.%03d\r\n", (int)(p),
		         (int)(1000 * fmod(copysign(p, 1.0), 1.0)));
		break;

	// computer commands

	case MSGTYPE_SETPID:
		if (*dp == NULL) {
			return RDY_RESET;
		}
		coeff = *dp;
		pidcfg.kp = coeff->kp;
		pidcfg.ki = coeff->ki;
		pidcfg.kd = coeff->kd;
		motionSetCoeff(&MOTION2, &pidcfg);
		break;

	case MSGTYPE_SETPOINT:
		if (*dp == NULL) {
			return RDY_RESET;
		}
		if (chMBPost(comm->config.mbox, (msg_t)*dp, TIME_IMMEDIATE) == RDY_OK) {
			*dp = NULL;
		}
		break;

	// invalid commands

	default:
		return RDY_RESET;
	}

	return RDY_OK;
}

msg_t commHandle(CommDriver *comm) {
	msgtype_header_t header;
	void *buf = NULL;
	msg_t ret = RDY_OK;

	ret = comm_lld_receive(comm, &header, &buf);
	if (ret == RDY_OK) {
		ret = comm_lld_service(comm, &header, &buf);
	}

	if (buf != NULL) {
		chPoolFree(comm->config.pool, buf);
	}

	return ret;
}
