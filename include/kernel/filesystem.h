/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#ifndef _KERNEL_FILESYSTEM_H_
# define _KERNEL_FILESYSTEM_H_

# include <chaosdef.h>

# define PATH_SEPARATOR		'/'
# define PATH_SEPARATOR_STR	"/"
# define PATH_CURRENT		'.'

# define FS_FILE		0x01
# define FS_DIRECTORY		0x02

struct fs_node;

typedef void (*open_cb_t)(struct fs_node *);
typedef ssize_t (*read_cb_t)(struct fs_node *, uint32, uint8 *, size_t);
typedef ssize_t (*write_cb_t)(struct fs_node *, uint32, uint8 const *, size_t);
typedef struct fs_node *(*find_file_cb_t)(struct fs_node *, char const *file);

struct fs_node
{
	char name[256];		/* Filename */
	uint32 type;		/* Type (file, directory etc.) */
	uint32 fs;		/* Mark of the filesystem it belongs to */
	uint32 inode;		/* inode numbrer */
	size_t length;		/* Length of the file */

	/* File operation variables */
	uint32 offset;
	int32 ref_count;

	/* File operations */
	open_cb_t open;
	read_cb_t read;
	write_cb_t write;
	find_file_cb_t find_file;
};

struct fs_node			*kopen(char const *relative, char const *file);

#endif /* !_KERNEL_FILESYSTEM_H_ */

