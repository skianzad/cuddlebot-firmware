/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <ch.h>
#include <hal.h>
#include <chprintf.h>

#include "addr.h"
#include "motor.h"
#include "msgtype.h"
#include "state.h"
#include "rs485.h"

msg_t stateCommCallback(RS485Driver *sd,
                        const msgtype_header_t *header,
                        const char *buf) {
	msg_t ret = RDY_OK;

	BaseSequentialStream *chp = (BaseSequentialStream *)sd;

	(void)sd;
	(void)header;
	(void)buf;

	switch (header->type) {
	case MSGTYPE_PING: {
		msgtype_shortmsg_t shortmsg = {addrGet(), MSGTYPE_PONG, 0, 0};
		ret = chnWrite(sd, (uint8_t *)&shortmsg, sizeof(msgtype_shortmsg_t));
		break;
	}

	case MSGTYPE_SETPID:
		break;

	case MSGTYPE_SETPOINT:
		break;

	case MSGTYPE_TEST:
		chprintf(chp, "Hello World!\r\n");
		break;

	case MSGTYPE_VALUE:
		chprintf(chp, "%d\r\n", motorGet());
		break;

	default:
		return RDY_RESET;
	}

	return ret;
}
