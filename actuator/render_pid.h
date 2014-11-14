/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#ifndef _RENDER_PID_H_
#define _RENDER_PID_H_

#include <stdint.h>
#include "pid.h"
#include "render.h"

/* PID renderer data. */
#define _pid_render_driver_data                                             \
  _base_render_driver_data                                                  \
  PIDDriver pid;                                                            \
  /* The position is read and saved before the system is locked. */         \
  float pos;                                                                \
  /* The setpoint is scaled to between [0, 2*pi]. */                        \
  float setpointf;                                                          \
  /* The original setpoint saved for memoization. */                        \
  uint16_t setpoint;                                                        \

/* PID renderer virtual methods table. */
struct PIDRenderDriverVMT {
  _base_render_driver_methods
};

/* PID renderer driver structure. */
typedef struct {
  const struct PIDRenderDriverVMT *vmt;
  _pid_render_driver_data
} PIDRenderDriver;

/*

Initialize a PID render driver object.

@param rdp The PID render driver object

*/
void pidrdObjectInit(PIDRenderDriver *rdp);

/*

Start a PID render driver object.

@param rdp The PID render driver object
@param pidconfig The PID configuration

*/
void pidrdStart(PIDRenderDriver *rdp, PIDConfig *pidcfg);

/* PID render object instance. */
extern PIDRenderDriver PIDRENDER1;

#endif // _RENDER_PID_H_
