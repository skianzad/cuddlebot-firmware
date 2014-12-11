/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#ifndef _RENDER_H_
#define _RENDER_H_

#include <stdint.h>

/*

Virtual method table methods.

`will_render` is called before locking the system for a motion update.

`render` is called every 1 ms to apply a setpoint update, if one is
available.

`has_rendered` is called after unlocking the system from a motion
update.

*/
#define _base_render_driver_methods                                         \
  void (*reset)(void *instance);                                            \
  void (*will_render)(void *instance);                                      \
  int8_t (*render)(void *instance, uint16_t setpoint);                      \
  void (*has_rendered)(void *instance);                                     \

/*

Base renderer data. Empty for now.

*/
#define _base_render_driver_data

/*

Base renderer virtual methods table.

*/
struct BaseRenderDriverVMT {
  _base_render_driver_methods
};

/*

Base renderer driver structure.

*/
typedef struct {
  const struct BaseRenderDriverVMT *vmt;
  _base_render_driver_data
} BaseRenderDriver;

/*

Call the `reset` method on `rp`.

@param rp The render driver

*/
#define rdReset(rp) ((rp)->vmt->reset(rp))

/*

Call the `will_render` method on `rp`.

@param rp The render driver

*/
#define rdWillRender(rp) ((rp)->vmt->will_render(rp))

/*

Call the `render` method on `rp`.

@param rp The render driver
@param sp The target setpoint

*/
#define rdRender(rp, sp) ((rp)->vmt->render(rp, sp))

/*

Call the `has_rendered` method on `rp`.

@param rp The render driver

*/
#define rdHasRendered(rp) ((rp)->vmt->has_rendered(rp))

#endif // _RENDER_H_
