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
#include "rs485.h"

/*

Receive master commands, ignoring messages not addressed to self.

@param chnp The comm channel
@param header Pointer to header struct to save header data
@param buf Buffer for data, should be at least 1KB
@param len Buffer size

*/
msg_t commReceive(BaseChannel *chnp, msgtype_header_t *header,
                  char *buf, size_t len);

/*

Service messages from master.

The following human-testable commands are implemented:

  ?   ping: the actuator transmits a "."
  t   test: the actuator runs local tests and prints the results
  v   value: the actuator prints the value of the position sensor

The commands, when not addressed via an envelope, will only work with
one actuator connected to the RS-485 bus.

@param chnp The comm channel
@param header The message header
@param dp The message data buffer

*/
msg_t commService(BaseChannel *chnp, const msgtype_header_t *header,
                  const void *dp);

#endif // _COMM_H_
