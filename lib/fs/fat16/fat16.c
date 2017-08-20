/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <lib/fs/fat16.h>

/* Debug */
#include <stdio.h>

static status_t
fat16_mount(struct bdev *bdev __unused, struct fscookie **fscookie __unused)
{
	return (OK);
}

static status_t
fat16_unmount(struct fscookie *cookie __unused)
{
	return (OK);
}

static status_t
fat16_open(struct fscookie *cookie __unused, char const *path __unused, struct filecookie **cokie __unused)
{
	return (OK);
}

static status_t
fat16_close(struct filecookie *cookie __unused)
{
	return (OK);
}

static struct fs_api fat16_api =
{
	.mount = &fat16_mount,
	.unmount = &fat16_unmount,
	.open = &fat16_open,
	.close = &fat16_close,
};

NEW_FILESYSTEM(fat16, &fat16_api);
