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

int dbgchaq(int c)
{
	if (c == '\n')
		dbgchaq('\r');
	while (hardware_uart2_putchar(c) == 0)
		;
	return c;
}
void putstrq(const char * s)
{
	while (* s)
		dbgchaq(* s ++);
}


#define GPIO_LED 25

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

void spi0_initialize(void)
{
	unsigned long spispeed = 100uL * 1000;

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
		(0x01uL << SPI0_SSPCR0_SPH_Pos) |	// SSPCLKOUT phase
		(0x01uL << SPI0_SSPCR0_SPO_Pos) |	// SSPCLKOUT polarity,
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
	while ((SPI0->SSPSR && SPI0_SSPSR_RNE_Msk) == 0)
		;
	(void) SPI0->SSPDR;
	SPI0->SSPDR = v & 0xFF;
}

portholder_t hardware_spi_complete_b8(void)	/* дождаться готовности передача 8-ти бит */
{
	while ((SPI0->SSPSR & SPI0_SSPSR_RNE_Msk) == 0)
			;
	return SPI0->SSPDR & 0xFF;
}

void hardware_spi_connect(uint_fast8_t spispeedindex, spi_modes_t spimode)	/* управление состоянием - подключено */
{
	//gpio0_pin_setstate(BOARD_ILI9341_GPIO_CSN, 0);
}

void hardware_spi_disconnect(void)	/* управление состоянием - отключено */
{
	//gpio0_pin_setstate(BOARD_ILI9341_GPIO_CSN, 1);
}

void prog_select_impl(
	spitarget_t target	/* SHIFTED addressing to chip (on ATMEGA - may be bit mask) */
	)
{
	PRINTF("prog_select_impl: state=%d\n", 0);
	gpio0_pin_setstate(BOARD_ILI9341_GPIO_CSN, 0);
}

void prog_unselect_impl(void)
{
	PRINTF("prog_unselect_impl: state=%d\n", 1);
	gpio0_pin_setstate(BOARD_ILI9341_GPIO_CSN, 1);
}

void board_update(void)
{

}

///////////////////////////

void board_lcd_rs(uint_fast8_t state)
{
	PRINTF("board_lcd_rs: state=%d\n", state);
	gpio0_pin_setstate(BOARD_ILI9341_GPIO_CD, state);
}

void board_lcd_reset(uint_fast8_t state)
{
	PRINTF("board_lcd_reset: state=%d\n", state);
	gpio0_pin_setstate(BOARD_ILI9341_GPIO_RESET, state);
}

///////////////////////////

// количество циклов на микросекунду
static unsigned long
local_delay_uscycles(unsigned timeUS, unsigned cpufreq_MHz)
{
#if CPUSTYLE_AT91SAM7S
	#warning TODO: calibrate constant	 looks like CPUSTYLE_STM32MP1
	const unsigned long top = timeUS * 175uL / cpufreq_MHz;
	//const unsigned long top = 55 * cpufreq_MHz * timeUS / 1000;
#elif CPUSTYLE_ATSAM3S
	#warning TODO: calibrate constant looks like CPUSTYLE_STM32MP1
	const unsigned long top = timeUS * 270uL / cpufreq_MHz;
	//const unsigned long top = 55 * cpufreq_MHz * timeUS / 1000;
#elif CPUSTYLE_ATSAM4S
	#warning TODO: calibrate constant looks like CPUSTYLE_STM32MP1
	const unsigned long top = timeUS * 270uL / cpufreq_MHz;
	//const unsigned long top = 55 * cpufreq_MHz * timeUS / 1000;
#elif CPUSTYLE_STM32F0XX
	#warning TODO: calibrate constant looks like CPUSTYLE_STM32MP1
	const unsigned long top = timeUS * 190uL / cpufreq_MHz;
	//const unsigned long top = 55 * cpufreq_MHz * timeUS / 1000;
#elif CPUSTYLE_RP20XX
	const unsigned long top = timeUS * 20uL / cpufreq_MHz;
#elif CPUSTYLE_STM32L0XX
	#warning TODO: calibrate constant looks like CPUSTYLE_STM32MP1
	const unsigned long top = timeUS * 20uL / cpufreq_MHz;
	//const unsigned long top = 55 * cpufreq_MHz * timeUS / 1000;
#elif CPUSTYLE_STM32F1XX
	#warning TODO: calibrate constant looks like CPUSTYLE_STM32MP1
	const unsigned long top = timeUS * 345uL / cpufreq_MHz;
	//const unsigned long top = 55 * cpufreq_MHz * timeUS / 1000;
#elif CPUSTYLE_STM32F30X
	#warning TODO: calibrate constant looks like CPUSTYLE_STM32MP1
	const unsigned long top = timeUS * 430uL / cpufreq_MHz;
	//const unsigned long top = 55 * cpufreq_MHz * timeUS / 1000;
#elif CPUSTYLE_STM32F4XX
	#warning TODO: calibrate constant looks like CPUSTYLE_STM32MP1
	const unsigned long top = timeUS * 3800uL / cpufreq_MHz;
#elif CPUSTYLE_STM32F7XX
	#warning TODO: calibrate constant looks like CPUSTYLE_STM32MP1
	const unsigned long top = 55uL * cpufreq_MHz * timeUS / 1000;
#elif CPUSTYLE_STM32H7XX
	#warning TODO: calibrate constant looks like CPUSTYLE_STM32MP1
	const unsigned long top = 77uL * cpufreq_MHz * timeUS / 1000;
#elif CPUSTYLE_R7S721
	const unsigned long top = 105uL * cpufreq_MHz * timeUS / 1000;
#elif CPUSTYLE_XC7Z
	const unsigned long top = 125uL * cpufreq_MHz * timeUS / 1000;
#elif CPUSTYLE_XCZU
	const unsigned long top = 125uL * cpufreq_MHz * timeUS / 1000;
#elif CPUSTYPE_ALLWNV3S
	#warning TODO: calibrate constant looks like CPUSTYLE_STM32MP1
	const unsigned long top = 125uL * cpufreq_MHz * timeUS / 1000;
#elif CPUSTYLE_STM32MP1
	// калибровано для 800 МГц процессора
	const unsigned long top = 120uL * cpufreq_MHz * timeUS / 1000;
#elif CPUSTYPE_TMS320F2833X && 1 // RAM code0
	#warning TODO: calibrate constant looks like CPUSTYLE_STM32MP1
	const unsigned long top = timeUS * 760uL / cpufreq_MHz;	// tested @ 100 MHz Execute from RAM
	//const unsigned long top = 55 * cpufreq_MHz * timeUS / 1000;
#elif CPUSTYPE_TMS320F2833X	&& 0	// FLASH code
	#warning TODO: calibrate constant looks like CPUSTYLE_STM32MP1
	const unsigned long top = 55uL * cpufreq_MHz * timeUS / 1000;
#else
	#error TODO: calibrate constant looks like CPUSTYLE_STM32MP1
	const unsigned long top = 55uL * cpufreq_MHz * timeUS / 1000;
#endif
	return top;
}
// Атрибут RAMFUNC_NONILINE убран, так как функция
// используется в инициализации SDRAM на процессорах STM32F746.
// TODO: перекалибровать для FLASH контроллеров.
void /* RAMFUNC_NONILINE */ local_delay_us(int timeUS)
{
	// Частота процессора приволится к мегагерцам.
	const unsigned long top = local_delay_uscycles(timeUS, CPU_FREQ / 1000000uL);
	//
	volatile unsigned long t;
	for (t = 0; t < top; ++ t)
	{
	}
}
// exactly as required
//
void local_delay_ms(int timeMS)
{
	// Частота процессора приволится к мегагерцам.
	const unsigned long top = local_delay_uscycles(1000, CPU_FREQ / 1000000uL);
	int n;
	for (n = 0; n < timeMS; ++ n)
	{
		volatile unsigned long t;
		for (t = 0; t < top; ++ t)
		{
		}
	}
}

void main(void)
{
    SystemCoreClockUpdate();
	gpio0_pin_initialize(GPIO_LED, GPIO_IO_TYPE_OUTPUT, GPIO_NOPULL, GPIO_SPEED_LOW, GPIO_AF_NONE);
	//__disable_irq();
	// Flash green led on GPIO25
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

	spi0_initialize();

	//spi0_initialize();

	PRINTF("Hello!\n");
	PRINTF("Hello2!\n");

	display_hardware_initialize();
	display_reset();
	display_initialize();
	display_clear();

	for (;;)
	{
/*
		if ((SPI0->SSPSR && SPI0_SSPSR_TNF_Msk) != 0)
		{
			SPI0->SSPDR = 0xF0;
		}
		continue;
*/
//		static int line;
//		char s [132];
//		snprintf(s, 132, "1 Hello! line = %d, SystemCoreClock=%lu, SSPSR=%08lX\n", line ++, SystemCoreClock, SPI0->SSPSR);
//		putstrq(s);
	}
}


