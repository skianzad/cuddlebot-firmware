/*

Fabric pressure sensor - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <ch.h>
#include <hal.h>

#include "sensor.h"

// Number of channels to be sampled for ADC1.
#define ADC_GRP_NUM_CHANNELS 1

/*

ADC conversion group.

Mode:     linear buffer, 1 sample of 1 channel, SW triggered.

Timing:   15 cycles sample time
          15 cycles conversion time
          30 total cycles
          ~1.43 µs total time @ 21 Mhz ADC clock

*/
const ADCConversionGroup adcgrpcfg = {
	.circular = FALSE,                        // linear buffer
	.num_channels = ADC_GRP_NUM_CHANNELS,     // channel 1
	.end_cb = NULL,
	.error_cb = NULL,
	// hardware-specific configuration
	.cr1 = 0,
	.cr2 = ADC_CR2_SWSTART,
	.smpr1 = 0,
	.smpr2 = ADC_SMPR2_SMP_AN1(ADC_SAMPLE_15),
	.sqr1 = ADC_SQR1_NUM_CH(ADC_GRP_NUM_CHANNELS),
	.sqr2 = 0,
	.sqr3 = ADC_SQR3_SQ1_N(ADC_CHANNEL_IN1)
};

/*

Sample the pressure grid.

1. Set the appropriate pin modes.
2. Enables the ADC.
3. Samples the pressure sensor grid.
4. Disables the ADC.
5. Sends the data using the serial driver.

*/
void sample_grid(sensor_sample_t *buf) {
	// set PA1 pin to analog input
	palSetPadMode(GPIOA, 1, PAL_MODE_INPUT_ANALOG);
	// set PD0-6 pins to push-pull output
	palSetGroupMode(GPIOD, 0x7F, 0, PAL_MODE_OUTPUT_PUSHPULL);

	// save current time, truncating to 32-bits
	// use formula to convert to milliseconds:
	//    msec = (ticks * 1000) / CH_FREQUENCY
	// where CH_FREQUENCY is defined in chconf.h
	buf->time = (chTimeNow() * 1000) / CH_FREQUENCY;

	// reset checksum
	buf->checksum = buf->time;

	// enable ADC
	adcStart(&ADCD1, NULL);

	// sample sensors
	uint8_t x, y;
	for (x = 0; x < GROUND_SIZE; x++) {

		// configure grounding mux
		palWriteGroup(GPIOD, 0x07, 0, x);

		for (y = 0; y < POWER_SIZE; y++) {
			// configure power mux
			palWriteGroup(GPIOD, 0x07, 3, y);

			// sample voltage
			adcConvert(&ADCD1, &adcgrpcfg, &buf->values[x][y], 1);

			// update checksum
			buf->checksum += buf->values[x][y];
		}
	}

	// disable ADC
	adcStop(&ADCD1);

	// set PA1 pin to high-z
	palSetPadMode(GPIOA, 1, PAL_MODE_INPUT);
	// set PD0-6 pins to high-z
	palSetGroupMode(GPIOD, 0x7F, 0, PAL_MODE_INPUT);
}
