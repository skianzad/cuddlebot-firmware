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

/* Low level metadata structure. */
typedef struct {
	uint8_t addr;
	uint8_t type;
	uint16_t len;
	void *buf;
} comm_lld_meta_t;

/*

Check if message type has data.

@param type The message type

*/
bool comm_lld_expect_data(uint8_t type) {
	switch (type) {
	case MSGTYPE_SETPOINTREQ:
	case MSGTYPE_SETPOINTRESP:
		return true;
	default:
		return false;
	}
}

/*

Check if data length matches expected length.

@param type The message type
@param len The message data length

*/
bool comm_lld_verify_length(uint8_t type, uint16_t len) {
	(void)len;

	switch (type) {
	case MSGTYPE_SETPOINTREQ:
	case MSGTYPE_SETPOINTRESP:
	// return true;
	default:
		return false;
	}
}

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

Receive request-response, ignoring invalid messages.

@param sd The serial driver
@param meta Struct with which to read/write message metadata
@param buf The buffer to store message data, must be at least 1KB
@param blen The length of the buffer
@return true if a message was received, false otherwise

*/
msg_t comm_lld_receive(SerialDriver *sd, comm_lld_meta_t *meta, uint8_t *buf, size_t blen) {
	msg_t ret;

	// reset meta buffer
	meta->buf = NULL;
	meta->len = 0;

	// read address and message type
	ret = sdReadTimeout(sd, &meta->addr, 2, MS2ST(1));
	if (ret < 0) {
		return ret;
	}

	// special case for messages with no data
	if (!comm_lld_expect_data(meta->type)) {
		return RDY_OK;
	}

	// read length
	sdRead(sd, (uint8_t *)&meta->len, 2);

	// verify data length
	uint16_t len;
	if (!comm_lld_verify_length(meta->type, len)) {
		return RDY_RESET;
	}

	// ignore messages not addressed to self or larger than the size of
	// the buffer
	if (!addrIsSelf(meta->addr) || len > blen) {
		// ignore data
		for (ret = 0; ret < len; ret++) {
			sdGet(sd);
		}
		// ignore checksum
		sdGet(sd);
		sdGet(sd);
		// return
		return RDY_RESET;
	}

	// read data
	ret = sdRead(sd, meta->buf, len);
	if (ret < 0) {
		return ret;
	}

	// read checksum
	uint16_t checksum;
	ret = sdRead(sd, (uint8_t *)&checksum, 2);
	if (ret < 0) {
		return ret;
	}

	// ignore messages not matching checksum
	if (checksum != (uint16_t)comm_lld_crc32(buf, len)) {
		return RDY_RESET;
	}

	// set buffer
	meta->buf = buf;
	meta->len = len;

	// valid message
	return RDY_OK;
}

/*

Service a request.

@param meta The message metadata
@param buf The message data buffer

*/
msg_t comm_lld_service(comm_lld_meta_t *meta) {
	switch (meta->type) {
	case MSGTYPE_PINGREQ:
	case MSGTYPE_SETPOINTREQ:
		break;
	default:
		// error
		break;
	}
	return RDY_OK;
}

/*

Transmit a response.

@param sd The serial driver
@param type The message type
@param buf The message data buffer
@param len The message data length

*/
msg_t comm_lld_transmit(SerialDriver *sd, comm_lld_meta_t *meta) {
	sdPut(sd, meta->addr);
	sdPut(sd, meta->type);
	if (meta->buf != NULL && meta->len > 0) {
		sdWrite(sd, meta->buf, meta->len);
	}
	return RDY_OK;
}

void commStart(SerialDriver *sd) {
	// start serial driver
	sdStart(sd, NULL);
	// enable RS-485 driver
	palSetPad(GPIOB, GPIOB_RS485_TXEN);
}

void commStop(SerialDriver *sd) {
	// disable RS-485 driver
	palClearPad(GPIOB, GPIOB_RS485_TXEN);
	// stop serial driver
	sdStop(sd);
}

void commListen(SerialDriver *sd, commscb_t scb) {
	comm_lld_meta_t meta;
	uint8_t buf[1024];

	// terminate the thread if no callback given
	if (scb == NULL) {
		chThdExit(RDY_RESET);
	}

	for (;;) {
		// receive master requests
		if (comm_lld_receive(sd, &meta, buf, sizeof(buf)) < RDY_OK) {
			continue;
		}
		// service requests
		msg_t ret = scb(meta.type, meta.buf, meta.len);
		if (ret < RDY_OK) {
			int16_t err = ret;
			// transmit error
			meta.type = MSGTYPE_ERRORRESP;
			meta.buf = (uint8_t *)&err;
			meta.len = 2;
		}
		// transmit response
		comm_lld_transmit(sd, &meta);
	}
}
