/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#ifndef _PID_H_
#define _PID_H_

#include <math.h>

#include <ch.h>
#include <hal.h>

#include "motor.h"

/* PID driver state. */
typedef struct {
	// coefficients
	float kp;
	float ki;
	float kd;
	float setpoint;
	// private: internal state
	float lasterr;
	float integrator;
} PIDDriver;

/* PID configuration. */
typedef struct {
	float kp;                             // The p coefficient value
	float ki;                             // The i coefficient value
	float kd;                             // The d coefficient value
	float setpoint;                       // Starting setpoint
	float frequency;                      // Frequency of updates, in Hz
} PIDConfig;

/* Default PID configuration. */
extern PIDConfig DefaultPIDConfig;

/*

Initialize PID driver.

@param pid The PID driver

*/
void pidObjectInit(PIDDriver *pid);

/*

Start the PID driver.

@param pid The PID driver
@param config The PID driver configuration

*/
void pidStart(PIDDriver *pid, const PIDConfig *config);

/*

Change the PID coefficients.

@param pid The PID driver
@param config The PID driver configuration for kp, ki, and kd

*/
void pidSetCoeff(PIDDriver *pid, const PIDConfig *config);

/*

Set PID setpoint, limiting the maximum change in value to 1 deg.

@param pid The PID driver
@param setpoint The setpoint
@return Actual setpoint value accepted

*/
float pidSetpoint(PIDDriver *pid, float setpoint);

/*

Update PID for position value.

@param pid The PID driver
@param value The position value
@return Motor PWM value

*/
float pidUpdate(PIDDriver *pid, float value);

#endif // _PID_H_
