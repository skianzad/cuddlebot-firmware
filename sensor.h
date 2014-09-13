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

#define GROUND_SIZE   8
#define POWER_SIZE    8

// Sensor sample and message.
typedef struct
{
    uint32_t time;
    adcsample_t values[GROUND_SIZE][POWER_SIZE];
    uint8_t checksum;
} sensor_sample_t;

/*

ADC conversion group.

Mode:     linear buffer, 1 sample of 1 channel, SW triggered.

Timing:   15 cycles sample time
          15 cycles conversion time
          30 total cycles
          ~1.43 µs total time @ 21 Mhz ADC clock

*/
extern const ADCConversionGroup adcgrpcfg;

/*

Sample the pressure grid.

1. Set the appropriate pin modes.
2. Enables the ADC.
3. Samples the pressure sensor grid.
4. Disables the ADC.
5. Sends the data using the serial driver.

*/
void sample_grid(sensor_sample_t *buf);

#endif // _SENSOR_H_
