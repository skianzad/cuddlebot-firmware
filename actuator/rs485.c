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

RS485Driver RSD3;

void txend1_cb(UARTDriver *uartp) {
	if (uartp == RSD3.uart) {
		chSysLockFromIsr();
		chBSemSignalI(&RSD3.ready);
		chSysUnlockFromIsr();
	}
}

void rxend_cb(UARTDriver *uartp) {
	if (uartp == RSD3.uart) {
		chSysLockFromIsr();
		chBSemSignalI(&RSD3.ready);
		RSD3.e = 0;
		chSysUnlockFromIsr();
	}
}

void rxchar_cb(UARTDriver *uartp, uint16_t c) {
	if (uartp == RSD3.uart) {
		if (RSD3.buf == NULL) {
			return;
		}
		if (RSD3.len >= RSD3.i) {
			RSD3.buf = NULL;
			chSysLockFromIsr();
			chBSemSignalI(&RSD3.ready);
			RSD3.e = 0;
			chSysUnlockFromIsr();
		}
		RSD3.buf[RSD3.i++] = c;
	}
}

void rxerr_cb(UARTDriver *uartp, uartflags_t e) {
	if (uartp == RSD3.uart) {
		chSysLockFromIsr();
		chBSemSignalI(&RSD3.ready);
		RSD3.e = e;
		chSysUnlockFromIsr();
	}
}

const UARTConfig uartcfg = {
	.txend1_cb = txend1_cb, // End of transmission buffer callback.
	.txend2_cb = NULL,      // Physical end of transmission callback.
	.rxend_cb = rxend_cb,   // Receive buffer filled callback.
	.rxchar_cb = NULL,      // Character received while idle.
	.rxerr_cb = rxerr_cb,   // Receive error callback.
	// hardware-specific configuration
	.speed = SERIAL_DEFAULT_BITRATE,
	.cr1 = 0, // useful: RWU (mute mode) and SBK (send break)
	.cr2 = 0,
	.cr3 = 0
};

static size_t writet(void *ip, const uint8_t *bp, size_t n, systime_t time) {
	RS485Driver *rsp = (RS485Driver *)ip;
	// lock system and UART
	chSysLock();
	chMtxLockS(&rsp->lock);
	// wait for idle
	rsp->uart->usart->CR1 = USART_CR1_RWU;
	while (rsp->uart->usart->CR1 & USART_CR1_RWU) {
		chSchDoYieldS();
	}
	// enable transmitter
	rsp->uart->usart->CR1 |= USART_CR1_TE;
	// enable RS-485 driver
	palSetPad(GPIOB, GPIOB_RS485_TXEN);
	// start sending
	uartStartSendI(rsp->uart, n, bp);
	// wait until finished
	rsp->err = chBSemWaitTimeoutS(&rsp->ready, time);
	if (rsp->err != RDY_OK) {
		// stop sending due to error
		n = uartStopSendI(rsp->uart);
	} else {
		// wait until add data cleared
		while (!(rsp->uart->usart->SR & USART_SR_TC)) {
			chSchDoYieldS();
		}
	}
	// disable RS-485 driver
	palClearPad(GPIOB, GPIOB_RS485_TXEN);
	// disable transmitter
	rsp->uart->usart->CR1 &= ~USART_CR1_TE;
	// unlock UART and system
	chMtxUnlockS();
	chSysUnlock();
	// return number of bytes transmitted
	return n;
}

static size_t readt(void *ip, uint8_t *bp, size_t n, systime_t time) {
	RS485Driver *rsp = (RS485Driver *)ip;
	// lock system and UART
	chSysLock();
	chMtxLockS(&rsp->lock);
	// enable receiver
	rsp->uart->usart->CR1 |= USART_CR1_RE;
	// start receiving
	uartStartReceiveI(rsp->uart, n, bp);
	// wait until finished
	rsp->err = chBSemWaitTimeoutS(&rsp->ready, time);
	if (rsp->err != RDY_OK || rsp->e != UART_NO_ERROR) {
		// stop receiving due to error
		n = uartStopReceiveI(rsp->uart);
	}
	// disable receiver
	rsp->uart->usart->CR1 &= ~USART_CR1_RE;
	// unlock UART and system
	chMtxUnlockS();
	chSysUnlock();
	// return number of bytes received
	return n;
}

static msg_t putt(void *ip, uint8_t b, systime_t timeout) {
	writet(ip, &b, 1, timeout);
	return ((RS485Driver *)ip)->err;
}

static msg_t gett(void *ip, systime_t timeout) {
	uint8_t b = 0;
	if (readt(ip, &b, 1, timeout) != 1) {
		RS485Driver *rsp = (RS485Driver *)ip;
		return rsp->e == UART_NO_ERROR ? rsp->err : RDY_RESET;
	}
	return b;
}

static size_t write(void *ip, const uint8_t *bp, size_t n) {
	return writet(ip, bp, n, TIME_INFINITE);
}

static size_t read(void *ip, uint8_t *bp, size_t n) {
	return readt(ip, bp, n, TIME_INFINITE);
}

static msg_t put(void *ip, uint8_t b) {
	return putt(ip, b, TIME_INFINITE);
}

static msg_t get(void *ip) {
	return gett(ip, TIME_INFINITE);
}

static const struct BaseAsynchronousChannelVMT vmt = {
	write, read, put, get,
	putt, gett, writet, readt
};

void rs485Init(void) {
	rs485ObjectInit(&RSD3, &UARTD3);
}

void rs485ObjectInit(RS485Driver *rsp, UARTDriver *uart) {
	rsp->vmt = &vmt;
	rsp->uart = uart;
	chMtxInit(&rsp->lock);
	chBSemInit(&rsp->ready, FALSE);
}

void rs485Start(RS485Driver *rsp) {
	// start UART driver
	uartStart(rsp->uart, &uartcfg);
	// disable transmitter and receiver
	rsp->uart->usart->CR1 &= ~(USART_CR1_TE | USART_CR1_RE);
}

void rs485Stop(RS485Driver *rsp) {
	// disable RS-485 driver
	palClearPad(GPIOB, GPIOB_RS485_TXEN);
	// stop UART driver
	uartStop(rsp->uart);
}

void rs485Wait(RS485Driver *rsp) {
	// lock system and UART
	chSysLock();
	chMtxLockS(&rsp->lock);
	// wait for idle
	rsp->uart->usart->CR1 = USART_CR1_RWU;
	while (rsp->uart->usart->CR1 & USART_CR1_RWU) {
		chSchDoYieldS();
	}
	// unlock UART and system
	chMtxUnlockS();
	chSysUnlock();
}
