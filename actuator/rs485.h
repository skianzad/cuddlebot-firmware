/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

/*

Low level RS-485 driver implementation. Not thread safe.

*/

#ifndef _RS485_H_
#define _RS485_H_

#include <ch.h>
#include <hal.h>

/* RS-485 driver structure. */
typedef struct {
  const struct BaseAsynchronousChannelVMT *vmt;
  _base_asynchronous_channel_data
  UARTDriver *uart;
  Mutex lock;
  BinarySemaphore ready;
  uartflags_t e;
  msg_t err;
  uint8_t *buf;
  size_t len;
  size_t i;
} RS485Driver;

/* RS-485 driver associated with USART3. */
extern RS485Driver RSD3;

/* Initialize the low level RS-485 driver. */
void rs485Init(void);

/*

Initialize RS-485 driver.

@param rsp Pointer to RS-485 driver
@param uart Pointer to UART driver

*/
void rs485ObjectInit(RS485Driver *rsp, UARTDriver *uart);

/*

Start the low level RS-485 driver.

@param rsp Pointer to RS-485 driver

*/
void rs485Start(RS485Driver *rsp);

/*

Stop the low level RS-485 driver.

@param rsp Pointer to RS-485 driver

*/
void rs485Stop(RS485Driver *rsp);

/*

Wait until the line is idle.

@param rsp Pointer to RS-485 driver

*/
void rs485Wait(RS485Driver *rsp);

#endif // _RS485_H_
