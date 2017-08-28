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
#include <kernel/thread.h>
#include <arch/common_op.h>
#include <lib/bdev/mem.h>
#include <stdio.h>
#include <string.h>

extern struct fs_hook const __start_fs_hook[] __weak;
extern struct fs_hook const __end_fs_hook[] __weak;

struct list_node mounts = LIST_INIT_VALUE(mounts);

static char *
resolve_input(char const *cwd, char const *input)
{
	size_t len;
	char *out;

	if (*input == '/') {
		out = strdup(input);
	} else {
		len = strlen(cwd) + strlen(input) + 2;
		out = kalloc(len * sizeof(char));
		if (unlikely(!out)) {
			return (NULL);
		}
		strcpy(out, cwd);
		strcat(out, "/");
		strcat(out, input);
	}
	return (out);
}

/*
**
** Resolves the given path, by removing double separators, '.' and '..'.
**
** Inspired by lk's 'fs_normalize_path()'
*/
static void
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
	} state = INITIAL;
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

/*
** Find the filesystem implementation for the given filesystem name
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
** Finds the mount structure holding the given path, and stores
** the remaining path in 'trimmed_path'.
**
** Bumps the reference counter before returning.
*/
static struct fs_mount *
find_mount(char const *path, char const **trimmed_path)
{
	struct fs_mount *mount;
	size_t pathlen;
	size_t mount_pathlen;

	pathlen = strlen(path);
	list_foreach_content(mount, &mounts, node) {
		mount_pathlen = strlen(mount->path);

		if (pathlen < mount_pathlen)
			continue;
		if (memcmp(path, mount->path, mount_pathlen) == 0) {
			if (trimmed_path)
				*trimmed_path = path + mount_pathlen;
			mount->ref_count++;
			return (mount);
		}
	}
	return (NULL);
}

/*
** Decrements the reference counter of the given mount structure,
** eventually causing a unmount operation if it reaches 0.
*/
static void
put_mount(struct fs_mount *mount)
{
	--mount->ref_count;
	if (mount->ref_count == 0) {
		list_delete(&mount->node);
		mount->api->unmount(mount->fscookie);
		kfree(mount->path);
		bdev_close(mount->device);
		kfree(mount);
	}
}

/*
** Mounts the given file system api at the given path for the given device.
*/
static status_t
mount(char const *path, char const *device, struct fs_api *const api)
{
	struct bdev *bdev;
	struct fscookie *cookie;
	struct fs_mount *mount;
	char const *relative;
	char *tmp;
	status_t err;

	bdev = NULL;
	mount = NULL;

	tmp = resolve_input(get_current_thread()->cwd, path);
	if (unlikely(!tmp)) {
		return (ERR_NO_MEMORY);
	}
	resolve_path(tmp);

	mount = find_mount(tmp, &relative);
	if (mount) {
		err = ERR_ALREADY_MOUNTED;
		goto err;
	}

	bdev = bdev_open(device);
	if (!bdev) {
		err = ERR_NOT_FOUND;
		goto err;
	}

	err = api->mount(bdev, &cookie);
	if (err) {
		goto err;
	}

	mount = kalloc(sizeof(struct fs_mount));
	if (unlikely(!mount)) {
		err = ERR_NO_MEMORY;
		goto err;
	}
	memset(mount, 0, sizeof(*mount));
	mount->path = tmp;
	mount->device= bdev;
	mount->fscookie = cookie;
	mount->ref_count = 1;
	mount->api = api;
	list_add(&mount->node, &mounts);
	return (OK);

err:
	if (mount) {
		put_mount(mount);
	}
	if (bdev) {
		bdev_close(bdev);
	}
	kfree(tmp);
	return (err);
}

/*
** Mounts a filesystem at a given path.
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
**
** TODO Recursive unmount
*/
status_t
fs_unmount(char const *path)
{
	char *tmp;
	struct fs_mount *mount;

	tmp = resolve_input(get_current_thread()->cwd, path);
	if (unlikely(!tmp)) {
		return (ERR_NO_MEMORY);
	}
	resolve_path(tmp);
	mount = find_mount(tmp, NULL);
	kfree(tmp);
	if (!mount) {
		return (ERR_NOT_FOUND);
	}
	put_mount(mount);
	if (mount->ref_count > 1) {
		return (ERR_TARGET_BUSY);
	} else {
		put_mount(mount);
	}
	return (OK);
}

/*
** Opens the file at the given path
*/
status_t
fs_open(char const *path, struct filehandler **handler)
{
	struct filehandler *fh;
	struct fs_mount *mount;
	char *tmp;
	char const *newpath;
	status_t err;

	mount = NULL;
	fh = NULL;
	tmp = resolve_input(get_current_thread()->cwd, path);
	if (unlikely(!tmp)) {
		return (ERR_NO_MEMORY);
	}
	resolve_path(tmp);
	mount = find_mount(tmp, &newpath);
	if (!mount) {
		err = ERR_NOT_FOUND;
		goto err;
	}

	fh = kalloc(sizeof(struct filehandler));
	if (fh == NULL) {
		err = ERR_NO_MEMORY;
		goto err;
	}
	memset(fh, 0, sizeof(*fh));
	fh->mount = mount;

	err = mount->api->open(mount->fscookie, newpath, fh);
	if (err) {
		goto err;
	}
	*handler = fh;
	return (OK);

err:
	kfree(tmp);
	if (mount) {
		put_mount(mount);
	}
	kfree(fh);
	return (err);
}

/*
** Closes the given handler, and kfrees it.
*/
status_t
fs_close(struct filehandler *handler)
{
	status_t err;
	struct fs_mount *mount;

	mount = handler->mount;
	err = mount->api->close(mount->fscookie, handler);
	if (err) {
		return (err);
	}
	put_mount(mount);
	kfree(handler);
	return (OK);
}

status_t
fs_readdir(struct filehandler *handler, struct dirent *dirent)
{
	struct fs_mount *mount;

	if (!handler->dir) {
		return (ERR_NOT_DIRECTORY);
	}
	mount = handler->mount;
	return (mount->api->readdir(mount->fscookie, handler->dircookie, dirent));
}

/*
** Duplicates the given file handler into a new one kheap-allocated.
*/
struct filehandler *
fs_dup_handler(struct filehandler const *handler)
{
	struct filehandler *nh;

	nh = kalloc(sizeof(*nh));
	if (nh != NULL) {
		memcpy(nh, handler, sizeof(*nh));
	}
	return (nh);
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
