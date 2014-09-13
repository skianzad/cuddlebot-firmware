/*

Fabric pressure sensor - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#ifndef _USB_SERIAL_H_
#define _USB_SERIAL_H_

#include <ch.h>
#include <hal.h>

// Initialize serial port over USB driver.
void cm_usb_serial_init(void);

#endif // _USB_SERIAL_H_
