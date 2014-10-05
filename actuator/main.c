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
#include <chprintf.h>

#include "address.h"
#include "motor.h"
#include "rs485.h"

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

	// start rs-485 serial driver
	rsdStart();

	// initialize motor
	// cm_motor_init();

	for (;;) {
		chprintf((BaseSequentialStream *)&SD3, "Hello World!\r\n");

		// send address
		switch (cm_address) {
		case ADDRESS_RIBS:
			chprintf((BaseSequentialStream *)&SD3, "I am the ribs motor!\r\n");
			break;
		case ADDRESS_HEAD_PITCH:
			chprintf((BaseSequentialStream *)&SD3, "I am the head pitch motor!\r\n");
			break;
		case ADDRESS_HEAD_YAW:
			chprintf((BaseSequentialStream *)&SD3, "I am the head yaw motor!\r\n");
			break;
		case ADDRESS_SPINE:
			chprintf((BaseSequentialStream *)&SD3, "I am the spine motor!\r\n");
			break;
		case ADDRESS_PURR:
			chprintf((BaseSequentialStream *)&SD3, "I am the purr motor!\r\n");
			break;
		default:
			chprintf((BaseSequentialStream *)&SD3, "I have no address!\r\n");
		}

		// sleep
		chThdSleepMilliseconds(500);
	}

	return RDY_OK;
}
