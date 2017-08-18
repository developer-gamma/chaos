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

/*
** This implementation is inspired by geist's work on LittleKernel.
** Huge thanks to him :)
*/

struct fscookie;
struct filecookie;
struct dircookie;
struct bdev;

struct fs_api
{
	status_t (*mount)(struct bdev *, struct fscookie **);
	status_t (*unmount)(struct fscookie *);
};

struct fs_mount
{
	struct list_node node;

	char *path;
	struct bdev *bdev;
	struct fscookie *cookie;
	const struct fs_api *api;
};

struct filehandler
{
	struct fscookie *cookie;
	struct fs_mount *mount;
};

struct dirhandler
{
	struct dircookie *cookie;
	struct fs_mount *mount;
};

struct fs_hook
{
	char const *name;
	struct fs_api *api;
};

# define NEW_FILESYSTEM(name, api) \
	__aligned(sizeof(void*)) __used __section("fs_hook")		\
	static const struct fs_hook _fs_hook_##name = {			\
		.name = #name,						\
		.api = api,						\
	}


#endif /* !_KERNEL_FS_H_ */

