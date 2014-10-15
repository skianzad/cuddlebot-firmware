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
#define ADDR_RIBS                       'r' // ribs actuator
#define ADDR_PURR                       'p' // purr motor
#define ADDR_SPINE                      's' // spine actuator
#define ADDR_HEAD_YAW                   'x' // head yaw actuator
#define ADDR_HEAD_PITCH                 'y'	// head pitch actuator

/* Message types. */
#define MSGTYPE_PING                    '?' // ping an actuator
#define MSGTYPE_PONG                    '.' // respond to ping
#define MSGTYPE_SETPID                  'c' // send PID coefficients
#define MSGTYPE_SETPOINT                'p' // send setpoints
#define MSGTYPE_TEST                    't' // run internal tests
#define MSGTYPE_VALUE                   'v' // get position value

#pragma pack(push, 1)  /* set alignment to 1 byte boundary */

/* Message header. */
typedef struct {
	uint16_t addr;                        // offset 0x00, board address
	uint16_t type;                        // offset 0x02, message type
	uint16_t size;                        // offset 0x04, message size
} msgtype_header_t;

/* Message to set PID coefficients. */
typedef struct {
	uint16_t kp;                          // offset 0x00, P coefficient
	uint16_t ki;                          // offset 0x02, I coefficient
	uint16_t kd;                          // offset 0x04, D coefficient
} msgtype_setpid_t;

/* Setpoint. */
typedef struct {
	uint16_t duration;                    // offset 0x00, duration in ms
	uint16_t setpoint;                    // offset 0x02, setpoint
} msgtype_setpoint_data_t;

/* Setpoint group. */
typedef struct {
	uint16_t loop;                        // offset 0x00, loop
	uint16_t n;                           // offset 0x02, # of points
	msgtype_setpoint_data_t points[0];    // offset 0x04, setpoints
} msgtype_setgroup_t;

/* Message to send next setpoints. */
typedef struct {
	uint16_t delay;                       // offset 0x00, delay in ms
	uint16_t n;                           // offset 0x02, # of groups
	msgtype_setgroup_t groups[0];         // offset 0x04, setgroups
} msgtype_setpoint_t;

#pragma pack(pop)   /* restore original alignment from stack */

#endif // _MSGTYPE_H_
