#include "gf_mux.h"

#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/mxc_i2c.h>

#define HDMI_CEC_PAD_CTRL (PAD_CTL_PUS_22K_UP |	\
	PAD_CTL_DSE_40ohm | PAD_CTL_ODE |			\
	PAD_CTL_SRE_SLOW  | PAD_CTL_HYS | 			\
	PAD_CTL_SPEED_MED)

#define KPP_ROW_PAD_CTRL (PAD_CTL_PUS_100K_UP |	\
	PAD_CTL_DSE_120ohm | 						\
	PAD_CTL_SRE_SLOW  | PAD_CTL_HYS)

#define KPP_COL_PAD_CTRL (PAD_CTL_PKE |			\
	PAD_CTL_DSE_40ohm | PAD_CTL_SPEED_MED |		\
	PAD_CTL_SRE_SLOW  | PAD_CTL_HYS)

#define AUDIO_PAD_CTRL (PAD_CTL_PUS_100K_DOWN | 	\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | 		\
	PAD_CTL_SRE_SLOW | PAD_CTL_HYS)

#define UART_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80ohm |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

//DSE(110) = 32 Ohm @ 3.3V
#define USDHC_PAD_CLK_CTRL (PAD_CTL_PUS_47K_UP |			\
	PAD_CTL_SPEED_LOW | (6 << 3) |			\
	PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP |			\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS)

#define SPI_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_SPEED_MED | \
		      PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#define I2C_PAD_CTRL  (PAD_CTL_PUS_100K_UP |                    \
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | PAD_CTL_HYS |   \
	PAD_CTL_ODE | PAD_CTL_SRE_FAST)
#define I2CPC MUX_PAD_CTRL(I2C_PAD_CTRL)
#define CONFIG_SYS_I2C_SPEED	100000

#define DIO_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_34ohm | PAD_CTL_HYS | PAD_CTL_SRE_FAST)
#define DIO_PAD_CFG   (MUX_PAD_CTRL(DIO_PAD_CTRL) | MUX_MODE_SION)
#define USBH1_PAD_NO_PULL_CFG   (MUX_PAD_CTRL(PAD_CTL_PKE |	 PAD_CTL_SPEED_MED |	\
								  PAD_CTL_DSE_34ohm | PAD_CTL_HYS | PAD_CTL_SRE_FAST) | MUX_MODE_SION)

#define DIO_PAD_PDOWN_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_34ohm | PAD_CTL_HYS | PAD_CTL_SRE_FAST)
#define DIO_PAD_PDOWN_CFG   (MUX_PAD_CTRL(DIO_PAD_PDOWN_CTRL) | MUX_MODE_SION)

#define DIO_PUP_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_34ohm | PAD_CTL_HYS | PAD_CTL_SRE_FAST)
#define DIO_PUP_PAD_CFG   (MUX_PAD_CTRL(DIO_PUP_PAD_CTRL) | MUX_MODE_SION)

#define GPMI_PAD_CTRL0 (PAD_CTL_PKE | PAD_CTL_PUE | PAD_CTL_PUS_100K_UP)
#define GPMI_PAD_CTRL1 (PAD_CTL_DSE_40ohm | PAD_CTL_SPEED_MED | \
			PAD_CTL_SRE_FAST)
#define GPMI_PAD_CTRL2 (GPMI_PAD_CTRL0 | GPMI_PAD_CTRL1)

#define NO_PAD_CTRL_SION_CFG	(MUX_PAD_CTRL(NO_PAD_CTRL) | MUX_MODE_SION)
#define DISP_PAD_CTRL			(0x10)
#define PWM_PAD_CTRL			(0x1b0b1)

/* UART1 RS-232 Debug*/
static iomux_v3_cfg_t const mx6_uart1_pads[] = {
	IOMUX_PADS(PAD_CSI0_DAT10__UART1_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_CSI0_DAT11__UART1_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D19__UART1_CTS_B | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D20__UART1_RTS_B | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

/* UART2 RS-232*/
static iomux_v3_cfg_t const mx6_uart2_pads[] = {
	IOMUX_PADS(PAD_SD4_DAT7__UART2_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT4__UART2_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_DAT6__UART2_CTS_B | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D29__UART2_RTS_B | MUX_PAD_CTRL(UART_PAD_CTRL)),
};

/* UART3 -> BT+BLE */
static iomux_v3_cfg_t const mx6_uart3_pads[] = {
	IOMUX_PADS(PAD_SD4_CMD__UART3_TX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_SD4_CLK__UART3_RX_DATA | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D30__UART3_CTS_B | MUX_PAD_CTRL(UART_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D31__UART3_RTS_B | MUX_PAD_CTRL(UART_PAD_CTRL)),
};


static void my_uart_init_mux(void)
{
	SETUP_IOMUX_PADS(mx6_uart1_pads);
	SETUP_IOMUX_PADS(mx6_uart2_pads);
	SETUP_IOMUX_PADS(mx6_uart3_pads);
}

/* I2C1 */
static struct i2c_pads_info mx6q_i2c_pad_info0 = {
	.scl = {
		.i2c_mode = MX6Q_PAD_CSI0_DAT9__I2C1_SCL | I2CPC,
		.gpio_mode = MX6Q_PAD_CSI0_DAT9__GPIO5_IO27 | I2CPC,
		.gp = IMX_GPIO_NR(5, 27)
	},
	.sda = {
		.i2c_mode = MX6Q_PAD_CSI0_DAT8__I2C1_SDA | I2CPC,
		.gpio_mode = MX6Q_PAD_CSI0_DAT8__GPIO5_IO26 | I2CPC,
		.gp = IMX_GPIO_NR(5, 26)
	}
};

/* I2C2 */
static struct i2c_pads_info mx6q_i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX6Q_PAD_EIM_EB2__I2C2_SCL | I2CPC,
		.gpio_mode = MX6Q_PAD_EIM_EB2__GPIO2_IO30 | I2CPC,
		.gp = IMX_GPIO_NR(2, 30)
	},
	.sda = {
		.i2c_mode = MX6Q_PAD_EIM_D16__I2C2_SDA | I2CPC,
		.gpio_mode = MX6Q_PAD_EIM_D16__GPIO3_IO16 | I2CPC,
		.gp = IMX_GPIO_NR(3, 16)
	}
};

/* I2C3 */
static struct i2c_pads_info mx6q_i2c_pad_info2 = {
	.scl = {
		.i2c_mode = MX6Q_PAD_EIM_D17__I2C3_SCL | I2CPC,
		.gpio_mode = MX6Q_PAD_EIM_D17__GPIO3_IO17 | I2CPC,
		.gp = IMX_GPIO_NR(3, 17)
	},
	.sda = {
		.i2c_mode = MX6Q_PAD_EIM_D18__I2C3_SDA | I2CPC,
		.gpio_mode = MX6Q_PAD_EIM_D18__GPIO3_IO18 | I2CPC,
		.gp = IMX_GPIO_NR(3, 18)
	}
};


static void my_i2c_init_mux(void)
{
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6q_i2c_pad_info0);
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6q_i2c_pad_info1);
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &mx6q_i2c_pad_info2);

}

/* ETHERNET */

static iomux_v3_cfg_t const mx6_enet_pads[] = {
	IOMUX_PADS(PAD_ENET_MDIO__ENET_MDIO			| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_MDC__ENET_MDC			| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TXC__RGMII_TXC			| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD0__RGMII_TD0			| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD1__RGMII_TD1			| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD2__RGMII_TD2			| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TD3__RGMII_TD3			| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_TX_CTL__RGMII_TX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_REF_CLK__ENET_TX_CLK	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RXC__RGMII_RXC			| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD0__RGMII_RD0			| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD1__RGMII_RD1			| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD2__RGMII_RD2			| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RD3__RGMII_RD3			| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_RGMII_RX_CTL__RGMII_RX_CTL	| MUX_PAD_CTRL(ENET_PAD_CTRL)),
	IOMUX_PADS(PAD_ENET_RXD1__GPIO1_IO26		| NO_PAD_CTRL_SION_CFG), // enet-int
	IOMUX_PADS(PAD_CSI0_DAT18__GPIO6_IO04		| DIO_PAD_CFG), // enable 3v3
	/* AR8031 PHY Reset */
	IOMUX_PADS(PAD_ENET_CRS_DV__GPIO1_IO25		| DIO_PAD_CFG),
};

static void my_enet_init_mux(void)
{
	SETUP_IOMUX_PADS(mx6_enet_pads);
	// Enable PHY supply
	gpio_direction_output((IMX_GPIO_NR(6, 4)),1);
}

/* ECSPI4*/
/* Notare che i chip select sono usati come GPIO e non nella
 * loro funzione nativa. La gestione nel kernel sembra diversa.
 * Fare riferimeno al driver mxc_spi.c in particolare alla funzione
 *
 * static int decode_cs(struct mxc_spi_slave *mxcs, unsigned int cs)
 *
 * La definizione del chip select  usato e' nel config
 * eg. #define CONFIG_SF_DEFAULT_CS		(0 | (IMX_GPIO_NR(3, 20) << 8))
 */
static iomux_v3_cfg_t const mx6_ecspi4_pads[] = {
	IOMUX_PADS(PAD_EIM_D21__ECSPI4_SCLK 	| MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D22__ECSPI4_MISO		| MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_EIM_D28__ECSPI4_MOSI 	| MUX_PAD_CTRL(SPI_PAD_CTRL)),
};

static iomux_v3_cfg_t const spinor_pads[] = {
	IOMUX_PADS(PAD_EIM_A25__GPIO5_IO02  	| NO_PAD_CTRL_SION_CFG),	//CS 1 -> pullup 10K
	IOMUX_PADS(PAD_EIM_D26__GPIO3_IO26  	| NO_PAD_CTRL_SION_CFG),	//WP   -> pulldown 47K
};

static iomux_v3_cfg_t const mx6_ecspi3_pads[] = {
	IOMUX_PADS(PAD_DISP0_DAT2__ECSPI3_MISO	| MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_DISP0_DAT1__ECSPI3_MOSI	| MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_DISP0_DAT0__ECSPI3_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL)),
	IOMUX_PADS(PAD_DISP0_DAT3__GPIO4_IO24  	| NO_PAD_CTRL_SION_CFG),	//CS 0
	IOMUX_PADS(PAD_DISP0_DAT4__GPIO4_IO25  	| NO_PAD_CTRL_SION_CFG),	//CS 1
};

static void my_spi4_init_mux(void)
{
	SETUP_IOMUX_PADS(mx6_ecspi4_pads);
}

static void my_spi3_init_mux(void)
{
	SETUP_IOMUX_PADS(mx6_ecspi3_pads);
}

static void my_spinor_init_mux(void)
{
	SETUP_IOMUX_PADS(spinor_pads);
}

/* eMCC
 *
 * SD3 -> Interna al Modulo
 *
 * */
static iomux_v3_cfg_t const mx6_usdhc3_pads[] = {
	IOMUX_PADS(PAD_SD3_CLK__SD3_CLK   | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_CMD__SD3_CMD   | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT0__SD3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT1__SD3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT2__SD3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT3__SD3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT4__SD3_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT5__SD3_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT6__SD3_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_DAT7__SD3_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD3_RST__SD3_RESET  | MUX_PAD_CTRL(NO_PAD_CTRL)), /* Segnale di Reset Gestito dal Bios */
};

static void my_emmc_init_mux(void)
{
	SETUP_IOMUX_PADS(mx6_usdhc3_pads);
}

/* Modulo WiFi SDIO
 *
 * SD2
 *
 * */
static iomux_v3_cfg_t const mx6_usdhc2_pads[] = {
	IOMUX_PADS(PAD_SD2_CLK__SD2_CLK			| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_CMD__SD2_CMD			| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT0__SD2_DATA0		| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT1__SD2_DATA1		| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT2__SD2_DATA2		| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
	IOMUX_PADS(PAD_SD2_DAT3__SD2_DATA3		| MUX_PAD_CTRL(USDHC_PAD_CTRL)),
};


static void my_sdio_wifi_init_mux(void)
{
	SETUP_IOMUX_PADS(mx6_usdhc2_pads);
}

/*
 * USB OTG
 *
 * */

static iomux_v3_cfg_t const usbotg_pads[] = {
	IOMUX_PADS(PAD_ENET_RX_ER__USB_OTG_ID	| DIO_PAD_CFG),
};


static void my_usb_init_mux(void)
{
	SETUP_IOMUX_PADS(usbotg_pads);
	/*set daisy chain for otg_pin_id on 6q. for 6dl, this bit is reserved*/
	imx_iomux_set_gpr_register(1, 13, 1, 0);
}

/**
 * eeprom modulo e carrier
 */
static iomux_v3_cfg_t const eeprom_pads[] = {
	IOMUX_PADS(PAD_EIM_D24__GPIO3_IO24			| DIO_PAD_CFG), // eeprom_wp modulo
	IOMUX_PADS(PAD_CSI0_DATA_EN__GPIO5_IO20		| DIO_PAD_CFG), // EEP_WP carrier
};

static void my_eeprom_init_mux(void)
{
	SETUP_IOMUX_PADS(eeprom_pads);
}

static iomux_v3_cfg_t const gpio_pads[] = {
	IOMUX_PADS(PAD_CSI0_VSYNC__GPIO5_IO21 	| DIO_PAD_CFG), // LED_GPIO5-IO21
	IOMUX_PADS(PAD_SD1_CLK__GPIO1_IO20		| DIO_PAD_PDOWN_CFG), // WLAN_EN_3V3 - GPIO 20
	IOMUX_PADS(PAD_SD1_CMD__GPIO1_IO18		| DIO_PAD_PDOWN_CFG), // BT_EN_3V3 - GPIO 18
	IOMUX_PADS(PAD_NANDF_RB0__GPIO6_IO10	| DIO_PAD_PDOWN_CFG), // WIFI_PWR_EN - GPIO 170
	IOMUX_PADS(PAD_CSI0_DAT19__GPIO6_IO05	| DIO_PAD_CFG), // WLAN_IRQ_3V3 - GPIO 165
	IOMUX_PADS(PAD_GPIO_8__XTALOSC_REF_CLK_32K	| DIO_PAD_CFG), // W32kHZ SLOW CLOCK
	IOMUX_PADS(PAD_CSI0_DAT14__GPIO6_IO00	| DIO_PAD_CFG), // GPIO6-IO00
	IOMUX_PADS(PAD_CSI0_DAT13__GPIO5_IO31	| DIO_PAD_CFG), // GPIO5-IO31
	IOMUX_PADS(PAD_CSI0_DAT12__GPIO5_IO30	| DIO_PAD_CFG), // GPIO5-IO30

	IOMUX_PADS(PAD_DISP0_DAT7__GPIO4_IO28		| DIO_PAD_CFG), // PMIC_nINT
	IOMUX_PADS(PAD_GPIO_9__PWM1_OUT				| DIO_PAD_CFG), // PWM1_OUT
	IOMUX_PADS(PAD_SD4_DAT1__GPIO2_IO09			| DIO_PAD_CFG), // PWM3_OUT - GPIO 41
	IOMUX_PADS(PAD_DISP0_DAT11__GPIO5_IO05		| DIO_PAD_CFG), // GPIO 133 - DISP0_ONOFF
};

static void my_gpio_init_mux(void)
{
	SETUP_IOMUX_PADS(gpio_pads);

}

static iomux_v3_cfg_t const can_pads[] = {
	IOMUX_PADS(PAD_KEY_ROW4__FLEXCAN2_RX	| NO_PAD_CTRL_SION_CFG),
	IOMUX_PADS(PAD_KEY_COL4__FLEXCAN2_TX	| NO_PAD_CTRL_SION_CFG),
	IOMUX_PADS(PAD_CSI0_DAT17__GPIO6_IO03	| DIO_PAD_CFG), // CAN-SHDN - GPIO 163
	IOMUX_PADS(PAD_CSI0_DAT16__GPIO6_IO02	| DIO_PAD_CFG), // CAN-SILENT - GPIO 162
};

static void my_can_init_mux(void)
{
	SETUP_IOMUX_PADS(can_pads);
	// Shut down can transceiver
	gpio_direction_output((IMX_GPIO_NR(6, 3)),1);
}

static iomux_v3_cfg_t const cam_digital_pads[] = {
	IOMUX_PADS(PAD_SD4_DAT3__GPIO2_IO11 	| DIO_PAD_CFG), // CAMD_nRST_3V3
	IOMUX_PADS(PAD_SD4_DAT0__GPIO2_IO08 	| DIO_PAD_CFG), // CAMD_PWDN_3V3
	IOMUX_PADS(PAD_GPIO_0__CCM_CLKO1		| MUX_PAD_CTRL(NO_PAD_CTRL)), // CAMD_CSI0_MCK_3V3
};

static void my_cam_init_mux(void)
{
	SETUP_IOMUX_PADS(cam_digital_pads);
}

static iomux_v3_cfg_t const audio_pads[] = {
	IOMUX_PADS(PAD_DISP0_DAT18__AUD5_TXFS | MUX_PAD_CTRL(AUDIO_PAD_CTRL)), // AUDIO_I2S_LRCLK
	IOMUX_PADS(PAD_DISP0_DAT16__AUD5_TXC 	| MUX_PAD_CTRL(AUDIO_PAD_CTRL)),  // AUDIO_I2S_SCLK
	IOMUX_PADS(PAD_DISP0_DAT19__AUD5_RXD 	| MUX_PAD_CTRL(AUDIO_PAD_CTRL)), // AUDIO_I2S_DIN
	IOMUX_PADS(PAD_DISP0_DAT17__AUD5_TXD 	| MUX_PAD_CTRL(AUDIO_PAD_CTRL)), // AUDIO_I2S_DOUT
};

static void my_audio_init_mux(void)
{
	SETUP_IOMUX_PADS(audio_pads);
}

static iomux_v3_cfg_t const power_management_pads[] = {
	IOMUX_PADS(PAD_NANDF_D2__GPIO2_IO02			| DIO_PAD_PDOWN_CFG), 		// IUSB2
	IOMUX_PADS(PAD_NANDF_D6__GPIO2_IO06			| DIO_PAD_PDOWN_CFG), 		// IUSB3
	IOMUX_PADS(PAD_NANDF_D1__GPIO2_IO01			| DIO_PAD_PDOWN_CFG), 		// nIUSB1
	IOMUX_PADS(PAD_SD1_DAT0__GPIO1_IO16			| DIO_PAD_CFG),			// PSHOLD - GPIO 16
	IOMUX_PADS(PAD_SD1_DAT2__GPIO1_IO19			| DIO_PAD_CFG),			// PBOUT - GPIO 19
	IOMUX_PADS(PAD_CSI0_MCLK__GPIO5_IO19		| DIO_PAD_PDOWN_CFG),	// PBCLR - GPIO 147
};

static void my_power_management_init_mux(void)
{
	SETUP_IOMUX_PADS(power_management_pads);
	// Enable power retention
	gpio_direction_output((IMX_GPIO_NR(1, 16)),1);
	//Initialize IUSB pin status to 1500mA
	gpio_direction_output((IMX_GPIO_NR(2, 1)),1); //nIUSB1 = 1
	gpio_direction_output((IMX_GPIO_NR(2, 2)),1); //IUSB2 = 1
	gpio_direction_output((IMX_GPIO_NR(2, 6)),0); //IUSB3 = 0

}

void egf_board_mux_init(int mode)
{
	switch(mode){
	case PROGRAMMER_MUX_MODE:
		my_power_management_init_mux();
		my_uart_init_mux();
		my_gpio_init_mux();
		my_i2c_init_mux();
		my_eeprom_init_mux();
		my_enet_init_mux();
		my_spi4_init_mux();
		my_emmc_init_mux();
		my_spinor_init_mux();
		my_usb_init_mux();
		break;
	case APPLICATION_MUX_MODE:
		my_power_management_init_mux();
		my_uart_init_mux();
		my_gpio_init_mux();
		my_i2c_init_mux();
		my_spi4_init_mux();
		my_spinor_init_mux();
		my_enet_init_mux();
		my_emmc_init_mux();
		my_sdio_wifi_init_mux();
		my_usb_init_mux();
		my_eeprom_init_mux();
		my_cam_init_mux();
		my_audio_init_mux();
		my_can_init_mux();
		my_spi3_init_mux();
		break;
	default:
		printf("MUX MODE NOT DEFINED!!!!!!!!!!!!\n");
		break;
	}
}

