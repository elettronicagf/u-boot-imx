/*
 * Copyright (C) 2015-2016 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mx7-pins.h>
#include <asm/errno.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/io.h>
#include <linux/sizes.h>
#include <common.h>
#include <fsl_esdhc.h>
#include <mmc.h>
#include <miiphy.h>
#include <netdev.h>
#include <power/pmic.h>
#include <power/pfuze3000_pmic.h>
#include "../common/pfuze.h"
#include <i2c.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/arch/crm_regs.h>
#include <usb.h>
#include <usb/ehci-fsl.h>
#include "gf_eeprom.h"
#include "gf_eeprom_port.h"
#include "gf_mux.h"
#if defined(CONFIG_MXC_EPDC)
#include <lcd.h>
#include <mxc_epdc_fb.h>
#endif
#include <asm/imx-common/video.h>

#ifdef CONFIG_FSL_FASTBOOT
#include <fsl_fastboot.h>
#ifdef CONFIG_ANDROID_RECOVERY
#include <recovery.h>
#endif
#endif /*CONFIG_FSL_FASTBOOT*/

DECLARE_GLOBAL_DATA_PTR;

#define DIO_PUP_STR_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_PUS_PU5KOHM | \
	PAD_CTL_DSE_3P3V_49OHM)
/* SW REVISIONS*/
#define REV_WID0547_AA0101 "WID0547_AA01.01"

static char *  __attribute__((section (".data"))) egf_sw_id_code;

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
	printf("Reading EEPROM for DRAM configuration...\n");
	if (!is_boot_from_usb()) {
		/* Carica EEPROM */
		load_revision();
	}
	printf("EGF SW ID: %s", egf_sw_id_code);
	if(!gf_strcmp(egf_sw_id_code,REV_WID0547_AA0101))
		gd->ram_size = SZ_2G;
	else
		gd->ram_size = PHYS_SDRAM_SIZE;
	printf("\nDRAM:  ");
	return 0;
}

#define BOARD_REV_C  0x300
#define BOARD_REV_B  0x200
#define BOARD_REV_A  0x100

static int mx7sabre_rev(void)
{
	/*
	 * Get Board ID information from OCOTP_GP1[15:8]
	 * i.MX7D SDB RevA: 0x41
	 * i.MX7D SDB RevB: 0x42
	 */
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[14];
	int reg = readl(&bank->fuse_regs[0]);
	int ret;

	if (reg != 0) {
		switch (reg >> 8 & 0x0F) {
		case 0x3:
			ret = BOARD_REV_C;
			break;
		case 0x02:
			ret = BOARD_REV_B;
			break;
		case 0x01:
		default:
			ret = BOARD_REV_A;
			break;
		}
	} else {
		/* If the gp1 fuse is not burn, we have to use TO rev for the board rev */
		if (is_soc_rev(CHIP_REV_1_0))
			ret = BOARD_REV_A;
		else if (is_soc_rev(CHIP_REV_1_1))
			ret = BOARD_REV_B;
		else
			ret = BOARD_REV_C;
	}

	return ret;
}

u32 get_board_rev(void)
{
	int rev = mx7sabre_rev();

	return (get_cpu_rev() & ~(0xF << 8)) | rev;
}

int load_revision(void)
{
	int ret;

	ret = gf_load_som_revision(&egf_sw_id_code,0);
	if (ret)
	{
		printf("System Hang.\n");
		while(1);
 	}

	if(!gf_strcmp(egf_sw_id_code,REV_WID0547_AA0101))
	{
		/* SW Revision is WID0547_AA01.01 */
		printf("GF Software ID Code: WID0547_AA01.01\n");
	}
	else {
		printf("Unrecognized EGF SW ID Code: %s\n",egf_sw_id_code);
		printf("System Hang.\n");
		while(1);
	}
	return 0;
}


#ifdef CONFIG_NAND_MXS
static void setup_gpmi_nand(void)
{
	/* NAND_USDHC_BUS_CLK is set in rom */
	set_clk_nand();
}
#endif

#ifdef CONFIG_VIDEO_MXS

void blc1143_enable(struct display_info_t const *dev)
{
	/* Enable LCD */
	gpio_direction_output(DISP_VDD_EN_GPIO, 1);
	gpio_direction_output(DISP_EN_GPIO , 1);
	udelay(500);

	/* Set Brightness to high */
	gpio_direction_output(DISP_BL_CONTROL_PWM_GPIO , 1);
}

struct display_info_t const displays[] = {
	{
		.bus	= ELCDIF1_IPS_BASE_ADDR,
		.addr	= 0,
		.pixfmt	= 24,
		.detect	= NULL,
		.enable	= blc1143_enable,
		.mode	= {
			.name           = "EGF_BLC1143", /* DLC0700OZG-T */
			.refresh        = 60,
			.xres           = 800,
			.yres           = 480,
			.pixclock       = 30030,
			.left_margin    = 210,
			.right_margin   = 46,
			.upper_margin   = 22,
			.lower_margin   = 23,
			.hsync_len      = 10,
			.vsync_len      = 10,
			.sync           = 0,
			.vmode          = FB_VMODE_NONINTERLACED
		}
	},
};

size_t display_count = ARRAY_SIZE(displays);
#endif

#ifdef CONFIG_FSL_ESDHC

static struct fsl_esdhc_cfg usdhc_cfg[3] = {
	{USDHC2_BASE_ADDR, 0, 4},
	{USDHC3_BASE_ADDR, 0, 8},
};

int board_mmc_get_env_dev(int devno)
{
	if (devno == 2)
		devno--;

	return devno;
}

int mmc_map_to_kernel_blk(int dev_no)
{
	if (dev_no == 1)
		dev_no++;

	return dev_no;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC2_BASE_ADDR:
		ret = !gpio_get_value(USDHC2_CD_GPIO);
		break;
	case USDHC3_BASE_ADDR:
		ret = 1; /* Assume uSDHC3 emmc is always present */
		break;
	}

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	int i, ret;
	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-Boot device node)    (Physical Port)
	 * mmc0                    USDHC2
	 * mmc1                    USDHC3 (eMMC)
	 */
	for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			gpio_request(USDHC2_CD_GPIO, "usdhc2_cd");
			gpio_direction_input(USDHC2_CD_GPIO);
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
			break;
		case 1:
			usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
				"(%d) than supported by the board\n", i + 1);
			return -EINVAL;
			}

			ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
			if (ret)
				return ret;
	}

	return 0;
}
#endif

#ifdef CONFIG_FEC_MXC

int board_eth_init(bd_t *bis)
{
	int ret;
	/* Reset LAN8720 PHY */
	gpio_direction_output(ENET_NRST_GPIO,0);
	udelay (1000);
	gpio_set_value(ENET_NRST_GPIO, 1);
	udelay (500);

	ret = fecmxc_initialize_multi(bis, 1,
		CONFIG_FEC_MXC_PHYADDR, IMX_FEC_BASE);
	if (ret)
		printf("FEC1 MXC: %s:failed\n", __func__);

	return ret;
}

static int setup_fec(void)
{
	struct iomuxc_gpr_base_regs *const iomuxc_gpr_regs
		= (struct iomuxc_gpr_base_regs *) IOMUXC_GPR_BASE_ADDR;

	/* Use 50M anatop REF_CLK2 for ENET2, clear gpr1[14], set gpr1[18]
	 * enable on the pin
	 */
	clrsetbits_le32(&iomuxc_gpr_regs->gpr[1],
					IOMUXC_GPR_GPR1_GPR_ENET2_TX_CLK_SEL_MASK,
					IOMUXC_GPR_GPR1_GPR_ENET2_CLK_DIR_MASK);

	return set_clk_enet(ENET_50MHz);

}


int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);
	return 0;
}
#endif

#ifdef CONFIG_FSL_QSPI

int board_qspi_init(void)
{
	/* Set the clock */
	set_clk_qspi();

	return 0;
}
#endif

#ifdef CONFIG_MXC_EPDC
static iomux_v3_cfg_t const epdc_enable_pads[] = {
	MX7D_PAD_EPDC_DATA00__EPDC_DATA0	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX7D_PAD_EPDC_DATA01__EPDC_DATA1	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX7D_PAD_EPDC_DATA02__EPDC_DATA2	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX7D_PAD_EPDC_DATA03__EPDC_DATA3	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX7D_PAD_EPDC_DATA04__EPDC_DATA4	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX7D_PAD_EPDC_DATA05__EPDC_DATA5	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX7D_PAD_EPDC_DATA06__EPDC_DATA6	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX7D_PAD_EPDC_DATA07__EPDC_DATA7	| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX7D_PAD_EPDC_SDCLK__EPDC_SDCLK		| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX7D_PAD_EPDC_SDLE__EPDC_SDLE		| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX7D_PAD_EPDC_SDOE__EPDC_SDOE		| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX7D_PAD_EPDC_SDSHR__EPDC_SDSHR		| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX7D_PAD_EPDC_SDCE0__EPDC_SDCE0		| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX7D_PAD_EPDC_SDCE1__EPDC_SDCE1		| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX7D_PAD_EPDC_GDCLK__EPDC_GDCLK		| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX7D_PAD_EPDC_GDOE__EPDC_GDOE		| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX7D_PAD_EPDC_GDRL__EPDC_GDRL		| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX7D_PAD_EPDC_GDSP__EPDC_GDSP		| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX7D_PAD_EPDC_BDR0__EPDC_BDR0		| MUX_PAD_CTRL(EPDC_PAD_CTRL),
	MX7D_PAD_EPDC_BDR1__EPDC_BDR1		| MUX_PAD_CTRL(EPDC_PAD_CTRL),
};

static iomux_v3_cfg_t const epdc_disable_pads[] = {
	MX7D_PAD_EPDC_DATA00__GPIO2_IO0,
	MX7D_PAD_EPDC_DATA01__GPIO2_IO1,
	MX7D_PAD_EPDC_DATA02__GPIO2_IO2,
	MX7D_PAD_EPDC_DATA03__GPIO2_IO3,
	MX7D_PAD_EPDC_DATA04__GPIO2_IO4,
	MX7D_PAD_EPDC_DATA05__GPIO2_IO5,
	MX7D_PAD_EPDC_DATA06__GPIO2_IO6,
	MX7D_PAD_EPDC_DATA07__GPIO2_IO7,
	MX7D_PAD_EPDC_SDCLK__GPIO2_IO16,
	MX7D_PAD_EPDC_SDLE__GPIO2_IO17,
	MX7D_PAD_EPDC_SDOE__GPIO2_IO18,
	MX7D_PAD_EPDC_SDSHR__GPIO2_IO19,
	MX7D_PAD_EPDC_SDCE0__GPIO2_IO20,
	MX7D_PAD_EPDC_SDCE1__GPIO2_IO21,
	MX7D_PAD_EPDC_GDCLK__GPIO2_IO24,
	MX7D_PAD_EPDC_GDOE__GPIO2_IO25,
	MX7D_PAD_EPDC_GDRL__GPIO2_IO26,
	MX7D_PAD_EPDC_GDSP__GPIO2_IO27,
	MX7D_PAD_EPDC_BDR0__GPIO2_IO28,
	MX7D_PAD_EPDC_BDR1__GPIO2_IO29,
};

vidinfo_t panel_info = {
	.vl_refresh = 85,
	.vl_col = 1024,
	.vl_row = 758,
	.vl_pixclock = 40000000,
	.vl_left_margin = 12,
	.vl_right_margin = 76,
	.vl_upper_margin = 4,
	.vl_lower_margin = 5,
	.vl_hsync = 12,
	.vl_vsync = 2,
	.vl_sync = 0,
	.vl_mode = 0,
	.vl_flag = 0,
	.vl_bpix = 3,
	.cmap = 0,
};

struct epdc_timing_params panel_timings = {
	.vscan_holdoff = 4,
	.sdoed_width = 10,
	.sdoed_delay = 20,
	.sdoez_width = 10,
	.sdoez_delay = 20,
	.gdclk_hp_offs = 524,
	.gdsp_offs = 327,
	.gdoe_offs = 0,
	.gdclk_offs = 19,
	.num_ce = 1,
};

static void setup_epdc_power(void)
{
	/* IOMUX_GPR1: bit30: Disable On-chip RAM EPDC Function */
	struct iomuxc_gpr_base_regs *const iomuxc_gpr_regs
		= (struct iomuxc_gpr_base_regs *) IOMUXC_GPR_BASE_ADDR;

	clrsetbits_le32(&iomuxc_gpr_regs->gpr[1],
		IOMUXC_GPR_GPR1_GPR_ENABLE_OCRAM_EPDC_MASK, 0);

	/* Setup epdc voltage */

	/* EPDC_PWRSTAT - GPIO2[31] for PWR_GOOD status */
	imx_iomux_v3_setup_pad(MX7D_PAD_EPDC_PWR_STAT__GPIO2_IO31 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));
	gpio_direction_input(IMX_GPIO_NR(2, 31));

	/* EPDC_VCOM0 - GPIO4[14] for VCOM control */
	imx_iomux_v3_setup_pad(MX7D_PAD_I2C4_SCL__GPIO4_IO14 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));

	/* Set as output */
	gpio_direction_output(IMX_GPIO_NR(4, 14), 1);

	/* EPDC_PWRWAKEUP - GPIO2[23] for EPD PMIC WAKEUP */
	imx_iomux_v3_setup_pad(MX7D_PAD_EPDC_SDCE3__GPIO2_IO23 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));
	/* Set as output */
	gpio_direction_output(IMX_GPIO_NR(2, 23), 1);

	/* EPDC_PWRCTRL0 - GPIO2[30] for EPD PWR CTL0 */
	imx_iomux_v3_setup_pad(MX7D_PAD_EPDC_PWR_COM__GPIO2_IO30 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));
	/* Set as output */
	gpio_direction_output(IMX_GPIO_NR(2, 30), 1);
}

static void epdc_enable_pins(void)
{
	/* epdc iomux settings */
	imx_iomux_v3_setup_multiple_pads(epdc_enable_pads,
				ARRAY_SIZE(epdc_enable_pads));
}

static void epdc_disable_pins(void)
{
	/* Configure MUX settings for EPDC pins to GPIO  and drive to 0 */
	imx_iomux_v3_setup_multiple_pads(epdc_disable_pads,
				ARRAY_SIZE(epdc_disable_pads));
}

static void setup_epdc(void)
{
	/*** epdc Maxim PMIC settings ***/

	/* EPDC_PWRSTAT - GPIO2[31] for PWR_GOOD status */
	imx_iomux_v3_setup_pad(MX7D_PAD_EPDC_PWR_STAT__GPIO2_IO31 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));

	/* EPDC_VCOM0 - GPIO4[14] for VCOM control */
	imx_iomux_v3_setup_pad(MX7D_PAD_I2C4_SCL__GPIO4_IO14 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));

	/* EPDC_PWRWAKEUP - GPIO4[23] for EPD PMIC WAKEUP */
	imx_iomux_v3_setup_pad(MX7D_PAD_EPDC_SDCE3__GPIO2_IO23 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));

	/* EPDC_PWRCTRL0 - GPIO4[20] for EPD PWR CTL0 */
	imx_iomux_v3_setup_pad(MX7D_PAD_EPDC_PWR_COM__GPIO2_IO30 |
				MUX_PAD_CTRL(EPDC_PAD_CTRL));

	/* Set pixel clock rates for EPDC in clock.c */

	panel_info.epdc_data.wv_modes.mode_init = 0;
	panel_info.epdc_data.wv_modes.mode_du = 1;
	panel_info.epdc_data.wv_modes.mode_gc4 = 3;
	panel_info.epdc_data.wv_modes.mode_gc8 = 2;
	panel_info.epdc_data.wv_modes.mode_gc16 = 2;
	panel_info.epdc_data.wv_modes.mode_gc32 = 2;

	panel_info.epdc_data.epdc_timings = panel_timings;

	setup_epdc_power();
}

void epdc_power_on(void)
{
	unsigned int reg;
	struct gpio_regs *gpio_regs = (struct gpio_regs *)GPIO2_BASE_ADDR;

	/* Set EPD_PWR_CTL0 to high - enable EINK_VDD (3.15) */
	gpio_set_value(IMX_GPIO_NR(2, 30), 1);
	udelay(1000);

	/* Enable epdc signal pin */
	epdc_enable_pins();

	/* Set PMIC Wakeup to high - enable Display power */
	gpio_set_value(IMX_GPIO_NR(2, 23), 1);

	/* Wait for PWRGOOD == 1 */
	while (1) {
		reg = readl(&gpio_regs->gpio_psr);
		if (!(reg & (1 << 31)))
			break;

		udelay(100);
	}

	/* Enable VCOM */
	gpio_set_value(IMX_GPIO_NR(4, 14), 1);

	udelay(500);
}

void epdc_power_off(void)
{
	/* Set PMIC Wakeup to low - disable Display power */
	gpio_set_value(IMX_GPIO_NR(2, 23), 0);

	/* Disable VCOM */
	gpio_set_value(IMX_GPIO_NR(4, 14), 0);

	epdc_disable_pins();

	/* Set EPD_PWR_CTL0 to low - disable EINK_VDD (3.15) */
	gpio_set_value(IMX_GPIO_NR(2, 30), 0);
}
#endif

#ifdef CONFIG_USB_EHCI_MX7

int board_usb_phy_mode(int port)
{
	if (port == 0)
		return usb_phy_mode(port);
	else
		return USB_INIT_HOST;
}
#endif

int board_early_init_f(void)
{
	if (is_boot_from_usb()) {
		egf_board_mux_init(PROGRAMMER_MUX_MODE);
		printf("Boot from USB: recovery mode\n");
	}
	else {
		egf_board_mux_init(APPLICATION_MUX_MODE);
	}
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_FEC_MXC
	setup_fec();
#endif

#ifdef CONFIG_NAND_MXS
	setup_gpmi_nand();
#endif

#ifdef CONFIG_FSL_QSPI
	board_qspi_init();
#endif

#ifdef CONFIG_MXC_EPDC
	setup_epdc();
#endif

	return 0;
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"sd1", MAKE_CFGVAL(0x10, 0x10, 0x00, 0x00)},
	{"emmc", MAKE_CFGVAL(0x10, 0x2a, 0x00, 0x00)},
	/* TODO: Nand */
	{"qspi", MAKE_CFGVAL(0x00, 0x40, 0x00, 0x00)},
	{NULL,   0},
};
#endif

#ifdef CONFIG_POWER
#define I2C_PMIC	0
int power_init_board(void)
{
	struct pmic *p;
	int ret;
	unsigned int reg, rev_id;

	ret = power_pfuze3000_init(I2C_PMIC);
	if (ret)
		return ret;

	p = pmic_get("PFUZE3000");
	ret = pmic_probe(p);
	if (ret)
		return ret;

	pmic_reg_read(p, PFUZE3000_DEVICEID, &reg);
	pmic_reg_read(p, PFUZE3000_REVID, &rev_id);
	printf("PMIC: PFUZE3000 DEV_ID=0x%x REV_ID=0x%x\n", reg, rev_id);

	/* disable Low Power Mode during standby mode */
	pmic_reg_read(p, PFUZE3000_LDOGCTL, &reg);
	reg |= 0x1;
	pmic_reg_write(p, PFUZE3000_LDOGCTL, reg);

	/* SW1A/1B mode set to APS/APS */
	reg = 0x8;
	pmic_reg_write(p, PFUZE3000_SW1AMODE, reg);
	pmic_reg_write(p, PFUZE3000_SW1BMODE, reg);

	/* SW1A/1B standby voltage set to 0.975V */
	reg = 0xb;
	pmic_reg_write(p, PFUZE3000_SW1ASTBY, reg);
	pmic_reg_write(p, PFUZE3000_SW1BSTBY, reg);

	/* decrease SW1B normal voltage to 0.975V */
	pmic_reg_read(p, PFUZE3000_SW1BVOLT, &reg);
	reg &= ~0x1f;
	reg |= PFUZE3000_SW1AB_SETP(975);
	pmic_reg_write(p, PFUZE3000_SW1BVOLT, reg);

	return 0;
}
#endif

int board_late_init(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;
#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif

#ifdef CONFIG_ENV_IS_IN_MMC
	board_late_mmc_env_init();
#endif

	set_wdog_reset(wdog);

	return 0;
}

int checkboard(void)
{
	printf("Board: EGF i.MX7D Evaluation Board\n");
	return 0;
}

#if defined(CONFIG_SPL_BUILD)
#include <spl.h>

/* 2GiB DDR3 SMR4712 x 2 chip */
void ddr3_wid0571aa0101_init(void)
{
	writel(0x00000002, 0x30391000); 	// deassert presetn
	writel(0x03040001, 0x307A0000); 	// DDRC_MSTR
	writel(0x0040005E, 0x307A0064); 	// DDRC_RFSHTMG
	writel(0x00000001, 0x307a0490);		// DDRC_PCTRL_0
	writel(0x00690000, 0x307A00D4); 	// DDRC_INIT1 (if using LPDDR3/LPDDR2, this line is automatically commented out)
	writel(0x00020083, 0x307A00D0);		// DDRC_INIT0
	writel(0x09300004, 0x307A00DC); 	// DDRC_INIT3
	writel(0x04080000, 0x307A00E0); 	// DDRC_INIT4
	writel(0x00100004, 0x307A00E4); 	// DDRC_INIT5
	writel(0x0000033F, 0x307A00F4); 	// DDRC_RANKCTL
	writel(0x090A110A, 0x307A0100); 	// DDRC_DRAMTMG0
	writel(0x0007020E, 0x307A0104); 	// DDRC_DRAMTMG1
	writel(0x03040407, 0x307A0108); 	// DDRC_DRAMTMG2
	writel(0x00002006, 0x307A010C); 	// DDRC_DRAMTMG3
	writel(0x04020204, 0x307A0110); 	// DDRC_DRAMTMG4
	writel(0x03030202, 0x307A0114); 	// DDRC_DRAMTMG5
	writel(0x00000803, 0x307A0120); 	// DDRC_DRAMTMG8
	writel(0x00800020, 0x307A0180); 	// DDRC_ZQCTL0
	writel(0x02098204, 0x307A0190); 	// DDRC_DFITMG0
	writel(0x00030303, 0x307A0194); 	// DDRC_DFITMG1
	writel(0x80400003, 0x307A01A0); 	// DDRC_DFIUPD0
	writel(0x00100020, 0x307A01A4); 	// DDRC_DFIUPD1
	writel(0x80100004, 0x307A01A8); 	// DDRC_DFIUPD2
	writel(0x00000016, 0x307A0200); 	// DDRC_ADDRMAP0
	writel(0x00080808, 0x307A0204); 	// DDRC_ADDRMAP1
	writel(0x00000F0F, 0x307A0210); 	// DDRC_ADDRMAP4
	writel(0x07070707, 0x307A0214); 	// DDRC_ADDRMAP5
	writel(0x0F070707, 0x307A0218); 	// DDRC_ADDRMAP6
	writel(0x06000604, 0x307A0240); 	// DDRC_ODTCFG
	writel(0x00001323, 0x307A0244); 	// DDRC_ODTMAP

	writel(0x00000000, 0x30391000); 	// deassert presetn
	writel(0x17420F40, 0x30790000); 	// DDR_PHY_PHY_CON0
	writel(0x10210100, 0x30790004); 	// DDR_PHY_PHY_CON1
	writel(0x00060807, 0x30790010); 	// DDR_PHY_PHY_CON4
	writel(0x1010007E, 0x307900B0); 	// DDR_PHY_MDLL_CON0
	writel(0x00000D6E, 0x3079009C); 	// DDR_PHY_DRVDS_CON0

	writel(0x06060606, 0x30790030); 	// DDR_PHY_OFFSET_WR_CON0
	writel(0x0C0C0C0C, 0x30790020); 	// DDR_PHY_OFFSET_RD_CON0
	writel(0x01000010, 0x30790050); 	// DDR_PHY_OFFSETD_CON0
	writel(0x00000010, 0x30790050); 	// DDR_PHY_OFFSETD_CON0
	writel(0x0000000F, 0x30790018); 	// DDR_PHY_LP_CON0
	writel(0x0E407304, 0x307900C0); 	// DDR_PHY_ZQ_CON0 - Start Manual ZQ
	writel(0x0E447304, 0x307900C0);
	writel(0x0E447306, 0x307900C0);
	while ((readl(0x307900c4) & 0x1) != 0x1);
	writel(0x0E447304, 0x307900C0);
	writel(0x0E407304, 0x307900C0); 	// DDR_PHY_ZQ_CON0 - End Manual ZQ

	writel(0x00000000, 0x30384130); 	//Disable Clock
	writel(0x00000178, 0x30340020); 	// IOMUX_GRP_GRP8 - Start input to PHY
	writel(0x00000002, 0x30384130); 	//Enable Clock
	//	<= NOTE: Depending on JTAG device used, may need ~ 250 us pause at this point.
	writel(0x0000000f, 0x30790018);
	while ((readl(0x307a0004) & 0x1) != 0x1);
}

static void spl_dram_init(void)
{
	if(!gf_strcmp(egf_sw_id_code,REV_WID0547_AA0101))
		ddr3_wid0571aa0101_init();

}

static void gpr_init(void)
{
	struct iomuxc_gpr_base_regs *gpr_regs =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;
	writel(0x4F400005, &gpr_regs->gpr[1]);
}

void board_init_f(ulong dummy)
{
	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	gpr_init();

	/* iomux */
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

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
}

#endif


