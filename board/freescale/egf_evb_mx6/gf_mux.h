#ifndef _GF_MUX_H_
#define _GF_MUX_H_
#define PROGRAMMER_MUX_MODE	 	 1
#define APPLICATION_MUX_MODE	 2

void egf_board_common_mux_init(int mode);
void pgf_0533_a01_mux(void);
void pgf_0533_a02_mux(void);
void pgf_0533_a03_mux(void);
void egf_wid0533ab0101_mux(void);
void egf_wid0533bc0101_mux(void);
#define SPINOR_WP_GPIO 				IMX_GPIO_NR(3, 8)
#define SD2_CD_GPIO					IMX_GPIO_NR(1, 4)
#define SD1_CD_GPIO					IMX_GPIO_NR(6, 4)
#define SD1_WP_GPIO					IMX_GPIO_NR(4, 20)
#define USB_OTG_POWER_ENABLE_GPIO	IMX_GPIO_NR(1, 6)
#define USB_H1_POWER_ENABLE_GPIO	IMX_GPIO_NR(1, 30)
/* PGF0533_A02 based boards only */
#define LCD_PWR_EN_GPIO				IMX_GPIO_NR(6, 6)

#endif
