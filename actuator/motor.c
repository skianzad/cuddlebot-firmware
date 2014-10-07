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
	PWMConfig config;
	pwmcnt_t offset;
	int8_t pwmstate;
};

MotorDriver MaxonDriver = {
	.config = {
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
	},
	.offset = 179,
	.pwmstate = 0
};

MotorDriver PurrDriver = {
	.config = {
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
	},
	.offset = 80,
	.pwmstate = 0
};

void motorStart(MotorDriver *md) {
	// disable the motor driver
	palClearPad(GPIOB, GPIOB_MOTOR_EN);
	// start the pwm peripherable
	pwmStart(&PWMD1, &md->config);
}

void motorStop(MotorDriver *md) {
	(void)md;

	// disable the motor driver
	palClearPad(GPIOB, GPIOB_MOTOR_EN);
	// stop the pwm peripherable
	pwmStop(&PWMD1);
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
		palClearPad(GPIOB, GPIOB_MOTOR_EN);

	} else {
		// start motors
		if (md->pwmstate == 0) {
			palSetPad(GPIOB, GPIOB_MOTOR_EN);
		}

		// new direction when signs don't match
		bool newdir = (md->pwmstate == 0) || ((md->pwmstate < 0) ^ (p < 0));

		// update forces
		if (p > 0) {
			pwmEnableChannel(&PWMD1, 0, md->offset + p);
			if (newdir) {
				pwmDisableChannel(&PWMD1, 1);
			}
		} else {
			if (newdir) {
				pwmDisableChannel(&PWMD1, 0);
			}
			pwmEnableChannel(&PWMD1, 1, md->offset - p);
		}
	}

	// update state
	md->pwmstate = p;
}
