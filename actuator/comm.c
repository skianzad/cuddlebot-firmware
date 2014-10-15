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

/* Comm handler memory addr_queue. */
static WORKING_AREA(comm_wa, 128);

/* Comm handler thread handle */
static Thread *comm_tp;

/*

Listen for and service master commands.

@param arg The service callback

*/
msg_t comm_lld_thread(void *arg) {
	commscb_t scb = (commscb_t)arg;
	msgtype_header_t header;

	// terminate the thread if no callback given
	if (scb == NULL) {
		chThdExit(RDY_RESET);
	}

listen:

	// restart serial driver to reset queues
	sdStop(&SD3);
	chThdSleepMilliseconds(1);
	sdStart(&SD3, NULL);

	while (!chThdShouldTerminate()) {
		// receive address and message type
		sdRead(&SD3, (uint8_t *)&header, sizeof(header));

		// ignore messages not addressed to self
		if (!addrIsSelf(header.addr)) {
			// also ignore crc16 checksum
			header.size += 2;
			// take bytes off queue
			while (header.size--) {
				if (sdGetTimeout(&SD3, MS2ST(1)) < RDY_OK) {
					goto listen;
				}
			}
		}

		// service requests
		if (scb(&SD3, &header) < RDY_OK) {
			goto listen;
		}
	}

	return RDY_OK;
}

void commStart(commscb_t scb) {
	// enable RS-485 driver
	palSetPad(GPIOB, GPIOB_RS485_TXEN);
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
