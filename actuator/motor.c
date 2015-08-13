/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba, 2015 Hong Yue Sean Liu

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <math.h>

#include <ch.h>
#include <hal.h>

#include "addr.h"
#include "motor.h"
#include "msgtype.h"

MotorDriver MD1;

/* PWM configuration for Maxon motors. */
static PWMConfig pwmcfg = {
	.frequency = 306 * 137254,                //  42.0 MHz; divider = 2
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

// Number of channels to be sampled for ADC1.
#define ADC_GRP_NUM_CHANNELS 3

/*

ADC conversion group.

Mode:     linear buffer, 1 sample of 3 channels, SW triggered.

Timing:   15 cycles sample time
          15 cycles conversion time
          30 total cycles

          @ 21 Mhz ADC clock

          ~0.71 µs sample time
          ~0.71 µs conversion time
          ~1.42 µs total time

*/
static const ADCConversionGroup adcgrpcfg = {
	.circular = FALSE,                        // linear buffer
	.num_channels = ADC_GRP_NUM_CHANNELS,     // channels 8-13
	.end_cb = NULL,
	.error_cb = NULL,
	// hardware-specific configuration
	.cr1 = 0,
	.cr2 = ADC_CR2_SWSTART,
	.smpr1 = (
	  ADC_SMPR1_SMP_AN13(ADC_SAMPLE_15) |     // motor pos cos
	  ADC_SMPR1_SMP_AN12(ADC_SAMPLE_15)),     // motor pos sin
	.smpr2 = (
	  ADC_SMPR2_SMP_AN9(ADC_SAMPLE_15)),      // vref 1V65
	.sqr1 = ADC_SQR1_NUM_CH(ADC_GRP_NUM_CHANNELS),
	.sqr2 = 0,
	.sqr3 = (
	  ADC_SQR3_SQ3_N(ADC_CHANNEL_IN12) |      // pos cos
	  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN13) |      // pos sin
	  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN9))        // vref 1V65
};

void motorInit(void) {
	// adjust PWM config for purr motor
	if (addrIsPurr()) {
		pwmcfg.frequency = 207 * 202898;    //  42.0 MHz; divider = 2
		pwmcfg.period = 207;                // 137.3 KHz
	}
	motorObjectInit(&MD1);
}

void motorObjectInit(MotorDriver *mdp) {
	// reset state
	mdp->pwmoffset = pwmcfg.period - 127;
	mdp->pwmstate = 0;
	mdp->flags = 0;
}

void motorStart(void) {
	// disable the motor driver
	palClearPad(GPIOB, GPIOB_MOTOR_EN);
	// start the pwm peripherable
	pwmStart(&PWMD1, &pwmcfg);

	// enable temperature compensation
	palSetPad(GPIOB, GPIOB_POS_TCCEN);
	// enable position sensor
	palClearPad(GPIOB, GPIOB_POS_NEN);

	// start the adc peripherable
	adcStart(&ADCD1, NULL);

	// wait for sensor to power up (ChibiOS will delay 1ms)
	chThdSleepMicroseconds(110);
}

void motorStop(void) {
	// disable the motor driver
	palClearPad(GPIOB, GPIOB_MOTOR_EN);
	// stop the pwm peripherable
	pwmStop(&PWMD1);
	// disable position sensor
	palSetPad(GPIOB, GPIOB_POS_NEN);
}

msg_t motor_lld_sample_pos(float *pos) {
	// sample buffer
	adcsample_t buf[ADC_GRP_NUM_CHANNELS];

	// sample the sensors
	msg_t err = adcConvert(&ADCD1, &adcgrpcfg, &buf[0], 1);
	if (err != RDY_OK) {
		return err;
	}

	// Position (in radians) = atan2(psin, pcos)
	pos[0] = buf[1] - buf[0];
	pos[1] = buf[2] - buf[0];

	return RDY_OK;
}

float motor_lld_calc_pos(float psin, float pcos) {
	// calculate sine based on 12-bit sampling
	psin = psin * 2 * M_PI / 2048.0f;
	// calculate sine based on 12-bit sampling
	pcos = pcos * 2 * M_PI / 2048.0f;
	// calculate radians
	float pos = atan2f(psin, pcos);
	// ensure position is bounded
	return fmod(pos + 2 * M_PI, 2 * M_PI);
}

void motor_lld_sample_calc(float *pos, const size_t isin, const size_t icos) {
	motor_lld_sample_pos(pos);
	pos[2] = motor_lld_calc_pos(pos[isin], pos[icos]);
}

void motorCalibrate(int8_t pwm) {
	size_t isin = 0;
	size_t icos = 1;
	const size_t ipos = 2;
	
	// reset state
	MD1.pwmstate = 0;
	MD1.flags &= ~MOTOR_INVERSE;

// 1st Run

	float startPos[3];
	float prevPos[3];
	float nextPos[3];

	int i = 0;

	// send motor to starting position
	motorSet(-pwm);

	// sample until stopped
	chThdSleepMilliseconds(100);
	motor_lld_sample_calc(nextPos, isin, icos);
	for (i = 0; i < 10; i++) {
		prevPos[ipos] = nextPos[ipos];

		chThdSleepMilliseconds(100);
		motor_lld_sample_calc(nextPos, isin, icos);

		float absDelta = fabs(nextPos[ipos] - prevPos[ipos]);
		if (absDelta > 1.0) {
			// this is fine, just wrapping around
		} else if (absDelta < 0.005) {
			// no movement, must be done
			break;
		}
	}

	// starting position
	startPos[0] = nextPos[0];
	startPos[1] = nextPos[1];
	startPos[2] = nextPos[2];

	// send motor towards ending position
	motorSet(pwm);

	// sample next position
	chThdSleepMilliseconds(100);
	motor_lld_sample_calc(nextPos, isin, icos);

	// determine direction
	bool increasing = startPos[ipos] > nextPos[ipos];
	bool inversed = false;

	// sample until stopped
	for (i = 0; i < 10; i++) {
		prevPos[0] = nextPos[0];
		prevPos[1] = nextPos[1];
		prevPos[2] = nextPos[2];

		chThdSleepMilliseconds(100);
		motor_lld_sample_calc(nextPos, isin, icos);

		float absDelta = fabs(nextPos[ipos] - prevPos[ipos]);

		// detect incongruence
		if (absDelta > 1.0) {
			// this is fine, just wrapping around
		} else if (absDelta < 0.005) {
			// no movement, must be done
			break;
		} else if (increasing ^ (nextPos[ipos] > prevPos[ipos])) {
			// swapping sine and cosine could yield cleaner values
			inversed = true;
			isin = 1;
			icos = 0;
			// recalculate positions
			startPos[ipos] = motor_lld_calc_pos(startPos[isin], startPos[icos]);
			prevPos[ipos] = motor_lld_calc_pos(prevPos[isin], prevPos[icos]);
			nextPos[ipos] = motor_lld_calc_pos(nextPos[isin], nextPos[icos]);
		}
	}

	// disable motor
	motorSet(0);
	chThdSleep(500);

// 2nd Run, opposite direction

	float startPos1[3];
	float prevPos1[3];
	float nextPos1[3];

	// send motor to starting position
	motorSet(pwm);

	// sample until stopped
	chThdSleepMilliseconds(100);
	motor_lld_sample_calc(nextPos1, isin, icos);
	for (i = 0; i < 10; i++) {
		prevPos1[ipos] = nextPos1[ipos];

		chThdSleepMilliseconds(100);
		motor_lld_sample_calc(nextPos1, isin, icos);

		float absDelta = fabs(nextPos1[ipos] - prevPos1[ipos]);
		if (absDelta > 1.0) {
			// this is fine, just wrapping around
		} else if (absDelta < 0.005) {
			// no movement, must be done
			break;
		}
	}

	// starting position
	startPos1[0] = nextPos1[0];
	startPos1[1] = nextPos1[1];
	startPos1[2] = nextPos1[2];

	// send motor towards ending position
	motorSet(-pwm);

	// sample next position
	chThdSleepMilliseconds(100);
	motor_lld_sample_calc(nextPos1, isin, icos);

	// determine direction
	bool increasing1 = startPos1[ipos] > nextPos1[ipos];

	// sample until stopped
	for (i = 0; i < 10; i++) {
		prevPos1[0] = nextPos1[0];
		prevPos1[1] = nextPos1[1];
		prevPos1[2] = nextPos1[2];

		chThdSleepMilliseconds(100);
		motor_lld_sample_calc(nextPos1, isin, icos);

		float absDelta = fabs(nextPos1[ipos] - prevPos1[ipos]);

		// detect incongruence
		if (absDelta > 1.0) {
			// this is fine, just wrapping around
		} else if (absDelta < 0.005) {
			// no movement, must be done
			break;
		} else if (increasing1 ^ (nextPos1[ipos] > prevPos1[ipos])) {
			// swapping sine and cosine could yield cleaner values
			isin = 1;
			icos = 0;
			// recalculate positions
			startPos1[ipos] = motor_lld_calc_pos(startPos1[isin], startPos1[icos]);
			prevPos1[ipos] = motor_lld_calc_pos(prevPos1[isin], prevPos1[icos]);
			nextPos1[ipos] = motor_lld_calc_pos(nextPos1[isin], nextPos1[icos]);
		}
	}

	// disable motor
	motorSet(0);

// Set parameters

	if (inversed) {
		MD1.flags |= MOTOR_INVERSE;
	}

	if (increasing) {
		MD1.offset = (startPos[ipos]+nextPos1[ipos])/2;
		MD1.hibound = (nextPos[ipos]+startPos1[ipos])/2;
	} else {
		MD1.offset = (nextPos[ipos]+startPos1[ipos])/2;
		MD1.hibound = (startPos[ipos]+nextPos1[ipos])/2;
	}

	MD1.hibound = fmod(MD1.hibound - MD1.offset + 2 * M_PI, 2 * M_PI);
}

void motorSet(int8_t p) {
	chSysLock();
	motorSetI(p);
	chSysUnlock();
}

void motorSetI(int8_t p) {
	// input is restricted to [-128, 127] by data type
	if (p == -128) {
		p = -127;
	}

	if (MD1.pwmstate == p) {

		// no-op

	} else if (p == 0) {

		// disable motors
		palClearPad(GPIOB, GPIOB_MOTOR_EN);
		pwmDisableChannelI(&PWMD1, 0);
		pwmDisableChannelI(&PWMD1, 1);

	} else {

		// new direction when signs don't match
		bool newdir = (MD1.pwmstate == 0) || ((MD1.pwmstate < 0) ^ (p < 0));

		// update PWM output
		if (p > 0) {
			pwmEnableChannelI(&PWMD1, 0, MD1.pwmoffset + p);
			if (newdir) {
				pwmDisableChannelI(&PWMD1, 1);
			}
		} else {
			if (newdir) {
				pwmDisableChannelI(&PWMD1, 0);
			}
			pwmEnableChannelI(&PWMD1, 1, MD1.pwmoffset - p);
		}

		// start motors
		if (MD1.pwmstate == 0) {
			palSetPad(GPIOB, GPIOB_MOTOR_EN);
		}

	}

	// update state
	MD1.pwmstate = p;
}

float motorPosition(void) {
	float pos[2];

	msg_t err = motor_lld_sample_pos(pos);
	if (err != RDY_OK) {
		return 0;
	}

	if (MD1.flags & MOTOR_INVERSE) {
		return motor_lld_calc_pos(pos[1], pos[0]);
	}

	return motor_lld_calc_pos(pos[0], pos[1]);
}

float motorCPosition(void) {
	float pos = motorPosition();
	float cpos = fmod(pos - MD1.offset + 2 * M_PI, 2 * M_PI);

	if (cpos < 0.0) {
		MD1.offset = cpos;
		MD1.hibound = fmod(MD1.hibound - MD1.offset + 2 * M_PI, 2 * M_PI);
	} else if (cpos > MD1.hibound) {
		MD1.hibound = cpos;
	}

	if (cpos < 0.0 || cpos > MD1.hibound) {
		MD1.hibound = fmod(MD1.hibound - MD1.offset + 2 * M_PI, 2 * M_PI);
		cpos = fmod(pos - MD1.offset + 2 * M_PI, 2 * M_PI);
	}

	return cpos;
}
