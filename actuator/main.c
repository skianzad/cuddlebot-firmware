/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

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

#include <math.h>

#include <ch.h>
#include <hal.h>
#include <chprintf.h>

#include "addr.h"
#include "comm.h"
#include "motor.h"
#include "pid.h"
#include "service.h"

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
	addrLoad();

	// halt system if address is invalid
	if (addrGet() == ADDR_INVALID) {
		chSysHalt();
	}

	// start motor
	motorStart();

	// start serial driver
	commStart(serviceHandler);

	// start PID driver
	PIDDriver PID1;
	pidStart(&PID1);

	const uint32_t frequency = 1000;
	PID1.kp = 100.0f;
	PID1.ki = 1.0f / frequency;
	PID1.kd = 1000.0f / frequency;
	PID1.setpoint = 2.5f;

	for (;;) {
		// chprintf((BaseSequentialStream *)&SD3, "I am %c\r\n", addrGet());
		chThdSleepMilliseconds(1);
		float p = motorCGet();

		// for debugging; remember to reduce frequency
		// chprintf((BaseSequentialStream *)&SD3, "%d.%03d\r\n",
		//          (int)(p),
		//          (int)(1000 * fmod(copysign(p, 1.0), 1.0)));

		motorSet(pidUpdate(&PID1, p));
	}

	return 0;
}
