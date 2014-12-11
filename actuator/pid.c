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

void pidReset(PIDDriver *pid, float pos) {
	// reset state
	pid->lasterr = 0;
	pid->integral = 0;
	// reset starting setpoint
	pid->setpoint = pos;
}

void pidObjectInit(PIDDriver *pid) {
	// reset coefficients
	pid->kp = 0;
	pid->ki = 0;
	pid->kd = 0;
	// reset state
	pidReset(pid, 0);
}

void pidStart(PIDDriver *pid, const PIDConfig *config) {
	pidSetCoeff(pid, config);
	pidReset(pid, config->setpoint);
}

void pidSetCoeff(PIDDriver *pid, const PIDConfig *config) {
	// set coefficients
	pid->kp = config->kp;
	pid->ki = config->ki / config->frequency;
	pid->kd = config->kd * config->frequency;
}

float pidSetpoint(PIDDriver *pid, float setpoint) {
	// edge case: if setpoint is invalid
	if (isinf(pid->setpoint) || isnan(pid->setpoint)) {
		pid->setpoint = 0.0;
	}
	// limit noise
	if (fabs(setpoint - pid->setpoint) > 0.01) {
		// save setpoint
		pid->setpoint = setpoint;
	}
	// edge case: if setpoint is invalid
	if (isinf(pid->setpoint) || isnan(pid->setpoint)) {
		pid->setpoint = 0.0;
	}
	// read actual value
	return pid->setpoint;
}

float pidUpdate(PIDDriver *pid, float pos) {
	// calculate error
	float error = pid->setpoint - pos;

	// add error to integral
	pid->integral += error;

	// integral upper bound
	if (pid->integral > 127.0) {
		pid->integral = 127.0;
	}
	// integral lower bound
	else if (pid->integral < -127.0) {
		pid->integral = -127.0;
	}
	// edge case: if integral becomes invalid
	else if (isinf(pid->integral) || isnan(pid->integral)) {
		pid->integral = 0.0;
	}

	const float derivative = error - pid->lasterr;

	// calculate output
	float output = pid->kp * error;
	output += pid->ki * pid->integral;
	output += pid->kd * derivative;

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
