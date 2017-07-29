/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/alloc.h>
#include <kernel/spinlock.h>
#include <kernel/init.h>
#include <stdio.h>

/*
** Kernel memory allocator.
** This is NOT suitable for user-space memory allocation.
*/

/* Malloc's data structures */
struct alloc_datas alloc_datas =
{
	.head = (void *)1u,
	.tail = NULL,
};

/*
** malloc(), but using memory in kernel space.
*/
virt_addr_t
kalloc(size_t size)
{
	(void)size;
	return (NULL);
}

/*
** free(), but using memory in kernel space.
*/
void
kfree(virt_addr_t ptr)
{
	(void)ptr;
}

/*
** realloc(), but using memory in kernel space.
*/
virt_addr_t
krealloc(virt_addr_t old, size_t ns)
{
	(void)old;
	(void)ns;
	return (NULL);
}

/*
** calloc(), but using memory in kernel space.
*/
virt_addr_t
kcalloc(size_t a, size_t b)
{
	(void)a;
	(void)b;
	return (NULL);
}

static void
init_kmalloc(enum init_level il __unused)
{
	printf("[OK]\tKernel Heap\n");
}

NEW_INIT_HOOK(kmalloc, &init_kmalloc, CHAOS_INIT_LEVEL_VMM + 1);
