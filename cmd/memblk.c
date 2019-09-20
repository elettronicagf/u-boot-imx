/*
 * MEMBLK support
 */

#include <common.h>
#include <blk.h>
#include <config.h>
#include <watchdog.h>
#include <command.h>
#include <image.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <memblk.h>

int do_memblk(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	if (argc == 2) {
		if (strncmp(argv[1], "init", 4) == 0) {
			memblk_init();
			printf("memblk initialized, start=0x%x size=%d\n", base, width);
			return 0;
		}
	}

	return -1;
}

U_BOOT_CMD(memblk, 5, 1, do_memblk,
		"MEMBLK sub-system",
		"init - initialize memblk device\n");
