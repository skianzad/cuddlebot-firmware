/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <ch.h>
#include <hal.h>

#include "motion.h"
#include "motor.h"
#include "render.h"

MotionDriver MOTION2;

/* General Purpose Timer callback. */
void gpt_callback(GPTDriver *gptp) {
	if (gptp == &GPTD2) {
		chSysLockFromIsr();
		chBSemSignalI(&MOTION2.ready);
		chSysUnlockFromIsr();
	}
}

/* General Purpose Timer configuration for 1 Khz. */
const GPTConfig gptcfg = {
	.frequency = 10000,
	.callback = gpt_callback,
	// hardware-specfic configuration
	.dier = 0
};

void motion_lld_free_sp(MotionDriver *mdp) {
	if (mdp->sp != NULL) {
		chPoolFree(mdp->config.pool, mdp->sp);
		mdp->sp = NULL;
	}
}

void motion_lld_free_sp_if_empty(MotionDriver *mdp) {
	// free setpoints if empty
	if (mdp->sp->n == 0) {
		motion_lld_free_sp(mdp);
	}
}

void motion_lld_load_sp_data(MotionDriver *mdp) {
	if (mdp->sp != NULL) {
		msgtype_spvalue_t *spp = &mdp->sp->setpoints[mdp->spindex];
		mdp->duration = spp->duration;
		mdp->setpoint = spp->setpoint;
	}
}

void motion_lld_load_nextsp(MotionDriver *mdp) {
	// get next setpoints
	msg_t ptr = 0;
	if (mdp->nextsp == NULL &&
	    chMBFetch(mdp->config.mbox, &ptr, TIME_IMMEDIATE) == RDY_OK) {
		// new setpoints available
		mdp->nextsp = (msgtype_setpoint_t *)ptr;
		mdp->delay = mdp->nextsp->delay;
	}
}

void motion_lld_activate_sp_after_delay(MotionDriver *mdp) {
	// check delay for next setpoints
	if (mdp->nextsp != NULL) {
		if (mdp->delay > 0) {
			mdp->delay--;
		} else {
			motion_lld_free_sp(mdp);
			mdp->sp = mdp->nextsp;
			mdp->nextsp = NULL;

			// reset state for new setpoint
			mdp->loop = mdp->sp->loop;
			mdp->spindex = 0;
			motion_lld_load_sp_data(mdp);
		}
	}
}

void motion_lld_step_motion(MotionDriver *mdp) {
	// decrement duration if greater than zero, otherwise interpret as
	// duration of 1ms
	if (mdp->duration > 0) {
		mdp->duration--;
	}

	// move to next setpoint when duration reached
	if (mdp->duration == 0) {

		// duration has ended for this setpoint
		mdp->spindex++;

		// if all setpoints have been rendered, start over
		if (mdp->spindex >= mdp->sp->n) {
			// decrement loop count unless infinite
			if (mdp->loop != MSGTYPE_LOOP_INFINITE) {
				mdp->loop--;
			}
			// reset index
			mdp->spindex = 0;
		}

		// update state for next setpoint
		motion_lld_load_sp_data(mdp);
	}
}

bool motion_lld_has_update(MotionDriver *mdp) {
	// disable motor if there are no setpoints
	if (mdp->sp == NULL) {
		return false;
	}

	// check iteration count
	if (mdp->loop == MSGTYPE_LOOP_INFINITE) {
		// loop forever
	} else if (mdp->loop > 0) {
		// continue looping
	} else {
		// done with this loop
		motion_lld_free_sp(mdp);
		return false;
	}

	return true;
}

msg_t driver_thread(void *p) {
	MotionDriver *mdp = p;
	mdp->active = false;

	while (!chThdShouldTerminate()) {
		if (chBSemWait(&mdp->ready) != RDY_OK) {
			continue;
		}

		motion_lld_load_nextsp(mdp);
		motion_lld_activate_sp_after_delay(mdp);
		motion_lld_free_sp_if_empty(mdp);

		if (!motion_lld_has_update(mdp)) {
			motorSet(0);
			mdp->active = false;
			continue;
		}

		if (!mdp->active) {
			rdReset(mdp->config.render);
			mdp->active = true;
		}

		rdWillRender(mdp->config.render);

		int8_t pwm = rdRender(mdp->config.render, mdp->setpoint);
		motorSet(pwm);
		motion_lld_step_motion(mdp);

		rdHasRendered(mdp->config.render);
	}

	mdp->active = false;
	return RDY_OK;
}

void motionInit(void) {
	motionObjectInit(&MOTION2);
	MOTION2.gptp = &GPTD2;
}

void motionObjectInit(MotionDriver *mdp) {
	chBSemInit(&mdp->ready, FALSE);
	mdp->state = MOTION_STOP;
	mdp->sp = NULL;
	mdp->nextsp = NULL;
	mdp->spindex = 0;
	mdp->loop = 0;
}

void motionStart(MotionDriver *mdp, MotionConfig *mdcfg) {
	if (mdp->state == MOTION_STOP) {
		mdp->config = *mdcfg;

		// start rendering thread
		mdp->thread_tp = chThdCreateStatic(
		                   mdp->config.thread_wa,
		                   mdp->config.thread_wa_size,
		                   mdp->config.thread_prio,
		                   driver_thread, mdp);

		// start timer
		gptStart(mdp->gptp, &gptcfg);
		gptStartContinuous(mdp->gptp, 10);
	}

	mdp->state = MOTION_READY;
}

void motionStop(MotionDriver *mdp) {
	if (mdp->state == MOTION_READY) {

		// stop timer
		gptStopTimer(mdp->gptp);
		gptStop(mdp->gptp);

		// terminate thread
		if (mdp->thread_tp) {
			chThdTerminate(mdp->thread_tp);
			chThdWait(mdp->thread_tp);
			mdp->thread_tp = NULL;
		}

		// free buffer
		if (mdp->sp) {
			chPoolFree(mdp->config.pool, mdp->sp);
		}

		// reset state
		motionObjectInit(mdp);
	}

	mdp->state = MOTION_STOP;
}

msg_t motionSetpoint(MotionDriver *mdp, msgtype_setpoint_t *sp) {
	if (mdp->state == MOTION_READY) {
		return RDY_RESET;
	}
	return chMBPost(mdp->config.mbox, (msg_t)sp, TIME_IMMEDIATE);
}
