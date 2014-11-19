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

#include "pid.h"

void pidObjectInit(PIDDriver *pid) {
	// reset coefficients
	pid->kp = 0;
	pid->ki = 0;
	pid->kd = 0;
	// reset starting setpoint
	pid->setpoint = 0;
	// reset state
	pid->lasterr = 0;
	pid->integrator = 0;
}

void pidStart(PIDDriver *pid, const PIDConfig *config) {
	pidSetCoeff(pid, config);
	pid->setpoint = config->setpoint;
}

void pidSetCoeff(PIDDriver *pid, const PIDConfig *config) {
	// set coefficients
	pid->kp = config->kp;
	pid->ki = config->ki / config->frequency;
	pid->kd = config->kd / config->frequency;
}

float pidSetpoint(PIDDriver *pid, float setpoint) {
	// edge case: if previous setpoint is infinite
	if (isinf(pid->setpoint) || isnan(pid->setpoint)) {
		pid->setpoint = 0.0;
	}
	// calculate change
	float delta = setpoint - pid->setpoint;
	// constrain delta to [-1 deg, +1 deg]
	if (fabs(delta) > 2 * M_PI / 360.0) {
		delta = copysign(delta, 1.0);
		// increment setpoint
		pid->setpoint += delta;
	} else {
		// save setpoint
		pid->setpoint = setpoint;
	}
	// read actual value
	return pid->setpoint;
}

float pidUpdate(PIDDriver *pid, float pos) {
	// calculate error
	float error = pid->setpoint - pos;

	// add error to integrator
	pid->integrator += error;

	// integrator upper bound
	if (pid->integrator > 127.0) {
		pid->integrator = 127.0;
	}
	// integrator lower bound
	else if (pid->integrator < -127.0) {
		pid->integrator = -127.0;
	}
	// edge case: if integrator becomes invalid
	else if (isinf(pid->integrator) || isnan(pid->integrator)) {
		pid->integrator = 0.0;
	}

	// calculate output
	float output = pid->kp * error;
	output += pid->ki * pid->integrator;
	output -= pid->kd * (error - pid->lasterr);

	// save last error
	pid->lasterr = error;

	// output upper bound
	if (output > 127.0) {
		output = 127.0;
	}
	// output lower bound
	else if (output < -127.0) {
		output = -127.0;
	}
	// edge case: if output becomes invalid
	else if (isinf(output) || isnan(output)) {
		output = 0.0;
	}

	// return result
	return output;
}
