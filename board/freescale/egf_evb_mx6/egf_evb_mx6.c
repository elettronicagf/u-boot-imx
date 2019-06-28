/*
 * Copyright (C) 2012-2015 Freescale Semiconductor, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/video.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>

#if defined(CONFIG_MX6DL) && defined(CONFIG_MXC_EPDC)
#include <lcd.h>
#include <mxc_epdc_fb.h>
#endif
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/crm_regs.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <i2c.h>
#include <ipu_pixfmt.h>
#include <linux/fb.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>
#include "../common/pfuze.h"
#include <usb.h>
#if defined(CONFIG_MX6DL) && defined(CONFIG_MXC_EPDC)
#include <lcd.h>
#include <mxc_epdc_fb.h>
#endif
#ifdef CONFIG_CMD_SATA
#include <asm/imx-common/sata.h>
#endif
#ifdef CONFIG_FSL_FASTBOOT
#include <fsl_fastboot.h>
#ifdef CONFIG_ANDROID_RECOVERY
#include <recovery.h>
#endif
#endif /*CONFIG_FSL_FASTBOOT*/

#include "gf_mux.h"
#include "gf_eeprom.h"
#include "gf_eeprom_port.h"
#ifdef CONFIG_SPL_BUILD
#include "gf_ddr_parameters.h"
#endif

#include "../drivers/video/mxcfb.h"

DECLARE_GLOBAL_DATA_PTR;

#define WID_LENGTH					15
#define EGF_FDT_FILE_NAME_LENGTH	(13 + WID_LENGTH + 1)

#define PWR_5V0_EN_3V3_GPIO		IMX_GPIO_NR(1,3)
#define VIO_3V3_EN				IMX_GPIO_NR(6,14)

#define I2C_PMIC	1

#define DISP0_EN				IMX_GPIO_NR(6, 16)
#define DISP0_BKL_PWM_GPIO		IMX_GPIO_NR(2, 10)
#define DISP0_BKL_PWR_EN_GPIO	IMX_GPIO_NR(3, 8)
#define DISP1_EN				IMX_GPIO_NR(2, 27)
#define DISP1_BKL_PWM_GPIO		IMX_GPIO_NR(1, 9)
#define DISP1_BKL_PWR_EN_GPIO	IMX_GPIO_NR(3, 14)
#define DISP1_LVDS_GPIO			IMX_GPIO_NR(5, 18)
#define DEBUG_UART_EN			IMX_GPIO_NR(2, 21)

#define PICOS2KHZ(a) (1000000000UL/(a))

//returns 0 if no console has to be enabled
u32 gf_get_debug_uart_base(void)
{
	u32 pcb_rev;
	int ret;

	pcb_rev = gf_get_pcb_rev();

	if (pcb_rev == PCB_REV_PGF0533_A01) {
		/* Board is based on PGF0533_A01. Debug UART is UART2 */
		return UART2_BASE;
	} else if (pcb_rev == PCB_REV_PGF0533_A02) {
		/* Board is based on PGF0533_A02. Debug UART is UART1 */
		return UART1_BASE;
	} else if (pcb_rev == PCB_REV_PGF0533_A03) {
		/* Board is based on PGF0533_A03. Debug UART is UART1 if J3 is not closed */
		gpio_direction_input(DEBUG_UART_EN);
		ret = gpio_get_value(DEBUG_UART_EN);
		//in programmazione sempre abilitata la seriale
		if (is_boot_from_usb())
			return UART1_BASE;

		if(ret == 0) //J3 closed
			return 0;
		else
			return UART1_BASE;
	} else if (pcb_rev == PCB_REV_UNKNOWN) {
			/* Unknown Board Revision. Default to UART1 */
		return UART1_BASE;
	} else {
		/* Board EEPROM is not programmed. Try to guess board revision */
		gpio_direction_input(IMX_GPIO_NR(1,4));
		ret = gpio_get_value(IMX_GPIO_NR(1,4));
		if (ret == 0){
			/* On PGF0533_A02 there is a 10k pulldown on GPIO1-IO04 */
			return UART1_BASE;
		} else {
			/* On PGF0533_A01 there is a 10k pullup on GPIO1-IO04 */
			return UART2_BASE;
		}
	}
}

static int gf_strcmp(const char * cs, const char * ct) {
	register signed char __res;

	while (1) {
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
	}

	return __res;
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	return 0;
}

void prepare_boot_env(void)
{
	char * egf_sw_id_code;
	char * board_sw_id_code;
	char* mac_address;
	char fdt_file_name[EGF_FDT_FILE_NAME_LENGTH];
	u32 uartBase;
	u32 pcb_rev;

	egf_sw_id_code = gf_eeprom_get_som_sw_id_code();
	fdt_file_name[0] = 0;
	gf_strcat(fdt_file_name, "imx6-egf-");
	gf_strcat(fdt_file_name, egf_sw_id_code);
	gf_strcat(fdt_file_name, ".dtb");

	setenv("fdt_file",fdt_file_name);

	mac_address = gf_eeprom_get_mac1_address();
	if (mac_address == NULL)
			printf("MAC Address not programmed.\n");
	else
	{
		setenv("ethaddr",mac_address);
	}

	uartBase = gf_get_debug_uart_base();
	switch(uartBase){
	case UART1_BASE:
		setenv("console","ttymxc0");
		break;
	case UART2_BASE:
		setenv("console","ttymxc1");
		break;
	case UART3_BASE:
		setenv("console","ttymxc2");
		break;
	case UART4_BASE:
		setenv("console","ttymxc3");
		break;
	case UART5_BASE:
		setenv("console","ttymxc4");
		break;
	default:
		setenv("console","null");
		setenv("silent","1");
		break;
	}

	pcb_rev = gf_get_pcb_rev();
	switch(pcb_rev){
	case PCB_REV_PGF0533_A01:
		setenv("pcb_rev", "PGF0533_A01");
		break;
	case PCB_REV_PGF0533_A02:
		setenv("pcb_rev", "PGF0533_A02");
		break;
	case PCB_REV_PGF0533_A03:
		setenv("pcb_rev", "PGF0533_A03");
		break;
	default:
		setenv("pcb_rev", "");
		break;
	}

	board_sw_id_code = gf_eeprom_get_board_sw_id_code();
	if (board_sw_id_code)
	{
		if((!gf_strcmp(board_sw_id_code, REV_WID0533_AB0101)) ||
		  (!gf_strcmp(board_sw_id_code, REV_WID0533_BC0101)))
			setenv("audio", "0");
	}

}

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg usdhc_cfg[2] = {
	{USDHC1_BASE_ADDR, 0, 4},
	{USDHC3_BASE_ADDR},
};

int mmc_get_env_devno(void)
{
	u32 soc_sbmr = readl(SRC_BASE_ADDR + 0x4);
	u32 dev_no;
	u32 bootsel;

	bootsel = (soc_sbmr & 0x000000FF) >> 6 ;

	/* If not boot from sd/mmc, use default value */
	if (bootsel != 1)
		return CONFIG_SYS_MMC_ENV_DEV;

	/* BOOT_CFG2[3] and BOOT_CFG2[4] */
	dev_no = (soc_sbmr & 0x00001800) >> 11;

	/* need ubstract 1 to map to the mmc device id
	 * see the comments in board_mmc_init function
	 */

	dev_no--;

	return dev_no;
}

int mmc_map_to_kernel_blk(int dev_no)
{
	return dev_no + 1;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
		ret = !gpio_get_value(SD1_CD_GPIO);
		break;
	case USDHC3_BASE_ADDR:
		ret = 1; /* eMMC is always present */
		break;
	}

	return ret;
}

int board_mmc_init(bd_t *bis)
{
#ifndef CONFIG_SPL_BUILD
	int ret;
	int i;

	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-boot device node)    (Physical Port)
	 * mmc0                    SD1 - microSD
	 * mmc1                    SD3 - eMMC
	 */
	for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			gpio_direction_input(SD1_CD_GPIO);
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
			break;
		case 1:
			usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
			       "(%d) then supported by the board (%d)\n",
			       i + 1, CONFIG_SYS_FSL_USDHC_NUM);
			return -EINVAL;
		}

		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		if (ret)
			return ret;
	}

	return 0;
#else
	struct src *psrc = (struct src *)SRC_BASE_ADDR;
	unsigned reg = readl(&psrc->sbmr1) >> 11;
	/*
	 * Upon reading BOOT_CFG register the following map is done:
	 * Bit 11 and 12 of BOOT_CFG register can determine the current
	 * mmc port
	 * 0x1                  SD1 - microSD
	 * 0x2                  SD3 - eMMC
	 */

	switch (reg & 0x3) {
	case 0x1:
		usdhc_cfg[0].esdhc_base = USDHC1_BASE_ADDR;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
		gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
		break;
	case 0x2:
		usdhc_cfg[0].esdhc_base = USDHC3_BASE_ADDR;
		usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
		gd->arch.sdhc_clk = usdhc_cfg[0].sdhc_clk;
		break;
	}

	return fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
#endif
}
#endif

int check_mmc_autodetect(void)
{
	char *autodetect_str = getenv("mmcautodetect");

	if ((autodetect_str != NULL) &&
		(strcmp(autodetect_str, "yes") == 0)) {
		return 1;
	}

	return 0;
}

void board_late_mmc_env_init(void)
{
	char cmd[32];
	char mmcblk[32];
	u32 dev_no = mmc_get_env_devno();

	if (!check_mmc_autodetect())
		return;

	setenv_ulong("mmcdev", dev_no);

	/* Set mmcblk env */
	sprintf(mmcblk, "/dev/mmcblk%dp2 rootwait rw",
		mmc_map_to_kernel_blk(dev_no));
	setenv("mmcroot", mmcblk);

	sprintf(cmd, "mmc dev %d", dev_no);
	run_command(cmd, 0);
}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

#if defined(CONFIG_VIDEO_IPUV3)
static void enable_lvds(struct display_info_t const *dev)
{
	struct iomuxc *iomux = (struct iomuxc *)
				IOMUXC_BASE_ADDR;
	u32 reg = readl(&iomux->gpr[2]);
	if (dev->pixfmt == IPU_PIX_FMT_RGB24)
		reg |= IOMUXC_GPR2_DATA_WIDTH_CH1_24BIT;
	else
		reg |= IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT;
	writel(reg, &iomux->gpr[2]);
}

static void vpll_change_frequency(unsigned int pixclock){
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	unsigned int timeout = 100000;
	unsigned int target_multiplier;
	unsigned int div_select;
	unsigned int num, denom;
	if(pixclock > 92500000){
		printf("Display Frequency not supported!\n");
		return;
	}
	/* Bypass PLL5 */
	setbits_le32(&ccm->analog_pll_video, BM_ANADIG_PLL_VIDEO_BYPASS);
	/* Power Down PLL5 */
	setbits_le32(&ccm->analog_pll_video, BM_ANADIG_PLL_VIDEO_POWERDOWN);

	// Set PLL5 POST_DIV_SELECT to 1 (divide by 2) and PLL5 POST_DIV_SELECT to 37
	// PLL Freq = Fref * (DIV_SELECT + NUM / DENOM) = 24M * (37 + 11/12) = 910
	// PLL5 post div select = 910 / 2 = 455 MHz
	// 455 MHz / 7 = 65 MHz (pixel clock)

	printf("Changing frequency to %d\n", pixclock);
	target_multiplier = (pixclock * 7 * 2) / 24;
	printf("Target multiplier %d\n", (int)target_multiplier);
	div_select = target_multiplier / 1000000;
	printf("Div_select is %d\n", div_select);
	num = target_multiplier - (div_select * 1000000);
	printf("num %d\n", num);
	denom = 1000000;
	printf("Denom is %d\n", denom);

	clrsetbits_le32(&ccm->analog_pll_video,
			BM_ANADIG_PLL_VIDEO_DIV_SELECT |
			(0x3 << 19),
			BF_ANADIG_PLL_VIDEO_DIV_SELECT(div_select) |
			(0x1 << 19));

	writel(BF_ANADIG_PLL_VIDEO_NUM_A(num), &ccm->analog_pll_video_num);
	writel(BF_ANADIG_PLL_VIDEO_DENOM_B(denom), &ccm->analog_pll_video_denom);

	clrbits_le32(&ccm->analog_pll_video, BM_ANADIG_PLL_VIDEO_POWERDOWN);

	while (timeout--)
		if (readl(&ccm->analog_pll_video) & BM_ANADIG_PLL_VIDEO_LOCK)
			break;
	if (timeout < 0)
		printf("Warning: video pll lock timeout!\n");

	/* Exit bypass Mode */
	clrsetbits_le32(&ccm->analog_pll_video,
			BM_ANADIG_PLL_VIDEO_BYPASS,
			BM_ANADIG_PLL_VIDEO_ENABLE);

}

static unsigned int get_disp_pix_clock(struct display_info_t const *dev) {
	return PICOS2KHZ(dev->mode.pixclock) * 1000;
}

static void lvds_disp_enable_pgf0533_a01(struct display_info_t const *dev)
{
	vpll_change_frequency(get_disp_pix_clock(dev));
	enable_lvds(dev);
	gpio_direction_output(DISP1_EN, 1);
	gpio_direction_output(DISP1_BKL_PWM_GPIO, 1);
	gpio_direction_output(DISP1_BKL_PWR_EN_GPIO, 1);
}

static void lvds_disp_enable_dual_bl_pgf0533_a01(struct display_info_t const *dev)
{
	vpll_change_frequency(get_disp_pix_clock(dev));
	enable_lvds(dev);
	gpio_direction_output(DISP1_EN, 1);
	gpio_direction_output(DISP1_BKL_PWM_GPIO, 1);
	gpio_direction_output(DISP1_BKL_PWR_EN_GPIO, 1);
	gpio_direction_output(DISP0_BKL_PWM_GPIO, 1);
	gpio_direction_output(DISP0_BKL_PWR_EN_GPIO, 1);
}

static void rgb_disp_enable_pgf0533_a01(struct display_info_t const *dev)
{
	gpio_direction_output(DISP0_EN, 1);
	gpio_direction_output(DISP0_BKL_PWM_GPIO, 1);
	gpio_direction_output(DISP0_BKL_PWR_EN_GPIO, 1);
}

static void lvds_disp_enable_pgf0533_a02(struct display_info_t const *dev)
{
	vpll_change_frequency(get_disp_pix_clock(dev));
	enable_lvds(dev);
	gpio_direction_output(LCD_PWR_EN_GPIO, 1);
	gpio_direction_output(DISP1_EN, 1);
	gpio_direction_output(DISP1_LVDS_GPIO, 1);
	gpio_direction_output(DISP1_BKL_PWM_GPIO, 1);
	gpio_direction_output(DISP1_BKL_PWR_EN_GPIO, 1);
}

static void rgb_disp_enable_pgf0533_a02(struct display_info_t const *dev)
{
	gpio_direction_output(LCD_PWR_EN_GPIO, 1);
	gpio_direction_output(DISP1_EN, 1);
	gpio_direction_output(DISP1_LVDS_GPIO, 1);
	gpio_direction_output(DISP1_BKL_PWM_GPIO, 1);
	gpio_direction_output(DISP1_BKL_PWR_EN_GPIO, 1);
}

static void lvds_disp_enable(struct display_info_t const *dev)
{
	int pcb_rev;
	pcb_rev = gf_get_pcb_rev();

	if (pcb_rev == PCB_REV_PGF0533_A02 ||
		pcb_rev == PCB_REV_PGF0533_A03    )
		lvds_disp_enable_pgf0533_a02(dev);
	else
		lvds_disp_enable_pgf0533_a01(dev);
}


struct display_info_t const displays[] = {
	/* PGF0533_A01 Only */
	{
		.bus	= 0,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= NULL,
		.enable	= lvds_disp_enable_dual_bl_pgf0533_a01,
		.mode	= {
			.name           = "EGF_BLC1133", /* DLC1010AZG-T-6 DLC 10.1" */
			.refresh        = 60,
			.xres           = 1024,
			.yres           = 600,
			.pixclock       = 19531,
			.left_margin    = 160,
			.right_margin   = 160,
			.upper_margin   = 23,
			.lower_margin   = 12,
			.hsync_len      = 60,
			.vsync_len      = 10,
			.sync           = FB_SYNC_EXT,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	},
	{
		.bus	= 0,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= NULL,
		.enable	= rgb_disp_enable_pgf0533_a01,
		.mode	= {
			.name           = "EGF_BLC1093", /*  KWH070KQ13-F02 Formike 7.0" */
			.refresh        = 60,
			.xres           = 800,
			.yres           = 480,
			.pixclock       = 25000,
			.left_margin    = 45,
			.right_margin   = 210,
			.upper_margin   = 22,
			.lower_margin   = 132,
			.hsync_len      = 1,
			.vsync_len      = 1,
			.sync           = 0,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	},
	{
		.bus	= 0,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= NULL,
		.enable	= rgb_disp_enable_pgf0533_a01,
		.mode	= {
			.name           = "EGF_BLC1113", /*  KWH070KQ13-F01 Formike 7.0 without touchscreen" */
			.refresh        = 60,
			.xres           = 800,
			.yres           = 480,
			.pixclock       = 25000,
			.left_margin    = 45,
			.right_margin   = 210,
			.upper_margin   = 22,
			.lower_margin   = 132,
			.hsync_len      = 1,
			.vsync_len      = 1,
			.sync           = 0,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	},
	{
		.bus	= 0,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= NULL,
		.enable	= rgb_disp_enable_pgf0533_a01,
		.mode	= {
			.name           = "EGF_BLC1081", /*  WL_AT070TN84-ETT-A1 T1242A 7.0" */
			.refresh        = 60,
			.xres           = 800,
			.yres           = 480,
			.pixclock       = 25000,
			.left_margin    = 45,
			.right_margin   = 210,
			.upper_margin   = 22,
			.lower_margin   = 132,
			.hsync_len      = 0,
			.vsync_len      = 0,
			.sync           = FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	},
	/* PGF0533_A02 Only */
	{
		.bus	= 0,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= NULL,
		.enable	= lvds_disp_enable_pgf0533_a02,
		.mode	= {
			.name           = "EGF_BLC1168", /* DLC1010LZG-T-1 10.1" Cap Touch */
			.refresh        = 60,
			.xres           = 1024,
			.yres           = 600,
			.pixclock       = 19531,
			.left_margin    = 160,
			.right_margin   = 160,
			.upper_margin   = 23,
			.lower_margin   = 12,
			.hsync_len      = 10,
			.vsync_len      = 10,
			.sync           = FB_SYNC_EXT,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	},
	{
		.bus	= 0,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= NULL,
		.enable	= lvds_disp_enable_pgf0533_a02,
		.mode	= {
			.name           = "EGF_BLC1167", /* DLC1010LZG-T 10.1" Res Touch */
			.refresh        = 60,
			.xres           = 1024,
			.yres           = 600,
			.pixclock       = 19531,
			.left_margin    = 160,
			.right_margin   = 160,
			.upper_margin   = 23,
			.lower_margin   = 12,
			.hsync_len      = 10,
			.vsync_len      = 10,
			.sync           = FB_SYNC_EXT,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	},
	{
		.bus	= 0,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= NULL,
		.enable	= lvds_disp_enable_pgf0533_a02,
		.mode	= {
			.name           = "EGF_BLC1173", /*  DLC1010ADM42LT-C-2 10.1" Cap Touch */
			.refresh        = 60,
			.xres           = 1024,
			.yres           = 600,
			.pixclock       = 19531,
			.left_margin    = 160,
			.right_margin   = 160,
			.upper_margin   = 23,
			.lower_margin   = 12,
			.hsync_len      = 10,
			.vsync_len      = 10,
			.sync           = FB_SYNC_EXT,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	},
	{
		.bus	= 0,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= NULL,
		.enable	= rgb_disp_enable_pgf0533_a02,
		.mode	= {
			.name           = "EGF_BLC1149", /*  DLC0700OZR-T 7" 4-wire Res Touch */
			.refresh        = 60,
			.xres           = 800,
			.yres           = 480,
			.pixclock       = 35000,
			.left_margin    = 46,
			.right_margin   = 20,
			.upper_margin   = 23,
			.lower_margin   = 10,
			.hsync_len      = 1,
			.vsync_len      = 1,
			.sync           = FB_SYNC_CLK_LAT_FALL | FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	},
	{
		.bus	= 0,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= NULL,
		.enable	= rgb_disp_enable_pgf0533_a02,
		.mode	= {
			.name           = "EGF_BLC1152", /*  DLC0700O2ZR-T-7 7" Cap Touch */
			.refresh        = 60,
			.xres           = 800,
			.yres           = 480,
			.pixclock       = 35000,
			.left_margin    = 46,
			.right_margin   = 20,
			.upper_margin   = 23,
			.lower_margin   = 10,
			.hsync_len      = 1,
			.vsync_len      = 1,
			.sync           = FB_SYNC_CLK_LAT_FALL | FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	},
	{
		.bus	= 0,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= NULL,
		.enable	= rgb_disp_enable_pgf0533_a02,
		.mode	= {
			.name           = "EGF_BLC1172", /*  DLC0700OZR-3 7" No Touch */
			.refresh        = 60,
			.xres           = 800,
			.yres           = 480,
			.pixclock       = 35000,
			.left_margin    = 46,
			.right_margin   = 20,
			.upper_margin   = 23,
			.lower_margin   = 10,
			.hsync_len      = 1,
			.vsync_len      = 1,
			.sync           = FB_SYNC_CLK_LAT_FALL | FB_SYNC_HOR_HIGH_ACT | FB_SYNC_VERT_HIGH_ACT,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	},
	/* Display supported by both PGF0533_A01 and PGF0533_A02 */
	{
		.bus	= 0,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= NULL,
		.enable	= lvds_disp_enable,
		.mode	= {
			.name           = "EGF_BLC1134", /* G121I1-L01 Innolux 12.1" */
			.refresh        = 60,
			.xres           = 1280,
			.yres           = 800,
			.pixclock       = 14084,
			.left_margin    = 80,
			.right_margin   = 80,
			.upper_margin   = 12,
			.lower_margin   = 11,
			.hsync_len      = 60,
			.vsync_len      = 10,
			.sync           = FB_SYNC_EXT,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	},


};
size_t display_count = ARRAY_SIZE(displays);

int board_video_skip(void)
{
	char * board_sw_id_code;
	int i;
	int ret;
	char panel[100];
	char const *panel_env = getenv("panel");

	panel[0] = 0;

	if (!panel_env){
		board_sw_id_code = gf_eeprom_get_board_sw_id_code();
		if (board_sw_id_code)
		{
			if(!gf_strcmp(board_sw_id_code, REV_WID0533_AA0101))
			{
				strcpy(panel, "EGF_BLC1133");
				setenv("panel", panel);
			}
			else if(!gf_strcmp(board_sw_id_code, REV_WID0533_AB0101))
			{
				strcpy(panel, "EGF_BLC1133");
				setenv("panel", panel);
			}
			else if(!gf_strcmp(board_sw_id_code, REV_WID0533_BA0101))
			{
				strcpy(panel, "EGF_BLC1168");
				setenv("panel", panel);
			}
			else if(!gf_strcmp(board_sw_id_code, REV_WID0533_BA0201))
			{
				strcpy(panel, "EGF_BLC1167");
				setenv("panel", panel);
			}
			else if(!gf_strcmp(board_sw_id_code, REV_WID0533_BB0101))
			{
				strcpy(panel, "EGF_BLC1149");
				setenv("panel", panel);
			}
			else if(!gf_strcmp(board_sw_id_code, REV_WID0533_BB0201))
			{
				strcpy(panel, "EGF_BLC1172");
				setenv("panel", panel);
			}
			else if(!gf_strcmp(board_sw_id_code, REV_WID0533_BB0301))
			{
				strcpy(panel, "EGF_BLC1152");
				setenv("panel", panel);
			}
			else if(!gf_strcmp(board_sw_id_code, REV_WID0533_BC0101))
			{
				strcpy(panel, "EGF_BLC1173");
				setenv("panel", panel);
			}

		}
	} else {
		strcpy(panel, panel_env);
	}

	if (!strlen(panel)) {
		for (i = 0; i < display_count; i++) {
			struct display_info_t const *dev = displays+i;
			if (dev->detect && dev->detect(dev)) {
				strcpy(panel,dev->mode.name);
				printf("auto-detected panel %s\n", panel);
				break;
			}
		}
		if (!strlen(panel)) {
			strcpy(panel,displays[0].mode.name);
			printf("No panel detected: default to %s\n", panel);
			i = 0;
		}
	} else {
		for (i = 0; i < display_count; i++) {
			if (!strcmp(panel, displays[i].mode.name))
				break;
		}
	}

	if (i < display_count) {
		ret = ipuv3_fb_init(&displays[i].mode, 0,
				    displays[i].pixfmt);
		if (!ret) {
			if (displays[i].enable)
				displays[i].enable(displays + i);

			printf("Display: %s (%ux%u)\n",
			       displays[i].mode.name,
			       displays[i].mode.xres,
			       displays[i].mode.yres);
		} else
			printf("LCD %s cannot be configured: %d\n",
			       displays[i].mode.name, ret);
	} else {
		printf("unsupported panel %s\n", panel);
		return -EINVAL;
	}

	return 0;
}

static void enable_vpll(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	int timeout = 100000;

	setbits_le32(&ccm->analog_pll_video, BM_ANADIG_PLL_VIDEO_POWERDOWN);

	// Set PLL5 POST_DIV_SELECT to 1 (divide by 2) and PLL5 POST_DIV_SELECT to 37
	// PLL Freq = Fref * (DIV_SELECT + NUM / DENOM) = 24M * (37 + 11/12) = 910
	// PLL5 post div select = 910 / 2 = 455 MHz
	// 455 MHz / 7 = 65 MHz (pixel clock)
	clrsetbits_le32(&ccm->analog_pll_video,
			BM_ANADIG_PLL_VIDEO_DIV_SELECT |
			(0x3 << 19),
			BF_ANADIG_PLL_VIDEO_DIV_SELECT(37) |
			(0x1 << 19));

	writel(BF_ANADIG_PLL_VIDEO_NUM_A(11), &ccm->analog_pll_video_num);
	writel(BF_ANADIG_PLL_VIDEO_DENOM_B(12), &ccm->analog_pll_video_denom);

	clrbits_le32(&ccm->analog_pll_video, BM_ANADIG_PLL_VIDEO_POWERDOWN);

	while (timeout--)
		if (readl(&ccm->analog_pll_video) & BM_ANADIG_PLL_VIDEO_LOCK)
			break;
	if (timeout < 0)
		printf("Warning: video pll lock timeout!\n");

	clrsetbits_le32(&ccm->analog_pll_video,
			BM_ANADIG_PLL_VIDEO_BYPASS,
			BM_ANADIG_PLL_VIDEO_ENABLE);
}

void select_ldb_di_clock_source_pll5(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	int reg;

	/*
	 * Need to follow a strict procedure when changing the LDB
	 * clock, else we can introduce a glitch. Things to keep in
	 * mind:
	 * 1. The current and new parent clocks must be disabled.
	 * 2. The default clock for ldb_dio_clk is mmdc_ch1 which has
	 * no CG bit.
	 * 3. In the RTL implementation of the LDB_DI_CLK_SEL mux
	 * the top four options are in one mux and the PLL3 option along
	 * with another option is in the second mux. There is third mux
	 * used to decide between the first and second mux.
	 * The code below switches the parent to the bottom mux first
	 * and then manipulates the top mux. This ensures that no glitch
	 * will enter the divider.
	 *
	 * Need to disable MMDC_CH1 clock manually as there is no CG bit
	 * for this clock. The only way to disable this clock is to move
	 * it to pll3_sw_clk and then to disable pll3_sw_clk
	 * Make sure periph2_clk2_sel is set to pll3_sw_clk
	 */

	/* Disable ldb_di clock parents */
	/* Disable PLL5 */
	reg = readl(&mxc_ccm->analog_pll_video);
	reg &= ~(1 << 13);
	writel(reg, &mxc_ccm->analog_pll_video);

	/* Set MMDC_CH1 mask bit */
	reg = readl(&mxc_ccm->ccdr);
	reg |= MXC_CCM_CCDR_MMDC_CH1_HS_MASK;
	writel(reg, &mxc_ccm->ccdr);

	/* Set periph2_clk2_sel to be sourced from PLL3_sw_clk */
	reg = readl(&mxc_ccm->cbcmr);
	reg &= ~(1<<20);
	writel(reg, &mxc_ccm->cbcmr);

	/*
	 * Set the periph2_clk_sel to the top mux so that
	 * mmdc_ch1 is from pll3_sw_clk.
	 */
	reg = readl(&mxc_ccm->cbcdr);
	reg |= (1<<26);
	writel(reg, &mxc_ccm->cbcdr);

	/* Wait for the clock switch */
	while (readl(&mxc_ccm->cdhipr))
		;

	/* Disable pll3_sw_clk by selecting bypass clock source */
	reg = readl(&mxc_ccm->ccsr);
	reg |= MXC_CCM_CCSR_PLL3_SW_CLK_SEL;
	writel(reg, &mxc_ccm->ccsr);

	/* Set the ldb_di0_clk and ldb_di1_clk to 111b */
	reg = readl(&mxc_ccm->cs2cdr);
	reg |= ((7 << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET)
	      | (7 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET));
	writel(reg, &mxc_ccm->cs2cdr);

	/* Set the ldb_di0_clk and ldb_di1_clk to 100b */
	reg = readl(&mxc_ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK
	      | MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK);
	reg |= ((4 << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET)
	      | (4 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET));
	writel(reg, &mxc_ccm->cs2cdr);

	/* Set the ldb_di0_clk and ldb_di1_clk to desired source */
	reg = readl(&mxc_ccm->cs2cdr);
	reg &= ~(MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK
	      | MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK);
	reg |= ((0 << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET)
	      | (0 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET));
	writel(reg, &mxc_ccm->cs2cdr);

	/* Unbypass pll3_sw_clk */
	reg = readl(&mxc_ccm->ccsr);
	reg &= ~MXC_CCM_CCSR_PLL3_SW_CLK_SEL;
	writel(reg, &mxc_ccm->ccsr);

	/*
	 * Set the periph2_clk_sel back to the bottom mux so that
	 * mmdc_ch1 is from its original parent.
	 */
	reg = readl(&mxc_ccm->cbcdr);
	reg &= ~(1<<26);
	writel(reg, &mxc_ccm->cbcdr);

	/* Wait for the clock switch */
	while (readl(&mxc_ccm->cdhipr))
		;

	/* Clear MMDC_CH1 mask bit */
	reg = readl(&mxc_ccm->ccdr);
	reg &= ~MXC_CCM_CCDR_MMDC_CH1_HS_MASK;
	writel(reg, &mxc_ccm->ccdr);

	/* Re-Enable PLL5 */
	reg = readl(&mxc_ccm->analog_pll_video);
	reg |= (1 << 13);
	writel(reg, &mxc_ccm->analog_pll_video);
}


static void setup_display(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int reg;

	enable_ipu_clock();
	enable_vpll();
	select_ldb_di_clock_source_pll5();

	/* Turn on LDB0, LDB1, IPU,IPU DI0 clocks */
	reg = readl(&mxc_ccm->CCGR3);
	reg |=  MXC_CCM_CCGR3_LDB_DI0_MASK | MXC_CCM_CCGR3_LDB_DI1_MASK;
	writel(reg, &mxc_ccm->CCGR3);

//	/* set LDB0, LDB1 clk select to 011/011 */
//	reg = readl(&mxc_ccm->cs2cdr);
//	reg &= ~(MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_MASK
//		 | MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_MASK);
//	reg |= (3 << MXC_CCM_CS2CDR_LDB_DI0_CLK_SEL_OFFSET)
//	      | (3 << MXC_CCM_CS2CDR_LDB_DI1_CLK_SEL_OFFSET);
//	writel(reg, &mxc_ccm->cs2cdr);

	reg = readl(&mxc_ccm->cscmr2);
	reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV | MXC_CCM_CSCMR2_LDB_DI1_IPU_DIV;
	writel(reg, &mxc_ccm->cscmr2);

	reg = readl(&mxc_ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<< MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<< MXC_CCM_CHSCCDR_IPU1_DI1_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->chsccdr);

	reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
	     | IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_LOW
	     | IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW
	     | IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG
	     | IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT
	     | IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
	     | IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT
	     | IOMUXC_GPR2_LVDS_CH0_MODE_DISABLED
	     | IOMUXC_GPR2_LVDS_CH1_MODE_ENABLED_DI0;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg = (reg & ~(IOMUXC_GPR3_LVDS1_MUX_CTL_MASK
			| IOMUXC_GPR3_HDMI_MUX_CTL_MASK))
	    | (IOMUXC_GPR3_MUX_SRC_IPU1_DI0
	       << IOMUXC_GPR3_LVDS1_MUX_CTL_OFFSET);
	writel(reg, &iomux->gpr[3]);
}
#endif /* CONFIG_VIDEO_IPUV3 */

/*
 * Do not overwrite the console
 * Use always serial for U-Boot console
 */
int overwrite_console(void)
{
	return 1;
}

int board_eth_init(bd_t *bis)
{
	if (is_mx6dqp()) {
		int ret;

		/* select ENET MAC0 TX clock from PLL */
		imx_iomux_set_gpr_register(5, 9, 1, 1);
		ret = enable_fec_anatop_clock(0, ENET_125MHZ);
		if (ret)
		    printf("Error fec anatop clock settings!\n");
	}

	/* Reset AR8035 PHY */
	gpio_direction_output(IMX_GPIO_NR(1, 25),0);
	udelay (500);
	gpio_set_value(IMX_GPIO_NR(1, 25), 1);
	udelay (500);

	return cpu_eth_init(bis);
}

#ifdef CONFIG_USB_EHCI_MX6
#define USB_OTHERREGS_OFFSET	0x800
#define UCTRL_PWR_POL		(1 << 9)

int board_ehci_hcd_init(int port)
{
	u32 *usbnc_usb_ctrl;

	if (port > 1)
		return -EINVAL;

	usbnc_usb_ctrl = (u32 *)(USB_BASE_ADDR + USB_OTHERREGS_OFFSET +
				 port * 4);

	setbits_le32(usbnc_usb_ctrl, UCTRL_PWR_POL);

	return 0;
}

int board_ehci_power(int port, int on)
{
	switch (port) {
	case 0:
		if (on){
			gpio_direction_output(USB_OTG_POWER_ENABLE_GPIO,1);
			mdelay(100);
		} else
			gpio_direction_output(USB_OTG_POWER_ENABLE_GPIO,0);
		break;
	case 1:
		if (on) {
			gpio_direction_output(USB_H1_POWER_ENABLE_GPIO,1);
			mdelay(100);
		} else
			gpio_direction_output(USB_H1_POWER_ENABLE_GPIO,0);
		break;
	default:
		printf("MXC USB port %d not yet supported\n", port);
		return -EINVAL;
	}

	return 0;
}
#endif

#ifdef CONFIG_MXC_SPI
static void setup_spinor(void ){
	gpio_direction_output(CONFIG_SF_CS_GPIO, 0);
	gpio_direction_output(CONFIG_SF_WPn_GPIO,0); //write protection on
}

int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	return (bus == 3 && cs == 1) ? (CONFIG_SF_CS_GPIO) : -1;
}
#endif

static void setup_eeprom(void){
	/* Set EEPROM write protected */
	gpio_direction_output(EEPROM_nWP_GPIO,0);
}

static void setup_power_management(void){
	gpio_direction_output(PWR_5V0_EN_3V3_GPIO,1);
	gpio_direction_output(VIO_3V3_EN,1);
}

int board_early_init_f(void)
{
	if (is_boot_from_usb()) {
		egf_board_common_mux_init(PROGRAMMER_MUX_MODE);
		printf("Is boot from usb! \n");
	}
	else {
		egf_board_common_mux_init(APPLICATION_MUX_MODE);
		printf("Is boot from usb! \n");
	}
	setup_power_management();
	setup_eeprom();

#if defined(CONFIG_VIDEO_IPUV3)
	setup_display();
#endif
#ifdef CONFIG_MXC_SPI
	setup_spinor();
#endif
	return 0;
}

int board_preserial_init(void)
{
	int pcb_rev;
	char * board_sw_id_code;
	gf_init_board_eeprom();

	pcb_rev = gf_get_pcb_rev();
	board_sw_id_code = gf_eeprom_get_board_sw_id_code();

	if (pcb_rev == PCB_REV_PGF0533_A03) {
		pgf_0533_a03_mux();
		if (!gf_strcmp(board_sw_id_code, REV_WID0533_BC0101)) {
			egf_wid0533bc0101_mux();
		}
	}
	else if (pcb_rev == PCB_REV_PGF0533_A02) {
		pgf_0533_a02_mux();
		if (!gf_strcmp(board_sw_id_code, REV_WID0533_BC0101)) {
			egf_wid0533bc0101_mux();
		}
	} else if(pcb_rev == PCB_REV_PGF0533_A01) {
		pgf_0533_a01_mux();
		if (!gf_strcmp(board_sw_id_code, REV_WID0533_AB0101)) {
			egf_wid0533ab0101_mux();
		}
	}

	return 0;
}

int board_init(void)
{
	char * egf_sw_id_code, * board_sw_id_code;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	if (!is_boot_from_usb())
	{
		gf_init_som_eeprom();
		egf_sw_id_code = gf_eeprom_get_som_sw_id_code();
		if (!egf_sw_id_code)
		{
			printf("System Hang.\n");
			while(1);
		} else {
			printf("SOM WID: %s\n", egf_sw_id_code);
		}
	}

	board_sw_id_code = gf_eeprom_get_board_sw_id_code();
	if (board_sw_id_code) {
		printf("Board WID: %s\n", board_sw_id_code);
	}

	return 0;

#if defined(CONFIG_MX6DL) && defined(CONFIG_MXC_EPDC)
	setup_epdc();
#endif

#ifdef CONFIG_CMD_SATA
	setup_sata();
#endif

	return 0;
}

static struct pmic *pfuze;
int power_init_board(void)
{
	unsigned int reg;
	int ret;

	pfuze = pfuze_common_init(I2C_PMIC);
	if (!pfuze)
		return -ENODEV;

	if (is_mx6dqp())
		ret = pfuze_mode_init(pfuze, APS_APS);
	else
		ret = pfuze_mode_init(pfuze, APS_PFM);

	if (ret < 0)
		return ret;

	if (is_mx6dqp()) {
		/* set SW1C staby volatage 1.075V*/
		pmic_reg_read(pfuze, PFUZE100_SW1CSTBY, &reg);
		reg &= ~0x3f;
		reg |= 0x1f;
		pmic_reg_write(pfuze, PFUZE100_SW1CSTBY, reg);

		/* set SW1C/VDDSOC step ramp up time to from 16us to 4us/25mV */
		pmic_reg_read(pfuze, PFUZE100_SW1CCONF, &reg);
		reg &= ~0xc0;
		reg |= 0x40;
		pmic_reg_write(pfuze, PFUZE100_SW1CCONF, reg);

		/* set SW2/VDDARM staby volatage 0.975V*/
		pmic_reg_read(pfuze, PFUZE100_SW2STBY, &reg);
		reg &= ~0x3f;
		reg |= 0x17;
		pmic_reg_write(pfuze, PFUZE100_SW2STBY, reg);

		/* set SW2/VDDARM step ramp up time to from 16us to 4us/25mV */
		pmic_reg_read(pfuze, PFUZE100_SW2CONF, &reg);
		reg &= ~0xc0;
		reg |= 0x40;
		pmic_reg_write(pfuze, PFUZE100_SW2CONF, reg);
	} else {
		/* set SW1AB staby volatage 0.975V*/
		pmic_reg_read(pfuze, PFUZE100_SW1ABSTBY, &reg);
		reg &= ~0x3f;
		reg |= 0x1b;
		pmic_reg_write(pfuze, PFUZE100_SW1ABSTBY, reg);

		/* set SW1AB/VDDARM step ramp up time from 16us to 4us/25mV */
		pmic_reg_read(pfuze, PFUZE100_SW1ABCONF, &reg);
		reg &= ~0xc0;
		reg |= 0x40;
		pmic_reg_write(pfuze, PFUZE100_SW1ABCONF, reg);

		/* set SW1C staby volatage 0.975V*/
		pmic_reg_read(pfuze, PFUZE100_SW1CSTBY, &reg);
		reg &= ~0x3f;
		reg |= 0x1b;
		pmic_reg_write(pfuze, PFUZE100_SW1CSTBY, reg);

		/* set SW1C/VDDSOC step ramp up time to from 16us to 4us/25mV */
		pmic_reg_read(pfuze, PFUZE100_SW1CCONF, &reg);
		reg &= ~0xc0;
		reg |= 0x40;
		pmic_reg_write(pfuze, PFUZE100_SW1CCONF, reg);
	}

	return 0;
}

#ifdef CONFIG_LDO_BYPASS_CHECK
void ldo_mode_set(int ldo_bypass)
{
	unsigned int value;
	int is_400M;
	unsigned char vddarm;
	struct pmic *p = pfuze;

	if (!p) {
		printf("No PMIC found!\n");
		return;
	}

	/* increase VDDARM/VDDSOC to support 1.2G chip */
	if (check_1_2G()) {
		ldo_bypass = 0;	/* ldo_enable on 1.2G chip */
		printf("1.2G chip, increase VDDARM_IN/VDDSOC_IN\n");
		if (is_mx6dqp()) {
			/* increase VDDARM to 1.425V */
			pmic_reg_read(p, PFUZE100_SW2VOL, &value);
			value &= ~0x3f;
			value |= 0x29;
			pmic_reg_write(p, PFUZE100_SW2VOL, value);
		} else {
			/* increase VDDARM to 1.425V */
			pmic_reg_read(p, PFUZE100_SW1ABVOL, &value);
			value &= ~0x3f;
			value |= 0x2d;
			pmic_reg_write(p, PFUZE100_SW1ABVOL, value);
		}
		/* increase VDDSOC to 1.425V */
		pmic_reg_read(p, PFUZE100_SW1CVOL, &value);
		value &= ~0x3f;
		value |= 0x2d;
		pmic_reg_write(p, PFUZE100_SW1CVOL, value);
	}
	/* switch to ldo_bypass mode , boot on 800Mhz */
	if (ldo_bypass) {
		prep_anatop_bypass();
		if (is_mx6dqp()) {
			/* decrease VDDARM for 400Mhz DQP:1.1V*/
			pmic_reg_read(p, PFUZE100_SW2VOL, &value);
			value &= ~0x3f;
			value |= 0x1c;
			pmic_reg_write(p, PFUZE100_SW2VOL, value);
		} else {
			/* decrease VDDARM for 400Mhz DQ:1.1V, DL:1.275V */
			pmic_reg_read(p, PFUZE100_SW1ABVOL, &value);
			value &= ~0x3f;
			if(is_cpu_type(MXC_CPU_MX6DL) || is_cpu_type(MXC_CPU_MX6SOLO)) {
				value |= 0x27;
			} else {
				value |= 0x20;
			}
			pmic_reg_write(p, PFUZE100_SW1ABVOL, value);
		}
		/* increase VDDSOC to 1.3V */
		pmic_reg_read(p, PFUZE100_SW1CVOL, &value);
		value &= ~0x3f;
		value |= 0x28;
		pmic_reg_write(p, PFUZE100_SW1CVOL, value);

		/*
		 * MX6Q/DQP:
		 * VDDARM:1.15V@800M; VDDSOC:1.175V@800M
		 * VDDARM:0.975V@400M; VDDSOC:1.175V@400M
		 * MX6DL:
		 * VDDARM:1.175V@800M; VDDSOC:1.175V@800M
		 * VDDARM:1.075V@400M; VDDSOC:1.175V@400M
		 */
		is_400M = set_anatop_bypass(2);
		if (is_mx6dqp()) {
			pmic_reg_read(p, PFUZE100_SW2VOL, &value);
			value &= ~0x3f;
			if (is_400M)
				value |= 0x17;
			else
				value |= 0x1e;
			pmic_reg_write(p, PFUZE100_SW2VOL, value);
		}

		if (is_400M) {
			if(is_cpu_type(MXC_CPU_MX6DL) || is_cpu_type(MXC_CPU_MX6SOLO))
				vddarm = 0x22;
			else
				vddarm = 0x1b;
		} else {
			if(is_cpu_type(MXC_CPU_MX6DL) || is_cpu_type(MXC_CPU_MX6SOLO))
				vddarm = 0x23;
			else
				vddarm = 0x22;
		}
		pmic_reg_read(p, PFUZE100_SW1ABVOL, &value);
		value &= ~0x3f;
		value |= vddarm;
		pmic_reg_write(p, PFUZE100_SW1ABVOL, value);

		/* decrease VDDSOC to 1.175V */
		pmic_reg_read(p, PFUZE100_SW1CVOL, &value);
		value &= ~0x3f;
		value |= 0x23;
		pmic_reg_write(p, PFUZE100_SW1CVOL, value);

		finish_anatop_bypass();
		printf("switch to ldo_bypass mode!\n");
	}
}
#endif

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"sd2",	 MAKE_CFGVAL(0x40, 0x28, 0x00, 0x00)},
	{"sd3",	 MAKE_CFGVAL(0x40, 0x30, 0x00, 0x00)},
	/* 8 bit bus width */
	{"emmc", MAKE_CFGVAL(0x60, 0x58, 0x00, 0x00)},
	{NULL,	 0},
};
#endif

int board_late_init(void)
{
	if (!is_boot_from_usb())
		prepare_boot_env();

#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif

#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif
	return 0;
}

int checkboard(void)
{
	puts("Board: MX6 Panel PC - Elettronica GF\n");
	return 0;
}

#ifdef CONFIG_FSL_FASTBOOT

void board_fastboot_setup(void)
{
	switch (get_boot_device()) {
#if defined(CONFIG_FASTBOOT_STORAGE_SATA)
	case SATA_BOOT:
		if (!getenv("fastboot_dev"))
			setenv("fastboot_dev", "sata");
		if (!getenv("bootcmd"))
			setenv("bootcmd", "boota sata");
		break;
#endif /*CONFIG_FASTBOOT_STORAGE_SATA*/
#if defined(CONFIG_FASTBOOT_STORAGE_MMC)
	case SD2_BOOT:
	case MMC2_BOOT:
	    if (!getenv("fastboot_dev"))
			setenv("fastboot_dev", "mmc0");
	    if (!getenv("bootcmd"))
			setenv("bootcmd", "boota mmc0");
	    break;
	case SD3_BOOT:
	case MMC3_BOOT:
	    if (!getenv("fastboot_dev"))
			setenv("fastboot_dev", "mmc1");
	    if (!getenv("bootcmd"))
			setenv("bootcmd", "boota mmc1");
	    break;
	case MMC4_BOOT:
	    if (!getenv("fastboot_dev"))
			setenv("fastboot_dev", "mmc2");
	    if (!getenv("bootcmd"))
			setenv("bootcmd", "boota mmc2");
	    break;
#endif /*CONFIG_FASTBOOT_STORAGE_MMC*/
	default:
		printf("unsupported boot devices\n");
		break;
	}

}

#ifdef CONFIG_ANDROID_RECOVERY

#define GPIO_VOL_DN_KEY IMX_GPIO_NR(1, 5)

int check_recovery_cmd_file(void)
{
    int button_pressed = 0;
    int recovery_mode = 0;

    recovery_mode = recovery_check_and_clean_flag();

    gpio_direction_input(GPIO_VOL_DN_KEY);

    if (gpio_get_value(GPIO_VOL_DN_KEY) == 0) { /* VOL_DN key is low assert */
		button_pressed = 1;
		printf("Recovery key pressed\n");
    }

    return recovery_mode || button_pressed;
}

void board_recovery_setup(void)
{
	int bootdev = get_boot_device();

	switch (bootdev) {
#if defined(CONFIG_FASTBOOT_STORAGE_SATA)
	case SATA_BOOT:
		if (!getenv("bootcmd_android_recovery"))
			setenv("bootcmd_android_recovery",
				"boota sata recovery");
		break;
#endif /*CONFIG_FASTBOOT_STORAGE_SATA*/
#if defined(CONFIG_FASTBOOT_STORAGE_MMC)
	case SD2_BOOT:
	case MMC2_BOOT:
		if (!getenv("bootcmd_android_recovery"))
			setenv("bootcmd_android_recovery",
				"boota mmc0 recovery");
		break;
	case SD3_BOOT:
	case MMC3_BOOT:
		if (!getenv("bootcmd_android_recovery"))
			setenv("bootcmd_android_recovery",
				"boota mmc1 recovery");
		break;
	case MMC4_BOOT:
		if (!getenv("bootcmd_android_recovery"))
			setenv("bootcmd_android_recovery",
				"boota mmc2 recovery");
		break;
#endif /*CONFIG_FASTBOOT_STORAGE_MMC*/
	default:
		printf("Unsupported bootup device for recovery: dev: %d\n",
			bootdev);
		return;
	}

	printf("setup env for recovery..\n");
	setenv("bootcmd", "run bootcmd_android_recovery");
}

#endif /*CONFIG_ANDROID_RECOVERY*/

#endif /*CONFIG_FSL_FASTBOOT*/

#ifdef CONFIG_SPL_BUILD
#include <asm/arch/mx6-ddr.h>
#include <spl.h>
#include <libfdt.h>

#define MT41K128M16JT_125 		1
#define MT41K256M16TW_107 		2
#define K4B4G1646D_BMK0 		3
#define DDR_BUS_WIDTH_16BIT		16
#define DDR_BUS_WIDTH_32BIT		32
#define DDR_BUS_WIDTH_64BIT		64

struct egf_som {
	int ram_model;
	int ram_bus_width;
	int ram_cs_used;
	void * iomux_ddr_regs;
	void * iomux_grp_regs;
	void * mmdc_calibration;
};

static struct egf_som __attribute__((section (".data"))) the_som;

static struct egf_som the_som_WID_0510_AA0101 = {
		MT41K128M16JT_125,
		DDR_BUS_WIDTH_32BIT,
		1,
		&mx6sdl_ddr_ioregs_standard,
		&mx6sdl_grp_ioregs_standard,
		&mx6sdl_128x16_mmdc_calib_default,
};

static struct egf_som the_som_WID_0510_AB0101 = {
		MT41K128M16JT_125,
		DDR_BUS_WIDTH_32BIT,
		1,
		&mx6dq_ddr_ioregs_standard,
		&mx6dq_grp_ioregs_standard,
		&mx6dq_128x16_mmdc_calib_default,
};

static struct egf_som the_som_WID_0510_AC0101 = {
		MT41K128M16JT_125,
		DDR_BUS_WIDTH_64BIT,
		1,
		&mx6sdl_ddr_ioregs_standard,
		&mx6sdl_grp_ioregs_standard,
		&mx6sdl_128x16_mmdc_calib_x64,
};

static struct egf_som the_som_WID_0510_AC0102 = {
		MT41K128M16JT_125,
		DDR_BUS_WIDTH_64BIT,
		1,
		&mx6sdl_ddr_ioregs_standard,
		&mx6sdl_grp_ioregs_standard,
		&mx6sdl_128x16_mmdc_calib_x64,
};

static struct egf_som the_som_WID_0510_AD0101 = {
		MT41K256M16TW_107,
		DDR_BUS_WIDTH_64BIT,
		1,
		&mx6dq_ddr_ioregs_standard,
		&mx6dq_grp_ioregs_standard,
		&mx6dq_128x16_mmdc_calib_x64,
};

static struct egf_som the_som_WID_0510_AE0101 = {
		MT41K128M16JT_125,
		DDR_BUS_WIDTH_64BIT,
		1,
		&mx6dq_ddr_ioregs_standard,
		&mx6dq_grp_ioregs_standard,
		&mx6dq_128x16_mmdc_calib_x64,
};

static struct egf_som the_som_WID_0510_AE0102 = {
		MT41K128M16JT_125,
		DDR_BUS_WIDTH_64BIT,
		1,
		&mx6dq_ddr_ioregs_standard,
		&mx6dq_grp_ioregs_standard,
		&mx6dq_128x16_mmdc_calib_x64,
};

static struct egf_som the_som_WID_0510_AF0101 = {
		MT41K128M16JT_125,
		DDR_BUS_WIDTH_32BIT,
		1,
		&mx6sdl_ddr_ioregs_standard,
		&mx6sdl_grp_ioregs_standard,
		&mx6sdl_128x16_mmdc_calib_default,
};

static struct egf_som the_som_WID_0510_AF0102 = {
		MT41K128M16JT_125,
		DDR_BUS_WIDTH_32BIT,
		1,
		&mx6sdl_ddr_ioregs_standard,
		&mx6sdl_grp_ioregs_standard,
		&mx6sdl_128x16_mmdc_calib_default,
};

static struct egf_som the_som_WID_0510_AG0101 = {
		K4B4G1646D_BMK0,
		DDR_BUS_WIDTH_32BIT,
		1,
		&mx6sdl_ddr_ioregs_standard,
		&mx6sdl_grp_ioregs_standard,
		&mx6s_128x16_mmdc_calib_smr4711,
};

static struct egf_som the_som_WID_0510_AG0102 = {
		K4B4G1646D_BMK0,
		DDR_BUS_WIDTH_32BIT,
		1,
		&mx6sdl_ddr_ioregs_standard,
		&mx6sdl_grp_ioregs_standard,
		&mx6s_128x16_mmdc_calib_smr4711,
};

static struct egf_som the_som_WID_0510_AJ0101 = {
		MT41K128M16JT_125,
		DDR_BUS_WIDTH_64BIT,
		1,
		&mx6sdl_ddr_ioregs_standard,
		&mx6sdl_grp_ioregs_standard,
		&mx6sdl_128x16_mmdc_calib_x64,
};

static struct egf_som the_som_WID_0510_AJ0102 = {
		MT41K128M16JT_125,
		DDR_BUS_WIDTH_64BIT,
		1,
		&mx6sdl_ddr_ioregs_standard,
		&mx6sdl_grp_ioregs_standard,
		&mx6sdl_128x16_mmdc_calib_x64,
};

static struct egf_som the_som_WID_0510_AK0101 = {
		MT41K128M16JT_125,
		DDR_BUS_WIDTH_64BIT,
		1,
		&mx6dq_ddr_ioregs_standard,
		&mx6dq_grp_ioregs_standard,
		&mx6dq_128x16_mmdc_calib_x64,
};

static struct egf_som the_som_WID_0510_AK0102 = {
		MT41K128M16JT_125,
		DDR_BUS_WIDTH_64BIT,
		1,
		&mx6dq_ddr_ioregs_standard,
		&mx6dq_grp_ioregs_standard,
		&mx6dq_128x16_mmdc_calib_x64,
};

static struct egf_som the_som_WID_0510_AN0101 = {
		MT41K128M16JT_125,
		DDR_BUS_WIDTH_32BIT,
		1,
		&mx6sdl_ddr_ioregs_standard,
		&mx6sdl_grp_ioregs_standard,
		&mx6sdl_128x16_mmdc_calib_default,
};

int load_som_revision(void)
{
	char * egf_som_sw_id_code;

	gf_init_som_eeprom();
	egf_som_sw_id_code = gf_eeprom_get_som_sw_id_code();
	if (!egf_som_sw_id_code)
	{
		printf("System Hang.\n");
		while(1);
 	}

	printf("SOM WID: %s\n", egf_som_sw_id_code);

	if(!gf_strcmp(egf_som_sw_id_code,REV_WID0510_AA0101))
	{
		memcpy(&the_som, &the_som_WID_0510_AA0101, sizeof(the_som));
	}
	else if(!gf_strcmp(egf_som_sw_id_code,REV_WID0510_AB0101))
	{
		memcpy(&the_som, &the_som_WID_0510_AB0101, sizeof(the_som));
	}
	else if(!gf_strcmp(egf_som_sw_id_code,REV_WID0510_AC0101))
	{
		memcpy(&the_som, &the_som_WID_0510_AC0101, sizeof(the_som));
	}
	else if(!gf_strcmp(egf_som_sw_id_code,REV_WID0510_AC0102))
	{
		memcpy(&the_som, &the_som_WID_0510_AC0102, sizeof(the_som));
	}
	else if(!gf_strcmp(egf_som_sw_id_code,REV_WID0510_AD0101))
	{
		memcpy(&the_som, &the_som_WID_0510_AD0101, sizeof(the_som));
	}
	else if(!gf_strcmp(egf_som_sw_id_code,REV_WID0510_AE0101))
	{
		memcpy(&the_som, &the_som_WID_0510_AE0101, sizeof(the_som));
	}
	else if(!gf_strcmp(egf_som_sw_id_code,REV_WID0510_AE0102))
	{
		memcpy(&the_som, &the_som_WID_0510_AE0102, sizeof(the_som));
	}
	else if(!gf_strcmp(egf_som_sw_id_code,REV_WID0510_AF0101))
	{
		memcpy(&the_som, &the_som_WID_0510_AF0101, sizeof(the_som));
	}
	else if(!gf_strcmp(egf_som_sw_id_code,REV_WID0510_AF0102))
	{
		memcpy(&the_som, &the_som_WID_0510_AF0102, sizeof(the_som));
	}
	else if(!gf_strcmp(egf_som_sw_id_code,REV_WID0510_AG0101))
	{
		memcpy(&the_som, &the_som_WID_0510_AG0101, sizeof(the_som));
	}
	else if(!gf_strcmp(egf_som_sw_id_code,REV_WID0510_AG0102))
	{
		memcpy(&the_som, &the_som_WID_0510_AG0102, sizeof(the_som));
	}
	else if(!gf_strcmp(egf_som_sw_id_code,REV_WID0510_AJ0101))
	{
		memcpy(&the_som, &the_som_WID_0510_AJ0101, sizeof(the_som));
	}
	else if(!gf_strcmp(egf_som_sw_id_code,REV_WID0510_AJ0102))
	{
		memcpy(&the_som, &the_som_WID_0510_AJ0102, sizeof(the_som));
	}
	else if(!gf_strcmp(egf_som_sw_id_code,REV_WID0510_AK0101))
	{
		memcpy(&the_som, &the_som_WID_0510_AK0101, sizeof(the_som));
	}
	else if(!gf_strcmp(egf_som_sw_id_code,REV_WID0510_AK0102))
	{
		memcpy(&the_som, &the_som_WID_0510_AK0102, sizeof(the_som));
	}
	else if(!gf_strcmp(egf_som_sw_id_code,REV_WID0510_AN0101))
	{
		memcpy(&the_som, &the_som_WID_0510_AN0101, sizeof(the_som));
	}
	else {
		printf("Unrecognized EGF SW ID Code: %s\n",egf_som_sw_id_code);
		printf("System Hang.\n");
		while(1);
	}
	return 0;
}

static void ccgr_init(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	writel(0x00C03F3F, &ccm->CCGR0);
	writel(0x0030FCC0, &ccm->CCGR1);
	writel(0x0FFFC000, &ccm->CCGR2);
	writel(0x3FF00000, &ccm->CCGR3);
	writel(0x00FFF300, &ccm->CCGR4);
	writel(0x0F0000C3, &ccm->CCGR5);
	writel(0x000003FF, &ccm->CCGR6);
}

static void gpr_init(void)
{
	struct iomuxc *iomux = (struct iomuxc *)IOMUXC_BASE_ADDR;

	/* enable AXI cache for VDOA/VPU/IPU */
	writel(0xF00000CF, &iomux->gpr[4]);
	/* set IPU AXI-id0 Qos=0xf(bypass) AXI-id1 Qos=0x7 */
	writel(0x007F007F, &iomux->gpr[6]);
	writel(0x007F007F, &iomux->gpr[7]);
}

/*
 * This section requires the differentiation between iMX6 Sabre boards, but
 * for now, it will configure only for the mx6q variant.
 */
static void spl_dram_init(void)
{
	struct mx6_ddr3_cfg *memory_timings = NULL;
	struct mx6_mmdc_calibration *memory_calib = NULL;
	struct mx6_ddr_sysinfo sysinfo = {
		/* width of data bus:0=16,1=32,2=64 */
		.dsize = the_som.ram_bus_width/32,
		/* config for full 4GB range so that get_mem_size() works */
		.cs_density = 32, /* 32Gb per CS */
		/* single chip select */
		.ncs = the_som.ram_cs_used,
		.cs1_mirror = 0,
		.rtt_wr = 1 /*DDR3_RTT_60_OHM*/,	/* RTT_Wr = RZQ/4 */
#ifdef RTT_NOM_120OHM
		.rtt_nom = 2 /*DDR3_RTT_120_OHM*/,	/* RTT_Nom = RZQ/2 */
#else
		.rtt_nom = 1 /*DDR3_RTT_60_OHM*/,	/* RTT_Nom = RZQ/4 */
#endif
		.walat = 1,	/* Write additional latency */
		.ralat = 5,	/* Read additional latency */
		.mif3_mode = 3,	/* Command prediction working mode */
		.bi_on = 1,	/* Bank interleaving enabled */
		.sde_to_rst = 0x10,	/* 14 cycles, 200us (JEDEC default) */
		.rst_to_cke = 0x23,	/* 33 cycles, 500us (JEDEC default) */
	};

	/*
	 * MMDC Calibration requires the following data:
	 *   mx6_mmdc_calibration - board-specific calibration (routing delays)
	 *      these calibration values depend on board routing, SoC, and DDR
	 *   mx6_ddr_sysinfo - board-specific memory architecture (width/cs/etc)
	 *   mx6_ddr_cfg - chip specific timing/layout details
	 */
	switch(the_som.ram_model)
	{
	case MT41K128M16JT_125:
		memory_timings = &mt41k128m16jt_125;
		break;
	case MT41K256M16TW_107:
		memory_timings = &mt41k256m16tw_107;
		break;
	case K4B4G1646D_BMK0:
		memory_timings = &k4b4g1646d_bmk0;
		break;
	default:
		puts("Error: Invalid Memory Configuration\n");
		hang();
	}

	memory_calib = the_som.mmdc_calibration;

	if (!memory_timings) {
		puts("Error: Invalid Memory Configuration\n");
		hang();
	}
	if (!memory_calib) {
		puts("Error: Invalid Board Calibration Configuration\n");
		hang();
	}

	if (is_cpu_type(MXC_CPU_MX6Q) || is_cpu_type(MXC_CPU_MX6D))
	{
		mx6dq_dram_iocfg(the_som.ram_bus_width, (struct mx6dq_iomux_ddr_regs *)the_som.iomux_ddr_regs,
					 (struct mx6dq_iomux_grp_regs *)the_som.iomux_grp_regs);
	} else
	{
		mx6sdl_dram_iocfg(the_som.ram_bus_width, (struct mx6sdl_iomux_ddr_regs *)the_som.iomux_ddr_regs,
					 (struct mx6sdl_iomux_grp_regs *)the_som.iomux_grp_regs);
	}
	mx6_dram_cfg(&sysinfo, memory_calib, memory_timings);
}

static void fix_clocks(void)
{
	struct mxc_ccm_reg *ccm_regs = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	u32 reg;

	if (is_cpu_type(MXC_CPU_MX6Q) || is_cpu_type(MXC_CPU_MX6D)){
		//Switch periph_clk2_sel to OSC
		reg = readl(&ccm_regs->cbcmr);
		reg &= ~MXC_CCM_CBCMR_PERIPH_CLK2_SEL_MASK;
		reg |= 1 << MXC_CCM_CBCMR_PERIPH_CLK2_SEL_OFFSET;
		writel(reg, &ccm_regs->cbcmr);
		//Switch periph_clk_sel to periph_clk2
		reg = readl(&ccm_regs->cbcdr);
		reg |= MXC_CCM_CBCDR_PERIPH_CLK_SEL;
		writel(reg, &ccm_regs->cbcdr);
		//Wait till busy bits clear
		while((readl(&ccm_regs->cdhipr) & 0x3F) != 0);
		//Set pre_periph_clk_sel to PLL2
		reg = readl(&ccm_regs->cbcmr);
		reg &= ~MXC_CCM_CBCMR_PRE_PERIPH_CLK_SEL_MASK;
		writel(reg, &ccm_regs->cbcmr);
		//Switch back periph_clk_sel to pre_periph_clk
		reg = readl(&ccm_regs->cbcdr);
		reg &= ~MXC_CCM_CBCDR_PERIPH_CLK_SEL;
		writel(reg, &ccm_regs->cbcdr);
		//Wait for busy bits to clear
		while((readl(&ccm_regs->cdhipr) & 0x3F) != 0);
	} else {
		//Switch periph_clk2_sel to OSC
		reg = readl(&ccm_regs->cbcmr);
		reg &= ~MXC_CCM_CBCMR_PERIPH_CLK2_SEL_MASK;
		reg |= 1 << MXC_CCM_CBCMR_PERIPH_CLK2_SEL_OFFSET;
		writel(reg, &ccm_regs->cbcmr);
		//Switch periph_clk_sel to periph_clk2
		reg = readl(&ccm_regs->cbcdr);
		reg |= MXC_CCM_CBCDR_PERIPH_CLK_SEL;
		writel(reg, &ccm_regs->cbcdr);
		//Wait for busy bits to clear
		while((readl(&ccm_regs->cdhipr) & 0x3F) != 0);
		//Set pre_periph_clk_sel to PLL2 PFD2
		reg = readl(&ccm_regs->cbcmr);
		reg &= ~MXC_CCM_CBCMR_PRE_PERIPH_CLK_SEL_MASK;
		reg |= 1 << MXC_CCM_CBCMR_PRE_PERIPH_CLK_SEL_OFFSET;
		writel(reg, &ccm_regs->cbcmr);
		//Set AHB_PODF to 3
		reg = readl(&ccm_regs->cbcdr);
		reg &= ~MXC_CCM_CBCDR_AHB_PODF_MASK;
		reg |= (0x2) << MXC_CCM_CBCDR_AHB_PODF_OFFSET;
		//Set periph_clk_sel to pll2
		reg &= ~MXC_CCM_CBCDR_PERIPH_CLK_SEL;
		writel(reg, &ccm_regs->cbcdr);
		//Wait for busy bits to clear
		while((readl(&ccm_regs->cdhipr) & 0x1003F) != 0);
	}
}

void board_init_f(ulong dummy)
{
	char * board_sw_id_code;

	fix_clocks();
	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();
	gpr_init();

	/* iomux and setup of i2c */
	board_early_init_f();

	/* setup GP timer */
	timer_init();

	board_preserial_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	if (!is_boot_from_usb()) {
		/* Carica EEPROM */
		load_som_revision();
	}

	board_sw_id_code = gf_eeprom_get_board_sw_id_code();
	if(board_sw_id_code)
		printf("Board WID: %s\n", board_sw_id_code);

	/* DDR initialization */
	spl_dram_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}

void reset_cpu(ulong addr)
{
}
#endif

#ifdef CONFIG_CFB_CONSOLE
# ifdef CONFIG_CONSOLE_EXTRA_INFO
# include <video_fb.h>
extern GraphicDevice smi;

void video_get_info_str (int line_number, char *info)
{
	u32 cpurev;
	char * egf_sw_id_code;
	char * egf_serial_number;
	char * egf_product_code;

	if (!is_boot_from_usb())
	{
		switch (line_number) {
		case 1:
			sprintf (info, " ");
			return;
		case 2:
			sprintf (info, " Elettronica GF s.r.l.");
			return;
		case 3:
			cpurev = get_cpu_rev();
			sprintf (info, " CPU: Freescale i.MX%s rev%d.%d at %d MHz",
					get_imx_type((cpurev & 0xFF000) >> 12),
					(cpurev & 0x000F0) >> 4,
					(cpurev & 0x0000F) >> 0,
					mxc_get_clock(MXC_ARM_CLK) / 1000000);
			return;
		case 4:
			sprintf (info, " DRAM: %d MiB", imx_ddr_size() / (1024 * 1024));
			return;
		case 5:
			egf_sw_id_code = gf_eeprom_get_som_sw_id_code();
			if(egf_sw_id_code)
				sprintf (info, " WID: %s", egf_sw_id_code);
			return;
		case 6:
			egf_product_code = gf_eeprom_get_som_code();
			if(egf_product_code)
				sprintf (info, " Model: %s", egf_product_code);
			return;
		case 7:
			egf_serial_number = gf_eeprom_get_som_serial_number();
			if(egf_serial_number)
				sprintf (info, " Board SN: %s", egf_serial_number);
			return;
		default:
			break;
		}
	} else {
		switch (line_number) {
		case 1:
			sprintf (info, " ");
			return;
		case 2:
			sprintf (info, " Programmazione SPL e u-boot su SPI");
			return;
		case 3:
			sprintf (info, " NOR Flash in corso...");
			return;
		case 4:
			sprintf (info, " Lo spegnimento del display indica il");
			return;
		case 5:
			sprintf (info, " completamento della programmazione.");
			return;
		default:
			break;
		}
	}
	/* no more info lines */
	*info = 0;
	return;
}
# endif	/* CONFIG_CONSOLE_EXTRA_INFO */
#endif	/* CONFIG_CFB_CONSOLE */

