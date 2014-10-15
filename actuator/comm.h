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

Message service callback.

@param sd The serial driver
@param header The message header

*/
typedef msg_t (*commscb_t)(SerialDriver *sd, msgtype_header_t *header);

/*

Start the RS-485 driver.

This function listens for requests from the master and services them
synchronously. Batched commands sent on the bus are serviced as they
arrive. The master must ensure that at most one message requiring a
response be sent per batch, and that this message be the last sent.

The service callback function `scb` is run in the same thread as the
serial handler and the header structure passed to it is only valid
until the function exits. The function must not use the header
structure after the function exits.

*/
void commStart(commscb_t scb);

/* Stop the RS-485 driver. */
void commStop(void);

#endif // _COMM_H_
