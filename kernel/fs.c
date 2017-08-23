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

extern struct fs_hook const __start_fs_hook[] __weak;
extern struct fs_hook const __end_fs_hook[] __weak;

/*
** Find the filesystem implementation for the given fs name
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
** Mounts the given file system api at the given path for the given device.
*/
static status_t
mount(char const *path, char const *device, struct fs_api *const api)
{
	struct fs_file *file;
	struct bdev *bdev;
	struct fscookie *cookie;
	struct fs_mount *mount;
	char *tmp;
	status_t err;

	bdev = NULL;
	tmp = strdup(path);
	if (unlikely(!tmp)) {
		return (ERR_NO_MEMORY);
	}
	resolve_path(tmp);
	file = find_file(tmp);
	kfree(tmp);

	if (file == NULL) {
		return (ERR_NOT_FOUND);
	}
	else if (!(file->type & FS_DIRECTORY)) {
		return (ERR_NOT_DIRECTORY);
	}
	else if (file->type & FS_MOUNTPOINT) {
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
	memset(mount, 0, sizeof(*mount));
	file->type |= FS_MOUNTPOINT;
	file->mount = mount;
	mount->bdev = bdev;
	mount->fscookie = cookie;
	mount->api = api;
	return (OK);
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
*/
status_t
fs_unmount(char const *path)
{
	char *tmp;
	struct fs_file *mountpoint;

	tmp = strdup(path);
	if (unlikely(!tmp)) {
		return (ERR_NO_MEMORY);
	}
	resolve_path(tmp);
	mountpoint = find_file(tmp);
	kfree(tmp);
	if (!mountpoint) {
		return (ERR_NOT_FOUND);
	}
	if (!(mountpoint->type & FS_MOUNTPOINT)) {
		return (ERR_INVALID_ARGS);
	}
	/* TODO do unmount */
	return (OK);
}

/*
** Opens the file at the given path.
*/
status_t
fs_open(char const *path, struct filehandler **handler)
{
	struct filehandler *fh;
	struct filecookie *cookie;
	struct fs_file *file;
	struct fs_mount *mount;
	status_t err;
	char *tmp;

	tmp = strdup(path);
	if (!tmp) {
		return (ERR_NO_MEMORY);
	}

	resolve_path(tmp);
	file = find_file(tmp);
	kfree(tmp);
	if (!file) {
		return (ERR_NOT_FOUND);
	}

	mount = file->mount;
	err = mount->api->open(mount->fscookie, tmp, &cookie);
	if (err) {
		return (err);
	}

	fh = kalloc(sizeof(struct filehandler));
	if (fh == NULL) {
		return (ERR_NO_MEMORY);
	}

	fh->file = file;
	fh->offset = 0;
	*handler = fh;
	return (OK);
}

/*
** Closes the given handler, and kfrees it.
*/
status_t
fs_close(struct filehandler *handler)
{
	if (handler->file->mount) {
		handler->file->mount->api->close(handler->file->cookie);
	}
	kfree(handler);
	return (OK);
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

	init_vfs();

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
