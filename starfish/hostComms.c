/*

\file comms__usart.c
Created: 2012-07-31
Author: Brett McGill
Author: Kenneth MacCallum

Once configured and activated, we don't offer a way to stop
communications; e.g. blocking send_comms_packet() or discarding
received chars/packets. Disabling of system communications is expected
to be controlled at a higher level. This leaves the comm's free for
sending/receiving shutdown messages to the Bluetooth device or host
computer.

*/

#include "config.h"
#include "hostComms.h"
#include "fastCode.h"
#include "trajectories.h"
#include "address.h"
#include "adc.h"
#include "pins.h"
#include "feedback.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_usart.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>


#define USART_DEF USART3

// variables and defines for UART Tx
#define TX_BUFFER_LINE_LEN      COMMS_BUFFER_LEN
#define NUM_TX_BUFFERS          15
static uint  tx_count[ NUM_TX_BUFFERS];
// used by slow code to index the rx buffer
static uint  tx_buffer_write_index = 0;
// used by fast code to index the rx buffer
static uint  tx_buffer_read_index  = 0;



#define TX_BUFFER_LINE_LENGTH 100
#define RX_BUFFER_LINE_LENGTH 100
#define NUM_RX_BUFFERS          4

union {
	uint i[TX_BUFFER_LINE_LENGTH];
	float f[TX_BUFFER_LINE_LENGTH];
	uint8_t b[TX_BUFFER_LINE_LENGTH][4];
} txBuffer;

union {
	uint i[NUM_RX_BUFFERS][RX_BUFFER_LINE_LENGTH];
	float f[NUM_RX_BUFFERS][RX_BUFFER_LINE_LENGTH];
	uint8_t b[NUM_RX_BUFFERS][RX_BUFFER_LINE_LENGTH][4];
	uint8_t br[NUM_RX_BUFFERS][RX_BUFFER_LINE_LENGTH * 4];
} rxBuffer;

//index to buffer currently being read by slow code
static uint rxReadBufferIndex = 0;
//index to buffer currently being written by fast code
static uint rxWriteBufferIndex = 0;

//packet types
#define MOTION_REQUEST_PACKET_TYPE 0
#define CONFIG_REQUEST_PACKET_TYPE 1
#define MOTION_REPLY_PACKET_TYPE   2
#define CONFIG_REPLY_PACKET_TYPE   3
#define NULL_PACKET_LENGTH 0
#define MOTION_REQUEST_PACKET_LENGTH 32
#define CONFIG_REQUEST_PACKET_LENGTH 44
#define MOTION_REPLY_PACKET_LENGTH   64
#define CONFIG_REPLY_PACKET_LENGTH   44
//motion request types
#define STOP_REQUEST 0
#define STATUS_REQUEST 1
#define QUEUE_REQUEST 2
#define QUEUE_SERIES_REQUEST 3
//config types
#define CONFIG_NULL 0
#define CONFIG_QUERY 1
#define CONFIG_UPDATE 2
//feedback types
#define FEEDBACK_DISABLED 0
#define FEEDBACK_OPEN_LOOP 1
#define FEEDBACK_POSITION 2
#define FEEDBACK_VELOCITY 3
#define FEEDBACK_FORCE 4
#define FEEDBACK_CURRENT 5

int32_t m_wordsToTransmit = 0;
bool m_rxNotTx = true;
static uint blinkTimer = 0;
static bool blink = false;

/*************************************************************************
 * Private Prototypes
 */

/**
 * compute a packet checksum and place it in the last entry of the specified array
 */
void calcChecksumAndSend(uint length);
void processMotionRequest(uint index);
void processConfigRequest(uint index);
void setRxNotTx(bool rxNotTx);
bool isRxNotTx(void);
void hostCommsTxBranch(void);
bool hostCommsRxBranch(void);
bool isTransmitting(void);
float intToFloat(uint *i);

/*************************************************************************
 * Code
 */

/**
 * This is the main branch to call from outside this module. It expects to run faster than 10kHz
 */
void hostCommsBranch() {
	static uint txBlankCount;

	//if we're in rx mode and we've got something
	if (isRxNotTx() && hostCommsRxBranch()) {
		//then prevent transmitting for a bit to be sure we don't collide with anyone else
		txBlankCount = getAddress() * ((uint32_t)20);
	} else {
		if (txBlankCount > 0) {
			--txBlankCount;
		}
	}
	//only run tx branch if nothing has been received for slaveAddress byte times
	if (txBlankCount == 0) {
		if (isTransmitting()) {
			setRxNotTx(false);
			hostCommsTxBranch();
		} else {
			setRxNotTx(true);
		}
	}
}

/*
uart_tx_branch - fast-code branch for sending data. This can be run in
the same branch as Rx

Could possibly add a timeout-check to make sure the transmit buffer is
emptying. We aren't using flow control so the only reason for this
problem is if the USART was glitched. The only sensible handling of
that would be to re-setup the UART.
*/
void hostCommsTxBranch(void) {
	static uint timeoutTimer = 0;
	static uint wordIndex = 0;
	static uint byteIndex = 4;

	//is the tx register not empty?
	if (USART_GetFlagStatus(USART_DEF, USART_SR_TXE) == RESET) {
		//if a byte hasn't been able to tx for a while then reset
		if (has_1ms_timer_expired(timeoutTimer, 5)) {
			restart_1ms_timer(timeoutTimer);
			byteIndex = 4;
			wordIndex = 0;
		}
	} else {
		//if we've sent all the bytes then wait until the USART is done
		// before cancelling transmit mode
		if (wordIndex == m_wordsToTransmit) {
			if (USART_GetFlagStatus(USART_DEF, USART_SR_TC) == SET || has_1ms_timer_expired(timeoutTimer, 5)) {
				m_wordsToTransmit = 0;
				wordIndex = 0;
				byteIndex = 4;
			}
		} else {
			restart_1ms_timer(timeoutTimer);
			--byteIndex;
			USART_SendData(USART_DEF, 0xff & txBuffer.b[wordIndex][byteIndex]);
		}
		if (byteIndex == 0) {
			++wordIndex;
			byteIndex = 4;
		}
	}
}

/**
 * uart_rx_branch - fast-code branch for receiving data.
 * Places discrete packet data into packet buffers in a FIFO structure.
 * returns true if a byte has been received. This allows the tx logic to be stopped until a defined time after rx
 */
bool hostCommsRxBranch(void) {
	// index of character being written into buffer
	static int32_t byteIndex    = 0;
	static uint packetLength = 0;
	static uint checksum = 0;
	static uint wordAcc = 0;
	static uint wordIndex = 0;
	static bool addressOk = false;
	static uint timeoutTimer = 0;
	bool result = false;

	// is there no incoming character?
	if (USART_GetFlagStatus(USART_DEF, USART_SR_RXNE) == RESET) {

		if (byteIndex != 0 && has_1ms_timer_expired(timeoutTimer, 5)) {
			//if we haven't seen a byte for a while then reset the receive routine
			restart_1ms_timer(timeoutTimer);
			byteIndex = 0;
		}
	} else {
		//there is an incoming character

		restart_1ms_timer(timeoutTimer);
		result = true;
		// this will clear the RXNE flag
		char c = USART_ReceiveData(USART_DEF);

		wordAcc <<= 8;
		wordAcc += (uint)c;

		if (byteIndex == 0) {
			checksum = 0;
			wordIndex = 0;
			addressOk = false;
			switch (c) {
			//Motion Request Packet
			case MOTION_REQUEST_PACKET_TYPE:
				packetLength = MOTION_REQUEST_PACKET_LENGTH;
				break;
			//Config Request Packet
			case CONFIG_REQUEST_PACKET_TYPE:
				packetLength = CONFIG_REQUEST_PACKET_LENGTH;
				break;
			//Motion Reply Packet
			case MOTION_REPLY_PACKET_TYPE:
				packetLength = MOTION_REPLY_PACKET_LENGTH;
				break;
			//config Reply Packet
			case CONFIG_REPLY_PACKET_TYPE:
				packetLength = CONFIG_REPLY_PACKET_LENGTH;
				break;
			default:
				packetLength = NULL_PACKET_LENGTH;
				//reset accumulator because this is not a valid packet start
				wordAcc = 0;
				byteIndex = -1;
				break;
			}
		} else if (byteIndex >= packetLength - 1) {
			byteIndex = -1;
			wordIndex = 0;
			if (addressOk) {
				if (checksum == wordAcc) {
					//this will indicate to the slow code that there's a packet ready for processing
					++rxWriteBufferIndex;
					if (rxWriteBufferIndex >= NUM_RX_BUFFERS) {
						rxWriteBufferIndex = 0;
					}
				}
			}
		} else if (byteIndex % 4 == 3) {
			rxBuffer.i[rxWriteBufferIndex][wordIndex] = wordAcc;
			++wordIndex;
			checksum += wordAcc;
			wordAcc = 0;
		} else if (byteIndex == 1 && wordIndex == 0) {
			if ( c == getAddress()) {
				addressOk = true;
			}
		} else {
			//nothing to do except store byte and advance checksum calc, which we've already done
		}
		++byteIndex;
	}

	return result;
}


/**
 * process packet in slow code
 *
 */
void hostCommsSlowCode(void) {
	if (blink) {
		if (has_1ms_timer_expired(blinkTimer, 30)) {
			blink = false;
		}
	} else {
		restart_1ms_timer(blinkTimer);
	}
	if (blink) {
		setLedD200();
	} else {
		clrLedD200();
	}
	if (rxWriteBufferIndex != rxReadBufferIndex) {
		//there's at least one packet to process
		blink = true;
		switch (rxBuffer.i[rxReadBufferIndex][0] & 0xff000000) {
		case (MOTION_REQUEST_PACKET_TYPE << 24):
			processMotionRequest(rxReadBufferIndex);
			queueMotionReplyPacket();
			break;
		case (CONFIG_REQUEST_PACKET_TYPE << 24):
			processConfigRequest(rxReadBufferIndex);
			queueConfigReplyPacket();
			break;
		case (MOTION_REPLY_PACKET_TYPE << 24):
			break;
		case (CONFIG_REPLY_PACKET_TYPE << 24):
			break;
		default:
			//don't do anything because we don't know how to deal with this type of packet
			break;
		}
		++rxReadBufferIndex;
		if (rxReadBufferIndex >= NUM_RX_BUFFERS) {
			rxReadBufferIndex = 0;
		}
	}
}

void processMotionRequest(uint index) {
	// note this is will make this value a float. I don't want to cast
	// because i don't want it to treat this as an int first
	float setpoint = rxBuffer.f[index][1];
	float dsdt = rxBuffer.f[index][2];
	float sTime = rxBuffer.f[index][3];
	uint skinVal = rxBuffer.b[index][4][2];
	uint skinDir = rxBuffer.b[index][4][0];
	uint motionType = 0xff & rxBuffer.b[index][0][1];
	switch (motionType) {
	case STOP_REQUEST:
		stopAllMotion();
		break;
	default:
	case STATUS_REQUEST:
		break;
	case QUEUE_REQUEST:
		updateTrajectories(setpoint, dsdt, sTime);
		break;
	case QUEUE_SERIES_REQUEST:
		break;
	}
	setSkinSensor(skinDir, skinVal);
	queueMotionReplyPacket();

}
void processConfigRequest(uint index) {
	uint header = rxBuffer.i[index][0];
	float pOffset = rxBuffer.f[index][1];
	float pGain = rxBuffer.f[index][2];
	float iGain = rxBuffer.f[index][3];
	float dGain = rxBuffer.f[index][4];
	uint configType = (header >> 8) & 0xff;
	uint feedbackType = (header >> 0) & 0xff;
	FeedbackType fbt;

	switch (feedbackType) {
	default:
	case FEEDBACK_DISABLED:
		fbt = FB_DISABLED;
		break;
	case FEEDBACK_OPEN_LOOP:
		fbt = FB_OPEN_LOOP;
		break;
	case FEEDBACK_POSITION:
		fbt = FB_POSITION;
		break;
	case FEEDBACK_VELOCITY:
		fbt = FB_VELOCITY;
		break;
	case FEEDBACK_FORCE:
		fbt = FB_FORCE;
		break;
	case FEEDBACK_CURRENT:
		fbt = FB_CURRENT;
		break;
	}

	switch (configType) {
	default:
	case CONFIG_NULL:
		break;
	case CONFIG_QUERY:
		break;
	case CONFIG_UPDATE:
		setPositionOffset(pOffset);
		updateCoeffs(pGain, iGain, dGain, fbt);

		break;
	}

	queueConfigReplyPacket();
}
void queueConfigReplyPacket() {
	// don't do anything if we're transmitting. This will cause this
	// packet request to be lost make header
	if (!isTransmitting()) {
		txBuffer.i[0] = (getFeedbackType() << 24) | (getAddress() << 8) | CONFIG_REPLY_PACKET_TYPE;
		txBuffer.f[1] = getPositionOffset();
		txBuffer.f[2] = getProportionalGain();
		txBuffer.f[3] = getIntegralGain();
		txBuffer.f[4] = getDerivativeGain();
		txBuffer.i[5] = VERSION_ID;
		calcChecksumAndSend(CONFIG_REPLY_PACKET_LENGTH / 4);
	}
}

void queueMotionReplyPacket() {
	// don't do anything if we're transmitting. This will cause this
	// packet request to be lost make header
	if (!isTransmitting()) {
		MotionStatusType mst = getMotionStatus();
		uint ms;
		switch (mst) {
		default:
		case MS_STOPPED:
			ms = 0;
			break;
		case MS_IDLE:
			ms = 1;
			break;
		case MS_MOVE_IN_PROGRESS:
			ms = 2;
			break;
		case MS_ERROR:
			ms = 3;
			break;
		}
		txBuffer.b[0][3] = 0xff & MOTION_REPLY_PACKET_TYPE;
		txBuffer.b[0][2] = 0xff & getAddress();
		txBuffer.b[0][1] = 0xff & ms;
		txBuffer.b[0][0] = 0;
#ifdef PWM_DISABLE
		txBuffer.f[1] = getSplineValue();
		txBuffer.f[2] = getSplineDerivative();
#else
		txBuffer.f[1] = getPosition();
		txBuffer.f[2] = getVelocity();
#endif
		txBuffer.f[3] = getTime();
		//loop index. Not yet implemented.
		txBuffer.i[4] = 0;
		txBuffer.f[5] = 1000.0 * getTorque();
		txBuffer.f[6] = getCurrent();
		txBuffer.f[7] = getSensor(0);
		txBuffer.f[8] = getSensor(1);
		txBuffer.f[9] = getSensor(2);
		txBuffer.f[10] = getSensor(3);
		txBuffer.f[11] = getSensor(4);
		txBuffer.f[12] = getSensor(5);
		txBuffer.f[13] = getSensor(6);
		txBuffer.f[14] = getSensor(7);
		//      //TODO: debug values
		//      txBuffer.f[1] = 0.1f;
		//      txBuffer.f[2] = 2.0f;
		//      txBuffer.f[3] = 0.005f;
		//      txBuffer.i[4] = 0;
		//      txBuffer.f[5] = 3.141f;
		//      txBuffer.f[6] = 3.141f;
		//      txBuffer.f[7] = 3.141f;
		//      txBuffer.f[8] = 3.141f;
		//      txBuffer.f[9] = 3.141f;
		//      txBuffer.f[10] = 3.141f;
		//      txBuffer.f[11] = 3.141f;
		//      txBuffer.f[12] = 3.141f;
		//      txBuffer.f[13] = 3.141f;
		//      txBuffer.f[14] = 0.0f;
		//      //TODO: end of debug values

		calcChecksumAndSend(MOTION_REPLY_PACKET_LENGTH / 4);
	}
}

/**
 * initialisation code for UART
 */
void hostCommsInit(void) {
	//-------------------------------------------------------------------------
	// setup application-specific data
	tx_count[0]           = 0;
	tx_buffer_write_index = 0;
	tx_buffer_read_index  = 0;
	//-------------------------------------------------------------------------
	// setup hardware

	// Enable UART clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	/* USART configuration:
	   - BaudRate = 115200 baud
	   - Word Length = 8 Bits
	   - One Stop Bit
	   - No parity
	   - Hardware flow control disabled (RTS and CTS signals)
	   - Receive and transmit enabled
	*/
	//  USART_InitTypeDef usartInit;
	//  USART_StructInit(&usartInit);
	//  usartInit.USART_BaudRate            = 115200;
	//  usartInit.USART_WordLength          = USART_WordLength_8b;
	//  usartInit.USART_StopBits            = USART_StopBits_1;
	//  usartInit.USART_Parity              = USART_Parity_No;
	//  usartInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	//  usartInit.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
	//  USART_Init(USART_DEF, &usartInit);

	setRxNotTx(true);

	// Enable USART
	USART_Cmd(USART_DEF, ENABLE);
}

void setRxNotTx(bool rxNotTx) {
	USART_InitTypeDef usartInit;
	USART_StructInit(&usartInit);
	usartInit.USART_BaudRate            = 115200;
	usartInit.USART_WordLength          = USART_WordLength_8b;
	usartInit.USART_StopBits            = USART_StopBits_1;
	usartInit.USART_Parity              = USART_Parity_No;
	usartInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	if (rxNotTx) {
		usartInit.USART_Mode                = USART_Mode_Rx;
		clrTxEn();
	} else {
		usartInit.USART_Mode                = USART_Mode_Tx;
		setTxEn();
	}
	USART_Init(USART_DEF, &usartInit);
	m_rxNotTx = rxNotTx;
}

bool isRxNotTx(void) {
	return m_rxNotTx;
}

/**
 * compute a packet checksum and place it in the last entry of the specified array
 */
void calcChecksumAndSend(uint length) {
	uint result = 0;
	for (uint i = 0; i < length - 1; ++i) {
		result += txBuffer.i[i];
	}
	txBuffer.i[length - 1] = result;
	m_wordsToTransmit = length;
}
bool isTransmitting(void) {
	return m_wordsToTransmit > 0;
}
float intToFloat(uint *i) {
	return *((float *)i);
}
