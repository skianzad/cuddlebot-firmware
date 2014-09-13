/*

Fabric pressure sensor - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#ifndef _SENSOR_H_
#define _SENSOR_H_

#include <ch.h>
#include <hal.h>

/*

Initialize sensor.

1. Initialize the timer.
2. Start the sampling thread.

@param chp BaseSequentialStream to write sample data

*/
void cm_sensor_init(BaseSequentialStream *chp);

/*

Start the sampling timer.

Does nothing if the sampling thread is still running.

*/
void cm_sensor_start(void);

/*

Stop the sampling timer.

Does nothing if the sampling thread is not running.

*/
void cm_sensor_stop(void);

#endif // _SENSOR_H_
