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

#include "fc3000_tft2.h"
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
    
    lcdc_wr_cmd(0x800);
    mdelay(50);
    lcdc_wr_dat(0x100);
    lcdc_wr_cmd(0x1000);
    lcdc_wr_dat(0x700);
    lcdc_wr_cmd(0x1800);
    lcdc_wr_dat(0xc002);
    lcdc_wr_cmd(0x2000);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x4000);
    lcdc_wr_dat(0x1200);
    lcdc_wr_cmd(0x4800);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x5000);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x6000);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x6800);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x7800);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x8000);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x8800);
    lcdc_wr_dat(0x3800);
    lcdc_wr_cmd(0x9000);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x9800);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x3800);
    lcdc_wr_dat(0x800);
    lcdc_wr_cmd(0x8000);
    lcdc_wr_dat(0x8682);
    lcdc_wr_cmd(0x8800);
    lcdc_wr_dat(0x3e60);
    lcdc_wr_cmd(0x9000);
    lcdc_wr_dat(0xc080);
    lcdc_wr_cmd(0x9800);
    lcdc_wr_dat(0x603);
    lcdc_wr_cmd(0x4820);
    lcdc_wr_dat(0xf000);
    lcdc_wr_cmd(0x5820);
    lcdc_wr_dat(0x7000);
    lcdc_wr_cmd(0x20);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x820);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x8020);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x8820);
    lcdc_wr_dat(0x3d00);
    lcdc_wr_cmd(0x9020);
    lcdc_wr_dat(0x2000);
    lcdc_wr_cmd(0xa820);
    lcdc_wr_dat(0x2a00);
    lcdc_wr_cmd(0xb020);
    lcdc_wr_dat(0x2000);
    lcdc_wr_cmd(0xb820);
    lcdc_wr_dat(0x3b00);
    lcdc_wr_cmd(0xc020);
    lcdc_wr_dat(0x1000);
    lcdc_wr_cmd(0xc820);
    lcdc_wr_dat(0x3f00);
    lcdc_wr_cmd(0xe020);
    lcdc_wr_dat(0x1500);
    lcdc_wr_cmd(0xe820);
    lcdc_wr_dat(0x2000);
    lcdc_wr_cmd(0x8040);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x8840);
    lcdc_wr_dat(0x78e0);
    lcdc_wr_cmd(0x9040);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x9840);
    lcdc_wr_dat(0xf920);
    lcdc_wr_cmd(0x60);
    lcdc_wr_dat(0x714);
    lcdc_wr_cmd(0x860);
    lcdc_wr_dat(0x800);
    lcdc_wr_cmd(0x5060);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x80);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x880);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x1080);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x1880);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x2080);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x2880);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x8080);
    lcdc_wr_dat(0x8000);
    lcdc_wr_cmd(0x9080);
    lcdc_wr_dat(0x600);
    lcdc_wr_cmd(0x1800);
    lcdc_wr_dat(0x4020);
    lcdc_wr_cmd(0x3800);
    lcdc_wr_dat(0x9920);
    lcdc_wr_cmd(0x8040);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x8840);
    lcdc_wr_dat(0x78e0);
    lcdc_wr_cmd(0x9040);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x9840);
    lcdc_wr_dat(0xf920);
    lcdc_wr_cmd(0x20);
    lcdc_wr_dat(0x78e0);
    lcdc_wr_cmd(0x820);
    lcdc_wr_dat(0x0);
    lcdc_wr_cmd(0x1020);
}

int fc3000_tft2_init(void)
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

