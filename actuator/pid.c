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
#include "sensor.h"

msg_t pidCalibrate(PIDDriver *pid) {
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
	motorSet(pid->md, 75);
	chThdSleepSeconds(1);

	// sample position sensor
	sensor_vitals_t vitals;
	msg_t ret = sensorConvert(&vitals);
	if (ret != RDY_OK) {
		return ret;
	}

	// save starting offset
	pid->offset = vitals.position;

	// send motor the other way
	motorSet(pid->md, -75);
	chThdSleepSeconds(1);

	// save limit offset
	pid->limit = vitals.position;

	// disable motor
	motorSet(pid->md, 0);

	// return ok
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
