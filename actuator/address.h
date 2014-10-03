/*

Cuddlemaster - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#ifndef _ADDRESS_H_
#define _ADDRESS_H_

#include <stdbool.h>
#include <stdint.h>

// Board bitset addresses.
#define ADDRESS_INVALID     0x00
#define ADDRESS_RIBS        0x01
#define ADDRESS_HEAD_PITCH  0x02
#define ADDRESS_HEAD_YAW    0x04
#define ADDRESS_SPINE       0x08
#define ADDRESS_PURR        0x10

// Board address type.
typedef uint8_t cm_address_t;

/*

Board address.

This value should not change after a call to `cm_address_read()`.

*/
extern cm_address_t cm_address;

/*

Read the board address and save to `cm_address`.

The actuator boards are arranged from rear to head of the CuddleBot
in the following order:

        Conn     ADDR0/1    Slave                H1 H0 L1 L0    ADDR
Rear    CN105    VCC HIZ    ADDRESS_RIBS          1  1  1  0    0x01
        CN107    VCC VCC    ADDRESS_PURR          1  1  1  1    0x10
        CN103    VCC GND    ADDRESS_HEAD_PITCH    1  0  1  0    0x02
        CN102    GND VCC    ADDRESS_SPINE         0  1  0  1    0x08
Head    CN101    GND GND    ADDRESS_HEAD_YAW      0  0  0  0    0x04

This function should be invoked during system initialization and
before configuring interrupt handlers and higher priority threads.

*/
void cm_address_read(void);

/*

Check if the input address matches the board address.

@param addr input address

*/
bool cm_address_is_self(cm_address_t addr);

#endif /* _ADDRESS_H_ */
