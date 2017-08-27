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

/* DEBUG */
#include <stdio.h>

static uint32
get_next_cluster(struct fs_fat *fat, uint32 cluster)
{
	uint8 buff[4];
	uint32 next_cluster;
	size_t offset;

	if (fat->type == FAT12) {
		offset = fat->reserved_sectors * fat->bytes_per_sector + cluster + (cluster / 2u);
		bdev_read(fat->dev, buff, offset, sizeof(buff));
		next_cluster = fat_read16(buff, 0);
		if (cluster & 0x0001) {
			next_cluster = next_cluster >> 4u;
		}
		else {
			next_cluster = next_cluster & 0x0FFF;
		}
		if (next_cluster > 0xFF0) {
			next_cluster |= 0x0FFFF000;
		}
	} else if (fat->type == FAT16) {
		offset = fat->reserved_sectors * fat->bytes_per_sector + cluster * 2u;
		bdev_read(fat->dev, buff, offset, sizeof(buff));
		next_cluster = fat_read16(buff, 0);
		if (next_cluster > 0xFFF0) {
			next_cluster |= 0x0FFF0000;
		}
	} else {
		panic("FAT32 not supported");
	}
	return (next_cluster);
}

static bool
fat_is_directory(struct fat_dirent *dirent)
{
	return (dirent->att & FAT_ATT_DIR);
}

/*
** Returns true if the given directory entry is present or not.
*/
bool
fat_is_entry_taken(struct fat_dirent *dirent)
{
	return (dirent->name[0] != 0x0 && dirent->name[0] != 0xe5);
}

/*
** Copies the filename in the given pointer
** Name must be at least 13 bytes.
*/
void
fat_get_filename(struct fat_dirent *dirent, char *name)
{
	size_t i;
	size_t j;

	i = 0;
	switch (dirent->name[0])
	{
	case 0x0:
	case 0xe5:
		goto end;
	case 0x05:
		name[i++] = 0xe5;
		break;
	default:
		name[i++] = dirent->name[0];
		break;
	}

	while (i < 8 && dirent->name[i] != ' ') {
		name[i] = dirent->name[i];
		++i;
	}

	if (dirent->ext[0] != ' ') {
		name[i++] = '.';
		j = 0;
		while (j < 3 && dirent->ext[j] != ' ') {
			name[i] = dirent->ext[j];
			++i;
			++j;
		}
	}

end:
	name[i] = '\0';
}

static size_t
get_cluster_offset(struct fs_fat *fat, uint32 cluster)
{
	return ((fat->data_start + (cluster - 2) * fat->sectors_per_cluster) * fat->bytes_per_sector);
}

/*
** Returns the next entry within the given directory
**
** This entry may be unused, you should check it before anything.
*/
status_t
fat_get_next_directory_entry(struct fs_fat *fat, struct fat_dircookie *dir, struct fat_dirent *dirent)
{
	size_t offset;

	if (dir->cluster >= 0x0FFFFFF0u
		|| (dir->root && dir->idx >= fat->root_entry_count))
	{
		return (ERR_NOT_FOUND);
	}
	if (dir->root) {
		offset = fat->root_start * fat->bytes_per_sector;
	}
	else {
		offset = get_cluster_offset(fat, dir->cluster);
	}
	bdev_read(fat->dev, dirent, offset + dir->idx * DIRENT_SIZE, DIRENT_SIZE);
	++dir->idx;
	if (!dir->root && dir->idx >= fat->dir_entries_per_cluster) {
		dir->cluster = get_next_cluster(fat, dir->cluster);
		dir->idx = 0;
	}
	return (OK);
}

/*
** Returns the entry with the given name if it exists.
*/
status_t
fat_find_dir_entry(struct fs_fat *fat, struct fat_dircookie *dir, char const *name, struct fat_dirent *dirent)
{
	char lookingname[13];
	status_t err;

	while (42) {
		err = fat_get_next_directory_entry(fat, dir, dirent);
		if (err) {
			return (err);
		}
		if (fat_is_entry_taken(dirent))
		{
			fat_get_filename(dirent, lookingname);
			if (!strcmp(name, lookingname)) {
				return (OK);
			}
		}
	}
}

status_t
fat_walk_until_file(struct fs_fat *fat, char const *ori_path, struct fat_filecookie *cookie)
{
	struct fat_dircookie dircookie;
	struct fat_dirent dirent;
	char *path;
	char *offset;
	size_t len;

	path = strdup(ori_path);
	if (path == NULL) {
		return (ERR_NO_MEMORY);
	}
	len = strlen(path);
	if (len == 0) { /* We're looking for the root directory */
		kfree(path);
		return (ERR_NOT_REGULAR_FILE);
	}
	offset = path;
	while (*offset)
	{
		if (*offset == '/') {
			*offset = '\0';
		}
		++offset;
	}
	offset = path;
	dircookie.root = true;
	while (offset < path + len)
	{
		dircookie.cluster = dirent.starting_cluster;
		dircookie.idx = 0;
		if (fat_find_dir_entry(fat, &dircookie, offset, &dirent)
			|| !fat_is_directory(&dirent)) {
			goto err;
		}
		dircookie.root = false;
		offset += strlen(offset) + 1;
	}
	kfree(path);
	cookie->cluster = dirent.starting_cluster;
	return (fat_is_directory(&dirent) ? ERR_NOT_REGULAR_FILE : OK);
err:
	kfree(path);
	return (ERR_NOT_FOUND);
}

status_t
fat_walk_until_dir(struct fs_fat *fat, char const *ori_path, struct fat_dircookie *cookie)
{
	struct fat_dircookie dircookie;
	struct fat_dirent dirent;
	char *path;
	char *offset;
	size_t len;

	path = strdup(ori_path);
	if (path == NULL) {
		return (ERR_NO_MEMORY);
	}
	len = strlen(path);
	if (len == 0) { /* We're looking for the root directory */
		kfree(path);
		cookie->root = true;
		cookie->cluster = fat->root_cluster;
		cookie->idx = 0;
		return (OK);
	}
	offset = path;
	while (*offset)
	{
		if (*offset == '/') {
			*offset = '\0';
		}
		++offset;
	}
	offset = path;
	dircookie.root = true;
	while (offset < path + len)
	{
		dircookie.cluster = dirent.starting_cluster;
		dircookie.idx = 0;
		if (fat_find_dir_entry(fat, &dircookie, offset, &dirent)
			|| !fat_is_directory(&dirent)) {
			goto err;
		}
		dircookie.root = false;
		offset += strlen(offset) + 1;
	}
	cookie->cluster = dirent.starting_cluster;
	cookie->idx = 0;
	cookie->root = false;
	return (fat_is_directory(&dirent) ? OK : ERR_NOT_DIRECTORY);
err:
	kfree(path);
	return (ERR_NOT_FOUND);
}
