/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */


#ifndef _LIB_FS_FAT_H_
# define _LIB_FS_FAT_H_

# include <kernel/fs.h>

# define BOOT_SECTOR_SIZE	512u
# define DIRENT_SIZE		32u

# define FAT_ATT_DIR		0x10

struct bdev;

struct fat_filecookie
{
	uint32 cluster;
};

struct fat_dircookie
{
	bool root;
	uint32 cluster;
	size_t idx;
};

struct fat_dirent
{
	uchar name[8];
	uchar ext[3];
	uchar att;
	uchar __reserved[10];
	uint16 time_update;
	uint16 date_update;
	uint16 starting_cluster;
	uint32 file_size;
} __packed;

static_assert(sizeof(struct fat_dirent) == DIRENT_SIZE);

enum fat_type
{
	FAT12,
	FAT16,
	FAT32,
};

struct fs_fat
{
	struct bdev *dev;
	uint32 bytes_per_sector;
	uint32 bytes_per_cluster;
	uint32 sectors_per_cluster;
	uint32 reserved_sectors;
	uint32 fat_count;
	uint32 sectors_per_fat;
	uint32 total_sectors;
	uint32 media_type;

	uint32 root_cluster;		/* Number of the root cluster */
	uint32 root_entry_count;	/* Numbers of entries in the root sector */
	uint32 root_start;		/* Number of the sector where the root starts */
	uint32 root_sectors;
	uint32 data_start;		/* Number of the sector where data starts */
	uint32 data_sectors;
	uint32 data_clusters;
	uint32 dir_entries_per_cluster;	/* Number of directory entries per cluster */

	enum fat_type type;
};

/* Note: fat datas are little endian */

static inline uint16
fat_read16(uint8 const *buffer, size_t offset)
{
	return (buffer[offset] | (buffer[offset + 1] << 8u));
}

static inline uint32
fat_read32(uint8 const *buffer, size_t offset)
{
	return (buffer[offset]
		| (buffer[offset + 1] << 8u)
		| (buffer[offset + 2] << 16u)
		| (buffer[offset + 3] << 24u)
	);
}

status_t	fat_get_next_directory_entry(struct fs_fat *, struct fat_dircookie *, struct fat_dirent *);
bool		fat_is_entry_taken(struct fat_dirent *dirent);
void		fat_get_filename(struct fat_dirent *dirent, char *name);
status_t	fat_walk_until_dir(struct fs_fat *, char const *, struct fat_dircookie *);
status_t	fat_walk_until_file(struct fs_fat *, char const *, struct fat_filecookie *);

#endif /* !_LIB_FS_FAT_H_ */
