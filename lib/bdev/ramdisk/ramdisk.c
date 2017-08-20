/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/init.h>
#include <lib/bdev/ramdisk.h>
#include <stdio.h>

static ssize_t
bdev_read(struct bdev *bdev, void *buf, size_t offset, size_t len)
{
	(void)bdev;
	(void)buf;
	(void)offset;
	(void)len;
	return (-1);
}

static ssize_t
bdev_write(struct bdev *bdev, void const *buf, size_t offset, size_t len)
{
	(void)bdev;
	(void)buf;
	(void)offset;
	(void)len;
	return (-1);
}

static struct bdev bdev_ramdisk =
{
	.node = LIST_CLEAR_VALUE,
	.name = "ramdisk",
	.read = &bdev_read,
	.write = &bdev_write,
};

void
init_ramdisk(enum init_level il __unused)
{
	printf("[OK]\tRamdisk\n");
	bdev_register(&bdev_ramdisk);
}

NEW_INIT_HOOK(ramdisk, &init_ramdisk, CHAOS_INIT_LEVEL_BDEV);
