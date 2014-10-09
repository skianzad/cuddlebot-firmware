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
struct MotorDriver {
  PWMDriver *pwm;
  ioportid_t enport;
  ioportmask_t enpad;
  pwmcnt_t offset;
  int8_t pwmstate;
};

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
