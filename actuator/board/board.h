/*

Cuddlemaster - Copyright (C) 2014 Michael Phan-Ba

Property of SPIN Research Group
ICICS/CS Building X508-2366 Main Mall
Vancouver, B.C. V6T 1Z4 Canada
(604) 822 8169 - maclean@cs.ubc.ca

----

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

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * Setup for STMicroelectronics STM32F4-Discovery board.
 */

/*
 * Board identifier.
 */
#define BOARD_ST_STM32F4_CUDDLEBOT
#define BOARD_NAME                  "Cuddlebot Actuator Board"


/*
 * Board oscillators-related settings.
 * NOTE: LSE not fitted.
 */
#if !defined(STM32_LSECLK)
#define STM32_LSECLK                0
#endif

#if !defined(STM32_HSECLK)
#define STM32_HSECLK                20000000
#endif

/*
 * Board voltages.
 * Required for performance limits calculation.
 */
#define STM32_VDD                   330

/*
 * MCU type as defined in the ST header.
 */
#define STM32F40_41xxx

/*
 * IO pins assignments.
 */
#define GPIOA_AIO0                  0
#define GPIOA_AIO1                  1
#define GPIOA_AIO2                  2
#define GPIOA_AIO3                  3
#define GPIOA_AIO4                  4
#define GPIOA_AIO5                  5
#define GPIOA_AIO6                  6
#define GPIOA_AIO7                  7
#define GPIOA_TIM1_CH1              8
#define GPIOA_TIM1_CH2              9
#define GPIOA_PIN10                 10 // unused
#define GPIOA_PIN11                 11 // unused
#define GPIOA_PIN12                 12 // unused
#define GPIOA_SWDIO                 13
#define GPIOA_SWCLK                 14
#define GPIOA_PIN15                 15 // unused

#define GPIOB_IVAL                  0
#define GPIOB_VREF                  1
#define GPIOB_PIN2                  2 // unused
#define GPIOB_SWO                   3
#define GPIOB_PIN4                  4 // unused
#define GPIOB_MOTOR_EN              5
#define GPIOB_POS_NEN               6
#define GPIOB_POS_TCCEN             7
#define GPIOB_PIN8                  8 // unused
#define GPIOB_PIN9                  9 // unused
#define GPIOB_USART3_TX             10
#define GPIOB_USART3_RX             11
#define GPIOB_LED1                  12
#define GPIOB_LED2                  13
#define GPIOB_RS485_TXEN            14
#define GPIOB_PIN15                 15 // unused

#define GPIOC_TORQUE                0
#define GPIOC_TEMP                  1
#define GPIOC_POS_SIN               2
#define GPIOC_POS_COS               3
#define GPIOC_VIO4                  4
#define GPIOC_VIO5                  5
#define GPIOC_VIO6                  6
#define GPIOC_VIO7                  7
#define GPIOC_VIO3                  8
#define GPIOC_VIO2                  9
#define GPIOC_VIO1                  10
#define GPIOC_VIO0                  11
#define GPIOC_PIN12                 12 // unused
#define GPIOC_ADDR0                 13
#define GPIOC_ADDR1                 14
#define GPIOC_ADDROUT               15

#define GPIOD_PIN2                  2 // unused

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 * Please refer to the STM32 Reference Manual for details.
 */
#define PIN_MODE_INPUT(n)           (0U << ((n) * 2))
#define PIN_MODE_OUTPUT(n)          (1U << ((n) * 2))
#define PIN_MODE_ALTERNATE(n)       (2U << ((n) * 2))
#define PIN_MODE_ANALOG(n)          (3U << ((n) * 2))
#define PIN_ODR_LOW(n)              (0U << (n))
#define PIN_ODR_HIGH(n)             (1U << (n))
#define PIN_OTYPE_PUSHPULL(n)       (0U << (n))
#define PIN_OTYPE_OPENDRAIN(n)      (1U << (n))
#define PIN_OSPEED_2M(n)            (0U << ((n) * 2))
#define PIN_OSPEED_25M(n)           (1U << ((n) * 2))
#define PIN_OSPEED_50M(n)           (2U << ((n) * 2))
#define PIN_OSPEED_100M(n)          (3U << ((n) * 2))
#define PIN_PUPDR_FLOATING(n)       (0U << ((n) * 2))
#define PIN_PUPDR_PULLUP(n)         (1U << ((n) * 2))
#define PIN_PUPDR_PULLDOWN(n)       (2U << ((n) * 2))
#define PIN_AFIO_AF(n, v)           ((v##U) << ((n % 8) * 4))

/*
 * GPIOA setup:
 *
 * PA0  - AIO0                      (input floating).
 * PA1  - AIO1                      (input floating).
 * PA2  - AIO2                      (input floating).
 * PA3  - AIO3                      (input floating).
 * PA4  - AIO4                      (input floating).
 * PA5  - AIO5                      (input floating).
 * PA6  - AIO6                      (input floating).
 * PA7  - AIO7                      (input floating).
 * PA8  - TIM1_CH1                  (alternate 1).
 * PA9  - TIM1_CH2                  (alternate 1).
 * PA10 - N/A                       (input floating).
 * PA11 - N/A                       (input floating).
 * PA12 - N/A                       (input floating).
 * PA13 - SWDIO                     (alternate 0).
 * PA14 - SWCLK                     (alternate 0).
 * PA15 - N/A                       (input floating).
 */
#define VAL_GPIOA_MODER             (PIN_MODE_INPUT(GPIOA_AIO0) |           \
                                     PIN_MODE_INPUT(GPIOA_AIO1) |           \
                                     PIN_MODE_INPUT(GPIOA_AIO2) |           \
                                     PIN_MODE_INPUT(GPIOA_AIO3) |           \
                                     PIN_MODE_INPUT(GPIOA_AIO4) |           \
                                     PIN_MODE_INPUT(GPIOA_AIO5) |           \
                                     PIN_MODE_INPUT(GPIOA_AIO6) |           \
                                     PIN_MODE_INPUT(GPIOA_AIO7) |           \
                                     PIN_MODE_ALTERNATE(GPIOA_TIM1_CH1) |   \
                                     PIN_MODE_ALTERNATE(GPIOA_TIM1_CH2) |   \
                                     PIN_MODE_INPUT(GPIOA_PIN10) |          \
                                     PIN_MODE_INPUT(GPIOA_PIN11) |          \
                                     PIN_MODE_INPUT(GPIOA_PIN12) |          \
                                     PIN_MODE_ALTERNATE(GPIOA_SWDIO) |      \
                                     PIN_MODE_ALTERNATE(GPIOA_SWCLK) |      \
                                     PIN_MODE_INPUT(GPIOA_PIN15))
#define VAL_GPIOA_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOA_AIO0) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_AIO1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_AIO2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_AIO3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_AIO4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_AIO5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_AIO6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_AIO7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOA_TIM1_CH1) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOA_TIM1_CH2) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN10) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN11) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_SWDIO) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_SWCLK) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOA_PIN15))
#define VAL_GPIOA_OSPEEDR           (PIN_OSPEED_100M(GPIOA_AIO0) |          \
                                     PIN_OSPEED_100M(GPIOA_AIO1) |          \
                                     PIN_OSPEED_100M(GPIOA_AIO2) |          \
                                     PIN_OSPEED_100M(GPIOA_AIO3) |          \
                                     PIN_OSPEED_100M(GPIOA_AIO4) |          \
                                     PIN_OSPEED_100M(GPIOA_AIO5) |          \
                                     PIN_OSPEED_100M(GPIOA_AIO6) |          \
                                     PIN_OSPEED_100M(GPIOA_AIO7) |          \
                                     PIN_OSPEED_100M(GPIOA_TIM1_CH1) |      \
                                     PIN_OSPEED_100M(GPIOA_TIM1_CH2) |      \
                                     PIN_OSPEED_2M(GPIOA_PIN10) |           \
                                     PIN_OSPEED_2M(GPIOA_PIN11) |           \
                                     PIN_OSPEED_2M(GPIOA_PIN12) |           \
                                     PIN_OSPEED_100M(GPIOA_SWDIO) |         \
                                     PIN_OSPEED_100M(GPIOA_SWCLK) |         \
                                     PIN_OSPEED_2M(GPIOA_PIN15))
#define VAL_GPIOA_PUPDR             (PIN_PUPDR_FLOATING(GPIOA_AIO0) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_AIO1) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_AIO2) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_AIO3) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_AIO4) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_AIO5) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_AIO6) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_AIO7) |       \
                                     PIN_PUPDR_FLOATING(GPIOA_TIM1_CH1) |   \
                                     PIN_PUPDR_FLOATING(GPIOA_TIM1_CH2) |   \
                                     PIN_PUPDR_FLOATING(GPIOA_PIN10) |      \
                                     PIN_PUPDR_FLOATING(GPIOA_PIN11) |      \
                                     PIN_PUPDR_FLOATING(GPIOA_PIN12) |      \
                                     PIN_PUPDR_PULLUP(GPIOA_SWDIO) |        \
                                     PIN_PUPDR_PULLDOWN(GPIOA_SWCLK) |      \
                                     PIN_PUPDR_FLOATING(GPIOA_PIN15))
#define VAL_GPIOA_ODR               (PIN_ODR_LOW(GPIOA_AIO0) |              \
                                     PIN_ODR_LOW(GPIOA_AIO1) |              \
                                     PIN_ODR_LOW(GPIOA_AIO2) |              \
                                     PIN_ODR_LOW(GPIOA_AIO3) |              \
                                     PIN_ODR_LOW(GPIOA_AIO4) |              \
                                     PIN_ODR_LOW(GPIOA_AIO5) |              \
                                     PIN_ODR_LOW(GPIOA_AIO6) |              \
                                     PIN_ODR_LOW(GPIOA_AIO7) |              \
                                     PIN_ODR_LOW(GPIOA_TIM1_CH1) |          \
                                     PIN_ODR_LOW(GPIOA_TIM1_CH2) |          \
                                     PIN_ODR_LOW(GPIOA_PIN10) |             \
                                     PIN_ODR_LOW(GPIOA_PIN11) |             \
                                     PIN_ODR_LOW(GPIOA_PIN12) |             \
                                     PIN_ODR_LOW(GPIOA_SWDIO) |             \
                                     PIN_ODR_LOW(GPIOA_SWCLK) |             \
                                     PIN_ODR_LOW(GPIOA_PIN15))
#define VAL_GPIOA_AFRL              (PIN_AFIO_AF(GPIOA_AIO0, 0) |           \
                                     PIN_AFIO_AF(GPIOA_AIO1, 0) |           \
                                     PIN_AFIO_AF(GPIOA_AIO2, 0) |           \
                                     PIN_AFIO_AF(GPIOA_AIO3, 0) |           \
                                     PIN_AFIO_AF(GPIOA_AIO4, 0) |           \
                                     PIN_AFIO_AF(GPIOA_AIO5, 0) |           \
                                     PIN_AFIO_AF(GPIOA_AIO6, 0) |           \
                                     PIN_AFIO_AF(GPIOA_AIO7, 0))
#define VAL_GPIOA_AFRH              (PIN_AFIO_AF(GPIOA_TIM1_CH1, 1) |       \
                                     PIN_AFIO_AF(GPIOA_TIM1_CH2, 1) |       \
                                     PIN_AFIO_AF(GPIOA_PIN10, 0) |          \
                                     PIN_AFIO_AF(GPIOA_PIN11, 0) |          \
                                     PIN_AFIO_AF(GPIOA_PIN12, 0) |          \
                                     PIN_AFIO_AF(GPIOA_SWDIO, 0) |          \
                                     PIN_AFIO_AF(GPIOA_SWCLK, 0) |          \
                                     PIN_AFIO_AF(GPIOA_PIN15, 0))

/*
 * GPIOB setup:
 *
 * PB0  - IVAL                      (input analog).
 * PB1  - VREF                      (input analog).
 * PB2  - N/A                       (input floating).
 * PB3  - SWO                       (alternate 0).
 * PB4  - N/A                       (input floating).
 * PB5  - MOTOR_EN                  (output pushpull).
 * PB6  - POS_NEN                   (output pushpull).
 * PB7  - POS_TCCEN                 (input pullup).
 * PB8  - N/A                       (input floating).
 * PB9  - N/A                       (input floating).
 * PB10 - USART3_TX                 (alternate 7).
 * PB11 - USART3_RX                 (alternate 7).
 * PB12 - LED1                      (input floating).
 * PB13 - LED2                      (input floating).
 * PB14 - USART3_TXEN               (output pushpull).
 * PB15 - N/A                       (input floating).
 */
#define VAL_GPIOB_MODER             (PIN_MODE_ANALOG(GPIOB_IVAL) |          \
                                     PIN_MODE_ANALOG(GPIOB_VREF) |          \
                                     PIN_MODE_INPUT(GPIOB_PIN2) |           \
                                     PIN_MODE_ALTERNATE(GPIOB_SWO) |        \
                                     PIN_MODE_INPUT(GPIOB_PIN4) |           \
                                     PIN_MODE_OUTPUT(GPIOB_MOTOR_EN) |      \
                                     PIN_MODE_OUTPUT(GPIOB_POS_NEN) |       \
                                     PIN_MODE_INPUT(GPIOB_POS_TCCEN) |      \
                                     PIN_MODE_INPUT(GPIOB_PIN8) |           \
                                     PIN_MODE_INPUT(GPIOB_PIN9) |           \
                                     PIN_MODE_ALTERNATE(GPIOB_USART3_TX) |  \
                                     PIN_MODE_ALTERNATE(GPIOB_USART3_RX) |  \
                                     PIN_MODE_INPUT(GPIOB_LED1) |           \
                                     PIN_MODE_INPUT(GPIOB_LED2) |           \
                                     PIN_MODE_OUTPUT(GPIOB_RS485_TXEN) |    \
                                     PIN_MODE_INPUT(GPIOB_PIN15))
#define VAL_GPIOB_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOB_IVAL) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_VREF) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_SWO) |        \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_MOTOR_EN) |   \
                                     PIN_OTYPE_PUSHPULL(GPIOB_POS_NEN) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOB_POS_TCCEN) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN8) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN9) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_USART3_TX) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOB_USART3_RX) |  \
                                     PIN_OTYPE_PUSHPULL(GPIOB_LED1) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_LED2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOB_RS485_TXEN) | \
                                     PIN_OTYPE_PUSHPULL(GPIOB_PIN15))
#define VAL_GPIOB_OSPEEDR           (PIN_OSPEED_100M(GPIOB_IVAL) |          \
                                     PIN_OSPEED_100M(GPIOB_VREF) |          \
                                     PIN_OSPEED_100M(GPIOB_PIN2) |          \
                                     PIN_OSPEED_100M(GPIOB_SWO) |           \
                                     PIN_OSPEED_100M(GPIOB_PIN4) |          \
                                     PIN_OSPEED_100M(GPIOB_MOTOR_EN) |      \
                                     PIN_OSPEED_100M(GPIOB_POS_NEN) |       \
                                     PIN_OSPEED_100M(GPIOB_POS_TCCEN) |     \
                                     PIN_OSPEED_100M(GPIOB_PIN8) |          \
                                     PIN_OSPEED_100M(GPIOB_PIN9) |          \
                                     PIN_OSPEED_100M(GPIOB_USART3_TX) |     \
                                     PIN_OSPEED_100M(GPIOB_USART3_RX) |     \
                                     PIN_OSPEED_100M(GPIOB_LED1) |          \
                                     PIN_OSPEED_100M(GPIOB_LED2) |          \
                                     PIN_OSPEED_100M(GPIOB_RS485_TXEN) |    \
                                     PIN_OSPEED_100M(GPIOB_PIN15))
#define VAL_GPIOB_PUPDR             (PIN_PUPDR_FLOATING(GPIOB_IVAL) |       \
                                     PIN_PUPDR_FLOATING(GPIOB_VREF) |       \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN2) |       \
                                     PIN_PUPDR_FLOATING(GPIOB_SWO) |        \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN4) |       \
                                     PIN_PUPDR_FLOATING(GPIOB_MOTOR_EN) |   \
                                     PIN_PUPDR_FLOATING(GPIOB_POS_NEN) |    \
                                     PIN_PUPDR_PULLUP(GPIOB_POS_TCCEN) |    \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN8) |       \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN9) |       \
                                     PIN_PUPDR_FLOATING(GPIOB_USART3_TX) |  \
                                     PIN_PUPDR_FLOATING(GPIOB_USART3_RX) |  \
                                     PIN_PUPDR_FLOATING(GPIOB_LED1) |       \
                                     PIN_PUPDR_FLOATING(GPIOB_LED2) |       \
                                     PIN_PUPDR_FLOATING(GPIOB_RS485_TXEN) | \
                                     PIN_PUPDR_FLOATING(GPIOB_PIN15))
#define VAL_GPIOB_ODR               (PIN_ODR_LOW(GPIOB_IVAL) |              \
                                     PIN_ODR_LOW(GPIOB_VREF) |              \
                                     PIN_ODR_LOW(GPIOB_PIN2) |              \
                                     PIN_ODR_LOW(GPIOB_SWO) |               \
                                     PIN_ODR_LOW(GPIOB_PIN4) |              \
                                     PIN_ODR_LOW(GPIOB_MOTOR_EN) |          \
                                     PIN_ODR_HIGH(GPIOB_POS_NEN) |          \
                                     PIN_ODR_LOW(GPIOB_POS_TCCEN) |         \
                                     PIN_ODR_LOW(GPIOB_PIN8) |              \
                                     PIN_ODR_LOW(GPIOB_PIN9) |              \
                                     PIN_ODR_LOW(GPIOB_USART3_TX) |         \
                                     PIN_ODR_LOW(GPIOB_USART3_RX) |         \
                                     PIN_ODR_LOW(GPIOB_LED1) |              \
                                     PIN_ODR_LOW(GPIOB_LED2) |              \
                                     PIN_ODR_LOW(GPIOB_RS485_TXEN) |        \
                                     PIN_ODR_LOW(GPIOB_PIN15))
#define VAL_GPIOB_AFRL              (PIN_AFIO_AF(GPIOB_IVAL, 0) |           \
                                     PIN_AFIO_AF(GPIOB_VREF, 0) |           \
                                     PIN_AFIO_AF(GPIOB_PIN2, 0) |           \
                                     PIN_AFIO_AF(GPIOB_SWO, 0) |            \
                                     PIN_AFIO_AF(GPIOB_PIN4, 0) |           \
                                     PIN_AFIO_AF(GPIOB_MOTOR_EN, 0) |       \
                                     PIN_AFIO_AF(GPIOB_POS_NEN, 0) |        \
                                     PIN_AFIO_AF(GPIOB_POS_TCCEN, 0))
#define VAL_GPIOB_AFRH              (PIN_AFIO_AF(GPIOB_PIN8, 0) |           \
                                     PIN_AFIO_AF(GPIOB_PIN9, 0) |           \
                                     PIN_AFIO_AF(GPIOB_USART3_TX, 7) |      \
                                     PIN_AFIO_AF(GPIOB_USART3_RX, 7) |      \
                                     PIN_AFIO_AF(GPIOB_LED1, 0) |           \
                                     PIN_AFIO_AF(GPIOB_LED2, 0) |           \
                                     PIN_AFIO_AF(GPIOB_RS485_TXEN, 0) |     \
                                     PIN_AFIO_AF(GPIOB_PIN15, 0))

/*
 * GPIOC setup:
 *
 * PC0  - TORQUE                    (input analog).
 * PC1  - TEMP                      (input analog).
 * PC2  - POS_SIN                   (input analog).
 * PC3  - POS_COS                   (input analog).
 * PC4  - VIO4                      (input floating).
 * PC5  - VIO5                      (input floating).
 * PC6  - VIO6                      (input floating).
 * PC7  - VIO7                      (input floating).
 * PC8  - VIO3                      (input floating).
 * PC9  - VIO2                      (input floating).
 * PC10 - VIO1                      (input floating).
 * PC11 - VIO0                      (input floating).
 * PC12 - N/A                       (input floating).
 * PC13 - ADDR0                     (input floating).
 * PC14 - ADDR1                     (input floating).
 * PC15 - ADDROUT                   (input floating).
 */
#define VAL_GPIOC_MODER             (PIN_MODE_ANALOG(GPIOC_TORQUE) |        \
                                     PIN_MODE_ANALOG(GPIOC_TEMP) |          \
                                     PIN_MODE_ANALOG(GPIOC_POS_SIN) |       \
                                     PIN_MODE_ANALOG(GPIOC_POS_COS) |       \
                                     PIN_MODE_INPUT(GPIOC_VIO4) |           \
                                     PIN_MODE_INPUT(GPIOC_VIO5) |           \
                                     PIN_MODE_INPUT(GPIOC_VIO6) |           \
                                     PIN_MODE_INPUT(GPIOC_VIO7) |           \
                                     PIN_MODE_INPUT(GPIOC_VIO3) |           \
                                     PIN_MODE_INPUT(GPIOC_VIO2) |           \
                                     PIN_MODE_INPUT(GPIOC_VIO1)  |          \
                                     PIN_MODE_INPUT(GPIOC_VIO0)  |          \
                                     PIN_MODE_INPUT(GPIOC_PIN12) |          \
                                     PIN_MODE_INPUT(GPIOC_ADDR0)  |         \
                                     PIN_MODE_INPUT(GPIOC_ADDR1)  |         \
                                     PIN_MODE_INPUT(GPIOC_ADDROUT))
#define VAL_GPIOC_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOC_TORQUE) |     \
                                     PIN_OTYPE_PUSHPULL(GPIOC_TEMP) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_POS_SIN) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOC_POS_COS) |    \
                                     PIN_OTYPE_PUSHPULL(GPIOC_VIO4) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_VIO5) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_VIO6) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_VIO7) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_VIO3) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_VIO2) |       \
                                     PIN_OTYPE_PUSHPULL(GPIOC_VIO1)  |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_VIO0)  |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_PIN12) |      \
                                     PIN_OTYPE_PUSHPULL(GPIOC_ADDR0)  |     \
                                     PIN_OTYPE_PUSHPULL(GPIOC_ADDR1)  |     \
                                     PIN_OTYPE_PUSHPULL(GPIOC_ADDROUT))
#define VAL_GPIOC_OSPEEDR           (PIN_OSPEED_100M(GPIOC_TORQUE) |        \
                                     PIN_OSPEED_100M(GPIOC_TEMP) |          \
                                     PIN_OSPEED_100M(GPIOC_POS_SIN) |       \
                                     PIN_OSPEED_100M(GPIOC_POS_COS) |       \
                                     PIN_OSPEED_100M(GPIOC_VIO4) |          \
                                     PIN_OSPEED_100M(GPIOC_VIO5) |          \
                                     PIN_OSPEED_100M(GPIOC_VIO6) |          \
                                     PIN_OSPEED_100M(GPIOC_VIO7) |          \
                                     PIN_OSPEED_100M(GPIOC_VIO3) |          \
                                     PIN_OSPEED_100M(GPIOC_VIO2) |          \
                                     PIN_OSPEED_100M(GPIOC_VIO1)  |         \
                                     PIN_OSPEED_100M(GPIOC_VIO0)  |         \
                                     PIN_OSPEED_2M(GPIOC_PIN12) |           \
                                     PIN_OSPEED_100M(GPIOC_ADDR0)  |        \
                                     PIN_OSPEED_100M(GPIOC_ADDR1)  |        \
                                     PIN_OSPEED_100M(GPIOC_ADDROUT))
#define VAL_GPIOC_PUPDR             (PIN_PUPDR_FLOATING(GPIOC_TORQUE) |     \
                                     PIN_PUPDR_FLOATING(GPIOC_TEMP) |       \
                                     PIN_PUPDR_FLOATING(GPIOC_POS_SIN) |    \
                                     PIN_PUPDR_FLOATING(GPIOC_POS_COS) |    \
                                     PIN_PUPDR_FLOATING(GPIOC_VIO4) |       \
                                     PIN_PUPDR_FLOATING(GPIOC_VIO5) |       \
                                     PIN_PUPDR_FLOATING(GPIOC_VIO6) |       \
                                     PIN_PUPDR_FLOATING(GPIOC_VIO7) |       \
                                     PIN_PUPDR_FLOATING(GPIOC_VIO3) |       \
                                     PIN_PUPDR_FLOATING(GPIOC_VIO2) |       \
                                     PIN_PUPDR_FLOATING(GPIOC_VIO1)  |      \
                                     PIN_PUPDR_FLOATING(GPIOC_VIO0)  |      \
                                     PIN_PUPDR_FLOATING(GPIOC_PIN12) |      \
                                     PIN_PUPDR_FLOATING(GPIOC_ADDR0)  |     \
                                     PIN_PUPDR_FLOATING(GPIOC_ADDR1)  |     \
                                     PIN_PUPDR_FLOATING(GPIOC_ADDROUT))
#define VAL_GPIOC_ODR               (PIN_ODR_LOW(GPIOC_TORQUE) |            \
                                     PIN_ODR_LOW(GPIOC_TEMP) |              \
                                     PIN_ODR_LOW(GPIOC_POS_SIN) |           \
                                     PIN_ODR_LOW(GPIOC_POS_COS) |           \
                                     PIN_ODR_LOW(GPIOC_VIO4) |              \
                                     PIN_ODR_LOW(GPIOC_VIO5) |              \
                                     PIN_ODR_LOW(GPIOC_VIO6) |              \
                                     PIN_ODR_LOW(GPIOC_VIO7) |              \
                                     PIN_ODR_LOW(GPIOC_VIO3) |              \
                                     PIN_ODR_LOW(GPIOC_VIO2) |              \
                                     PIN_ODR_LOW(GPIOC_VIO1)  |             \
                                     PIN_ODR_LOW(GPIOC_VIO0)  |             \
                                     PIN_ODR_LOW(GPIOC_PIN12) |             \
                                     PIN_ODR_LOW(GPIOC_ADDR0)  |            \
                                     PIN_ODR_LOW(GPIOC_ADDR1)  |            \
                                     PIN_ODR_LOW(GPIOC_ADDROUT))
#define VAL_GPIOC_AFRL              (PIN_AFIO_AF(GPIOC_TORQUE, 0) |         \
                                     PIN_AFIO_AF(GPIOC_TEMP, 0) |           \
                                     PIN_AFIO_AF(GPIOC_POS_SIN, 0) |        \
                                     PIN_AFIO_AF(GPIOC_POS_COS, 0) |        \
                                     PIN_AFIO_AF(GPIOC_VIO4, 0) |           \
                                     PIN_AFIO_AF(GPIOC_VIO5, 0) |           \
                                     PIN_AFIO_AF(GPIOC_VIO6, 0) |           \
                                     PIN_AFIO_AF(GPIOC_VIO7, 0))
#define VAL_GPIOC_AFRH              (PIN_AFIO_AF(GPIOC_VIO3, 0) |           \
                                     PIN_AFIO_AF(GPIOC_VIO2, 0) |           \
                                     PIN_AFIO_AF(GPIOC_VIO1 , 0) |          \
                                     PIN_AFIO_AF(GPIOC_VIO0 , 0) |          \
                                     PIN_AFIO_AF(GPIOC_PIN12, 0) |          \
                                     PIN_AFIO_AF(GPIOC_ADDR0 , 0) |         \
                                     PIN_AFIO_AF(GPIOC_ADDR1 , 0) |         \
                                     PIN_AFIO_AF(GPIOC_ADDROUT , 0))

/*
 * GPIOD setup:
 *
 * PC2  - N/A                       (input floating).
 */
#define VAL_GPIOD_MODER             (PIN_MODE_INPUT(GPIOD_PIN2))
#define VAL_GPIOD_OTYPER            (PIN_OTYPE_PUSHPULL(GPIOD_PIN2))
#define VAL_GPIOD_OSPEEDR           (PIN_OSPEED_2M(GPIOD_PIN2))
#define VAL_GPIOD_PUPDR             (PIN_PUPDR_FLOATING(GPIOD_PIN2))
#define VAL_GPIOD_ODR               (PIN_ODR_LOW(GPIOD_PIN2))
#define VAL_GPIOD_AFRL              (PIN_AFIO_AF(GPIOD_PIN2, 0))
#define VAL_GPIOD_AFRH              (0)

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */
