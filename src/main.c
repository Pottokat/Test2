/*
 * main.c
 *
 *  Created on: 25 окт. 2021 г.
 *      Author: User
 */


#include "RP20xx.h"

#include "gpio2.h"
#include "serial2.h"
//#include "board.h"
#include "clocks.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int dbgchaq(int c)
{
	if (c == '\n')
		dbgchaq('\r');
	while (hardware_uart1_putchar(c) == 0)
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
	if (state)
	{
		gpio0_pin_set(GPIO_LED);
		gpio0_pin_set(21);
	}
	else
	{
		gpio0_pin_clr(GPIO_LED);
		gpio0_pin_clr(21);
	}
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
			gpio0_pin_set(GPIO_LED);
			idelay(t);
			gpio0_pin_clr(GPIO_LED);
			idelay(t);
		}
		idelay(t * 2);
	}
}

void spi0_initialize(void)
{
	unsigned long spispeed = 1000000;

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

	SPI0->SSPCR0 =
		(0x00uL << SPI0_SSPCR0_SCR_Pos) |
		0;
	SPI0->SSPCR1 =
		(0x00uL << SPI0_SSPCR1_SOD_Pos) |
		0;
	const uint_fast32_t divider =  rp20xx_get_clk_peri_freq() / spispeed;

	SPI0->SSPCPSR = (SPI0->SSPCPSR & ~ (SPI0_SSPCPSR_CPSDVSR_Msk)) |
			(divider << SPI0_SSPCPSR_CPSDVSR_Pos) |
			0;

	HARDWARE_SPI0_INITIALIZE();
}

void main(void)
{
    SystemCoreClockUpdate();
	gpio0_pin_initialize(21, GPIO_IO_TYPE_OUTPUT, GPIO_NOPULL, GPIO_SPEED_LOW, GPIO_AF_NONE);
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
	hardware_uart1_initialize(1);
	hardware_uart1_set_speed(115200);

	for (;;)
	{
		static int line;
		char s [132];
		snprintf(s, 132, "Hello! line = %d, SystemCoreClock=%lu\n", line ++, SystemCoreClock);
		putstrq(s);
	}
}


