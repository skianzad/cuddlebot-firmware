/*

Cuddlemaster - Copyright (C) 2014 Michael Phan-Ba

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

#include "address.h"
#include "comm.h"
#include "motor.h"

// Application entry point.
int main(void) {
	// initialize the system
	// - HAL: initialize the configured device drivers and perform
	//   board-specific initializations
	// - Kernel: the main() function becomes a thread and the RTOS is
	//   active
	halInit();
	chSysInit();

	// read board address
	cm_address_read();
	// initialize communications
	cm_comm_init();
	// initialize motor
	// cm_motor_init();

	// enable LED1 and LED2
	palSetPadMode(GPIOB, GPIOB_LED1, PAL_MODE_OUTPUT_PUSHPULL);
	palSetPadMode(GPIOB, GPIOB_LED2, PAL_MODE_OUTPUT_PUSHPULL);
	// toggle LED1
	palTogglePad(GPIOB, GPIOB_LED1);

	// enable RS-485 transmitter
	palSetPad(GPIOB, GPIOB_RS485_TXEN);

	for (;;) {
		// toggle LED1 and LED2
		palTogglePad(GPIOB, GPIOB_LED1);
		palTogglePad(GPIOB, GPIOB_LED2);

		// enable RS-485 transmitter
		// palSetPad(GPIOB, GPIOB_RS485_TXEN);

		// write hello world
		const uint8_t *helloworld = (uint8_t *)("Hello World!");
		uartStopSend(&UARTD3);
		uartStartSend(&UARTD3, sizeof(helloworld), helloworld);

		// disable RS-485 transmitter
		// palClearPad(GPIOB, GPIOB_RS485_TXEN);

		// sleep
		chThdSleepMilliseconds(500);
	}

	return RDY_OK;
}
