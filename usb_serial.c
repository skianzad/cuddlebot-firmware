/*

Fabric pressure sensor - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <ch.h>
#include <hal.h>
#include <chprintf.h>

#include "usb_serial.h"
#include "usbcfg.h"

// Virtual serial port over USB driver.
SerialUSBDriver SDU1;

// Initialize serial port over USB driver.
void cm_usb_serial_init(void) {

	// initialize the serial-over-USB driver
	sduObjectInit(&SDU1);
	sduStart(&SDU1, &serusbcfg);

	// Activate the USB driver and then the USB bus pull-up on D+. Note,
	// a delay is inserted in order to not have to disconnect the cable
	// after a reset.
	usbDisconnectBus(serusbcfg.usbp);
	chThdSleepMilliseconds(1000);
	usbStart(serusbcfg.usbp, &usbcfg);
	usbConnectBus(serusbcfg.usbp);
}
