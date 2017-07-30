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
*/
virt_addr_t
kalloc(size_t size)
{
	struct block *block;

	assert(sizeof(struct block) % sizeof(void *) == 0);
	size += (size == 0);
	size = ALIGN(size, sizeof(void *));
	block = get_free_block(size);
	if (block) {
		block->used = true;
		split_block(block, size);
		return ((void *)((char *)block + sizeof(struct block)));
	}
	block = ksbrk(sizeof(struct block) + size);
	if (unlikely(block == (void *)-1u)) {
		return (NULL);
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
	return ((void *)((char *)block + sizeof(struct block)));
}

/*
** free(), but using memory in kernel space.
*/
void
kfree(virt_addr_t ptr)
{
	struct block *block;

	if (ptr)
	{
		block = (struct block *)((char *)ptr - sizeof(struct block));
		assert(block->used);
		block->used = false;
		join_block(block);
		if (block->prev) {
			join_block(block->prev);
		}
	}
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
