/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/fs.h>
#include <kernel/bdev.h>
#include <kernel/list.h>
#include <kernel/init.h>
#include <kernel/kalloc.h>
#include <kernel/multiboot.h>
#include <arch/common_op.h>
#include <lib/bdev/mem.h>
#include <stdio.h>
#include <string.h>

static struct list_node mounts = LIST_INIT_VALUE(mounts);

extern struct fs_hook const __start_fs_hook[] __weak;
extern struct fs_hook const __end_fs_hook[] __weak;

static void resolve_path(char *path);

/*
** Find the filesystem implementation with the given name
*/
static struct fs_hook const *
find_fs(char const *name)
{
	struct fs_hook const *hook;

	hook = __start_fs_hook;
	while (hook != __end_fs_hook)
	{
		if (!strcmp(hook->name, name)) {
			return (hook);
		}
		++hook;
	}
	return (NULL);
}

/*
** Searches the mount holding the given path
** Bump the reference counter of the mount before returning
*/
static struct fs_mount *find_mount(char const *path)
{
	struct fs_mount *mount;
	size_t path_len;
	size_t mount_path_len;

	path_len = strlen(path);
	list_foreach_content(mount, &mounts, node) {

		mount_path_len = strlen(mount->path);
		if (path_len < mount_path_len)
			continue;

		if (!strncmp(mount->path, path, mount_path_len)) {
			atomic_add(&mount->ref_count, 1);
			return (mount);
		}
	}
	return (NULL);
}

/*
** Decrements the reference counter of the mount structure, which
** may cause an unmount operation.
*/
static void
put_mount(struct fs_mount *mount)
{
	if (atomic_add(&mount->ref_count, -1) == 0) {
		list_delete(&mount->node);
		mount->api->unmount(mount->cookie);
		if (mount->bdev) {
			bdev_close(mount->bdev);
		}
		kfree(mount->path);
		kfree(mount);
	}
}

/*
** Mounts the given file system api at the given path for the given device.
*/
static status_t
mount(char const *path, char const *device, struct fs_api *const api)
{
	struct fs_mount *mount;
	struct bdev *bdev;
	struct fscookie *cookie;
	char *tmp;
	status_t err;

	bdev = NULL;
	tmp = strdup(path);
	if (unlikely(!tmp)) {
		return (ERR_NO_MEMORY);
	}
	resolve_path(tmp);
	mount = find_mount(path);
	kfree(tmp);

	if (mount) {
		return (ERR_ALREADY_MOUNTED);
	}

	bdev = bdev_open(device);
	if (!bdev) {
		return (ERR_NOT_FOUND);
	}

	err = api->mount(bdev, &cookie);
	if (err) {
		bdev_close(bdev);
		return (err);
	}

	mount = kalloc(sizeof(struct fs_mount));
	if (unlikely(!mount)) {
		bdev_close(bdev);
		return (ERR_NO_MEMORY);
	}
	mount->path = strdup(path);
	mount->bdev = bdev;
	mount->cookie = cookie;
	mount->api = api;
	mount->ref_count = 1;

	list_add(&mount->node, &mounts);
	return (OK);
}

/*
** Mount a filesystem at a given path.
*/
status_t
fs_mount(char const *path, char const *fs_name, char const *device)
{
	struct fs_hook const *hook;

	hook = find_fs(fs_name);
	if (!hook) {
		return (ERR_NOT_FOUND);
	}
	return (mount(path, device, hook->api));
}

/*
** Unmounts a filesystem mounted at the given path.
*/
status_t
fs_unmount(char const *path)
{
	char *tmp;
	struct fs_mount *mount;

	tmp = strdup(path);
	if (unlikely(!tmp)) {
		return (ERR_NO_MEMORY);
	}
	resolve_path(tmp);
	mount = find_mount(path);
	kfree(tmp);
	if (!mount) {
		return (ERR_NOT_FOUND);
	}

	put_mount(mount); /* First one for the find_mount() that brought us here*/
	put_mount(mount); /* Second one for mount() when creating the mount point */
	return (OK);
}

status_t
fs_open(char const *path, struct filehandler **handler)
{
	struct filehandler *fh;
	struct filecookie *cookie;
	struct fs_mount *mount;
	status_t err;
	char *tmp;

	tmp = strdup(path);
	if (!tmp) {
		return (ERR_NO_MEMORY);
	}
	resolve_path(tmp);

	mount = find_mount(tmp);
	kfree(tmp);
	if (!mount) {
		return (ERR_NOT_FOUND);
	}

	err = mount->api->open(mount->cookie, tmp, &cookie);
	if (err) {
		put_mount(mount);
		return (err);
	}

	fh = kalloc(sizeof(struct filehandler));
	fh->cookie = cookie;
	fh->mount = mount;
	*handler = fh;
	return (OK);
}

status_t
fs_close(struct filehandler *handler)
{
	status_t err;

	err = handler->mount->api->close(handler->cookie);
	if (err) {
		return (err);
	}
	put_mount(handler->mount);
	kfree(handler);
	return (OK);
}

/*
**
** Resolves the given path, by removing double separators, '.' and '..'.
**
** Inspired by lk's 'fs_normalize_path()'
*/
void
resolve_path(char *path)
{
	int outpos;
	int pos;
	bool done;
	char c;

	enum {
		INITIAL,
		FIELD_START,
		IN_FIELD,
		SEPARATOR,
		SEEN_SEPARATOR,
		DOT,
		SEEN_DOT,
		DOTDOT,
		SEEN_DOTDOT,
	} state;

	state = INITIAL;
	pos = 0;
	outpos = 0;
	done = false;

	/* Resolve the given path */
	while (!done)
	{
		c = path[pos];
		switch (state) {
		case INITIAL:
			if (c == '/') {
				state = SEPARATOR;
			}
			else if (c == '.') {
				state = DOT;
			} else {
				state = FIELD_START;
			}
			break;
		case FIELD_START:
			if (c == '.') {
				state = DOT;
			} else if ( c == '\0') {
				done = true;
			} else {
				state = IN_FIELD;
			}
			break;
		case IN_FIELD:
			if (c == '/') {
				state = SEPARATOR;
			} else if (c == '\0') {
				done = true;
			} else {
				path[outpos++] = c;
				pos++;
			}
			break;
		case SEPARATOR:
			path[outpos++] = '/';
			pos++;
			state = SEEN_SEPARATOR;
			break;
		case SEEN_SEPARATOR:
			if (c == '/') {
				++pos;
			} else if (c == '\0') {
				done = true;
			} else {
				state = FIELD_START;
			}
			break;
		case DOT:
			pos++;
			state = SEEN_DOT;
			break;
		case SEEN_DOT:
			if (c == '.') {
				state = DOTDOT;
			} else if (c == '/') { /* Filename == '.', eat it */
				++pos;
				state = SEEN_SEPARATOR;
			} else if (c == '\0') {
				done = true;
			} else { /* Filename starting with '.' */
				path[outpos++] = '.';
				state = IN_FIELD;
			}
			break;
		case DOTDOT:
			pos++;
			state = SEEN_DOTDOT;
			break;
		case SEEN_DOTDOT:
			if (c == '/' || c == '\0') { /* Filename == '..' */
				outpos -= (outpos > 0);
				while (outpos > 0) { /* Walk backward */
					if (path[outpos - 1] == '/') {
						break;
					}
					--outpos;
				}
				pos++;
				state = SEEN_SEPARATOR;
				if (c == '\0') {
					done = true;
				}
			} else { /* Filename starting with '.. */
				path[outpos++] = '.';
				path[outpos++] = '.';
				state = IN_FIELD;
			}
			break;
		}
	}

	/* Remove trailing slash */
	if (outpos == 0) {
		path[outpos++] = '/';
	} else if (outpos > 1 && path[outpos - 1] == '/') {
		--outpos;
	}

	path[outpos] = 0;
}

static void
init_fs(enum init_level il __unused)
{
	printf("[..]\tFilesystem");
	if (multiboot_infos.initrd.present)
	{
		/* Set up initrd */
		assert_eq(register_membdev(
				"initrd",
				multiboot_infos.initrd.vstart,
				multiboot_infos.initrd.size
			), OK);

		/* Mount initrd on root */
		assert_eq(fs_mount("/", "fat16", "initrd"), OK);
	}
	printf("\r[OK]\tFilestem ('/' mounted)\n");
}

NEW_INIT_HOOK(filesystem, &init_fs, CHAOS_INIT_LEVEL_FILESYSTEM);
