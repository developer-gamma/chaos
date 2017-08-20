/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#ifndef _KERNEL_BDEV_H_
# define _KERNEL_BDEV_H_

# include <kernel/list.h>

struct bdev
{
	struct list_node node;
	char const *name;

	/* api */
	ssize_t (*read)(struct bdev *, void *buf, size_t offset, size_t len);
	ssize_t (*write)(struct bdev *, void const *buf, size_t offset, size_t len);
	void (*close)(struct bdev *);
};

struct bdev		*bdev_open(char const *name);
void			bdev_close(struct bdev *);
void			bdev_register(struct bdev *bdev);
void			bdev_unregister(struct bdev *bdev);

#endif /* !_KERNEL_BDEV_H_ */
