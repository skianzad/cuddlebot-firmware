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

static void reset(void *instance) {
	PIDRenderDriver *rdp = instance;
	float pos = motorCPosition();
	pidReset(&rdp->pid, pos);
	rdp->pos = pos;
}

static void will_render(void *instance) {
	PIDRenderDriver *rdp = instance;
	float pos = motorCPosition();
	// save new position if moved more the noise
	if (fabs(rdp->pos - pos) > 0.01f) {
		rdp->pos = pos;
	}
}

static int8_t render(void *instance, uint16_t setpoint) {
	PIDRenderDriver *rdp = instance;
	// accept setpoint as percentage
	float sp = (float)setpoint / (float)0xffff;
	// use 5% high and lowmargin
	sp = (sp * 0.9 + 0.05) * motorHiBound();
	// edge cases
	if (sp < 0.01 || isnan(sp) || isinf(sp)) {
		sp = 0.0;
	}
	// update setpoint
	pidSetpoint(&rdp->pid, sp);
	// update PID state
	return pidUpdate(&rdp->pid, rdp->pos);
}

static void has_rendered(void *instance) {
	PIDRenderDriver *rdp = instance;
	(void)rdp;
}

static const struct PIDRenderDriverVMT vmt = {
	reset, will_render, render, has_rendered
};

void pidrdObjectInit(PIDRenderDriver *rdp) {
	rdp->vmt = &vmt;
	rdp->pos = 0.0;
	pidObjectInit(&rdp->pid);
}

void pidrdStart(PIDRenderDriver *rdp, PIDConfig *pidcfg) {
	pidStart(&rdp->pid, pidcfg);
	// set starting setpoint
	pidReset(&rdp->pid, motorCPosition());
}
