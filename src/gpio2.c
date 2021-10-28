/*
 * gpio.c
 *
 *  Created on: Oct 27, 2021
 *      Author: gena
 */

#include "RP20xx.h"

#include "gpio.h"
#include "gpio2.h"
#include "board.h"


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Set GPIO bank 0 pin state to 1
void gpio0_pin_set(unsigned gpio_no)
{
	//__IOM uint32_t * const GPIO_STATUS_REG = (__IOM uint32_t *) (IO_BANK0_BASE + 0x00 + (0x08 * gpio_no));
	__IOM uint32_t * const GPIO_CTRL_REG = (__IOM uint32_t *) (IO_BANK0_BASE + 0x04 + (0x08 * gpio_no));

	MODIFY_REG(* GPIO_CTRL_REG, IO_BANK0_GPIO0_CTRL_OUTOVER_Msk, GPIO_OUTPUT_HIGH);
}

// Set GPIO bank 0 pin state to 0
void gpio0_pin_clr(unsigned gpio_no)
{
	//__IOM uint32_t * const GPIO_STATUS_REG = (__IOM uint32_t *) (IO_BANK0_BASE + 0x00 + (0x08 * gpio_no));
	__IOM uint32_t * const GPIO_CTRL_REG = (__IOM uint32_t *) (IO_BANK0_BASE + 0x04 + (0x08 * gpio_no));

	MODIFY_REG(* GPIO_CTRL_REG, IO_BANK0_GPIO0_CTRL_OUTOVER_Msk, GPIO_OUTPUT_LOW);
}

void gpio0_pin_setstate(unsigned gpio_no, uint_fast8_t state)
{
	__IOM uint32_t * const GPIO_CTRL_REG = (__IOM uint32_t *) (IO_BANK0_BASE + 0x04 + (0x08 * gpio_no));

	MODIFY_REG(* GPIO_CTRL_REG, IO_BANK0_GPIO0_CTRL_OUTOVER_Msk, state ? GPIO_OUTPUT_HIGH : GPIO_OUTPUT_LOW);
}

void gpio0_pin_initialize(uint32_t gpio_no, eGPIO_IO_Type io_type, eGPIO_Pull pu_pd, eGPIO_Speed speed, uint32_t alternate_function)
{
    __IOM uint32_t * const IO_BANK_CTRL_REG = (__IOM uint32_t *)(IO_BANK0_BASE + 0x04 + (0x08 * gpio_no));
    __IOM uint32_t * const PADS_BANK_CTRL_REG = (__IOM uint32_t *)(PADS_BANK0_BASE + 0x04 + (0x04 * gpio_no));

    *PADS_BANK_CTRL_REG = (uint32_t)pu_pd | PADS_BANK0_GPIO0_SCHMITT_Msk |
        (uint32_t) speed | PADS_BANK0_GPIO0_IE_Msk;

    *IO_BANK_CTRL_REG = io_type | (alternate_function << IO_BANK0_GPIO0_CTRL_FUNCSEL_Pos);

    if (io_type == GPIO_IO_TYPE_INPUT) {
        SET_BIT(*PADS_BANK_CTRL_REG, PADS_BANK0_GPIO0_OD_Msk);
    } else {
        CLEAR_BIT(*PADS_BANK_CTRL_REG, PADS_BANK0_GPIO0_OD_Msk);
    }
}

