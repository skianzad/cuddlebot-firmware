/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <math.h>
#include <stdint.h>

#include "motor.h"
#include "render.h"
#include "render_ps.h"

PSRenderDriver PSRENDER1;

static void will_render(void *instance) {
	PSRenderDriver *rdp = instance;
	(void)rdp;
}

static void render(void *instance, uint16_t setpoint) {
	PSRenderDriver *rdp = instance;
	int8_t pwm = 0;
	// calculate pulse-step duration
	if (rdp->setpoint.v != setpoint) {
		uint8_t prev_step = rdp->setpoint.ps.step;
		// update setpoint
		rdp->setpoint.v = setpoint;
		// set pulse magnitude and direction
		if (prev_step < rdp->setpoint.ps.step) {
			rdp->pulse_pwm = 127;
		} else {
			rdp->pulse_pwm = -127;
		}
		// save pulse duration
		rdp->pulse_duration = rdp->setpoint.ps.pulse_duration;
	}
	// pulse for duration
	if (rdp->pulse_duration > 0) {
		pwm = rdp->pulse_pwm;
		rdp->pulse_duration--;
	} else {
		pwm = rdp->setpoint.ps.step;
	}
	// apply output
	motorSetI(pwm);
}

static void has_rendered(void *instance) {
	PSRenderDriver *rdp = instance;
	(void)rdp;
}

static const struct PSRenderDriverVMT vmt = {
	will_render, render, has_rendered
};

void psrdObjectInit(PSRenderDriver *rdp) {
	rdp->vmt = &vmt;
	rdp->pulse_duration = 0;
	rdp->setpoint.v = 0;
	rdp->pulse_pwm = 0;
}
