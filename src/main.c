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


