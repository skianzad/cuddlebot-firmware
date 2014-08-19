/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "ch.h"
#include "hal.h"

#include "chprintf.h"

#include "usbcfg.h"

// Virtual serial port over USB
SerialUSBDriver SDU1;

//
// Sensor configuration.
//

#define GROUND_SIZE   8
#define POWER_SIZE    8

#define MBOX_SIZE     2
#define BUFFER_LEN    4

// signal_t encapsulates a sensor sample
typedef struct {
  uint32_t time;
  uint16_t values[POWER_SIZE * GROUND_SIZE];
  uint8_t checksum;
} signal_t;

typedef struct {
  ioportid_t port;
  uint8_t channel;
} adc_portmap_t;

static uint8_t mboxbuf[MBOX_SIZE];
static MAILBOX_DECL(mbox, mboxbuf, MBOX_SIZE);

static const uint8_t ground_pins[GROUND_SIZE] = {
  0, 1, 2, 3, 6, 7, 8, 9
};

static const uint8_t power_pins[POWER_SIZE] = {
  4, 5, 7, 8, 11, 12, 13, 14
};

static adc_portmap_t adc_pins[POWER_SIZE] = {
  { GPIOB, ADC_SQR3_SQ1_N(ADC_CHANNEL_IN8) },
  { GPIOB, ADC_SQR3_SQ1_N(ADC_CHANNEL_IN9) },
  { GPIOC, ADC_SQR3_SQ1_N(ADC_CHANNEL_IN11) },
  { GPIOC, ADC_SQR3_SQ1_N(ADC_CHANNEL_IN12) },
  { GPIOC, ADC_SQR3_SQ1_N(ADC_CHANNEL_IN14) },
  { GPIOC, ADC_SQR3_SQ1_N(ADC_CHANNEL_IN15) },
  { GPIOA, ADC_SQR3_SQ1_N(ADC_CHANNEL_IN1) },
  { GPIOA, ADC_SQR3_SQ1_N(ADC_CHANNEL_IN2) }
};

/*
 * ADC conversion group.
 * Mode:        Linear buffer, 4 samples of 2 channels, SW triggered.
 * Channels:    IN11   (48 cycles sample time)
 *              Sensor (192 cycles sample time)
 */
static ADCConversionGroup adcgrpcfg = {
  FALSE,
  1,
  NULL,
  NULL,
  /* HW dependent part.*/
  0,
  ADC_CR2_SWSTART,
  0,
  ADC_SMPR2_SMP_AN0(ADC_SAMPLE_15),
  ADC_SQR1_NUM_CH(1),
  0,
  0
};

//
// Sensor related.
//

// internal data buffers used for mailbox messages
static signal_t databuf[BUFFER_LEN];
// next buffer
static uint8_t nextbuf = 0;

static WORKING_AREA(wa_sensor_reader, 128);
static msg_t sensor_reader(void *arg) {
  (void)arg;

  // reset message box
  chMBReset(&mbox);
  nextbuf = 0;

  // ground and power pin index
  uint8_t gpi, ppi;

  while (!chThdShouldTerminate()) {
    // get next buffer
    signal_t *buf = databuf + nextbuf;

    // systime_t ticks = chTimeNow();

    // save current time, truncating to 32-bits
    // use formula to convert to seconds:
    //    sec = ticks / CH_FREQUENCY
    // where CH_FREQUENCY = 1000 is defined in chconf.h
    buf->time = chTimeNow() / (CH_FREQUENCY / 1000);

    // reset checksum
    buf->checksum = buf->time;

#if 1
    // sample sensors
    for (gpi = 0; gpi < GROUND_SIZE; gpi++) {
      const uint8_t gp = ground_pins[gpi];

      // chprintf((BaseSequentialStream *)&SDU1,
      //   "Setting GPIOD %d to PAL_MODE_INPUT_PULLDOWN mode\r\n", gp);

      // enable ground strip
      // palClearPad(GPIOD, gp);
      palSetPadMode(GPIOD, gp, PAL_MODE_INPUT_PULLDOWN);

      for (ppi = 0; ppi < POWER_SIZE; ppi++) {
        const uint8_t pp = power_pins[ppi];
        const adc_portmap_t *pm = adc_pins + ppi;

        // chprintf((BaseSequentialStream *)&SDU1,
        //   "Setting GPIOB %d to PAL_MODE_INPUT_PULLUP mode\r\n", pp);

        // enable power strip
        palSetPadMode(GPIOB, pp, PAL_MODE_INPUT_PULLUP);

        // assume that 15 ticks is long enough for ADC to stabilize

        // delay between sampling grid points to prevent artifacts
        // https://en.wikipedia.org/wiki/Propagation_delay
        // p = d / s = 0.2m / .59c = 0.2 / 176877550 ~= 11.3e-9
        // safe to delay less than 1 microsecond
        // chThdSleepMicroseconds(1);

        // sample voltage
        adcgrpcfg.sqr3 = pm->channel;
        adcConvert(&ADCD2, &adcgrpcfg,
          (adcsample_t *)((&buf->values) + (GROUND_SIZE * gpi + ppi)), 1);

        // chprintf((BaseSequentialStream *)&SDU1,
        //   "Setting GPIOB %d to PAL_MODE_INPUT mode\r\n", pp);

        // disable power strip
        palSetPadMode(GPIOB, pp, PAL_MODE_INPUT);
      }

      // chprintf((BaseSequentialStream *)&SDU1,
      //   "Setting GPIOD %d to PAL_MODE_INPUT mode\r\n", gp);

      // disable ground strip
      // palSetPad(GPIOD, gp);
      palSetPadMode(GPIOD, gp, PAL_MODE_INPUT);
    }
#else
  do {
    systime_t start;
    int i;

    // enable ground strip
    start = chTimeNow();
    for (i = 0; i < 1000; i++) {
      // palClearPad(GPIOD, ground_pins[0]);
      palSetPadMode(GPIOD, ground_pins[0], PAL_MODE_INPUT_PULLDOWN);
    }
    chprintf((BaseSequentialStream *)&SDU1,
      "enable ground strip: %d\r\n",
      chTimeElapsedSince(start) / MS2ST(1));

    // enable power strip
    start = chTimeNow();
    for (i = 0; i < 1000; i++) {
      palSetPadMode(GPIOB, power_pins[0], PAL_MODE_INPUT_PULLUP);
    }
    chprintf((BaseSequentialStream *)&SDU1,
      "enable power strip: %d\r\n",
      chTimeElapsedSince(start) / MS2ST(1));

    // delay
    start = chTimeNow();
    for (i = 0; i < 1000; i++) {
      chThdSleepMicroseconds(1);
    }
    chprintf((BaseSequentialStream *)&SDU1,
      "delay: %d\r\n",
      chTimeElapsedSince(start) / MS2ST(1));

    // sample voltage
    start = chTimeNow();
    for (i = 0; i < 1000; i++) {
      const adc_portmap_t *pm = adc_pins;
      adcgrpcfg.sqr3 = pm->channel;
      adcConvert(pm->adcp, &adcgrpcfg,
        (adcsample_t *)((&buf->values) + (GROUND_SIZE * 0 + 0)), 1);
    }
    chprintf((BaseSequentialStream *)&SDU1,
      "sample voltage: %d\r\n",
      chTimeElapsedSince(start) / MS2ST(1));

    // disable power strip
    start = chTimeNow();
    for (i = 0; i < 1000; i++) {
      palSetPadMode(GPIOB, power_pins[0], PAL_MODE_INPUT);
    }
    chprintf((BaseSequentialStream *)&SDU1,
      "disable power strip: %d\r\n",
      chTimeElapsedSince(start) / MS2ST(1));

    // disable ground strip
    start = chTimeNow();
    for (i = 0; i < 1000; i++) {
      // palSetPad(GPIOD, ground_pins[gpi]);
      palSetPadMode(GPIOD, ground_pins[gpi], PAL_MODE_INPUT);
    }
    chprintf((BaseSequentialStream *)&SDU1,
      "disable ground strip: %d\r\n",
      chTimeElapsedSince(start) / MS2ST(1));

  } while(FALSE);
#endif

    // chprintf((BaseSequentialStream *)&SDU1, "posting...\r\n");

    // systime_t start = chTimeNow();

    // send data
    if (chMBPost(&mbox, nextbuf, TIME_IMMEDIATE) == RDY_OK) {
      // advance buffer offset if posted, otherwise skip this sample
      nextbuf = (nextbuf + 1) % BUFFER_LEN;
    }

    // chprintf((BaseSequentialStream *)&SDU1,
    //   "post: %d\r\n",
    //   chTimeElapsedSince(start) / MS2ST(1));

    // chprintf((BaseSequentialStream *)&SDU1,
    //   "ticks = %d, elapsed = %d, conversion = %d\r\nsleep = %d\r\n\r\n",
    //   ticks,
    //   chTimeElapsedSince(ticks),
    //   S2ST(1),
    //   chTimeNow() - (chTimeNow() % MS2ST(10)) + MS2ST(10));

    // sleep until next sampling time / 10 ms = at 100 Hz
    chThdSleepUntil(chTimeNow() - (chTimeNow() % MS2ST(10)) + MS2ST(10));
  }

  return RDY_OK;
}

//
// Serial related.
//

static WORKING_AREA(wa_serial_writer, 128);
static msg_t serial_writer(void *p) {
  BaseSequentialStream *chp = (BaseSequentialStream *)p;
  chRegSetThreadName("serial_writer");

  while (!chThdShouldTerminate()) {
    msg_t msgp;

    if (chMBFetch(&mbox, &msgp, TIME_INFINITE) != RDY_OK) {
      chprintf(chp, "No data!\r\n");
      break;
    }

    // chprintf(chp, "Got data! %d / %d\r\n", msgp, chMBGetUsedCountI(&mbox));

    signal_t *buf = databuf + msgp;
    // chprintf(chp, "%x\r\n", *buf);

    chSequentialStreamWrite(chp, (uint8_t *)buf, sizeof(*buf));
    chSchRescheduleS();
  }

  return RDY_OK;
}

/*===========================================================================*/
/* Initialization and main thread.                                           */
/*===========================================================================*/

static inline void manage_thread_on_usb_active(
  Thread **tp, void *wsp, size_t size, tprio_t prio, tfunc_t pf, void *arg
) {
  // existing thread
  if (*tp) {
    if (chThdTerminated(*tp)) {
      // clear thread pointer if terminated
      *tp = NULL;
    } else if (SDU1.config->usbp->state != USB_ACTIVE) {
      // terminate if USB not active
      chThdTerminate(*tp);
    }
  }
  // (re)spawn thread on USB active
  if (!*tp && SDU1.config->usbp->state == USB_ACTIVE) {
    // create static thread on USB active
    *tp = chThdCreateStatic(wsp, size, prio, pf, arg);
  }
}

/*
 * Application entry point.
 */
int main(void) {
  Thread *sensor_reader_tp = NULL;
  Thread *serial_writer_tp = NULL;

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Initializes a serial-over-USB CDC driver.
   */
  sduObjectInit(&SDU1);
  sduStart(&SDU1, &serusbcfg);

  /*
   * Activates the USB driver and then the USB bus pull-up on D+.
   * Note, a delay is inserted in order to not have to disconnect the cable
   * after a reset.
   */
  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(1000);
  usbStart(serusbcfg.usbp, &usbcfg);
  usbConnectBus(serusbcfg.usbp);

  // enable ADC
  // adcStart(&ADCD1, NULL);
  adcStart(&ADCD2, NULL);
  // adcStart(&ADCD3, NULL);

  // enable ADC ref voltage
  adcSTM32EnableTSVREFE();

  // set ADC pins to analog input
  // PA 1, 2
  palSetGroupMode(GPIOA, 0b11, 1, PAL_MODE_INPUT_ANALOG);
  // PB 0, 1
  palSetGroupMode(GPIOB, 0b11, 0, PAL_MODE_INPUT_ANALOG);
  // PC 1, 2, 4, 5
  palSetGroupMode(GPIOC, 0b11011, 1, PAL_MODE_INPUT_ANALOG);

  // set ground pins to open drain output with high-Z output
  // PD 0, 1, 2, 3, 6, 7, 8, 9
  // palSetGroupMode(GPIOD, 0b0000001111001111, 0, PAL_MODE_OUTPUT_OPENDRAIN);
  // palWriteGroup(GPIOD, 0b0000001111001111, 0, 0xffff);

  // set ground pins to high-Z mode
  // PD 0, 1, 2, 3, 6, 7, 8, 9
  palSetGroupMode(GPIOD, 0b0000001111001111, 0, PAL_MODE_INPUT);
  // set power pins to high-Z mode
  // PB 4, 5, 7, 8, 11, 12, 13, 14
  palSetGroupMode(GPIOB, 0b0111100110110000, 0, PAL_MODE_INPUT);

  // start threads when connected
  while (TRUE) {
    // sensor reader
    manage_thread_on_usb_active(
      &sensor_reader_tp, wa_sensor_reader, sizeof(wa_sensor_reader),
      HIGHPRIO, sensor_reader, NULL);
    // serial writer
    manage_thread_on_usb_active(
      &serial_writer_tp, wa_serial_writer, sizeof(wa_serial_writer),
      NORMALPRIO, serial_writer, (void *)&SDU1);
    // sleep!
    chThdSleepMilliseconds(500);
  }

  return 0;
}
