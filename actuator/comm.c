/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <ch.h>
#include <hal.h>

#include "comm.h"
#include "msgtype.h"

/*

Calculate a CRC-32 checksum.

@param buf The data buffer
@param len The length of the data

*/
uint32_t comm_lld_crc32(const uint8_t *buf, const size_t len) {
	size_t i;

	// reset CRC-32 unit
	CRC->CR = 1;

	// update CRC-32 calculation per byte
	for (i = 0; i < len; i++) {
		CRC->DR = buf[i];
	}

	// return hardware-calculated value
	return CRC->DR;
}

/*

Fill buffer to length.

@param comm The serial driver
@param meta The message metadata

*/
msg_t comm_lld_fillbuff(CommDriver *comm, CommMeta *meta) {
	msg_t ret;

	// ignore messages not addressed to self
	// ignore message data larger than the size of the buffer
	if (!comm->acb(meta->addr) || meta->len > sizeof(comm->buf)) {
		// ignore data and checksum
		uint16_t i;
		for (i = 0; i < meta->len + 2; i++) {
			ret = sdGetTimeout(comm->sd, comm->timeout);
			if (ret < RDY_OK) {
				return ret;
			}
		}
		// return
		if (meta->len > sizeof(comm->buf)) {
			return RDY_RESET;
		} else {
			return RDY_OK;
		}
	}

	// read data
	ret = sdReadTimeout(comm->sd, comm->buf, meta->len, comm->timeout);
	if (ret < 0) {
		return ret;
	}

	// read checksum
	uint16_t crcexpect;
	ret = sdReadTimeout(comm->sd, (uint8_t *)&crcexpect, 2, comm->timeout);
	if (ret < 0) {
		return ret;
	}

	// calculate checksum
	uint16_t crc = comm_lld_crc32(meta->buf, meta->len);

	// ignore messages not matching checksum
	if (meta->len && crcexpect != crc) {
		return RDY_RESET;
	}

	// assign buf
	meta->buf = comm->buf;

	// return ok
	return RDY_OK;
}

/*

Receive request-response, ignoring invalid messages.

@param comm The serial driver
@param meta Struct with which to read/write message metadata

*/
msg_t comm_lld_receive(CommDriver *comm, CommMeta *meta) {
	msg_t ret;

	// read address, message type, and data length
	ret = sdReadTimeout(comm->sd, &meta->addr, 4, comm->timeout);
	if (ret < 0) {
		return ret;
	}

	// if data is available...
	if (meta->len) {

		// fill buffer
		ret = comm_lld_fillbuff(comm, meta);
		if (ret < RDY_OK) {
			return ret;
		}

	} else {

		// otherwise reset buffer
		meta->buf = NULL;

	}

	// valid message
	return RDY_OK;
}

/*

Transmit a response.

@param comm The serial driver
@param meta The message metadata

*/
msg_t comm_lld_transmit(CommDriver *comm, CommMeta *meta) {
	msg_t ret;

	// calculate checksum
	uint16_t crc = 0;
	if (meta->len) {
		crc = comm_lld_crc32(meta->buf, meta->len);
	}

	// send header
	ret = sdWriteTimeout(comm->sd, &meta->addr, 4, comm->timeout);
	if (ret < RDY_OK) {
		return ret;
	}

	// send data
	if (meta->len) {
		ret = sdWriteTimeout(comm->sd, meta->buf, meta->len, comm->timeout);
		if (ret < RDY_OK) {
			return ret;
		}
	}

	// send checksum
	ret = sdWriteTimeout(comm->sd, (uint8_t *)&crc, 2, comm->timeout);
	if (ret < RDY_OK) {
		return ret;
	}

	// return ok
	return RDY_OK;
}

/*

Listen for and service master commands.

This function listens for requests from the master and services them
synchronously. Batched commands sent on the bus are serviced as they
arrive and responded to in turn. For example, if the master sends a
request to clients 1, 5, and 4, the clients will listen on the bus and
wait to respond in the same order.

@param arg The serial driver

*/
msg_t comm_lld_thread(void *arg) {
	CommDriver *comm = (CommDriver *)arg;
	CommMeta meta;

	// terminate the thread if no callback given
	if (comm->scb == NULL) {
		chThdExit(RDY_RESET);
	}

	while (!chThdShouldTerminate()) {

		// receive master requests
		if (comm_lld_receive(comm, &meta) < RDY_OK) {
			// restart serial driver to clear buffers
			sdStop(comm->sd);
			sdStart(comm->sd, NULL);
			// continue receiving
			continue;
		}

		// service requests
		msg_t ret = comm->scb(&meta);
		if (ret < RDY_OK) {
			int16_t err = ret;
			// transmit error
			meta.type = MSGTYPE_ERRORRESP;
			meta.buf = (uint8_t *)&err;
			meta.len = 2;
		}

		// transmit response
		comm_lld_transmit(comm, &meta);
	}

	return RDY_OK;
}

void commStart(CommDriver *comm) {
	// enable RS-485 driver
	palSetPad(comm->txenport, comm->txenpad);
	// start serial driver
	sdStart(comm->sd, NULL);
	// start listening thread
	comm->tp = chThdCreateStatic(comm->wa, sizeof(comm->wa),
	                             comm->prio, comm_lld_thread, comm);
}

void commStop(CommDriver *comm) {
	// terminate sampling thread
	if (comm->tp) {
		chThdTerminate(comm->tp);
		chThdWait(comm->tp);
		comm->tp = NULL;
	}
	// stop serial driver
	sdStop(comm->sd);
	// disable RS-485 driver
	palClearPad(comm->txenport, comm->txenpad);
}
