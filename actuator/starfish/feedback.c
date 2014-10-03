/*
 * computations relating to closed loop feedback
 *
 */
#include "feedback.h"
#include "trajectories.h"
#include "adc.h"
#include "pwm.h"
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

float m_proportionalGain = 0.0f;
float m_integralGain = 0.0f;
float m_derivativeGain = 0.0f;

float m_velocity = 0.0f;
float m_rawDerivative = 0.0f;
float m_derivative = 0.0f;
float m_derivativeFilterAcc[3] = {0.0f, 0.0f, 0.0f};
float m_integral = 0.0f;
float m_lastPosition = 0.0f;
float m_lastError = 0.0f;
float m_rawVelocity = 0.0f;
float m_velocityFilterAcc[3] = {0.0f, 0.0f, 0.0f};

#define ERROR_INTEGRAL_LIMIT 0.2f
#define POSITION_MAX (0.5f*(float)M_PI)//max swing of position about m_positionOffset

FeedbackType m_feedbackType = FB_DISABLED;

/***************************************************************
 * private prototypes
 */
float constrainPosition(float pos);
float lowPassFilter(float acc[], float in);

/***************************************************************
 * code
 */

void feedbackInit(void) {
	m_feedbackType = FB_DISABLED;
	m_integral = 0;
	m_proportionalGain = 0;
	m_integralGain = 0;
	m_derivativeGain = 0;
}

void updateCoeffs(float pGain, float iGain, float dGain, FeedbackType fbt) {
	m_proportionalGain = pGain;
	m_integralGain = iGain;
	m_derivativeGain = dGain;
	m_feedbackType = fbt;
}

void calcFeedback(float setpoint, float pos, float current, float torque) {

	//compute actual position
	float position = pos;//constrainPosition(pos);

	//compute latest m_velocity
	m_rawVelocity = constrainPosition(position - m_lastPosition) * getSampleFreq();
	m_velocity = lowPassFilter(m_velocityFilterAcc, m_rawVelocity);

	m_lastPosition = position;

	float error;

	switch (isMotionStopped() ? FB_DISABLED : m_feedbackType) {
	default:
	case FB_DISABLED:
		error = 0.0f;
		m_integral = 0.0f;
		break;
	case FB_OPEN_LOOP:
		error = setpoint;
		break;
	case FB_POSITION:
		error = constrainPosition(setpoint - position);
		break;
	case FB_VELOCITY:
		error = setpoint - m_velocity;
		break;
	case FB_FORCE:
		error = setpoint - torque;
		break;
	case FB_CURRENT:
		error = setpoint - current;
		break;
	}
	//trap illegal numbers
	if (!isfinite(error)) {
		error = 0.0f;
	}
	//compute new derivative
	m_rawDerivative = (m_lastError - error) * getSampleFreq();
	m_derivative = lowPassFilter(m_derivativeFilterAcc, m_rawDerivative);
	m_lastError = error;

	//compute new integral
	if (error > ERROR_INTEGRAL_LIMIT) {
		m_integral = 0.0f;
	} else {
		m_integral += error * getSampleTime();
	}

	float temp = 0.0f;
	//now compute output
	temp = error * m_proportionalGain + m_derivative * m_derivativeGain + m_integral * m_integralGain;

	//zero output if we're out of bounds in that direction
	if (position > POSITION_MAX && temp > 0.0f) {
		temp = 0.0f;
	} else if (position < -POSITION_MAX && temp < 0.0f) {
		temp = 0.0f;
	}

	setPwmValue(-temp);
}

float constrainPosition(float pos) {
	//assume it's close
	if (pos > PI) {
		return pos - 2 * PI;
	} else if (pos < -PI) {
		return pos + 2 * PI;
	} else {
		return pos;
	}
}
FeedbackType getFeedbackType() {
	return m_feedbackType;
}
float getProportionalGain() {
	return m_proportionalGain;
}

float getIntegralGain() {
	return m_integralGain;
}
float getDerivativeGain() {
	return m_derivativeGain;
}

float getVelocity() {
	return m_velocity;
}
float lowPassFilter(float acc[], float in) {
	//first stage
	acc[0] = 0.95f * acc[0] + 0.05f * in;
	if (!isfinite(acc[0])) {
		acc[0] = in;
	}
	//second stage
	acc[1] = 0.95f * acc[1] + 0.05f * acc[0];
	if (!isfinite(acc[1])) {
		acc[1] = acc[0];
	}
	//third stage
	acc[2] = 0.95f * acc[2] + 0.05f * acc[1];
	if (!isfinite(acc[2])) {
		acc[2] = acc[1];
	}
	return acc[2];
}
