/*
 * Copyright (c) 2018 Elettronica GF s.r.l.
 *    Andrea Collamati <andrea.collamati@elettronicagf.it>
 *    Stefano Donati <stefano.donati@elettronicagf.it>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <memblk.h>
#include <part.h>
#include <asm/io.h>
#include <mapmem.h>
#include <memblk.h>

#define BLOCK_SIZE 512

u32 base = CFG_UPDATE_PACKAGE_FAT_BLOB_LOAD_ADDRESS;
u32 width = CFG_UPDATE_PACKAGE_FAT_BLOB_LENGTH;

static unsigned long memblk_read(struct blk_desc *block_dev,
								  lbaint_t start,
								  lbaint_t blkcnt,
								  void *buffer);

struct blk_desc mem_dev_desc;

void memblk_init(void)
{
		mem_dev_desc.if_type = IF_TYPE_UNKNOWN;
		mem_dev_desc.devnum = 0;
		mem_dev_desc.part_type = PART_TYPE_DOS;
		mem_dev_desc.type = DEV_TYPE_HARDDISK;
		mem_dev_desc.blksz = BLOCK_SIZE;
		mem_dev_desc.lba = (CFG_UPDATE_PACKAGE_FAT_BLOB_LENGTH / BLOCK_SIZE);
		mem_dev_desc.log2blksz = LOG2(BLOCK_SIZE);
		mem_dev_desc.removable = 1;
		mem_dev_desc.block_read = memblk_read;
}

/*
 * This function is called (by dereferencing the block_read pointer in
 * the dev_desc) to read blocks of data. The return value is the
 * number of blocks read. A zero return indicates an error.
 */
static unsigned long memblk_read(struct blk_desc *block_dev,
								  lbaint_t start,
								  lbaint_t blkcnt,
								  void *buffer)
{

#ifdef DEBUG_MEMBLK
	printf("memblk: read %lu sectors at %lu\n", blkcnt, start);
#endif

	ulong bytes = mem_dev_desc.blksz * blkcnt;
	const char* reqAddress = (const char*)(base + mem_dev_desc.blksz * start);

	memcpy(buffer,reqAddress,bytes);
	int ret = memcmp(buffer,reqAddress,bytes);

	return blkcnt;
}

U_BOOT_LEGACY_BLK(mem) = {
	.if_typename	= "mem",
	.if_type	= IF_TYPE_UNKNOWN,
	.max_devs	= 1,
	.desc       = &mem_dev_desc,
};
