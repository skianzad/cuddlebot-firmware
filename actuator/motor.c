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

#include "motor.h"

#define PWM_PERIOD 128

// PWM configuration.
static const PWMConfig pwmcfg = {
	.frequency = 208333 * PWM_PERIOD,         // 26.67 MHz
	.period = PWM_PERIOD,                     // 208.3 KHz
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

void motorStart(void) {
	// disable the motor driver
	palClearPad(GPIOB, GPIOB_MOTOR_EN);
	// start the pwm peripherable
	pwmStart(&PWMD1, &pwmcfg);
}

void motorStop(void) {
	// disable the motor driver
	palClearPad(GPIOB, GPIOB_MOTOR_EN);
	// stop the pwm peripherable
	pwmStop(&PWMD1);
}

void motorSet(int8_t p) {
	// previous pwm state
	static int8_t pwmstate = 0;

	// input is restricted to [-128, 127] by data type
	if (p == -128) {
		p = -127;
	}

	if (pwmstate == p) {
		// no-op
	} else if (p == 0) {
		// disable motors
		palClearPad(GPIOB, GPIOB_MOTOR_EN);
	} else {
		// start motors
		if (pwmstate == 0) {
			palSetPad(GPIOB, GPIOB_MOTOR_EN);
		}
		// new direction when signs don't match
		bool newdir = (pwmstate < 0) ^ (p < 0);
		// update forces
		if (p > 0) {
			pwmEnableChannel(&PWMD1, 0, PWM_FRACTION_TO_WIDTH(&PWMD1, PWM_PERIOD, p));
			if (newdir) pwmDisableChannel(&PWMD1, 1);
		} else {
			if (newdir) pwmDisableChannel(&PWMD1, 0);
			pwmEnableChannel(&PWMD1, 1, PWM_FRACTION_TO_WIDTH(&PWMD1, PWM_PERIOD, -p));
		}
	}

	// update state
	pwmstate = p;
}
