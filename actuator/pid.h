/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#ifndef _PID_H_
#define _PID_H_

#include <ch.h>
#include <hal.h>

#include "motor.h"

typedef struct {
	// coefficients
	uint16_t setpoint;
	uint16_t kp;
	uint16_t ki;
	uint16_t kd;
	// private: calibration
	uint16_t offset;
	uint16_t limit;
	int8_t dir;
	// private: internal state
	int32_t lasterr;
	int32_t integrator;
} PIDDriver;

/*

Start and calibrate PID driver.

@param pid The PID driver

*/
msg_t pidStart(PIDDriver *pid);

/*

Set PID setpoint.

@param pid The PID driver
@param setpoint The setpoint

*/
int8_t pidUpdate(PIDDriver *pid, uint16_t setpoint);

#endif // _PID_H_
