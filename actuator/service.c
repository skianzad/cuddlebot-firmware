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

#include "motor.h"
#include "msgtype.h"
#include "service.h"

msg_t serviceHandler(SerialDriver *sd, msgtype_header_t *header) {
	BaseSequentialStream *chp = (BaseSequentialStream *)sd;
	msg_t ret = RDY_OK;

	switch (header->type) {
	case MSGTYPE_PING:
		ret = sdPut(sd, MSGTYPE_PONG);
		break;

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
