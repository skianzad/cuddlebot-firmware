/*

Cuddlemaster - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#ifndef _MOTOR_H_
#define _MOTOR_H_

// Initialize motor.
void cm_motor_init(void);

// Enable motor.
void cm_motor_enable(void);

// Disable motor.
void cm_motor_disable(void);

/*
Set motor output.

@param p integer between -2048 and 2048
*/
void cm_motor_set(int16_t p);

#endif /* _MOTOR_H_ */
