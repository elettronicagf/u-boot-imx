/*
 * Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
int reset_gf_som_eeprom_content(char* egf_sw_id_code, int ask_confirmation);

static int do_init_eeprom(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;
	if (argc != 2) {
		return CMD_RET_USAGE;
	}

	ret = reset_gf_som_eeprom_content(argv[1], 0);

	return ret;
}

U_BOOT_CMD(
	init_eeprom, 2,	1,	do_init_eeprom,
	"initialize eGF SOM EEPROM with only WID",
	"initialize eGF SOM EEPROM with only WID\n"
);
