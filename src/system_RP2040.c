/**************************************************************************/ /**
 * @file     system_ARMCM0plus.c
 * @brief    CMSIS Device System Source File for
 *           ARMCM0plus Device
 * @version  V1.0.0
 * @date     09. July 2018
 ******************************************************************************/
/*
 * Copyright (c) 2009-2018 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "RP20XX.h"
#include "system_RP2040.h"
#include "cmsis_compiler.h"
#include "board.h"

/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/

#define SYSTEM_CLOCK (XOSC_VALUE)

/*----------------------------------------------------------------------------
  System Core Clock Variable
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = SYSTEM_CLOCK; /* System Core Clock Frequency */

#define CLK_REF_SRC_ROSC 0x00
#define CLK_REF_SRC_AUX  0x01
#define CLK_REF_SRC_XOSC 0x02

/*----------------------------------------------------------------------------
  System Core Clock update function
 *----------------------------------------------------------------------------*/

void SystemCoreClockUpdate(void)
{
    uint32_t clock_source = CLOCKS->CLK_REF_CTRL;

   switch ((clock_source & CLOCKS_CLK_REF_CTRL_SRC_Msk) >> CLOCKS_CLK_REF_CTRL_SRC_Pos) {
        case CLK_REF_SRC_ROSC:
            // We can't directly calculate ROSC frequency, we have to use other stable source
            // In this case we use known frequency of XOSC
            CLOCKS->FC0_REF_KHZ  = XOSC_VALUE / 1000;
            CLOCKS->FC0_MIN_KHZ  = 0;
            CLOCKS->FC0_MAX_KHZ  = 0;
            CLOCKS->FC0_INTERVAL = 4;    // 1us * 2 * value
            // Since this gets more complicated we don't support this source now
            SystemCoreClock = 6000000;
             break;
        case CLK_REF_SRC_XOSC:
			{
			    uint32_t ref_div;
				ref_div = (PLL_SYS->CS & PLL_SYS_CS_REFDIV_Msk) >> PLL_SYS_CS_REFDIV_Pos;
				if (PLL_SYS->CS & PLL_SYS_CS_BYPASS_Msk) {
					SystemCoreClock = XOSC_VALUE / ref_div;
			   } else {
				    const uint32_t SCALE = 1;
					uint32_t fb_div;
					uint32_t postdiv1;
					uint32_t postdiv2;
					// Calculation from datasheet
					// (FREF / REFDIV) × FBDIV / (POSTDIV1 × POSTDIV2)
					fb_div          = (PLL_SYS->FBDIV_INT & PLL_SYS_FBDIV_INT_FBDIV_INT_Msk) >> PLL_SYS_FBDIV_INT_FBDIV_INT_Pos;
					postdiv1        = (PLL_SYS->PRIM & PLL_SYS_PRIM_POSTDIV1_Msk) >> PLL_SYS_PRIM_POSTDIV1_Pos;	// 1..7
					postdiv2        = (PLL_SYS->PRIM & PLL_SYS_PRIM_POSTDIV2_Msk) >> PLL_SYS_PRIM_POSTDIV2_Pos;	// 1..7
					SystemCoreClock = (XOSC_VALUE / SCALE / ref_div) * fb_div / (postdiv1 * postdiv2) * SCALE;
				}
			}
           break;
        case CLK_REF_SRC_AUX:
           break;

        default:
            break;

    }

    return;
}

/*----------------------------------------------------------------------------
  System initialization function
 *----------------------------------------------------------------------------*/
extern const VECTOR_TABLE_Type __VECTOR_TABLE[48];

static void rp20xx_pll_sys_initialize(void)
{

	PLL_SYS->PWR = 0x0;
	while (PLL_SYS->PWR != 0)
		;

	PLL_SYS->CS |= PLL_SYS_CS_BYPASS_Msk;
	while ((PLL_SYS->CS & PLL_SYS_CS_BYPASS_Msk) == 0)
		;

	// Set PLL parameters
	PLL_SYS->CS = (PLL_SYS->CS & ~ (PLL_SYS_CS_REFDIV_Msk)) |
			((uint32_t) PLL1_REFDIV << PLL_SYS_CS_REFDIV_Pos) |
			0;
	PLL_SYS->FBDIV_INT = (PLL_SYS->FBDIV_INT & ~ (PLL_SYS_FBDIV_INT_FBDIV_INT_Msk))	|
			((uint32_t) PLL1_MUL << PLL_SYS_FBDIV_INT_FBDIV_INT_Pos) |
			0;
	PLL_SYS->PRIM = (PLL_SYS->PRIM & ~ (PLL_SYS_PRIM_POSTDIV1_Msk | PLL_SYS_PRIM_POSTDIV2_Msk)) |
			((uint32_t) PLL1_DIV1 << PLL_SYS_PRIM_POSTDIV1_Pos) |
			((uint32_t) PLL1_DIV2 << PLL_SYS_PRIM_POSTDIV2_Pos) |
			0;

	PLL_SYS->CS &= ~ PLL_SYS_CS_BYPASS_Msk;
	while ((PLL_SYS->CS & PLL_SYS_CS_BYPASS_Msk) != 0)
		;

	/* Wait for PLL lock state */
	while ((PLL_SYS->CS & PLL_SYS_CS_LOCK_Msk) == 0)
		;

}

static void rp20xx_pll_usb_initialize(void)
{

	PLL_USB->PWR = 0x0;
	while (PLL_USB->PWR != 0)
		;
	PLL_USB->CS |= PLL_SYS_CS_BYPASS_Msk;
	while ((PLL_USB->CS & PLL_SYS_CS_BYPASS_Msk) == 0)
		;

	// Set PLL parameters
	PLL_USB->CS = (PLL_USB->CS & ~ (PLL_SYS_CS_REFDIV_Msk)) |
			((uint32_t) PLLUSB_REFDIV << PLL_SYS_CS_REFDIV_Pos) |
			0;
	PLL_USB->FBDIV_INT = (PLL_USB->FBDIV_INT & ~ (PLL_SYS_FBDIV_INT_FBDIV_INT_Msk))	|
			((uint32_t) PLLUSB_MUL << PLL_SYS_FBDIV_INT_FBDIV_INT_Pos) |
			0;
	PLL_USB->PRIM = (PLL_USB->PRIM & ~ (PLL_SYS_PRIM_POSTDIV1_Msk | PLL_SYS_PRIM_POSTDIV2_Msk)) |
			((uint32_t) PLLUSB_DIV1 << PLL_SYS_PRIM_POSTDIV1_Pos) |
			((uint32_t) PLLUSB_DIV2 << PLL_SYS_PRIM_POSTDIV2_Pos) |
			0;

	PLL_USB->CS &= ~ PLL_SYS_CS_BYPASS_Msk;
	while ((PLL_USB->CS & PLL_SYS_CS_BYPASS_Msk) != 0)
		;

	/* Wait for PLL lock state */
	while ((PLL_USB->CS & PLL_SYS_CS_LOCK_Msk) == 0)
		;

}

void SystemInit(void)
 {
#if defined(__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
	SCB->VTOR = (uint32_t) &(__VECTOR_TABLE);
#endif

	// We only want to set XOSC to run on our board
	// Setup XOSC range
	XOSC->CTRL = 0xaa0; // 1_15Mhz
	// Setup startup delay
	uint32_t startup_delay = (((12 * 1000000) / 1000) + 128) / 256;
	XOSC->STARTUP = startup_delay;
	// Enable XOSC
	XOSC->CTRL = (XOSC->CTRL & ~XOSC_CTRL_ENABLE_Msk)
			| (0x0FAB << XOSC_CTRL_ENABLE_Pos) | 0;
	while ((XOSC->STATUS & XOSC_STATUS_ENABLED_Msk) == 0)
		;

	// Change from using PLL to CLK_REF
	CLOCKS->CLK_REF_CTRL = (CLOCKS->CLK_REF_CTRL & ~(CLOCKS_CLK_REF_CTRL_SRC_Msk | 0*CLOCKS_CLK_REF_CTRL_AUXSRC_Msk)) |
			(0x00 << CLOCKS_CLK_REF_CTRL_AUXSRC_Pos) |
			(0x00 << CLOCKS_CLK_REF_CTRL_SRC_Pos) |
			0;
	while (CLOCKS->CLK_REF_SELECTED == 0)
		;

	// First bypass PLL
	RESETS->RESET |= RESETS_RESET_pll_sys_Msk;
	RESETS->RESET &= ~RESETS_RESET_pll_sys_Msk;
	while (!(RESETS->RESET_DONE & RESETS_RESET_DONE_pll_sys_Msk))
		;

	rp20xx_pll_sys_initialize();
	//rp20xx_pll_usb_initialize();

	// Change from using PLL to CLK_REF
	CLOCKS->CLK_REF_CTRL = (CLOCKS->CLK_REF_CTRL & ~(CLOCKS_CLK_REF_CTRL_SRC_Msk | 0*CLOCKS_CLK_REF_CTRL_AUXSRC_Msk)) |
			//(0x00 << CLOCKS_CLK_REF_CTRL_AUXSRC_Pos) |
			(0x02 << CLOCKS_CLK_REF_CTRL_SRC_Pos) |	// 0x2 → xosc_clksrc
			0;
	while (CLOCKS->CLK_REF_SELECTED == 0)
		;

	CLOCKS->CLK_PERI_CTRL = (CLOCKS->CLK_PERI_CTRL & ~ (CLOCKS_CLK_PERI_CTRL_AUXSRC_Msk)) |
			(0x01 << CLOCKS_CLK_PERI_CTRL_AUXSRC_Pos) |
			0;
	while ((CLOCKS->CLK_PERI_SELECTED == 0))
		;

	CLOCKS->CLK_SYS_CTRL = (CLOCKS->CLK_SYS_CTRL & ~ (CLOCKS_CLK_SYS_CTRL_SRC_Msk | CLOCKS_CLK_SYS_CTRL_AUXSRC_Msk)) |
			//(0x03uL << CLOCKS_CLK_SYS_CTRL_AUXSRC_Pos) |	// 0x3 → xosc_clksrc
			(0x00uL << CLOCKS_CLK_SYS_CTRL_AUXSRC_Pos) |	// 0x0 → clksrc_pll_sys
			(0x01uL << CLOCKS_CLK_SYS_CTRL_SRC_Pos) |		// 0x1 → clksrc_clk_sys_aux
			0;
	while (CLOCKS->CLK_SYS_SELECTED == 0)
		;

	// Disable all clocks
	CLOCKS->ENABLED0 = 0x0;
	CLOCKS->ENABLED1 = 0x0;

	SET_BIT(RESETS->RESET, RESETS_RESET_pads_bank0_Msk);
	CLEAR_BIT(RESETS->RESET, RESETS_RESET_pads_bank0_Msk);
	while (!(RESETS->RESET_DONE & RESETS_RESET_DONE_pads_bank0_Msk))
		;

	SET_BIT(RESETS->RESET, RESETS_RESET_io_bank0_Msk);
	CLEAR_BIT(RESETS->RESET, RESETS_RESET_io_bank0_Msk);
	while (!(RESETS->RESET_DONE & RESETS_RESET_DONE_io_bank0_Msk))
		;

	uint32_t reset_peri_val =
		RESETS_RESET_pads_bank0_Msk |
		RESETS_RESET_pads_qspi_Msk |
		RESETS_RESET_io_bank0_Msk |
		RESETS_RESET_io_qspi_Msk |
		RESETS_RESET_timer_Msk |
		RESETS_RESET_uart0_Msk |
		0;
//	SET_BIT(RESETS->RESET, reset_peri_val);
	CLEAR_BIT(RESETS->RESET, reset_peri_val);
//	while ((RESETS->RESET_DONE & reset_peri_val) != reset_peri_val)
//		;

}
