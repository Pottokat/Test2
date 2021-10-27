/*
 * clocks.c
 *
 *  Created on: 26 окт. 2021 г.
 *      Author: User
 */

#include "RP20xx.h"

#include "clocks.h"
#include "board.h"


// Clock source
// Clock from pin gpclk0
unsigned long rp20xx_get_clksrc_gpin0_freq(void)
{
	return GPIN0_VALUE;
}

// Clock source
// Clock from pin gpclk1
unsigned long rp20xx_get_clksrc_gpin1_freq(void)
{
	return GPIN1_VALUE;
}

// Clock source
unsigned long rp20xx_get_xosc_clksrc_freq(void)
{
	return XOSC_VALUE;
}


// Clock source
unsigned long rp20xx_get_rosc_clksrc_freq(void)
{
	return 6000000uL;
}

// Clock source
//	The divider has 2 outputs, rosc_clksrc and rosc_clksrc_ph,
//	the second being a phase shifted version of the first.
//	This is primarily intended for use during product
//	development and the outputs will be
//	identical if the PHASE register is left in its default state.

unsigned long rp20xx_get_rosc_clksrc_ph_freq(void)
{
	return rp20xx_get_rosc_clksrc_freq();
}


static unsigned long rp20xx_divideradj(unsigned long v)
{
	return v == 0 ? (0x01uL << 16) : v;
}
// Clock source
unsigned long rp20xx_get_clksrc_pll_usb_freq(void)
{
    uint32_t ref_div;
	ref_div = (PLL_USB->CS & PLL_SYS_CS_REFDIV_Msk) >> PLL_SYS_CS_REFDIV_Pos;
	if (PLL_USB->CS & PLL_SYS_CS_BYPASS_Msk) {
		return rp20xx_get_xosc_clksrc_freq() / ref_div;
   } else {
	    const uint32_t SCALE = 1;
		uint32_t fb_div;
		uint32_t postdiv1;
		uint32_t postdiv2;
		// Calculation from datasheet
		// (FREF / REFDIV) × FBDIV / (POSTDIV1 × POSTDIV2)
		fb_div          = (PLL_USB->FBDIV_INT & PLL_SYS_FBDIV_INT_FBDIV_INT_Msk) >> PLL_SYS_FBDIV_INT_FBDIV_INT_Pos;
		postdiv1        = (PLL_USB->PRIM & PLL_SYS_PRIM_POSTDIV1_Msk) >> PLL_SYS_PRIM_POSTDIV1_Pos;	// 1..7
		postdiv2        = (PLL_USB->PRIM & PLL_SYS_PRIM_POSTDIV2_Msk) >> PLL_SYS_PRIM_POSTDIV2_Pos;	// 1..7
		return (rp20xx_get_xosc_clksrc_freq() / SCALE / ref_div) * fb_div / (postdiv1 * postdiv2) * SCALE;
	}

}

unsigned long rp20xx_get_clksrc_pll_sys_freq(void)
{
    uint32_t ref_div;
	ref_div = (PLL_SYS->CS & PLL_SYS_CS_REFDIV_Msk) >> PLL_SYS_CS_REFDIV_Pos;
	if (PLL_SYS->CS & PLL_SYS_CS_BYPASS_Msk) {
		return rp20xx_get_xosc_clksrc_freq() / ref_div;
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
		return (rp20xx_get_xosc_clksrc_freq() / SCALE / ref_div) * fb_div / (postdiv1 * postdiv2) * SCALE;
	}
}

unsigned long rp20xx_get_clk_gpout0_freq(void)
{
	const unsigned long divider = rp20xx_divideradj((CLOCKS->CLK_GPOUT0_DIV & CLOCKS_CLK_GPOUT0_DIV_INT_Msk) >> CLOCKS_CLK_GPOUT0_DIV_INT_Pos);
	//	0x0 → clksrc_pll_sys
	//	0x1 → clksrc_gpin0
	//	0x2 → clksrc_gpin1
	//	0x3 → clksrc_pll_usb
	//	0x4 → rosc_clksrc
	//	0x5 → xosc_clksrc
	//	0x6 → clk_sys
	//	0x7 → clk_usb
	//	0x8 → clk_adc
	//	0x9 → clk_rtc
	//	0xa → clk_ref
	switch ((CLOCKS->CLK_GPOUT0_CTRL & CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_Msk) >> CLOCKS_CLK_GPOUT0_CTRL_AUXSRC_Pos)
	{
	default:
	case 0x00:
		return rp20xx_get_clksrc_pll_sys_freq() / divider;
	case 0x01:
		return rp20xx_get_clksrc_gpin0_freq() / divider;
	case 0x02:
		return rp20xx_get_clksrc_gpin1_freq() / divider;
	case 0x03:
		return rp20xx_get_clksrc_pll_usb_freq() / divider;
	case 0x04:
		return rp20xx_get_rosc_clksrc_freq() / divider;
	case 0x05:
		return rp20xx_get_xosc_clksrc_freq() / divider;
	case 0x06:
		return rp20xx_get_clk_sys_freq() / divider;
	case 0x07:
		return rp20xx_get_clk_usb_freq() / divider;
	case 0x08:
		return rp20xx_get_clk_adc_freq() / divider;
	case 0x09:
		return rp20xx_get_clk_rtc_freq() / divider;
	case 0x0A:
		return rp20xx_get_clk_ref_freq() / divider;
	}
}

unsigned long rp20xx_get_clk_gpout1_freq(void)
{
	const unsigned long divider = rp20xx_divideradj((CLOCKS->CLK_GPOUT1_DIV & CLOCKS_CLK_GPOUT1_DIV_INT_Msk) >> CLOCKS_CLK_GPOUT1_DIV_INT_Pos);
	//	0x0 → clksrc_pll_sys
	//	0x1 → clksrc_gpin0
	//	0x2 → clksrc_gpin1
	//	0x3 → clksrc_pll_usb
	//	0x4 → rosc_clksrc
	//	0x5 → xosc_clksrc
	//	0x6 → clk_sys
	//	0x7 → clk_usb
	//	0x8 → clk_adc
	//	0x9 → clk_rtc
	//	0xa → clk_ref
	switch ((CLOCKS->CLK_GPOUT1_CTRL & CLOCKS_CLK_GPOUT1_CTRL_AUXSRC_Msk) >> CLOCKS_CLK_GPOUT1_CTRL_AUXSRC_Pos)
	{
	default:
	case 0x00:
		return rp20xx_get_clksrc_pll_sys_freq() / divider;
	case 0x01:
		return rp20xx_get_clksrc_gpin0_freq() / divider;
	case 0x02:
		return rp20xx_get_clksrc_gpin1_freq() / divider;
	case 0x03:
		return rp20xx_get_clksrc_pll_usb_freq() / divider;
	case 0x04:
		return rp20xx_get_rosc_clksrc_freq() / divider;
	case 0x05:
		return rp20xx_get_xosc_clksrc_freq() / divider;
	case 0x06:
		return rp20xx_get_clk_sys_freq() / divider;
	case 0x07:
		return rp20xx_get_clk_usb_freq() / divider;
	case 0x08:
		return rp20xx_get_clk_adc_freq() / divider;
	case 0x09:
		return rp20xx_get_clk_rtc_freq() / divider;
	case 0x0A:
		return rp20xx_get_clk_ref_freq() / divider;
	}
}

unsigned long rp20xx_get_clk_gpout2_freq(void)
{
	const unsigned long divider = rp20xx_divideradj((CLOCKS->CLK_GPOUT2_DIV & CLOCKS_CLK_GPOUT2_DIV_INT_Msk) >> CLOCKS_CLK_GPOUT2_DIV_INT_Pos);
	//	0x0 → clksrc_pll_sys
	//	0x1 → clksrc_gpin0
	//	0x2 → clksrc_gpin1
	//	0x3 → clksrc_pll_usb
	//	0x4 → rosc_clksrc_ph
	//	0x5 → xosc_clksrc
	//	0x6 → clk_sys
	//	0x7 → clk_usb
	//	0x8 → clk_adc
	//	0x9 → clk_rtc
	//	0xa → clk_ref
	switch ((CLOCKS->CLK_GPOUT2_CTRL & CLOCKS_CLK_GPOUT2_CTRL_AUXSRC_Msk) >> CLOCKS_CLK_GPOUT2_CTRL_AUXSRC_Pos)
	{
	default:
	case 0x00:
		return rp20xx_get_clksrc_pll_sys_freq() / divider;
	case 0x01:
		return rp20xx_get_clksrc_gpin0_freq() / divider;
	case 0x02:
		return rp20xx_get_clksrc_gpin1_freq() / divider;
	case 0x03:
		return rp20xx_get_clksrc_pll_usb_freq() / divider;
	case 0x04:
		return rp20xx_get_rosc_clksrc_ph_freq() / divider;
	case 0x05:
		return rp20xx_get_xosc_clksrc_freq() / divider;
	case 0x06:
		return rp20xx_get_clk_sys_freq() / divider;
	case 0x07:
		return rp20xx_get_clk_usb_freq() / divider;
	case 0x08:
		return rp20xx_get_clk_adc_freq() / divider;
	case 0x09:
		return rp20xx_get_clk_rtc_freq() / divider;
	case 0x0A:
		return rp20xx_get_clk_ref_freq() / divider;
	}

}

unsigned long rp20xx_get_clk_gpout3_freq(void)
{
	const unsigned long divider = rp20xx_divideradj((CLOCKS->CLK_GPOUT3_DIV & CLOCKS_CLK_GPOUT3_DIV_INT_Msk) >> CLOCKS_CLK_GPOUT3_DIV_INT_Pos);
	//	0x0 → clksrc_pll_sys
	//	0x1 → clksrc_gpin0
	//	0x2 → clksrc_gpin1
	//	0x3 → clksrc_pll_usb
	//	0x4 → rosc_clksrc_ph
	//	0x5 → xosc_clksrc
	//	0x6 → clk_sys
	//	0x7 → clk_usb
	//	0x8 → clk_adc
	//	0x9 → clk_rtc
	//	0xa → clk_ref
	switch ((CLOCKS->CLK_GPOUT3_CTRL & CLOCKS_CLK_GPOUT3_CTRL_AUXSRC_Msk) >> CLOCKS_CLK_GPOUT3_CTRL_AUXSRC_Pos)
	{
	default:
	case 0x00:
		return rp20xx_get_clksrc_pll_sys_freq() / divider;
	case 0x01:
		return rp20xx_get_clksrc_gpin0_freq() / divider;
	case 0x02:
		return rp20xx_get_clksrc_gpin1_freq() / divider;
	case 0x03:
		return rp20xx_get_clksrc_pll_usb_freq() / divider;
	case 0x04:
		return rp20xx_get_rosc_clksrc_ph_freq() / divider;
	case 0x05:
		return rp20xx_get_xosc_clksrc_freq() / divider;
	case 0x06:
		return rp20xx_get_clk_sys_freq() / divider;
	case 0x07:
		return rp20xx_get_clk_usb_freq() / divider;
	case 0x08:
		return rp20xx_get_clk_adc_freq() / divider;
	case 0x09:
		return rp20xx_get_clk_rtc_freq() / divider;
	case 0x0A:
		return rp20xx_get_clk_ref_freq() / divider;
	}

}

unsigned long rp20xx_get_clk_adc_freq(void)
{
	const unsigned long divider = rp20xx_divideradj((CLOCKS->CLK_ADC_DIV & CLOCKS_CLK_ADC_DIV_INT_Msk) >> CLOCKS_CLK_ADC_DIV_INT_Pos);
	//	0x0 → clksrc_pll_usb
	//	0x1 → clksrc_pll_sys
	//	0x2 → rosc_clksrc_ph
	//	0x3 → xosc_clksrc
	//	0x4 → clksrc_gpin0
	//	0x5 → clksrc_gpin1
	switch ((CLOCKS->CLK_ADC_CTRL & CLOCKS_CLK_ADC_CTRL_AUXSRC_Msk) >> CLOCKS_CLK_ADC_CTRL_AUXSRC_Pos)
	{
	default:
	case 0x00:
		return rp20xx_get_clksrc_pll_usb_freq() / divider;
	case 0x01:
		return rp20xx_get_clksrc_pll_sys_freq() / divider;
	case 0x02:
		return rp20xx_get_rosc_clksrc_ph_freq() / divider;
	case 0x03:
		return rp20xx_get_xosc_clksrc_freq() / divider;
	case 0x04:
		return rp20xx_get_clksrc_gpin0_freq() / divider;
	case 0x05:
		return rp20xx_get_clksrc_gpin1_freq() / divider;
	}

}

unsigned long rp20xx_get_clk_rtc_freq(void)
{
	const unsigned long divider = rp20xx_divideradj((CLOCKS->CLK_RTC_DIV & CLOCKS_CLK_RTC_DIV_INT_Msk) >> CLOCKS_CLK_RTC_DIV_INT_Pos);
	//	0x0 → clksrc_pll_usb
	//	0x1 → clksrc_pll_sys
	//	0x2 → rosc_clksrc_ph
	//	0x3 → xosc_clksrc
	//	0x4 → clksrc_gpin0
	//	0x5 → clksrc_gpin1
	switch ((CLOCKS->CLK_RTC_CTRL & CLOCKS_CLK_RTC_CTRL_AUXSRC_Msk) >> CLOCKS_CLK_RTC_CTRL_AUXSRC_Pos)
	{
	default:
	case 0x00:
		return rp20xx_get_clksrc_pll_sys_freq() / divider;
	case 0x01:
		return rp20xx_get_clksrc_pll_usb_freq() / divider;
	case 0x02:
		return rp20xx_get_rosc_clksrc_ph_freq() / divider;
	case 0x03:
		return rp20xx_get_xosc_clksrc_freq() / divider;
	case 0x04:
		return rp20xx_get_clksrc_gpin0_freq() / divider;
	case 0x05:
		return rp20xx_get_clksrc_gpin1_freq() / divider;
	}
}

unsigned long rp20xx_get_clk_sys_freq(void)
{
	const unsigned long divider = rp20xx_divideradj((CLOCKS->CLK_SYS_DIV & CLOCKS_CLK_SYS_DIV_INT_Msk) >> CLOCKS_CLK_SYS_DIV_INT_Pos);
	//	0x0 → clksrc_pll_sys
	//	0x1 → clksrc_pll_usb
	//	0x2 → rosc_clksrc
	//	0x3 → xosc_clksrc
	//	0x4 → clksrc_gpin0
	//	0x5 → clksrc_gpin1
	switch ((CLOCKS->CLK_SYS_CTRL & CLOCKS_CLK_SYS_CTRL_AUXSRC_Msk) >> CLOCKS_CLK_SYS_CTRL_AUXSRC_Pos)
	{
	default:
	case 0x00:
		return rp20xx_get_clksrc_pll_sys_freq() / divider;
	case 0x01:
		return rp20xx_get_clksrc_pll_usb_freq() / divider;
	case 0x02:
		return rp20xx_get_rosc_clksrc_freq() / divider;
	case 0x03:
		return rp20xx_get_xosc_clksrc_freq() / divider;
	case 0x04:
		return rp20xx_get_clksrc_gpin0_freq() / divider;
	case 0x05:
		return rp20xx_get_clksrc_gpin1_freq() / divider;
	}
}

unsigned long rp20xx_get_clk_usb_freq(void)
{
	const unsigned long divider = rp20xx_divideradj((CLOCKS->CLK_USB_DIV & CLOCKS_CLK_USB_DIV_INT_Msk) >> CLOCKS_CLK_USB_DIV_INT_Pos);
	//	0x0 → clksrc_pll_usb
	//	0x1 → clksrc_pll_sys
	//	0x2 → rosc_clksrc_ph
	//	0x3 → xosc_clksrc
	//	0x4 → clksrc_gpin0
	//	0x5 → clksrc_gpin1
	switch ((CLOCKS->CLK_USB_CTRL & CLOCKS_CLK_USB_CTRL_AUXSRC_Msk) >> CLOCKS_CLK_USB_CTRL_AUXSRC_Pos)
	{
	default:
	case 0x00:
		return rp20xx_get_clksrc_pll_usb_freq() / divider;
	case 0x01:
		return rp20xx_get_clksrc_pll_sys_freq() / divider;
	case 0x02:
		return rp20xx_get_rosc_clksrc_ph_freq() / divider;
	case 0x03:
		return rp20xx_get_xosc_clksrc_freq() / divider;
	case 0x04:
		return rp20xx_get_clksrc_gpin0_freq() / divider;
	case 0x05:
		return rp20xx_get_clksrc_gpin1_freq() / divider;
	}
}
unsigned long rp20xx_get_clk_peri_freq(void)
{
	//	0x0 → clk_sys
	//	0x1 → clksrc_pll_sys
	//	0x2 → clksrc_pll_usb
	//	0x3 → rosc_clksrc_ph
	//	0x4 → xosc_clksrc
	//	0x5 → clksrc_gpin0
	//	0x6 → clksrc_gpin1
	switch ((CLOCKS->CLK_PERI_CTRL & CLOCKS_CLK_PERI_CTRL_AUXSRC_Msk) >> CLOCKS_CLK_PERI_CTRL_AUXSRC_Pos)
	{
	default:
	case 0x00:
		return rp20xx_get_clk_sys_freq();
	case 0x01:
		return rp20xx_get_clksrc_pll_sys_freq();
	case 0x02:
		return rp20xx_get_clksrc_pll_usb_freq();
	case 0x03:
		return rp20xx_get_rosc_clksrc_ph_freq();
	case 0x04:
		return rp20xx_get_xosc_clksrc_freq();
	case 0x05:
		return rp20xx_get_clksrc_gpin0_freq();
	case 0x06:
		return rp20xx_get_clksrc_gpin1_freq();
	}
}

unsigned long rp20xx_get_clksrc_clk_ref_aux_freq(void)
{
	//	0x0 → clksrc_pll_usb
	//	0x1 → clksrc_gpin0
	//	0x2 → clksrc_gpin1
	switch ((CLOCKS->CLK_REF_CTRL & CLOCKS_CLK_REF_CTRL_AUXSRC_Msk) >> CLOCKS_CLK_REF_CTRL_AUXSRC_Pos)
	{
	default:
	case 0x00:
		return rp20xx_get_clksrc_pll_usb_freq();
	case 0x01:
		return rp20xx_get_clksrc_gpin0_freq();
	case 0x02:
		return rp20xx_get_clksrc_gpin1_freq();
	}
}

unsigned long rp20xx_get_clk_ref_freq(void)
{
//	0x0 → rosc_clksrc_ph
//	0x1 → clksrc_clk_ref_aux
//	0x2 → xosc_clksrc
	switch ((CLOCKS->CLK_REF_CTRL & CLOCKS_CLK_REF_CTRL_SRC_Msk) >> CLOCKS_CLK_REF_CTRL_SRC_Pos)
	{
	default:
	case 0x00:
		return rp20xx_get_rosc_clksrc_ph_freq();
	case 0x01:
		return rp20xx_get_clksrc_clk_ref_aux_freq();
	case 0x02:
		return rp20xx_get_xosc_clksrc_freq();
	}
}


