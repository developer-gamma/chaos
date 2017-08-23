/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#ifndef _KERNEL_FS_H_
# define _KERNEL_FS_H_

# include <kernel/list.h>
# include <chaoserr.h>

# define FS_FILE		0b0000
# define FS_DIRECTORY		0b0001
# define FS_CHARDEVICE		0b0010
# define FS_BLOCKDEVICE		0b0100
# define FS_MOUNTPOINT		0b1000

typedef struct fscookie fscookie;
typedef struct filecookie filecookie;
typedef struct dircookie dircookie;
struct bdev;
struct cdev;

/*
** Api that a filesystem must (at least partialy) implement
*/
struct fs_api
{
	status_t (*mount)(struct bdev *, struct fscookie **);
	status_t (*unmount)(struct fscookie *);
	status_t (*open)(struct fscookie *, char const *, struct filecookie **);
	status_t (*close)(struct filecookie *);
	struct fs_file *(*finddir)(struct fs_file *dir, char const *name);
};

/*
** Virtual File-System's file representation
*/
struct fs_file
{
	char name[256];
	uint type;

	struct fs_mount *mount;		/* the mouting point of this file */
	struct filecookie *cookie;	/* fs file datas */

	union { /* Special more data depending of the type of this file */
		struct bdev *bdev_data;
		struct cdev *cdev_data;
	};
};

/*
** Some more content if the file is a mount point
*/
struct fs_mount
{
	struct bdev *bdev;
	struct fscookie *fscookie;	/* fs datas */
	struct fs_api const *api;	/* api of this filesystem */
};

/*
** Handler on an opened file
** Used for reading, writing etc.
*/
struct filehandler
{
	struct fs_file *file;
	size_t offset;
};

/* Vfs */
status_t		init_vfs(void);
void			resolve_path(char *);
struct fs_file		*find_file(char *);

/* Generic fs functions */
status_t		fs_mount(char const *p, char const *fs, char const *dev);
status_t		fs_unmount(char const *path);
status_t		fs_open(char const *path, struct filehandler **handler);
status_t		fs_close(struct filehandler *filehandler);
struct filehandler	*fs_dup_handler(struct filehandler const *handler);

struct fs_hook
{
	char const *name;
	struct fs_api *api;
};

# define NEW_FILESYSTEM(n, a)						\
	__aligned(sizeof(void*)) __used __section("fs_hook")		\
	static const struct fs_hook _fs_hook_##n = {			\
		.name = #n,						\
		.api = a,						\
	}


#endif /* !_KERNEL_FS_H_ */

