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
#include "commtest.h"
#include "crc16.h"
#include "motor.h"
#include "motion.h"
#include "msgtype.h"
#include "render_pid.h"
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

	BaseChannel *chnp = comm->config.io.chnp;
	const size_t len = comm->config.object_size;
	*buf = NULL;
	size_t n;

	for (;;) {

		// read header
		chSequentialStreamRead(chnp, &header->addr, 1);

		// handle human-testable commands
		switch (header->addr) {
		case MSGTYPE_SLEEP:
		case MSGTYPE_PING:
		case MSGTYPE_TEST:
		case MSGTYPE_VALUE:
			header->type = header->addr;
			header->size = 0;
			return RDY_OK;
		}

		// ignore messages not addressed to self
		if (!addrIsSelf(header->addr)) {
			rs485Wait(comm->config.io.rsdp);
			continue;
		}

		// read rest of header
		uint8_t *bp = &header->type;
		const size_t htlen = sizeof(*header) - sizeof(header->addr);
		n = chnReadTimeout(chnp, bp, htlen, MS2ST(10));

		// check header and buffer should be large enough to hold data
		if (n != htlen || header->size > len) {
			goto error;
		}

		// receive data
		if (header->size) {
			// allocate memory
			*buf = chPoolAlloc(comm->config.pool);
			if (*buf == NULL) {
				goto error;
			}
			// read with a timeout long enough to accept all data
			n = chnReadTimeout(chnp, *buf, header->size, S2ST(1));
			if (n != header->size) {
				goto error;
			}
		}

		// read footer
		msgtype_footer_t footer;
		const size_t tlen = sizeof(msgtype_footer_t);
		n = chnReadTimeout(chnp, (uint8_t *)&footer, tlen, MS2ST(10));
		if (n != tlen) {
			goto error;
		}

		// calculate checksum
		crc16_t c;
		crc16Reset(&c);
		crc16UpdateN(&c, (uint8_t *)header, sizeof(*header));
		if (*buf != NULL) {
			crc16UpdateN(&c, *buf, header->size);
		}
		uint16_t crc16 = crc16Value(&c);

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

	BaseSequentialStream *chp = comm->config.io.chp;

	switch (header->type) {

	// human-testable commands

	case MSGTYPE_SLEEP: {
		if (*dp == NULL) {
			*dp = chPoolAlloc(comm->config.pool);
		}

		msgtype_setpoint_t *sb = *dp;
		sb->delay = 0;
		sb->loop = 0;
		sb->n = 1;
		sb->setpoints[0].duration = 0;
		sb->setpoints[0].setpoint = 0;

		// post stop message
		chMBReset(comm->config.mbox);
		if (chMBPost(comm->config.mbox, (msg_t)*dp, TIME_IMMEDIATE) == RDY_OK) {
			*dp = NULL;
		}

		break;
	}

	case MSGTYPE_PING:
		chprintf(chp, "%c\r\n", MSGTYPE_PONG);
		break;

	case MSGTYPE_TEST:
		commtestAll(comm);
		break;

	case MSGTYPE_VALUE: {
		float p = 0;
		if (!addrIsPurr()) {
			p = pidrdValue(&PIDRENDER1);
		}
		chprintf(chp, "%d.%03d\r\n", (int)(p),
		         (int)(1000 * fmod(copysign(p, 1.0), 1.0)));
		break;
	}

	// computer commands

	case MSGTYPE_SETPID: {
		if (*dp == NULL || addrIsPurr()) {
			return RDY_RESET;
		}
		msgtype_setpid_t *coeff = *dp;
		PIDConfig pidcfg = {coeff->kp, coeff->ki, coeff->kd, 0, 1000};
		pidrdSetCoeff(&PIDRENDER1, &pidcfg);
		break;
	}

	case MSGTYPE_SETPOINT:
		if (*dp == NULL) {
			return RDY_RESET;
		}
		if (chMBPost(comm->config.mbox, (msg_t)*dp, TIME_IMMEDIATE) == RDY_OK) {
			*dp = NULL;
		}
		break;
	
	case MSGTYPE_SMOOTH: {
		if (*dp == NULL || addrIsPurr()) {
			return RDY_RESET;
		}

		/*msgtype_setpoint_t *sb = NULL;
		sb = chPoolAlloc(comm->config.pool);
		sb->delay = 0;
		sb->loop = 1;
		sb->n = 1;
		sb->setpoints[0].duration = 1000;
		sb->setpoints[0].setpoint = 0;

		if (chMBPost(comm->config.mbox, (msg_t)sb, TIME_IMMEDIATE) == RDY_OK) {
			*dp = NULL;
		}
		break;*/

// (pos-offset)/(hibound-offset) *65535

		msgtype_smooth_t *instructs = *dp;
		float curr_pos = pidrdValue(&PIDRENDER1);
		uint16_t currsetpt = (uint16_t) ( 65535*(curr_pos-MD1.offset)/(MD1.hibound-MD1.offset) );

		msgtype_setpoint_t *interval_setpoints = NULL;
		interval_setpoints = chPoolAlloc(comm->config.pool);
		interval_setpoints->delay = 0;
		interval_setpoints->loop = 1;
		uint16_t n = (uint16_t)floor(instructs->time / SMOOTH_MININTERVAL_MS);
		interval_setpoints->n = n+1;
		uint16_t setpt_diffs = (uint16_t)floor((instructs->setpoints->setpoint - currsetpt)/n);

		uint16_t i;
		for (i=0; i<n-1; i++) {
			interval_setpoints->setpoints[i].duration = SMOOTH_MININTERVAL_MS;
			interval_setpoints->setpoints[i].setpoint = currsetpt+(i+1)*setpt_diffs;
		}
		interval_setpoints->setpoints[n].duration = instructs->setpoints->duration;
		interval_setpoints->setpoints[n].setpoint = instructs->setpoints->setpoint;
		
		if (chMBPost(comm->config.mbox, (msg_t)interval_setpoints, TIME_IMMEDIATE) == RDY_OK) {
			*dp = NULL;
		}
		break;
	}


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
