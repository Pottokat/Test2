/*
 * main.c
 *
 *  Created on: 25 окт. 2021 г.
 *      Author: User
 */

#include "hardware.h"
#include "board.h"
#include "formats.h"
#include "spi.h"
#include "display/display.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void toggle(void)
{
	static int state;

	state = ! state;
	gpio0_pin_setstate(GPIO_LED, state);
}

void SysTick_Handler(void)
{
	static int n;

	if (++ n >= 5)
	{
		n = 0;
		toggle();
	}
}

static void idelay(int n)
{
	while (n --)
	{
		volatile unsigned t;
		for (t = 10000; t--; )
		{

		}
	}
}

void flashed(int pulses)
{
	for (;;)
	{
		int t = 100;
		int n = pulses;
		while (n --)
		{
			gpio0_pin_setstate(GPIO_LED, 1);
			idelay(t);
			gpio0_pin_setstate(GPIO_LED, 0);
			idelay(t);
		}
		idelay(t * 2);
	}
}

void board_update(void)
{

}

///////////////////////////

void board_lcd_rs(uint_fast8_t state)
{
	//PRINTF("board_lcd_rs: state=%d\n", state);
	gpio0_pin_setstate(BOARD_ILI9341_GPIO_CD, state);
	hardware_spi_io_delay();
}

void board_lcd_reset(uint_fast8_t state)
{
	//PRINTF("board_lcd_reset: state=%d\n", state);
	gpio0_pin_setstate(BOARD_ILI9341_GPIO_RESET, state);
	hardware_spi_io_delay();
}


// Вызовы этой функции (или группу вызовов) требуется "обрамить" парой вызовов
// display_wrdatabar_begin() и display_wrdatabar_end().
//
void
//NOINLINEAT
display_bar(
	uint_fast16_t xpix,
	uint_fast16_t ypix,
	uint_fast8_t width,	/* количество знакомест, занимаемых индикатором */
	uint_fast8_t value,		/* значение, которое надо отобразить */
	uint_fast8_t tracevalue,		/* значение маркера, которое надо отобразить */
	uint_fast8_t topvalue,	/* значение, соответствующее полностью заполненному индикатору */
	uint_fast8_t vpattern,	/* DISPLAY_BAR_HALF или DISPLAY_BAR_FULL */
	uint_fast8_t vpatternmax,	/* DISPLAY_BAR_HALF или DISPLAY_BAR_FULL - для отображения запомненного значения */
	uint_fast8_t vemptyp			/* паттерн для заполнения между штрихами */
	)
{
	//enum { DISPLAY_BAR_LEVELS = 6 };	// количество градаций в одном знакоместе

	//value = value < 0 ? 0 : value;
	const uint_fast16_t wfull = GRID2X(width);
	const uint_fast16_t wpart = (uint_fast32_t) wfull * value / topvalue;
	const uint_fast16_t wmark = (uint_fast32_t) wfull * tracevalue / topvalue;
	uint_fast8_t i = 0;

	for (; i < wpart; ++ i)
	{
		if (i == wmark)
		{
			xpix = display_barcolumn(xpix, ypix, vpatternmax);
			continue;
		}
#if (DSTYLE_G_X132_Y64 || DSTYLE_G_X128_Y64) && DSTYLE_UR3LMZMOD
		xpix = display_barcolumn(xpix, ypix, vpattern);
#elif DSTYLE_G_X64_Y32
		xpix = display_barcolumn(xpix, ypix, (i % 6) != 5 ? vpattern : vemptyp);
#else
		xpix = display_barcolumn(xpix, ypix, (i % 2) == 0 ? vpattern : PATTERN_SPACE);
#endif
	}

	for (; i < wfull; ++ i)
	{
		if (i == wmark)
		{
			xpix = display_barcolumn(xpix, ypix, vpatternmax);
			continue;
		}
#if (DSTYLE_G_X132_Y64 || DSTYLE_G_X128_Y64) && DSTYLE_UR3LMZMOD
		xpix = display_barcolumn(xpix, ypix, vemptyp);
#elif DSTYLE_G_X64_Y32
		xpix = display_barcolumn(xpix, ypix, (i % 6) == 5 ? vpattern : vemptyp);
#else
		xpix = display_barcolumn(xpix, ypix, (i % 2) == 0 ? vemptyp : PATTERN_SPACE);
#endif
	}
}


// Адресация для s-meter
static uint_fast8_t
display_bars_x_rx(
	uint_fast8_t x,
	uint_fast8_t xoffs	// grid
	)
{
	return x + xoffs;
}

// Адресация для swr-meter
static uint_fast8_t
display_bars_x_swr(
	uint_fast8_t x,
	uint_fast8_t xoffs	// grid
	)
{
	return display_bars_x_rx(x, xoffs);
}

// Адресация для pwr-meter
static uint_fast8_t
display_bars_x_pwr(
	uint_fast8_t x,
	uint_fast8_t xoffs	// grid
	)
{
#if WITHSHOWSWRPWR	/* на дисплее одновременно отображаются SWR-meter и PWR-meter */
	return display_bars_x_rx(x, xoffs + CHARS2GRID(BDTH_ALLSWR + BDTH_SPACESWR));
#else
	return display_bars_x_rx(x, xoffs);
#endif
}


static uint_fast8_t display_mapbar(
	uint_fast8_t val,
	uint_fast8_t bottom, uint_fast8_t top,
	uint_fast8_t mapleft,
	uint_fast8_t mapinside,
	uint_fast8_t mapright
	)
{
	if (val < bottom)
		return mapleft;
	if (val < top)
		return mapinside;
	return mapright;
}

void display_smeter(
	uint_fast8_t x,
	uint_fast8_t y,
	uint_fast8_t value,		// текущее значение
	uint_fast8_t tracemax,	// метка запомненного максимума
	uint_fast8_t level9,	// s9 level
	uint_fast8_t delta1,	// s9 - s0 delta
	uint_fast8_t delta2)	// s9+50 - s9 delta
{
	tracemax = value > tracemax ? value : tracemax;	// защита от рассогласования значений
	//delta1 = delta1 > level9 ? level9 : delta1;

	const uint_fast8_t leftmin = level9 - delta1;
	const uint_fast8_t mapleftval = display_mapbar(value, leftmin, level9, 0, value - leftmin, delta1);
	const uint_fast8_t mapleftmax = display_mapbar(tracemax, leftmin, level9, delta1, tracemax - leftmin, delta1); // delta1 - invisible
	const uint_fast8_t maprightval = display_mapbar(value, level9, level9 + delta2, 0, value - level9, delta2);
	const uint_fast8_t maprightmax = display_mapbar(tracemax, level9, level9 + delta2, delta2, tracemax - level9, delta2); // delta2 - invisible

	colmain_setcolors(LCOLOR, BGCOLOR);
	uint_fast16_t ypix;
	uint_fast16_t xpix = display_wrdatabar_begin(display_bars_x_rx(x, CHARS2GRID(0)), y, & ypix);
	display_bar(xpix, ypix, BDTH_LEFTRX, mapleftval, mapleftmax, delta1, PATTERN_BAR_HALF, PATTERN_BAR_FULL, PATTERN_BAR_EMPTYHALF);		//ниже 9 баллов ничего
	display_wrdatabar_end();
	//
	colmain_setcolors(RCOLOR, BGCOLOR);
	uint_fast16_t ypix2;
	uint_fast16_t xpix2 = display_wrdatabar_begin(display_bars_x_rx(x, CHARS2GRID(BDTH_LEFTRX)), y, & ypix2);
	display_bar(xpix2, ypix2, BDTH_RIGHTRX, maprightval, maprightmax, delta2, PATTERN_BAR_FULL, PATTERN_BAR_FULL, PATTERN_BAR_EMPTYFULL);		// выше 9 баллов ничего нет.
	display_wrdatabar_end();

	if (BDTH_SPACERX != 0)
	{
		uint_fast16_t ypix;
		uint_fast16_t xpix = display_wrdatabar_begin(display_bars_x_pwr(x, CHARS2GRID(BDTH_ALLRX)), y, & ypix);
		display_bar(xpix, ypix, BDTH_SPACERX, 0, 1, 1, PATTERN_SPACE, PATTERN_SPACE, PATTERN_SPACE);
		display_wrdatabar_end();
	}
}

void main(void)
{
    SystemCoreClockUpdate();
	// Flash green led on GPIO25
	gpio0_pin_initialize(GPIO_LED, GPIO_IO_TYPE_OUTPUT, GPIO_NOPULL, GPIO_SPEED_LOW, GPIO_AF_NONE);
    if (SystemCoreClock == 0)
	{
		flashed(2);
	}
	if (SysTick_Config(SystemCoreClock / 100) != 0)	// 100 Hz ticks
	{
		flashed(3);
	}
	//SysTick_Config(10000);
	//flashed(7);
	HARDWARE_DEBUG_INITIALIZE();
	HARDWARE_DEBUG_SET_SPEED(115200);

	hardware_spi_initialize();

	PRINTF(__DATE__ " " __TIME__ " - Hello!\n");
	PRINTF("Hello2!, SystemCoreClock=%lu\n", (unsigned long) SystemCoreClock);

//	for (;;)
//	{
//		spi_select2(targetlcd, 0, 0);
//		spi_progval8_p1(targetlcd, 0xF0);
//		spi_complete(targetlcd);
//		spi_unselect(targetlcd);
//	}
	display_hardware_initialize();
	display_reset();
	display_initialize();
	display_clear();

	display_at(0,  0, "Hello!");


	// software delays calibration test
//	gpio0_pin_initialize(26, GPIO_IO_TYPE_OUTPUT, GPIO_NOPULL, GPIO_SPEED_LOW, GPIO_AF_NONE);
//	for (;;)
//	{
//		gpio0_pin_setstate(26, 1);
//		local_delay_ms(20);
//		gpio0_pin_setstate(26, 0);
//		local_delay_ms(20);
//	}

	int value = 50;
	display_smeter(0, 2, value, 39, 60, 60, 60);
	for (;;)
	{
		char c;
		if (dbg_getchar(& c))
		{
			dbg_putchar(c);

			// s-meter test
			value = (value + 5) % 120;
			display_smeter(0, 2, value, 39, 60, 60, 60);
		}
//		static int line;
//		char s [132];
//		snprintf(s, 132, "1 Hello! line = %d, SystemCoreClock=%lu, SSPSR=%08lX\n", line ++, SystemCoreClock, SPI0->SSPSR);
//		putstrq(s);
	}
}


