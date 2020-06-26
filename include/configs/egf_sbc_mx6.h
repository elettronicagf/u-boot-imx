/*
 * Copyright (C) 2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 *
 * Configuration settings for ElettronicaGF SBC imx6UL/ULL/ULZ
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef __EGF_SBC_CONFIG_H
#define __EGF_SBC_CONFIG_H


#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>
#include "mx6_common.h"
#include <asm/mach-imx/gpio.h>
#include "imx_env.h"

#define CFG_LCD_POWER_ENABLE	46

#ifdef CONFIG_SECURE_BOOT
#ifndef CONFIG_CSF_SIZE
#define CONFIG_CSF_SIZE 0x4000
#endif
#endif

#define is_mx6ull_9x9_evk()	CONFIG_IS_ENABLED(TARGET_MX6ULL_9X9_EVK)

#ifdef CONFIG_SPL
#include "imx6_spl.h"
#endif

#define CONFIG_MEMBLK

/* uncomment for PLUGIN mode support */
/* #define CONFIG_USE_PLUGIN */

/* uncomment for SECURE mode support */
/* #define CONFIG_SECURE_BOOT */

/* uncomment for BEE support, needs to enable CONFIG_CMD_FUSE */
/* #define CONFIG_CMD_BEE */

#ifdef CONFIG_SECURE_BOOT
#ifndef CONFIG_CSF_SIZE
#define CONFIG_CSF_SIZE 0x4000
#endif
#endif

#define CONFIG_BOOTARGS_CMA_SIZE   ""
/* DCDC used on 14x14 EVK, no PMIC */
#undef CONFIG_LDO_BYPASS_CHECK

#define CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(16 * SZ_1M)

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_IMX6_PWM_PER_CLK 66666000
#define CONFIG_PWM_IMX

#define CONFIG_MXC_UART
#define CONFIG_MXC_UART_BASE		UART3_BASE

/* #define CONFIG_MMC */
#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_BOUNCE_BUFFER
#define CONFIG_FSL_ESDHC
#define CONFIG_FSL_USDHC

#ifdef CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC1_BASE_ADDR
#endif

#undef CONFIG_SUPPORT_EMMC_BOOT

/* I2C configs */
#define CONFIG_CMD_I2C
#ifdef CONFIG_CMD_I2C
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_MXC_I2C4		/* enable I2C bus 2 */
#define CONFIG_SYS_I2C_SPEED		100000

/* PMIC */
#define CONFIG_POWER
#define CONFIG_POWER_I2C
#define CONFIG_POWER_PFUZE3000
#define CONFIG_POWER_PFUZE3000_I2C_ADDR  0x08
#endif

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

#define CFG_UPDATE_PACKAGE_LOADADDR 	0x83100000
#define CFG_LOGO_LOADADDR 			0x84C00000
#define CFG_UPDATE_PACKAGE_HEADER_LENGTH	36
#define CFG_UPDATE_PACKAGE_FAT_BLOB_LOAD_ADDRESS (CFG_UPDATE_PACKAGE_LOADADDR + CFG_UPDATE_PACKAGE_HEADER_LENGTH)
#define CFG_UPDATE_PACKAGE_FAT_BLOB_LENGTH 0x1000000 /* Must be multiple of 512 */
#define CFG_UPDATE_PACKAGE_INITIAL_LOAD_LENGTH 0x1000024 /* CFG_UPDATE_PACKAGE_HEADER_LENGTH + CFG_UPDATE_PACKAGE_FAT_BLOB_LENGTH */



#ifdef CONFIG_WID
#define CONFIG_MFG_ENV_SETTINGS \
	    "console=serial;" \
		"setenv stdin serial; " \
		"setenv stdout serial; " \
		"setenv stderr serial; " \
		"g_mass_storage.stall=0 g_mass_storage.removable=1 " \
		"g_mass_storage.file=/fat g_mass_storage.ro=1 " \
		"g_mass_storage.idVendor=0x066F g_mass_storage.idProduct=0x37FF "\
		"g_mass_storage.iSerialNumber=\"\" \0" \
		"destroyenv=sf probe; gpio set " __stringify(CONFIG_SF_WPn_GPIO) ";" \
				"sf unlock; sf erase 0x1F0000 0x10000;sf lock;" \
				"gpio clear " __stringify(CONFIG_SF_WPn_GPIO) ";" \
				"env default -f -a;\0" \
		"spl_copy_addr=0x80100000\0" \
		"uboot_img_copy_addr=0x80600000\0" \
		"bootcmd_mfg=sf probe;" \
		"gpio clear " __stringify(CFG_LCD_POWER_ENABLE) ";" \
		"gpio set " __stringify(CONFIG_SF_WPn_GPIO) ";" \
		"sf unlock;" \
		"sf erase 0x0 0x200000;" \
		"sf write ${spl_copy_addr} " __stringify(CONFIG_SYS_SPI_SPL_OFFS) " 0x20000;" \
		"sf write ${uboot_img_copy_addr} " __stringify(CONFIG_SYS_SPI_U_BOOT_OFFS) " 0x100000;" \
		"mw.l 800eFFF8 0100fbfa 1;" \
		"if cmp.b 800eFFFC 800eFFF8 4; then " \
			"init_eeprom " CONFIG_WID ";" \
		"fi;" \
		"sf lock;" \
		"gpio clear " __stringify(CONFIG_SF_WPn_GPIO) ";" \
		"mw.l 20e01b8 5 1;" \
		"gpio set " __stringify(CFG_LCD_POWER_ENABLE) ";\0 "
#else
#define CONFIG_MFG_ENV_SETTINGS \
		"setenv stdin serial; " \
		"setenv stdout serial; " \
		"setenv stderr serial; "
#endif

#if defined(CONFIG_SYS_BOOT_NAND)
#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_MFG_ENV_SETTINGS \
	"fdt_addr=0x83000000\0" \
	"fdt_high=0xffffffff\0"	  \
	"console=ttymxc0\0" \
	"bootargs=console=ttymxc0,115200 ubi.mtd=3 "  \
		"root=ubi0:rootfs rootfstype=ubifs "		     \
		CONFIG_BOOTARGS_CMA_SIZE \
		"mtdparts=gpmi-nand:64m(boot),16m(kernel),16m(dtb),-(rootfs)\0"\
	"bootcmd=nand read ${loadaddr} 0x4000000 0x800000;"\
		"nand read ${fdt_addr} 0x5000000 0x100000;"\
		"bootz ${loadaddr} - ${fdt_addr}\0"

#else
#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_MFG_ENV_SETTINGS \
	"locknor=sf probe; gpio set " __stringify(CONFIG_SF_WPn_GPIO) ";sf lock; gpio clear " __stringify(CONFIG_SF_WPn_GPIO) ";\0" \
	"unlocknor=sf probe; gpio set " __stringify(CONFIG_SF_WPn_GPIO) ";sf unlock;\0" \
	"script=boot.scr\0" \
	"image=zImage\0" \
	"console=ttymxc2\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_file=undefined\0" \
	"fdt_addr=0x83000000\0" \
	"boot_fdt=try\0" \
	"update_md5=\0" \
	"logo=logo-boot.bmp\0" \
	"g_ether_args=g_ether.dev_addr=58:05:56:00:04:5e g_ether.host_addr=58:05:56:00:04:5d\0" \
	"ip_dyn=yes\0" \
	"usb_pgood_delay=1600\0" \
	"destroyenv=sf probe; gpio set " __stringify(CONFIG_SF_WPn_GPIO) ";" \
			"sf unlock; sf erase 0x1E0000 0x10000;sf lock;" \
			"gpio clear " __stringify(CONFIG_SF_WPn_GPIO) ";" \
			"env default -f -a;\0" \
	"fix_dt=fdt addr ${fdt_addr}; " \
		    "if test \"${panel}\" = \"EGF_BLC1154\"; then " \
		     	 "fdt rm EGF_BLC1155; " \
		     	 "fdt rm EGF_BLC1156; " \
		     	 "fdt rm EGF_BLC1165; " \
		     	 "fdt rm EGF_BLC1177; " \
		     	 "fdt rm EGF_BLC1182; " \
				 "fdt rm EGF_BLC1185; " \
		    "elif test \"${panel}\" = \"EGF_BLC1155\"; then " \
		     	 "fdt rm EGF_BLC1154; " \
		     	 "fdt rm EGF_BLC1156; " \
		     	 "fdt rm EGF_BLC1165; " \
		     	 "fdt rm EGF_BLC1177; " \
		     	 "fdt rm EGF_BLC1182; " \
				 "fdt rm EGF_BLC1185; " \
		    "elif test \"${panel}\" = \"EGF_BLC1156\"; then " \
		     	 "fdt rm EGF_BLC1154; " \
		     	 "fdt rm EGF_BLC1155; " \
		     	 "fdt rm EGF_BLC1165; " \
		     	 "fdt rm EGF_BLC1177; " \
		     	 "fdt rm EGF_BLC1182; " \
				 "fdt rm EGF_BLC1185; " \
		     "elif test \"${panel}\" = \"EGF_BLC1165\"; then " \
		     	 "fdt rm EGF_BLC1154; " \
		     	 "fdt rm EGF_BLC1155; " \
		     	 "fdt rm EGF_BLC1156; " \
		     	 "fdt rm EGF_BLC1177; " \
		     	 "fdt rm EGF_BLC1182; " \
				 "fdt rm EGF_BLC1185; " \
		     "elif test \"${panel}\" = \"EGF_BLC1177\"; then " \
		     	 "fdt rm EGF_BLC1154; " \
		     	 "fdt rm EGF_BLC1155; " \
		     	 "fdt rm EGF_BLC1156; " \
		     	 "fdt rm EGF_BLC1165; " \
		     	 "fdt rm EGF_BLC1182; " \
				 "fdt rm EGF_BLC1185; " \
		     "elif test \"${panel}\" = \"EGF_BLC1182\"; then " \
		     	 "fdt rm EGF_BLC1154; " \
		     	 "fdt rm EGF_BLC1155; " \
		     	 "fdt rm EGF_BLC1156; " \
		     	 "fdt rm EGF_BLC1165; " \
				 "fdt rm EGF_BLC1177; " \
				 "fdt rm EGF_BLC1185; " \
		     "elif test \"${panel}\" = \"EGF_BLC1185\"; then " \
		     	 "fdt rm EGF_BLC1154; " \
		     	 "fdt rm EGF_BLC1155; " \
		     	 "fdt rm EGF_BLC1156; " \
		     	 "fdt rm EGF_BLC1165; " \
				 "fdt rm EGF_BLC1177; " \
				 "fdt rm EGF_BLC1182; " \
			"else " \
				"echo invalid display selection ${panel}; " \
			"fi;\0" \
	"check_update_header=egf_update_validate_header " __stringify(CFG_UPDATE_PACKAGE_LOADADDR) "\0" \
	"display_update_logo=fatload mem 0 " __stringify(CFG_LOGO_LOADADDR)" ${logo};bmp display " __stringify(CFG_LOGO_LOADADDR)" ;\0" \
	"loadfdt_update=fatload mem 0 ${fdt_addr} ${fdt_file}\0" \
	"loadimage_update=fatload mem 0 ${loadaddr} ${image}\0" \
	"start_ota=egf_ota_start\0" \
	"loadfdt_usb=fatload usb 0 ${fdt_addr} ${fdt_file}\0" \
	"loadimage_usb=fatload usb 0 ${loadaddr} ${image}\0" \
	"emmcargs=setenv bootargs ${bootargs_base} ${smp} ${g_ether_args} panel=${panel} root=/dev/mmcblk0p2 rootfstype=ext4 rootwait rw \0" \
	"loadimage_emmc=fatload mmc 0 ${loadaddr} ${image}\0" \
	"loadfdt_emmc=fatload mmc 0 ${fdt_addr} ${fdt_file}\0" \
	"loadsplash_emmc=if fatload mmc 0 0x80000000 ${logo}; then bmp d 0x80000000; fi;\0" \
	"usbargs=setenv bootargs console=${console},${baudrate} ${smp} ${g_ether_args} panel=${panel}\0" \
	"usbboot=echo Try Booting from USB...;" \
			"usb start;" \
			"if run loadfdt_usb; then " \
				"if run loadimage_usb; then " \
					"echo Booting from USB key; " \
					"setenv usbargs ${usbargs} root=/dev/sda2 rootwait rw; " \
					"fatload usb 0 0x80000000 ${logo};bmp d 0x80000000;" \
					"run usbargs; " \
					"run fix_dt; " \
					"bootz ${loadaddr} - ${fdt_addr};" \
				"fi; " \
			"fi;\0 " \
	"loadupdatepackage_usb=fatload usb 0 " __stringify(CFG_UPDATE_PACKAGE_LOADADDR) " update.eup " __stringify(CFG_UPDATE_PACKAGE_INITIAL_LOAD_LENGTH) "\0" \
	"usbupdate_args=setenv bootargs console=${console},${baudrate} ${smp} ${g_ether_args} update_md5=${update_md5} ${wid} imx_cpu=${imx_cpu} sn=${sn} panel=${panel}\0" \
	"usbupdate=echo Try Update from USB...;" \
			"usb start;" \
			"if run loadupdatepackage_usb; then " \
				"echo Found an update package file on usb_key; " \
				"if run check_update_header; then " \
					"echo Update package signature verified; " \
					"run display_update_logo; " \
					"egf_ota_disable; "\
					"if run loadfdt_update; then " \
						"if run loadimage_update; then " \
							"run usbupdate_args; " \
							"run fix_dt; " \
							"bootz ${loadaddr} - ${fdt_addr};" \
						"fi; " \
					"fi; " \
				"fi; " \
			"fi;\0" \
	"loadupdatepackage_ota=ext4load mmc 0 " __stringify(CFG_UPDATE_PACKAGE_LOADADDR) " update.eup " __stringify(CFG_UPDATE_PACKAGE_INITIAL_LOAD_LENGTH) "\0" \
	"otaupdate_args=setenv bootargs console=${console},${baudrate} ${smp} ${g_ether_args} otaupdate update_md5=${update_md5} ${wid} imx_cpu=${imx_cpu} sn=${sn} panel=${panel}\0" \
	"otaupdate=echo Try Update OTA...;" \
				"if run start_ota; then " \
					"echo OTA update pending...;" \
					"if run loadupdatepackage_ota; then " \
						"echo Found an update package file on OTA partition; " \
						"if run check_update_header; then " \
							"echo Update package signature verified; " \
							"if run loadfdt_update; then " \
								"if run loadimage_update; then " \
									"run otaupdate_args; " \
									"run fix_dt; " \
									"bootz ${loadaddr} - ${fdt_addr};" \
								"fi; " \
							"fi; " \
						"fi; " \
					"fi; " \
				"fi;\0" \
	"emmcboot=echo Try Booting from eMMC...; " \
		"mmc rescan; " \
		"if run loadfdt_emmc; then " \
			"if run loadimage_emmc; then " \
				"echo Booting from eMMC Card; " \
			    "run loadfdt_emmc; "\
			    "run fix_dt; " \
				"run emmcargs; " \
				"echo ${bootargs}; " \
				"bootz ${loadaddr} - ${fdt_addr}; " \
			"fi; " \
		"fi;\0 " \


#define CONFIG_BOOTCOMMAND \
	   "setenv splashpos m,m; " \
	   "mmc rescan; " \
	   "run loadsplash_emmc;" \
	   "memblk init;usb reset;" \
	   "run usbupdate;" \
	   "run otaupdate;" \
	   "run usbboot;" \
	   "run emmcboot;"

#endif

/* Miscellaneous configurable options */
#define CONFIG_CMD_INIT_EEPROM
#define CONFIG_CMD_MEMTEST
#define CONFIG_SYS_MEMTEST_START	0x80000000
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_MEMTEST_START + 0x8000000)

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_HZ			1000

#define CONFIG_STACKSIZE		SZ_128K

/* Physical Memory Map */
#define CONFIG_NR_DRAM_BANKS		1
#define PHYS_SDRAM			MMDC0_ARB_BASE_ADDR

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* FLASH and environment organization */
#define CONFIG_SYS_NO_FLASH

/* #define CONFIG_CMD_BMODE */

#define CONFIG_SYS_USE_SPINOR
#define CONFIG_MXC_SPI
#define CONFIG_CMD_SF
#define CONFIG_SPI_FLASH
#define CONFIG_SPI_FLASH_BAR
#define CONFIG_SF_DEFAULT_SPEED	15000000
#define CONFIG_SF_DEFAULT_BUS		2
#define CONFIG_SF_DEFAULT_CS 		0
#define CONFIG_SF_DEFAULT_MODE		SPI_MODE_0
#define CONFIG_SF_WPn_GPIO			131  /* GPIO5_IO03 */
#define CONFIG_SPI_FLASH_SPANSION
#define CONFIG_SPI_FLASH_WINBOND
#define CONFIG_SPI_FLASH_MACRONIX
#define CONFIG_SPI_FLASH_SST
#define CONFIG_SPI_FLASH_STMICRO
#define CONFIG_CMD_MTDPARTS

/* NAND stuff */
#ifdef CONFIG_SYS_USE_NAND
#define CONFIG_CMD_NAND
#define CONFIG_CMD_NAND_TRIMFFS
/* NAND */
#define CONFIG_CMD_UBI
#define CONFIG_CMD_UBIFS
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_RBTREE
#define CONFIG_LZO

#define CONFIG_NAND_MXS
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0x40000000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_ONFI_DETECTION

/* DMA stuff, needed for GPMI/MXS NAND support */
#define CONFIG_APBH_DMA
#define CONFIG_APBH_DMA_BURST
#define CONFIG_APBH_DMA_BURST8
#endif

#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_SIZE			SZ_8K
#if defined(CONFIG_ENV_IS_IN_MMC)
#define CONFIG_ENV_OFFSET		(12 * SZ_64K)
#elif defined(CONFIG_ENV_IS_IN_SPI_FLASH)
#define CONFIG_ENV_SECT_SIZE		(64 * 1024)
#define CONFIG_ENV_OFFSET		(480 * 4 * 1024)
#define CONFIG_ENV_SPI_BUS		CONFIG_SF_DEFAULT_BUS
#define CONFIG_ENV_SPI_CS		CONFIG_SF_DEFAULT_CS
#define CONFIG_ENV_SPI_MODE		CONFIG_SF_DEFAULT_MODE
#define CONFIG_ENV_SPI_MAX_HZ		CONFIG_SF_DEFAULT_SPEED
#elif defined(CONFIG_ENV_IS_IN_NAND)
#undef CONFIG_ENV_SIZE
#define CONFIG_ENV_OFFSET		(60 << 20)
#define CONFIG_ENV_SECT_SIZE		(128 << 10)
#define CONFIG_ENV_SIZE			CONFIG_ENV_SECT_SIZE
#endif


/* USB Configs */
#ifndef CONFIG_WID
#define CONFIG_CMD_USB
#endif
#ifdef CONFIG_CMD_USB
#define CONFIG_USB_EHCI
#define CONFIG_USB_STORAGE
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_USB_HOST_ETHER
#define CONFIG_USB_ETHER_ASIX
#define CONFIG_MXC_USB_PORTSC  (PORT_PTS_UTMI | PORT_PTS_PTW)
#define CONFIG_MXC_USB_FLAGS   0
#define CONFIG_USB_MAX_CONTROLLER_COUNT 2
#endif

#define CONFIG_CMD_EGF_UPDATE

#ifdef CONFIG_CMD_NET
#define CONFIG_LIB_RAND
#define CONFIG_NET_RANDOM_ETHADDR
#define CONFIG_FEC_MXC
#define CONFIG_MII
#define CONFIG_FEC_ENET_DEV		0
#define IMX_FEC_BASE			ENET_BASE_ADDR
#define CONFIG_FEC_MXC_PHYADDR          0x0
#define CONFIG_FEC_XCV_TYPE             RMII
#define CONFIG_ETHPRIME			"FEC"
#define CONFIG_PHYLIB
#define CONFIG_PHY_SMSC
#define CONFIG_CMD_MII
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_PING
#endif

#define CONFIG_IMX_THERMAL

#ifndef CONFIG_SPL_BUILD
#define CONFIG_VIDEO
#ifdef CONFIG_VIDEO
#define CONFIG_CFB_CONSOLE
#define CONFIG_NO_CFB_STDOUT
#define CONFIG_VIDEO_MXS
#define CONFIG_VIDEO_SW_CURSOR
#define CONFIG_VGA_AS_SINGLE_DEVICE
#define CONFIG_SYS_CONSOLE_IS_IN_ENV
#define CONFIG_SPLASH_SCREEN
#define CONFIG_SPLASH_SCREEN_ALIGN
#define CONFIG_CMD_BMP

#define CONFIG_SYS_CONSOLE_FG_COL 0xFF
#define CONFIG_SYS_CONSOLE_BG_COL 0x00

#ifdef CONFIG_WID
#define CONFIG_VIDEO_LOGO
#define CONFIG_CONSOLE_EXTRA_INFO
#else

#endif


#define CONFIG_BMP_16BPP
#define CONFIG_VIDEO_BMP_RLE8
#define CONFIG_VIDEO_BMP_LOGO
#endif
#endif

#define CONFIG_MODULE_FUSE

#endif
