#include "gf_mux.h"

#include <common.h>
#include <i2c.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/arch/imx-regs.h>

#define UART_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define SPI_PAD_CTRL (PAD_CTL_HYS |				\
	PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm | PAD_CTL_SRE_FAST)

#define CAN_PAD_CTRL (PAD_CTL_HYS |				\
	PAD_CTL_PUS_100K_UP | PAD_CTL_PKE | PAD_CTL_PUE | \
	PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_60ohm | PAD_CTL_SRE_SLOW)

#define I2C_PAD_CTRL    (PAD_CTL_PKE | PAD_CTL_PUE |            \
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |               \
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS |			\
	PAD_CTL_ODE)

#define DIO_PDOWN_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_DOWN | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS | PAD_CTL_SRE_FAST)
#define DIO_PDOWN_PAD_CFG   (MUX_PAD_CTRL(DIO_PDOWN_PAD_CTRL) | MUX_MODE_SION)

#define DIO_PUP_PAD_CTRL  (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_100K_UP | PAD_CTL_SPEED_MED |		\
	PAD_CTL_DSE_40ohm | PAD_CTL_HYS | PAD_CTL_SRE_FAST)
#define DIO_PUP_PAD_CFG   (MUX_PAD_CTRL(DIO_PUP_PAD_CTRL) | MUX_MODE_SION)

#define GPMI_PAD_CTRL0 (PAD_CTL_PKE | PAD_CTL_PUE | PAD_CTL_PUS_100K_UP)
#define GPMI_PAD_CTRL1 (PAD_CTL_DSE_40ohm | PAD_CTL_SPEED_MED | \
			PAD_CTL_SRE_FAST)
#define GPMI_PAD_CTRL2 (GPMI_PAD_CTRL0 | GPMI_PAD_CTRL1)

#define LCD_PAD_CTRL    (PAD_CTL_HYS | PAD_CTL_PUS_100K_UP | PAD_CTL_PUE | \
	PAD_CTL_PKE | PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm)

#define ENET_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_PUE |     \
	PAD_CTL_SPEED_HIGH   |                                  \
	PAD_CTL_DSE_48ohm   | PAD_CTL_SRE_FAST)

#define MDIO_PAD_CTRL  (PAD_CTL_PUS_100K_UP | PAD_CTL_PUE |     \
	PAD_CTL_DSE_48ohm   | PAD_CTL_SRE_FAST | PAD_CTL_ODE)

#define ENET_CLK_PAD_CTRL  (PAD_CTL_DSE_40ohm   | PAD_CTL_SRE_FAST)

#define OTG_ID_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_47K_UP  | PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define USDHC_PAD_CTRL (PAD_CTL_PKE | PAD_CTL_PUE |		\
	PAD_CTL_PUS_22K_UP  | PAD_CTL_SPEED_LOW |		\
	PAD_CTL_DSE_80ohm   | PAD_CTL_SRE_FAST  | PAD_CTL_HYS)

#define ADC_PAD_CTRL (PAD_CTL_DSE_40ohm | PAD_CTL_SPEED_MED)

#define AUDIO_PAD_CTRL (PAD_CTL_PUS_100K_DOWN | 	\
	PAD_CTL_SPEED_MED | PAD_CTL_DSE_40ohm | 		\
	PAD_CTL_SRE_SLOW | PAD_CTL_HYS)

#define NO_PAD_CTRL_SION_CFG	(MUX_PAD_CTRL(NO_PAD_CTRL) | MUX_MODE_SION)

// CN8 rs232/rs485
static iomux_v3_cfg_t const uart1_pads[] = {
	MX6_PAD_UART1_TX_DATA__UART1_DCE_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_UART1_RX_DATA__UART1_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_UART1_CTS_B__GPIO1_IO18 	| DIO_PDOWN_PAD_CFG,
	MX6_PAD_UART1_RTS_B__GPIO1_IO19 	| DIO_PDOWN_PAD_CFG,
};

//wifi/bt
static iomux_v3_cfg_t const uart2_pads[] = {
	MX6_PAD_NAND_DATA04__UART2_DCE_TX 	 | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_NAND_DATA05__UART2_DCE_RX 	 | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_NAND_DATA06__UART2_DCE_CTS 	 | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_NAND_DATA07__UART2_DCE_RTS 	 | MUX_PAD_CTRL(UART_PAD_CTRL),
};

//debug
static iomux_v3_cfg_t const uart3_pads[] = {
	MX6_PAD_NAND_READY_B__UART3_DCE_TX  | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_UART3_RX_DATA__UART3_DCE_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

/* ULZ spinor  */
static iomux_v3_cfg_t const ecspi2_pads_ulz[] = {
	MX6_PAD_CSI_DATA00__ECSPI2_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_CSI_DATA01__GPIO4_IO22  | MUX_PAD_CTRL(DIO_PUP_PAD_CTRL), //CS
	MX6_PAD_CSI_DATA02__ECSPI2_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_CSI_DATA03__ECSPI2_MISO | MUX_PAD_CTRL(SPI_PAD_CTRL),
};

/* ULL UL spinor  */
static iomux_v3_cfg_t const ecspi3_pads[] = {
	MX6_PAD_UART2_TX_DATA__GPIO1_IO20 	| MUX_PAD_CTRL(DIO_PUP_PAD_CTRL), // CS
	MX6_PAD_UART2_RX_DATA__ECSPI3_SCLK 	| MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_UART2_CTS_B__ECSPI3_MOSI 	| MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_UART2_RTS_B__ECSPI3_MISO 	| MUX_PAD_CTRL(SPI_PAD_CTRL),
};

/* GPIOs */
static iomux_v3_cfg_t const misc_gpio_pads[] = {
	MX6_PAD_SNVS_TAMPER0__GPIO5_IO00 	| DIO_PUP_PAD_CFG, 					// PMIC_nINT - GPIO 128
	MX6_PAD_SNVS_TAMPER7__GPIO5_IO07	| DIO_PDOWN_PAD_CFG , 				// RTC_3V3_nINT - GPIO 135
	MX6_PAD_SNVS_TAMPER8__GPIO5_IO08	| DIO_PDOWN_PAD_CFG , 				// UART1_3V3_nRS232_RS485 - GPIO 136
	MX6_PAD_SNVS_TAMPER9__GPIO5_IO09	| MUX_PAD_CTRL(DIO_PUP_PAD_CTRL),	// UART1_3V3_RS485-ECHO - GPIO 137

	MX6_PAD_SNVS_TAMPER3__GPIO5_IO03 	| MUX_PAD_CTRL(DIO_PDOWN_PAD_CTRL), // SPINOR-WP - GPIO 131
	MX6_PAD_SNVS_TAMPER2__GPIO5_IO02	| MUX_PAD_CTRL(DIO_PUP_PAD_CTRL),   // EEP_WP - GPIO 130

	MX6_PAD_GPIO1_IO05__GPIO1_IO05		| MUX_PAD_CTRL(DIO_PDOWN_PAD_CTRL), // GPIO1_IO05
	MX6_PAD_GPIO1_IO08__GPIO1_IO08		| MUX_PAD_CTRL(DIO_PDOWN_PAD_CTRL), // GPIO1_IO08
	MX6_PAD_GPIO1_IO09__GPIO1_IO09		| MUX_PAD_CTRL(DIO_PDOWN_PAD_CTRL), // GPIO1_IO09
};

static iomux_v3_cfg_t const touch_pads[] = {
	MX6_PAD_NAND_CE0_B__GPIO4_IO13		| DIO_PUP_PAD_CFG, 					// TS_nINT - GPIO 109
	MX6_PAD_NAND_CE1_B__GPIO4_IO14 		| MUX_PAD_CTRL(DIO_PDOWN_PAD_CTRL), // TS_PWR_EN - GPIO 110
	MX6_PAD_NAND_WP_B__GPIO4_IO11  		| MUX_PAD_CTRL(DIO_PDOWN_PAD_CTRL), // TS_nRST - GPIO 107
};

static iomux_v3_cfg_t const wdog_pads[] = {
	MX6_PAD_LCD_RESET__WDOG1_WDOG_ANY	| MUX_PAD_CTRL(DIO_PUP_PAD_CTRL),
};

static iomux_v3_cfg_t const wifi_ATWIL3000_pads[] = {
	MX6_PAD_SNVS_TAMPER4__GPIO5_IO04 | DIO_PDOWN_PAD_CFG, // WMOD_3V3_nRST - GPIO 132
	MX6_PAD_SNVS_TAMPER6__GPIO5_IO06 | DIO_PDOWN_PAD_CFG, // WMOD_3V3_EN   - GPIO 134
	MX6_PAD_SNVS_TAMPER5__GPIO5_IO05 | DIO_PUP_PAD_CFG,   // WMOD_3V3_nINT - GPIO 133
};

static iomux_v3_cfg_t const lcd_pads[] = {
	MX6_PAD_LCD_CLK__LCDIF_CLK 			| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_ENABLE__LCDIF_ENABLE 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_HSYNC__LCDIF_HSYNC 		| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_VSYNC__LCDIF_VSYNC 		| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA00__LCDIF_DATA00 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA01__LCDIF_DATA01 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA02__LCDIF_DATA02 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA03__LCDIF_DATA03 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA04__LCDIF_DATA04 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA05__LCDIF_DATA05 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA06__LCDIF_DATA06 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA07__LCDIF_DATA07 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA08__LCDIF_DATA08 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA09__LCDIF_DATA09 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA10__LCDIF_DATA10 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA11__LCDIF_DATA11 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA12__LCDIF_DATA12	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA13__LCDIF_DATA13 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA14__LCDIF_DATA14 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA15__LCDIF_DATA15 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA16__LCDIF_DATA16 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA17__LCDIF_DATA17 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA18__LCDIF_DATA18 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA19__LCDIF_DATA19 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA20__LCDIF_DATA20 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA21__LCDIF_DATA21 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA22__LCDIF_DATA22 	| MUX_PAD_CTRL(LCD_PAD_CTRL),
	MX6_PAD_LCD_DATA23__LCDIF_DATA23 	| MUX_PAD_CTRL(LCD_PAD_CTRL),

	MX6_PAD_ENET2_TX_CLK__GPIO2_IO14	| DIO_PDOWN_PAD_CFG, 				// LCD_3V3_PWREN - GPIO 46
	MX6_PAD_NAND_DQS__PWM5_OUT 			| MUX_PAD_CTRL(DIO_PDOWN_PAD_CTRL), // BKL_PWM - PWM5 - GPIO 112
};

static iomux_v3_cfg_t const fec1_pads[] = {
	MX6_PAD_GPIO1_IO06__ENET1_MDIO 			| MUX_PAD_CTRL(MDIO_PAD_CTRL),
	MX6_PAD_GPIO1_IO07__ENET1_MDC 			| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET1_TX_DATA0__ENET1_TDATA00 	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET1_TX_DATA1__ENET1_TDATA01 	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET1_TX_EN__ENET1_TX_EN 		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET1_TX_CLK__ENET1_REF_CLK1 	| MUX_PAD_CTRL(ENET_CLK_PAD_CTRL),
	MX6_PAD_ENET1_RX_DATA0__ENET1_RDATA00 	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET1_RX_DATA1__ENET1_RDATA01 	| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET1_RX_ER__ENET1_RX_ER 		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET1_RX_EN__ENET1_RX_EN 		| MUX_PAD_CTRL(ENET_PAD_CTRL),
	MX6_PAD_ENET2_RX_ER__GPIO2_IO15			| MUX_PAD_CTRL(DIO_PUP_PAD_CTRL), 	// ENET_nRST
	MX6_PAD_SNVS_TAMPER1__GPIO5_IO01 		| DIO_PUP_PAD_CFG, 					// ENET_nINT
};

static iomux_v3_cfg_t const usb_pads[] = {
	MX6_PAD_ENET2_TX_DATA1__GPIO2_IO12	  | MUX_PAD_CTRL(DIO_PDOWN_PAD_CTRL), // USB_OTG2_PWR
	MX6_PAD_ENET2_TX_EN__GPIO2_IO13		  | DIO_PUP_PAD_CFG,				  // USB_OTG2_nOC
	MX6_PAD_ENET2_RX_DATA1__GPIO2_IO09	  | DIO_PUP_PAD_CFG, 				  // USB_OTG1_nOC
	MX6_PAD_ENET2_RX_DATA0__GPIO2_IO08    | MUX_PAD_CTRL(DIO_PDOWN_PAD_CTRL), // USB_OTG1_PWR
	MX6_PAD_UART3_TX_DATA__ANATOP_OTG1_ID | MUX_PAD_CTRL(OTG_ID_PAD_CTRL), 	  // USB_OTG1_ID
};

//wifi
static iomux_v3_cfg_t const sd2_pads[] = {
	MX6_PAD_NAND_RE_B__USDHC2_CLK 	    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_WE_B__USDHC2_CMD 	    | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA00__USDHC2_DATA0 	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA01__USDHC2_DATA1 	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA02__USDHC2_DATA2 	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_NAND_DATA03__USDHC2_DATA3 	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

//emmc uSD
static iomux_v3_cfg_t const sd1_pads[] = {
	MX6_PAD_NAND_CLE__GPIO4_IO15		| DIO_PDOWN_PAD_CFG, // SD1_3V3_nCD - GPIO
	MX6_PAD_SD1_CLK__USDHC1_CLK 		| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_CMD__USDHC1_CMD 		| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DATA0__USDHC1_DATA0 	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DATA1__USDHC1_DATA1 	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DATA2__USDHC1_DATA2 	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX6_PAD_SD1_DATA3__USDHC1_DATA3 	| MUX_PAD_CTRL(USDHC_PAD_CTRL),
};

/* PWM3 for buzzer */
static iomux_v3_cfg_t const buzzer_pads[] = {
		MX6_PAD_NAND_ALE__PWM3_OUT | 	MUX_PAD_CTRL(DIO_PDOWN_PAD_CTRL), // PWM3_OUT - BUZZER_PWM
};

/* I2C1 for PMIC and EEPROM */
static struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode =  MX6_PAD_UART4_TX_DATA__I2C1_SCL 	| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6_PAD_UART4_TX_DATA__GPIO1_IO28 	| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(1, 28),
	},
	.sda = {
		.i2c_mode = MX6_PAD_UART4_RX_DATA__I2C1_SDA 	| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6_PAD_UART4_RX_DATA__GPIO1_IO29 	| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(1, 29),
	},
};

/* I2C4 for display and ssc */
static struct i2c_pads_info i2c_pad_info4 = {
	.scl = {
		.i2c_mode =  MX6_PAD_ENET2_RX_EN__I2C4_SCL 			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6_PAD_ENET2_RX_EN__GPIO2_IO10 		| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(2, 10),
	},
	.sda = {
		.i2c_mode = MX6_PAD_ENET2_TX_DATA0__I2C4_SDA 		| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6_PAD_ENET2_TX_DATA0__GPIO2_IO11 	| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(2, 11),
	},
};

static void my_uart_init_mux(void)
{
	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
	imx_iomux_v3_setup_multiple_pads(uart2_pads, ARRAY_SIZE(uart2_pads));
	imx_iomux_v3_setup_multiple_pads(uart3_pads, ARRAY_SIZE(uart3_pads));
}

static void my_i2c_init_mux(void)
{
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);
	setup_i2c(3, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info4);
}

static void my_gpio_init_mux(void)
{
	imx_iomux_v3_setup_multiple_pads(misc_gpio_pads, ARRAY_SIZE(misc_gpio_pads));
}

static void my_spinor_init_mux(void)
{
#if 1 //ULL UL
	imx_iomux_v3_setup_multiple_pads(ecspi3_pads, ARRAY_SIZE(ecspi3_pads));
#else //ULZ
	imx_iomux_v3_setup_multiple_pads(ecspi2_pads_ulz, ARRAY_SIZE(ecspi2_pads_ulz));
#endif
}

static void my_lcd_init_mux(void)
{
	gpio_direction_output(BKL_DISPLAY_PWM_GPIO, 0);
	imx_iomux_v3_setup_multiple_pads(lcd_pads, ARRAY_SIZE(lcd_pads));
}

static void my_fec1_init_mux(void)
{
	imx_iomux_v3_setup_multiple_pads(fec1_pads, ARRAY_SIZE(fec1_pads));
}

static void my_usb_init_mux(void)
{
	imx_iomux_v3_setup_multiple_pads(usb_pads, ARRAY_SIZE(usb_pads));
}

static void my_buzzer_init_mux(void)
{
	imx_iomux_v3_setup_multiple_pads(buzzer_pads, ARRAY_SIZE(buzzer_pads));
}

static void my_touchscreen_init_mux(void)
{
	imx_iomux_v3_setup_multiple_pads(touch_pads, ARRAY_SIZE(touch_pads));
}

static void my_emmc_init_mux(void)
{
	imx_iomux_v3_setup_multiple_pads(sd1_pads, ARRAY_SIZE(sd1_pads));
}

static void my_wdog_init_mux(void)
{
	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));
}

static void my_wifi_init_mux(void)
{
	imx_iomux_v3_setup_multiple_pads(wifi_ATWIL3000_pads, ARRAY_SIZE(wifi_ATWIL3000_pads));
	imx_iomux_v3_setup_multiple_pads(uart2_pads, ARRAY_SIZE(uart2_pads));
	imx_iomux_v3_setup_multiple_pads(sd2_pads, ARRAY_SIZE(sd2_pads));
}

//***************************
//* expansion connector CN1 *
//***************************
static struct i2c_pads_info i2c_pad_info2 = {
	.scl = {
		.i2c_mode =  MX6_PAD_GPIO1_IO00__I2C2_SCL 			| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6_PAD_GPIO1_IO00__GPIO1_IO00 		| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(1, 0),
	},
	.sda = {
		.i2c_mode = MX6_PAD_UART5_RX_DATA__I2C2_SDA 		| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gpio_mode = MX6_PAD_UART5_RX_DATA__GPIO1_IO31	 	| MUX_PAD_CTRL(I2C_PAD_CTRL),
		.gp = IMX_GPIO_NR(1, 31),
	},
};

static iomux_v3_cfg_t const ecspi1_pads[] = {
	MX6_PAD_CSI_DATA04__ECSPI1_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_CSI_DATA06__ECSPI1_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_CSI_DATA07__ECSPI1_MISO | MUX_PAD_CTRL(DIO_PDOWN_PAD_CTRL),
};

static iomux_v3_cfg_t const ecspi2_pads[] = {
	MX6_PAD_CSI_DATA00__ECSPI2_SCLK | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_CSI_DATA02__ECSPI2_MOSI | MUX_PAD_CTRL(SPI_PAD_CTRL),
	MX6_PAD_CSI_DATA03__ECSPI2_MISO | MUX_PAD_CTRL(DIO_PDOWN_PAD_CTRL),
};

static iomux_v3_cfg_t const cn1_gpio_pads[] = {
	MX6_PAD_GPIO1_IO04__GPIO1_IO04    | DIO_PUP_PAD_CFG,
	MX6_PAD_GPIO1_IO03__GPIO1_IO03    | DIO_PUP_PAD_CFG,
	MX6_PAD_GPIO1_IO02__GPIO1_IO02    | DIO_PUP_PAD_CFG,
	MX6_PAD_GPIO1_IO01__GPIO1_IO01    | DIO_PUP_PAD_CFG,
	MX6_PAD_JTAG_TRST_B__GPIO1_IO15	  | DIO_PUP_PAD_CFG,
	MX6_PAD_JTAG_TDO__GPIO1_IO12	  | DIO_PUP_PAD_CFG,
	MX6_PAD_JTAG_TMS__GPIO1_IO11	  | DIO_PUP_PAD_CFG, 	//SPI1_CS0
	MX6_PAD_CSI_DATA05__GPIO4_IO26    | DIO_PUP_PAD_CFG,    //SPI2_CS0
};

static iomux_v3_cfg_t const uart5_pads[] = {
	MX6_PAD_UART5_TX_DATA__UART5_DCE_TX	 | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI_DATA01__UART5_DCE_RX 	 | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_GPIO1_IO09__UART5_DCE_CTS 	 | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_GPIO1_IO08__UART5_DCE_RTS 	 | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const uart6_pads[] = {
	MX6_PAD_CSI_MCLK__UART6_DCE_TX		 | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI_PIXCLK__UART6_DCE_RX 	 | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI_HSYNC__UART6_DCE_CTS 	 | MUX_PAD_CTRL(UART_PAD_CTRL),
	MX6_PAD_CSI_VSYNC__UART6_DCE_RTS 	 | MUX_PAD_CTRL(UART_PAD_CTRL),
};

static iomux_v3_cfg_t const pwm_pads[] = {
	MX6_PAD_JTAG_TDI__PWM6_OUT | 	MUX_PAD_CTRL(DIO_PDOWN_PAD_CTRL), // PWM6
	MX6_PAD_JTAG_TCK__PWM7_OUT | 	MUX_PAD_CTRL(DIO_PDOWN_PAD_CTRL), // PWM7
};

static iomux_v3_cfg_t const flexcan1_pads[] = {
	MX6_PAD_UART3_RTS_B__FLEXCAN1_RX | NO_PAD_CTRL_SION_CFG,
	MX6_PAD_UART3_CTS_B__FLEXCAN1_TX | NO_PAD_CTRL_SION_CFG,
};
static iomux_v3_cfg_t const adc_pads[] = {
	MX6_PAD_GPIO1_IO05__GPIO1_IO05  | MUX_PAD_CTRL(ADC_PAD_CTRL),
};

static void expansion_connector_init_mux_ull(void)
{
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info2);
	imx_iomux_v3_setup_multiple_pads(cn1_gpio_pads, ARRAY_SIZE(cn1_gpio_pads));
	imx_iomux_v3_setup_multiple_pads(ecspi1_pads, ARRAY_SIZE(ecspi1_pads));
	imx_iomux_v3_setup_multiple_pads(ecspi2_pads, ARRAY_SIZE(ecspi2_pads));
	imx_iomux_v3_setup_multiple_pads(uart5_pads, ARRAY_SIZE(uart5_pads));
	imx_iomux_v3_setup_multiple_pads(uart6_pads, ARRAY_SIZE(uart6_pads));
	imx_iomux_v3_setup_multiple_pads(pwm_pads, ARRAY_SIZE(pwm_pads));
	imx_iomux_v3_setup_multiple_pads(flexcan1_pads, ARRAY_SIZE(flexcan1_pads));
	imx_iomux_v3_setup_multiple_pads(adc_pads, ARRAY_SIZE(adc_pads));
}

void egf_board_mux_init(int mode)
{
	switch(mode){
	case PROGRAMMER_MUX_MODE:
		my_uart_init_mux();
		my_i2c_init_mux();
		my_spinor_init_mux();
		my_lcd_init_mux();
		my_fec1_init_mux();
		break;
	case APPLICATION_MUX_MODE:
		my_uart_init_mux();
		my_i2c_init_mux();
		my_gpio_init_mux();
		my_spinor_init_mux();
		my_lcd_init_mux();
		my_fec1_init_mux();
		my_usb_init_mux();
		my_buzzer_init_mux();
		my_touchscreen_init_mux();
		my_emmc_init_mux();
		my_wdog_init_mux();
		my_wifi_init_mux();
		expansion_connector_init_mux_ull();
		break;
	default:
		printf("MUX MODE NOT DEFINED!!!!!!!!!!!!\n");
		break;
	}
}


