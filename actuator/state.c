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

msg_t stateUpdate(const msgtype_header_t *header, const char *dp) {
	msg_t ret = RDY_OK;
	(void)dp;

	switch (header->type) {
	case MSGTYPE_SETPID:
		break;

	case MSGTYPE_SETPOINT:
		break;
	}

	return ret;
}
