/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <ch.h>
#include <hal.h>
#include <chprintf.h>

#include "addr.h"
#include "comm.h"
#include "commtest.h"
#include "render_ps.h"

void commtestAll(CommDriver *comm) {
	commtestMotion(comm);
}

void commtestMotion(CommDriver *comm) {
	BaseSequentialStream *chp = comm->config.io.chp;
	msgtype_setpoint_t *sb = NULL;

	sb = chPoolAlloc(comm->config.pool);
	if (sb != NULL) {
		sb->delay = 0;
		sb->loop = MSGTYPE_LOOP_INFINITE;
		sb->n = 1;
		sb->setpoints[0].duration = MSGTYPE_LOOP_INFINITE;
		if (addrIsPurr()) {
			ps_setpoint_t *sp = (ps_setpoint_t *)&sb->setpoints[0].setpoint;
			sp->pulse_duration = 10;
			sp->step = 63;
		} else {
			sb->setpoints[0].setpoint = 15488;
		}
		chMBPost(comm->config.mbox, (msg_t)sb, TIME_INFINITE);
	}

	sb = chPoolAlloc(comm->config.pool);
	if (sb != NULL) {
		sb->delay = 2000;
		sb->loop = 4;
		sb->n = 2;
		if (addrIsPurr()) {
			ps_setpoint_t *sp = NULL;
			sb->setpoints[0].duration = 1000;
			sp = (ps_setpoint_t *)&sb->setpoints[0].setpoint;
			sp->pulse_duration = 10;
			sp->step = 10;
			sb->setpoints[1].duration = 2000;
			sp = (ps_setpoint_t *)&sb->setpoints[1].setpoint;
			sp->pulse_duration = 10;
			sp->step = 80;
		} else {
			sb->setpoints[0].duration = 1000;
			sb->setpoints[0].setpoint = 1500;
			sb->setpoints[1].duration = 2000;
			sb->setpoints[1].setpoint = 10000;
		}
		chMBPost(comm->config.mbox, (msg_t)sb, TIME_INFINITE);
	}

	chprintf(chp, "Testing motor control...\r\n");
	chThdSleepSeconds(6);
}
