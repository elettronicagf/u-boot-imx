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
#include <spi.h>

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
#include "ddr_mx6/WID0500_AA01.01.c"
#include "ddr_mx6/WID0500_AB01.01.c"
#include "ddr_mx6/WID0500_AC01.01.c"
#endif
DECLARE_GLOBAL_DATA_PTR;

#define WID_LENGTH					15
#define EGF_FDT_FILE_NAME_LENGTH	(13 + WID_LENGTH + 1)

#define I2C_PMIC	1

#define PICOS2KHZ(a) (1000000000UL/(a))

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	return 0;
}

void prepare_boot_env(void)
{
	char * egf_sw_id_code;
	char* mac_address;
	char fdt_file_name[EGF_FDT_FILE_NAME_LENGTH];

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
	 * 0x1                  SD3 - eMMC
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
		reg |= IOMUXC_GPR2_DATA_WIDTH_CH0_24BIT;
	else
		reg |= IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT;
	writel(reg, &iomux->gpr[2]);
}

/* Adjust display pixel clock frequency to 12.8 MHz for BLC1071 */
static void vpll_adjust_frequency(void){
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	unsigned int timeout = 100000;
	unsigned int div_select;
	unsigned int num, denom;

	/* Bypass PLL5 */
	setbits_le32(&ccm->analog_pll_video, BM_ANADIG_PLL_VIDEO_BYPASS);
	/* Power Down PLL5 */
	setbits_le32(&ccm->analog_pll_video, BM_ANADIG_PLL_VIDEO_POWERDOWN);

	div_select = 30;
	num = 1;
	denom = 1000000;

	// Set PLL5 POST_DIV_SELECT to 1 (divide by 2), VIDEO_DIV to 3 (divide by 4) and PLL5 DIV_SELECT to 30
	// PLL Freq = Fref * (DIV_SELECT + NUM / DENOM) = 24M * (30 + 1/1000000) = 720M
	// PLL5 post div select = 720 / 2 = 360 MHz
	// PLL5 post video div = 360 / 4 = 90 MHz
	// 90 MHz / 7 = 12.85 MHz (pixel clock)

	clrsetbits_le32(&ccm->analog_pll_video,
			BM_ANADIG_PLL_VIDEO_DIV_SELECT |
			(0x3 << 19),
			BF_ANADIG_PLL_VIDEO_DIV_SELECT(div_select) |
			(0x1 << 19));

	writel(BF_ANADIG_PLL_VIDEO_NUM_A(num), &ccm->analog_pll_video_num);
	writel(BF_ANADIG_PLL_VIDEO_DENOM_B(denom), &ccm->analog_pll_video_denom);

	setbits_le32(0x20c8170, 0x3<<30);
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

/***  AMOLED *******/

typedef struct SpiCommandType {
  unsigned char addr;								// Register address
  unsigned char data;								// Register data
} SpiCommand;

static SpiCommand spi_display_init_commands[]= {						// Init command table
  {0x04, 0x23}, //DISPLAY_MODE2: Mode 480RGBx272 + 24Bit Parallel RGB + DE
  {0x05, 0x82}, //DISPLAY_MODE3: Stripe RGB + S160->S1
  {0x03, 0x03}, //Adattato per Beagle HSync VSync (alti)  DE (basso) CLK(discesa)
  {0x07, 0x0F}, //DRIVER_CAPABILITY: 67%
  {0x34, 0x18}, //
  {0x35, 0x28}, //T4: 40 DCLK
  {0x36, 0x16}, //TF: 22 DCLK
  {0x37, 0x01}, //TB:  1 DCLK
  {0x02, 0x00}, //OTPOFF
  {0x0A, 0x79}, //VGHVGL=+/- 6V
  {0x09, 0x28}, //VGAM1OUT= 5.02V
  {0x10, 0x77}, //R_SLOPE: Gamma correction
  {0x11, 0x76}, //G_SLOPE: Gamma correction
  {0x12, 0x77}, //B_SLOPE: Gamma correction
  {0x13, 0x00}, //R_GAMMA0
  {0x14, 0x00}, //R_GAMMA10
  {0x15, 0x00}, //R_GAMMA36
  {0x16, 0x00}, //R_GAMMA80
  {0x17, 0x00}, //R_GAMMA124
  {0x18, 0x00}, //R_GAMMA168
  {0x19, 0x01}, //R_GAMMA212
  {0x1A, 0x03}, //R_GAMMA255
  {0x1B, 0x00}, //G_GAMMA0
  {0x1C, 0x03}, //G_GAMMA10
  {0x1D, 0x00}, //G_GAMMA36
  {0x1E, 0x02}, //G_GAMMA80
  {0x1F, 0x01}, //G_GAMMA124
  {0x20, 0x02}, //G_GAMMA168
  {0x21, 0x02}, //G_GAMMA212
  {0x22, 0x06}, //G_GAMMA255
  {0x23, 0x00}, //B_GAMMA0
  {0x24, 0x02}, //B_GAMMA10
  {0x25, 0x01}, //B_GAMMA36
  {0x26, 0x02}, //B_GAMMA80
  {0x27, 0x02}, //B_GAMMA124
  {0x28, 0x03}, //B_GAMMA168
  {0x29, 0x03}, //B_GAMMA212
  {0x2A, 0x08}, //B_GAMMA255
  {0x06, 0x03}, //POWER_CTRL1: No Reset + Normal Mode (No Standby)
};

static void send_display_spi_command(struct spi_slave *slave, SpiCommand* command, int delay_us)
{
  unsigned int txBuffer = 0;
  unsigned int rxBuffer = 0;
  udelay(delay_us);
  txBuffer = (command->data << 8) & 0xFF00;
  txBuffer |= (((command->addr << 1) & 0xFE));
  spi_xfer(slave, 16, &txBuffer, &rxBuffer, SPI_XFER_BEGIN | SPI_XFER_END);
}

static void init_display(void)
{
	struct spi_slave *slave;
	int i;
	int n_commands;

	printf("EGF Amoled init display\n");
	slave = spi_setup_slave(CONFIG_AMOLED_SPI_BUS, CONFIG_AMOLED_CS, 1000000, CONFIG_AMOLED_SPI_MODE);
	if (!slave) {
		puts("unable to setup slave\n");
		return;
	}
	if (spi_claim_bus(slave)) {
		spi_free_slave(slave);
		return;
	}
	gpio_direction_output(GPIO_DISPLAY_NRESET, 0);
	gpio_direction_output(DISP0_EN,0);
	udelay(300);
	gpio_direction_output(DISP0_EN,1);
	udelay(100);;
	gpio_direction_output(GPIO_DISPLAY_NRESET, 1);
	udelay(600);
	n_commands = sizeof(spi_display_init_commands)/sizeof(SpiCommand);
	for(i=0;i<n_commands;i++){
		send_display_spi_command(slave, &spi_display_init_commands[i],250);
	}
	spi_release_bus(slave);
}

static void blc1071_enable(struct display_info_t const *dev)
{
	vpll_adjust_frequency();
	enable_lvds(dev);
}

struct display_info_t const displays[] = {
	{
		.bus	= 0,
		.addr	= 0,
		.pixfmt	= IPU_PIX_FMT_RGB24,
		.detect	= NULL,
		.enable	= blc1071_enable,
		.mode	= {
			.name           = "EGF_BLC1071", /*  KWH070KQ13-F02 Formike 7.0" */
			.refresh        = 60,
			.xres           = 480,
			.yres           = 272,
			/* HACK: Pixel clock set to 50MHz to avoid a bug in ipu timings management.
			 * Checked all timings are calculated correctly, pixel clock is gen by pll5
			 * If set to 12MHz, timings are not calculated correctly and image appear
			 * stretched.
			 */
			.pixclock       = 20000,
			.left_margin    = 102,
			.right_margin   = 30,
			.upper_margin   = 20,
			.lower_margin   = 10,
			.hsync_len      = 30,
			.vsync_len      = 3,
			.sync 			= FB_SYNC_EXT,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	},
};
size_t display_count = ARRAY_SIZE(displays);

static void enable_vpll(void)
{
	struct mxc_ccm_reg *ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;
	int timeout = 100000;

	setbits_le32(&ccm->analog_pll_video, BM_ANADIG_PLL_VIDEO_POWERDOWN);

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

	select_ldb_di_clock_source_pll5();
	enable_vpll();

	reg = readl(&mxc_ccm->cscmr2);
	reg |= MXC_CCM_CSCMR2_LDB_DI0_IPU_DIV | MXC_CCM_CSCMR2_LDB_DI1_IPU_DIV;
	writel(reg, &mxc_ccm->cscmr2);

	reg = readl(&mxc_ccm->chsccdr);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<< MXC_CCM_CHSCCDR_IPU1_DI0_CLK_SEL_OFFSET);
	reg |= (CHSCCDR_CLK_SEL_LDB_DI0
		<< MXC_CCM_CHSCCDR_IPU1_DI1_CLK_SEL_OFFSET);
	writel(reg, &mxc_ccm->chsccdr);

	/* Turn on LDB0, LDB1, IPU,IPU DI0 clocks */
	reg = readl(&mxc_ccm->CCGR3);
	reg |=  MXC_CCM_CCGR3_LDB_DI0_MASK | MXC_CCM_CCGR3_LDB_DI1_MASK;
	writel(reg, &mxc_ccm->CCGR3);

	enable_ipu_clock();

	reg = IOMUXC_GPR2_BGREF_RRMODE_EXTERNAL_RES
	     | IOMUXC_GPR2_DI1_VS_POLARITY_ACTIVE_LOW
	     | IOMUXC_GPR2_DI0_VS_POLARITY_ACTIVE_LOW
	     | IOMUXC_GPR2_BIT_MAPPING_CH1_SPWG
	     | IOMUXC_GPR2_DATA_WIDTH_CH1_18BIT
	     | IOMUXC_GPR2_BIT_MAPPING_CH0_SPWG
	     | IOMUXC_GPR2_DATA_WIDTH_CH0_18BIT
		 | IOMUXC_GPR2_LVDS_CH0_MODE_ENABLED_DI0
	     | IOMUXC_GPR2_LVDS_CH1_MODE_DISABLED;
	writel(reg, &iomux->gpr[2]);

	reg = readl(&iomux->gpr[3]);
	reg = (reg & ~(IOMUXC_GPR3_LVDS0_MUX_CTL_MASK
			| IOMUXC_GPR3_HDMI_MUX_CTL_MASK))
	    | (IOMUXC_GPR3_MUX_SRC_IPU1_DI0
	       << IOMUXC_GPR3_LVDS0_MUX_CTL_OFFSET);
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

	if (port > 0)
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
		if (on)
			gpio_direction_output(USB_OTG_PWR_EN_GPIO, 1);
		else
			gpio_direction_output(USB_OTG_PWR_EN_GPIO, 0);
		break;
	case 1:
		if (on)
			gpio_direction_output(USB_H1_PWR_EN_GPIO, 1);
		else
			gpio_direction_output(USB_H1_PWR_EN_GPIO, 0);
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
	if (bus == 3 && cs == 1)
		return CONFIG_SF_CS_GPIO;
	if (bus == 2 && cs == 1) {
		printf("Asked for amoled cs\n");
		return CONFIG_AMOLED_CS_GPIO;
	}
	return -1;
}
#endif

static void setup_eeprom(void){
	/* Set EEPROM write protected */
	gpio_direction_output(EEPROM_nWP_GPIO,0);
}

int board_early_init_f(void)
{
	if (is_boot_from_usb()) {
		egf_board_mux_init(PROGRAMMER_MUX_MODE);
		printf("Is boot from usb! \n");
	}
	else {
		egf_board_mux_init(APPLICATION_MUX_MODE);
		printf("Is boot from usb! \n");
	}
	setup_eeprom();

#if defined(CONFIG_VIDEO_IPUV3)
	setup_display();
#endif
#ifdef CONFIG_MXC_SPI
	setup_spinor();
#endif
	return 0;
}

int board_init(void)
{
	char * egf_sw_id_code;
	int ret;

	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	if (!is_boot_from_usb())
	{
		ret = gf_load_som_revision(&egf_sw_id_code,0);
		if (ret)
		{
			printf("System Hang.\n");
			while(1);
		}
	}

	init_display();

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
	puts("Board: Q29 i.MX6 SOM Evaluation Board - Elettronica GF\n");
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

/* SW REVISIONS*/
#define REV_WID0500_AA0101 "WID0500_AA01.01"
#define REV_WID0500_AB0101 "WID0500_AB01.01"
#define REV_WID0500_AC0101 "WID0500_AC01.01"

#define MT41K128M16JT_125 		1
#define MT41K256M16HA_125 		2
#define DDR_BUS_WIDTH_16BIT		16
#define DDR_BUS_WIDTH_32BIT		32
#define DDR_BUS_WIDTH_64BIT		64

struct egf_som {
	void (*ddr_initialization_script)(void);
};

static struct egf_som __attribute__((section (".data"))) the_som;

static struct egf_som the_som_WID_0500_AA0101 = {
	WID0500_AA01_01_ddr_setup,
};

static struct egf_som the_som_WID_0500_AB0101 = {
	WID0500_AB01_01_ddr_setup,
};

static struct egf_som the_som_WID_0500_AC0101 = {
	WID0500_AC01_01_ddr_setup,
};

static int gf_strcmp(const char * cs, const char * ct) {
	register signed char __res;

	while (1) {
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
	}

	return __res;
}

int load_revision(void)
{
	char * egf_sw_id_code;
	int ret;

	ret = gf_load_som_revision(&egf_sw_id_code,0);
	if (ret)
	{
		printf("System Hang.\n");
		while(1);
 	}

	if(!gf_strcmp(egf_sw_id_code,REV_WID0500_AA0101))
	{
		/* SW Revision is WID0500_AA01.01 */
		printf("GF Software ID Code: WID0500_AA01.01\n");
		memcpy(&the_som, &the_som_WID_0500_AA0101, sizeof(the_som));
	}
	else if (!gf_strcmp(egf_sw_id_code,REV_WID0500_AB0101))
	{
		/* SW Revision is WID0500_AB01.01 */
		printf("GF Software ID Code: WID0500_AB01.01\n");
		memcpy(&the_som, &the_som_WID_0500_AB0101, sizeof(the_som));
	}
	else if (!gf_strcmp(egf_sw_id_code,REV_WID0500_AC0101))
	{
		/* SW Revision is WID0500_AC01.01 */
		printf("GF Software ID Code: WID0500_AC01.01\n");
		memcpy(&the_som, &the_som_WID_0500_AC0101, sizeof(the_som));
	}
	else {
		printf("Unrecognized EGF SW ID Code: %s\n",egf_sw_id_code);
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
	the_som.ddr_initialization_script();
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
		//Set pre_periph_clk_sel to PLL2 PFD2
		reg = readl(&ccm_regs->cbcmr);
		reg &= ~MXC_CCM_CBCMR_PRE_PERIPH_CLK_SEL_MASK;
		reg |= (0x1 << MXC_CCM_CBCMR_PRE_PERIPH_CLK_SEL_OFFSET);
		writel(reg, &ccm_regs->cbcmr);
		//Set AHB_PODF to 3
		reg = readl(&ccm_regs->cbcdr);
		reg &= ~MXC_CCM_CBCDR_AHB_PODF_MASK;
		reg |= (0x2) << MXC_CCM_CBCDR_AHB_PODF_OFFSET;
		//Set periph_clk_sel to pll2
		reg &= ~MXC_CCM_CBCDR_PERIPH_CLK_SEL;
		writel(reg, &ccm_regs->cbcdr);
		//Wait for busy bits to clear
		while((readl(&ccm_regs->cdhipr) & 0x3F) != 0);

		//Disable MMDC_CH1 handshake
		reg = readl(&ccm_regs->ccdr);
		reg |= (1 << 16);
		writel(reg, &ccm_regs->ccdr);
		//Switch periph2_clk2_sel to pll2
		reg = readl(&ccm_regs->cbcmr);
		reg |= 1 << 20;
		writel(reg, &ccm_regs->cbcmr);

		//Switch periph2_clk_sel to periph2_clk2
		reg = readl(&ccm_regs->cbcdr);
		reg |= MXC_CCM_CBCDR_PERIPH2_CLK_SEL;
		writel(reg, &ccm_regs->cbcdr);
		//Wait till busy bits clear
		while((readl(&ccm_regs->cdhipr) & 0x3F) != 0);

		//Set pre_periph2_clk_sel to PLL2
		reg = readl(&ccm_regs->cbcmr);
		reg &= ~MXC_CCM_CBCMR_PRE_PERIPH2_CLK_SEL_MASK;
		writel(reg, &ccm_regs->cbcmr);
		//Set periph2_clk_sel to pll2
		reg = readl(&ccm_regs->cbcdr);
		reg &= ~MXC_CCM_CBCDR_PERIPH2_CLK_SEL;
		writel(reg, &ccm_regs->cbcdr);
		//Wait for busy bits to clear
		while((readl(&ccm_regs->cdhipr) & 0x3F) != 0);

		//Enable MMDC_CH1 handshake
		reg = readl(&ccm_regs->ccdr);
		reg &= ~(1 << 16);
		writel(reg, &ccm_regs->ccdr);


	}
}

void p_udelay(int time)
{
	int i, j;

	for (i = 0; i < time; i++) {
		for (j = 0; j < 200; j++) {
			asm("nop");
			asm("nop");
		}
	}
}

void board_init_f(ulong dummy)
{
	fix_clocks();
	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();
	gpr_init();

	/* iomux and setup of i2c */
	board_early_init_f();

	/* setup GP timer */
	timer_init();

	/* UART clocks enabled and gd valid - init serial console */
	preloader_console_init();

	if (!is_boot_from_usb()) {
		/* Carica EEPROM */
		load_revision();
	}

	/* DDR initialization */
	spl_dram_init();

	p_udelay(1000);

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
