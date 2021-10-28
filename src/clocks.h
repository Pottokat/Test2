/*
 * clocks.h
 *
 *  Created on: 26 окт. 2021 г.
 *      Author: User
 */

#ifndef SRC_CLOCKS_H_
#define SRC_CLOCKS_H_



unsigned long rp20xx_get_clksrc_gpin0_freq(void);
unsigned long rp20xx_get_clksrc_gpin1_freq(void);
unsigned long rp20xx_get_xosc_clksrc_freq(void);
unsigned long rp20xx_get_rosc_clksrc_freq(void);
unsigned long rp20xx_get_rosc_clksrc_ph_freq(void);
unsigned long rp20xx_get_clksrc_pll_usb_freq(void);
unsigned long rp20xx_get_clksrc_pll_sys_freq(void);
unsigned long rp20xx_get_clk_gpout0_freq(void);
unsigned long rp20xx_get_clk_gpout1_freq(void);
unsigned long rp20xx_get_clk_gpout2_freq(void);
unsigned long rp20xx_get_clk_gpout3_freq(void);
unsigned long rp20xx_get_clk_adc_freq(void);
unsigned long rp20xx_get_clk_usb_freq(void);
unsigned long rp20xx_get_clk_sys_freq(void);
unsigned long rp20xx_get_clk_peri_freq(void);
unsigned long rp20xx_get_clk_ref_freq(void);
unsigned long rp20xx_get_clksrc_clk_ref_aux_freq(void);
unsigned long rp20xx_get_clk_rtc_freq(void);

#define CPU_FREQ (rp20xx_get_clk_sys_freq())

#endif /* SRC_CLOCKS_H_ */
