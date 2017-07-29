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

struct block
{
	bool used;
	size_t size;
	struct block *prev;
};

struct alloc_datas
{
	struct block *head;
	struct block *tail;
};

virt_addr_t	kalloc(size_t);
virt_addr_t	krealloc(virt_addr_t, size_t);
virt_addr_t	kcalloc(size_t, size_t);
void		kfree(virt_addr_t);

#endif /* !_KERNEL_ALLOC_H_ */
