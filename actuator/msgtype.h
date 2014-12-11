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

#include <stdint.h>

/* Board addresses. */
#define ADDR_INVALID                    0 // invalid address
#define ADDR_RIBS                       'r' // ribs actuator
#define ADDR_PURR                       'p' // purr motor
#define ADDR_SPINE                      's' // spine actuator
#define ADDR_HEAD_YAW                   'x' // head yaw actuator
#define ADDR_HEAD_PITCH                 'y'	// head pitch actuator

/* Message types. */
#define MSGTYPE_INVALID                 0 // invalid message
#define MSGTYPE_PING                    '?' // ping an actuator
#define MSGTYPE_PONG                    '.' // respond to ping
#define MSGTYPE_SETPID                  'c' // send PID coefficients
#define MSGTYPE_SETPOINT                'g' // send setpoints
#define MSGTYPE_SLEEP                   'z' // deactivate motor output
#define MSGTYPE_TEST                    't' // run internal tests
#define MSGTYPE_VALUE                   'v' // get position value

/* Setpoint loop special values. */
#define MSGTYPE_LOOP_INFINITE           0xffff

#pragma pack(push, 1)  /* set alignment to 1 byte boundary */

/* Message header. */
typedef struct {
	uint8_t addr;                         // offset 0x00, board address
	uint8_t type;                         // offset 0x02, message type
	uint16_t size;                        // offset 0x04, message size
} msgtype_header_t;

/* Message footer. */
typedef struct {
	uint16_t crc16;                       // offset 0x00, crc-16 checksum
} msgtype_footer_t;

/* Short message. */
typedef struct {
	uint8_t addr;                         // offset 0x00, board address
	uint8_t type;                         // offset 0x02, message type
	uint16_t size;                        // offset 0x04, message size
	uint16_t crc16;                       // offset 0x08, crc-16 checksum
} msgtype_shortmsg_t;

/* Message to set PID coefficients. */
typedef struct {
	float kp;                             // offset 0x00, P coefficient
	float ki;                             // offset 0x04, I coefficient
	float kd;                             // offset 0x08, D coefficient
} msgtype_setpid_t;

/* Setpoint. */
typedef struct {
	uint16_t duration;                    // offset 0x00, duration in ms
	uint16_t setpoint;                    // offset 0x02, setpoint
} msgtype_spvalue_t;

/* Message to send next setpoints. */
typedef struct {
	uint16_t delay;                       // offset 0x00, delay in ms
	uint16_t loop;                        // offset 0x02, loop
	uint16_t n;                           // offset 0x04, # of groups
	msgtype_spvalue_t setpoints[0];       // offset 0x08, setpoints
} msgtype_setpoint_t;

#pragma pack(pop)   /* restore original alignment from stack */

#endif // _MSGTYPE_H_
