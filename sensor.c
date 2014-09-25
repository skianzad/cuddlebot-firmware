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

#define GROUND_SIZE   8
#define POWER_SIZE    8

// Sampling timer semaphore for triggering an ADC sample.
static BSEMAPHORE_DECL(gpt_trigger, FALSE);

/*

Signal the sampling thread to take a sample and send the data.

The separation between timer callback and the sampling thread is
needed because this function runs as an interrupt routine. While in
the interrupt routine, the ADC is blocked.

*/
static void gpt_callback(GPTDriver *gptp) {
	(void)gptp;
	chBSemSignalI(&gpt_trigger);
}

// General Purpose Timer configuration.
static const GPTConfig gptcfg = {
	.frequency = 1000,
	.callback = gpt_callback,
	// hardware-specfic configuration
	.dier = 0
};

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
#define ADC_GRP_NUM_CHANNELS 1
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

// Sensor sample and message.
typedef struct {
	uint32_t time;
	uint16_t values[POWER_SIZE][GROUND_SIZE];
	uint8_t checksum;
} sensor_sample_t;

/*

Sample the pressure grid.

1. Set the appropriate pin modes.
2. Enable the ADC.
3. Sample the pressure sensor grid.
4. Disable the ADC.

*/
void sample_grid(sensor_sample_t *buf) {
	// set PA1 pin to analog input
	palSetPadMode(GPIOA, 1, PAL_MODE_INPUT_ANALOG);
	// set PD0-6 pins to push-pull output
	palSetGroupMode(GPIOD, 0x3F, 0, PAL_MODE_OUTPUT_PUSHPULL);

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
	for (x = 0; x < POWER_SIZE; x++) {

		// configure power mux
		palWriteGroup(GPIOD, 0x07, 0, x);

		for (y = 0; y < GROUND_SIZE; y++) {
			// configure ground mux
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
	palSetGroupMode(GPIOD, 0x3F, 0, PAL_MODE_INPUT);
}

// Stack space for sensor sampling thread.
static WORKING_AREA(sampling_thread_wa, 128);

// Pointer to sampling thread.
static Thread *sampling_thread_tp;

/*

Sensor sampling thread.

1. Wait for timer signal.
2. Sample grid.
3. Send the data using the serial driver.

*/
static msg_t sampling_thread(void *arg) {
	BaseSequentialStream *chp = (BaseSequentialStream *)arg;

	while (!chThdShouldTerminate()) {
		// wait for trigger
		if (chBSemWait(&gpt_trigger) != RDY_OK) {
			continue;
		}

		// sample grid
		static sensor_sample_t buf;
		sample_grid(&buf);

		// send data
		const int32_t header = -1;
		chSequentialStreamWrite(chp, (uint8_t *)&header, sizeof(header));
		chSequentialStreamWrite(chp, (uint8_t *)&buf, sizeof(buf));
	}

	return RDY_OK;
}

/*

Initialize sensor.

1. Initialize the timer.
2. Start the sampling thread.

*/
void cm_sensor_init() {
	// initialize general purpose timer driver
	gptStart(&GPTD9, &gptcfg);
}

/*

Start the sampling timer.

Does nothing if the thread is still running.

@param chp BaseSequentialStream to write sample data

*/
void cm_sensor_start(BaseSequentialStream *chp) {
	if (sampling_thread_tp) {
		return;
	}

	// start sensor sampling thread
	sampling_thread_tp = chThdCreateStatic(
	                       sampling_thread_wa, sizeof(sampling_thread_wa),
	                       HIGHPRIO, sampling_thread,
	                       (void *)chp);

	// 1000 / 10 = 100 Hz
	gptStartContinuous(&GPTD9, 10);
}

/*

Stop the sampling timer.

Does nothing if the thread is not running.

*/
void cm_sensor_stop(void) {
	if (!sampling_thread_tp) {
		return;
	}

	if (chThdTerminated(sampling_thread_tp)) {
		sampling_thread_tp = NULL;
	} else {
		gptStopTimer(&GPTD9);
		chThdTerminate(sampling_thread_tp);
	}
}
