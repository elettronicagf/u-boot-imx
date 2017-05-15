#ifndef _GF_MUX_H_
#define _GF_MUX_H_
#define PROGRAMMER_MUX_MODE	 	 1
#define APPLICATION_MUX_MODE	 2

#define EEPROM_nWP_GPIO IMX_GPIO_NR(5,1)
#define USDHC2_CD_GPIO IMX_GPIO_NR(5,9)
void egf_board_mux_init(int mode);
#endif
