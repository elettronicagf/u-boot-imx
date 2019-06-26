/*
 * Copyright (C) 2012-2015 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale i.MX6Q SabreSD board.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __MX6EGFEVB_CONFIG_H
#define __MX6EGFEVB_CONFIG_H

#include <asm/arch/imx-regs.h>
#include <asm/imx-common/gpio.h>

#ifdef CONFIG_SPL
#define CONFIG_SPL_LIBCOMMON_SUPPORT
#define CONFIG_SPL_MMC_SUPPORT
#include "imx6_spl.h"
#endif

#define CONFIG_MACH_TYPE	3980
#define GF_EEPROM_SERIAL_SEL
#define CONFIG_MMCROOT			"/dev/mmcblk2p2"  /* SDHC3 */

#define EGF_EVB_MX6

#define CONFIG_CMD_INIT_EEPROM

#define CONFIG_MX6

#ifdef CONFIG_MX6SOLO
#define CONFIG_MX6DL
#endif

/* uncomment for PLUGIN mode support */
/* #define CONFIG_USE_PLUGIN */

/* uncomment for SECURE mode support */
/* #define CONFIG_SECURE_BOOT */

#ifdef CONFIG_SECURE_BOOT
#ifndef CONFIG_CSF_SIZE
#define CONFIG_CSF_SIZE 0x4000
#endif
#endif

#include "mx6_common.h"
#include <linux/sizes.h>

#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

#include <asm/arch/imx-regs.h>
#include <asm/imx-common/gpio.h>

#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_INITRD_TAG
#define CONFIG_REVISION_TAG

#define CONFIG_SYS_GENERIC_BOARD

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(16 * SZ_1M)

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_BOARD_LATE_INIT
#define CONFIG_MXC_GPIO

/* disable console output */
#define CONFIG_SILENT_CONSOLE
#define CONFIG_SYS_DEVICE_NULLDEV
#define CONFIG_SILENT_CONSOLE_UPDATE_ON_SET
#define CONFIG_DISABLE_CONSOLE

#define CONFIG_MXC_UART

#define CONFIG_CMD_GPIO

#define CONFIG_CMD_FUSE
#ifdef CONFIG_CMD_FUSE
#define CONFIG_MXC_OCOTP
#endif

/* MMC Configs */
#define CONFIG_FSL_ESDHC
#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR      0

#define CONFIG_MMC
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_BOUNCE_BUFFER
#define CONFIG_CMD_EXT2
#define CONFIG_CMD_EXT4
#define CONFIG_CMD_EXT4_WRITE
#define CONFIG_CMD_FAT
#define CONFIG_DOS_PARTITION

#define CONFIG_SUPPORT_EMMC_BOOT /* eMMC specific */

#define CONFIG_CMD_PING
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#define CONFIG_FEC_MXC
#define CONFIG_MII
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_XCV_TYPE		RGMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_FEC_MXC_PHYADDR		0

#define CONFIG_PHYLIB
#define CONFIG_PHY_ATHEROS

/* allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_CONS_INDEX              1
#define CONFIG_BAUDRATE                        115200

/* Command definition */
#include <config_cmd_default.h>

#define CONFIG_CMD_BMODE
#define CONFIG_CMD_BOOTZ
#define CONFIG_CMD_SETEXPR
#undef CONFIG_CMD_IMLS

#define CONFIG_BOOTDELAY               1

#define CONFIG_LOADADDR                        0x12000000
#define CONFIG_SYS_TEXT_BASE           0x17800000
#define CONFIG_SYS_MMC_IMG_LOAD_PART	1

#if defined CONFIG_SYS_BOOT_SPINOR
#define CONFIG_SYS_USE_SPINOR
#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_SPL_SPI_SUPPORT
#define CONFIG_SPL_SPI_FLASH_SUPPORT
#define CONFIG_SPL_SPI_LOAD
#define CONFIG_SPL_SPI_BUS		CONFIG_SF_DEFAULT_BUS
#define CONFIG_SPL_SPI_CS		CONFIG_SF_DEFAULT_CS
#define CONFIG_SYS_SPI_U_BOOT_OFFS	0x40000
#define CONFIG_SYS_SPI_SPL_OFFS		0x400
#else
#error "Boot possibile solo da NOR Flash SPI"
#endif

#ifdef CONFIG_SYS_BOOT_NAND
#define CONFIG_MFG_NAND_PARTITION "mtdparts=gpmi-nand:64m(boot),16m(kernel),16m(dtb),-(rootfs) "
#else
#define CONFIG_MFG_NAND_PARTITION ""
#endif

#define CONFIG_DISP1_BKL_PWM_GPIO		9
#define CONFIG_DISP1_BKL_PWR_EN_GPIO	78
#define CONFIG_DISP0_BKL_PWM_GPIO		42
#define CONFIG_DISP0_BKL_PWR_EN_GPIO	72

#ifdef CONFIG_WID
#define CONFIG_MFG_ENV_SETTINGS \
		"g_mass_storage.stall=0 g_mass_storage.removable=1 " \
		"g_mass_storage.file=/fat g_mass_storage.ro=1 " \
		"g_mass_storage.idVendor=0x066F g_mass_storage.idProduct=0x37FF "\
		"g_mass_storage.iSerialNumber=\"\" \0" \
		"destroyenv=sf probe; gpio set " __stringify(CONFIG_SF_WPn_GPIO) ";" \
				"sf unlock; sf erase 0x3F0000 0x10000;sf lock;" \
				"gpio clear " __stringify(CONFIG_SF_WPn_GPIO) ";" \
				"env default -f -a;\0" \
		"spl_copy_addr=0x12000000\0" \
		"uboot_img_copy_addr=0x12500000\0" \
		"bootcmd_mfg=sf probe;" \
		"gpio set " __stringify(CONFIG_SF_WPn_GPIO) ";" \
		"sf unlock;" \
		"sf erase 0x0 0x3F0000;" \
		"sf write ${spl_copy_addr} " __stringify(CONFIG_SYS_SPI_SPL_OFFS) " 0x20000;" \
		"mw.l 11fffff8 0100fbfa 1;" \
		"if cmp.b 11fffffc 11fffff8 4; then " \
			"init_eeprom " CONFIG_WID ";" \
		"fi;" \
		"gpio clear " __stringify(CONFIG_SF_WPn_GPIO) ";" \
		"sf write ${uboot_img_copy_addr} " __stringify(CONFIG_SYS_SPI_U_BOOT_OFFS) " 0x100000;" \
		"sf lock;" \
		"gpio clear " __stringify(CONFIG_SF_WPn_GPIO) ";" \
		"gpio clear " __stringify(CONFIG_DISP1_BKL_PWM_GPIO) ";" \
		"gpio clear " __stringify(CONFIG_DISP1_BKL_PWR_EN_GPIO) ";" \
		"gpio clear " __stringify(CONFIG_DISP0_BKL_PWM_GPIO) ";" \
		"gpio clear " __stringify(CONFIG_DISP0_BKL_PWR_EN_GPIO) ";\0 "
#else
#define CONFIG_MFG_ENV_SETTINGS ""
#endif

#ifdef CONFIG_SUPPORT_EMMC_BOOT
#define EMMC_ENV \
	"emmcdev=2\0" \
	"update_emmc_firmware=" \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"if ${get_cmd} ${update_sd_firmware_filename}; then " \
			"if mmc dev ${emmcdev} 1; then "	\
				"setexpr fw_sz ${filesize} / 0x200; " \
				"setexpr fw_sz ${fw_sz} + 1; "	\
				"mmc write ${loadaddr} 0x2 ${fw_sz}; " \
			"fi; "	\
		"fi\0"
#else
#define EMMC_ENV ""
#endif

#if defined(CONFIG_SYS_BOOT_NAND)
	/*
	 * The dts also enables the WEIN NOR which is mtd0.
	 * So the partions' layout for NAND is:
	 *     mtd1: 16M      (uboot)
	 *     mtd2: 16M      (kernel)
	 *     mtd3: 16M      (dtb)
	 *     mtd4: left     (rootfs)
	 */
#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_MFG_ENV_SETTINGS \
	"fdt_addr=0x18000000\0" \
	"fdt_high=0xffffffff\0"	  \
	"bootargs=console=" CONFIG_CONSOLE_DEV ",115200 ubi.mtd=4 "  \
		"root=ubi0:rootfs rootfstype=ubifs "		     \
		"mtdparts=gpmi-nand:64m(boot),16m(kernel),16m(dtb),-(rootfs)\0"\
	"bootcmd=nand read ${loadaddr} 0x4000000 0x800000;"\
		"nand read ${fdt_addr} 0x5000000 0x100000;"\
		"bootz ${loadaddr} - ${fdt_addr}\0"

#elif defined(CONFIG_SYS_BOOT_SATA)

#define CONFIG_EXTRA_ENV_SETTINGS \
		CONFIG_MFG_ENV_SETTINGS \
		"fdt_addr=0x18000000\0" \
		"fdt_high=0xffffffff\0"   \
		"bootargs=console=" CONFIG_CONSOLE_DEV ",115200 \0"\
		"bootargs_sata=setenv bootargs ${bootargs} " \
			"root=/dev/sda1 rootwait rw \0" \
		"bootcmd_sata=run bootargs_sata; sata init; " \
			"sata read ${loadaddr} 0x800  0x4000; " \
			"sata read ${fdt_addr} 0x8000 0x800; " \
			"bootz ${loadaddr} - ${fdt_addr} \0" \
		"bootcmd=run bootcmd_sata \0"

#else
#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_MFG_ENV_SETTINGS \
	"locknor=sf probe;gpio set " __stringify(CONFIG_SF_WPn_GPIO) ";sf lock;gpio clear " __stringify(CONFIG_SF_WPn_GPIO) ";\0" \
	"unlocknor=sf probe;gpio set " __stringify(CONFIG_SF_WPn_GPIO) ";sf unlock; ;\0" \
	"script=boot.scr\0" \
	"image=zImage\0" \
	"fdt_addr=0x18000000\0" \
	"boot_fdt=try\0" \
	"ip_dyn=yes\0" \
	"g_ether_args=g_cdc.dev_addr=58:05:56:00:04:5e g_cdc.host_addr=58:05:56:00:04:5d\0"\
	"fdt_high=0xffffffff\0"	  \
	"bootargs=console=${console},${baudrate} ${smp} ${g_ether_args}\0" \
	"initrd_high=0xffffffff\0" \
	"audio=1\0 " \
	"mmcautodetect=yes\0" \
	"destroyenv=sf probe; gpio set " __stringify(CONFIG_SF_WPn_GPIO) ";" \
			"sf unlock; sf erase 0x3F0000 0x10000;sf lock;" \
			"gpio clear " __stringify(CONFIG_SF_WPn_GPIO) ";" \
			"env default -f -a;\0" \
	"fix_dt=fdt addr ${fdt_addr}; " \
			"if test \"${pcb_rev}\" = \"PGF0533_A01\"; then " \
				"fdt rm rtc_pcf85063a; " \
				"fdt set wdog1 status \"okay\"; " \
				"fdt set wdog2 status \"disabled\"; " \
			"elif test \"${pcb_rev}\" = \"PGF0533_A02\"; then " \
				"fdt rm rtc_mcp7941x; " \
				"fdt rm rtc_mcp7941x_eeprom; " \
			"fi; " \
			"if test \"${audio}\" = \"0\"; then " \
				"fdt rm sound; " \
			"fi; " \
			"if test \"${panel}\" = \"EGF_BLC1134\"; then " \
				"fdt rm EGF_BLC1133; " \
				"fdt rm EGF_BLC1168; " \
				"fdt rm EGF_BLC1173; " \
				"fdt set lvds_channel1 status \"okay\"; " \
				"fdt rm ft5x06; " \
				"fdt rm ar1020; " \
				"fdt rm tsc2046; " \
				"if test \"${pcb_rev}\" = \"PGF0533_A01\"; then " \
					"fdt set backlight3 status \"okay\"; " \
					"fdt set panel3 status \"okay\"; " \
				"else " \
					"fdt set backlight3 status \"okay\"; " \
					"fdt set panel4 status \"okay\"; " \
				"fi; " \
			"elif test \"${panel}\" = \"EGF_BLC1133\"; then " \
				"fdt rm EGF_BLC1134; " \
				"fdt rm EGF_BLC1168; " \
				"fdt rm EGF_BLC1173; " \
				"fdt rm EGF_BLC1167; " \
				"fdt set lvds_channel1 status \"okay\"; " \
				"fdt rm ar1020; " \
				"fdt rm tsc2046; " \
				"fdt set backlight2 status \"okay\"; " \
				"fdt set panel2 status \"okay\"; " \
			"elif test \"${panel}\" = \"EGF_BLC1093\"; then " \
				"fdt rm EGF_BLC1134; " \
				"fdt rm EGF_BLC1133; " \
				"fdt rm EGF_BLC1168; " \
				"fdt rm EGF_BLC1173; " \
				"fdt rm EGF_BLC1167; " \
				"fdt rm ft5x06; " \
				"fdt rm tsc2046; " \
				"fdt set mxcfb0 disp_dev \"lcd\"; " \
				"fdt set mxcfb0 mode_str \"EGF_BLC1093\"; " \
				"fdt set backlight1 status \"okay\"; " \
				"fdt set panel1 status \"okay\"; " \
			"elif test \"${panel}\" = \"EGF_BLC1113\"; then " \
				"fdt rm EGF_BLC1134; " \
				"fdt rm EGF_BLC1133; " \
				"fdt rm EGF_BLC1168; " \
				"fdt rm EGF_BLC1173; " \
				"fdt rm EGF_BLC1167; " \
				"fdt rm ft5x06; " \
				"fdt rm ar1020; " \
				"fdt rm tsc2046; " \
				"fdt set mxcfb0 disp_dev \"lcd\"; " \
				"fdt set mxcfb0 mode_str \"EGF_BLC1113\"; " \
				"fdt set backlight1 status \"okay\"; " \
				"fdt set panel1 status \"okay\"; " \
			"elif test \"${panel}\" = \"EGF_BLC1081\"; then " \
				"fdt rm EGF_BLC1134; " \
				"fdt rm EGF_BLC1133; " \
				"fdt rm EGF_BLC1168; " \
				"fdt rm EGF_BLC1173; " \
				"fdt rm EGF_BLC1167; " \
				"fdt rm ft5x06; " \
				"fdt rm tsc2046; " \
				"fdt set mxcfb0 disp_dev \"lcd\"; " \
				"fdt set mxcfb0 mode_str \"EGF_BLC1081\"; " \
				"fdt set backlight1 status \"okay\"; " \
				"fdt set panel1 status \"okay\"; " \
			"elif test \"${panel}\" = \"EGF_BLC1168\"; then " \
				"fdt rm EGF_BLC1134; " \
				"fdt rm EGF_BLC1133; " \
				"fdt rm EGF_BLC1173; " \
				"fdt rm EGF_BLC1167; " \
				"fdt set lvds_channel1 status \"okay\"; " \
				"fdt rm ar1020; " \
				"fdt rm tsc2046; " \
				"fdt set backlight3 status \"okay\"; " \
				"fdt set panel4 status \"okay\"; " \
			"elif test \"${panel}\" = \"EGF_BLC1167\"; then " \
				"fdt rm EGF_BLC1134; " \
				"fdt rm EGF_BLC1133; " \
				"fdt rm EGF_BLC1173; " \
				"fdt rm EGF_BLC1168; " \
				"fdt set lvds_channel1 status \"okay\"; " \
				"fdt rm ar1020; " \
				"fdt rm ft5x06; " \
				"fdt set tsc2046 ti,x-plate-ohms [02 b2]; " \
				"fdt set tsc2046 ti,y-plate-ohms [00 cc]; " \
				"fdt set backlight3 status \"okay\"; " \
				"fdt set panel4 status \"okay\"; " \
			"elif test \"${panel}\" = \"EGF_BLC1173\"; then " \
				"fdt rm EGF_BLC1134; " \
				"fdt rm EGF_BLC1133; " \
				"fdt rm EGF_BLC1168; " \
				"fdt rm EGF_BLC1167; " \
				"fdt set lvds_channel1 status \"okay\"; " \
				"fdt rm ar1020; " \
				"fdt rm tsc2046; " \
				"fdt set backlight3 status \"okay\"; " \
				"fdt set panel4 status \"okay\"; " \
			"elif test \"${panel}\" = \"EGF_BLC1149\"; then " \
				"fdt rm EGF_BLC1134; " \
				"fdt rm EGF_BLC1133; " \
				"fdt rm EGF_BLC1168; " \
				"fdt rm EGF_BLC1173; " \
				"fdt rm EGF_BLC1167; " \
				"fdt rm ar1020; " \
				"fdt rm ft5x06; " \
				"fdt set tsc2046 ti,x-plate-ohms [02 5f]; " \
				"fdt set tsc2046 ti,y-plate-ohms [00 e1]; " \
				"fdt set mxcfb0 disp_dev \"lcd\"; " \
				"fdt set mxcfb0 mode_str \"EGF_BLC1149\"; " \
				"fdt set backlight3 status \"okay\"; " \
				"fdt set panel4 status \"okay\"; " \
			"elif test \"${panel}\" = \"EGF_BLC1152\"; then " \
				"fdt rm EGF_BLC1134; " \
				"fdt rm EGF_BLC1133; " \
				"fdt rm EGF_BLC1168; " \
				"fdt rm EGF_BLC1173; " \
				"fdt rm EGF_BLC1167; " \
				"fdt rm ar1020; " \
				"fdt rm tsc2046; " \
				"fdt set ft5x06 invert-x-axis [00]; " \
				"fdt set ft5x06 invert-y-axis [00]; " \
				"fdt set mxcfb0 disp_dev \"lcd\"; " \
				"fdt set mxcfb0 mode_str \"EGF_BLC1152\"; " \
				"fdt set backlight3 status \"okay\"; " \
				"fdt set panel4 status \"okay\"; " \
			"elif test \"${panel}\" = \"EGF_BLC1172\"; then " \
				"fdt rm EGF_BLC1134; " \
				"fdt rm EGF_BLC1133; " \
				"fdt rm EGF_BLC1168; " \
				"fdt rm EGF_BLC1173; " \
				"fdt rm EGF_BLC1167; " \
				"fdt rm ar1020; " \
				"fdt rm tsc2046; " \
				"fdt rm ft5x06; " \
				"fdt set mxcfb0 disp_dev \"lcd\"; " \
				"fdt set mxcfb0 mode_str \"EGF_BLC1172\"; " \
				"fdt set backlight3 status \"okay\"; " \
				"fdt set panel4 status \"okay\"; " \
			"else " \
				"echo invalid display selection ${panel}; " \
			"fi;" \
			"i2c dev 2; " \
			"if i2c probe 0x50 ; then " \
				"echo \"------ have HDMI monitor\";" \
				"fdt rm lvds_channel2 primary; " \
			"else " \
				"echo \"------ No HDMI monitor\";" \
				"fdt rm mxcfb1; " \
				"fdt rm hdmi_core; " \
				"fdt rm hdmi_video; " \
				"fdt rm hdmi_cec; " \
				"fdt rm hdmi_audio; " \
				"if test \"${pcb_rev}\" = \"PGF0533_A02\"; then " \
					"fdt set mxcfb2 status \"okay\"; " \
					"fdt set lvds_channel2 status \"okay\"; " \
					"fdt get value lvds1stat lvds_channel1 status; " \
					"if test \"${lvds1stat}\" = \"disabled\"; then " \
						"fdt rm lvds_channel1 primary; " \
					"else " \
						"fdt rm lvds_channel2 primary; " \
					"fi; " \
					"fdt set panel5 status \"okay\"; " \
					"gpio set 176; " \
					"fdt set backlight1 status \"okay\"; " \
				"else " \
					"fdt rm lvds_channel2 primary; " \
				"fi; " \
			"fi;\0" \
	"smp=" CONFIG_SYS_NOSMP "\0"\
	"sdargs=setenv bootargs console=${console},${baudrate} ${smp} ${g_ether_args} panel=${panel}\0" \
	"loadimage_sd=fatload mmc 0:1 ${loadaddr} ${image}\0" \
	"loadfdt_sd=fatload mmc 0:1 ${fdt_addr} ${fdt_file}\0" \
	"loadsplash_sd=fatload mmc 0:1 0x10000000 logo.bmp;bmp d 0x10000000;\0" \
	"sdboot=echo Try Booting from SD...; " \
		"mmc rescan; " \
		"if run loadfdt_sd; then " \
			"if run loadimage_sd; then " \
				"if test -e mmc 0:1 update.bin; then " \
					"echo Loading update from SDCard; " \
				"else " \
					"echo Booting from SD Card;" \
					"setenv sdargs ${sdargs} root=/dev/mmcblk0p2 rootwait rw; " \
				"fi; " \
				"run sdargs; " \
				"run loadsplash_sd; " \
				"echo ${bootargs}; " \
				"run fix_dt; " \
				"bootz ${loadaddr} - ${fdt_addr}; " \
			"fi; " \
		"fi;\0 " \
	"emmcargs=setenv bootargs console=${console},${baudrate} ${smp} ${g_ether_args} root=/dev/mmcblk2p2 rootwait rw panel=${panel}\0" \
	"loadimage_emmc=fatload mmc 1:1 ${loadaddr} ${image}\0" \
	"loadfdt_emmc=fatload mmc 1:1 ${fdt_addr} ${fdt_file}\0" \
	"loadsplash_emmc=fatload mmc 1:1 0x10000000 logo.bmp;bmp d 0x10000000;\0" \
	"emmcboot=echo Try Booting from eMMC...; " \
		"mmc rescan; " \
		"if run loadfdt_emmc; then " \
			"if run loadimage_emmc; then " \
				"echo Booting from eMMC Card; " \
				"run emmcargs; " \
				"run loadsplash_emmc;" \
				"echo ${bootargs}; " \
				"run fix_dt; " \
				"bootz ${loadaddr} - ${fdt_addr}; " \
			"fi; " \
		"fi;\0 " \
	"loadfdt_usb=fatload usb 0 ${fdt_addr} ${fdt_file}\0" \
	"loadimage_usb=fatload usb 0 ${loadaddr} ${image}\0" \
	"usbargs=setenv bootargs console=${console},${baudrate} ${smp} ${g_ether_args} panel=${panel}\0" \
	"usbboot=echo Try Booting from USB...;" \
		"usb start;" \
		"if run loadfdt_usb; then " \
			"if run loadimage_usb; then " \
				"if test -e usb 0 update.bin; then " \
					"echo Loading update from USB key; " \
				"else " \
					"echo Booting from USB key; " \
					"setenv usbargs ${usbargs} root=/dev/sda2 rootwait rw; " \
				"fi;" \
				"fatload usb 0 0x10000000 /logo.bmp;bmp d 0x10000000;" \
				"run usbargs; " \
				"run fix_dt; " \
				"bootz ${loadaddr} - ${fdt_addr};" \
			"fi; " \
		"fi;\0 " \
	"bootargs_sata=setenv bootargs console=${console},${baudrate} ${smp} " \
			"root=/dev/sda1 rootwait rw \0" \
	"bootcmd_sata=echo Booting from sata ...; " \
			"run bootargs_sata; sata init; " \
			"sata read ${loadaddr} 0x800  0x4000; " \
			"sata read ${fdt_addr} 0x8000 0x800; " \
			"bootz ${loadaddr} - ${fdt_addr} \0" \
	"bootcmd=echo Booting...; " \
	"run locknor;" \
	/* Try sdcard update */ \
	"run sdboot;" \
	/* Try usb update key */ \
	"run usbboot;" \
	/* Boot from eMMC */ \
	"run emmcboot;\0"
#endif

#define CONFIG_ARP_TIMEOUT     200UL

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP
#define CONFIG_SYS_HUSH_PARSER
#define CONFIG_SYS_PROMPT_HUSH_PS2     "> "
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE              1024

/* Print Buffer Size */
#define CONFIG_SYS_PBSIZE (CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS             256
#define CONFIG_SYS_BARGSIZE CONFIG_SYS_CBSIZE

#define CONFIG_CMD_MEMTEST
#define CONFIG_SYS_MEMTEST_START       0x10000000
#define CONFIG_SYS_MEMTEST_END         0x10010000
#define CONFIG_SYS_MEMTEST_SCRATCH     0x10800000

#define CONFIG_SYS_LOAD_ADDR           CONFIG_LOADADDR

#define CONFIG_CMDLINE_EDITING
#define CONFIG_STACKSIZE               (128 * 1024)

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS           1
#define PHYS_SDRAM                     MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE          PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR       IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE       IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* FLASH and environment organization */
#define CONFIG_SYS_NO_FLASH

#define CONFIG_ENV_SIZE			(8 * 1024)

#ifndef CONFIG_SYS_NOSMP
#define CONFIG_SYS_NOSMP
#endif

#ifdef CONFIG_CMD_SATA
#define CONFIG_DWC_AHSATA
#define CONFIG_SYS_SATA_MAX_DEVICE	1
#define CONFIG_DWC_AHSATA_PORT_ID	0
#define CONFIG_DWC_AHSATA_BASE_ADDR	SATA_ARB_BASE_ADDR
#define CONFIG_LBA48
#define CONFIG_LIBATA
#endif

#define CONFIG_SYS_USE_SPINOR
#ifdef CONFIG_SYS_USE_SPINOR
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_BAR
#define CONFIG_SPI_FLASH_SPANSION
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_SPI_FLASH_MACRONIX
#define CONFIG_MXC_SPI
#define CONFIG_SF_DEFAULT_BUS  3
#define CONFIG_SF_DEFAULT_SPEED 15000000
#define CONFIG_SF_DEFAULT_MODE (SPI_MODE_0)
#define CONFIG_SF_CS_GPIO			IMX_GPIO_NR(5, 2)
#define CONFIG_SF_DEFAULT_CS		1
#define CONFIG_SF_WPn_GPIO			90  /* GPIO3_IO26 */
#endif

#ifdef CONFIG_SYS_USE_EIMNOR
#undef CONFIG_SYS_NO_FLASH
#define CONFIG_SYS_FLASH_BASE           WEIM_ARB_BASE_ADDR
#define CONFIG_SYS_FLASH_SECT_SIZE     (128 * 1024)
#define CONFIG_SYS_MAX_FLASH_BANKS 1    /* max number of memory banks */
#define CONFIG_SYS_MAX_FLASH_SECT 256   /* max number of sectors on one chip */
#define CONFIG_SYS_FLASH_CFI            /* Flash memory is CFI compliant */
#define CONFIG_FLASH_CFI_DRIVER         /* Use drivers/cfi_flash.c */
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE /* Use buffered writes*/
#define CONFIG_SYS_FLASH_EMPTY_INFO
#endif

#ifdef CONFIG_SYS_USE_NAND
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NAND_TRIMFFS

/* NAND stuff */
#define CONFIG_NAND_MXS
#define CONFIG_SYS_MAX_NAND_DEVICE     1
#define CONFIG_SYS_NAND_BASE           0x40000000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_ONFI_DETECTION

/* DMA stuff, needed for GPMI/MXS NAND support */
#define CONFIG_APBH_DMA
#define CONFIG_APBH_DMA_BURST
#define CONFIG_APBH_DMA_BURST8
#endif

#if defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_ENV_OFFSET		(8 * 64 * 1024)
#elif defined(CONFIG_ENV_IS_IN_SPI_FLASH)
#define CONFIG_ENV_OFFSET              (1008 * 4 * 1024)
#define CONFIG_ENV_SECT_SIZE           (64 * 1024)
#define CONFIG_ENV_SPI_BUS             CONFIG_SF_DEFAULT_BUS
#define CONFIG_ENV_SPI_CS              CONFIG_SF_DEFAULT_CS
#define CONFIG_ENV_SPI_MODE            CONFIG_SF_DEFAULT_MODE
#define CONFIG_ENV_SPI_MAX_HZ          CONFIG_SF_DEFAULT_SPEED
#elif defined(CONFIG_ENV_IS_IN_FLASH)
#undef CONFIG_ENV_SIZE
#define CONFIG_ENV_SIZE                        CONFIG_SYS_FLASH_SECT_SIZE
#define CONFIG_ENV_SECT_SIZE           CONFIG_SYS_FLASH_SECT_SIZE
#define CONFIG_ENV_OFFSET              (4 * CONFIG_SYS_FLASH_SECT_SIZE)
#elif defined(CONFIG_ENV_IS_IN_NAND)
#undef CONFIG_ENV_SIZE
#define CONFIG_ENV_OFFSET              (60 << 20)
#define CONFIG_ENV_SECT_SIZE           (128 << 10)
#define CONFIG_ENV_SIZE                        CONFIG_ENV_SECT_SIZE
#elif defined(CONFIG_ENV_IS_IN_SATA)
#define CONFIG_ENV_OFFSET		(768 * 1024)
#define CONFIG_SATA_ENV_DEV		0
#define CONFIG_SYS_DCACHE_OFF /* remove when sata driver support cache */
#endif

#define CONFIG_OF_LIBFDT

#ifndef CONFIG_SYS_DCACHE_OFF
#define CONFIG_CMD_CACHE
#endif

/* I2C Configs */
#define CONFIG_CMD_I2C
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_SPEED		  100000

/* Framebuffer */
#define CONFIG_VIDEO
#define CONFIG_VIDEO_IPUV3
#define CONFIG_CFB_CONSOLE
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_BMP_16BPP
#define CONFIG_VIDEO_LOGO
#define CONFIG_VIDEO_BMP_LOGO
#define CONFIG_IMX_HDMI
/*#define CONFIG_IMX_VIDEO_SKIP*/

#if defined(CONFIG_ANDROID_SUPPORT)
#include "mx6sabreandroid_common.h"
#endif

#define CONFIG_SYS_FSL_USDHC_NUM	2 	/* SD1 SD - SD2 WiFi - SD3 eMMC */
#define CONFIG_SYS_MMC_ENV_DEV		1	/* SDHC3 */
#define CONFIG_SYS_MMC_ENV_PART                0       /* user partition */

/*
 * imx6 q/dl/solo pcie would be failed to work properly in kernel, if
 * the pcie module is iniialized/enumerated both in uboot and linux
 * kernel.
 * rootcause:imx6 q/dl/solo pcie don't have the reset mechanism.
 * it is only be RESET by the POR. So, the pcie module only be
 * initialized/enumerated once in one POR.
 * Set to use pcie in kernel defaultly, mask the pcie config here.
 * Remove the mask freely, if the uboot pcie functions, rather than
 * the kernel's, are required.
 */
/* #define CONFIG_CMD_PCI */
#ifdef CONFIG_CMD_PCI
#define CONFIG_PCI
#define CONFIG_PCI_PNP
#define CONFIG_PCI_SCAN_SHOW
#define CONFIG_PCIE_IMX
#define CONFIG_PCIE_IMX_PERST_GPIO	IMX_GPIO_NR(7, 12)
#define CONFIG_PCIE_IMX_POWER_GPIO	IMX_GPIO_NR(3, 19)
#endif

/* PMIC */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_PFUZE100
#define CONFIG_POWER_PFUZE100_I2C_ADDR	0x08

/* USB Configs */
#define CONFIG_CMD_USB
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_EHCI_MX6
#define CONFIG_USB_STORAGE
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_MXC_USB_PORTSC		(PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS		0
#define CONFIG_USB_MAX_CONTROLLER_COUNT	2 /* Enabled USB controller number */

#define CONFIG_CI_UDC
#define CONFIG_USBD_HS
#define CONFIG_USB_GADGET_DUALSPEED
#define CONFIG_USB_ETHER
#define CONFIG_USB_ETH_CDC
#define CONFIG_NETCONSOLE
#define CONFIG_SYS_USB_EVENT_POLL_VIA_CONTROL_EP
#define CONFIG_USB_GADGET
#define CONFIG_USB_GADGET_MASS_STORAGE
#define CONFIG_USBDOWNLOAD_GADGET
#define CONFIG_USB_GADGET_VBUS_DRAW	2
#define CONFIG_G_DNL_VENDOR_NUM		0x18d1
#define CONFIG_G_DNL_PRODUCT_NUM	0x0d02
#define CONFIG_G_DNL_MANUFACTURER	"FSL"
#define CONFIG_CMD_USB_MASS_STORAGE
#endif

#define EEPROM_nWP_GPIO (IMX_GPIO_NR(1, 27))

/*#define CONFIG_SPLASH_SCREEN*/
/*#define CONFIG_MXC_EPDC*/

#define CONFIG_CMD_BMP
#define CONFIG_CONSOLE_EXTRA_INFO
/*
 * SPLASH SCREEN Configs
 */
#if defined(CONFIG_SPLASH_SCREEN) && defined(CONFIG_MXC_EPDC)
	/*
	 * Framebuffer and LCD
	 */
	#define CONFIG_CMD_BMP
	#define CONFIG_LCD
	#define CONFIG_SYS_CONSOLE_IS_IN_ENV
	#undef LCD_TEST_PATTERN
	/* #define CONFIG_SPLASH_IS_IN_MMC			1 */
	#define LCD_BPP					LCD_MONOCHROME
	/* #define CONFIG_SPLASH_SCREEN_ALIGN		1 */

	#define CONFIG_WAVEFORM_BUF_SIZE		0x200000
#endif /* CONFIG_SPLASH_SCREEN && CONFIG_MXC_EPDC */

#endif                         /* __MX6EGFEVB_CONFIG_H */
