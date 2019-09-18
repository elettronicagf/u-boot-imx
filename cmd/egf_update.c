/*
 * Copyright 2018 Elettronica GF s.r.l.
 *
 * Mantainers:
 * Stefano Donati <stefano.donati@elettronicagf.it>
 * Andrea Collamati <andrea.collamati@elettronicagf.it>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <spi.h>
#include <spi_flash.h>
#include <asm/io.h>
#include <asm/gpio.h>


#define UPDATE_STATUS_READY_FOR_INSTALLATION 	0
#define UPDATE_STATUS_INSTALLING_ATTEMPT_1 		1
#define UPDATE_STATUS_INSTALLING_ATTEMPT_2 		2
#define UPDATE_STATUS_INSTALLING_ATTEMPT_3 		3
#define UPDATE_STATUS_INSTALLING_ATTEMPT_4 		4
#define UPDATE_STATUS_INSTALLING_ATTEMPT_5 		5
#define UPDATE_DISABLED	 						254
#define UPDATE_INSTALLED_SUCCESSFULLY	 		255

typedef enum ota_event_id_
{
	OtaSetReadyCommand,
	OtaUpdateStartedEvent,
	OtaDisableUpdateCommand,
	OtaUpdateFailedEvent,
	OtaUpdateSuccessfulEvent
}  ota_event_id;


struct egf_ota_status {
	char id[4];
	uint32_t status;
};

static int check_status_and_fix(struct egf_ota_status *upg_status)
{
	int valid = 1;
	int changed = 0;
	if ((upg_status->id[0] != 'e') ||
			(upg_status->id[1] != 'G') ||
			(upg_status->id[2] != 'F') ||
			(upg_status->id[3] != '\0')) {
		valid = 0;

	}

	switch (upg_status->status) {
	case UPDATE_STATUS_READY_FOR_INSTALLATION:
	case UPDATE_STATUS_INSTALLING_ATTEMPT_1:
	case UPDATE_STATUS_INSTALLING_ATTEMPT_2:
	case UPDATE_STATUS_INSTALLING_ATTEMPT_3:
	case UPDATE_STATUS_INSTALLING_ATTEMPT_4:
	case UPDATE_STATUS_INSTALLING_ATTEMPT_5:
	case UPDATE_DISABLED:
	case UPDATE_INSTALLED_SUCCESSFULLY:
		break;
	default:
		valid = 0;
	}

	if (!valid) {
		upg_status->id[0] = 'e';
		upg_status->id[1] = 'G';
		upg_status->id[2] = 'F';
		upg_status->id[3] = '\0';
		upg_status->status = UPDATE_DISABLED;
		changed = 1;
		printf("Initializing update structure\n");
	}
	return changed;

}

static int write_status(struct egf_ota_status *upg_status)
{
	int ret;
	unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	unsigned int cs = CONFIG_SF_DEFAULT_CS;
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	uint32_t update_status_offset;
	struct spi_flash *flash;

	flash = spi_flash_probe(bus, cs, speed, mode);
	if (!flash) {
		printf("Failed to initialize SPI flash at %u:%u\n", bus, cs);
		return 1;
	}

	/* Update status is saved at the beginning of SPI NOR flash last sector */
	update_status_offset = flash->size - flash->erase_size;

	gpio_direction_output(CONFIG_SF_WPn_GPIO, 1);
	gpio_set_value(CONFIG_SF_WPn_GPIO, 1);

	ret = spi_flash_lock_enable(flash, 0);
	if (ret) {
		printf("Error disabling SPI NOR write lock\n");
		return ret;
	}


	/* Erase SPI NOR Flash last sector */
	ret = spi_flash_erase(flash, update_status_offset, flash->erase_size);
	if (ret) {
		printf("Error erasing flash sector at offset: %x, size: %x\n",
				update_status_offset, flash->erase_size);
		return ret;
	}

	ret = spi_flash_write(flash, update_status_offset, sizeof(*upg_status), upg_status);
	if (ret) {
		printf("Error writing flash  at offset: %x, size: %x\n",
				update_status_offset, sizeof(*upg_status));
		return ret;
	}

	ret = spi_flash_lock_enable(flash, 1);
	if (ret) {
		printf("Error enabling SPI NOR write lock\n");
		return ret;
	}
	gpio_set_value(CONFIG_SF_WPn_GPIO, 0);

	return 0;

}

static int read_status(struct egf_ota_status *upg_status)
{
	int ret;
	unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	unsigned int cs = CONFIG_SF_DEFAULT_CS;
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	uint32_t update_status_offset;
	struct spi_flash *flash;

	flash = spi_flash_probe(bus, cs, speed, mode);
	if (!flash) {
		printf("Failed to initialize SPI flash at %u:%u\n", bus, cs);
		return 1;
	}

	/* Update status is saved at the beginning of SPI NOR flash last sector */
	update_status_offset = flash->size - flash->erase_size;
	ret = spi_flash_read(flash, update_status_offset, sizeof(*upg_status), upg_status);
	if (ret) {
		printf("Failed to read SPI flash at %u:%u, offset %x, len %x\n",
				bus, cs, update_status_offset, sizeof(*upg_status));
		return 1;
	}

	ret = check_status_and_fix(upg_status);
	if (ret)
		write_status(upg_status);

	return 0;
}

static int pending_installation(struct egf_ota_status *upg_status)
{
	int ret;

	switch (upg_status->status) {
	case UPDATE_STATUS_READY_FOR_INSTALLATION:
	case UPDATE_STATUS_INSTALLING_ATTEMPT_1:
	case UPDATE_STATUS_INSTALLING_ATTEMPT_2:
	case UPDATE_STATUS_INSTALLING_ATTEMPT_3:
	case UPDATE_STATUS_INSTALLING_ATTEMPT_4:
		ret = 1;
		break;
	default:
		ret = 0;
		break;
	}

	return ret;
}

static void run_state_machine(struct egf_ota_status *upg_status, ota_event_id cmd)
{
	if(cmd == OtaDisableUpdateCommand && upg_status->status != UPDATE_DISABLED)
	{
		upg_status->status  = UPDATE_DISABLED;
		write_status(upg_status);
		return;
	}
	else if(cmd == OtaUpdateFailedEvent && upg_status->status != UPDATE_DISABLED)
	{
		upg_status->status  = UPDATE_DISABLED;
		write_status(upg_status);
		return;
	}
	else if(cmd == OtaUpdateSuccessfulEvent && upg_status->status != UPDATE_INSTALLED_SUCCESSFULLY)
	{
		upg_status->status  = UPDATE_INSTALLED_SUCCESSFULLY;
		write_status(upg_status);
		return;
	}
	else if(cmd == OtaSetReadyCommand && upg_status->status != UPDATE_STATUS_READY_FOR_INSTALLATION)
	{
		upg_status->status  = UPDATE_STATUS_READY_FOR_INSTALLATION;
		write_status(upg_status);
		return;
	}
	else if (cmd == OtaUpdateStartedEvent)
	{
		if(pending_installation(upg_status)){

			/* avanzo di stato */
			upg_status->status++;
			write_status(upg_status);
			return;
		}
		else if(upg_status->status == UPDATE_STATUS_INSTALLING_ATTEMPT_5)
		{
			upg_status->status  = UPDATE_DISABLED;
			write_status(upg_status);
			return;
		}
	}

	return;
}

static int do_egf_ota_status_read(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;
	struct egf_ota_status upg_status;

	if (argc != 1) {
		return CMD_RET_USAGE;
	}

	ret = read_status(&upg_status);
	if (ret)
		printf("ERROR %d\n", ret);
	else {
		printf("id = %s, status = %d\n", upg_status.id, upg_status.status);
		printf("OK\n");
	}
	return ret;
}

static int do_egf_ota_start (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;
	struct egf_ota_status upg_status;

	if (argc != 1) {
		return CMD_RET_USAGE;
	}

	ret = read_status(&upg_status);
	if (ret)
		printf("Error reading update status from NOR %d\n", ret);
	else {
		printf("id = %s, status = %d\n", upg_status.id, upg_status.status);
	}

	ret = pending_installation(&upg_status);
	if (!ret)
		printf("Update cannot be installed\n");
	run_state_machine(&upg_status, OtaUpdateStartedEvent);

	return !ret;
}

static int do_egf_ota_disable (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;
	struct egf_ota_status upg_status;

	if (argc != 1) {
		return CMD_RET_USAGE;
	}

	ret = read_status(&upg_status);
	if (ret)
		printf("Error reading update status from NOR %d\n", ret);
	else {
		printf("id = %s, status = %d\n", upg_status.id, upg_status.status);
	}

	run_state_machine(&upg_status, OtaDisableUpdateCommand);

	return upg_status.status == UPDATE_DISABLED;
}


static int do_egf_ota_status_write(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret;
	struct egf_ota_status upg_status;
	unsigned long status_to_write;

	if (argc != 2) {
		return CMD_RET_USAGE;
	}

	status_to_write = simple_strtoul(argv[1], NULL, 10);
	printf("Status to Write is %ld\n", status_to_write);


	ret = read_status(&upg_status);
	upg_status.status = status_to_write;

	ret = write_status(&upg_status);
	if (ret) {
		printf("Error writing status to NOR\n");
	}
	return ret;
}

static int do_egf_update_validate_header(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long addr;
	unsigned char *buf;
	unsigned char md5[33];
	int ret = 0;

	if (argc != 2) {
		return CMD_RET_USAGE;
	}

	addr = simple_strtoul(argv[1], NULL, 16);
	if (*argv[1] == 0)
		return -1;

	buf = map_physmem(addr, CFG_UPDATE_PACKAGE_HEADER_LENGTH, MAP_WRBACK);
	if ((*buf == 'e') &&
		*(buf + 1) == 'G' &&
		*(buf + 2) == 'F' &&
		*(buf + 3) == '1')
		ret = 0;
	else
		ret = 1;
	strncpy(md5, buf + 4, 32);
	md5[32] = 0;
	unmap_physmem(buf, CFG_UPDATE_PACKAGE_HEADER_LENGTH);
	env_set("update_md5", md5);
	return ret;
}

U_BOOT_CMD(
		egf_ota_read_status, 1,	1,	do_egf_ota_status_read,
		"Read eGF Update System status flags",
		"Read eGF Update System status flags\n"
);

U_BOOT_CMD(
		egf_ota_start, 1, 1,	do_egf_ota_start,
		"Check if update can be installed and update status",
		"Check if update can be installed and update status\n"
);

U_BOOT_CMD(
		egf_ota_disable, 1, 1,	do_egf_ota_disable,
		"Disable ota update",
		"Mark ota update status to disabled\n"
);

U_BOOT_CMD(
		egf_ota_write_status, 2,	1,	do_egf_ota_status_write,
		"Write eGF Update System status flags",
		"<new status>\n"
		"    - check update system status flag to 'new status'\n"
);

U_BOOT_CMD(
		egf_update_validate_header, 2, 1,	do_egf_update_validate_header,
		"Check if update package header is valid",
		"<addr>\n"
		"    - check update package header at address 'addr'\n"
);

