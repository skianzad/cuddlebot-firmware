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

PIDConfig DefaultPIDConfig = {
	.kp = 127.0 / M_PI,
	.ki = 1.0,
	.kd = -1.0,
	.setpoint = 2.5,
	.frequency = 1000
};

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
	// set coefficients
	pid->kp = config->kp;
	pid->ki = config->ki / config->frequency;
	pid->kd = config->kd / config->frequency;
	// set starting setpoint
	pid->setpoint = config->setpoint;
}

void pidSetCoeff(PIDDriver *pid, const PIDConfig *config) {
	// set coefficients
	pid->kp = config->kp;
	pid->ki = config->ki / config->frequency;
	pid->kd = config->kd / config->frequency;
}

float pidSetpoint(PIDDriver *pid, float setpoint) {
	// calculate change
	float delta = setpoint - pid->setpoint;
	// constrain delta to [-1 deg, +1 deg]
	if (fabs(delta) > M_2_PI / 360.0) {
		delta = copysign(1.0, delta);
		// increment setpoint
		pid->setpoint += delta;
	} else {
		// save setpoint
		pid->setpoint = setpoint;
	}
	// read actual value
	return pid->setpoint;
}

float pidUpdate(PIDDriver *pid, float value) {
	// calculate error
	float error = value - pid->setpoint;
	// add error to integrator
	pid->integrator += error;
	// calculate output
	float output = pid->kp * error;
	output += pid->ki * pid->integrator;
	output += pid->kd * (error - pid->lasterr);
	// save last error
	pid->lasterr = error;
	// return result
	return -fmod(output, 127.0f);
}
