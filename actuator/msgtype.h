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

/* Board addresses. */
#define ADDR_INVALID                    0
#define ADDR_ANY                        'a'
#define ADDR_RIBS                       'r'
#define ADDR_PURR                       'p'
#define ADDR_SPINE                      's'
#define ADDR_HEAD_YAW                   'x'
#define ADDR_HEAD_PITCH                 'y'

/* Message types. */
#define MSGTYPE_ACK                     '+' // acknowledge
#define MSGTYPE_NAK                     '-' // negative acknowledge
#define MSGTYPE_PREAMBLE                'h' // preamble header
#define MSGTYPE_PING                    '?'
#define MSGTYPE_PONG                    '.'
#define MSGTYPE_SETPID                  'c'
#define MSGTYPE_SETPOINT                'p'
#define MSGTYPE_TEST                    't'

#pragma pack(push, 1)  /* set alignment to 1 byte boundary */

typedef struct {
	uint8_t n;                            // offset 0x00, # of messages
} msgtype_header_t;

typedef struct {
	uint16_t kp;                          // offset 0x00, P coefficient
	uint16_t ki;                          // offset 0x02, I coefficient
	uint16_t kd;                          // offset 0x04, D coefficient
	uint16_t setpoint;                    // offset 0x08, setpoint
} msgtype_setpid_t;

typedef struct {
	uint16_t duration;                    // offset 0x00, duration in ms
	uint16_t setpoint;                    // offset 0x02, setpoint
} setpoint_t;

typedef struct {
	uint16_t delay;                       // offset 0x00, delay in ms
	uint16_t loop;                        // offset 0x02, loop
	uint16_t n;                           // offset 0x04, # of points
	setpoint_t points[0];                 // offset 0x08, setpoints
} msgtype_setpoint_t;

#pragma pack(pop)   /* restore original alignment from stack */

#endif // _MSGTYPE_H_
