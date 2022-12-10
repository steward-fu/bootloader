#ifndef __BOARD_H__
#define __BOARD_H__

#include "dram.h"
#include "sunxi_spi.h"
#include "sunxi_usart.h"
#include "sunxi_sdhci.h"

#define CONFIG_BOOT_SPINAND
#define CONFIG_BOOT_SDCARD

#define CONFIG_CPU_FREQ 1200000000

#define CONFIG_ENABLE_CPU_FREQ_DUMP

#define CONFIG_KERNEL_FILENAME "zImage"
#define CONFIG_DTB_FILENAME	   "sun8i-t113-mangopi-dual.dtb"

#define CONFIG_KERNEL_LOAD_ADDR (SDRAM_BASE + (72 * 1024 * 1024))
#define CONFIG_DTB_LOAD_ADDR	(SDRAM_BASE + (64 * 1024 * 1024))

#define CONFIG_SPINAND_DTB_ADDR	   (128 * 2048)
#define CONFIG_SPINAND_KERNEL_ADDR (256 * 2048)

extern dram_para_t	 ddr_param;
extern sunxi_usart_t usart_dbg;
extern sunxi_spi_t	 sunxi_spi0;

extern void board_init(void);
extern int	board_sdhci_init(void);

#endif
