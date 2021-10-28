/*
 * hardware.h
 *
 *  Created on: 27 окт. 2021 г.
 *      Author: User
 */

#ifndef HARDWARE_H_
#define HARDWARE_H_

#define CPUSTYLE_RP20XX	1
#define WITHDEBUG 1

#define WITHSPIHW 1
#define WITHUART2HW 1
#define WITHDEBUG_USART2 1

#include "RP20xx.h"

#include "gpio2.h"
#include "serial2.h"
#include "clocks.h"

#define FLASHMEM
#define NOINLINEAT
#define PSTR(s) s

typedef uint_fast32_t portholder_t;
typedef uint_fast32_t spitarget_t;


void local_delay_us(int timeUS);
void local_delay_ms(int timeMS);


#endif /* HARDWARE_H_ */
