/*
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <stdio.h>
#include <stdlib.h>
#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/display.h>
#include <asm/arch/gpio.h>
#include <asm/arch/lcdc.h>
#include <asm/arch/pwm.h>
#include <asm/arch/tve.h>
#include <asm/global_data.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <axp_pmic.h>
#include <errno.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <i2c.h>
#include <malloc.h>
#include <video_fb.h>

#include "fc3000_ips2.h"
#include "booting_logo.h"

static uint32_t swapRB(uint16_t v)
{
    return ((v & 0x001f) << 11) | (v & 0x07e0) | ((v & 0xf800) >> 11);
}

static void lcdc_wr_bus(uint32_t is_data, uint32_t val)
{
    uint32_t ret=0;
	struct sunxi_gpio_reg *const gpio=(struct sunxi_gpio_reg*)SUNXI_PIO_BASE;

    ret = (val & 0x00ff) << 1;
    ret|= (val & 0xff00) << 2;
    ret|= is_data ? 0x80000 : 0;
    ret|= 0x100000;
    writel(ret, &gpio->gpio_bank[SUNXI_GPIO_D].dat);
    ret|= 0x40000;
    writel(ret, &gpio->gpio_bank[SUNXI_GPIO_D].dat);
}

static void lcdc_wr_cmd(uint32_t val)
{
    lcdc_wr_bus(0, val);
}

static void lcdc_wr_dat(uint32_t val)
{
    lcdc_wr_bus(1, val);
}

static void lcd_panel_init(void)
{
    uint32_t ret=0;
	struct sunxi_gpio_reg *const gpio = (struct sunxi_gpio_reg*)SUNXI_PIO_BASE;
  
    // PD = output
    writel(0x11111117, &gpio->gpio_bank[SUNXI_GPIO_D].cfg[0]);
    writel(0x11111171, &gpio->gpio_bank[SUNXI_GPIO_D].cfg[1]);
    writel(0x00111111, &gpio->gpio_bank[SUNXI_GPIO_D].cfg[2]);
    writel(0xffffffff, &gpio->gpio_bank[SUNXI_GPIO_D].dat);

    // PE6 = backlight
    ret = readl(&gpio->gpio_bank[SUNXI_GPIO_E].cfg[0]);
    ret&= 0xf0ffffff;
    ret|= 0xf1ffffff;
    writel(ret, &gpio->gpio_bank[SUNXI_GPIO_E].cfg[0]);

    // turns backlight on
    ret = readl(&gpio->gpio_bank[SUNXI_GPIO_E].dat);
    ret|= 0x0040;
    writel(ret, &gpio->gpio_bank[SUNXI_GPIO_E].dat);

    // PE11 = reset
    ret = readl(&gpio->gpio_bank[SUNXI_GPIO_E].cfg[1]);
    ret&= 0xffff0fff;
    ret|= 0xffff1fff;
    writel(ret, &gpio->gpio_bank[SUNXI_GPIO_E].cfg[1]);

    // reset lcd panel
    ret = readl(&gpio->gpio_bank[SUNXI_GPIO_E].dat);
    ret&= ~0x0800;
    writel(ret, &gpio->gpio_bank[SUNXI_GPIO_E].dat);
    mdelay(250);
    ret|= 0x0800;
    writel(ret, &gpio->gpio_bank[SUNXI_GPIO_E].dat);
    mdelay(150);
    
    lcdc_wr_cmd(swapRB(0x11));
    mdelay(50);

    lcdc_wr_cmd(swapRB(0x36));	//MADCTL - Memory Data Access Control
    lcdc_wr_dat(swapRB(0x6C));	//D7-D0 = MY MX MV ML RGB MH ** ** = 01101100

    lcdc_wr_cmd(swapRB(0x3A));	//COLMOD - Interface Pixel format
    lcdc_wr_dat(swapRB(0x55));	//D7-D0 = 01010101 = 0x55 = 65k color, 16bit

    lcdc_wr_cmd(swapRB(0xB2));	//PORCTRL - Porch Setting
    lcdc_wr_dat(swapRB(0x0C));	//BPA
    lcdc_wr_dat(swapRB(0x0C));	//FPA
    lcdc_wr_dat(swapRB(0x00));	//PSEN
    lcdc_wr_dat(swapRB(0x33));	//FPB
    lcdc_wr_dat(swapRB(0x33));	//BPC

    lcdc_wr_cmd(swapRB(0xB7));	//GCTRL - Gate control
    lcdc_wr_dat(swapRB(0x35));	//D7=0,D6-D4=VGHS,D3=0,D2-D0=VGLS

    lcdc_wr_cmd(swapRB(0xBB));	//VCOMS - Vcom Setting
    lcdc_wr_dat(swapRB(0x19));	//D7-D6=0,D5-D0=VCOM (2B)

    lcdc_wr_cmd(swapRB(0xC0));	//LCMCTRL - LCM Control
    lcdc_wr_dat(swapRB(0x0C));	//XOR command 0x36 paramters

    lcdc_wr_cmd(swapRB(0xC2));	//VDVVRHEN - VDV and VRH command enable
    lcdc_wr_dat(swapRB(0x01));	//enable/disable
    lcdc_wr_dat(swapRB(0xFF));	//

    lcdc_wr_cmd(swapRB(0xC3));	//VRHS - VRH set
    lcdc_wr_dat(swapRB(0x10));	// (11)

    lcdc_wr_cmd(swapRB(0xC4));	//VDVS - VDV set
    lcdc_wr_dat(swapRB(0x20));	//

    lcdc_wr_cmd(swapRB(0xC6));	// FPS
    lcdc_wr_dat(swapRB(0x0F));	// 0F=60, 0B=69, 0A=72, 15=50

    lcdc_wr_cmd(swapRB(0xD0));	//PWCTRL1 - Power control 1
    lcdc_wr_dat(swapRB(0xA4));	//
    lcdc_wr_dat(swapRB(0xA1));	//D7-D6=AVDD,D5-D4=AVCL,D3-D2=0,D1-D0=VDS

    lcdc_wr_cmd(swapRB(0x2A));	//CASET - Column Adress set
    lcdc_wr_dat(swapRB(0x00));
    lcdc_wr_dat(swapRB(0x00));
    lcdc_wr_dat(swapRB(0x01));
    lcdc_wr_dat(swapRB(0x3F));

    lcdc_wr_cmd(swapRB(0x2B));	//RASET - Row Adress set
    lcdc_wr_dat(swapRB(0x00));
    lcdc_wr_dat(swapRB(0x00));
    lcdc_wr_dat(swapRB(0x00));
    lcdc_wr_dat(swapRB(0xEF));

    lcdc_wr_cmd(swapRB(0x51));
    lcdc_wr_dat(swapRB(0xff));

    lcdc_wr_cmd(swapRB(0x29));	//Display on
    lcdc_wr_cmd(swapRB(0x21));	//Invert Display
    lcdc_wr_cmd(swapRB(0x2C));	//Enable Write Ram
}

int fc3000_ips2_init(void)
{
    int x=0, y=0;
    uint16_t *p=booting_logo;

    lcd_panel_init();
    for(y=0; y<240; y++){
        for(x=0; x<320; x++){
            lcdc_wr_dat(swapRB(*p++));
        }
    }
    return 0;
}

