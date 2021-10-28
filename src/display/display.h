/*
 * display.h
 *
 *  Created on: 27 окт. 2021 г.
 *      Author: User
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "hardware.h"
#include "board.h"


// самый маленький шрифт
uint_fast16_t display_wrdata2_begin(uint_fast8_t xcell, uint_fast8_t ycell, uint_fast16_t * yp);
void display_wrdata2_end(void);
uint_fast16_t display_put_char_small2(uint_fast16_t xpix, uint_fast16_t ypix, uint_fast8_t c, uint_fast8_t lowhalf);
// полоса индикатора
uint_fast16_t display_wrdatabar_begin(uint_fast8_t xcell, uint_fast8_t ycell, uint_fast16_t * yp);
uint_fast16_t display_barcolumn(uint_fast16_t xpix, uint_fast16_t ypix, uint_fast8_t pattern);	// Выдать восемь цветных пикселей, младший бит - самый верхний в растре
void display_wrdatabar_end(void);
// большие и средние цифры (частота)
uint_fast16_t display_wrdatabig_begin(uint_fast8_t xcell, uint_fast8_t ycell, uint_fast16_t * yp);
uint_fast16_t display_put_char_big(uint_fast16_t xpix, uint_fast16_t ypix, uint_fast8_t c, uint_fast8_t lowhalf);
uint_fast16_t display_put_char_half(uint_fast16_t xpix, uint_fast16_t ypix, uint_fast8_t c, uint_fast8_t lowhalf);
void display_wrdatabig_end(void);
// обычный шрифт
uint_fast16_t display_wrdata_begin(uint_fast8_t xcell, uint_fast8_t ycell, uint_fast16_t * yp);
uint_fast16_t display_put_char_small(uint_fast16_t xpix, uint_fast16_t ypix, uint_fast8_t c, uint_fast8_t lowhalf);
void display_wrdata_end(void);


/* Индикатор 160*128 с контроллером Sitronix ST7735 */
/* Индикатор 176*132 с контроллером ILITEK ILI9163 */
/* Индикатор 320*240 с контроллером ILITEK ILI9341 */

#include "st7735.h"

#define HALFCOUNT_BIG 1		// big and half sizes
#define HALFCOUNT_SMALL 1	// small size
#define HALFCOUNT_SMALL2 1	// small2 size
#define HALFCOUNT_BAR 1		// small size

#define HALFCOUNT_FREQA HALFCOUNT_BIG


#define WSIGNFLAG 0x80	// отображается плюс или минус в зависимости от знака значения
#define WMINUSFLAG 0x40	// отображается пробел или минус в зависимости от знака значения
#define WWIDTHFLAG 0x3F	// оставшиеся биты под ширину поля

#define COLORMAIN_BLACK TFTRGB(0, 0, 0)
#define COLORMAIN_WHITE TFTRGB(255, 255, 255)

uint_fast8_t smallfont_decode(uint_fast8_t c);
uint_fast8_t bigfont_decode(uint_fast8_t c);

COLORMAIN_T display_getbgcolor(void);
void display_setbgcolor(COLORMAIN_T c);
void display_clear(void);

// Интерфейсные функции, специфические для драйвера дисплея - зависящие от типа микросхемы контроллера.
void display_hardware_initialize(void);	/* вызывается при запрещённых прерываниях. */
void display_hdmi_initialize(void);
void display_reset(void);				/* вызывается при разрешённых прерываниях. */
void display_initialize(void);			/* вызывается при разрешённых прерываниях. */
void display_uninitialize(void);			/* вызывается при разрешённых прерываниях. */
void display_nextfb(void);				/* переключаем на следующий фреймбуфер */
void display_set_contrast(uint_fast8_t v);
void display_palette(void);				// Palette reload

void display_at(uint_fast8_t x, uint_fast8_t y, const char * s);
void display_at_P(uint_fast8_t x, uint_fast8_t y, const char FLASHMEM * s);
void colmain_setcolors(COLORMAIN_T fg, COLORMAIN_T bg);

#endif /* DISPLAY_H_ */
