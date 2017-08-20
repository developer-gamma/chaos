/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/init.h>
#include <kernel/kalloc.h>
#include <lib/bdev/mem.h>
#include <stdio.h>
#include <string.h>

static ssize_t
mem_read(struct bdev *bdev, void *buf, size_t offset, size_t len)
{
	struct mem_bdev *mem;

	mem = (struct mem_bdev *)bdev;
	memcpy(buf, (uint8 *)mem->ptr + offset, len);
	return (len);
}

static ssize_t
mem_write(struct bdev *bdev, void const *buf, size_t offset, size_t len)
{
	struct mem_bdev *mem;

	mem = (struct mem_bdev *)bdev;
	memcpy((uint8 *)mem->ptr + offset, buf, len);
	return (len);
}

status_t
register_membdev(char const *name, void *ptr, size_t len)
{
	struct mem_bdev *dev;
	status_t ret;

	dev = kalloc(sizeof(struct mem_bdev));
	if (dev == NULL) {
		return (ERR_NO_MEMORY);
	}
	ret = bdev_init(&dev->bdev,
			name,
			MEM_BLOCKSIZE,
			len / MEM_BLOCKSIZE,
			BDEV_FLAGS_NONE
	);
	if (ret) {
		kfree(dev);
		return (ret);
	}

	dev->bdev.read = &mem_read;
	dev->bdev.write = &mem_write;
	dev->ptr = ptr;
	bdev_register(&dev->bdev);
	return (OK);
}
