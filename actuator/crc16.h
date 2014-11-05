/*

This file is modified from modbus-master.

modbus-master is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

modbus-master is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with modbus-master.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef _CRC16_H_
#define _CRC16_H_

#include <stdint.h>
#include <string.h>

typedef union {
  struct {
    uint8_t lo;
    uint8_t hi;
  } b;
  uint16_t v;
} crc16_t;

void crc16Reset(crc16_t *c);

void crc16Update(crc16_t *c, const uint8_t b);

void crc16UpdateN(crc16_t *c, const uint8_t *buf, const size_t n);

#define crc16Value(c) ((c)->v)

#endif // _CRC16_H_
