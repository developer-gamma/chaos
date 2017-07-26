/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#ifndef _KERNEL_ALLOC_H_
# define _KERNEL_ALLOC_H_

# include <kernel/vmm.h>
# include <chaosdef.h>

enum malloc_node_status
{
	NODE_TAKEN,
	NODE_FREE,
};

struct malloc_node
{
	struct malloc_node *next;
	struct malloc_node *prev;
	size_t size;
	enum malloc_node_status status;
};

virt_addr_t	kmalloc(size_t);
void		kfree(virt_addr_t);

#endif /* !_KERNEL_ALLOC_H_ */
