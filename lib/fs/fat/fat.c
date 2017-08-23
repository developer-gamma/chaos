/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/bdev.h>
#include <kernel/kalloc.h>
#include <lib/fs/fat.h>

/* Debug */
#include <stdio.h>

static status_t
fat_mount(struct bdev *bdev, struct fscookie **fscookie)
{
	status_t err;
	struct fs_fat *fat;
	uint8 *br;

	br = kalloc(BOOT_SECTOR_SIZE);
	fat = kalloc(sizeof(struct fs_fat));
	if (br == NULL || fat == NULL) {
		err = ERR_NO_MEMORY;
		goto err;
	}

	/* Read the first 512 bytes */
	if (bdev_read(bdev, br, 0, BOOT_SECTOR_SIZE) != BOOT_SECTOR_SIZE) {
		err = ERR_BAD_DEVICE;
		goto err;
	}

	fat->dev = bdev;

	/* Verify partition signature */
	if (br[510] != 0x55 && br[511] != 0xAA) {
		err = ERR_INVALID_ARGS;
		goto err;
	}

	/* Retrieve informations */
	fat->bytes_per_sector = fat_read16(br, 0xB);
	fat->sectors_per_cluster = br[0xD];
	fat->reserved_sectors = fat_read16(br, 0xE);
	fat->fat_count = br[0x10];
	fat->media_type = br[0x15];
	fat->sectors_per_fat = fat_read16(br, 0x16);

	/* Some error handling */
	if (fat->fat_count == 0
		|| !fat->bytes_per_sector)
	{
		err = ERR_INVALID_ARGS;
		goto err;
	}

	/* Trigger an error if something unsupported is detected */
	if (fat->media_type != 0xF8) { /* 0xF8 stands for a hard drive */
		err = ERR_NOT_SUPPORTED;
		goto err;
	}

	if (fat->sectors_per_fat == 0) { /* Fat32 */
		printf("UNSUPPORTED FAT32\n");
		err = ERR_NOT_SUPPORTED;
		goto err;
	}
	else { /* Fat12/Fat16 */
		if (fat->fat_count != 0x2) {
			err = ERR_INVALID_ARGS;
			goto err;
		}

		fat->total_sectors = fat_read16(br, 0x13);
		if (fat->total_sectors == 0x0) {
			fat->total_sectors = fat_read32(br, 0x20);
		}

		fat->root_sector = 0x0; /* Always 0 on Fat12/Fat16 */
		fat->root_entry_count = fat_read16(br, 0x11);
		fat->root_start = fat->reserved_sectors + fat->fat_count * fat->sectors_per_fat;
		fat->data_start = fat->root_start + fat->root_entry_count * 0x20 / fat->bytes_per_sector;
		fat->data_sectors = (fat->total_sectors - fat->data_start);
		fat->total_clusters = fat->data_sectors / fat->sectors_per_cluster;

		fat->fat_type = FAT_16;
	}

	fat->bytes_per_cluster = fat->sectors_per_cluster * fat->bytes_per_sector;

	*fscookie = (struct fscookie *)fat;
	return (OK);
err:
	kfree(br);
	kfree(fat);
	return (err);
}

static status_t
fat_unmount(struct fscookie *cookie)
{
	struct fs_fat *fat;

	fat = (struct fs_fat *)cookie;
	kfree(fat);
	return (OK);
}

static status_t
fat_open(struct fscookie *cookie __unused, char const *path __unused, struct filecookie **cokie __unused)
{
	return (OK);
}

static status_t
fat_close(struct filecookie *cookie __unused)
{
	return (OK);
}

static struct fs_api fat_api =
{
	.mount = &fat_mount,
	.unmount = &fat_unmount,
	.open = &fat_open,
	.close = &fat_close,
};

NEW_FILESYSTEM(fat16, &fat_api);
NEW_FILESYSTEM(fat32, &fat_api);
