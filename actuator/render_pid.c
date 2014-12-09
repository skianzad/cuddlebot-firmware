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
#include "pid.h"
#include "render.h"
#include "render_pid.h"

PIDRenderDriver PIDRENDER1;

static void will_render(void *instance) {
	PIDRenderDriver *rdp = instance;
	float pos = motorCPosition();
	// save new position if moved more the noise
	if (fabs(rdp->pos - pos) > 0.01f) {
		rdp->pos = pos;
	}
}

static void render(void *instance, uint16_t setpoint) {
	PIDRenderDriver *rdp = instance;
	// memoize setpoint conversion
	if (rdp->setpoint != setpoint) {
		rdp->setpointf = ((float)setpoint) * (motorHiBound() / 65535.0);
		if (rdp->setpointf > motorHiBound()) {
			rdp->setpointf = motorHiBound();
		}
	}
	// update setpoint
	pidSetpoint(&rdp->pid, rdp->setpointf);
	// update PID state
	int8_t pwm = pidUpdate(&rdp->pid, rdp->pos);
	// set motor output
	motorSetI(pwm);
}

static void has_rendered(void *instance) {
	PIDRenderDriver *rdp = instance;
	(void)rdp;
}

static const struct PIDRenderDriverVMT vmt = {
	will_render, render, has_rendered
};

void pidrdObjectInit(PIDRenderDriver *rdp) {
	rdp->vmt = &vmt;
	rdp->pos = 0.0;
	rdp->setpointf = 0.0;
	rdp->setpoint = 0;
	pidObjectInit(&rdp->pid);
}

void pidrdStart(PIDRenderDriver *rdp, PIDConfig *pidcfg) {
	pidStart(&rdp->pid, pidcfg);
	// set starting setpoint
	pidReset(&rdp->pid, motorCPosition());
}
