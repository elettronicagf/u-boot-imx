/*
 * Copyright (C) 2015-2016 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/clock.h>
#include <asm/arch/iomux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/io.h>
#include <common.h>
#include <fsl_esdhc.h>
#include <i2c.h>
#include <miiphy.h>
#include <linux/sizes.h>
#include <mmc.h>
#include <mxsfb.h>
#include <netdev.h>
#include <power/pmic.h>
#include <power/pfuze3000_pmic.h>
#include "../../freescale/common/pfuze.h"
#include <usb.h>
#include <usb/ehci-ci.h>
#include <asm/mach-imx/video.h>
#include "gf_mux.h"
#include "gf_eeprom.h"
#include "gf_eeprom_port.h"
#ifdef CONFIG_SPL_BUILD
#include "gf_ddr_parameters.h"
#endif
#ifdef CONFIG_FSL_FASTBOOT
#include <fsl_fastboot.h>
#ifdef CONFIG_ANDROID_RECOVERY
#include <recovery.h>
#endif
#endif /*CONFIG_FSL_FASTBOOT*/
#include "../drivers/video/mxcfb.h"

DECLARE_GLOBAL_DATA_PTR;

#define WID_LENGTH					15
#define EGF_FDT_FILE_NAME_LENGTH	(13 + WID_LENGTH + 1)

/* SW REVISIONS*/
#define REV_WID0659_AA0101 "WID0659_AA01.01"

#ifdef CONFIG_SYS_I2C_MXC
#ifdef CONFIG_POWER
#define I2C_PMIC       0

static int gf_strcmp(const char * cs, const char * ct) {
	register signed char __res;

	while (1) {
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
	}

	return __res;
}

int power_init_board(void)
{
	struct pmic *pfuze;
	int ret;
	unsigned int reg, rev_id;

	ret = power_pfuze3000_init(I2C_PMIC);
	if (ret)
		return ret;

	pfuze = pmic_get("PFUZE3000");
	ret = pmic_probe(pfuze);
	if (ret)
		return ret;

	pmic_reg_read(pfuze, PFUZE3000_DEVICEID, &reg);
	pmic_reg_read(pfuze, PFUZE3000_REVID, &rev_id);
	printf("PMIC: PFUZE3000 DEV_ID=0x%x REV_ID=0x%x\n",
		   reg, rev_id);

	/* disable Low Power Mode during standby mode */
	pmic_reg_read(pfuze, PFUZE3000_LDOGCTL, &reg);
	reg |= 0x1;
	pmic_reg_write(pfuze, PFUZE3000_LDOGCTL, reg);

	/* SW1B step ramp up time from 2us to 4us/25mV */
	reg = 0x40;
	pmic_reg_write(pfuze, PFUZE3000_SW1BCONF, reg);

	/* SW1B mode to APS/PFM */
	reg = 0xc;
	pmic_reg_write(pfuze, PFUZE3000_SW1BMODE, reg);

	/* SW1B standby voltage set to 0.975V */
	reg = 0xb;
	pmic_reg_write(pfuze, PFUZE3000_SW1BSTBY, reg);

	return 0;
}

#ifdef CONFIG_LDO_BYPASS_CHECK
void ldo_mode_set(int ldo_bypass)
{
	unsigned int value;
	u32 vddarm;

	struct pmic *p = pmic_get("PFUZE3000");

	if (!p) {
		printf("No PMIC found!\n");
		return;
	}

	/* switch to ldo_bypass mode */
	if (ldo_bypass) {
		prep_anatop_bypass();
		/* decrease VDDARM to 1.275V */
		pmic_reg_read(p, PFUZE3000_SW1BVOLT, &value);
		value &= ~0x1f;
		value |= PFUZE3000_SW1AB_SETP(1275);
		pmic_reg_write(p, PFUZE3000_SW1BVOLT, value);

		set_anatop_bypass(1);
		vddarm = PFUZE3000_SW1AB_SETP(1175);

		pmic_reg_read(p, PFUZE3000_SW1BVOLT, &value);
		value &= ~0x1f;
		value |= vddarm;
		pmic_reg_write(p, PFUZE3000_SW1BVOLT, value);

		finish_anatop_bypass();

		printf("switch to ldo_bypass mode!\n");
	}
}
#endif
#endif
#endif

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();

	return 0;
}

void prepare_boot_env(void)
{
	char * egf_sw_id_code;
	char * som_serial_number;
	char* mac_address;
	char fdt_file_name[EGF_FDT_FILE_NAME_LENGTH];
	u32 cpurev;

	egf_sw_id_code = gf_eeprom_get_som_sw_id_code();
	som_serial_number = gf_eeprom_get_som_serial_number();
	fdt_file_name[0] = 0;
	gf_strcat(fdt_file_name, "egf-sbc-");
	gf_strcat(fdt_file_name, egf_sw_id_code);
	gf_strcat(fdt_file_name, ".dtb");

	env_set("fdt_file",fdt_file_name);
	env_set("wid", egf_sw_id_code);
	env_set("sn", som_serial_number);

	cpurev = get_cpu_rev();
	env_set("imx_cpu", get_imx_type((cpurev & 0xFF000) >> 12));

	mac_address = gf_eeprom_get_mac1_address();
	if (mac_address == NULL)
			printf("MAC Address not programmed.\n");
	else
	{
		env_set("ethaddr",mac_address);
	}
}

#ifdef CONFIG_USB_EHCI_MX6
#define USB_OTHERREGS_OFFSET	0x800
#define UCTRL_PWR_POL		(1 << 9)

int board_usb_phy_mode(int port)
{
	if (port == 0)
		return USB_INIT_DEVICE;
	else
		return USB_INIT_HOST; //usb_phy_mode(port);
}

int board_ehci_hcd_init(int port)
{
	u32 *usbnc_usb_ctrl;

	if (port > 1)
		return -EINVAL;

	usbnc_usb_ctrl = (u32 *)(USB_BASE_ADDR + USB_OTHERREGS_OFFSET +
				 port * 4);

	/* Set Power polarity */
	setbits_le32(usbnc_usb_ctrl, UCTRL_PWR_POL);

	return 0;
}

int board_ehci_power(int port, int on)
{
	switch (port) {
	case 0:
		//OTG port, disabled in u-boot
//		if (on) {
//			gpio_direction_output(USB_OTG1_PWR_EN_GPIO,1);
//			mdelay(100);
//		} else
//			gpio_direction_output(USB_OTG1_PWR_EN_GPIO,0);
		return -EINVAL;
		break;
	case 1:
		if (on) {
			gpio_direction_output(USB_OTG2_PWR_EN_GPIO,1);
			mdelay(100);
		} else
			gpio_direction_output(USB_OTG2_PWR_EN_GPIO,0);
		break;
	default:
		printf("MXC USB port %d not yet supported\n", port);
		return -EINVAL;
	}

	return 0;
}

#endif

#ifdef CONFIG_NAND_MXS

static void setup_gpmi_nand(void)
{
	struct mxc_ccm_reg *mxc_ccm = (struct mxc_ccm_reg *)CCM_BASE_ADDR;

	setup_gpmi_io_clk((3 << MXC_CCM_CSCDR1_BCH_PODF_OFFSET) |
			  (3 << MXC_CCM_CSCDR1_GPMI_PODF_OFFSET));

	/* enable apbh clock gating */
	setbits_le32(&mxc_ccm->CCGR0, MXC_CCM_CCGR0_APBHDMA_MASK);
}
#endif

#ifdef CONFIG_FEC_MXC
int board_eth_init(bd_t *bis)
{
#if 0
	char * egf_sw_id_code;

#ifdef CONFIG_WID
		egf_sw_id_code = CONFIG_WID;
#else
		egf_sw_id_code = gf_eeprom_get_som_sw_id_code();
#endif
#endif

	/* Reset LAN8720 PHY */
	gpio_direction_output(ENET_NRST_GPIO,0);
	udelay (1000);
	gpio_set_value(ENET_NRST_GPIO, 1);
	udelay (500);

	return cpu_eth_init(bis);

}

static int setup_fec(int fec_id)
{
	struct iomuxc *const iomuxc_regs = (struct iomuxc *)IOMUXC_BASE_ADDR;
	int ret;

	if (fec_id == 0) {
		if (check_module_fused(MX6_MODULE_ENET1))
			return -1;

		/*
		 * Use 50M anatop loopback REF_CLK1 for ENET1,
		 * clear gpr1[13], set gpr1[17].
		 */
		clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC1_MASK,
				IOMUX_GPR1_FEC1_CLOCK_MUX1_SEL_MASK);
	} else {
		if (check_module_fused(MX6_MODULE_ENET2))
			return -1;

		/*
		 * Use 50M anatop loopback REF_CLK2 for ENET2,
		 * clear gpr1[14], set gpr1[18].
		 */
		clrsetbits_le32(&iomuxc_regs->gpr[1], IOMUX_GPR1_FEC2_MASK,
				IOMUX_GPR1_FEC2_CLOCK_MUX1_SEL_MASK);
	}

	ret = enable_fec_anatop_clock(fec_id, ENET_50MHZ);
	if (ret)
		return ret;

	enable_enet_clk(1);

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}
#endif

#ifdef CONFIG_FSL_ESDHC
static struct fsl_esdhc_cfg usdhc_cfg[1] = {
	{USDHC1_BASE_ADDR, 0, 4},
};

int mmc_map_to_kernel_blk(int devno)
{
	if (devno == 0 && mx6_esdhc_fused(USDHC1_BASE_ADDR))
		devno = 1;

	return devno;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
		ret = 1;
		break;
	}

	return ret;
}

int board_mmc_init(bd_t *bis)
{
	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-boot device node)    (Physical Port)
	 * mmc0                    USDHC1
	 */
	usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	if (fsl_esdhc_initialize(bis, &usdhc_cfg[0]))
		printf("Warning: failed to initialize mmc dev 0\n");

	return 0;
}
#endif


#ifdef CONFIG_VIDEO_MXS
void do_enable_parallel_lcd(struct display_info_t const *dev)
{
	enable_lcdif_clock(dev->bus, true);

	gpio_direction_output(LCD_POWER_ENABLE, 1);

	/* Set Brightness to high */
	gpio_direction_output(BKL_DISPLAY_PWM_GPIO, 1);
}

struct display_info_t const displays[] = {
{
	.bus = MX6UL_LCDIF1_BASE_ADDR,
	.addr = 0,
	.pixfmt = 24,
	.detect = NULL,
	.enable	= do_enable_parallel_lcd,
	.mode	= {
		.name			= "EGF_BLC1156",
		.xres           = 480,
		.yres           = 272,
		.pixclock       = 111111,
		.left_margin    = 43,
		.right_margin   = 8,
		.upper_margin   = 12,
		.lower_margin   = 8,
		.hsync_len      = 4,
		.vsync_len      = 4,
		.sync           = FB_SYNC_CLK_LAT_FALL,
		.vmode          = FB_VMODE_NONINTERLACED
	}
},
{
	.bus = MX6UL_LCDIF1_BASE_ADDR,
	.addr = 0,
	.pixfmt = 24,
	.detect = NULL,
	.enable	= do_enable_parallel_lcd,
	.mode	= {
		.name			= "EGF_BLC1154",
		.xres           = 480,
		.yres           = 272,
		.pixclock       = 111111,
		.left_margin    = 43,
		.right_margin   = 8,
		.upper_margin   = 12,
		.lower_margin   = 8,
		.hsync_len      = 4,
		.vsync_len      = 4,
		.sync           = FB_SYNC_CLK_LAT_FALL,
		.vmode          = FB_VMODE_NONINTERLACED
	}
},
{
	.bus = MX6UL_LCDIF1_BASE_ADDR,
	.addr = 0,
	.pixfmt = 24,
	.detect = NULL,
	.enable	= do_enable_parallel_lcd,
	.mode	= {
		.name			= "EGF_BLC1155",
		.xres           = 800,
		.yres           = 480,
		.pixclock       = 33333,
		.left_margin    = 40,
		.right_margin   = 88,
		.upper_margin   = 32,
		.lower_margin   = 13,
		.hsync_len      = 48,
		.vsync_len      = 3,
		.sync           = FB_SYNC_CLK_LAT_FALL,
		.vmode          = FB_VMODE_NONINTERLACED
	}
},
{
	.bus = MX6UL_LCDIF1_BASE_ADDR,
	.addr = 0,
	.pixfmt = 24,
	.detect = NULL,
	.enable	= do_enable_parallel_lcd,
	.mode	= {
		.name			= "EGF_BLC1165",
		.xres           = 800,
		.yres           = 480,
		.pixclock       = 33333,
		.left_margin    = 46,
		.right_margin   = 210,
		.upper_margin   = 23,
		.lower_margin   = 22,
		.hsync_len      = 10,
		.vsync_len      = 5,
		.sync           = FB_SYNC_CLK_LAT_FALL,
		.vmode          = FB_VMODE_NONINTERLACED
	}
}
};
size_t display_count = ARRAY_SIZE(displays);

int board_video_skip(void)
{
	char * egf_sw_id_code;
	int i;
	int ret;
	char panel[100];
	char const *panel_env = env_get("panel");

	panel[0] = 0;

	if (!panel_env){
#ifdef CONFIG_WID
		egf_sw_id_code = CONFIG_WID;
#else
		egf_sw_id_code = gf_eeprom_get_som_sw_id_code();
#endif
		//default panel
		strcpy(panel, "EGF_BLC1154");
		env_set("panel", panel);
/*
		if (egf_sw_id_code)
		{
			if (!gf_strcmp(egf_sw_id_code, REV_WID0571_AH0101))
			{
				strcpy(panel, "EGF_BLC1154");
				env_set("panel", panel);
			} else if (!gf_strcmp(egf_sw_id_code, REV_WID0571_AG0101)) {
				strcpy(panel, "EGF_BLC1165");
				env_set("panel", panel);
			} else if (!gf_strcmp(egf_sw_id_code, REV_WID0571_AI0101)) {
				strcpy(panel, "EGF_BLC1165");
				env_set("panel", panel);
			}
		}
*/
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
		ret = mxs_lcd_panel_setup(displays[i].mode,
					displays[i].pixfmt,
				    displays[i].bus);
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
#endif

#ifdef CONFIG_SYS_USE_SPINOR
int board_spi_cs_gpio(unsigned bus, unsigned cs)
{
	if(bus == 1)
		return SPINOR_SS0_GPIO_ULZ;
	else if(bus == 2)
		return SPINOR_SS0_GPIO_ULL;
	else
		return -1;
}
#endif

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
	return 0;
}

int board_init(void)
{
	char * egf_sw_id_code;
	int ret;

	/* Address of boot parameters */
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

#ifdef CONFIG_WID
	egf_sw_id_code = CONFIG_WID;
#else
	egf_sw_id_code = gf_eeprom_get_som_sw_id_code();
#endif

#ifdef	CONFIG_FEC_MXC
	setup_fec(CONFIG_FEC_ENET_DEV);
#endif

#ifdef CONFIG_NAND_MXS
	setup_gpmi_nand();
#endif

	return 0;
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
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

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "0659 SBC");

#endif

//	set_wdog_reset((struct wdog_regs *)WDOG1_BASE_ADDR);

	return 0;
}

int checkboard(void)
{
#ifdef CONFIG_MX6ULL
	puts("Board: 0659 SBC MX6ULL\n");
#elif CONFIG_MX6UL
	puts("Board: 0659 SBC MX6UL\n");
#elif CONFIG_MX6ULZ
	puts("Board: 0659 SBC MX6ULZ\n");
#else
	puts("Board: 0659 SBC Unrecognized Processor\n");
#endif
	return 0;
}

#ifdef CONFIG_FSL_FASTBOOT
void board_fastboot_setup(void)
{
	switch (get_boot_device()) {
#if defined(CONFIG_FASTBOOT_STORAGE_MMC)
	case SD1_BOOT:
	case MMC1_BOOT:
		if (!env_get("fastboot_dev"))
			env_set("fastboot_dev", "mmc0");
		if (!env_get("bootcmd"))
			env_set("bootcmd", "boota mmc0");
		break;
	case SD2_BOOT:
	case MMC2_BOOT:
		if (!env_get("fastboot_dev"))
			env_set("fastboot_dev", "mmc1");
		if (!env_get("bootcmd"))
			env_set("bootcmd", "boota mmc1");
		break;
#endif /*CONFIG_FASTBOOT_STORAGE_MMC*/
#if defined(CONFIG_FASTBOOT_STORAGE_NAND)
	case NAND_BOOT:
		if (!env_get("fastboot_dev"))
			env_set("fastboot_dev", "nand");
		if (!env_get("fbparts"))
			env_set("fbparts", ANDROID_FASTBOOT_NAND_PARTS);
		if (!env_get("bootcmd"))
			env_set("bootcmd",
				"nand read ${loadaddr} ${boot_nand_offset} "
				"${boot_nand_size};boota ${loadaddr}");
		break;
#endif /*CONFIG_FASTBOOT_STORAGE_NAND*/

	default:
		printf("unsupported boot devices\n");
		break;
	}
}
#endif /*CONFIG_FSL_FASTBOOT*/

#ifdef CONFIG_SPL_BUILD
#include <linux/libfdt.h>
#include <spl.h>

#define MT41K128M16JT_125 		1
#define K4B4G1646E_BYK0 		2
#define MT41K64M16TW_107		3
#define MT41K512M16HA_125		4
#define MT41K256M16TW_107		5

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
	void * ddr_sysinfo;
};

static struct egf_som __attribute__((section (".data"))) the_som;

static struct egf_som the_som_WID_0659_AA0101 = {
		K4B4G1646E_BYK0,
		DDR_BUS_WIDTH_16BIT,
		1,
		&mx6ull_ddr_ioregs_standard,
		&mx6_grp_ioregs_standard,
		&mx6ull_mmcd_calib_standard,
		&mx6_ddr_sysinfo_standard,
};

/*
static struct egf_som the_som_WID_0571_AH0101 = {
		MT41K256M16TW_107,
		DDR_BUS_WIDTH_16BIT,
		1,
		&mx6ull_ddr_ioregs_standard,
		&mx6_grp_ioregs_standard,
		&mx6ull_mmcd_calib_standard,
		&mx6_ddr_sysinfo_standard,
};

static struct egf_som the_som_WID_0571_AI0101 = {
		MT41K256M16TW_107,
		DDR_BUS_WIDTH_16BIT,
		1,
		&mx6ull_ddr_ioregs_standard,
		&mx6_grp_ioregs_standard,
		&mx6ull_mmcd_calib_standard,
		&mx6_ddr_sysinfo_standard,
};
*/
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

	if(!gf_strcmp(egf_sw_id_code,REV_WID0659_AA0101))
	{
		/* SW Revision is WID0659_AA01.01 */
		printf("GF Software ID Code: WID0659_AA01.01\n");
		memcpy(&the_som, &the_som_WID_0659_AA0101, sizeof(the_som));
	}
//	else if(!gf_strcmp(egf_sw_id_code,REV_WID0571_AH0101))
//	{
//		/* SW Revision is WID0571_AH01.01 */
//		printf("GF Software ID Code: WID0571_AH01.01\n");
//		memcpy(&the_som, &the_som_WID_0571_AH0101, sizeof(the_som));
//	}
//	else if(!gf_strcmp(egf_sw_id_code,REV_WID0571_AI0101))
//	{
//		/* SW Revision is WID0571_AI01.01 */
//		printf("GF Software ID Code: WID0571_AI01.01\n");
//		memcpy(&the_som, &the_som_WID_0571_AI0101, sizeof(the_som));
//	}
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

	writel(0xFFFFFFFF, &ccm->CCGR0);
	writel(0xFFFFFFFF, &ccm->CCGR1);
	writel(0xFFFFFFFF, &ccm->CCGR2);
	writel(0xFFFFFFFF, &ccm->CCGR3);
	writel(0xFFFFFFFF, &ccm->CCGR4);
	writel(0xFFFFFFFF, &ccm->CCGR5);
	writel(0xFFFFFFFF, &ccm->CCGR6);
	writel(0xFFFFFFFF, &ccm->CCGR7);
}


static void spl_dram_init(void)
{
	struct mx6_ddr3_cfg *memory_timings = NULL;
	struct mx6_mmdc_calibration *memory_calib = NULL;
	struct mx6_ddr_sysinfo *sysinfo = NULL;

	switch(the_som.ram_model)
	{
	case MT41K128M16JT_125:
		memory_timings = &mt41k128m16jt_125;
		break;
	case K4B4G1646E_BYK0:
		memory_timings = &k4b4g1646e_byk0;
		break;
	case MT41K64M16TW_107:
		memory_timings = &mt41k64m16tw_107;
		break;
	case MT41K512M16HA_125:
		memory_timings = &mt41k512m16ha_125;
		break;
	case MT41K256M16TW_107:
		memory_timings = &mt41k256m16tw_107;
		break;
	default:
		puts("Error: Invalid Memory Configuration\n");
		hang();
	}
	memory_calib = the_som.mmdc_calibration;
	sysinfo = the_som.ddr_sysinfo;

	if (!memory_timings) {
		puts("Error: Invalid Memory Configuration\n");
		hang();
	}
	if (!memory_calib) {
		puts("Error: Invalid Board Calibration Configuration\n");
		hang();
	}

	if (!sysinfo) {
		puts("Error: Invalid Board DDR sysinfo Configuration\n");
		hang();
	}

	mx6ul_dram_iocfg(the_som.ram_bus_width, (struct mx6ul_iomux_ddr_regs *)the_som.iomux_ddr_regs,
			 (struct mx6ul_iomux_grp_regs *)the_som.iomux_grp_regs);

	mx6_dram_cfg(sysinfo, memory_calib, memory_timings);

}

void board_init_f(ulong dummy)
{
	/* setup AIPS and disable watchdog */
	arch_cpu_init();

	ccgr_init();

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

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	/* load/boot image from boot device */
	board_init_r(NULL, 0);
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


