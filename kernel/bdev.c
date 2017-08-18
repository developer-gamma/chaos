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
#include <string.h>

static struct list_node bdev_list = LIST_INIT_VALUE(bdev_list);

struct bdev *
bdev_open(char const *name)
{
	struct bdev *bdev;

	list_foreach_content(bdev, &bdev_list, node) {
		if (!strcmp(name, bdev->name)) {
			return (bdev);
		}
	}
	return (NULL);
}

void
bdev_close(struct bdev *bdev)
{
	if (bdev->close) {
		bdev->close(bdev);
	}
	kfree(bdev->name);
}

void
bdev_register(struct bdev *bdev)
{
	list_add_tail(&bdev->node, &bdev_list);
}

void
bdev_unregister(struct bdev *bdev)
{
	list_delete(&bdev->node);
	bdev_close(bdev);
}


