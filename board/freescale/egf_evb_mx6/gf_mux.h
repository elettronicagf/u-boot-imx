#ifndef _GF_MUX_H_
#define _GF_MUX_H_
#define PROGRAMMER_MUX_MODE	 	 1
#define APPLICATION_MUX_MODE	 2

void egf_board_mux_init(int mode);
#define SPINOR_WP_GPIO 				IMX_GPIO_NR(3, 8)
#define SD1_CD_GPIO					IMX_GPIO_NR(6, 8)
#define DISP0_EN					IMX_GPIO_NR(5, 5)
#define GPIO_DISPLAY_NRESET			IMX_GPIO_NR(5, 31)
#define USB_H1_PWR_EN_GPIO			IMX_GPIO_NR(2, 1)
#define USB_OTG_PWR_EN_GPIO			IMX_GPIO_NR(2, 5)
#endif
