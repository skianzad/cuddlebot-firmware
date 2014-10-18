/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

/* RS-485 driver implementation. */

#ifndef _COMM_H_
#define _COMM_H_

#include <ch.h>
#include <hal.h>

#include "msgtype.h"

/*

Start the RS-485 driver.

This function listens for requests from the master and services them
synchronously. Batched commands sent on the bus are serviced as they
arrive. The master must ensure that at most one message requiring a
response be sent per batch, and that this message be the last sent.

@param sdp The serial driver

*/
void commStart(SerialDriver *sdp);

/*

Stop the RS-485 driver.

@param sdp The serial driver

*/
void commStop(SerialDriver *sdp);

/*

Receive master commands, ignoring messages not addressed to self.

@param sdp The serial driver
@param header Pointer to header struct to save header data
@param buf Buffer for data, should be at least 1KB
@param len Buffer size

*/
msg_t commReceive(SerialDriver *sdp, msgtype_header_t *header,
  char *buf, size_t len);

#endif // _COMM_H_
