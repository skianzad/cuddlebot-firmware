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
	  ADC_SQR3_SQ5_N(ADC_CHANNEL_IN9) |       // vref 1V65
	  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN12) |      // pos sin
	  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN13))       // pos cos
};

void motorStart(MotorDriver *md, PWMConfig *pwmcfg) {
	// reset state
	md->pwmoffset = pwmcfg->period - 127;
	md->pwmstate = 0;
	// disable the motor driver
	palClearPad(md->enport, md->enpad);
	// start the pwm peripherable
	pwmStart(md->pwm, pwmcfg);
}

void motorStop(MotorDriver *md) {
	// disable the motor driver
	palClearPad(md->enport, md->enpad);
	// stop the pwm peripherable
	pwmStop(md->pwm);
}

void motorSet(MotorDriver *md, int8_t p) {
	// input is restricted to [-128, 127] by data type
	if (p == -128) {
		p = -127;
	}

	if (md->pwmstate == p) {

		// no-op

	} else if (p == 0) {

		// disable motors
		palClearPad(md->enport, md->enpad);

	} else {
		// start motors
		if (md->pwmstate == 0) {
			palSetPad(md->enport, md->enpad);
		}

		// new direction when signs don't match
		bool newdir = (md->pwmstate == 0) || ((md->pwmstate < 0) ^ (p < 0));

		// update forces
		if (p > 0) {
			pwmEnableChannel(md->pwm, 0, md->pwmoffset + p);
			if (newdir) {
				pwmDisableChannel(md->pwm, 1);
			}
		} else {
			if (newdir) {
				pwmDisableChannel(md->pwm, 0);
			}
			pwmEnableChannel(md->pwm, 1, md->pwmoffset - p);
		}
	}

	// update state
	md->pwmstate = p;
}

msg_t motorPosition(MotorDriver *md, uint16_t *p) {
	(void)md;

	// sample buffer
	static adcsample_t buf[ADC_GRP_NUM_CHANNELS];

	// enable temperature compensation
	palSetPad(GPIOB, GPIOB_POS_TCCEN);

	// enable position sensor
	palClearPad(GPIOB, GPIOB_POS_NEN);

	// start the adc peripherable
	adcStart(&ADCD1, NULL);
	// sample the sensors
	msg_t err = adcConvert(&ADCD1, &adcgrpcfg, (adcsample_t *)&buf, 1);

	// disable position sensor
	palSetPad(GPIOB, GPIOB_POS_NEN);

	// return on error
	if (err != RDY_OK) {
		return err;
	}

	// Position (in radians) = atan2(pcos, psin)
	//
	// Peak-to-peak voltage for sine and cosine are 0.56 * VCC typical
	// and we adjust output to compensate: pos_adj = pos_cos *
	// (0.5/0.56).
	float pcos = buf[0] - buf[2];
	float psin = buf[1] - buf[2];
	// calculate sine based on 12-bit sampling
	pcos = pcos * M_2_PI * 0.893f / 2048.0f;
	// calculate sine based on 12-bit sampling
	psin = psin * M_2_PI * 0.893f / 2048.0f;
	// calculate radians
	float prad = atan2f(pcos, psin);
	// bound to between -π and π and save to output
	*p = ((fmod(prad, M_PI) / M_PI) + 1.0) * 0xffff;

	return RDY_OK;
}
