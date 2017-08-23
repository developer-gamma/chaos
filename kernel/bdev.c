/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/bdev.h>
#include <kernel/kalloc.h>
#include <arch/common_op.h>
#include <string.h>

static struct list_node bdev_list = LIST_INIT_VALUE(bdev_list);

static inline void
bdev_inc_ref(struct bdev *bdev)
{
	atomic_add(&bdev->ref_count, 1);
}

static inline void
bdev_dec_ref(struct bdev *bdev)
{
	if (atomic_add(&bdev->ref_count, -1) == 1) {
		if (bdev->close) {
			bdev->close(bdev);
		}
		kfree(bdev->name);
	}
}

status_t
bdev_init(struct bdev *bdev,
	  char const *name,
	  size_t block_size,
	  size_t block_count,
	  uint flags)
{
	bdev->name = strdup(name);
	if (bdev->name == NULL) {
		return (ERR_NO_MEMORY);
	}
	bdev->block_size = block_size;
	bdev->block_count = block_count;
	bdev->flags = flags;
	bdev->ref_count = 0;
	return (OK);
}

struct bdev *
bdev_open(char const *name)
{
	struct bdev *bdev;

	list_foreach_content(bdev, &bdev_list, node) {
		if (!strcmp(name, bdev->name)) {
			bdev_inc_ref(bdev);
			return (bdev);
		}
	}
	return (NULL);
}

ssize_t
bdev_read(struct bdev *dev, void *buff, size_t offset, size_t len)
{
	/* TODO bound checking */
	return (dev->read(dev, buff, offset, len));
}

ssize_t
bdev_write(struct bdev *dev, void const *buff, size_t offset, size_t len)
{
	/* TODO bound checking */
	return (dev->write(dev, buff, offset, len));
}

void
bdev_close(struct bdev *bdev)
{
	bdev_dec_ref(bdev);
}

void
bdev_register(struct bdev *bdev)
{
	bdev_inc_ref(bdev);
	list_add_tail(&bdev->node, &bdev_list);
}

void
bdev_unregister(struct bdev *bdev)
{
	list_delete(&bdev->node);
	bdev_dec_ref(bdev);
}
