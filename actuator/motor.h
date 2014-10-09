/*

Cuddlemaster - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#ifndef _MOTOR_H_
#define _MOTOR_H_

/* Motor driver state. */
typedef struct MotorDriver MotorDriver;

/* Motor driver instance. */
extern MotorDriver MD1;

/* PWM configuration for Maxon motors. */
extern PWMConfig MaxonPWMConfig;

/* PWM configuration for Purr motors. */
extern PWMConfig PurrPWMConfig;

/* Start motor driver. */
void motorStart(MotorDriver *md, PWMConfig *pwm);

/* Stop motor driver. */
void motorStop(MotorDriver *md);

/*
  Set motor output.

  @param p integer between -127 and 127
*/
void motorSet(MotorDriver *md, int8_t p);

#endif /* _MOTOR_H_ */
