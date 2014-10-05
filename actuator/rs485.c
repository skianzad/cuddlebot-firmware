/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <ch.h>
#include <hal.h>

#include "rs485.h"
#include "rs485_lld.h"

void rsdInit(void) {
	rsd_lld_init();
}

void rsdStart(void) {
	// enable RS-485 driver
	rsd_lld_tx_enable();
	// start USART driver
	rsd_lld_start();
}

void rsdStop(void) {
	// disable RS-485 driver
	rsd_lld_tx_disable();
	// stop USART driver
	rsd_lld_stop();
}

msg_t rsdSend(size_t n, const void *txbuf) {
	// lock system
	chSysLock();
	// send data
	msg_t err = rsdSendS(n, txbuf);
	// unlock system
	chSysUnlock();
	// return error
	return err;
}

msg_t rsdRecv(size_t n, void *rxbuf) {
	// lock system
	chSysLock();
	// receive data
	msg_t err = rsdRecvS(n, rxbuf);
	// unlock system
	chSysUnlock();
	// return error
	return err;
}
