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

struct MotorDriver {
	PWMDriver *pwm;
	ioportid_t enport;
	ioportmask_t enpad;
	pwmcnt_t offset;
	int8_t pwmstate;
};

MotorDriver MD1 = {
	.pwm = &PWMD1,
	.enport = GPIOB,
	.enpad = GPIOB_MOTOR_EN,
	.offset = 179,
	.pwmstate = 0
};

PWMConfig MaxonPWMConfig = {
	.frequency = 306 * 137255,                //  42.0 MHz; divider = 2
	.period = 306,                            // 137.3 KHz
	.callback = NULL,
	.channels = {
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_DISABLED, NULL},
		{PWM_OUTPUT_DISABLED, NULL}
	},
	// HW dependent part.
	.cr2 = 0,
	.dier = 0
};

PWMConfig PurrPWMConfig = {
	.frequency = 207 * 202898,                //  42.0 MHz; divider = 2
	.period = 207,                            // 137.3 KHz
	.callback = NULL,
	.channels = {
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_ACTIVE_HIGH, NULL},
		{PWM_OUTPUT_DISABLED, NULL},
		{PWM_OUTPUT_DISABLED, NULL}
	},
	// HW dependent part.
	.cr2 = 0,
	.dier = 0
};

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
