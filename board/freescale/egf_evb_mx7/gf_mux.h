#ifndef _GF_MUX_H_
#define _GF_MUX_H_
#define PROGRAMMER_MUX_MODE	 	 1
#define APPLICATION_MUX_MODE	 2

#define EEPROM_nWP_GPIO IMX_GPIO_NR(5,1)
#define USDHC2_CD_GPIO IMX_GPIO_NR(5,9)
#define DISP_BL_CONTROL_PWM_GPIO IMX_GPIO_NR(1,2)
#define DISP_EN_GPIO IMX_GPIO_NR(3,4)
#define DISP_VDD_EN_GPIO IMX_GPIO_NR(2,4)
#define ENET_NRST_GPIO IMX_GPIO_NR(7,14)

void egf_board_mux_init(int mode);
#endif
