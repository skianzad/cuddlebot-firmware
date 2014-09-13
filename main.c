/*

Fabric pressure sensor - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <ch.h>
#include <hal.h>
#include <chprintf.h>

#include "sensor.h"
#include "usb_serial.h"

// Virtual serial port over USB driver.
extern SerialUSBDriver SDU1;

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
		// start sampling timer
		cm_sensor_start();
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
		cm_sensor_stop();
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
	cm_usb_serial_init();
	// initialize the sensor
	cm_sensor_init((BaseSequentialStream *)&SDU1);

	// execute state machine
	for (;;) {
		sample_on_connection();
	}

	return 0;
}
