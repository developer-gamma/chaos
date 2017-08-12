/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/thread.h>
#include <kernel/kalloc.h>
#include <arch/x86/tss.h>
#include <arch/x86/asm.h>
#include <arch/x86/vmm.h>
#include <string.h>
#include <stdio.h>

static struct thread *current_thread = NULL;
extern struct spinlock thread_table_lock;

/*
** Very first entry point of each thread. Calls the main entry point of the thread.
*/
static void
thread_main(void)
{
	/* Release the lock acquired by the thread_yield() that brought us here. */
	release_lock(&thread_table_lock);
	arch_enable_interrupts();

	/* User mode, never returns. still a WIP */
	//x86_jump_userspace(current_thread->entry);

	thread_exit(current_thread->entry());
}

/*
** Very first entry point of a child thread after a fork. Returns from the fork.
*/
static void
thread_return_fork(void)
{
	/* Release the lock acquired by the thread_yield() that brought us here. */
	release_lock(&thread_table_lock);
	arch_enable_interrupts();

	x86_return_userspace(current_thread->arch.iframe);
}

/*
** Initializes the thread.
*/
void
arch_init_thread(struct thread *t)
{
	virt_addr_t stack_top;
	struct context_switch_frame *frame;

	/* Allocate thread's kernel stack */
	t->arch.kernel_stack = kalloc(DEFAULT_KERNEL_STACK_SIZE);
	t->arch.kernel_stack_size = DEFAULT_KERNEL_STACK_SIZE;
	assert_neq(t->arch.kernel_stack, 0);

	stack_top = t->arch.kernel_stack + t->arch.kernel_stack_size;
	stack_top = (virt_addr_t)ROUND_DOWN((uintptr)stack_top, 8); // Stack must be 8 byte aligned

	frame = (struct context_switch_frame *)stack_top;
	frame--;

	memset(frame, 0, sizeof(*frame));
	frame->eip = (uintptr)&thread_main;
	frame->eflags = FL_DEFAULT | FL_IOPL_3;
	t->arch.sp = frame;
}

/*
** Initializes the new thread ater a fork.
*/
void
arch_init_fork_thread(struct thread *t)
{
	struct context_switch_frame *frame;

	/* Allocate thread's kernel stack */
	t->arch.kernel_stack = kalloc(current_thread->arch.kernel_stack_size);
	t->arch.kernel_stack_size = current_thread->arch.kernel_stack_size;
	assert_neq(t->arch.kernel_stack, 0);

	/* Copy kernel stack */
	memcpy(t->arch.kernel_stack, current_thread->arch.kernel_stack, t->arch.kernel_stack_size);

	/* Set the value of arch.iframe */
	t->arch.iframe = t->arch.kernel_stack + ((uintptr)current_thread->arch.iframe - (uintptr)current_thread->arch.kernel_stack);

	assert_eq(t->arch.iframe->eip, current_thread->arch.iframe->eip);
	t->arch.iframe->eax = 0; /* Set the return value of fork() for the new process */

	frame = (struct context_switch_frame *)t->arch.iframe;
	frame--;

	memset(frame, 0, sizeof(*frame));
	frame->eip = (uintptr)&thread_return_fork;
	frame->eflags = FL_DEFAULT | FL_IOPL_3;
	t->arch.sp = frame;

}

void
arch_thread_exit(void)
{
	/* if no other thread held this memory space */
	if (current_thread->vaspace->ref_count == 0)
	{
		/* radical way to clean up the memory space (okay, it may not be the best...) */
		munmap(NULL, GET_PD_IDX(KERNEL_VIRTUAL_BASE) * 1024 * PAGE_SIZE);
	}
	/*
	** kernel stack can't be freed from exit() (we're inside it),
	** so we'll free it from waitpid()
	*/
}

/*
** Called from thread_waitpid() when waiting a zombie thread.
** Used to finish cleaning up this thread.
*/
void
arch_cleanup_thread(struct thread *t)
{
	kfree(t->arch.kernel_stack);
	t->arch.kernel_stack = NULL;
}

/*
** Clone the 'src' of index 'pidx' within 'dest'.
*/
void
clone_page_table(struct page_table *dest, struct page_table *src, size_t pidx)
{
	phys_addr_t pa;
	void *kalloc_page;
	void *page;
	size_t i;

	kalloc_page = kalloc(2 * PAGE_SIZE); /* Dirty way to have page-aligned allocation */
	assert_neq(kalloc_page, NULL);
	page = (void *)ALIGN((uintptr)kalloc_page, PAGE_SIZE);
	pa = get_paddr(page);

	i = 0;
	while (i < 1024)
	{
		dest->entries[i].value = src->entries[i].value;
		if (src->entries[i].present) {
			dest->entries[i].frame = pa >> 12u;
			memcpy(page, GET_VADDR(pidx, i), PAGE_SIZE);

			/* Set a new frame address for the next page */
			pa = alloc_frame();
			assert_neq(pa, NULL_FRAME);
			set_paddr(page, pa);
		}
		++i;
	}
	kfree(kalloc_page);
}

/*
** Clone the given virtual space into a new one.
** Returns NULL if the clone failed.
*/
struct vaspace *
arch_clone_vaspace(struct vaspace *src)
{
	struct vaspace *vas;
	void *kalloc_pd;
	void *kalloc_pt;
	struct page_dir *pd;
	struct page_table *pt;
	phys_addr_t pa;
	size_t i;

	vas = kalloc(sizeof(*vas));
	kalloc_pd = kalloc(2 * sizeof(*pd)); /* Dirty way to have page-aligned allocations */
	kalloc_pt = kalloc(2 * sizeof(*pt));
	if (vas == NULL || kalloc_pd == NULL || kalloc_pt == NULL) {
		kfree(vas);
		kfree(kalloc_pd);
		kfree(kalloc_pt);
		return (NULL);
	}

	/* Copy most of the virtual address space structure */
	memcpy(vas, src, sizeof(*vas));

	/* Set usable page directory and page table. */
	pd = (struct page_dir *)ALIGN((uintptr)kalloc_pd, PAGE_SIZE);
	pt = (struct page_table *)ALIGN((uintptr)kalloc_pt, PAGE_SIZE);

	vas->arch.pagedir = get_paddr(pd);
	vas->ref_count = 1;

	pa = get_paddr(pt);

	i = 0;
	while (i < 1023)
	{
		pd->entries[i].value = GET_PAGE_DIRECTORY->entries[i].value;

		/* Kernel page tables are linked together so we only care about user page tables */
		if (i < GET_PD_IDX(KERNEL_VIRTUAL_BASE)
			&& GET_PAGE_DIRECTORY->entries[i].present)
		{
			/* Set the new page table frame */
			pd->entries[i].frame = pa >> 12u;
			clone_page_table(pt, GET_PAGE_TABLE(i), i);

			/* Set a new frame address for the next page table */
			pa = alloc_frame();
			assert_neq(pa, NULL_FRAME);
			set_paddr(pt, pa);
		}
		++i;
	}

	/* Set up recursiv mapping */
	pd->entries[1023].value = 0;
	pd->entries[1023].present = true;
	pd->entries[1023].rw = true;
	pd->entries[1023].frame = get_paddr(pd) >> 12u;

	/* Set a new frame address for the page directory */
	pa = alloc_frame();
	assert_neq(pa, NULL_FRAME);
	set_paddr(pd, pa);

	kfree(kalloc_pd);
	kfree(kalloc_pt);
	return (vas);
}

/*
** Do the arch-dependant part of context switching.
** Calls an asm routine, defined in context.asm.
*/
void
arch_context_switch(struct thread *old, struct thread *new)
{
	if (old->vaspace != new->vaspace) {
		set_cr3(new->vaspace->arch.pagedir);
	}
	set_kernel_stack((uintptr)new->arch.kernel_stack);
	x86_context_switch(&old->arch.sp, new->arch.sp);
}

/*
** Sets the current thread to the given one.
*/
void
set_current_thread(struct thread *thread)
{
	current_thread = thread;
}

/*
** Returns the currently running thread.
*/
struct thread *
get_current_thread(void)
{
	return (current_thread);
}
