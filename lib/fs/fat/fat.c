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
#include <string.h>

/* Debug */
#include <stdio.h>

static void __unused
fat_dump(struct fs_fat *fat)
{
	static char const *fat_type[3] = {
		"FAT12",
		"FAT16",
		"FAT32",
	};
	printf("Fat type: %s\n", fat_type[fat->type]);
	printf("Bytes per sector: %u\n", fat->bytes_per_sector);
	printf("Bytes per cluster: %u\n", fat->bytes_per_cluster);
	printf("Sector per cluster: %u\n", fat->sectors_per_cluster);
	printf("Reserved sectors: %u\n", fat->reserved_sectors);
	printf("Fat Count: %u\n", fat->fat_count);
	printf("Sectors per fat: %u\n", fat->sectors_per_fat);
	printf("Total sectors: %u\n", fat->total_sectors);
	printf("Media type: %u\n", fat->media_type);

	printf("Root cluster: %#x\n", fat->root_cluster);
	printf("Root entry count: %u\n", fat->root_entry_count);
	printf("Root sector start: %#x\n", fat->root_start);
	printf("Root sectors count: %#x\n", fat->root_sectors);
	printf("Data start: %#x\n", fat->data_start);
	printf("Data sectors: %#x\n", fat->data_sectors);
	printf("Data clusters: %#x\n", fat->data_clusters);
	printf("Directory entries per cluster: %#x\n", fat->dir_entries_per_cluster);
}

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
	fat->bytes_per_cluster = fat->sectors_per_cluster * fat->bytes_per_sector;
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
		/* We do not support FAT32 yet */
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

		fat->root_cluster = 0x0; /* In fat12/fat16, there is no root cluster */
		fat->root_entry_count = fat_read16(br, 0x11);
		fat->dir_entries_per_cluster = (fat->bytes_per_cluster / DIRENT_SIZE);
		fat->root_start = fat->reserved_sectors + fat->fat_count * fat->sectors_per_fat;
		fat->data_start= fat->root_start + fat->root_entry_count * DIRENT_SIZE / fat->bytes_per_sector;
		fat->root_sectors = fat->data_start - fat->root_start;
		fat->data_sectors = fat->total_sectors - fat->root_start + fat->root_sectors;
		fat->data_clusters = fat->data_sectors / fat->sectors_per_cluster;
		fat->type = fat->data_clusters < 4085 ? FAT12 : FAT16;
	}

	if (fat->bytes_per_cluster % DIRENT_SIZE != 0
		|| fat->root_entry_count % fat->dir_entries_per_cluster != 0) {
		err  = ERR_INVALID_ARGS;
		goto err;
	}

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
fat_open(struct fscookie *fscookie, char const *path, struct filecookie **cookieptr)
{
	struct fat_filecookie *cookie;
	status_t err;

	cookie = kalloc(sizeof(struct fat_filecookie));
	if (cookie == NULL) {
		return (ERR_NO_MEMORY);
	}
	memset(cookie, 0, sizeof(*cookie));
	err = fat_walk_until_file((struct fs_fat *)fscookie, path, cookie);
	if (err) {
		kfree(cookie);
		return (err);
	}
	*cookieptr = (struct filecookie *)cookie;
	return (OK);
}

static status_t
fat_close(struct fscookie *fscookie __unused, struct filecookie *cookie)
{
	kfree(cookie);
	return (OK);
}

static status_t
fat_opendir(struct fscookie *fscookie, char const *path, struct dircookie **cookieptr)
{
	struct fat_dircookie *cookie;
	status_t err;

	cookie = kalloc(sizeof(struct fat_dircookie));
	if (cookie == NULL) {
		return (ERR_NO_MEMORY);
	}
	memset(cookie, 0, sizeof(*cookie));
	err = fat_walk_until_dir((struct fs_fat *)fscookie, path, cookie);
	if (err) {
		kfree(cookie);
		return (err);
	}
	*cookieptr = (struct dircookie *)cookie;
	return (OK);
}

status_t
fat_readdir(struct fscookie *fscookie, struct dircookie *dir, struct dirent *dirent)
{
	struct fs_fat *fat;
	struct fat_dircookie *dircookie;
	struct fat_dirent fdirent;
	status_t err;

	fat = (struct fs_fat *)fscookie;
	dircookie = (struct fat_dircookie *)dir;
	while (42)
	{
		err = fat_get_next_directory_entry(fat, dircookie, &fdirent);
		if (err) {
			return (err);
		}
		fat_get_filename(&fdirent, dirent->name);
		if (fat_is_entry_taken(&fdirent)) {
			dirent->dir = fdirent.att & FAT_ATT_DIR;
			return (OK);
		}
	}
}

static status_t
fat_closedir(struct fscookie *fscookie __unused, struct dircookie *cookie)
{
	kfree(cookie);
	return (OK);
}

static struct fs_api fat_api =
{
	.mount = &fat_mount,
	.unmount = &fat_unmount,
	.open = &fat_open,
	.close = &fat_close,

	.opendir = &fat_opendir,
	.readdir = &fat_readdir,
	.closedir = &fat_closedir,
};


NEW_FILESYSTEM(fat12, &fat_api);
NEW_FILESYSTEM(fat16, &fat_api);
