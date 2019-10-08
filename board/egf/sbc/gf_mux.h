#ifndef _GF_MUX_H_
#define _GF_MUX_H_
#define PROGRAMMER_MUX_MODE	 	 1
#define APPLICATION_MUX_MODE	 2

#define ENET_NRST_GPIO 			IMX_GPIO_NR(2, 15)
#define EEPROM_nWP_GPIO 		IMX_GPIO_NR(5, 2)
#define BKL_DISPLAY_PWM_GPIO 	IMX_GPIO_NR(4, 16)
#define SPINOR_SS0_GPIO_ULL		IMX_GPIO_NR(1, 20)
#define SPINOR_SS0_GPIO_ULZ		IMX_GPIO_NR(4, 22)
#define USB_OTG1_PWR_EN_GPIO 	IMX_GPIO_NR(2, 8)
#define USB_OTG2_PWR_EN_GPIO 	IMX_GPIO_NR(2, 12)
#define TS_nRST					IMX_GPIO_NR(4, 11)
#define TS_PWR_EN				IMX_GPIO_NR(4, 14)
#define LCD_POWER_ENABLE		IMX_GPIO_NR(2, 14)

void egf_board_mux_init(int mode);

#endif
