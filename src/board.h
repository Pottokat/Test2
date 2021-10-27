/*
 * board.h
 *
 *  Created on: 25 окт. 2021 г.
 *      Author: User
 */

#ifndef SRC_BOARD_H_
#define SRC_BOARD_H_

void SysTick_Handler(void);



#define XOSC_VALUE   (12000000UL) /* Oscillator frequency */
#define GPIN0_VALUE	1
#define GPIN1_VALUE	1

// 12*100/3/3 = 133.333(3) MHz
#define PLL1_REFDIV 1	/* Reference clock frequency min=5MHz, max=800MHz */
#define PLL1_MUL 100	/* Feedback divider min=16, max=320, VCO frequency min=400MHz, max=1600MHz */
#define PLL1_DIV1 3
#define PLL1_DIV2 3

// 12*100/3/3 = 133.333(3) MHz
#define PLLUSB_REFDIV 1	/* Reference clock frequency min=5MHz, max=800MHz */
#define PLLUSB_MUL 100	/* Feedback divider min=16, max=320, VCO frequency min=400MHz, max=1600MHz */
#define PLLUSB_DIV1 3
#define PLLUSB_DIV2 3


#define BOARD_ILI9341_GPIO_MOSI		19
#define BOARD_ILI9341_GPIO_MISO		16
#define BOARD_ILI9341_GPIO_SCLK		18

#define BOARD_ILI9341_GPIO_RESET	17
#define BOARD_ILI9341_GPIO_CD		28
#define BOARD_ILI9341_GPIO_CSN		22

// See 2.19.2. Function Select
// GPIO 28: UART0 TX
// GPIO 29: UART0 RX
#define HARDWARE_UART0_INITIALIZE() do { \
		gpio0_pin_initialize(28, GPIO_IO_TYPE_PERIPHERAL, GPIO_NOPULL, GPIO_SPEED_LOW, GPIO_AF_2); /* GPIO 28: UART0 TX */ \
		gpio0_pin_initialize(29, GPIO_IO_TYPE_PERIPHERAL, GPIO_NOPULL, GPIO_SPEED_LOW, GPIO_AF_2); /* GPIO 29: UART0 RX */ \
		} while (0)

// See 2.19.2. Function Select
// GPIO 20: UART1 TX
// GPIO 21: UART1 RX
#define HARDWARE_UART1_INITIALIZE() do { \
		gpio0_pin_initialize(20, GPIO_IO_TYPE_PERIPHERAL, GPIO_NOPULL, GPIO_SPEED_LOW, GPIO_AF_2); /* GPIO 20: UART1 TX */ \
		gpio0_pin_initialize(21, GPIO_IO_TYPE_PERIPHERAL, GPIO_NOPULL, GPIO_SPEED_LOW, GPIO_AF_2); /* GPIO 21: UART1 RX */ \
		} while (0)

// See 2.19.2. Function Select
// GPIO 18: SPI0 SCK
// GPIO 19: SPI0 TX
// GPIO 16: SPI0 RX
#define HARDWARE_SPI0_INITIALIZE() do { \
		gpio0_pin_initialize(BOARD_ILI9341_GPIO_SCLK, GPIO_IO_TYPE_PERIPHERAL, GPIO_NOPULL, GPIO_SPEED_LOW, GPIO_AF_1); /* GPIO 18: SPI0 SCK */ \
		gpio0_pin_initialize(BOARD_ILI9341_GPIO_MOSI, GPIO_IO_TYPE_PERIPHERAL, GPIO_NOPULL, GPIO_SPEED_LOW, GPIO_AF_1); /* GPIO 19: SPI0 TX */ \
		gpio0_pin_initialize(BOARD_ILI9341_GPIO_MISO, GPIO_IO_TYPE_PERIPHERAL, GPIO_NOPULL, GPIO_SPEED_LOW, GPIO_AF_1); /* GPIO 16: SPI0 RX */ \
		} while (0)


#endif /* SRC_BOARD_H_ */
