#ifndef GF_DDR_PARAMS
#define GF_DDR_PARAMS
#include <asm/arch/mx6-ddr.h>

/* MT41K128M16JT-125:K (2Gb density) */
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

//0659
/* K4B4G1646E-BYK0 (4Gb density) */
static struct mx6_ddr3_cfg k4b4g1646e_byk0 = {
	.mem_speed = 1600,
	.density = 4,
	.width = 16,
	.banks = 8,
	.rowaddr = 15,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1391,
	.trcmin = 4791,
	.trasmin = 3400,
};

/* MT41K64M16TW-107:J (1Gb density) */
static struct mx6_ddr3_cfg mt41k64m16tw_107 = {
	.mem_speed = 1600,
	.density = 1,
	.width = 16,
	.banks = 8,
	.rowaddr = 13,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1391,
	.trcmin = 4791,
	.trasmin = 3400,
};

/* MT41K512M16HA-125:K (8Gb density) */
static struct mx6_ddr3_cfg mt41k512m16ha_125 = {
	.mem_speed = 1600,
	.density = 8,
	.width = 16,
	.banks = 8,
	.rowaddr = 16,
	.coladdr = 10,
	.pagesz = 2,
	.trcd = 1375,
	.trcmin = 4875,
	.trasmin = 3500,
};

/* MT41K256M16TW-107 (4Gb density) */
static struct mx6_ddr3_cfg mt41k256m16tw_107 = {
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


static struct mx6ul_iomux_ddr_regs mx6ul_ddr_ioregs_standard = {
	.dram_dqm0 = 0x00000028,
	.dram_dqm1 = 0x00000028,
	.dram_ras = 0x00000028,
	.dram_cas = 0x00000028,
	.dram_odt0 = 0x00000028,
	.dram_odt1 = 0x00000028,
	.dram_sdba2 = 0x00000000,
	.dram_sdclk_0 = 0x00000028,
	.dram_sdqs0 = 0x00000028,
	.dram_sdqs1 = 0x00000028,
	.dram_reset = 0x00000028,
};

//0659
static struct mx6ul_iomux_ddr_regs mx6ull_ddr_ioregs_standard = {
	.dram_dqm0 = 0x00000030,
	.dram_dqm1 = 0x00000030,
	.dram_ras = 0x00000030,
	.dram_cas = 0x00000030,
	.dram_odt0 = 0x00000030,
	.dram_odt1 = 0x00000030,
	.dram_sdba2 = 0x00000000,
	.dram_sdclk_0 = 0x00000030,
	.dram_sdqs0 = 0x00000030,
	.dram_sdqs1 = 0x00000030,
	.dram_reset = 0x000C0030,
};

static struct mx6ul_iomux_grp_regs mx6_grp_ioregs_standard = {
	.grp_addds = 0x00000028,
	.grp_ddrmode_ctl = 0x00020000,
	.grp_b0ds = 0x00000028,
	.grp_ctlds = 0x00000028,
	.grp_b1ds = 0x00000028,
	.grp_ddrpke = 0x00000000,
	.grp_ddrmode = 0x00020000,
	.grp_ddr_type = 0x000c0000,
};

//0659
struct mx6_ddr_sysinfo mx6_ddr_sysinfo_standard = {
	.dsize = 0,
	.cs_density = 20,
	.ncs = 1,
	.cs1_mirror = 0,
	.rtt_wr = 2,
	.rtt_nom = 1,		/* RTT_Nom = RZQ/2 */
	.walat = 1,		/* Write additional latency */
	.ralat = 5,		/* Read additional latency */
	.mif3_mode = 3,		/* Command prediction working mode */
	.bi_on = 1,		/* Bank interleaving enabled */
	.sde_to_rst = 0x10,	/* 14 cycles, 200us (JEDEC default) */
	.rst_to_cke = 0x23,	/* 33 cycles, 500us (JEDEC default) */
	.ddr_type = DDR_TYPE_DDR3,
};

struct mx6_ddr_sysinfo mx6_ddr_sysinfo_1g = {
	.dsize = 0,
	.cs_density = 24,
	.ncs = 1,
	.cs1_mirror = 0,
	.rtt_wr = 2,
	.rtt_nom = 1,		/* RTT_Nom = RZQ/2 */
	.walat = 1,		/* Write additional latency */
	.ralat = 5,		/* Read additional latency */
	.mif3_mode = 3,		/* Command prediction working mode */
	.bi_on = 1,		/* Bank interleaving enabled */
	.sde_to_rst = 0x10,	/* 14 cycles, 200us (JEDEC default) */
	.rst_to_cke = 0x23,	/* 33 cycles, 500us (JEDEC default) */
	.ddr_type = DDR_TYPE_DDR3,
};

/*
 * calibration - these are the various CPU/DDR3 combinations we support
 */

static struct mx6_mmdc_calibration mx6ul_mmcd_calib_standard = {
	.p0_mpwldectrl0 = 0x00000000,
	.p0_mpdgctrl0 = 0x0144014C,
	.p0_mprddlctl = 0x40405450,
	.p0_mpwrdlctl = 0x40405450,
};

//0659
static struct mx6_mmdc_calibration mx6ull_mmcd_calib_standard = {
	.p0_mpwldectrl0 = 0x00000000,
	.p0_mpwldectrl1 = 0x001F001F,
	.p0_mpdgctrl0 = 0x01440144,
	.p0_mprddlctl = 0x40403430,
	.p0_mpwrdlctl = 0x40403A36,
};

#endif
