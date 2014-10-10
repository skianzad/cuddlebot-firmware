/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#ifndef _SENSOR_H_
#define _SENSOR_H_

#include <ch.h>
#include <hal.h>

// vital sampling results
typedef struct {
  uint16_t position;
  int16_t torque;
  int16_t current;
  // raw values
  uint16_t pcos;
  uint16_t psin;
  uint16_t vref;
} sensor_vitals_t;

/*

Sample vitals sensors.

Sensors sampled by this function:
- internal temperature
- KMZ60 temperature sensor
- KMZ60 cosine position
- KMZ60 sine position
- torque
- current
- vref 1V65

*/
msg_t sensorConvert(sensor_vitals_t *vitals);

#endif /* _SENSOR_H_ */
