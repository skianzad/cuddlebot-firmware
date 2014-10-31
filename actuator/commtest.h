/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

#ifndef _COMMTEST_H_
#define _COMMTEST_H_

#include "comm.h"

/*

Run all tests.

@param comm Comm driver

*/
void commtestAll(CommDriver *comm);

/*

Test motion driver.

@param comm Comm driver

*/
void commtestMotion(CommDriver *comm);

#endif // _TEST_H_
