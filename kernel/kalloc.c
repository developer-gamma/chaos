/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/kalloc.h>
#include <kernel/spinlock.h>
#include <kernel/init.h>
#include <stdio.h>
#include <string.h>

/*
** Kernel memory allocator.
** This is NOT suitable for user-space memory allocation.
**
** This is a pretty naive and straightforward implementation.
** It can definitely be improved, but i didn't want to waste
** time doing it. Feel free to improve it :)
*/

/* Malloc's data structures */
struct alloc_datas alloc_datas =
{
	.head = (void *)1u,
	.tail = NULL,
};

static struct spinlock kernel_heap_lock;


/*
** Looks for a free block that can contain at least the given size.
*/
static struct block *
get_free_block(size_t size)
{
	struct block *block;

	block = alloc_datas.head;
	while (block <= alloc_datas.tail)
	{
		if (!block->used && block->size >= size)
			return (block);
		block = (struct block *)((char *)block + sizeof(struct block) + block->size);
	}
	return (NULL);
}

/*
** Split the given block at the given size, if possible.
** Note that size must not be greater that the block size.
*/
static void
split_block(struct block *block, size_t size)
{
	struct block *new;
	struct block *next;

	if (block->size - size > sizeof(struct block) + 1)
	{
		new = (struct block *)((char *)block + sizeof(struct block) + size);
		new->used = false;
		new->size = block->size - size - sizeof(struct block);
		if (alloc_datas.tail == block) {
			alloc_datas.tail = new;
		}
		block->size = size;
		new->prev = block;

		next = (struct block *)((char *)new + sizeof(struct block) + new->size);
		if (next <= alloc_datas.tail) {
			next->prev = new;
		}
	}
}

/*
** Try to join the given block with the next one if they are both free.
*/
static void
join_block(struct block *block)
{
	struct block *other;
	struct block *next;

	other = (struct block *)((char *)block + sizeof(struct block) + block->size);
	if (!block->used && other <= alloc_datas.tail && !other->used)
	{
		block->size += sizeof(struct block) + other->size;
		if (other == alloc_datas.tail) {
			alloc_datas.tail = block;
		}

		next = (struct block *)((char *)other + sizeof(struct block) + other->size);
		if (next <= alloc_datas.tail) {
			next->prev = block;
		}
	}
}

/*
** malloc(), but using memory in kernel space.
** TODO Make this function safer (overflow)
*/
virt_addr_t
kalloc(size_t size)
{
	struct block *block;

	LOCK_KHEAP(state);
	size += (size == 0);
	size = ALIGN(size, sizeof(void *));
	block = get_free_block(size);
	if (block) {
		block->used = true;
		split_block(block, size);
		goto ret_ok;
	}
	block = ksbrk(sizeof(struct block) + size);
	if (unlikely(block == (void *)-1u)) {
		goto ret_err;
	}
	block->used = true;
	block->size = size;
	block->prev = alloc_datas.tail;
	alloc_datas.tail = block;
	if (unlikely(alloc_datas.head == (void *)1u)) {
		alloc_datas.head = block;
	}
	if (block->prev) {
		join_block(block->prev);
	}

ret_ok:
	RELEASE_KHEAP(state);
	return ((void *)((char *)block + sizeof(struct block)));

ret_err:
	RELEASE_KHEAP(state);
	return (NULL);


}

/*
** free(), but using memory in kernel space.
** TODO Make this function safer (overflow)
*/
void
kfree(virt_addr_t ptr)
{
	struct block *block;

	if (ptr)
	{
		LOCK_KHEAP(state);
		block = (struct block *)((char *)ptr - sizeof(struct block));
		assert(block->used);
		block->used = false;
		join_block(block);
		if (block->prev) {
			join_block(block->prev);
		}
		RELEASE_KHEAP(state);
	}
}

/*
** realloc(), but using memory in kernel space.
** TODO Make this function safer (overflow)
*/
virt_addr_t
krealloc(virt_addr_t old, size_t ns)
{
	void *ptr;
	struct block *block;

	ptr = kalloc(ns);
	if (ptr != NULL && old) {
		block = (struct block *)((char *)old - sizeof(struct block));
		memcpy(ptr, old, block->size > ns ? ns : block->size);
		kfree(old);
	}
	return (ptr);
}

/*
** calloc(), but using memory in kernel space.
** TODO Make this function safer (overflow)
*/
virt_addr_t
kcalloc(size_t a, size_t b)
{
	void *ptr;

	ptr = kalloc(a * b);
	if (likely(ptr != NULL)) {
		memset(ptr, 0, a * b);
	}
	return (ptr);
}

static void
init_kmalloc(enum init_level il __unused)
{
	init_lock(&kernel_heap_lock);
	printf("[OK]\tKernel Heap\n");
}

NEW_INIT_HOOK(kmalloc, &init_kmalloc, CHAOS_INIT_LEVEL_VMM + 1);
