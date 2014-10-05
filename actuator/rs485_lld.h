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

#ifndef _RS485_LLD_H_
#define _RS485_LLD_H_

/*

Start the low level RS-485 driver.

*/
void rsd_lld_start(void);

/*

Enable RS-485 driver.

*/
void rsd_lld_tx_enable(void);

/*

Disable RS-485 driver.

*/
void rsd_lld_tx_disable(void);

/*

Stop the low level RS-485 driver.

*/
void rsd_lld_stop(void);

/*

Send data and wait until transmission completes.

*/
msg_t rsd_lld_send(size_t n, const void *txbuf);

/*

Receive data and wait until the buffer is filled.

*/
msg_t rsd_lld_recv(size_t n, void *rxbuf);

#endif // _RS485_LLD_H_
