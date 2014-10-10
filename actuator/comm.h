/*

Cuddlebot actuator firmware - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

*/

/* RS-485 driver implementation. */

#ifndef _COMM_H_
#define _COMM_H_

/* Message metadata. */
typedef struct {
  uint8_t addr;               // message address
  uint8_t type;               // message type
  uint16_t len;               // message data length
  uint8_t *buf;               // message data buffer
} CommMeta;

/*

Message address check callback.

The callback should return false to ignore a message.

@param addr The address to check

*/
typedef bool (*commacb_t)(uint8_t addr);

/*

Message service callback.

@param type The message type
@param buf The message data buffer
@param len The message data length

*/
typedef msg_t (*commscb_t)(CommMeta *meta);

/*

Comm driver state.

The service callback function `scb` is run in the same thread as the
serial handler and the metadata structure passed to it is only valid
until the function exits. The function must not use the metadata
structure after the function exits.

The driver state contains the data buffer used for receiving messages
and the driver should be statically allocated to avoid memory
allocation problems.

*/
typedef struct {
  SerialDriver *sd;           // serial driver
  const commacb_t acb;        // address check callback
  const commscb_t scb;        // service callback
  const ioportid_t txenport;    // RS-485 enable port
  const ioportmask_t txenpad;   // RS-485 enable pad in port
  const tprio_t prio;         // service thread priority
  const systime_t timeout;    // RS-485 comm timeout
  // internal state
  uint8_t buf[1024];          // data buffer
  Thread *tp;                 // handler thread buffer
  WORKING_AREA(wa, 128);      // handler thread stack memory
} CommDriver;

/*

Start the RS-485 driver.

@param sd The serial driver

*/
void commStart(CommDriver *comm);

/*

Stop the RS-485 driver.

@param sd The serial driver

*/
void commStop(CommDriver *comm);

#endif // _COMM_H_
