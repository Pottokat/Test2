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


// Flash green led on GPIO25
#define GPIO_LED 25

#define LCDMODE_ILI9341	1	/* 320*240 SF-TC240T-9370-T с контроллером ILI9341 - STM32F4DISCO */

#define BOARD_ILI9341_GPIO_MOSI		19
#define BOARD_ILI9341_GPIO_MISO		16
#define BOARD_ILI9341_GPIO_SCLK		18

#define BOARD_ILI9341_GPIO_RESET	17
#define BOARD_ILI9341_GPIO_CD		28
#define BOARD_ILI9341_GPIO_CSN		22

#if WITHUART1HW
	// See 2.19.2. Function Select
	// GPIO 28: UART0 TX
	// GPIO 29: UART0 RX
	#define HARDWARE_UART0_INITIALIZE() do { \
			gpio0_pin_initialize(28, GPIO_IO_TYPE_PERIPHERAL, GPIO_NOPULL, GPIO_SPEED_LOW, GPIO_AF_2); /* GPIO 28: UART0 TX */ \
			gpio0_pin_initialize(29, GPIO_IO_TYPE_PERIPHERAL, GPIO_NOPULL, GPIO_SPEED_LOW, GPIO_AF_2); /* GPIO 29: UART0 RX */ \
			} while (0)
#endif /* WITHUART1HW */

#if WITHUART2HW
	// See 2.19.2. Function Select
	// GPIO 20: UART1 TX
	// GPIO 21: UART1 RX
	#define HARDWARE_UART1_INITIALIZE() do { \
			gpio0_pin_initialize(20, GPIO_IO_TYPE_PERIPHERAL, GPIO_NOPULL, GPIO_SPEED_LOW, GPIO_AF_2); /* GPIO 20: UART1 TX */ \
			gpio0_pin_initialize(21, GPIO_IO_TYPE_PERIPHERAL, GPIO_NOPULL, GPIO_SPEED_LOW, GPIO_AF_2); /* GPIO 21: UART1 RX */ \
			} while (0)
#endif /* WITHUART2HW */

#if WITHSPIHW
	// See 2.19.2. Function Select
	// GPIO 18: SPI0 SCK
	// GPIO 19: SPI0 TX
	// GPIO 16: SPI0 RX
	#define HARDWARE_SPI0_INITIALIZE() do { \
			gpio0_pin_initialize(BOARD_ILI9341_GPIO_SCLK, GPIO_IO_TYPE_PERIPHERAL, GPIO_NOPULL, GPIO_SPEED_VERY_HIGH, GPIO_AF_1); /* GPIO 18: SPI0 SCK */ \
			gpio0_pin_initialize(BOARD_ILI9341_GPIO_MOSI, GPIO_IO_TYPE_PERIPHERAL, GPIO_NOPULL, GPIO_SPEED_VERY_HIGH, GPIO_AF_1); /* GPIO 19: SPI0 TX */ \
			gpio0_pin_initialize(BOARD_ILI9341_GPIO_MISO, GPIO_IO_TYPE_PERIPHERAL, GPIO_PULL_UP, GPIO_SPEED_VERY_HIGH, GPIO_AF_1); /* GPIO 16: SPI0 RX */ \
			} while (0)
#endif /* WITHSPIHW */

#define DISPLAY_BUS_INITIALIZE() do { \
		gpio0_pin_initialize(BOARD_ILI9341_GPIO_RESET, GPIO_IO_TYPE_OUTPUT, GPIO_NOPULL, GPIO_SPEED_LOW, GPIO_AF_NONE); \
		gpio0_pin_initialize(BOARD_ILI9341_GPIO_CD, GPIO_IO_TYPE_OUTPUT, GPIO_NOPULL, GPIO_SPEED_LOW, GPIO_AF_NONE); \
		gpio0_pin_initialize(BOARD_ILI9341_GPIO_CSN, GPIO_IO_TYPE_OUTPUT, GPIO_NOPULL, GPIO_SPEED_LOW, GPIO_AF_NONE); \
		gpio0_pin_setstate(BOARD_ILI9341_GPIO_CSN, 1); \
		gpio0_pin_setstate(BOARD_ILI9341_GPIO_RESET, 1); \
		} while (0)


#define targetlcd 0

#if LCDMODE_ILI9341
	#define LCDMODE_HARD_SPI	1	/* SPI interface */
	#define LCDMODE_SPI_RA	1	/* SPI only, with Reset, with Address */
	#define DIM_X 320
	#define DIM_Y 240
	#define LCDMODE_COLORED	1
	#define LCDMODE_RGB565 1
	#define LCDMODE_PIXELSIZE 2
#endif

#if DIM_X == 320 && DIM_Y == 240
	#define DSTYLE_G_X320_Y240	1
	#define CHAR_W	10
	#define CHAR_H	8
	#define SMALLCHARH 16 /* Font height */
	#define CHARS2GRID(columns) ((columns) * 1)		/* перевести количество символов в ячейкт сетки разметки отображния */
	#define ROWS2GRID(rows) ((rows) * 1)		/* перевести количество символов в ячейкт сетки разметки отображния */
	#define GRID2X(cellsx) ((cellsx) * CHAR_W)	/* перевод ячеек сетки разметки в номер пикселя по горизонталм */
	#define GRID2Y(cellsy) ((cellsy) * CHAR_H)	/* перевод ячеек сетки разметки в номер пикселя по вертикали */
#endif
void board_lcd_rs(uint_fast8_t v);	// выставить уровень на сигнале lcd register select - не требуется board_update
void board_lcd_reset(uint_fast8_t v);	// выставить уровень на сигнале lcd reset
void board_update(void);

#endif /* SRC_BOARD_H_ */
