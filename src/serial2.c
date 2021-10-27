/*
 * serial.c
 *
 *  Created on: Oct 27, 2021
 *      Author: gena
 */


#include "RP20xx.h"

#include "gpio2.h"
#include "serial2.h"
#include "board.h"
#include "clocks.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#if 1

// RP2040 has 2 identical instances of a UART peripheral,
// based on the ARM Primecell UART (PL011) (Revision r1p5).
void hardware_uart1_initialize(uint_fast8_t debug)
{
	CLOCKS->CLK_PERI_CTRL |= CLOCKS_CLK_PERI_CTRL_ENABLE_Msk;
	while ((CLOCKS->CLK_PERI_CTRL & CLOCKS_CLK_PERI_CTRL_ENABLE_Msk) == 0)
		;

	while ((CLOCKS->ENABLED1 & CLOCKS_ENABLED1_clk_sys_uart0_Msk) == 0)
		;
	while ((CLOCKS->ENABLED1 & CLOCKS_ENABLED1_clk_peri_uart0_Msk) == 0)
		;

//	CLOCKS->WAKE_EN1 |= CLOCKS_WAKE_EN1_clk_sys_uart0_Msk;
//	CLOCKS->WAKE_EN1 |= CLOCKS_WAKE_EN1_clk_peri_uart0_Msk;

	SET_BIT(RESETS->RESET, RESETS_RESET_uart0_Msk);
	CLEAR_BIT(RESETS->RESET, RESETS_RESET_uart0_Msk);
	while ((RESETS->RESET_DONE & RESETS_RESET_DONE_uart0_Msk) == 0)
		;

	UART0->UARTLCR_H =
		(0x00uL << UART0_UARTLCR_H_SPS_Pos) |
		(0x03uL << UART0_UARTLCR_H_WLEN_Pos) |	// 8 bit
		(0x01uL << UART0_UARTLCR_H_FEN_Pos) |	// FIFO enable
		(0x00uL << UART0_UARTLCR_H_STP2_Pos) |	// 1 stop bit
		(0x00uL << UART0_UARTLCR_H_EPS_Pos) |
		(0x00uL << UART0_UARTLCR_H_PEN_Pos) |
		(0x00uL << UART0_UARTLCR_H_BRK_Pos) |
		0;
	UART0->UARTLCR_H = UART0->UARTLCR_H;

	UART0->UARTRSR = 0x0F;

	UART0->UARTCR |= (UART0_UARTCR_RXE_Msk | UART0_UARTCR_TXE_Msk);
	(void) UART0->UARTCR;
	UART0->UARTCR |= UART0_UARTCR_UARTEN_Msk;
	(void) UART0->UARTCR;

	HARDWARE_UART0_INITIALIZE();
}

void hardware_uart1_set_speed(uint_fast32_t baudrate)
{
	const uint_fast32_t rawbr = baudrate * 16;
	const ldiv_t d = ldiv(rp20xx_get_clk_peri_freq(), rawbr);
	const uint_fast32_t divider =  d.quot;
	const uint_fast32_t fractmax = (UART0_UARTFBRD_BAUD_DIVFRAC_Msk >> UART0_UARTFBRD_BAUD_DIVFRAC_Pos) + 1;
	const uint_fast32_t fractional =  d.rem * fractmax / rawbr;

	UART0->UARTIBRD = (UART0->UARTIBRD & ~ (UART0_UARTIBRD_BAUD_DIVINT_Msk)) |
		(divider << UART0_UARTIBRD_BAUD_DIVINT_Pos) |
		0;
	UART0->UARTFBRD = (UART0->UARTFBRD & ~ (UART0_UARTFBRD_BAUD_DIVFRAC_Msk)) |
		(fractional << UART0_UARTFBRD_BAUD_DIVFRAC_Pos) |
		0;
	UART0->UARTLCR_H = UART0->UARTLCR_H;
}

void hardware_uart1_tx(void * ctx, uint_fast8_t c)
{
	UART0->UARTDR = c;
}

void hardware_uart1_enabletx(uint_fast8_t state)
{
	if (state != 0)
	{
		UART0->UARTIMSC |= UART0_UARTIMSC_TXIM_Msk;
	}
	else
	{
		UART0->UARTIMSC &= ~ UART0_UARTIMSC_TXIM_Msk;
	}
}

void hardware_uart1_enablerx(uint_fast8_t state)
{
	if (state != 0)
	{
		UART0->UARTIMSC |= UART0_UARTIMSC_RXIM_Msk;
	}
	else
	{
		UART0->UARTIMSC &= ~ UART0_UARTIMSC_RXIM_Msk;
	}
}

uint_fast8_t hardware_uart1_putchar(uint_fast8_t c)
{
	if ((UART0->UARTFR & UART0_UARTFR_TXFF_Msk) == 0)
	{
		UART0->UARTDR = c & UART0_UARTDR_DATA_Msk;
		return 1;
	}
	return 0;
}

uint_fast8_t hardware_uart1_getchar(char * cp) /* приём символа, если готов порт */
{

	if ((UART0->UARTFR & UART0_UARTFR_RXFE_Msk) == 0)
	{
		* cp = UART0->UARTDR & UART0_UARTDR_DATA_Msk;
		return 1;
	}
	return 0;
}

#endif /* dddd */


#if 1

// RP2040 has 2 identical instances of a UART peripheral,
// based on the ARM Primecell UART (PL011) (Revision r1p5).
void hardware_uart2_initialize(uint_fast8_t debug)
{
	CLOCKS->CLK_PERI_CTRL |= CLOCKS_CLK_PERI_CTRL_ENABLE_Msk;
	while ((CLOCKS->CLK_PERI_CTRL & CLOCKS_CLK_PERI_CTRL_ENABLE_Msk) == 0)
		;

	while ((CLOCKS->ENABLED1 & CLOCKS_ENABLED1_clk_sys_uart1_Msk) == 0)
		;
	while ((CLOCKS->ENABLED1 & CLOCKS_ENABLED1_clk_peri_uart1_Msk) == 0)
		;

//	CLOCKS->WAKE_EN1 |= CLOCKS_WAKE_EN1_clk_sys_uart1_Msk;
//	CLOCKS->WAKE_EN1 |= CLOCKS_WAKE_EN1_clk_peri_uart1_Msk;

	SET_BIT(RESETS->RESET, RESETS_RESET_uart1_Msk);
	CLEAR_BIT(RESETS->RESET, RESETS_RESET_uart1_Msk);
	while ((RESETS->RESET_DONE & RESETS_RESET_DONE_uart1_Msk) == 0)
		;

	UART1->UARTLCR_H =
		(0x00uL << UART0_UARTLCR_H_SPS_Pos) |
		(0x03uL << UART0_UARTLCR_H_WLEN_Pos) |	// 8 bit
		(0x01uL << UART0_UARTLCR_H_FEN_Pos) |	// FIFO enable
		(0x00uL << UART0_UARTLCR_H_STP2_Pos) |	// 1 stop bit
		(0x00uL << UART0_UARTLCR_H_EPS_Pos) |
		(0x00uL << UART0_UARTLCR_H_PEN_Pos) |
		(0x00uL << UART0_UARTLCR_H_BRK_Pos) |
		0;
	UART1->UARTLCR_H = UART1->UARTLCR_H;

	UART1->UARTRSR = 0x0F;

	UART1->UARTCR |= (UART0_UARTCR_RXE_Msk | UART0_UARTCR_TXE_Msk);
	(void) UART1->UARTCR;
	UART1->UARTCR |= UART0_UARTCR_UARTEN_Msk;
	(void) UART1->UARTCR;

	HARDWARE_UART0_INITIALIZE();
}

void hardware_uart2_set_speed(uint_fast32_t baudrate)
{
	const uint_fast32_t rawbr = baudrate * 16;
	const ldiv_t d = ldiv(rp20xx_get_clk_peri_freq(), rawbr);
	const uint_fast32_t divider =  d.quot;
	const uint_fast32_t fractmax = (UART0_UARTFBRD_BAUD_DIVFRAC_Msk >> UART0_UARTFBRD_BAUD_DIVFRAC_Pos) + 1;
	const uint_fast32_t fractional =  d.rem * fractmax / rawbr;

	UART1->UARTIBRD = (UART1->UARTIBRD & ~ (UART0_UARTIBRD_BAUD_DIVINT_Msk)) |
		(divider << UART0_UARTIBRD_BAUD_DIVINT_Pos) |
		0;
	UART1->UARTFBRD = (UART1->UARTFBRD & ~ (UART0_UARTFBRD_BAUD_DIVFRAC_Msk)) |
		(fractional << UART0_UARTFBRD_BAUD_DIVFRAC_Pos) |
		0;
}

void hardware_uart2_tx(void * ctx, uint_fast8_t c)
{
	UART1->UARTDR = c;
}

void hardware_uart2_enabletx(uint_fast8_t state)
{
	if (state != 0)
	{
		UART1->UARTIMSC |= UART0_UARTIMSC_TXIM_Msk;
	}
	else
	{
		UART1->UARTIMSC &= ~ UART0_UARTIMSC_TXIM_Msk;
	}
}

void hardware_uart2_enablerx(uint_fast8_t state)
{
	if (state != 0)
	{
		UART1->UARTIMSC |= UART0_UARTIMSC_RXIM_Msk;
	}
	else
	{
		UART1->UARTIMSC &= ~ UART0_UARTIMSC_RXIM_Msk;
	}
}

uint_fast8_t hardware_uart2_putchar(uint_fast8_t c)
{
	if ((UART1->UARTFR & UART0_UARTFR_TXFF_Msk) == 0)
	{
		UART1->UARTDR = c & UART0_UARTDR_DATA_Msk;
		return 1;
	}
	return 0;
}

uint_fast8_t hardware_uart2_getchar(char * cp) /* приём символа, если готов порт */
{

	if ((UART1->UARTFR & UART0_UARTFR_RXFE_Msk) == 0)
	{
		* cp = UART1->UARTDR & UART0_UARTDR_DATA_Msk;
		return 1;
	}
	return 0;
}

#endif /* dddd */

