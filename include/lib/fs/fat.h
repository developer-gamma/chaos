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

struct bdev;

struct fs_fat
{
	struct bdev *dev;
	uint32 bytes_per_sector;
	uint32 bytes_per_cluster;
	uint32 sectors_per_cluster;
	uint32 reserved_sectors;
	uint32 fat_count;
	uint32 total_sectors;
	uint32 media_type;
	uint32 sectors_per_fat;

	uint32 root_sector;		/* Number of the root sector, relative to root_start */
	uint32 root_entry_count;	/* Numbers of entries in the root sector */
	uint32 root_start;		/* Number of the sector where the root starts */
	uint32 data_start;
	uint32 total_clusters;

	uint32 fat_type;		/* Fat16 or Fat32 ? */
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


#endif /* !_LIB_FS_FAT_H_ */
