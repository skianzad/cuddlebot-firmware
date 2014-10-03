/**
 *  \file main.c
 *  Created: 2012-07-18
 *  Author: Brett McGill
 */

#include "config.h"
#include "hostComms.h"
#include "fastCode.h"
#include "pins.h"
#include "adc.h"
#include "pwm.h"
#include "feedback.h"
#include "address.h"
#include "trajectories.h"
#include "main.h"
#include "watchdog.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_pwr.h"
#include "system_stm32f4xx.h"
#include <stdbool.h>
#include <stdint.h>

//-------------------------------------------------------------------------
// system checking

/** system check
 * call periodically, but not too frequently. Say every 500ms to 2000ms
 *
 *  Checks one of several processes, round-robin
 * \return true if all OK, false otherwise
 */
static bool check_system_is_ok(void) {
	static uint system_check = 0;

	system_check++;

	// do one test per call to this function
	switch (system_check) {
	/// MUST: number from 1, with no gaps
	case  1: if (!check_watchdog_is_ok() ) return false; break;
	case  2: if (!isFastCodeOk()) return false; break;
	/// add cases for other check functions as required
	default:
		// start test cycle from beginning
		system_check = 0;
		break;
	}
	return true;
}

//-------------------------------------------------------------------------
// main

#define BREATHING_OFFSET -0.139f
#define HEAD_RL_OFFSET 0.902f
#define HEAD_NOD_OFFSET  1.28560102f
#define SPINE_OFFSET 0.442f
#define PURR_OFFSET 0.0f

// values for timer timeouts
#define BATTERY_CHECK_SERVICE_INTERVAL  100 // ms
#define SYSTEM_CHECK_INTERVAL           500 // ms
#define STATE_CHECK_INTERVAL              2 // ms
#define ERROR_CHECK_INTERVAL            200 // ms

bool debugFlag = false;
float debugSetpoint = 0.0f;
float debugTime = 0.0f;
float m_startupTimer = 0.0f;
float debugArray[16384];
int debugArrayIndex = 0;
int debugDecimate = 0;

int main(void) {
	// add other timers here as required for timing periodic tasks.
	static uint32_t  system_check_timer_start;
	static uint32_t m_blinkTimer;

	// DO THIS FIRST
#ifndef __FPU_USED
#error "FPU off!"
#endif


	// DO THIS SECOND
	SystemInit();
	SystemCoreClockUpdate();

	// DO THIS THIRD
	addressInit();

	pinsInit();
	adcInit();
	hostCommsInit();
	pwmInit();
	trajectoriesInit();
	feedbackInit();

	// initialise local timers
	restart_1ms_timer(system_check_timer_start);
	restart_1ms_timer(m_blinkTimer);

	// do this when everything has been set up and just before entering main loop
	enable_watchdog(NORMAL_WD_WAKEUP_TIME);
	fastCodeInit();
	enable_interrupts();

	/// main processing loop of "Slow-Code", will run forever
	/// all system components must be configured before entering the main loop

	while (1) {
		// Kick Watchdog:
		// Watchdog hardware will reset the micro if main() doesn't call this within the alloted time frame
		// Only call this function from within the main loop (NEVER from any functions called from here)
		main_hit_wd();

		//pinsInit();//it should be ok to run this over and over.

		//enable temp compensation of position sensor and turn it on
		clrPosSensorPwrDwn();
		setPosSensorTccEn();
		//process any packets
		hostCommsSlowCode();
		addressDetectSlowCode();
		//calculate feedback

		if (isSampleReady()) {
			m_startupTimer += getSampleTime();
			calcFeedback(getNextValue(getSampleTime()), getPosition(), getCurrent(), getTorque());
		}

#ifdef FAKE_TRAJECTORIES
		FeedbackType fbType;
		static uint32_t trajectoryPeriod = 1000;
		static uint32_t m_trajectoryTimer;

		if (m_startupTimer < 0.01f) {
			updateCoeffs(5.0f, 0.0f, 0.0f, 0.0f, FB_DISABLED);
			updateTrajectories(0.0f, 0.0f, 0.01f);
		}

		//blink LED with 1Hz cycle
		//uint32_t temp = getAdcDebug();
		if (has_1ms_timer_expired(m_trajectoryTimer, trajectoryPeriod)) {
			restart_1ms_timer(m_trajectoryTimer);

			float posOffset;

			switch (getAddress()) {
			case ADDRESS_BREATHING:
				posOffset = BREATHING_OFFSET;
				fbType = FB_POSITION;
				trajectoryPeriod = 2000;
				updateTrajectories(0.8f, 0.0f, 0.66666f);
				updateTrajectories(-0.8f, 0.0f, 1.0f);
				break;
			case ADDRESS_HEAD_NOD:
				posOffset = HEAD_NOD_OFFSET;
				fbType = FB_POSITION;
				trajectoryPeriod = 30000;
				updateTrajectories(0.8f, 0.0f, 1.0f);
				updateTrajectories(0.0f, 0.0f, 1.0f);
				updateTrajectories(0.0f, 0.0f, 7.0f);
				updateTrajectories(-0.8f, 0.0f, 1.0f);
				updateTrajectories(0.0f, 0.0f, 1.0f);
				break;
			case ADDRESS_HEAD_RL:
				posOffset = HEAD_RL_OFFSET;
				fbType = FB_POSITION;
				trajectoryPeriod = 25000;
				updateTrajectories(0.8f, 0.0f, 1.0f);
				updateTrajectories(0.0f, 0.0f, 1.0f);
				updateTrajectories(0.0f, 0.0f, 5.0f);
				updateTrajectories(-0.8f, 0.0f, 1.0f);
				updateTrajectories(0.0f, 0.0f, 1.0f);
				break;
			case ADDRESS_NONE:
			default:
				posOffset = 0;
				fbType = FB_DISABLED;
				trajectoryPeriod = 1000;
				break;
			case ADDRESS_PURR:
				posOffset = PURR_OFFSET;
				fbType = FB_POSITION;
				trajectoryPeriod = 1000;
				break;
			case ADDRESS_SPINE:
				posOffset = SPINE_OFFSET;
				fbType = FB_POSITION;
				trajectoryPeriod = 20000;
				updateTrajectories(0.8f, 0.0f, 5.0f);
				updateTrajectories(0.0f, 0.0f, 5.0f);
				updateTrajectories(-0.8f, 0.0f, 5.0f);
				break;
			}
		}
#endif

		if (has_1ms_timer_expired(m_blinkTimer, 1000)) {
			togLedD201();//TODO: Debug LED
			restart_1ms_timer(m_blinkTimer);
		}

		// perform regular system check
		if (has_1ms_timer_expired(system_check_timer_start, SYSTEM_CHECK_INTERVAL)) {
			// reset the timer
			restart_1ms_timer(system_check_timer_start);
			if (!check_system_is_ok()) {
				// do nothing - each check function reinitialises its local block if there is a problem
				// watchdog_die(); // alternative: restart CPU if a problem is detected
			}
		}
	}
}

