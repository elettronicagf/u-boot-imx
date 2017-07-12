#ifndef _GF_MUX_H_
#define _GF_MUX_H_
#define PROGRAMMER_MUX_MODE	 	 1
#define APPLICATION_MUX_MODE	 2

#define EEPROM_nWP_GPIO IMX_GPIO_NR(5,9)
#define DISP_BL_CONTROL_PWM_GPIO IMX_GPIO_NR(1,1)
#define USDHC1_CD_GPIO IMX_GPIO_NR(5,0)
#define ENET1_NRST_GPIO IMX_GPIO_NR(6,16)
#define ENET2_NRST_GPIO IMX_GPIO_NR(6,14)

#define DISP_EN_GPIO IMX_GPIO_NR(3,4)
#define DISP_VDD_EN_GPIO IMX_GPIO_NR(2,4)

void egf_common_mux_init(void);
void egf_mux_init_wid0575_aa0101(void);
void egf_mux_init_wid0575_ab0101(void);
#endif
