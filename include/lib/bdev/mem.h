/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#ifndef _LIB_BDEV_MEM_H_
# define _LIB_BDEV_MEM_H_

# include <kernel/bdev.h>

# define MEM_BLOCKSIZE		512

struct mem_bdev
{
	struct bdev bdev;	/* Base device */
	void *ptr;		/* Point where this device starts in memory */
};

status_t		register_membdev(char const *, void *, size_t);

#endif /* !_LIB_BDEV_MEM_H_ */
