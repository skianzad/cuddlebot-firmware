/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <ch.h>
#include <hal.h>

#include "addr.h"
#include "pid.h"

msg_t pidStart(PIDDriver *pid) {
	msg_t ret;
	uint16_t pos;

	// set motor direction based on position on board
	switch (addrGet()) {
	// case ADDR_HEAD_PITCH:
	case ADDR_HEAD_YAW:
		// case ADDR_SPINE:
		// case ADDR_PURR:
		// case ADDR_RIBS:
		pid->dir = -1;
		break;
	default:
		pid->dir = 1;
	}

	// send motor to starting position
	motorSet(75 * pid->dir);
	chThdSleepSeconds(1);

	// sample position sensor
	ret = motorPosition(&pos);
	if (ret != RDY_OK) {
		return ret;
	}

	// save starting offset
	pid->offset = pos;

	// send motor the other way
	motorSet(-75 * pid->dir);
	chThdSleepSeconds(1);

	// sample position sensor
	ret = motorPosition(&pos);
	if (ret != RDY_OK) {
		return ret;
	}

	// save limit offset
	pid->limit = pos;

	// disable motor
	motorSet(0);

	return RDY_OK;
}

int8_t pidUpdate(PIDDriver *pid, uint16_t value) {
	int32_t error = value - pid->setpoint;

	// TODO: handle overflow
	pid->integrator += error;

	int32_t output = pid->kp * error;
	output += pid->ki * pid->integrator;
	output += pid->kd * (error - pid->lasterr);

	pid->lasterr = error;

	return output >> 24;
}
