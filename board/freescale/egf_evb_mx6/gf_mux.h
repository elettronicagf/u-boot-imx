#ifndef _GF_MUX_H_
#define _GF_MUX_H_
#define PROGRAMMER_MUX_MODE	 	 1
#define APPLICATION_MUX_MODE	 2

void egf_board_mux_init(int mode);
#define SPINOR_WP_GPIO 				IMX_GPIO_NR(3, 8)
#define SD2_CD_GPIO					IMX_GPIO_NR(1, 4)
#define SD1_CD_GPIO					IMX_GPIO_NR(6, 4)
#define SD1_WP_GPIO					IMX_GPIO_NR(4, 20)
#define USB_OTG_POWER_ENABLE_GPIO	IMX_GPIO_NR(1, 6)
#define USB_H1_POWER_ENABLE_GPIO	IMX_GPIO_NR(1, 30)

#endif
