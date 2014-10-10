/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

/* RS-485 wire protocol message definitions. */

#ifndef _MSGTYPE_H_
#define _MSGTYPE_H_

#define MSGTYPE_NOOP               0
#define MSGTYPE_PINGREQ            1
#define MSGTYPE_PINGRESP           2
#define MSGTYPE_SETPOINTREQ        31
#define MSGTYPE_SETPOINTRESP       32
#define MSGTYPE_ERRORRESP          255

typedef struct {} msgtype_noop_t;
typedef msgtype_noop_t msgtype_pingreq_t;
typedef msgtype_noop_t msgtype_pingresp_t;

typedef struct {
	uint16_t duration;
	uint16_t setpoint;
} msgtype_setpoint_t;

typedef struct {
	uint8_t loop;
	msgtype_setpoint_t points[1];
} msgtype_setpointreq_t;

typedef msgtype_noop_t msgtype_setpointresp_t;

#endif // _MSGTYPE_H_
