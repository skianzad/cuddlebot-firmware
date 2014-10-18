/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#include <ch.h>
#include <hal.h>

#include "crc32.h"

/*

Calculate a CRC-32 checksum.

@param buf The data buffer
@param len The length of the data

*/
uint32_t crc32(const char *buf, const size_t len) {
  size_t i;

  // reset CRC-32 unit
  CRC->CR = 1;

  // update CRC-32 calculation per byte
  for (i = 0; i < len; i++) {
    CRC->DR = buf[i];
  }

  // return hardware-calculated value
  return CRC->DR;
}
