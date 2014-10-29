/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#ifndef _CRC32_H_
#define _CRC32_H_

#include <stdint.h>

/* Reset CRC calculation unit. */
#define crcReset() { CRC->CR = 1; }

/*

Update CRC calculation with a byte of data.

@param b The byte of data

*/
#define crcUpdate(b) { CRC->DR = b; }

/*

Read CRC value.

@return 32-bit CRC value

*/
#define crcValue() (CRC->DR)

/*

Update CRC calculation with multiple bytes of data.

@param buf The array of bytes of data
@param n The number of bytes

*/
void crcUpdateN(const uint8_t *buf, const size_t n);

#endif // _CRC32_H_
