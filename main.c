/*

Fabric pressure sensor - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

----

ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include <ch.h>
#include <hal.h>
#include <chprintf.h>

#include "usbcfg.h"
#include "sensor.h"

// Virtual serial port over USB driver.
SerialUSBDriver SDU1;

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

// Stack space for sensor sampling thread.
static WORKING_AREA(wa_sampling_thread, 128);

// Sensor sampling thread.
static msg_t sampling_thread(void *arg) {
	BaseSequentialStream *chp = (BaseSequentialStream *)arg;

	while (!chThdShouldTerminate()) {
		// wait for trigger
		if (chBSemWait(&gpt_trigger) != RDY_OK) {
			continue;
		}

		// get next buffer
		static sensor_sample_t buf;

		// sample grid
		sample_grid(&buf);

		// send data
		chSequentialStreamWrite(chp, (uint8_t *)&buf, sizeof(buf));
	}

	return RDY_OK;
}

// Sampling timer state machine states.
typedef enum {
	SERWRITE_UNINIT,
	SERWRITE_STOP,
	SERWRITE_READY,
	SERWRITE_ACTIVE
} serwriter_state_t;

/*

Activate the sampling timer when the USB is connected.

The sampling timer manager uses a state machine to start and stop the
sampling timer on USB connect and disconnect, respectively. The
function is not thread-safe, as it maintains an internal state.

*/
void sample_on_connection(void) {
	static serwriter_state_t state = SERWRITE_UNINIT;

	switch (state) {
	case SERWRITE_UNINIT:

		// check USB is connected every 500ms
		if (SDU1.config->usbp->state == USB_ACTIVE) {
			state = SERWRITE_READY;
		} else {
			chThdSleepMilliseconds(500);
		}
		break;

	case SERWRITE_READY:
		// start sampling timer at 1000 / 10 = 100 Hz
		gptStartContinuous(&GPTD9, 10);
		// next state active
		state = SERWRITE_ACTIVE;
		break;

	case SERWRITE_ACTIVE:
		// check USB is connected every 500ms
		if (SDU1.config->usbp->state == USB_ACTIVE) {
			chThdSleepMilliseconds(500);
		} else {
			state = SERWRITE_STOP;
		}
		break;

	case SERWRITE_STOP:
		// stop sampling timer
		gptStopTimer(&GPTD9);
		// next state uninit
		state = SERWRITE_UNINIT;
		break;

	default:
		state = SERWRITE_UNINIT;
	}
}

/*

Application entry point.

*/
int main(void) {

	// initialize the system
	// - HAL: initialize the configured device drivers and perform
	//   board-specific initializations
	// - Kernel: the main() function becomes a thread and the RTOS is
	//   active
	halInit();
	chSysInit();

	// initialize the serial-over-USB driver
	sduObjectInit(&SDU1);
	sduStart(&SDU1, &serusbcfg);

	// Activate the USB driver and then the USB bus pull-up on D+. Note,
	// a delay is inserted in order to not have to disconnect the cable
	// after a reset.
	usbDisconnectBus(serusbcfg.usbp);
	chThdSleepMilliseconds(1000);
	usbStart(serusbcfg.usbp, &usbcfg);
	usbConnectBus(serusbcfg.usbp);

	// initialize general purpose timer driver
	gptStart(&GPTD9, &gptcfg);

	// start sensor sampling thread
	Thread *tp_sampling_thread = chThdCreateStatic(
	                               wa_sampling_thread, sizeof(wa_sampling_thread),
	                               HIGHPRIO, sampling_thread, &SDU1);

	// execute state machine
	for (;;) {
		sample_on_connection();
	}

	return 0;
}
