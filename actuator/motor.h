/*

Cuddlemaster - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#ifndef _MOTOR_H_
#define _MOTOR_H_

/*

Start motor.

*/
void motorStart(void);

/*

Stop motor.

*/
void motorStop(void);

/*

Set motor output.

@param p integer between -128 and 127

*/
void motorSet(int8_t p);

#endif /* _MOTOR_H_ */
