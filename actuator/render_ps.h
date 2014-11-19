/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#ifndef _RENDER_PS_H_
#define _RENDER_PS_H_

#include <stdint.h>
#include "render.h"

/* Pulse-step setpoint structure for idiomatic access. */
typedef struct {
  uint8_t pulse_duration;
  uint8_t step;
} ps_setpoint_t;

/* Pulse-step renderer data. */
#define _ps_render_driver_data                                              \
  _base_render_driver_data                                                  \
  /* Union for idiomatic access to setpoint data. */                        \
  union {                                                                   \
    /* Pulse-step data access. */                                           \
    struct {                                                                \
      /* First byte (first 8 bits in little-endian) is the pulse duration. */ \
      uint8_t pulse_duration;                                               \
      /* Second byte (last 8 bits in little-endian) is the step value. */   \
      int8_t step;                                                          \
    } ps;                                                                   \
    /* Setpoint data access. */                                             \
    uint16_t v;                                                             \
  } setpoint;                                                               \
  /* Pulse duration counter. */                                             \
  uint16_t pulse_duration;                                                  \
  /* Pulse PWM output value. */                                             \
  int8_t pulse_pwm;                                                         \

/* Pulse-step renderer virtual methods table. */
struct PSRenderDriverVMT {
  _base_render_driver_methods
};

/* Pulse-step renderer driver structure. */
typedef struct {
  const struct PSRenderDriverVMT *vmt;
  _ps_render_driver_data
} PSRenderDriver;

/*

Initialize a pulse-step render driver object.

@param rdp The pulse-step render driver object

*/
void psrdObjectInit(PSRenderDriver *rdp);

/* Pulse-step render object instance. */
extern PSRenderDriver PSRENDER1;

#endif // _RENDER_PS_H_
