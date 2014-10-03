/**
 *  \file comms__usart.h
 *  Created: 2012-07-31
 *  Author: Brett McGill
 */


#ifndef COMMS__USART_H_
#define COMMS__USART_H_


#include "config.h"
#include <stdbool.h>
#include <stdint.h>

// check against longest string in comms__bluetooth_config.c: buffers must be be enough to hold any defined string
#define COMMS_BUFFER_LEN  90

// fast-code branch for receiving data.
// initialisation code for UART
void hostCommsInit(void);
void hostCommsSlowCode(void);
void hostCommsBranch();
void queueConfigReplyPacket();
void queueMotionReplyPacket();
#endif // COMMS__USART_H_
