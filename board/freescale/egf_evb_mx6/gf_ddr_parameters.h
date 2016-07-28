#ifndef GF_DDR_PARAMS
#define GF_DDR_PARAMS
#include <asm/arch/mx6-ddr.h>

/* configure MX6Q/DUAL mmdc DDR io registers */
struct mx6dq_iomux_ddr_regs mx6dq_ddr_ioregs_standard = {
	/* SDCLK[0:1], CAS, RAS, Reset: Differential input, 40ohm */
	.dram_sdclk_0 = 0x00000028,
	.dram_sdclk_1 = 0x00000028,
	.dram_cas = 0x00000028,
	.dram_ras = 0x00000028,
	.dram_reset = 0x00000028,
	/* SDBA2: pull-up disabled */
	.dram_sdba2 = 0x00000000,
	/* SDODT[0:1]: 100k pull-up, 40 ohm */
	.dram_sdodt0 = 0x00000028,
	.dram_sdodt1 = 0x00000028,
	/* SDQS[0:7]: Differential input, 40 ohm */
	.dram_sdqs0 = 0x00000028,
	.dram_sdqs1 = 0x00000028,
	.dram_sdqs2 = 0x00000028,
	.dram_sdqs3 = 0x00000028,
	.dram_sdqs4 = 0x00000028,
	.dram_sdqs5 = 0x00000028,
	.dram_sdqs6 = 0x00000028,
	.dram_sdqs7 = 0x00000028,
	/* DQM[0:7]: Differential input, 40 ohm */
	.dram_dqm0 = 0x00000028,
	.dram_dqm1 = 0x00000028,
	.dram_dqm2 = 0x00000028,
	.dram_dqm3 = 0x00000028,
	.dram_dqm4 = 0x00000028,
	.dram_dqm5 = 0x00000028,
	.dram_dqm6 = 0x00000028,
	.dram_dqm7 = 0x00000028,
};

/* configure MX6Q/DUAL mmdc GRP io registers */
struct mx6dq_iomux_grp_regs mx6dq_grp_ioregs_standard = {
	/* DDR3 */
	.grp_ddr_type = 0x000c0000,
	.grp_ddrmode_ctl = 0x00020000,
	/* disable DDR pullups */
	.grp_ddrpke = 0x00000000,
	/* ADDR[00:16], SDBA[0:1]: 40 ohm */
	.grp_addds = 0x00000028,
	/* CS0/CS1/SDBA2/CKE0/CKE1/SDWE: 40 ohm */
	.grp_ctlds = 0x00000028,
	/* DATA[00:63]: Differential input, 40 ohm */
	.grp_ddrmode = 0x00020000,
	.grp_b0ds = 0x00000028,
	.grp_b1ds = 0x00000028,
	.grp_b2ds = 0x00000028,
	.grp_b3ds = 0x00000028,
	.grp_b4ds = 0x00000028,
	.grp_b5ds = 0x00000028,
	.grp_b6ds = 0x00000028,
	.grp_b7ds = 0x00000028,
};

/* configure MX6SOLO/DUALLITE mmdc DDR io registers */
struct mx6sdl_iomux_ddr_regs mx6sdl_ddr_ioregs_standard = {
	/* SDCLK[0:1], CAS, RAS, Reset: Differential input, 40ohm */
	.dram_sdclk_0 = 0x00000028,
	.dram_sdclk_1 = 0x00000028,
	.dram_cas = 0x00000028,
	.dram_ras = 0x00000028,
	.dram_reset = 0x00000028,
	/* SDBA2: pull-up disabled */
	.dram_sdba2 = 0x00000000,
	/* SDODT[0:1]: 100k pull-up, 40 ohm */
	.dram_sdodt0 = 0x00000028,
	.dram_sdodt1 = 0x00000028,
	/* SDQS[0:7]: Differential input, 40 ohm */
	.dram_sdqs0 = 0x00000028,
	.dram_sdqs1 = 0x00000028,
	.dram_sdqs2 = 0x00000028,
	.dram_sdqs3 = 0x00000028,
	.dram_sdqs4 = 0x00000028,
	.dram_sdqs5 = 0x00000028,
	.dram_sdqs6 = 0x00000028,
	.dram_sdqs7 = 0x00000028,
	/* DQM[0:7]: Differential input, 40 ohm */
	.dram_dqm0 = 0x00000028,
	.dram_dqm1 = 0x00000028,
	.dram_dqm2 = 0x00000028,
	.dram_dqm3 = 0x00000028,
	.dram_dqm4 = 0x00000028,
	.dram_dqm5 = 0x00000028,
	.dram_dqm6 = 0x00000028,
	.dram_dqm7 = 0x00000028,
};

/* configure MX6SOLO/DUALLITE mmdc GRP io registers */
struct mx6sdl_iomux_grp_regs mx6sdl_grp_ioregs_standard = {
	/* DDR3 */
	.grp_ddr_type = 0x000c0000,
	/* SDQS[0:7]: Differential input, 40 ohm */
	.grp_ddrmode_ctl = 0x00020000,
	/* disable DDR pullups */
	.grp_ddrpke = 0x00000000,
	/* ADDR[00:16], SDBA[0:1]: 40 ohm */
	.grp_addds = 0x00000028,
	/* CS0/CS1/SDBA2/CKE0/CKE1/SDWE: 40 ohm */
	.grp_ctlds = 0x00000028,
	/* DATA[00:63]: Differential input, 40 ohm */
	.grp_ddrmode = 0x00020000,
	.grp_b0ds = 0x00000028,
	.grp_b1ds = 0x00000028,
	.grp_b2ds = 0x00000028,
	.grp_b3ds = 0x00000028,
	.grp_b4ds = 0x00000028,
	.grp_b5ds = 0x00000028,
	.grp_b6ds = 0x00000028,
	.grp_b7ds = 0x00000028,
};

/* MT41K128M16JT-125 (2Gb density) */
static struct mx6_ddr3_cfg mt41k128m16jt_125 = {
	.mem_speed = 1600,
	.density = 2,
	.width = 16,
	.banks = 8,
	.rowaddr = 14,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
};

/* MT41K128M16JT-125 (4Gb density) */
static struct mx6_ddr3_cfg mt41k256m16ha_125 = {
	.mem_speed = 1600,
	.density = 4,
	.width = 16,
	.banks = 8,
	.rowaddr = 15,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
};


/*
 * calibration - these are the various CPU/DDR3 combinations we support
 */

static struct mx6_mmdc_calibration mx6sdl_128x16_mmdc_calib_default = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x00470049,
	.p0_mpwldectrl1 = 0x003C0042,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x02300230,
	.p0_mpdgctrl1 = 0x021C0220,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x40424844,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x36322A32,
};

static struct mx6_mmdc_calibration mx6sdl_128x16_mmdc_calib_x64 = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x003C0042,
	.p0_mpwldectrl1 = 0x00300036,
	.p1_mpwldectrl0 = 0x001D0026,
	.p1_mpwldectrl1 = 0x0018002E,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x0224021C,
	.p0_mpdgctrl1 = 0x020C021C,
	.p1_mpdgctrl0 = 0x02080210,
	.p1_mpdgctrl1 = 0x02080210,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x40444642,
	.p1_mprddlctl = 0x46484A44,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x34322C30,
	.p1_mpwrdlctl = 0x38343230,
};

static struct mx6_mmdc_calibration mx6dq_128x16_mmdc_calib_default = {
	/* write leveling calibration determine */
	.p0_mpwldectrl0 = 0x00160016,
	.p0_mpwldectrl1 = 0x001A000F,
	/* Read DQS Gating calibration */
	.p0_mpdgctrl0 = 0x03180328,
	.p0_mpdgctrl1 = 0x03180318,
	/* Read Calibration: DQS delay relative to DQ read access */
	.p0_mprddlctl = 0x3A283030,
	/* Write Calibration: DQ/DM delay relative to DQS write access */
	.p0_mpwrdlctl = 0x36323C3E,
};


#endif
