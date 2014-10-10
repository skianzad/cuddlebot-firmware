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

/*

Message service callback.

@param type The message type
@param buf The message data buffer
@param len The message data length

*/
typedef msg_t (*commscb_t)(uint8_t type, void *buf, size_t len);

/*

Start the RS-485 driver.

The service callback function `scb` is run in the same thread as the
serial handler and the metadata structure passed to it is only valid
until the function exits. The function must not use the metadata
structure after the function exits.

*/
void commStart(commscb_t scb);

/* Stop the RS-485 driver. */
void commStop(void);

#endif // _COMM_H_
