/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

/*

RS-485 driver implementation.

*/

#ifndef _RS485_H_
#define _RS485_H_

/* Start the RS-485 driver. */
void rsdStart(SerialDriver *sd);

/* Stop the RS-485 driver. */
void rsdStop(SerialDriver *sd);

/* Listen for master commands. */
void rsdListen(SerialDriver *sd);

#endif // _RS485_H_
