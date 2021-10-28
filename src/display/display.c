/* $Id$ */
//
// Проект HF Dream Receiver (КВ приёмник мечты)
// автор Гена Завидовский mgs2001@mail.ru
// UA1ARN
//
// Доработки для LS020 Василий Линывый, livas60@mail.ru
//

#include "hardware.h"
#include "board.h"
#include "display.h"
#include "formats.h"
#include "spi.h"	// hardware_spi_master_send_frame
//#include "display2.h"
#include <string.h>
#include <stdlib.h>


const char * savestring = "no data";
const char * savewhere = "no func";

#if LCDMODE_LTDC

#define FONTSHERE 1

#include "fontmaps.h"

#if WITHALTERNATIVEFONTS
	uint8_t const * font_big = ltdc_CenturyGothic_big [0] [0];
	uint8_t const * font_half = ltdc_CenturyGothic_half [0] [0];
	const size_t size_bigfont = sizeof ltdc_CenturyGothic_big [0] [0];
	const size_t size_halffont = sizeof ltdc_CenturyGothic_half [0] [0];
#else
	uint8_t const * font_big = S1D13781_bigfont_LTDC [0] [0];
	uint8_t const * font_half = S1D13781_halffont_LTDC [0] [0];
	const size_t size_bigfont = sizeof S1D13781_bigfont_LTDC [0] [0];
	const size_t size_halffont = sizeof S1D13781_halffont_LTDC [0] [0];
#endif /* WITHALTERNATIVEFONTS */

//
//#if ! LCDMODE_LTDC_L24
//#include "./byte2crun.h"
//#endif /* ! LCDMODE_LTDC_L24 */

typedef PACKEDCOLORMAIN_T FRAMEBUFF_T [LCDMODE_MAIN_PAGES] [GXSIZE(DIM_SECOND, DIM_FIRST)];

#if defined (SDRAM_BANK_ADDR) && LCDMODE_LTDCSDRAMBUFF && LCDMODE_LTDC
	#define framebuff (* (FRAMEBUFF_T *) SDRAM_BANK_ADDR)
#else /* defined (SDRAM_BANK_ADDR) && LCDMODE_LTDCSDRAMBUFF && LCDMODE_LTDC */
	//#define framebuff (framebuff0)
	//extern FRAMEBUFF_T framebuff0;	//L8 (8-bit Luminance or CLUT)
#endif /* defined (SDRAM_BANK_ADDR) && LCDMODE_LTDCSDRAMBUFF && LCDMODE_LTDC */

#if ! defined (SDRAM_BANK_ADDR)
	// буфер экрана
	RAMFRAMEBUFF ALIGNX_BEGIN FRAMEBUFF_T fbfX ALIGNX_END;

	static uint_fast8_t drawframe;

	// переключиться на использование для DRAW следующего фреймбуфера (его номер возвращается)
	uint_fast8_t colmain_fb_next(void)
	{
		drawframe = (drawframe + 1) % LCDMODE_MAIN_PAGES;
		return drawframe;
	}

	PACKEDCOLORMAIN_T *
	colmain_fb_draw(void)
	{
		return fbfX [drawframe];
	}

	void colmain_fb_initialize(void)
	{
		uint_fast8_t i;
		for (i = 0; i < LCDMODE_MAIN_PAGES; ++ i)
			memset(fbfX [i], 0, sizeof fbfX [0]);
	}

	uint_fast8_t colmain_getindexbyaddr(uintptr_t addr)
	{
		uint_fast8_t i;
		for (i = 0; i < LCDMODE_MAIN_PAGES; ++ i)
		{
			if ((uintptr_t) fbfX [i] == addr)
				return i;
		}
		ASSERT(0);
		return 0;
	}

#elif WITHSDRAMHW && LCDMODE_LTDCSDRAMBUFF

	// переключиться на использование для DRAW следующего фреймбуфера (его номер возвращается)
	uint_fast8_t colmain_fb_next(void)
	{
		return 0;
	}

	PACKEDCOLORMAIN_T *
	colmain_fb_draw(void)
	{
		return & framebuff[0][0];
	}

	void colmain_fb_initialize(void)
	{
		memset(framebuff, 0, sizeof framebuff);
	}

	uint_fast8_t colmain_getindexbyaddr(uintptr_t addr)
	{
		uint_fast8_t i;
		for (i = 0; i < LCDMODE_MAIN_PAGES; ++ i)
		{
			if ((uintptr_t) framebuff [i] == addr)
				return i;
		}
		ASSERT(0);
		return 0;
	}

#else
	RAMFRAMEBUFF ALIGNX_BEGIN PACKEDCOLORMAIN_T fbf [GXSIZE(DIM_SECOND, DIM_FIRST)] ALIGNX_END;

	// переключиться на использование для DRAW следующего фреймбуфера (его номер возвращается)
	uint_fast8_t colmain_fb_next(void)
	{
		return 0;
	}

	PACKEDCOLORMAIN_T *
	colmain_fb_draw(void)
	{
		return fbf;
	}

	void colmain_fb_initialize(void)
	{
		memset(fbf, 0, sizeof fbf);
	}

	uint_fast8_t colmain_getindexbyaddr(uintptr_t addr)
	{
		return 0;
	}


#endif /* LCDMODE_LTDC */

#if LCDMODE_LTDC

void display_putpixel(
	uint_fast16_t x,	// горизонтальная координата пикселя (0..dx-1) слева направо
	uint_fast16_t y,	// вертикальная координата пикселя (0..dy-1) сверху вниз
	COLORMAIN_T color
	)
{
	PACKEDCOLORMAIN_T * const buffer = colmain_fb_draw();
	const uint_fast16_t dx = DIM_X;
	const uint_fast16_t dy = DIM_Y;
	colmain_putpixel(buffer, dx, dy, x, y, color);
}

/* заполнение прямоугольника на основном экране произвольным цветом
*/
void
display_fillrect(
	uint_fast16_t x, uint_fast16_t y, 	// координаты в пикселях
	uint_fast16_t w, uint_fast16_t h, 	// размеры в пикселях
	COLORMAIN_T color
	)
{
	PACKEDCOLORMAIN_T * const buffer = colmain_fb_draw();
	const uint_fast16_t dx = DIM_X;
	const uint_fast16_t dy = DIM_Y;

	colmain_fillrect(buffer, dx, dy, x, y, w, h, color);
}

/* рисование линии на основном экране произвольным цветом
*/
void
display_line(
	int x1, int y1,
	int x2, int y2,
	COLORMAIN_T color
	)
{
	PACKEDCOLORMAIN_T * const buffer = colmain_fb_draw();
	const uint_fast16_t dx = DIM_X;
	const uint_fast16_t dy = DIM_Y;

	colmain_line(buffer, dx, dy, x1, y1, x2, y2, color, 0);
}

#endif /* LCDMODE_LTDC */

/* копирование содержимого окна с перекрытием для водопада */
void
display_scroll_down(
	uint_fast16_t x0,	// левый верхний угол окна
	uint_fast16_t y0,	// левый верхний угол окна
	uint_fast16_t w, 	// до 65535 пикселей - ширина окна
	uint_fast16_t h, 	// до 65535 пикселей - высота окна
	uint_fast16_t n,	// количество строк прокрутки
	int_fast16_t hshift	// количество пиксеелей для сдвига влево (отрицательное число) или вправо (положительное).
	)
{
	PACKEDCOLORMAIN_T * const buffer = colmain_fb_draw();
	const uint_fast16_t dx = DIM_X;
	const uint_fast16_t dy = DIM_Y;

#if WITHDMA2DHW && LCDMODE_LTDC

#if LCDMODE_HORFILL && defined (DMA2D_FGPFCCR_CM_VALUE_MAIN)
	// для случая когда горизонтальные пиксели в видеопямяти располагаются подряд
	/* TODO: В DMA2D нет средств управления направлением пересылки, потому данный код копирует сам на себя данные (размножает) */
	/* исходный растр */
	DMA2D->FGMAR = (uintptr_t) colmain_mem_at(buffer, dx, dy, y0 + 0, x0);
	DMA2D->FGOR = (DMA2D->FGOR & ~ (DMA2D_FGOR_LO)) |
		((DIM_X - w) << DMA2D_FGOR_LO_Pos) |
		0;
	/* целевой растр */
	DMA2D->OMAR = (uintptr_t) colmain_mem_at(buffer, dx, dy, y0 + n, x0);
	DMA2D->OOR = (DMA2D->OOR & ~ (DMA2D_OOR_LO)) |
		((DIM_X - w) << DMA2D_OOR_LO_Pos) |
		0;
	/* размер пересылаемого растра */
	DMA2D->NLR = (DMA2D->NLR & ~ (DMA2D_NLR_NL | DMA2D_NLR_PL)) |
		((h - n) << DMA2D_NLR_NL_Pos) |
		(w << DMA2D_NLR_PL_Pos) |
		0;
	/* формат пикселя */
	DMA2D->FGPFCCR = (DMA2D->FGPFCCR & ~ (DMA2D_FGPFCCR_CM)) |
		DMA2D_FGPFCCR_CM_VALUE_MAIN |	/* Color mode - framebuffer pixel format */
		0;

	/* запустить операцию */
	DMA2D->CR = (DMA2D->CR & ~ (DMA2D_CR_MODE)) |
		0 * DMA2D_CR_MODE_0 |	// 00: Memory-to-memory (FG fetch only)
		1 * DMA2D_CR_START |
		0;

	/* ожидаем выполнения операции */
	while ((DMA2D->CR & DMA2D_CR_START) != 0)
		hardware_nonguiyield();
	__DMB();

	ASSERT((DMA2D->ISR & DMA2D_ISR_CEIF) == 0);	// Configuration Error
	ASSERT((DMA2D->ISR & DMA2D_ISR_TEIF) == 0);	// Transfer Error

#else /* LCDMODE_HORFILL */
#endif /* LCDMODE_HORFILL */

#endif /* WITHDMA2DHW && LCDMODE_LTDC */
}

/* копирование содержимого окна с перекрытием для водопада */
void
display_scroll_up(
	uint_fast16_t x0,	// левый верхний угол окна
	uint_fast16_t y0,	// левый верхний угол окна
	uint_fast16_t w, 	// до 65535 пикселей - ширина окна
	uint_fast16_t h, 	// до 65535 пикселей - высота окна
	uint_fast16_t n,	// количество строк прокрутки
	int_fast16_t hshift	// количество пиксеелей для сдвига влево (отрицательное число) или вправо (положительное).
	)
{
	PACKEDCOLORMAIN_T * const buffer = colmain_fb_draw();
	const uint_fast16_t dx = DIM_X;
	const uint_fast16_t dy = DIM_Y;

#if WITHDMA2DHW && LCDMODE_LTDC
#if LCDMODE_HORFILL && defined (DMA2D_FGPFCCR_CM_VALUE_MAIN)
	// для случая когда горизонтальные пиксели в видеопямяти располагаются подряд

	/* исходный растр */
	DMA2D->FGMAR = (uintptr_t) colmain_mem_at(buffer, dx, dy, y0 + n, x0);
	DMA2D->FGOR = (DMA2D->FGOR & ~ (DMA2D_FGOR_LO)) |
		((DIM_X - w) << DMA2D_FGOR_LO_Pos) |
		0;
	/* целевой растр */
	DMA2D->OMAR = (uintptr_t) colmain_mem_at(buffer, dx, dy, y0 + 0, x0);
	DMA2D->OOR = (DMA2D->OOR & ~ (DMA2D_OOR_LO)) |
		((DIM_X - w) << DMA2D_OOR_LO_Pos) |
		0;
	/* размер пересылаемого растра */
	DMA2D->NLR = (DMA2D->NLR & ~ (DMA2D_NLR_NL | DMA2D_NLR_PL)) |
		((h - n) << DMA2D_NLR_NL_Pos) |
		(w << DMA2D_NLR_PL_Pos) |
		0;
	/* формат пикселя */
	DMA2D->FGPFCCR = (DMA2D->FGPFCCR & ~ (DMA2D_FGPFCCR_CM)) |
		DMA2D_FGPFCCR_CM_VALUE_MAIN |	/* Color mode - framebuffer pixel format */
		0;

	/* запустить операцию */
	DMA2D->CR = (DMA2D->CR & ~ (DMA2D_CR_MODE)) |
		0 * DMA2D_CR_MODE_0 |	// 00: Memory-to-memory (FG fetch only)
		1 * DMA2D_CR_START |
		0;

	/* ожидаем выполнения операции */
	while ((DMA2D->CR & DMA2D_CR_START) != 0)
		hardware_nonguiyield();
	__DMB();

	ASSERT((DMA2D->ISR & DMA2D_ISR_CEIF) == 0);	// Configuration Error
	ASSERT((DMA2D->ISR & DMA2D_ISR_TEIF) == 0);	// Transfer Error

#else /* LCDMODE_HORFILL */
#endif /* LCDMODE_HORFILL */
#endif /* WITHDMA2DHW && LCDMODE_LTDC */
}


#if ! LCDMODE_LTDC_L24
#include "./byte2crun.h"
#endif /* ! LCDMODE_LTDC_L24 */

static PACKEDCOLORMAIN_T ltdc_fg = COLORMAIN_WHITE, ltdc_bg = COLORMAIN_BLACK;

#if ! LCDMODE_LTDC_L24
static const FLASHMEM PACKEDCOLORMAIN_T (* byte2runmain) [256][8] = & byte2runmain_COLORMAIN_WHITE_COLORMAIN_BLACK;
//static const FLASHMEM PACKEDCOLORPIP_T (* byte2runpip) [256][8] = & byte2runpip_COLORPIP_WHITE_COLORPIP_BLACK;
#endif /* ! LCDMODE_LTDC_L24 */

void colmain_setcolors(COLORMAIN_T fg, COLORMAIN_T bg)
{

#if ! LCDMODE_LTDC_L24
	ltdc_fg = fg;
	ltdc_bg = bg;
#else /* ! LCDMODE_LTDC_L24 */

	ltdc_fg.r = fg >> 16;
	ltdc_fg.g = fg >> 8;
	ltdc_fg.b = fg >> 0;
	ltdc_bg.r = bg >> 16;
	ltdc_bg.g = bg >> 8;
	ltdc_bg.b = bg >> 0;

#endif /* ! LCDMODE_LTDC_L24 */

#if ! LCDMODE_LTDC_L24

	COLORMAIN_SELECTOR(byte2runmain);

#endif /* ! LCDMODE_LTDC_L24 */

	//COLORPIP_SELECTOR(byte2runpip);

}

void colmain_setcolors3(COLORMAIN_T fg, COLORMAIN_T bg, COLORMAIN_T fgbg)
{
	colmain_setcolors(fg, bg);
}

/* индивидуальные функции драйвера дисплея - реализованы в соответствующем из файлов */
void display_clear(void)
{
	const COLORMAIN_T bg = display_getbgcolor();
	PACKEDCOLORMAIN_T * const buffer = colmain_fb_draw();

	colmain_fillrect(buffer, DIM_X, DIM_Y, 0, 0, DIM_X, DIM_Y, bg);
}

void display_plotstart(
	uint_fast16_t dy	// Высота окна источника в пикселях
	)
{

}

void display_plotstop(void)
{

}

// Вызовы этой функции (или группу вызовов) требуется "обрамить" парой вызовов
// display_wrdatabar_begin() и display_wrdatabar_end().
void display_bar(
	uint_fast16_t x,
	uint_fast16_t y,
	uint_fast8_t width,	/* количество знакомест, занимаемых индикатором */
	uint_fast8_t value,		/* значение, которое надо отобразить */
	uint_fast8_t tracevalue,		/* значение маркера, которое надо отобразить */
	uint_fast8_t topvalue,	/* значение, соответствующее полностью заполненному индикатору */
	uint_fast8_t vpattern,	/* DISPLAY_BAR_HALF или DISPLAY_BAR_FULL */
	uint_fast8_t patternmax,	/* DISPLAY_BAR_HALF или DISPLAY_BAR_FULL - для отображения запомненного значения */
	uint_fast8_t emptyp			/* паттерн для заполнения между штрихами */
	)
{
	PACKEDCOLORMAIN_T * const buffer = colmain_fb_draw();
	const uint_fast16_t dx = DIM_X;
	const uint_fast16_t dy = DIM_Y;
	ASSERT(value <= topvalue);
	ASSERT(tracevalue <= topvalue);
	const uint_fast16_t wfull = GRID2X(width);
	const uint_fast16_t h = SMALLCHARH; //GRID2Y(1);
	const uint_fast16_t wpart = (uint_fast32_t) wfull * value / topvalue;
	const uint_fast16_t wmark = (uint_fast32_t) wfull * tracevalue / topvalue;
	const uint_fast8_t hpattern = 0x33;

	colmain_fillrect(buffer, dx, dy, 	x, y, 			wpart, h, 			ltdc_fg);
	colmain_fillrect(buffer, dx, dy, 	x + wpart, y, 	wfull - wpart, h, 	ltdc_bg);
	if (wmark < wfull && wmark >= wpart)
		colmain_fillrect(buffer, dx, dy, x + wmark, y, 	1, h, 				ltdc_fg);
}

// самый маленький шрифт
uint_fast16_t display_wrdata2_begin(uint_fast8_t x, uint_fast8_t y, uint_fast16_t * yp)
{
	//ltdc_secondoffs = 0;
	//ltdc_h = SMALLCHARH;

	* yp = GRID2Y(y);
	return GRID2X(x);
}

void display_wrdata2_end(void)
{
}


// Выдать один цветной пиксель
static void
ltdc_pix1color(
	uint_fast16_t x,	// горизонтальная координата пикселя (0..dx-1) слева направо
	uint_fast16_t y,	// вертикальная координата пикселя (0..dy-1) сверху вниз
	PACKEDCOLORMAIN_T color
	)
{
	PACKEDCOLORMAIN_T * const buffer = colmain_fb_draw();
	const uint_fast16_t dx = DIM_X;
	const uint_fast16_t dy = DIM_Y;
	volatile PACKEDCOLORMAIN_T * const tgr = colmain_mem_at(buffer, dx, dy, x, y);
	* tgr = color;
	//arm_hardware_flush((uintptr_t) tgr, sizeof * tgr);
}


// Выдать один цветной пиксель (фон/символ)
static void
ltdc_pixel(
	uint_fast16_t x,	// горизонтальная координата пикселя (0..dx-1) слева направо
	uint_fast16_t y,	// вертикальная координата пикселя (0..dy-1) сверху вниз
	uint_fast8_t v			// 0 - цвет background, иначе - foreground
	)
{
	ltdc_pix1color(x, y, v ? ltdc_fg : ltdc_bg);
}


// Выдать восемь цветных пикселей, младший бит - самый верхний в растре
static void
ltdc_vertical_pixN(
	uint_fast16_t x,	// горизонтальная координата пикселя (0..dx-1) слева направо
	uint_fast16_t y,	// вертикальная координата пикселя (0..dy-1) сверху вниз
	uint_fast8_t pattern,		// pattern
	uint_fast8_t w		// number of lower bits used in pattern
	)
{

#if LCDMODE_LTDC_L24 || LCDMODE_HORFILL

	// TODO: для паттернов шире чем восемь бит, повторить нужное число раз.
	ltdc_pixel(x, y + 0, pattern & 0x01);
	ltdc_pixel(x, y + 1, pattern & 0x02);
	ltdc_pixel(x, y + 2, pattern & 0x04);
	ltdc_pixel(x, y + 3, pattern & 0x08);
	ltdc_pixel(x, y + 4, pattern & 0x10);
	ltdc_pixel(x, y + 5, pattern & 0x20);
	ltdc_pixel(x, y + 6, pattern & 0x40);
	ltdc_pixel(x, y + 7, pattern & 0x80);

	// сместить по вертикали?
	//ltdc_secondoffs ++;

#else /* LCDMODE_LTDC_L24 */
	PACKEDCOLORMAIN_T * const buffer = colmain_fb_draw();
	const uint_fast16_t dx = DIM_X;
	const uint_fast16_t dy = DIM_Y;
	PACKEDCOLORMAIN_T * const tgr = colmain_mem_at(buffer, dx, dy, x, y);
	// размещаем пиксели по горизонтали
	// TODO: для паттернов шире чем восемь бит, повторить нужное число раз.
	const FLASHMEM PACKEDCOLORMAIN_T * const pcl = (* byte2runmain) [pattern];
	memcpy(tgr, pcl, sizeof (* pcl) * w);
	//arm_hardware_flush((uintptr_t) tgr, sizeof (PACKEDCOLORMAIN_T) * w);
#endif /* LCDMODE_LTDC_L24 */
}

#if LCDMODE_HORFILL

// для случая когда горизонтальные пиксели в видеопямяти располагаются подряд
void RAMFUNC ltdc_horizontal_pixels(
	PACKEDCOLORMAIN_T * tgr,		// target raster
	const FLASHMEM uint8_t * raster,
	uint_fast16_t width	// number of bits (start from LSB first byte in raster)
	)
{
	uint_fast16_t col;
	uint_fast16_t w = width;

	for (col = 0; w >= 8; col += 8, w -= 8)
	{
		const FLASHMEM PACKEDCOLORMAIN_T * const pcl = (* byte2runmain) [* raster ++];
		memcpy(tgr + col, pcl, sizeof (* tgr) * 8);
	}
	if (w != 0)
	{
		const FLASHMEM PACKEDCOLORMAIN_T * const pcl = (* byte2runmain) [* raster ++];
		memcpy(tgr + col, pcl, sizeof (* tgr) * w);
	}
	//arm_hardware_flush((uintptr_t) tgr, sizeof (* tgr) * width);
}


uint_fast16_t
RAMFUNC_NONILINE ltdc_horizontal_put_char_unified(
	const FLASHMEM uint8_t * fontraster,
	uint_fast8_t width,		// пикселей в символе по горизонтали знакогнератора
	uint_fast8_t width2,	// пикселей в символе по горизонтали отображается (для уменьшеных в ширину символов большиз шрифтов)
	uint_fast8_t height,	// строк в символе по вертикали
	uint_fast8_t bytesw,	// байтов в одной строке символа
	PACKEDCOLORMAIN_T * const __restrict buffer,
	const uint_fast16_t dx,
	const uint_fast16_t dy,
	uint_fast16_t x, uint_fast16_t y,
	uint_fast8_t c
	)
{
	uint_fast8_t cgrow;
	for (cgrow = 0; cgrow < height; ++ cgrow)
	{
		PACKEDCOLORMAIN_T * const tgr = colmain_mem_at(buffer, dx, dy, x, y + cgrow);
		ltdc_horizontal_pixels(tgr, & fontraster [(c * height + cgrow) * bytesw], width2);
	}
	return x + width2;
}

// Вызов этой функции только внутри display_wrdata_begin() и display_wrdata_end();
// return new x
static uint_fast16_t
RAMFUNC_NONILINE
ltdc_horizontal_put_char_small(uint_fast16_t x, uint_fast16_t y, char cc)
{
	PACKEDCOLORMAIN_T * const buffer = colmain_fb_draw();
	const uint_fast16_t dx = DIM_X;
	const uint_fast16_t dy = DIM_Y;
	const uint_fast8_t c = smallfont_decode((unsigned char) cc);
	return ltdc_horizontal_put_char_unified(S1D13781_smallfont_LTDC [0] [0], SMALLCHARW, SMALLCHARW, SMALLCHARH, sizeof S1D13781_smallfont_LTDC [0] [0], buffer, dx, dy, x, y, c);
//	const uint_fast8_t width = SMALLCHARW;
//	const uint_fast8_t c = smallfont_decode((unsigned char) cc);
//	uint_fast8_t cgrow;
//	for (cgrow = 0; cgrow < SMALLCHARH; ++ cgrow)
//	{
//		PACKEDCOLORMAIN_T * const tgr = colmain_mem_at(buffer, dx, dy, x, y + cgrow);
//		ltdc_horizontal_pixels(tgr, S1D13781_smallfont_LTDC [c] [cgrow], width);
//	}
//	return x + width;
}

// Вызов этой функции только внутри display_wrdatabig_begin() и display_wrdatabig_end();
// return new x coordinate
static uint_fast16_t RAMFUNC_NONILINE ltdc_horizontal_put_char_big(uint_fast16_t x, uint_fast16_t y, char cc)
{
	PACKEDCOLORMAIN_T * const buffer = colmain_fb_draw();
	const uint_fast16_t dx = DIM_X;
	const uint_fast16_t dy = DIM_Y;
	const uint_fast8_t width = ((cc == '.' || cc == '#') ? BIGCHARW_NARROW  : BIGCHARW);	// полнаяширина символа в пикселях
    const uint_fast8_t c = bigfont_decode((unsigned char) cc);
	return ltdc_horizontal_put_char_unified(font_big, BIGCHARW, width, BIGCHARH, size_bigfont, buffer, dx, dy, x, y, c);
//	uint_fast8_t cgrow;
//	for (cgrow = 0; cgrow < BIGCHARH; ++ cgrow)
//	{
//		PACKEDCOLORMAIN_T * const tgr = colmain_mem_at(buffer, dx, dy, x, y + cgrow);
//		ltdc_horizontal_pixels(tgr, S1D13781_bigfont_LTDC [c] [cgrow], width);
//	}
//	return x + width;
}

// Вызов этой функции только внутри display_wrdatabig_begin() и display_wrdatabig_end();
// return new x coordinate
static uint_fast16_t RAMFUNC_NONILINE ltdc_horizontal_put_char_half(uint_fast16_t x, uint_fast16_t y, char cc)
{
	PACKEDCOLORMAIN_T * const buffer = colmain_fb_draw();
	const uint_fast16_t dx = DIM_X;
	const uint_fast16_t dy = DIM_Y;
	const uint_fast8_t width = HALFCHARW;
    const uint_fast8_t c = bigfont_decode((unsigned char) cc);
	return ltdc_horizontal_put_char_unified(font_half, HALFCHARW, width, HALFCHARH, size_halffont, buffer, dx, dy, x, y, c);
//	uint_fast8_t cgrow;
//	for (cgrow = 0; cgrow < HALFCHARH; ++ cgrow)
//	{
//		PACKEDCOLORMAIN_T * const tgr = colmain_mem_at(buffer, dx, dy, x, y + cgrow);
//		ltdc_horizontal_pixels(tgr, S1D13781_halffont_LTDC [c] [cgrow], width);
//	}
//	return x + width;
}

#else /* LCDMODE_HORFILL */

// Вызов этой функции только внутри display_wrdata_begin() и 	display_wrdata_end();
static uint_fast16_t RAMFUNC_NONILINE ltdc_vertical_put_char_small(uint_fast16_t x, uint_fast16_t y, char cc)
{
	uint_fast8_t i = 0;
	const uint_fast8_t c = smallfont_decode((unsigned char) cc);
	enum { NBYTES = (sizeof ls020_smallfont [0] / sizeof ls020_smallfont [0] [0]) };
	const FLASHMEM uint8_t * const p = & ls020_smallfont [c] [0];

	for (; i < NBYTES; ++ i)
		ltdc_vertical_pixN(x ++, y, p [i], 8);	// Выдать восемь цветных пикселей, младший бит - самый верхний в растре
	return x;
}

// Вызов этой функции только внутри display_wrdatabig_begin() и display_wrdatabig_end();
static uint_fast16_t RAMFUNC_NONILINE ltdc_vertical_put_char_big(uint_fast16_t x, uint_fast16_t y, char cc)
{
	// '#' - узкий пробел
	enum { NBV = (BIGCHARH / 8) }; // сколько байтов в одной вертикали
	uint_fast8_t i = NBV * ((cc == '.' || cc == '#') ? 12 : 0);	// начальная колонка знакогенератора, откуда начинать.
    const uint_fast8_t c = bigfont_decode((unsigned char) cc);
	enum { NBYTES = (sizeof ls020_bigfont [0] / sizeof ls020_bigfont [0] [0]) };
	const FLASHMEM uint8_t * const p = & ls020_bigfont [c] [0];

	for (; i < NBYTES; ++ i)
		ltdc_vertical_pixN(x ++, y, p [i], 8);	// Выдать восемь цветных пикселей, младший бит - самый верхний в растре
	return x;
}

// Вызов этой функции только внутри display_wrdatabig_begin() и display_wrdatabig_end();
static uint_fast16_t RAMFUNC_NONILINE ltdc_vertical_put_char_half(uint_fast16_t x, uint_fast16_t y, char cc)
{
	uint_fast8_t i = 0;
    const uint_fast8_t c = bigfont_decode((unsigned char) cc);
	enum { NBYTES = (sizeof ls020_halffont [0] / sizeof ls020_halffont [0] [0]) };
	const FLASHMEM uint8_t * const p = & ls020_halffont [c] [0];

	for (; i < NBYTES; ++ i)
		ltdc_vertical_pixN(x ++, y, p [i], 8);	// Выдать восемь цветных пикселей, младший бит - самый верхний в растре
	return x;
}


#endif /* LCDMODE_HORFILL */

#if 0
uint_fast16_t display_put_char_small2(uint_fast16_t x, uint_fast16_t y, uint_fast8_t c, uint_fast8_t lowhalf)
{
#if LCDMODE_HORFILL
	// для случая когда горизонтальные пиксели в видеопямяти располагаются подряд
	return ltdc_horizontal_put_char_small(x, y, c);
#else /* LCDMODE_HORFILL */
	return ltdc_vertical_put_char_small(x, y, c);
#endif /* LCDMODE_HORFILL */
}
#endif

// полоса индикатора
uint_fast16_t display_wrdatabar_begin(uint_fast8_t x, uint_fast8_t y, uint_fast16_t * yp)
{
//	ltdc_secondoffs = 0;
//	ltdc_h = 8;

	* yp = GRID2Y(y);
	return GRID2X(x);
}

// Выдать восемь цветных пикселей, младший бит - самый верхний в растре
uint_fast16_t
display_barcolumn(uint_fast16_t xpix, uint_fast16_t ypix, uint_fast8_t pattern)
{
//	ltdc_vertical_pixN(pattern, 8);	// Выдать восемь цветных пикселей, младший бит - самый верхний в растре
	return xpix + 1;
}

void display_wrdatabar_end(void)
{
}

// большие и средние цифры (частота)
uint_fast16_t display_wrdatabig_begin(uint_fast8_t x, uint_fast8_t y, uint_fast16_t * yp)
{
	//ltdc_secondoffs = 0;
	//ltdc_h = BIGCHARH;

	* yp = GRID2Y(y);
	return GRID2X(x);
}

uint_fast16_t display_put_char_big(uint_fast16_t x, uint_fast16_t y, uint_fast8_t c, uint_fast8_t lowhalf)
{
	savewhere = __func__;
#if LCDMODE_HORFILL
	// для случая когда горизонтальные пиксели в видеопямяти располагаются подряд
	return ltdc_horizontal_put_char_big(x, y, c);
#else /* LCDMODE_HORFILL */
	return ltdc_vertical_put_char_big(x, y, c);
#endif /* LCDMODE_HORFILL */
}

uint_fast16_t display_put_char_half(uint_fast16_t x, uint_fast16_t y, uint_fast8_t c, uint_fast8_t lowhalf)
{
	savewhere = __func__;
#if LCDMODE_HORFILL
	// для случая когда горизонтальные пиксели в видеопямяти располагаются подряд
	return ltdc_horizontal_put_char_half(x, y, c);
#else /* LCDMODE_HORFILL */
	return ltdc_vertical_put_char_half(x, y, c);
#endif /* LCDMODE_HORFILL */
}

void display_wrdatabig_end(void)
{
}

// обычный шрифт
uint_fast16_t display_wrdata_begin(uint_fast8_t x, uint_fast8_t y, uint_fast16_t * yp)
{
//	ltdc_secondoffs = 0;
//	ltdc_h = SMALLCHARH;

	* yp = GRID2Y(y);
	return GRID2X(x);
}

uint_fast16_t display_put_char_small(uint_fast16_t x, uint_fast16_t y, uint_fast8_t c, uint_fast8_t lowhalf)
{
#if LCDMODE_HORFILL
	// для случая когда горизонтальные пиксели в видеопямяти располагаются подряд
	return ltdc_horizontal_put_char_small(x, y, c);
#else /* LCDMODE_HORFILL */
	return ltdc_vertical_put_char_small(x, y, c);
#endif /* LCDMODE_HORFILL */
}

void display_wrdata_end(void)
{
}


#if LCDMODE_LQ043T3DX02K || LCDMODE_AT070TN90 || LCDMODE_AT070TNA2 || LCDMODE_H497TLB01P4

// заглушки

/* аппаратный сброс дисплея - перед инициализаций */
/* вызывается при разрешённых прерываниях. */
void
display_reset(void)
{
	board_lcd_reset(1); 	// Pull RST pin up
	board_update();
	local_delay_ms(1); // Delay 1ms
	board_lcd_reset(0); 	// Pull RST pin down
	board_update();
	local_delay_ms(10); // Delay 10ms
	board_lcd_reset(1); 	// Pull RST pin up
	board_update();
	local_delay_ms(50); // Delay 50 ms
}

void display_set_contrast(uint_fast8_t v)
{
}

// для framebufer дисплеев - вытолкнуть кэш память
// Функция используется только в тестах и для выдачи аварийных сообщений.
// Ждать синхронизации дисплея не требуется.
void display_flush(void)
{
	const uintptr_t frame = (uintptr_t) colmain_fb_draw();
//	char s [32];
//	local_snprintf_P(s, 32, "F=%08lX", (unsigned long) frame);
//	display_at(0, 0, s);
	arm_hardware_flush(frame, (uint_fast32_t) GXSIZE(DIM_X, DIM_Y) * sizeof (PACKEDCOLORMAIN_T));
	arm_hardware_ltdc_main_set_no_vsync(frame);
}

/* переключаем на следующий фреймбуфер. Модификация этой памяти больше производиться не будет. */
void display_nextfb(void)
{
	const uintptr_t frame = (uintptr_t) colmain_fb_draw();	// Тот буфер, в котором рисовали, станет отображаемым
//	char s [32];
//	local_snprintf_P(s, 32, "B=%08lX ", (unsigned long) frame);
//	display_at(0, 0, s);
	ASSERT((frame % DCACHEROWSIZE) == 0);
	arm_hardware_flush_invalidate(frame, (uint_fast32_t) GXSIZE(DIM_X, DIM_Y) * sizeof (PACKEDCOLORMAIN_T));
	arm_hardware_ltdc_main_set(frame);
	const unsigned page = colmain_fb_next();	// возвращает новый индекс страницы отрисовки
#if WITHOPENVG
	openvg_next(page);
#endif /* WITHOPENVG */
}

/* вызывается при разрешённых прерываниях. */
void display_initialize(void)
{
#if WITHOPENVG
	PACKEDCOLORMAIN_T * frames [LCDMODE_MAIN_PAGES];
	unsigned i;
	for (i = 0; i < LCDMODE_MAIN_PAGES; ++ i)
	{
		frames [i] = fbfX [i];
	}
	openvg_init(frames);
#endif /* WITHOPENVG */
}

/* Разряжаем конденсаторы питания */
void display_uninitialize(void)
{
#if WITHOPENVG
	openvg_deinit();
#endif /* WITHOPENVG */
}

#endif /* LCDMODE_LQ043T3DX02K */
#endif /* LCDMODE_LTDC */

uint_fast8_t
bigfont_decode(uint_fast8_t c)
{
	// '#' - узкий пробел
	if (c == ' ' || c == '#')
		return 11;
	if (c == '_')
		return 10;		// курсор - позиция редактирвания частоты
	if (c == '.')
		return 12;		// точка
	if (c > '9')
		return 10;		// ошибка - курсор - позиция редактирвания частоты
	return c - '0';		// остальные - цифры 0..9
}

uint_fast8_t
smallfont_decode(uint_fast8_t c)
{
	if (c < ' ' || c > 0x7F)
		return '$' - ' ';
	return c - ' ';
}


#if 0
// Используется при выводе на графический индикатор,
// самый маленький шрифт
static void
display_string2(uint_fast8_t xcell, uint_fast8_t ycell, const char * s, uint_fast8_t lowhalf)
{
	char c;
	uint_fast16_t ypix;
	uint_fast16_t xpix = display_wrdata2_begin(xcell, ycell, & ypix);
	while((c = * s ++) != '\0')
		xpix = display_put_char_small2(xpix, ypix, c, lowhalf);
	display_wrdata2_end();
}



// Используется при выводе на графический индикатор,
// самый маленький шрифт
static void
display_string2_P(uint_fast8_t xcell, uint_fast8_t ycell, const FLASHMEM  char * s, uint_fast8_t lowhalf)
{
	char c;

	uint_fast16_t ypix;
	uint_fast16_t xpix = display_wrdata2_begin(xcell, ycell, & ypix);
	while((c = * s ++) != '\0')
		xpix = display_put_char_small2(xpix, xpix, c, lowhalf);
	display_wrdata2_end();
}
#endif
// Используется при выводе на графический индикатор,
static void
display_string(uint_fast8_t xcell, uint_fast8_t ycell, const char * s, uint_fast8_t lowhalf)
{
	savestring = s;
	savewhere = __func__;
	char c;

	uint_fast16_t ypix;
	uint_fast16_t xpix = display_wrdata_begin(xcell, ycell, & ypix);
	while((c = * s ++) != '\0')
		xpix = display_put_char_small(xpix, ypix, c, lowhalf);
	display_wrdata_end();
}

// Выдача строки из ОЗУ в указанное место экрана.
void
//NOINLINEAT
display_at(uint_fast8_t x, uint_fast8_t y, const char * s)
{
	uint_fast8_t lowhalf = HALFCOUNT_SMALL - 1;
	do
	{
		display_string(x, y + lowhalf, s, lowhalf);

	} while (lowhalf --);
}

// Используется при выводе на графический индикатор,
static void
display_string_P(uint_fast8_t xcell, uint_fast8_t ycell, const FLASHMEM  char * s, uint_fast8_t lowhalf)
{
	char c;

	uint_fast16_t ypix;
	uint_fast16_t xpix = display_wrdata_begin(xcell, ycell, & ypix);
	while((c = * s ++) != '\0')
		xpix = display_put_char_small(xpix, ypix, c, lowhalf);
	display_wrdata_end();
}

// Выдача строки из ПЗУ в указанное место экрана.
void
//NOINLINEAT
display_at_P(uint_fast8_t x, uint_fast8_t y, const FLASHMEM char * s)
{
	uint_fast8_t lowhalf = HALFCOUNT_SMALL - 1;
	do
	{
		display_string_P(x, y + lowhalf, s, lowhalf);

	} while (lowhalf --);
}

static const FLASHMEM int32_t vals10 [] =
{
	1000000000UL,
	100000000UL,
	10000000UL,
	1000000UL,
	100000UL,
	10000UL,
	1000UL,
	100UL,
	10UL,
	1UL,
};

// Отображение цифр в поле "больших цифр" - индикатор основной частоты настройки аппарата.
void
NOINLINEAT
display_value_big(
	uint_fast8_t x,	// x координата начала вывода значения
	uint_fast8_t y,	// y координата начала вывода значения
	uint_fast32_t freq,
	uint_fast8_t width, // = 8;	// full width
	uint_fast8_t comma, // = 2;	// comma position (from right, inside width)
	uint_fast8_t comma2,	// = comma + 3;		// comma position (from right, inside width)
	uint_fast8_t rj,	// = 1;		// right truncated
	uint_fast8_t blinkpos,		// позиция, где символ заменён пробелом
	uint_fast8_t blinkstate,	// 0 - пробел, 1 - курсор
	uint_fast8_t withhalf,		// 0 - только большие цифры
	uint_fast8_t lowhalf		// lower half
	)
{
	//const uint_fast8_t comma2 = comma + 3;		// comma position (from right, inside width)
	const uint_fast8_t j = (sizeof vals10 /sizeof vals10 [0]) - rj;
	uint_fast8_t i = (j - width);
	uint_fast8_t z = 1;	// only zeroes
	uint_fast8_t half = 0;	// отображаем после второй запатой - маленьким шрифтом

	uint_fast16_t ypix;
	uint_fast16_t xpix = display_wrdatabig_begin(x, y, & ypix);
	for (; i < j; ++ i)
	{
		const ldiv_t res = ldiv(freq, vals10 [i]);
		const uint_fast8_t g = (j - i);		// десятичная степень текущего разряда на отображении

		// разделитель десятков мегагерц
		if (comma2 == g)
		{
			xpix = display_put_char_big(xpix, ypix, (z == 0) ? '.' : '#', lowhalf);	// '#' - узкий пробел. Точка всегда узкая
		}
		else if (comma == g)
		{
			z = 0;
			half = withhalf;
			xpix = display_put_char_big(xpix, ypix, '.', lowhalf);
		}

		if (blinkpos == g)
		{
			const uint_fast8_t bc = blinkstate ? '_' : ' ';
			// эта позиция редактирования частоты. Справа от неё включаем все нули
			z = 0;
			if (half)
				xpix = display_put_char_half(xpix, ypix, bc, lowhalf);

			else
				xpix = display_put_char_big(xpix, ypix, bc, lowhalf);
		}
		else if (z == 1 && (i + 1) < j && res.quot == 0)
			xpix = display_put_char_big(xpix, ypix, ' ', lowhalf);	// supress zero
		else
		{
			z = 0;
			if (half)
				xpix = display_put_char_half(xpix, ypix, '0' + res.quot, lowhalf);

			else
				xpix = display_put_char_big(xpix, ypix, '0' + res.quot, lowhalf);
		}
		freq = res.rem;
	}
	display_wrdatabig_end();
}

void
NOINLINEAT
display_value_lower(
	uint_fast8_t x,	// x координата начала вывода значения
	uint_fast8_t y,	// y координата начала вывода значения
	uint_fast32_t freq,
	uint_fast8_t width, // = 8;	// full width
	uint_fast8_t comma, // = 2;	// comma position (from right, inside width)
	uint_fast8_t rj	// = 1;		// right truncated
	)
{
	const uint_fast8_t j = (sizeof vals10 /sizeof vals10 [0]) - rj;
	uint_fast8_t i = (j - width);
	uint_fast8_t z = 1;	// only zeroes
	uint_fast8_t half = 0;	// отображаем после второй запатой - маленьким шрифтом

	uint_fast16_t ypix;
	uint_fast16_t xpix = display_wrdatabig_begin(x, y, & ypix);
	for (; i < j; ++ i)
	{
		const ldiv_t res = ldiv(freq, vals10 [i]);
		const uint_fast8_t g = (j - i);		// десятичная степень текущего разряда на отображении

		if (comma == g || comma + 3 == g)
		{
			z = 0;
			xpix = display_put_char_big(xpix, ypix, '.', 0);
		}

		if (z == 1 && (i + 1) < j && res.quot == 0)
			xpix = display_put_char_big(xpix, ypix, ' ', 0);	// supress zero
		else
		{
			z = 0;
			xpix = display_put_char_half(xpix, ypix, '0' + res.quot, 0);
		}
		freq = res.rem;
	}
	display_wrdatabig_end();
}

void
NOINLINEAT
display_value_small(
	uint_fast8_t x,	// x координата начала вывода значения
	uint_fast8_t y,	// y координата начала вывода значения
	int_fast32_t freq,
	uint_fast8_t width,	// full width (if >= 128 - display with sign)
	uint_fast8_t comma,		// comma position (from right, inside width)
	uint_fast8_t comma2,
	uint_fast8_t rj,		// right truncated
	uint_fast8_t lowhalf
	)
{
	const uint_fast8_t wsign = (width & WSIGNFLAG) != 0;
	const uint_fast8_t wminus = (width & WMINUSFLAG) != 0;
	const uint_fast8_t j = (sizeof vals10 /sizeof vals10 [0]) - rj;
	uint_fast8_t i = j - (width & WWIDTHFLAG);	// Номер цифры по порядку
	uint_fast8_t z = 1;	// only zeroes

	uint_fast16_t ypix;
	uint_fast16_t xpix = display_wrdata_begin(x, y, & ypix);
	if (wsign || wminus)
	{
		// отображение со знаком.
		z = 0;
		if (freq < 0)
		{
			xpix = display_put_char_small(xpix, ypix, '-', lowhalf);
			freq = - freq;
		}
		else if (wsign)
			xpix = display_put_char_small(xpix, ypix, '+', lowhalf);
		else
			xpix = display_put_char_small(xpix, ypix, ' ', lowhalf);
	}
	for (; i < j; ++ i)
	{
		const ldiv_t res = ldiv(freq, vals10 [i]);
		const uint_fast8_t g = (j - i);
		// разделитель десятков мегагерц
		if (comma2 == g)
		{
			xpix = display_put_char_small(xpix, ypix, (z == 0) ? '.' : ' ', lowhalf);
		}
		else if (comma == g)
		{
			z = 0;
			xpix = display_put_char_small(xpix, ypix, '.', lowhalf);
		}

		if (z == 1 && (i + 1) < j && res.quot == 0)
			xpix = display_put_char_small(xpix, ypix, ' ', lowhalf);	// supress zero
		else
		{
			z = 0;
			xpix = display_put_char_small(xpix, ypix, '0' + res.quot, lowhalf);
		}
		freq = res.rem;
	}
	display_wrdata_end();
}

#if LCDMODE_COLORED
static COLORMAIN_T bgcolor = COLORMAIN_BLACK;
#endif /* LCDMODE_COLORED */

void
display_setbgcolor(COLORMAIN_T c)
{
#if LCDMODE_COLORED
	bgcolor = c;
#endif /* LCDMODE_COLORED */
}

COLORMAIN_T
display_getbgcolor(void)
{
#if LCDMODE_COLORED
	return bgcolor;
#else /* LCDMODE_COLORED */
	return COLOR_BLACK;
#endif /* LCDMODE_COLORED */
}


#if LCDMODE_LTDC && (LCDMODE_MAIN_L8 && LCDMODE_PIP_RGB565) || (! LCDMODE_MAIN_L8 && LCDMODE_PIP_L8)

// Выдать буфер на дисплей
// В случае фреймбуфеных дисплеев - формат цвета и там и там одинаковый
// если разный - то заглушка

//#warning colpip_to_main is dummy for this LCDMODE_LTDC combination

void colpip_to_main(
	const PACKEDCOLORPIP_T * buffer,
	uint_fast16_t dx,
	uint_fast16_t dy,
	uint_fast16_t col,	// горизонтальная координата левого верхнего угла на экране (0..dx-1) слева направо
	uint_fast16_t row	// вертикальная координата левого верхнего угла на экране (0..dy-1) сверху вниз
	)
{
	ASSERT(0);
}


// Координаты в пикселях
void display_plotfrom(uint_fast16_t x, uint_fast16_t y)
{
}

#elif LCDMODE_LTDC

// Выдать буфер на дисплей. Функции бывают только для не L8 режимов
// В случае фреймбуфеных дисплеев - формат цвета и там и там одинаковый
void colpip_to_main(
	uintptr_t srcinvalidateaddr,	// параметры clean источника
	int_fast32_t srcinvalidatesize,
	const PACKEDCOLORPIP_T * buffer,	// источник
	uint_fast16_t dx,	// ширина буфера источника
	uint_fast16_t dy,	// высота буфера источника
	uint_fast16_t col,	// целевая горизонтальная координата левого верхнего угла на экране (0..dx-1) слева направо
	uint_fast16_t row	// целевая вертикальная координата левого верхнего угла на экране (0..dy-1) сверху вниз
	)
{
	ASSERT(dx <= DIM_X);
	ASSERT(dy <= DIM_Y);
	ASSERT(((uintptr_t) buffer % DCACHEROWSIZE) == 0);
#if LCDMODE_HORFILL
	hwaccel_copy(
		(uintptr_t) colmain_fb_draw(), sizeof (PACKEDCOLORPIP_T) * GXSIZE(DIM_X, DIM_Y),	// target area invalidate parameters
		colmain_mem_at(colmain_fb_draw(), DIM_X, DIM_Y, col, row), DIM_X, DIM_Y,
		srcinvalidateaddr, srcinvalidatesize,	// параметры clean источника
		buffer, dx, dy
		);
#else /* LCDMODE_HORFILL */
	hwaccel_copy(
		(uintptr_t) colmain_fb_draw(), sizeof (PACKEDCOLORPIP_T) * GXSIZE(DIM_X, DIM_Y),	// target area invalidate parameters
		colmain_mem_at(colmain_fb_draw(), DIM_X, DIM_Y, col, row), DIM_X, DIM_Y,
		srcinvalidateaddr, srcinvalidatesize,	// параметры clean источника
		buffer, dx, dy
		);
#endif /* LCDMODE_HORFILL */
}

// Координаты в пикселях
void display_plotfrom(uint_fast16_t x, uint_fast16_t y)
{
}

#else
#endif /*  */

#if WITHLTDCHW

#if LCDMODE_LQ043T3DX02K
	// Sony PSP-1000 display panel
	// LQ043T3DX02K panel (272*480)
	// RK043FN48H-CT672B  panel (272*480) - плата STM32F746G-DISCO
	/**
	  * @brief  RK043FN48H Size
	  */
const videomode_t vdmode0 =
{
	.width = 480,				/* LCD PIXEL WIDTH            */
	.height = 272,			/* LCD PIXEL HEIGHT           */
	/**
	  * @brief  RK043FN48H Timing
	  */
	.hsync = 41,				/* Horizontal synchronization */
	.hbp = 2,				/* Horizontal back porch      */
	.hfp = 2,				/* Horizontal front porch     */

	.vsync = 10,				/* Vertical synchronization   */
	.vbp = 2,					/* Vertical back porch        */
	.vfp = 4,					/* Vertical front porch       */

	// MODE: DE/SYNC mode select.
	// DE MODE: MODE="1", VS and HS must pull high.
	// SYNC MODE: MODE="0". DE must be grounded
	.vsyncneg = 1,			/* Negative polarity required for VSYNC signal */
	.hsyncneg = 1,			/* Negative polarity required for HSYNC signal */
	.deneg = 0,				/* Negative DE polarity: (normal: DE is 0 while sync) */
	.lq43reset = 1,		/* требуется формирование сигнала RESET для панели по этому выводу после начала формирования синхронизации */
	//.ltdc_dotclk = 9000000uL,	// частота пикселей при работе с интерфейсом RGB
	.fps = 60	/* frames per secound */
};
	/* SONY PSP-1000 display (4.3") required. */
	/* Используется при BOARD_DEMODE = 0 */
	//#define BOARD_DERESET 1		/* требуется формирование сигнала RESET для панели по этому выводу после начала формирования синхронизации */

#elif LCDMODE_AT070TN90

	/* AT070TN90 panel (800*480) - 7" display HV mode */
const videomode_t vdmode0 =
{
	.width = 800,			/* LCD PIXEL WIDTH            */
	.height = 480,			/* LCD PIXEL HEIGHT           */
	/**
	  * @brief  AT070TN90 Timing
	  * MODE=0 (DE)
	  * When selected DE mode, VSYNC & HSYNC must pulled HIGH
	  * MODE=1 (SYNC)
	  * When selected sync mode, de must be grounded.
	  */
	.hsync = 40,				/* Horizontal synchronization 1..40 */
	.hbp = 6,				/* Horizontal back porch      */
	.hfp = 210,				/* Horizontal front porch  16..354   */

	.vsync = 20,				/* Vertical synchronization 1..20  */
	.vbp = 3,				/* Vertical back porch      */
	.vfp = 22,				/* Vertical front porch  7..147     */

	/* Accumulated parameters for this display */
	//LEFTMARGIN = 46,		/* horizontal blanking EXACTLY */
	//TOPMARGIN = 23,			/* vertical blanking EXACTLY */

	// MODE: DE/SYNC mode select.
	// DE MODE: MODE="1", VS and HS must pull high.
	// SYNC MODE: MODE="0". DE must be grounded
	.vsyncneg = 1,			/* Negative polarity required for VSYNC signal */
	.hsyncneg = 1,			/* Negative polarity required for HSYNC signal */
	.deneg = 0,				/* Negative DE polarity: (normal: DE is 0 while sync) */
	.lq43reset = 0,	// LQ043T3DX02K require DE reset
	//.ltdc_dotclk = 30000000uL,	// частота пикселей при работе с интерфейсом RGB
	.fps = 60	/* frames per secound */
};

#elif 1 && LCDMODE_AT070TNA2

/* AT070TNA2 panel (1024*600) - 7" display HV mode */
// HX8282-A01.pdf, page 38
const videomode_t vdmode0 =
{
	.width = 1024,			/* LCD PIXEL WIDTH            */
	.height = 600,			/* LCD PIXEL HEIGHT           */
	/**
	  * @brief  AT070TN90 Timing
	  * MODE=0 (DE)
	  * When selected DE mode, VSYNC & HSYNC must pulled HIGH
	  * MODE=1 (SYNC)
	  * When selected sync mode, de must be grounded.
	  */
	.hsync = 140,			/* Horizontal synchronization 1..140 */
	.hbp = 20,				/* Horizontal back porch  xxx   */
	.hfp = 160,				/* Horizontal front porch  16..216   */

	.vsync = 20,				/* Vertical synchronization 1..20  */
	.vbp = 3,				/* Vertical back porch  xxx   */
	.vfp = 12,				/* Vertical front porch  1..127     */

	/* Accumulated parameters for this display */
	//LEFTMARGIN = 160,		/* horizontal blanking EXACTLY */
	//TOPMARGIN = 23,			/* vertical blanking EXACTLY */

	// MODE: DE/SYNC mode select.
	// DE MODE: MODE="1", VS and HS must pull high.
	// SYNC MODE: MODE="0". DE must be grounded
	.vsyncneg = 1,			/* Negative polarity required for VSYNC signal */
	.hsyncneg = 1,			/* Negative polarity required for HSYNC signal */
	.deneg = 0,				/* Negative DE polarity: (normal: DE is 0 while sync) */
	.lq43reset = 0,	// LQ043T3DX02K require DE reset
	//.ltdc_dotclk = 51200000uL,	// частота пикселей при работе с интерфейсом RGB 40.8..67.2
	.fps = 60	/* frames per secound */
};


#elif 0 && LCDMODE_AT070TNA2
	// 1280 * 720
	/* AT070TNA2 panel (1024*600) - 7" display HV mode */
	// HX8282-A01.pdf, page 38
	//	pinfo.xres = 1280;
	//	pinfo.yres = 720;
	//	pinfo.type = HDMI_PANEL;
	//	pinfo.pdest = DISPLAY_1;
	//	pinfo.wait_cycle = 0;
	//	pinfo.bpp = 24;
	//	pinfo.fb_num = 2;
	//	pinfo.clk_rate = 74250000;
	//	pinfo.lcdc.h_back_porch = 124;
	//	pinfo.lcdc.h_front_porch = 110;
	//	pinfo.lcdc.h_pulse_width = 136;
	//	pinfo.lcdc.v_back_porch = 19;
	//	pinfo.lcdc.v_front_porch = 5;
	//	pinfo.lcdc.v_pulse_width = 6;
	//	pinfo.lcdc.border_clr = 0;
	//	pinfo.lcdc.underflow_clr = 0xff;
	//	pinfo.lcdc.hsync_skew = 0;
const videomode_t vdmode0 =
{
	.width = 1280,			/* LCD PIXEL WIDTH            */
	.height = 720,			/* LCD PIXEL HEIGHT           */
	/**
	  * @brief  AT070TN90 Timing
	  * MODE=0 (DE)
	  * When selected DE mode, VSYNC & HSYNC must pulled HIGH
	  * MODE=1 (SYNC)
	  * When selected sync mode, de must be grounded.
	  */
	.hsync = 136,			/* Horizontal synchronization 1..140 */
	.hbp = xxx,				/* Horizontal back porch  xxx   */
	.hfp = 110,				/* Horizontal front porch  16..216   */

	.vsync = 6,				/* Vertical synchronization 1..20  */
	.vbp = xx,				/* Vertical back porch  xxx   */
	.vfp = 5,				/* Vertical front porch  1..127     */

	/* Accumulated parameters for this display */
	//LEFTMARGIN = 160,		/* horizontal blanking EXACTLY */
	//TOPMARGIN = 23,			/* vertical blanking EXACTLY */

	// MODE: DE/SYNC mode select.
	// DE MODE: MODE="1", VS and HS must pull high.
	// SYNC MODE: MODE="0". DE must be grounded
	.vsyncneg = 1,			/* Negative polarity required for VSYNC signal */
	.hsyncneg = 1,			/* Negative polarity required for HSYNC signal */
	.deneg = 0,				/* Negative DE polarity: (normal: DE is 0 while sync) */
	.lq43reset = 0,	// LQ043T3DX02K require DE reset
	//.ltdc_dotclk = 74250000uL,	// частота пикселей при работе с интерфейсом RGB
	.fps = 60	/* frames per secound */
};

#elif LCDMODE_ILI8961
	// HHT270C-8961-6A6 (320*240)
const videomode_t vdmode0 =
{
	.width = 320 * 3,				/* LCD PIXEL WIDTH            */
	.height = 240,			/* LCD PIXEL HEIGHT           */

	/**
	  * @brief  RK043FN48H Timing
	  */
	.hsync = 1,				/* Horizontal synchronization */
	.hbp = 2,				/* Horizontal back porch      */
	.hfp = 2,				/* Horizontal front porch     */

	.vsync = 1,				/* Vertical synchronization   */
	.vbp = 2,					/* Vertical back porch        */
	.vfp = 2,					/* Vertical front porch       */

	.vsyncneg = 1,			/* Negative polarity required for VSYNC signal */
	.hsyncneg = 1,			/* Negative polarity required for HSYNC signal */
	.deneg = 0,				/* DE polarity: (normal: DE is 0 while sync) */
	.lq43reset = 0,	// LQ043T3DX02K require DE reset
	//.ltdc_dotclk = 24000000uL,	// частота пикселей при работе с интерфейсом RGB
	.fps = 60	/* frames per secound */
};

#elif LCDMODE_ILI9341
	// SF-TC240T-9370-T (320*240)
static const const videomode_t vdmode0 =
{

	.width = 240,				/* LCD PIXEL WIDTH            */
	.height = 320,			/* LCD PIXEL HEIGHT           */

	/**
	  * @brief  ILI9341 Timing
	  */
	.hsync = 10,				/* Horizontal synchronization */
	.hbp = 20,				/* Horizontal back porch      */
	.hfp = 10,				/* Horizontal front porch     */

	.vsync = 2,				/* Vertical synchronization   */
	.vbp = 2,					/* Vertical back porch        */
	.vfp = 4,					/* Vertical front porch       */

	.vsyncneg = 1,			/* Negative polarity required for VSYNC signal */
	.hsyncneg = 1,			/* Negative polarity required for HSYNC signal */
	.deneg = 0,				/* Negative DE polarity: (normal: DE is 0 while sync) */
	.lq43reset = 0,	// LQ043T3DX02K require DE reset
	//.ltdc_dotclk = 3000000uL,	// частота пикселей при работе с интерфейсом RGB
	.fps = 60	/* frames per secound */
};

#elif LCDMODE_H497TLB01P4
	/* 720xRGBx1280 - 5" AMOELD Panel H497TLB01.4 */
	// See also:
	// https://github.com/bbelos/rk3188-kernel/blob/master/drivers/video/rockchip/transmitter/tc358768.c
	// https://github.com/tanish2k09/venom_kernel_aio_otfp/blob/master/drivers/input/touchscreen/mediatek/S3202/synaptics_dsx_i2c.c
	// https://stash.phytec.com/projects/TIRTOS/repos/vps-phytec/raw/src/boards/src/bsp_boardPriv.h?at=e8b92520f41e6523301d120dae15db975ad6d0da
	//https://code.ihub.org.cn/projects/825/repositories/874/file_edit_page?file_name=am57xx-idk-common.dtsi&path=arch%2Farm%2Fboot%2Fdts%2Fam57xx-idk-common.dtsi&rev=master
const videomode_t vdmode0 =
{
	.width = 720,			/* LCD PIXEL WIDTH            */
	.height = 1280,			/* LCD PIXEL HEIGHT           */
	/**
	  * @brief  AT070TN90 Timing
	  * MODE=0 (DE)
	  * When selected DE mode, VSYNC & HSYNC must pulled HIGH
	  * MODE=1 (SYNC)
	  * When selected sync mode, de must be grounded.
	  */
	.hsync = 5,				/* Horizontal synchronization 1..40 */
	.hbp = 11,				/* Horizontal back porch      */
	.hfp = 16,				/* Horizontal front porch  16..354   */

	.vsync = 5,				/* Vertical synchronization 1..20  */
	.vbp = 11,					/* Vertical back porch        */
	.vfp = 16,				/* Vertical front porch  7..147     */

	// MODE: DE/SYNC mode select.
	// DE MODE: MODE="1", VS and HS must pull high.
	// SYNC MODE: MODE="0". DE must be grounded
	.vsyncneg = 1,			/* Negative polarity required for VSYNC signal */
	.hsyncneg = 1,			/* Negative polarity required for HSYNC signal */
	.deneg = 0,				/* Negative DE polarity: (normal: DE is 0 while sync) */
	.lq43reset = 0,	// LQ043T3DX02K require DE reset
	//.ltdc_dotclk = 3000000uL	// частота пикселей при работе с интерфейсом RGB
	.fps = 60	/* frames per secound */
};

#else
	#error Unsupported LCDMODE_xxx

#endif

#endif /* WITHLTDCHW */
/*
 * настройка портов для последующей работы с дополнительными (кроме последовательного канала)
 * сигналами дисплея.
 */

/* вызывается при запрещённых прерываниях. */
void display_hardware_initialize(void)
{
	PRINTF(PSTR("display_hardware_initialize start\n"));


#if WITHDMA2DHW
	// Image construction hardware
	arm_hardware_dma2d_initialize();

#endif /* WITHDMA2DHW */
#if WITHMDMAHW
	// Image construction hardware
	arm_hardware_mdma_initialize();

#endif /* WITHMDMAHW */

#if WITHLTDCHW
	const videomode_t * const vdmode = & vdmode0;
	colmain_fb_initialize();
	uintptr_t frames [LCDMODE_MAIN_PAGES];
	unsigned i;
	for (i = 0; i < LCDMODE_MAIN_PAGES; ++ i)
	{
		frames [i] = (uintptr_t) fbfX [i];
	}
	// STM32xxx LCD-TFT Controller (LTDC)
	// RENESAS Video Display Controller 5
	//PRINTF("display_getdotclock=%lu\n", (unsigned long) display_getdotclock(vdmode));
	arm_hardware_ltdc_initialize(frames, vdmode);
	colmain_setcolors(COLORMAIN_WHITE, COLORMAIN_BLACK);
	arm_hardware_ltdc_main_set((uintptr_t) colmain_fb_draw());
	arm_hardware_ltdc_L8_palette();
#endif /* WITHLTDCHW */

#if LCDMODETX_TC358778XBG
	tc358768_initialize(vdmode);
	panel_initialize(vdmode);
#endif /* LCDMODETX_TC358778XBG */
#if LCDMODEX_SII9022A
	/* siiI9022A Lattice Semiconductor Corp HDMI Transmitter */
	sii9022x_initialize(vdmode);
#endif /* LCDMODEX_SII9022A */

#if LCDMODE_HARD_SPI
	DISPLAY_BUS_INITIALIZE();

#elif LCDMODE_HARD_I2C
	DISPLAY_BUS_INITIALIZE();
#elif LCDMODE_LTDC
	DISPLAY_BUS_INITIALIZE();
#else
	#if LCDMODE_HD44780 && (LCDMODE_SPI == 0)
		hd44780_io_initialize();
	#else /* LCDMODE_HD44780 && (LCDMODE_SPI == 0) */
		DISPLAY_BUS_INITIALIZE();	// see LCD_CONTROL_INITIALIZE, LCD_DATA_INITIALIZE_WRITE
	#endif /* LCDMODE_HD44780 && (LCDMODE_SPI == 0) */
#endif
	PRINTF(PSTR("display_hardware_initialize done\n"));
}
