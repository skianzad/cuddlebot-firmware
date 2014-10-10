/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#define _USE_MATH_DEFINES
#include <math.h>

#include <ch.h>
#include <hal.h>

#include "sensor.h"

// Number of channels to be sampled for ADC1.
#define ADC_GRP_NUM_CHANNELS 6

/*

ADC conversion group.

Mode:     linear buffer, 1 sample of 7 channels, SW triggered.

Timing:   15 cycles sample time
          15 cycles conversion time
          30 total cycles

          @ 21 Mhz ADC clock

          ~0.71 µs sample time
          ~0.71 µs conversion time
          ~1.42 µs total time

Timing:   480 cycles sample time
          15 cycles conversion time
          495 total cycles

          @ 21 Mhz ADC clock

          ~22.9 µs sample time
          ~0.71 µs conversion time
          ~23.6 µs total time

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
	  ADC_SMPR1_SMP_SENSOR(ADC_SAMPLE_480) |  // internal temperature, min 10 µs
	  ADC_SMPR1_SMP_AN13(ADC_SAMPLE_15) |     // motor pos cos
	  ADC_SMPR1_SMP_AN12(ADC_SAMPLE_15) |     // motor pos sin
	  ADC_SMPR1_SMP_AN11(ADC_SAMPLE_15) |     // motor pos temperature
	  ADC_SMPR1_SMP_AN10(ADC_SAMPLE_15)),     // torque
	.smpr2 = (
	  ADC_SMPR2_SMP_AN9(ADC_SAMPLE_15) |      // vref 1V65
	  ADC_SMPR2_SMP_AN8(ADC_SAMPLE_15)),      // current
	.sqr1 = ADC_SQR1_NUM_CH(ADC_GRP_NUM_CHANNELS),
	.sqr2 = 0,
	.sqr3 = (
	  ADC_SQR3_SQ5_N(ADC_CHANNEL_IN9) |       // vref 1V65
	  ADC_SQR3_SQ4_N(ADC_CHANNEL_IN8) |       // current
	  ADC_SQR3_SQ3_N(ADC_CHANNEL_IN10) |      // torque
	  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN13) |      // pos sin
	  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN12))       // pos cos
};

// idiomatic access to sample buffer
typedef struct {
	adcsample_t pos_cos;
	adcsample_t pos_sin;
	adcsample_t torque;
	adcsample_t current;
	adcsample_t vref;
} sensor_sample_vitals_t;

msg_t sensorConvert(sensor_vitals_t *vitals) {
	// sample buffer
	static adcsample_t buf[ADC_GRP_NUM_CHANNELS];
	// idiomatic access to sample data
	sensor_sample_vitals_t *samples = (sensor_sample_vitals_t *)buf;

	// enable temperature compensation
	palSetPad(GPIOB, GPIOB_POS_TCCEN);

	// enable position sensor
	palClearPad(GPIOB, GPIOB_POS_NEN);

	// enable internal temperature sensor and reference voltage
	adcSTM32EnableTSVREFE();
	// start the adc peripherable
	adcStart(&ADCD1, NULL);
	// sample the sensors
	msg_t err = adcConvert(&ADCD1, &adcgrpcfg, (adcsample_t *)&buf, 1);
	// disable internal temperature sensor and reference voltage
	adcSTM32DisableTSVREFE();

	// disable position sensor
	palSetPad(GPIOB, GPIOB_POS_NEN);

	// return on error
	if (err != RDY_OK) {
		return err;
	}

	/*

	Calculate the temperature using the following formula:

	  Temperature (in °C) = {(VSENSE – V25) / Avg_Slope} + 25

	Where:

	– V25 = VSENSE value for 25° C
	– Avg_Slope = average slope of the temperature vs. VSENSE curve
	  (given in mV/°C or μV/°C)

	Refer to the datasheet's electrical characteristics section for the
	actual values of V25 and Avg_Slope.

	Temperature sensor characteristics:

	- V25 = 0.76 V
	- Avg_Slope = 2.5 mV/°C

	*/
	// // calculate voltage based on 12-bit sampling 0V to 3V3 range
	// int32_t temp = (samples->temperature * 33000) / 4096;
	// // apply formula to convert to °C multiplicative factor 10000
	// temp = ((temp - 7600) / 25) + 250000;
	// // scale back to units of °C
	// vitals->temperature = temp / 10000; // ±1°C

	/*

	Temperature ±35 (in °C)
	  = -240 + 1090 * VSENSE - 105 * VSENSE * VSENSE
	  = -240 + (1090 - 105 * VSENSE) * VSENSE

	*/
	// // calculate voltage based on 12-bit sampling 0V to 3V3 range
	// temp = samples->pos_temperature * 33000 / 4096;
	// // apply quadratic interpolation
	// temp = -240 + (1090 - 105 * temp) * temp;
	// // scale back to units of °C
	// vitals->external_temperature = temp / 10000;

	/*

	Position (in radians) = atan2(pos_sin, pos_cos)

	Peak-to-peak voltage for sine and cosine are 0.56 * VCC typical and
	we adjust output to compensate: pos_adj = pos_cos * (0.5/0.56).

	*/
	// calculate sine based on 12-bit sampling
	float psin = ((float)(samples->pos_sin - samples->vref)) * M_2_PI * 0.893f / 2048.0f;
	// calculate sine based on 12-bit sampling
	float pcos = ((float)(samples->pos_cos - samples->vref)) * M_2_PI * 0.893f / 2048.0f;
	// calculate radians
	float prad = atan2f(psin, pcos);
	// bound to between -π and π and convert to signed 16-bit value
	vitals->position = (int16_t)(((fmod(prad, M_PI) + M_PI) / M_PI) * 0xffff);

	// torque and current values from -32768 to +32767
	vitals->torque = samples->torque - samples->vref;
	vitals->current = samples->current - samples->vref;

	// save raw values
	vitals->pcos = samples->pos_cos;
	vitals->psin = samples->pos_sin;
	vitals->vref = samples->vref;

	return RDY_OK;
}
