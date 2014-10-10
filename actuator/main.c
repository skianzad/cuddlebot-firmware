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

#include <ch.h>
#include <hal.h>
#include <chprintf.h>

#include "addr.h"
#include "comm.h"
#include "motor.h"
#include "sensor.h"

/* Communications driver. */
CommDriver CD1 = {
	.sd = &SD3,
	.acb = addrIsSelf,
	.scb = NULL,
	.enport = GPIOB,
	.enpad = GPIOB_RS485_TXEN,
	.prio = LOWPRIO,
	.timeout = MS2ST(1)
};

/* Motor driver configuration. */
MotorDriver MD1 = {
	.pwm = &PWMD1,
	.enport = GPIOB,
	.enpad = GPIOB_MOTOR_EN,
	.offset = 0,
	.pwmstate = 0
};

/* PWM configuration for Maxon motors. */
PWMConfig MaxonPWMConfig = {
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

/* PWM configuration for Purr motors. */
PWMConfig PurrPWMConfig = {
	.frequency = 207 * 202898,                //  42.0 MHz; divider = 2
	.period = 207,                            // 137.3 KHz
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

	// start serial driver
	commStart(&CD1);

	// start motor
	if (addrGet() == ADDR_PURR) {
		motorStart(&MD1, &PurrPWMConfig);
	} else {
		motorStart(&MD1, &MaxonPWMConfig);
	}

	int i = 0;
	int v = 0;
	BaseSequentialStream *bss = (BaseSequentialStream *)CD1.sd;

	for (;;) {

		// send address
		if (i-- <= 0) {
			i = 10;

			if (addrGet() == ADDR_PURR) {
				motorSet(&MD1, v);
				// if (++v < 0) v = 0;
			} else {
				motorSet(&MD1, v);
				v = -v;
			}

			switch (addrGet()) {
			case ADDR_RIBS:
				chprintf(bss, "I am the ribs motor!\r\n");
				break;
			case ADDR_HEAD_PITCH:
				chprintf(bss, "I am the head pitch motor!\r\n");
				break;
			case ADDR_HEAD_YAW:
				chprintf(bss, "I am the head yaw motor!\r\n");
				break;
			case ADDR_SPINE:
				chprintf(bss, "I am the spine motor!\r\n");
				break;
			case ADDR_PURR:
				chprintf(bss, "I am the purr motor!\r\n");
				break;
			default:
				chprintf(bss, "I have no address!\r\n");
			}
		}

		sensor_vitals_t vitals;
		sensorReadVitals(&vitals);

		chprintf(bss, "position = %d\r\n", vitals.position);
		chprintf(bss, "pcos = %d\r\n", vitals.pcos);
		chprintf(bss, "psin = %d\r\n", vitals.psin);
		chprintf(bss, "torque = %d\r\n", vitals.torque);
		chprintf(bss, "current = %d\r\n", vitals.current);
		chprintf(bss, "vref = %d\r\n", vitals.vref);

		// sleep
		chThdSleepMilliseconds(100);
	}

	return 0;
}
