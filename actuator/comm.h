/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

/*

RS-485 driver implementation.

*/

#ifndef _COMM_H_
#define _COMM_H_

#define MSGTYPE_NOOP               0
#define MSGTYPE_PINGREQ            1
#define MSGTYPE_PINGRESP           2
#define MSGTYPE_SETPOINTREQ        31
#define MSGTYPE_SETPOINTRESP       32
#define MSGTYPE_ERRORRESP          255

typedef msg_t (*commscb_t)(uint8_t type, void *buf, uint16_t len);

/*

Start the RS-485 driver.

@param sd The serial driver

*/
void commStart(SerialDriver *sd);

/*

Stop the RS-485 driver.

@param sd The serial driver

*/
void commStop(SerialDriver *sd);

/*

Listen for master commands.

This function listens for requests from the master and services them
synchronously. Batched commands sent on the bus are serviced as they
arrive and responded to in turn. For example, if the master sends a
request to clients 1, 5, and 4, the clients will listen on the bus and
wait to respond in the same order.

@param sd The serial driver

*/
void commListen(SerialDriver *sd, commscb_t scb);

#endif // _COMM_H_
