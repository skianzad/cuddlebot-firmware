/*

Cuddlemaster - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#ifndef _MOTOR_H_
#define _MOTOR_H_

#include <ch.h>
#include <hal.h>

/* Motor driver state. */
typedef struct MotorDriver MotorDriver;

/* Motor driver instance. */
struct MotorDriver {
	PWMDriver *pwm;
	ioportid_t enport;
	ioportmask_t enpad;
	// private configuration
	pwmcnt_t pwmoffset;
	int8_t pwmstate;
};

/*

Start motor driver.

@param md The motor driver
@param pwm The PWM driver configuration

*/
void motorStart(MotorDriver *md, PWMConfig *pwm);

/*

Stop motor driver.

@param md The motor driver

*/
void motorStop(MotorDriver *md);

/*

Set motor output.

@param md The motor driver
@param p integer between -127 and 127

*/
void motorSet(MotorDriver *md, int8_t p);

#endif /* _MOTOR_H_ */
