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

void pidInit(PIDDriver *pid, const PIDConfig *config) {
	// initialize mutex
	chMtxInit(&pid->lock);
	// set coefficients
	pid->kp = config->kp;
	pid->ki = config->ki / config->frequency;
	pid->kd = config->kd / config->frequency;
	// set starting setpoint
	pid->setpoint = config->setpoint;
	// reset state
	pid->lasterr = pid->integrator = 0;
}

void pidCoeff(PIDDriver *pid, const PIDConfig *config) {
	// lock PID controller
	chMtxLock(&pid->lock);
	// set coefficients
	pid->kp = config->kp;
	pid->ki = config->ki / config->frequency;
	pid->kd = config->kd / config->frequency;
	// unlock PID controller
	chMtxUnlock();
}

float pidSet(PIDDriver *pid, float setpoint) {
	// lock PID controller
	chMtxLock(&pid->lock);
	// calculate change
	float delta = setpoint - pid->setpoint;
	// constrain delta to [-1 deg, +1 deg]
	if (fabs(delta) > M_2_PI / 360.0) {
		delta = copysign(1.0, delta);
	}
	// update setpoint
	pid->setpoint += delta;
	// read actual value
	setpoint = pid->setpoint;
	// unlock PID controller
	chMtxUnlock();
	// return setpoint
	return setpoint;
}

float pidUpdate(PIDDriver *pid, float value) {
	// lock PID controller
	chMtxLock(&pid->lock);
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
	// unlock PID controller
	chMtxUnlock();
	// return result
	return -fmod(output, 127.0f);
}
