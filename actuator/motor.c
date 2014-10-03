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

#define PWM_PERIOD 2048

// PWM configuration.
static const PWMConfig pwmcfg = {
	.frequency = (200000 * PWM_PERIOD),       // 819200 kHz, 200 kHz pulse
	.period = PWM_PERIOD,                     // 4096 cycles per period
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

/*

Initialize motor.

*/
void cm_motor_init(void) {
	// currently no-op
}

/*

Enable motor.

*/
void cm_motor_enable(void) {
	// enable the motor driver chip
	palSetPad(GPIOB, GPIOB_MOTOR_EN);
	// start the pwm peripherable
	pwmStart(&PWMD1, &pwmcfg);
}

/*

Disable motor.

*/
void cm_motor_disable(void) {
	// stop the pwm peripherable
	pwmStop(&PWMD1);
	// disable the motor driver chip
	palClearPad(GPIOB, GPIOB_MOTOR_EN);
}

/*

Set motor output.

@param p integer between -2048 and 2048

*/
void cm_motor_set(int16_t p) {
	// previous pwm state
	static int16_t pwmstate = 0;

	// bound input
	if (p > PWM_PERIOD) {
		p = PWM_PERIOD;
	} else if (p < -PWM_PERIOD) {
		p = -PWM_PERIOD;
	}

	if (pwmstate == p) {
		// no-op
	} else if (p == 0) {
		// disable motors
		cm_motor_disable();
	} else {
		// start motors
		if (pwmstate == 0) {
			cm_motor_enable();
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
