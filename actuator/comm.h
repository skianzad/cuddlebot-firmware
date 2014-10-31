/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

/* RS-485 driver implementation. */

#ifndef _COMM_H_
#define _COMM_H_

#include <ch.h>
#include <hal.h>

#include "msgtype.h"
#include "rs485.h"

/* Communication driver states. */
typedef enum {
  COMM_UNINIT = 0,
  COMM_STOP = 1,
  COMM_READY = 2
} commstate_t;

/* Communication I/O types. */
typedef union {
  BaseChannel *chnp;
  BaseSequentialStream *chp;
  RS485Driver *rsdp;
} commio_t;

/* Communication driver configuration. */
typedef struct {
  MemoryPool *pool;                     // setpoint memory pool
  Mailbox *mbox;                        // setpoint mailbox
  size_t object_size;                   // setpoint object size
  commio_t io;                          // I/O accessors
} CommConfig;

/* Communication driver structure. */
typedef struct {
  commstate_t state;                    // driver state
  CommConfig config;                    // configuration
} CommDriver;

extern CommDriver COMM1;

void commInit(void);

void commObjectInit(CommDriver *comm);

void commStart(CommDriver *comm, CommConfig *config);

msg_t commHandle(CommDriver *comm);

#endif // _COMM_H_
