/*
 * gpio.h
 *
 *  Created on: Oct 27, 2021
 *      Author: gena
 */

#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_


#include "gpio.h"
#include <stdint.h>

// Column in 2.19.2. Function Select
enum
{
	GPIO_AF_1 = 1,
	GPIO_AF_2,
	GPIO_AF_3,
	GPIO_AF_4,
	GPIO_AF_5,

	//
	GPIO_AF_last
};

void gpio0_pin_set(unsigned gpio_no);
void gpio0_pin_clr(unsigned gpio_no);
void gpio0_pin_initialize(uint32_t gpio_no, eGPIO_IO_Type io_type, eGPIO_Pull pu_pd, eGPIO_Speed speed, uint32_t alternate_function);
void gpio0_pin_setstate(unsigned gpio_no, uint_fast8_t state);

#endif /* SRC_GPIO_H_ */
