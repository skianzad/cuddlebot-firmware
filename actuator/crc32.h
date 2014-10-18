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

uint32_t crc32(const char *buf, const size_t len);

#endif // _CRC32_H_
