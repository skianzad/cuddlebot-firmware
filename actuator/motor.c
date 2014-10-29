/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

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

// idiomatic access to sample buffer
typedef struct {
	adcsample_t pos_cos;
	adcsample_t pos_sin;
	adcsample_t vref;
} motor_lld_sample_t;

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
	if (addrGet() == ADDR_PURR) {
		pwmcfg.frequency = 207 * 202898;    //  42.0 MHz; divider = 2
		pwmcfg.period = 207;                // 137.3 KHz
	}

	// reset state
	MD1.pwmoffset = pwmcfg.period - 127;
	MD1.pwmstate = 0;

	// set motor direction based on position on board
	switch (addrGet()) {
	case ADDR_SPINE:
		MD1.dir = -1;
		break;
	default:
		MD1.dir = 1;
	}
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

void motorCalibrate(void) {
	// send motor to starting position
	motorSet(-40);
	chThdSleepSeconds(1);
	// save lower bound
	MD1.lobound = motorGet();

	// send motor the other way
	motorSet(40);
	chThdSleepSeconds(1);
	// save higher bound
	MD1.hibound = motorGet();

	// disable motor
	motorSet(0);
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

	// set direction
	if (MD1.dir < 0) {
		p = -p;
	}

	if (MD1.pwmstate == p) {

		// no-op

	} else if (p == 0) {

		// disable motors
		palClearPad(GPIOB, GPIOB_MOTOR_EN);

	} else {
		// start motors
		if (MD1.pwmstate == 0) {
			palSetPad(GPIOB, GPIOB_MOTOR_EN);
		}

		// new direction when signs don't match
		bool newdir = (MD1.pwmstate == 0) || ((MD1.pwmstate < 0) ^ (p < 0));

		// update forces
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
	}

	// update state
	MD1.pwmstate = p;
}

float motorGet(void) {
	// sample buffer
	static adcsample_t buf[ADC_GRP_NUM_CHANNELS];

	// start the adc peripherable
	adcStart(&ADCD1, NULL);
	// sample the sensors
	msg_t err = adcConvert(&ADCD1, &adcgrpcfg, &buf[0], 1);

	// return on error
	if (err != RDY_OK) {
		return 0;
	}

	// Position (in radians) = atan2(psin, pcos)
	float psin = buf[1] - buf[0];
	float pcos = buf[2] - buf[0];
	// calculate sine based on 12-bit sampling
	psin = psin * M_2_PI / 2048.0f;
	// calculate sine based on 12-bit sampling
	pcos = pcos * M_2_PI / 2048.0f;
	// calculate radians
	float pos = atan2f(psin, pcos);

	// flip axis when sensor is mounted upside-down relative to arm
	switch (addrGet()) {
	case ADDR_RIBS:
	case ADDR_SPINE:
	case ADDR_HEAD_PITCH:
		if (pos < 0) {
			pos = -M_PI - pos;
		} else {
			pos = M_PI - pos;
		}
		break;
	default:
		break;
	}

	return pos;
}

float motorCGet(void) {
	return motorGet() - MD1.lobound;
}
