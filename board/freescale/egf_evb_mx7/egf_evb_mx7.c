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
#include <malloc.h>
#include <i2c.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/arch/crm_regs.h>
#include <usb.h>
#include <usb/ehci-fsl.h>
#include "gf_eeprom.h"
#include "gf_eeprom_port.h"
#include "gf_mux.h"
#include <asm/imx-common/video.h>

#ifdef CONFIG_FSL_FASTBOOT
#include <fsl_fastboot.h>
#ifdef CONFIG_ANDROID_RECOVERY
#include <recovery.h>
#endif
#endif /*CONFIG_FSL_FASTBOOT*/

DECLARE_GLOBAL_DATA_PTR;

/* SW REVISIONS*/
#define WID_LENGTH					15
#define EGF_FDT_FILE_NAME_LENGTH	(13 + WID_LENGTH + 1)
#define REV_WID0575_AA0101 "WID0575_AA01.01"
#define REV_WID0575_AB0101 "WID0575_AB01.01"
#define REV_WID0575_AA0102 "WID0575_AA01.02"

static char *  __attribute__((section (".data"))) egf_sw_id_code;

static int gf_strcmp(const char * cs, const char * ct) {
	register signed char __res;

	while (1) {
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
	}

	return __res;
}

void egf_mux_init(void)
{
	if(!gf_strcmp(egf_sw_id_code,REV_WID0575_AA0101))
	{
		/* SW Revision is WID0575_AA01.01 */
		printf("GF Software ID Code: REV_WID0575_AA01.01\n");
		egf_mux_init_wid0575_aa0101();
	}
	else if(!gf_strcmp(egf_sw_id_code,REV_WID0575_AA0102))
	{
		/* SW Revision is WID0575_AA01.02 */
		printf("GF Software ID Code: REV_WID0575_AA01.02\n");
		egf_mux_init_wid0575_aa0101();
	}
	else if (!gf_strcmp(egf_sw_id_code,REV_WID0575_AB0101))
	{
		/* SW Revision is WID0575_AB01.01 */
		printf("GF Software ID Code: REV_WID0575_AB01.01\n");
		egf_mux_init_wid0575_ab0101();
	}
	else {
		printf("Unrecognized EGF SW ID Code: %s\n",egf_sw_id_code);
		printf("System Hang.\n");
		while(1);
	}
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

	egf_mux_init();

	return 0;
}


int dram_init(void)
{
#ifdef CONFIG_WID
	egf_sw_id_code = (char *) malloc(sizeof(CONFIG_WID));
	gf_strcpy(egf_sw_id_code, CONFIG_WID);
	egf_mux_init();
#else
	printf("Reading EEPROM for DRAM configuration...\n");
	/* Carica EEPROM */
	load_revision();
#endif
	printf("EGF SW ID: %s", egf_sw_id_code);
	if(!gf_strcmp(egf_sw_id_code,REV_WID0575_AA0101) ||
	   !gf_strcmp(egf_sw_id_code,REV_WID0575_AB0101) ||
	   !gf_strcmp(egf_sw_id_code,REV_WID0575_AA0102))
		gd->ram_size = SZ_2G;
	else
		gd->ram_size = PHYS_SDRAM_SIZE;
	printf("\nDRAM:  ");
	return 0;
}

void prepare_boot_env(void)
{
	char * egf_sw_id_code;
	char* mac_address;
	char fdt_file_name[EGF_FDT_FILE_NAME_LENGTH];

	egf_sw_id_code = gf_eeprom_get_som_sw_id_code();
	fdt_file_name[0] = 0;
	gf_strcat(fdt_file_name, "imx7d-egf-");
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

#ifdef CONFIG_NAND_MXS
static void setup_gpmi_nand(void)
{
	/* NAND_USDHC_BUS_CLK is set in rom */
	set_clk_nand();
}
#endif

#ifdef CONFIG_VIDEO_MXS
struct display_info_t const displays[] = {

};

size_t display_count = ARRAY_SIZE(displays);
#endif

#ifdef CONFIG_FSL_ESDHC

static struct fsl_esdhc_cfg usdhc_cfg[2] = {
	{USDHC1_BASE_ADDR, 0, 4},
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
	case USDHC1_BASE_ADDR:
		ret = !gpio_get_value(USDHC1_CD_GPIO);
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
	 * mmc0                    USDHC1 - micro SD on 0574
	 * mmc1                    USDHC3 - eMMC on 0575
	 */
	for (i = 0; i < CONFIG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			gpio_request(USDHC1_CD_GPIO, "usdhc1_cd");
			gpio_direction_input(USDHC1_CD_GPIO);
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
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


#ifdef CONFIG_SYS_USE_SPINOR
int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	return (bus == 1 && cs == 0) ? CONFIG_SF_DEFAULT_CS_GPIO : -1;
}
#endif

#ifdef CONFIG_FEC_MXC

int board_eth_init(bd_t *bis)
{
	int ret;

	/* Reset AR8035 PHY 1*/
	gpio_direction_output(ENET1_NRST_GPIO,0);
	udelay (500);
	gpio_set_value(ENET1_NRST_GPIO, 1);
	udelay (500);

	/* Reset AR8035 PHY 2*/
	gpio_direction_output(ENET2_NRST_GPIO,0);
	udelay (500);
	gpio_set_value(ENET2_NRST_GPIO, 1);
	udelay (500);


	ret = fecmxc_initialize_multi(bis, CONFIG_FEC_ENET_DEV,
		CONFIG_FEC_MXC_PHYADDR, IMX_FEC_BASE);
	if (ret)
		printf("FEC MXC: %s:failed %d\n", __func__, ret);


	return ret;
}

static int setup_fec(int fec_id)
{
	struct iomuxc_gpr_base_regs *const iomuxc_gpr_regs
		= (struct iomuxc_gpr_base_regs *) IOMUXC_GPR_BASE_ADDR;

	if (0 == fec_id) {
		/* Use 125M anatop REF_CLK1 for ENET1, clear gpr1[13], gpr1[17]*/
		clrsetbits_le32(&iomuxc_gpr_regs->gpr[1],
			(IOMUXC_GPR_GPR1_GPR_ENET1_TX_CLK_SEL_MASK |
			 IOMUXC_GPR_GPR1_GPR_ENET1_CLK_DIR_MASK), 0);
	} else {
		/* Use 125M anatop REF_CLK2 for ENET2, clear gpr1[14], gpr1[18]*/
		clrsetbits_le32(&iomuxc_gpr_regs->gpr[1],
			(IOMUXC_GPR_GPR1_GPR_ENET2_TX_CLK_SEL_MASK |
			 IOMUXC_GPR_GPR1_GPR_ENET2_CLK_DIR_MASK), 0);
	}

	return set_clk_enet(ENET_125MHz);

}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);
	return 0;
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
	egf_common_mux_init();
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_FEC_MXC
	setup_fec(CONFIG_FEC_ENET_DEV);
#endif

#ifdef CONFIG_NAND_MXS
	setup_gpmi_nand();
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
#define I2C_PMIC	1
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

	if (!is_boot_from_usb())
		prepare_boot_env();

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
void ddr3_2g_smr4712_init(void)
{
	writel(0x00000002, 0x30391000); 	// deassert presetn
	writel(0x03040001, 0x307A0000); 	// DDRC_MSTR
	writel(0x0020005E, 0x307A0064);		// DDRC_RFSHTMG
	writel(0x00000001, 0x307a0490);		// DDRC_PCTRL_0
	writel(0x00690000, 0x307A00D4); 	// DDRC_INIT1 (if using LPDDR3/LPDDR2, this line is automatically commented out)
	writel(0x00020083, 0x307A00D0);		// DDRC_INIT0
	writel(0x09300006, 0x307A00DC); 	// DDRC_INIT3
	writel(0x04880000, 0x307A00E0); 	// DDRC_INIT4
	writel(0x00100004, 0x307A00E4); 	// DDRC_INIT5
	writel(0x0000033F, 0x307A00F4); 	// DDRC_RANKCTL
	writel(0x090E080A, 0x307A0100); 	// DDRC_DRAMTMG0
	writel(0x0007020E, 0x307A0104); 	// DDRC_DRAMTMG1
	writel(0x03040407, 0x307A0108); 	// DDRC_DRAMTMG2
	writel(0x00002006, 0x307A010C); 	// DDRC_DRAMTMG3
	writel(0x04020304, 0x307A0110); 	// DDRC_DRAMTMG4
	writel(0x03030202, 0x307A0114); 	// DDRC_DRAMTMG5
	writel(0x00000803, 0x307A0120); 	// DDRC_DRAMTMG8
	writel(0x00800020, 0x307A0180); 	// DDRC_ZQCTL0
	writel(0x02098204, 0x307A0190); 	// DDRC_DFITMG0
	writel(0x00030303, 0x307A0194); 	// DDRC_DFITMG1
	writel(0x80400003, 0x307A01A0); 	// DDRC_DFIUPD0
	writel(0x00100020, 0x307A01A4); 	// DDRC_DFIUPD1
	writel(0x80100004, 0x307A01A8); 	// DDRC_DFIUPD2
	writel(0x00000016, 0x307A0200); 	// DDRC_ADDRMAP0
	writel(0x00171717, 0x307A0204); 	// DDRC_ADDRMAP1
	writel(0x00000000, 0x307A020C); 	// DDRC_ADDRMAP3
	writel(0x00000F0F, 0x307A0210); 	// DDRC_ADDRMAP4
	writel(0x04040404, 0x307A0214); 	// DDRC_ADDRMAP5
	writel(0x0F040404, 0x307A0218); 	// DDRC_ADDRMAP6
	writel(0x06000604, 0x307A0240); 	// DDRC_ODTCFG
	writel(0x00000201, 0x307A0244); 	// DDRC_ODTMAP

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

/* 2GiB DDR3 SMR4722 x 2 chip */
void ddr3_2g_smr4722_init(void)
{
	writel(0x00000002, 0x30391000); 	// deassert presetn
	writel(0x01040001, 0x307A0000); 	// DDRC_MSTR
	writel(0x00200050, 0x307A0064);		// DDRC_RFSHTMG
	writel(0x00000001, 0x307a0490);		// DDRC_PCTRL_0
	writel(0x00690000, 0x307A00D4); 	// DDRC_INIT1 (if using LPDDR3/LPDDR2, this line is automatically commented out)
	writel(0x00020083, 0x307A00D0);		// DDRC_INIT0
	writel(0x09300004, 0x307A00DC); 	// DDRC_INIT3
	writel(0x04880000, 0x307A00E0); 	// DDRC_INIT4
	writel(0x00100004, 0x307A00E4); 	// DDRC_INIT5
	writel(0x0000033F, 0x307A00F4); 	// DDRC_RANKCTL
	writel(0x09080809, 0x307A0100); 	// DDRC_DRAMTMG0
	writel(0x0007020D, 0x307A0104); 	// DDRC_DRAMTMG1
	writel(0x03040407, 0x307A0108); 	// DDRC_DRAMTMG2
	writel(0x00002006, 0x307A010C); 	// DDRC_DRAMTMG3
	writel(0x04020205, 0x307A0110); 	// DDRC_DRAMTMG4
	writel(0x03030202, 0x307A0114); 	// DDRC_DRAMTMG5
	writel(0x00000803, 0x307A0120); 	// DDRC_DRAMTMG8
	writel(0x00800020, 0x307A0180); 	// DDRC_ZQCTL0
	writel(0x02098204, 0x307A0190); 	// DDRC_DFITMG0
	writel(0x00030303, 0x307A0194); 	// DDRC_DFITMG1
	writel(0x80400003, 0x307A01A0); 	// DDRC_DFIUPD0
	writel(0x00100020, 0x307A01A4); 	// DDRC_DFIUPD1
	writel(0x80100004, 0x307A01A8); 	// DDRC_DFIUPD2
	writel(0x0000001F, 0x307A0200); 	// DDRC_ADDRMAP0
	writel(0x00080808, 0x307A0204); 	// DDRC_ADDRMAP1
	writel(0x00000000, 0x307A020C); 	// DDRC_ADDRMAP3
	writel(0x00000F0F, 0x307A0210); 	// DDRC_ADDRMAP4
	writel(0x07070707, 0x307A0214); 	// DDRC_ADDRMAP5
	writel(0x07070707, 0x307A0218); 	// DDRC_ADDRMAP6
	writel(0x06000604, 0x307A0240); 	// DDRC_ODTCFG
	writel(0x00000001, 0x307A0244); 	// DDRC_ODTMAP

	writel(0x00000000, 0x30391000); 	// deassert presetn
	writel(0x17420F40, 0x30790000); 	// DDR_PHY_PHY_CON0
	writel(0x10210100, 0x30790004); 	// DDR_PHY_PHY_CON1
	writel(0x00060807, 0x30790010); 	// DDR_PHY_PHY_CON4
	writel(0x1010007E, 0x307900B0); 	// DDR_PHY_MDLL_CON0
	writel(0x00000D6E, 0x3079009C); 	// DDR_PHY_DRVDS_CON0
	writel(0x04040404, 0x30790030); 	// DDR_PHY_OFFSET_WR_CON0
	writel(0x0A0A0A0A, 0x30790020); 	// DDR_PHY_OFFSET_RD_CON0
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
	if(!gf_strcmp(egf_sw_id_code,REV_WID0575_AA0101) || !gf_strcmp(egf_sw_id_code,REV_WID0575_AB0101))
		ddr3_2g_smr4712_init();
	else if (!gf_strcmp(egf_sw_id_code,REV_WID0575_AA0102))
		ddr3_2g_smr4722_init();
	else
		printf("Error: WID not supported!\n");

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


