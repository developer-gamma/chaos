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
**
** This is a naïve implementation, so yeah, it sucks.
*/

/* Malloc's linked list */
static struct malloc_node *first = NULL;
static struct malloc_node *last = NULL;
static struct spinlock lock;

/*
** malloc(), but with memory in kernel space.
*/
virt_addr_t
kmalloc(size_t size)
{
	acquire_lock(&lock);
	release_lock(&lock);
	return (NULL);
}

/*
** free(), but with memory in kernel space.
*/
void
kfree(virt_addr_t ptr)
{
	acquire_lock(&lock);
	release_lock(&lock);
}

static void
init_kmalloc(enum init_level il __unused)
{
	printf("[OK]\tKernel Heap\n");
}

NEW_INIT_HOOK(kmalloc, &init_kmalloc, CHAOS_INIT_LEVEL_VMM + 1);
