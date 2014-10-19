/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#ifndef _STATE_H_
#define _STATE_H_

#include <ch.h>
#include <hal.h>

#include "comm.h"
#include "msgtype.h"
#include "rs485.h"

/*

Service messages from the RS-485 bus.

@param sd The serial driver
@param header The message header
@param buf The message data buffer

*/
msg_t stateCommCallback(RS485Driver *sd,
                        const msgtype_header_t *header,
                        const char *buf);

#endif // _STATE_H_
