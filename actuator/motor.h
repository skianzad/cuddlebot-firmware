/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#ifndef _MOTOR_H_
#define _MOTOR_H_

#include <ch.h>
#include <hal.h>

/* Start motor driver. */
void motorStart(void);

/* Stop motor driver. */
void motorStop(void);

/*

Set motor output.

@param p integer between -127 and 127

*/
void motorSet(int8_t p);

/*

Get motor mosition.

@param p The address to store the rotary position of the motor between
          0 and 65535

*/
msg_t motorPosition(uint16_t *p);

#endif /* _MOTOR_H_ */
