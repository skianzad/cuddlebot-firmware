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

#define MSGTYPE_MULTIMSG           0
#define MSGTYPE_PINGREQ            1
#define MSGTYPE_PINGRESP           2
#define MSGTYPE_SETPIDREQ          31
#define MSGTYPE_SETPIDRESP         32
#define MSGTYPE_SETPOINTREQ        33
#define MSGTYPE_SETPOINTRESP       34
#define MSGTYPE_ERRORRESP          255

#pragma pack(push, 1)  /* set alignment to 1 byte boundary */

typedef struct {} msgtype_noop_t;

typedef struct {
	uint8_t n;                            // offset 0x00
} msgtype_multimsg_t;

typedef msgtype_noop_t msgtype_pingreq_t;
typedef msgtype_noop_t msgtype_pingresp_t;

typedef struct {
	uint16_t kp;                          // offset 0x00
	uint16_t ki;                          // offset 0x02
	uint16_t kd;                          // offset 0x04
	uint16_t setpoint;                    // offset 0x08
} msgtype_setpidreq_t;

typedef struct {
	uint16_t duration;                    // offset 0x00
	uint16_t setpoint;                    // offset 0x02
} msgtype_setpoint_t;

typedef struct {
	uint16_t delay;                       // offset 0x00
	uint16_t loop;                        // offset 0x02
	msgtype_setpoint_t points[0];         // offset 0x04
} msgtype_setpointreq_t;

typedef msgtype_noop_t msgtype_setpointresp_t;

#pragma pack(pop)   /* restore original alignment from stack */

#endif // _MSGTYPE_H_
