/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

/*
** Architecture independant virtual memory management.
*/

#include <kernel/init.h>
#include <kernel/vmm.h>
#include <kernel/thread.h>
#include <kernel/interrupts.h>
#include <stdio.h>
#include <string.h>

/* Heap main variables */
virt_addr_t kernel_heap_start;
size_t kernel_heap_size;

/*
** Map contiguous virtual addresses to a random physical addresses.
** In case of error, the state mush be as it was before the call.
** If the given virtual address is NULL, then the kernel chooses
** the destination address.
** Size must be page aligned.
**
** Returns the virtual address holding the mapping, or NULL if
** it fails.
**
** Weak symbol, can be re-implemented for each supported architecture, but
** a default implemententation is given.
*/
__weak virt_addr_t
mmap(virt_addr_t va, size_t size)
{
	virt_addr_t ori_va;
	struct vaspace *vaspace;

	assert(IS_PAGE_ALIGNED(va));
	assert(IS_PAGE_ALIGNED(size));

	LOCK_VASPACE(state);

	ori_va = va;
	if (va == NULL) /* Allocate on the memory mapping segment */
	{
		/* TODO Use unmaped memory of the Memory Mapping segment */
		vaspace = get_current_thread()->vaspace;
		ori_va = mmap((char *)vaspace->mmapping_start - vaspace->mmapping_size - size, size);
		if (ori_va != NULL) {
			vaspace->mmapping_size += size;
		}
		goto ok_ret;
	}
	else
	{
		while (va < ori_va + size)
		{
			if (unlikely(arch_map_page(va) != OK)) {
				arch_munmap(ori_va, va - ori_va);
				goto err_ret;
			}
			va += PAGE_SIZE;
		}
		goto ok_ret;
	}

ok_ret:
	RELEASE_VASPACE(state);
	return (ori_va);

err_ret:
	RELEASE_VASPACE(state);
	return (NULL);
}

/*
** Sets the new end of kernel heap.
** Interrupts must be disable in order to call this function.
**
** TODO Make this function safer (overflow, bounds)
*/
status_t
kbrk(virt_addr_t new_brk)
{
	virt_addr_t brk;
	intptr add;
	intptr round_add;

	assert(!are_int_enabled());
	if (new_brk >= kernel_heap_start)
	{
		add = new_brk - (kernel_heap_start + kernel_heap_size);
		new_brk = (virt_addr_t)ROUND_DOWN((uintptr)new_brk, PAGE_SIZE);
		brk = (virt_addr_t)(ROUND_DOWN((uintptr)(kernel_heap_start + kernel_heap_size), PAGE_SIZE));
		round_add = new_brk - brk;
		kernel_heap_size += add;
		if (round_add > 0)
		{
			if (unlikely(mmap(brk + PAGE_SIZE, round_add) == NULL)) {
				kernel_heap_size -= add;
				return (ERR_NO_MEMORY);
			}
		}
		else if (round_add < 0) {
			arch_munmap(brk + round_add + PAGE_SIZE, -round_add);
		}
		return (OK);
	}
	return (ERR_INVALID_ARGS);
}

/*
** Increments or decrement the kernel heap of 'inc' bytes.
** Interrupts must be disable in order to call this function.
**
** TODO Make this function safer (overflow, bounds)
*/
virt_addr_t
ksbrk(intptr inc)
{
	void *old_brk;

	old_brk = kernel_heap_start + kernel_heap_size;
	if (kbrk(kernel_heap_start + kernel_heap_size + inc) == OK) {
		return (old_brk);
	}
	return ((virt_addr_t)-1u);
}

/*
** Sets the new end of user heap.
**
** TODO Make this function safer (overflow, bounds)
*/
status_t
ubrk(virt_addr_t new_brk)
{
	virt_addr_t brk;
	intptr add;
	intptr round_add;
	struct vaspace *vaspace;

	LOCK_VASPACE(state);
	vaspace = get_current_thread()->vaspace;
	if (new_brk >= vaspace->heap_start)
	{
		add = new_brk - (vaspace->heap_start + vaspace->heap_size);
		new_brk = (virt_addr_t)ROUND_DOWN((uintptr)new_brk, PAGE_SIZE);
		brk = (virt_addr_t)(ROUND_DOWN((uintptr)(vaspace->heap_start + vaspace->heap_size), PAGE_SIZE));
		round_add = new_brk - brk;
		vaspace->heap_size += add;
		if (round_add > 0)
		{
			if (unlikely(mmap(brk + PAGE_SIZE, round_add) == NULL)) {
				vaspace->heap_size -= add;
				RELEASE_VASPACE(state);
				return (ERR_NO_MEMORY);
			}
		}
		else if (round_add < 0) {
			arch_munmap(brk + round_add + PAGE_SIZE, -round_add);
		}
		RELEASE_VASPACE(state);
		return (OK);
	}
	RELEASE_VASPACE(state);
	return (ERR_INVALID_ARGS);
}

/*
** Increments or decrement the user heap of 'inc' bytes.
** TODO Make this function safer (overflow, bounds)
*/
virt_addr_t
usbrk(intptr inc)
{
	void *old_brk;
	struct vaspace *vaspace;

	vaspace = get_current_thread()->vaspace;
	LOCK_VASPACE(state);
	old_brk = vaspace->heap_start + vaspace->heap_size;
	if (ubrk(vaspace->heap_start + vaspace->heap_size + inc) == OK) {
		RELEASE_VASPACE(state);
		return (old_brk);
	}
	RELEASE_VASPACE(state);
	return ((virt_addr_t)-1u);
}

/*
** Initalises the arch-independant stuff of virtual memory management.
** Calls the arch-dependent vmm init function.
*/
static void
vmm_init(enum init_level il __unused)
{
	/* Set-up kernel heap */
	kernel_heap_start = (virt_addr_t)ALIGN((uintptr)KERNEL_VIRTUAL_END, 1024u * PAGE_SIZE);
	kernel_heap_size = 0;

	arch_vmm_init();

	/* Allocate the first heap page or the kbrk algorithm will not work. */
	assert_neq(mmap(kernel_heap_start, PAGE_SIZE), NULL);

	/* Defined in kernel/thread.c */
	extern struct vaspace default_vaspace;

	/* Allocate the first heap page of boot thread the ubrk algorithm will not work. */
	assert_neq(mmap(default_vaspace.heap_start, PAGE_SIZE), NULL);

	printf("[OK]\tVirtual Memory Management\n");
}

NEW_INIT_HOOK(vmm, &vmm_init, CHAOS_INIT_LEVEL_VMM);
