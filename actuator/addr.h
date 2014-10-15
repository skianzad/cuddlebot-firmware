/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#ifndef _ADDR_H_
#define _ADDR_H_

#include <stdbool.h>
#include <stdint.h>

#include "msgtype.h"

/*

Board address.

This value should not change after a call to `addrRead()`.

*/
extern uint8_t _local_addr;

/*

Read the board address.

The actuator boards are arranged from rear to head of the CuddleBot
in the following order:

        Conn     ADDR0/1    Slave              H1 H0 L1 L0    ADDR
Rear    CN105    VCC HIZ    ADDR_RIBS          1  1  0  1    0x10
        CN107    VCC VCC    ADDR_PURR          1  1  1  1    0x08
        CN103    VCC GND    ADDR_SPINE         0  1  0  1    0x04
        CN102    GND VCC    ADDR_HEAD_YAW      1  0  1  0    0x02
Head    CN101    GND GND    ADDR_HEAD_PITCH    0  0  0  0    0x01

These values were measured rather than calculated.

This function should be invoked during system initialization and
before configuring interrupt handlers and higher priority threads.

*/
void addrLoad(void);

/* Get the board address. */
#define addrGet() (_local_addr)

/*

Check if the input address matches the board address.

@param addr input address

*/
#define addrIsSelf(addr) (addr == addrGet())

#endif /* _ADDR_H_ */
