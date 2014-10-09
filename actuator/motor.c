/*

Cuddlemaster - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include <ch.h>
#include <hal.h>

#include "addr.h"
#include "motor.h"

void motorStart(MotorDriver *md, PWMConfig *pwmcfg) {
	// reset state
	md->offset = pwmcfg->period - 127;
	md->pwmstate = 0;
	// disable the motor driver
	palClearPad(md->enport, md->enpad);
	// start the pwm peripherable
	pwmStart(md->pwm, pwmcfg);
}

void motorStop(MotorDriver *md) {
	// disable the motor driver
	palClearPad(md->enport, md->enpad);
	// stop the pwm peripherable
	pwmStop(md->pwm);
}

void motorSet(MotorDriver *md, int8_t p) {
	// input is restricted to [-128, 127] by data type
	if (p == -128) {
		p = -127;
	}

	if (md->pwmstate == p) {

		// no-op

	} else if (p == 0) {

		// disable motors
		palClearPad(md->enport, md->enpad);

	} else {
		// start motors
		if (md->pwmstate == 0) {
			palSetPad(md->enport, md->enpad);
		}

		// new direction when signs don't match
		bool newdir = (md->pwmstate == 0) || ((md->pwmstate < 0) ^ (p < 0));

		// update forces
		if (p > 0) {
			pwmEnableChannel(md->pwm, 0, md->offset + p);
			if (newdir) {
				pwmDisableChannel(md->pwm, 1);
			}
		} else {
			if (newdir) {
				pwmDisableChannel(md->pwm, 0);
			}
			pwmEnableChannel(md->pwm, 1, md->offset - p);
		}
	}

	// update state
	md->pwmstate = p;
}
