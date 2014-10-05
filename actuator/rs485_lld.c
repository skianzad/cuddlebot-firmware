/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <ch.h>
#include <hal.h>

#include "rs485_lld.h"

static MUTEX_DECL(uart_txlock);
static MUTEX_DECL(uart_rxlock);
static BSEMAPHORE_DECL(uart_txsig, FALSE);
static BSEMAPHORE_DECL(uart_rxsig, FALSE);

void txend1_cb(UARTDriver *uartp) {
	(void)uartp;
	chBSemSignalI(&uart_txsig);
}

void rxend_cb(UARTDriver *uartp) {
	(void)uartp;
	chBSemSignalI(&uart_rxsig);
}

void rxchar_cb(UARTDriver *uartp, uint16_t c) {
	(void)uartp;
	(void)c;
}

const UARTConfig uartcfg = {
	.txend1_cb = txend1_cb, // End of transmission buffer callback.
	.txend2_cb = NULL, 			// Physical end of transmission callback.
	.rxend_cb = rxend_cb,   // Receive buffer filled callback.
	.rxchar_cb = NULL,      // Character received while idle.
	.rxerr_cb = NULL,       // Receive error callback.
	// hardware-specific configuration
	.speed = SERIAL_DEFAULT_BITRATE,
	.cr1 = 0, // useful: RWU (mute mode) and SBK (send break)
	.cr2 = 0,
	.cr3 = 0
};

void rsd_lld_start(void) {
	// start UART driver
	uartStart(&UARTD3, &uartcfg);
}

void rsd_lld_stop(void) {
	// stop UART driver
	uartStop(&UARTD3);
}

void rsd_lld_tx_enable(void) {
	// enable RS-485 driver
	palSetPad(GPIOB, GPIOB_RS485_TXEN);
}

void rsd_lld_tx_disable(void) {
	// disable RS-485 driver
	palClearPad(GPIOB, GPIOB_RS485_TXEN);
}

msg_t rsd_lld_send(size_t n, const void *txbuf) {
	// get exclusive lock on transmission
	chMtxLockS(&uart_txlock);
	// start sending
	uartStartSendI(&UARTD3, n, txbuf);
	// wait until finish transmitting
	msg_t err = chBSemWaitS(&uart_txsig);
	if (err != RDY_OK) {
		return err;
	}
	// relinquish lock on transmission
	chMtxUnlockS();
	// return OK
	return RDY_OK;
}

msg_t rsd_lld_recv(size_t n, void *rxbuf) {
	// get exclusive lock on receive
	chMtxLockS(&uart_rxlock);
	// start receiving
	uartStartReceiveI(&UARTD3, n, rxbuf);
	// wait until finish receiving
	msg_t err = chBSemWaitS(&uart_rxsig);
	if (err != RDY_OK) {
		return err;
	}
	// relinquish lock on transmission
	chMtxUnlockS();
	// return OK
	return RDY_OK;
}
