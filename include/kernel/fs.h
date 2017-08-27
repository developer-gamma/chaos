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

struct fscookie;
struct filecookie;
struct dircookie;
struct bdev;
struct cdev;
struct dirent;

/*
** Api that a filesystem must (at least partialy) implement
*/
struct fs_api
{
	status_t (*mount)(struct bdev *, struct fscookie **);
	status_t (*unmount)(struct fscookie *);
	status_t (*open)(struct fscookie *, char const *, struct filecookie **);
	status_t (*close)(struct fscookie *, struct filecookie *);

	status_t (*opendir)(struct fscookie *, char const *, struct dircookie **);
	status_t (*readdir)(struct fscookie *, struct dircookie *, struct dirent *);
	status_t (*closedir)(struct fscookie *, struct dircookie *);
};

/*
** Handler on a mounted filesystem
*/
struct fs_mount
{
	struct list_node node;

	char *path;
	struct bdev *device;
	struct fscookie *fscookie;
	int ref_count;
	struct fs_api *api;
};

/*
** Handler on an opened file
** Used for reading, writing etc.
*/
struct filehandler
{
	struct filecookie *filecookie;
	struct fs_mount *mount;
	size_t offset;
};

/*
** Handler on an opened directory
** Used for reading, listing etc.
*/
struct dirhandler
{
	struct dircookie *dircookie;
	struct fs_mount *mount;
};

/*
** A directory entry, as returned by the readdir syscall.
*/
struct dirent
{
	char name[256];
	bool dir;
};

/* Generic fs functions */
status_t		fs_mount(char const *p, char const *fs, char const *dev);
status_t		fs_unmount(char const *path);
status_t		fs_open(char const *path, struct filehandler **handler);
status_t		fs_close(struct filehandler *filehandler);
status_t		fs_opendir(char const *, struct dirhandler **);
status_t		fs_readdir(struct dirhandler *handler, struct dirent *dirent);
status_t		fs_closedir(struct dirhandler *dirhandler);
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

