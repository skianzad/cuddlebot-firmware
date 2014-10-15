/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#ifndef _CRC32_H_
#define _CRC32_H_

#include <ch.h>
#include <hal.h>

#include "msgtype.h"

/*

Service messages from the RS-485 bus.

@param chp The serial driver
@param header The message header

*/
msg_t serviceHandler(SerialDriver *sd, msgtype_header_t *header);

#endif // _CRC32_H_
