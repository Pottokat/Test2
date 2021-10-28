/*
 * spi.c
 *
 *  Created on: 27 окт. 2021 г.
 *      Author: User
 */

#include "hardware.h"
#include "board.h"
#include "spi.h"

void hardware_spi_io_delay(void)
{
	local_delay_us(2);
}

// See ARM PrimeCell Synchronous Serial Port (PL022) Technical Reference Manual
// DDI0194H_ssp_pl022_trm.pdf
void hardware_spi_initialize(void)
{
	unsigned long spispeed = 10uL * 1000 * 1000;
	//unsigned long spispeed = 100uL * 1000;

	CLOCKS->CLK_PERI_CTRL |= CLOCKS_CLK_PERI_CTRL_ENABLE_Msk;
	while ((CLOCKS->CLK_PERI_CTRL & CLOCKS_CLK_PERI_CTRL_ENABLE_Msk) == 0)
		;

	while ((CLOCKS->ENABLED0 & CLOCKS_ENABLED0_clk_sys_spi0_Msk) == 0)
		;
	while ((CLOCKS->ENABLED0 & CLOCKS_ENABLED0_clk_peri_spi0_Msk) == 0)
		;

//	CLOCKS->WAKE_EN1 |= CLOCKS_WAKE_EN1_clk_sys_spi0_Msk;
//	CLOCKS->WAKE_EN1 |= CLOCKS_WAKE_EN1_clk_peri_spi0_Msk;

	SET_BIT(RESETS->RESET, RESETS_RESET_spi0_Msk);
	CLEAR_BIT(RESETS->RESET, RESETS_RESET_spi0_Msk);
	while ((RESETS->RESET_DONE & RESETS_RESET_DONE_spi0_Msk) == 0)
		;

	SPI0->SSPCR0 = 0;
	SPI0->SSPCR1 = 0;

	SPI0->SSPCR0 =
		1*(0x01uL << SPI0_SSPCR0_SPH_Pos) |	// SSPCLKOUT phase
		1*(0x01uL << SPI0_SSPCR0_SPO_Pos) |	// SSPCLKOUT polarity,
		(0x07uL << SPI0_SSPCR0_DSS_Pos) |	// 8 bit
		0;
	SPI0->SSPCR1 =
		(0x00uL << SPI0_SSPCR1_MS_Pos) |	// 0: master
		0;

	SPI0->SSPCR1 |= SPI0_SSPCR1_SSE_Msk;

	const uint_fast32_t divider = (rp20xx_get_clk_peri_freq() / spispeed / 2) * 2;

	SPI0->SSPCPSR = (SPI0->SSPCPSR & ~ (SPI0_SSPCPSR_CPSDVSR_Msk)) |
			((divider << SPI0_SSPCPSR_CPSDVSR_Pos) & SPI0_SSPCPSR_CPSDVSR_Msk) |
			0;

	HARDWARE_SPI0_INITIALIZE();
}

void hardware_spi_b8_p1(portholder_t v)	/* передача первого байта в последовательности */
{
	SPI0->SSPDR = v & 0xFF;
}

void hardware_spi_b8_p2(portholder_t v)	/* дождаться готовности, передача байта */
{
	while ((SPI0->SSPSR & SPI0_SSPSR_RNE_Msk) == 0)
		;
	(void) SPI0->SSPDR;
	SPI0->SSPDR = v & 0xFF;
}

portholder_t hardware_spi_complete_b8(void)	/* дождаться готовности передача 8-ти бит */
{
	portholder_t v;
	while ((SPI0->SSPSR & SPI0_SSPSR_RNE_Msk) == 0)
			;
	v = SPI0->SSPDR & 0xFF;
	while ((SPI0->SSPSR & SPI0_SSPSR_BSY_Msk) != 0)
			;
	return v;
}

void hardware_spi_connect(uint_fast8_t spispeedindex, spi_modes_t spimode)	/* управление состоянием - подключено */
{
	//gpio0_pin_setstate(BOARD_ILI9341_GPIO_CSN, 0);
}

void hardware_spi_disconnect(void)	/* управление состоянием - отключено */
{
	while ((SPI0->SSPSR & SPI0_SSPSR_BSY_Msk) != 0)
			;
	//gpio0_pin_setstate(BOARD_ILI9341_GPIO_CSN, 1);
}

void prog_select_impl(
	spitarget_t target	/* SHIFTED addressing to chip (on ATMEGA - may be bit mask) */
	)
{
	//PRINTF("prog_select_impl: state=%d\n", 0);
	gpio0_pin_setstate(BOARD_ILI9341_GPIO_CSN, 0);
	hardware_spi_io_delay();
}

void prog_unselect_impl(void)
{
	//PRINTF("prog_unselect_impl: state=%d\n", 1);
	gpio0_pin_setstate(BOARD_ILI9341_GPIO_CSN, 1);
	hardware_spi_io_delay();
}

