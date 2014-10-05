/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

/*

RS-485 driver implementation.

*/

#ifndef _RS485_H_
#define _RS485_H_

#include "rs485_lld.h"

/*

Start the RS-485 driver.

*/
void rsdStart(void);

/*

Stop the RS-485 driver.

*/
void rsdStop(void);

/*

Send data and wait until transmission completes.

*/
msg_t rsdSend(size_t n, const void *txbuf);

/*

Send data and wait until transmission completes, without locking the
system.

*/
#define rsdSendS(n, txbuf) rsd_lld_send(n, txbuf)

/*

Receive data and wait until the buffer is filled.

*/
msg_t rsdRecv(size_t n, void *rxbuf);

/*

Receive data and wait until the buffer is filled, without locking the
system.

*/
#define rsdRecvS(n, rxbuf) rsd_lld_recv(n, rxbuf)

#endif // _RS485_H_
