/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <math.h>

#include <ch.h>
#include <hal.h>

#include "addr.h"
#include "pid.h"

msg_t pidStart(PIDDriver *pid) {
	// reset coefficients
	pid->kp = pid->ki = pid->kd = 0;
	// reset state
	pid->setpoint = pid->lasterr = pid->integrator = 0;

	return RDY_OK;
}

int8_t pidUpdate(PIDDriver *pid, float value) {
	float error = value - pid->setpoint;

	pid->integrator += error;

	float output = pid->kp * error;
	output += pid->ki * pid->integrator;
	output += pid->kd * (error - pid->lasterr);

	pid->lasterr = error;

	return -fmod(output, 127.0f);
}
