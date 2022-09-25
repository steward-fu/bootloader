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

#include "fc3000_tft1.h"
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
    
    lcdc_wr_cmd(swapRB(0x2e));
    mdelay(50);
    lcdc_wr_dat(swapRB(0x89));
    lcdc_wr_cmd(swapRB(0x29));
    lcdc_wr_dat(swapRB(0x8f));
    lcdc_wr_cmd(swapRB(0x2b));
    lcdc_wr_dat(swapRB(0x02));
    lcdc_wr_cmd(swapRB(0xe2));
    lcdc_wr_dat(swapRB(0x00));
    lcdc_wr_cmd(swapRB(0xe4));
    lcdc_wr_dat(swapRB(0x01));
    lcdc_wr_cmd(swapRB(0xe5));
    lcdc_wr_dat(swapRB(0x10));
    lcdc_wr_cmd(swapRB(0xe6));
    lcdc_wr_dat(swapRB(0x01));
    lcdc_wr_cmd(swapRB(0xe7));
    lcdc_wr_dat(swapRB(0x10));
    lcdc_wr_cmd(swapRB(0xe8));
    lcdc_wr_dat(swapRB(0x70));
    lcdc_wr_cmd(swapRB(0xf2));
    lcdc_wr_dat(swapRB(0x00));
    lcdc_wr_cmd(swapRB(0xea));
    lcdc_wr_dat(swapRB(0x00));
    lcdc_wr_cmd(swapRB(0xeb));
    lcdc_wr_dat(swapRB(0x20));
    lcdc_wr_cmd(swapRB(0xec));
    lcdc_wr_dat(swapRB(0x3c));
    lcdc_wr_cmd(swapRB(0xed));
    lcdc_wr_dat(swapRB(0xc8));
    lcdc_wr_cmd(swapRB(0xe9));
    lcdc_wr_dat(swapRB(0x38));
    lcdc_wr_cmd(swapRB(0xf1));
    lcdc_wr_dat(swapRB(0x01));
    lcdc_wr_cmd(swapRB(0x40));
    lcdc_wr_dat(swapRB(0x00));
    lcdc_wr_cmd(swapRB(0x41));
    lcdc_wr_dat(swapRB(0x00));
    lcdc_wr_cmd(swapRB(0x42));
    lcdc_wr_dat(swapRB(0x00));
    lcdc_wr_cmd(swapRB(0x43));
    lcdc_wr_dat(swapRB(0x15));
    lcdc_wr_cmd(swapRB(0x44));
    lcdc_wr_dat(swapRB(0x13));
    lcdc_wr_cmd(swapRB(0x45));
    lcdc_wr_dat(swapRB(0x3f));
    lcdc_wr_cmd(swapRB(0x47));
    lcdc_wr_dat(swapRB(0x55));
    lcdc_wr_cmd(swapRB(0x48));
    lcdc_wr_dat(swapRB(0x00));
    lcdc_wr_cmd(swapRB(0x49));
    lcdc_wr_dat(swapRB(0x12));
    lcdc_wr_cmd(swapRB(0x4a));
    lcdc_wr_dat(swapRB(0x19));
    lcdc_wr_cmd(swapRB(0x4b));
    lcdc_wr_dat(swapRB(0x19));
    lcdc_wr_cmd(swapRB(0x4c));
    lcdc_wr_dat(swapRB(0x16));
    lcdc_wr_cmd(swapRB(0x50));
    lcdc_wr_dat(swapRB(0x00));
    lcdc_wr_cmd(swapRB(0x51));
    lcdc_wr_dat(swapRB(0x2c));
    lcdc_wr_cmd(swapRB(0x52));
    lcdc_wr_dat(swapRB(0x2a));
    lcdc_wr_cmd(swapRB(0x53));
    lcdc_wr_dat(swapRB(0x3f));
    lcdc_wr_cmd(swapRB(0x54));
    lcdc_wr_dat(swapRB(0x3f));
    lcdc_wr_cmd(swapRB(0x55));
    lcdc_wr_dat(swapRB(0x3f));
    lcdc_wr_cmd(swapRB(0x56));
    lcdc_wr_dat(swapRB(0x2a));
    lcdc_wr_cmd(swapRB(0x57));
    lcdc_wr_dat(swapRB(0x7e));
    lcdc_wr_cmd(swapRB(0x58));
    lcdc_wr_dat(swapRB(0x09));
    lcdc_wr_cmd(swapRB(0x59));
    lcdc_wr_dat(swapRB(0x06));
    lcdc_wr_cmd(swapRB(0x5a));
    lcdc_wr_dat(swapRB(0x06));
    lcdc_wr_cmd(swapRB(0x5b));
    lcdc_wr_dat(swapRB(0x0d));
    lcdc_wr_cmd(swapRB(0x5c));
    lcdc_wr_dat(swapRB(0x1f));
    lcdc_wr_cmd(swapRB(0x5d));
    lcdc_wr_dat(swapRB(0xff));
    lcdc_wr_cmd(swapRB(0x1b));
    lcdc_wr_dat(swapRB(0x1a));
    lcdc_wr_cmd(swapRB(0x1a));
    lcdc_wr_dat(swapRB(0x02));
    lcdc_wr_cmd(swapRB(0x24));
    lcdc_wr_dat(swapRB(0x61));
    lcdc_wr_cmd(swapRB(0x25));
    lcdc_wr_dat(swapRB(0x5c));
    lcdc_wr_cmd(swapRB(0x23));
    lcdc_wr_dat(swapRB(0x62));
    lcdc_wr_cmd(swapRB(0x18));
    lcdc_wr_dat(swapRB(0x36));
    lcdc_wr_cmd(swapRB(0x19));
    lcdc_wr_dat(swapRB(0x01));
    lcdc_wr_cmd(swapRB(0x1f));
    lcdc_wr_dat(swapRB(0x88));
    lcdc_wr_cmd(swapRB(0x1f));
    lcdc_wr_dat(swapRB(0x80));
    lcdc_wr_cmd(swapRB(0x1f));
    lcdc_wr_dat(swapRB(0x90));
    lcdc_wr_cmd(swapRB(0x1f));
    lcdc_wr_dat(swapRB(0xd4));
    lcdc_wr_cmd(swapRB(0x17));
    lcdc_wr_dat(swapRB(0x05));
    lcdc_wr_cmd(swapRB(0x36));
    lcdc_wr_dat(swapRB(0x08));
    lcdc_wr_cmd(swapRB(0x28));
    lcdc_wr_dat(swapRB(0x38));
    lcdc_wr_cmd(swapRB(0x28));
    lcdc_wr_dat(swapRB(0x3c));

    lcdc_wr_cmd(swapRB(0x02));
    lcdc_wr_dat(swapRB(0x00));
    lcdc_wr_cmd(swapRB(0x03));
    lcdc_wr_dat(swapRB(0x00));
    lcdc_wr_cmd(swapRB(0x04));
    lcdc_wr_dat(swapRB(0x01));
    lcdc_wr_cmd(swapRB(0x05));
    lcdc_wr_dat(swapRB(0x3f));
    lcdc_wr_cmd(swapRB(0x06));
    lcdc_wr_dat(swapRB(0x00));
    lcdc_wr_cmd(swapRB(0x07));
    lcdc_wr_dat(swapRB(0x00));
    lcdc_wr_cmd(swapRB(0x08));
    lcdc_wr_dat(swapRB(0x00));
    lcdc_wr_cmd(swapRB(0x09));
    lcdc_wr_dat(swapRB(0xef));

    lcdc_wr_cmd(swapRB(0x16));
    lcdc_wr_dat(swapRB(0x28));
    lcdc_wr_cmd(swapRB(0x22));
}

int fc3000_tft1_init(void)
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

