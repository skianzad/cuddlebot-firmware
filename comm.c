/*

Fabric pressure sensor - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <ch.h>
#include <hal.h>


void txend1_cb(UARTDriver *uartp) {
	(void)uartp;
	// const uint8_t *helloworld = (uint8_t *)("Hello World!\n");
	// uartStartSend(&UARTD3, sizeof(helloworld), helloworld);
}

void rxchar_cb(UARTDriver *uartp, uint16_t c) {
	(void)uartp;
	(void)c;
}

const UARTConfig uartcfg = {
	.txend1_cb = NULL,
	.txend2_cb = NULL,
	.rxend_cb = NULL,
	.rxchar_cb = NULL,
	.rxerr_cb = NULL,
	// hardware-specific configuration
	.speed = SERIAL_DEFAULT_BITRATE,
	.cr1 = 0, // useful: RWU (mute mode) and SBK (send break)
	.cr2 = 0,
	.cr3 = 0
};

void cm_comm_init(void) {
	// start uart driver
	uartStart(&UARTD3, &uartcfg);

	// debug
	// txend1_cb(&UARTD3);
}

// void cm_comm_send(size_t n, const void *txbuf) {
//  uartStartSend(&UARTD3, n, txbuf);
// }

// void cm_comm_sendi(size_t n, const void *txbuf) {
//  uartStartSendI(&UARTD3, n, txbuf);
// }

// void cm_comm_stop_send(size_t n, const void *txbuf) {
//  uartStopSend(&UARTD3);
// }

// void cm_comm_stop_sendi(size_t n, const void *txbuf) {
//  uartStopSendI(&UARTD3);
// }

// void cm_comm_recv(size_t n, const void *txbuf) {
//  uartStartReceive(&UARTD3, n, txbuf);
// }

// void cm_comm_recvi(size_t n, const void *txbuf) {
//  uartStartReceiveI(&UARTD3, n, txbuf);
// }

// void cm_comm_stop_recv(size_t n, const void *txbuf) {
//  uartStopReceive(&UARTD3);
// }

// void cm_comm_stop_recvi(size_t n, const void *txbuf) {
//  uartStopReceiveI(&UARTD3);
// }
