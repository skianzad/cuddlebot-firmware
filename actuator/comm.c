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
#include "comm.h"
#include "msgtype.h"

/* Message metadata. */
typedef struct {
	uint8_t addr;               // message address
	uint8_t type;               // message type
	uint16_t len;               // message data length
	uint8_t *buf;               // message data buffer
} comm_lld_meta;

/* Message data buffer */
static uint8_t comm_buf[1024];

/* Comm handler memory stack. */
static WORKING_AREA(comm_wa, 128);

/* Comm handler thread handle */
static Thread *comm_tp;

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

@param meta The message metadata

*/
msg_t comm_lld_fillbuff(comm_lld_meta *meta) {
	msg_t ret;

	// ignore messages not addressed to self
	// ignore message data larger than the size of the buffer
	if (!addrIsSelf(meta->addr) || meta->len > sizeof(comm_buf)) {
		// ignore data and checksum
		uint16_t i;
		uint16_t len = meta->len + 2;
		for (i = 0; i < len; i++) {
			ret = sdGetTimeout(&SD3, S2ST(10));
			if (ret < RDY_OK) {
				return ret;
			}
		}
		// return
		if (meta->len > sizeof(comm_buf)) {
			return RDY_RESET;
		} else {
			return RDY_OK;
		}
	}

	// read data
	ret = sdReadTimeout(&SD3, comm_buf, meta->len, S2ST(10));
	if (ret < 0) {
		return ret;
	}

	// read checksum
	uint16_t crcexpect;
	ret = sdReadTimeout(&SD3, (uint8_t *)&crcexpect, 2, S2ST(10));
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
	meta->buf = comm_buf;

	// return ok
	return RDY_OK;
}

/*

Receive request-response, ignoring invalid messages.

@param meta Struct with which to read/write message metadata

*/
msg_t comm_lld_receive(comm_lld_meta *meta) {
	msg_t ret;

	// read address, message type, and data length
	ret = sdReadTimeout(&SD3, &meta->addr, 4, S2ST(10));
	if (ret < 0) {
		return ret;
	}

	// if data is available...
	if (meta->len) {

		// fill buffer
		ret = comm_lld_fillbuff(meta);
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
msg_t comm_lld_transmit(comm_lld_meta *meta) {
	msg_t ret;

	// calculate checksum
	uint16_t crc = 0;
	if (meta->len) {
		crc = comm_lld_crc32(meta->buf, meta->len);
	}

	// send header
	ret = sdWriteTimeout(&SD3, &meta->addr, 4, MS2ST(1));
	if (ret < RDY_OK) {
		return ret;
	}

	// send data in 100ms => 1152 bytes @ 115200 baud
	if (meta->len) {
		ret = sdWriteTimeout(&SD3, meta->buf, meta->len, MS2ST(100));
		if (ret < RDY_OK) {
			return ret;
		}
	}

	// send checksum
	ret = sdWriteTimeout(&SD3, (uint8_t *)&crc, 2, MS2ST(1));
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

@param arg The service callback

*/
msg_t comm_lld_thread(void *arg) {
	commscb_t scb = (commscb_t)arg;
	comm_lld_meta meta;

	// terminate the thread if no callback given
	if (scb == NULL) {
		chThdExit(RDY_RESET);
	}

	while (!chThdShouldTerminate()) {

		// receive master requests
		if (comm_lld_receive(&meta) < RDY_OK) {
			// restart serial driver to clear buffers
			sdStop(&SD3);
			sdStart(&SD3, NULL);
			// continue receiving
			continue;
		}

		// service requests
		msg_t ret = scb(meta.type, meta.buf, meta.len);
		if (ret < RDY_OK) {
			int16_t err = ret;
			// transmit error
			meta.type = MSGTYPE_NAK;
			meta.buf = (uint8_t *)&err;
			meta.len = 2;
		}

		// transmit response
		comm_lld_transmit(&meta);
	}

	return RDY_OK;
}

void commStart(commscb_t scb) {
	// enable RS-485 driver
	palSetPad(GPIOB, GPIOB_RS485_TXEN);
	// start serial driver
	sdStart(&SD3, NULL);
	// start listening thread
	comm_tp = chThdCreateStatic(comm_wa, sizeof(comm_wa),
	                            LOWPRIO, comm_lld_thread, scb);
}

void commStop(void) {
	// terminate sampling thread
	if (comm_tp) {
		chThdTerminate(comm_tp);
		chThdWait(comm_tp);
		comm_tp = NULL;
	}
	// stop serial driver
	sdStop(&SD3);
	// disable RS-485 driver
	palClearPad(GPIOB, GPIOB_RS485_TXEN);
}
