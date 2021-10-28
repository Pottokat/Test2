
#include "hardware.h"
#include "board.h"
#include "clocks.h"

#include <stdio.h>

// See git clone https://gitlab.com/khockuba/cmsis-pi-pico.git~

void NMI_Handler(void)
{

}

void BusFault_Handler(void)
{

}

void UsageFault_Handler(void)
{

}

void HardFault_Handler(void)
{

}

void MemManage_Handler(void)
{

}

void SVC_Handler(void)
{

}

void DebugMon_Handler(void)
{

}

void PendSV_Handler(void)
{

}

extern unsigned long __stack;


/**
 * \brief This is the code that gets called on processor reset.
 * To initialize the device, and call the main() routine.
 */

void Reset_Handler(void)
{
	  SystemInit();                             /* CMSIS System Initialization */
	  __PROGRAM_START();                        /* Enter PreMain (C library entry point) */
}


const
__VECTOR_TABLE_ATTRIBUTE
VECTOR_TABLE_Type __Vectors [48] = {

    /* Configure Initial Stack Pointer, using linker-generated symbols */
    (VECTOR_TABLE_Type)(& __stack),
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    NULL,         /* Reserved */
	NULL,         /* Reserved */
	NULL,         /* Reserved */
	NULL,         /* Reserved */
    SVC_Handler,
    DebugMon_Handler,
	NULL,                  /* Reserved  */
    PendSV_Handler,		/* -2 */
    SysTick_Handler,	/* -1 */
};

//void __attribute__((used, section(".page0")))  bootsram5(void)
//{
//	//SCB->VTOR = (uint32_t) & __Vectors;
//    SCB->VTOR = 0x10000100;
//    __set_MSP(*(uint32_t*)0x10000100);
//    ((void (*)(void)) *(uint32_t*)0x10000104)();
//}

const uint8_t __attribute__((used, section(".page0"))) boot0 [256] =
{
	  0x00, 0xB5, 0x32, 0x4B, 0x21, 0x20, 0x58, 0x60, 0x98, 0x68, 0x02, 0x21, 0x88, 0x43, 0x98, 0x60,
	  0xD8, 0x60, 0x18, 0x61, 0x58, 0x61, 0x2E, 0x4B, 0x00, 0x21, 0x99, 0x60, 0x02, 0x21, 0x59, 0x61,
	  0x01, 0x21, 0xF0, 0x22, 0x99, 0x50, 0x2B, 0x49, 0x19, 0x60, 0x01, 0x21, 0x99, 0x60, 0x35, 0x20,
	  0x00, 0xF0, 0x44, 0xF8, 0x02, 0x22, 0x90, 0x42, 0x14, 0xD0, 0x06, 0x21, 0x19, 0x66, 0x00, 0xF0,
	  0x34, 0xF8, 0x19, 0x6E, 0x01, 0x21, 0x19, 0x66, 0x00, 0x20, 0x18, 0x66, 0x1A, 0x66, 0x00, 0xF0,
	  0x2C, 0xF8, 0x19, 0x6E, 0x19, 0x6E, 0x19, 0x6E, 0x05, 0x20, 0x00, 0xF0, 0x2F, 0xF8, 0x01, 0x21,
	  0x08, 0x42, 0xF9, 0xD1, 0x00, 0x21, 0x99, 0x60, 0x1B, 0x49, 0x19, 0x60, 0x00, 0x21, 0x59, 0x60,
	  0x1A, 0x49, 0x1B, 0x48, 0x01, 0x60, 0x01, 0x21, 0x99, 0x60, 0xEB, 0x21, 0x19, 0x66, 0xA0, 0x21,
	  0x19, 0x66, 0x00, 0xF0, 0x12, 0xF8, 0x00, 0x21, 0x99, 0x60, 0x16, 0x49, 0x14, 0x48, 0x01, 0x60,
	  0x01, 0x21, 0x99, 0x60, 0x01, 0xBC, 0x00, 0x28, 0x00, 0xD0, 0x00, 0x47, 0x12, 0x48, 0x13, 0x49,
	  0x08, 0x60, 0x03, 0xC8, 0x80, 0xF3, 0x08, 0x88, 0x08, 0x47, 0x03, 0xB5, 0x99, 0x6A, 0x04, 0x20,
	  0x01, 0x42, 0xFB, 0xD0, 0x01, 0x20, 0x01, 0x42, 0xF8, 0xD1, 0x03, 0xBD, 0x02, 0xB5, 0x18, 0x66,
	  0x18, 0x66, 0xFF, 0xF7, 0xF2, 0xFF, 0x18, 0x6E, 0x18, 0x6E, 0x02, 0xBD, 0x00, 0x00, 0x02, 0x40,
	  0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x07, 0x00, 0x00, 0x03, 0x5F, 0x00, 0x21, 0x22, 0x00, 0x00,
	  0xF4, 0x00, 0x00, 0x18, 0x22, 0x20, 0x00, 0xA0, 0x00, 0x01, 0x00, 0x10, 0x08, 0xED, 0x00, 0xE0,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x74, 0xB2, 0x4E, 0x7A
};

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
	CLOCKS->CLK_SYS_CTRL = (CLOCKS->CLK_SYS_CTRL & ~ (CLOCKS_CLK_SYS_CTRL_SRC_Msk | CLOCKS_CLK_SYS_CTRL_AUXSRC_Msk)) |
			//(0x03uL << CLOCKS_CLK_SYS_CTRL_AUXSRC_Pos) |	// 0x3 → xosc_clksrc
			(0x00uL << CLOCKS_CLK_SYS_CTRL_AUXSRC_Pos) |	// 0x0 → clksrc_pll_sys
			(0x00uL << CLOCKS_CLK_SYS_CTRL_SRC_Pos) |		// 0x1 → clksrc_clk_sys_aux
			0;
	while (CLOCKS->CLK_SYS_SELECTED == 0)
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
