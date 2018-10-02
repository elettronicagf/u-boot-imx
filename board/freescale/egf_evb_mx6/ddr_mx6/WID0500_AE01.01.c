#include <asm/io.h>
#define write_reg(X,Y) __raw_writel(Y,X)
void WID0500_AE01_01_ddr_setup(void){

write_reg( 0x020c4068, 0xffffffff);
write_reg( 0x020c406c, 0xffffffff);
write_reg( 0x020c4070, 0xffffffff);
write_reg( 0x020c4074, 0xffffffff);
write_reg( 0x020c4078, 0xffffffff);
write_reg( 0x020c407c, 0xffffffff);
write_reg( 0x020c4080, 0xffffffff);
write_reg( 0x020c4084, 0xffffffff);

write_reg( 0x020e0798, 0x00080000	);//IOMUXC_SW_PAD_CTL_GRP_DDR_TYPE
write_reg( 0x020e0758, 0x00000000	);//IOMUXC_SW_PAD_CTL_GRP_DDRPKE
write_reg( 0x020e0588, 0x00000030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_SDCLK_0
write_reg( 0x020e0594, 0x00000030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_SDCLK_1
write_reg( 0x020e056c, 0x00000030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_CAS
write_reg( 0x020e0578, 0x00000030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_RAS
write_reg( 0x020e074c, 0x00000030	);//IOMUXC_SW_PAD_CTL_GRP_ADDDS
write_reg( 0x020e057c, 0x00000030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_RESET
write_reg( 0x020e058c, 0x00000000	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_SDBA2 - DSE can be configured using Group Control Register: IOMUXC_SW_PAD_CTL_GRP_CTLDS
write_reg( 0x020e059c, 0x00000030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_SDODT0
write_reg( 0x020e05a0, 0x00000030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_SDODT1
write_reg( 0x020e078c, 0x00000030	);//IOMUXC_SW_PAD_CTL_GRP_CTLDS
write_reg( 0x020e0750, 0x00020000	);//IOMUXC_SW_PAD_CTL_GRP_DDRMODE_CTL
write_reg( 0x020e05a8, 0x00003030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS0
write_reg( 0x020e05b0, 0x00003030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS1
write_reg( 0x020e0524, 0x00003030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS2
write_reg( 0x020e051c, 0x00003030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS3
write_reg( 0x020e0518, 0x00003030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS4
write_reg( 0x020e050c, 0x00003030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS5
write_reg( 0x020e05b8, 0x00003030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS6
write_reg( 0x020e05c0, 0x00003030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_SDQS7
write_reg( 0x020e0774, 0x00020000	);//IOMUXC_SW_PAD_CTL_GRP_DDRMODE
write_reg( 0x020e0784, 0x00000030	);//IOMUXC_SW_PAD_CTL_GRP_B0DS
write_reg( 0x020e0788, 0x00000030	);//IOMUXC_SW_PAD_CTL_GRP_B1DS
write_reg( 0x020e0794, 0x00000030	);//IOMUXC_SW_PAD_CTL_GRP_B2DS
write_reg( 0x020e079c, 0x00000030	);//IOMUXC_SW_PAD_CTL_GRP_B3DS
write_reg( 0x020e07a0, 0x00000030	);//IOMUXC_SW_PAD_CTL_GRP_B4DS
write_reg( 0x020e07a4, 0x00000030	);//IOMUXC_SW_PAD_CTL_GRP_B5DS
write_reg( 0x020e07a8, 0x00000030	);//IOMUXC_SW_PAD_CTL_GRP_B6DS
write_reg( 0x020e0748, 0x00000030	);//IOMUXC_SW_PAD_CTL_GRP_B7DS
write_reg( 0x020e05ac, 0x00000030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM0
write_reg( 0x020e05b4, 0x00000030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM1
write_reg( 0x020e0528, 0x00000030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM2
write_reg( 0x020e0520, 0x00000030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM3
write_reg( 0x020e0514, 0x00000030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM4
write_reg( 0x020e0510, 0x00000030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM5
write_reg( 0x020e05bc, 0x00000030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM6
write_reg( 0x020e05c4, 0x00000030	);//IOMUXC_SW_PAD_CTL_PAD_DRAM_DQM7
write_reg( 0x021b001c, 0x00008000	);//MMDC0_MDSCR, set the Configuration request bit during MMDC set up
write_reg( 0x021b401c, 0x00008000	);//MMDC1_MDSCR, set the Configuration request bit during MMDC set up
write_reg( 0x021b085c, 0x1B4700C7	);//MMDC0_MPZQLP2CTL,LPDDR2 ZQ params
write_reg( 0x021b485c, 0x1B4700C7	);//MMDC1_MPZQLP2CTL,LPDDR2 ZQ params
write_reg( 0x021b0800, 0xA1390003	);//DDR_PHY_P0_MPZQHWCTRL, enable both one-time & periodic HW ZQ calibration.
write_reg( 0x021b0890, 0x00400000	);//values of 20,40,50,60,7f tried. no difference seen
write_reg( 0x021b4890, 0x00400000	);//values of 20,40,50,60,7f tried. no difference seen
write_reg( 0x021b0848, 0x3E3C3E40	);//MPRDDLCTL PHY0
write_reg( 0x021b4848, 0x3C3C3A44	);//MPRDDLCTL PHY1
write_reg( 0x021b0850, 0x3A384038	);//MPWRDLCTL PHY0
write_reg( 0x021b4850, 0x4232403C	);//MPWRDLCTL PHY1
write_reg( 0x021b083c, 0x20000000	);
write_reg( 0x021b0840, 0x00000000	);
write_reg( 0x021b483c, 0x20000000	);
write_reg( 0x021b4840, 0x00000000	);
write_reg( 0x021b081c, 0x33333333	);//DDR_PHY_P0_MPREDQBY0DL3
write_reg( 0x021b0820, 0x33333333	);//DDR_PHY_P0_MPREDQBY1DL3
write_reg( 0x021b0824, 0x33333333	);//DDR_PHY_P0_MPREDQBY2DL3
write_reg( 0x021b0828, 0x33333333	);//DDR_PHY_P0_MPREDQBY3DL3
write_reg( 0x021b481c, 0x33333333	);//DDR_PHY_P1_MPREDQBY0DL3
write_reg( 0x021b4820, 0x33333333	);//DDR_PHY_P1_MPREDQBY1DL3
write_reg( 0x021b4824, 0x33333333	);//DDR_PHY_P1_MPREDQBY2DL3
write_reg( 0x021b4828, 0x33333333	);//DDR_PHY_P1_MPREDQBY3DL3
write_reg( 0x021b082c, 0xF3333333	);//DDR_PHY_P0_MPREDQBY0DL3
write_reg( 0x021b0830, 0xF3333333	);//DDR_PHY_P0_MPREDQBY1DL3
write_reg( 0x021b0834, 0xF3333333	);//DDR_PHY_P0_MPREDQBY2DL3
write_reg( 0x021b0838, 0xF3333333	);//DDR_PHY_P0_MPREDQBY3DL3
write_reg( 0x021b482c, 0xF3333333	);//DDR_PHY_P1_MPREDQBY0DL3
write_reg( 0x021b4830, 0xF3333333	);//DDR_PHY_P1_MPREDQBY1DL3
write_reg( 0x021b4834, 0xF3333333	);//DDR_PHY_P1_MPREDQBY2DL3
write_reg( 0x021b4838, 0xF3333333	);//DDR_PHY_P1_MPREDQBY3DL3
write_reg( 0x021b08b8, 0x00000800	);//DDR_PHY_P0_MPMUR0, frc_msr
write_reg( 0x021b48b8, 0x00000800	);//DDR_PHY_P0_MPMUR0, frc_msr
write_reg( 0x021b0004, 0x00020036	);//MMDC0_MDPDC
write_reg( 0x021b0008, 0x00000000	);//MMDC0_MDOTC
write_reg( 0x021b000c, 0x53574133	);//MMDC0_MDCFG0
write_reg( 0x021b0010, 0x00100A82	);//MMDC0_MDCFG1
write_reg( 0x021b0014, 0x00000093	);//MMDC0_MDCFG2
write_reg( 0x021b0018, 0x0000174C	);//MMDC0_MDMISC
write_reg( 0x021b001c, 0x00008000	);//MMDC0_MDSCR, set the Configuration request bit during MMDC set up
write_reg( 0x021b002c, 0x0F9F26D2	);//MMDC0_MDRWD
write_reg( 0x021b0030, 0x00000010	);//MMDC0_MDOR
write_reg( 0x021b0038, 0x00190778	);//MMDC0_MDCFG3LP
write_reg( 0x021b0040, 0x00000063	);//Chan0 CS0_END
write_reg( 0x021b0400, 0x11420000	);//MMDC0_MAARCR ADOPT optimized priorities. Dyn jump disabled
write_reg( 0x021b0000, 0x83110000	);//MMDC0_MDCTL
write_reg( 0x021b4004, 0x00020036	);//MMDC1_MDPDC
write_reg( 0x021b4008, 0x00000000	);//MMDC1_MDOTC
write_reg( 0x021b400c, 0x53574133	);//MMDC1_MDCFG0
write_reg( 0x021b4010, 0x00100A82	);//MMDC1_MDCFG1
write_reg( 0x021b4014, 0x00000093	);//MMDC1_MDCFG2
write_reg( 0x021b4018, 0x0000174C	);//MMDC1_MDMISC
write_reg( 0x021b401c, 0x00008000	);//MMDC1_MDSCR, set the Configuration request bit during MMDC set up
write_reg( 0x021b402c, 0x0F9F26D2	);//MMDC1_MDRWD
write_reg( 0x021b4030, 0x00000010	);//MMDC1_MDOR
write_reg( 0x021b4038, 0x00190778	);//MMDC1_MDCFG3LP
write_reg( 0x021b4040, 0x00000023	);//Chan1 CS0_END
write_reg( 0x021b4400, 0x11420000	);//MMDC1_MAARCR ADOPT optimized priorities. Dyn jump disabled
write_reg( 0x021b4000, 0x83110000	);//MMDC1_MDCTL
write_reg( 0x021b001c, 0x003F8030	);//MRW: BA=0 CS=0 MR_ADDR=63 MR_OP=0
write_reg( 0x021b001c, 0xFF0A8030	);//MRW: BA=0 CS=0 MR_ADDR=10 MR_OP=ff
write_reg( 0x021b001c, 0x82018030	);//MRW: BA=0 CS=0 MR_ADDR=1  MR_OP=c2
write_reg( 0x021b001c, 0x04028030	);//MRW: BA=0 CS=0 MR_ADDR=2  MR_OP=6. tcl=8, tcwl=4
write_reg( 0x021b001c, 0x02038030	);//MRW: BA=0 CS=0 MR_ADDR=3  MR_OP=2.drive=240/6
write_reg( 0x021b401c, 0x003F8030	);//MRW: BA=0 CS=0 MR_ADDR=63 MR_OP=0
write_reg( 0x021b401c, 0xFF0A8030	);//MRW: BA=0 CS=0 MR_ADDR=10 MR_OP=ff
write_reg( 0x021b401c, 0x82018030	);//MRW: BA=0 CS=0 MR_ADDR=1  MR_OP=c2
write_reg( 0x021b401c, 0x04028030	);//MRW: BA=0 CS=0 MR_ADDR=2  MR_OP=6. tcl=8, tcwl=4
write_reg( 0x021b401c, 0x02038030	);//MRW: BA=0 CS=0 MR_ADDR=3  MR_OP=2.drive=240/6
write_reg( 0x021b0800, 0xA1390003	);//DDR_PHY_P0_MPZQHWCTRL, enable both one-time & periodic HW ZQ calibration.
write_reg( 0x021b0020, 0x00001800	);//MMDC0_MDREF
write_reg( 0x021b4020, 0x00001800	);//MMDC1_MDREF
write_reg( 0x021b0818, 0x00000000	);//DDR_PHY_P0_MPODTCTRL
write_reg( 0x021b4818, 0x00000000	);//DDR_PHY_P1_MPODTCTRL
write_reg( 0x021b0004, 0x00025576	);//MMDC0_MDPDC now SDCTL power down enabled
write_reg( 0x021b4004, 0x00025576	);//MMDC0_MDPDC now SDCTL power down enabled
write_reg( 0x021b0404, 0x00011006	);//MMDC0_MAPSR ADOPT power down enabled, MMDC will enter automatically to self-refresh while the number of idle cycle reached.
write_reg( 0x021b4404, 0x00011006	);//MMDC0_MAPSR ADOPT power down enabled, MMDC will enter automatically to self-refresh while the number of idle cycle reached.
write_reg( 0x021b001c, 0x00000000	);//MMDC0_MDSCR, clear this register (especially the configuration bit as initialization is complete)
write_reg( 0x021b401c, 0x00000000	);//MMDC0_MDSCR, clear this register (especially the configuration bit as initialization is complete)
}
