/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/vaspace.h>
#include <kernel/thread.h>
#include <kernel/kalloc.h>
#include <arch/x86/vmm.h>
#include <string.h>

/*
** Set up a new virtual address space
*/
void
arch_init_vaspace(void)
{
	get_current_thread()->vaspace->arch.pagedir = get_cr3();
}

/*
** Clone the page table 'src' of index 'pidx' within 'dest'.
*/
static void
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
** Free the virtual address space
*/
void
arch_free_vaspace(void)
{
	size_t i;
	struct pagedir_entry *pde;

	phys_addr_t pa;

	/* free page tables */
	i = 0;
	while (i < GET_PD_IDX(KERNEL_VIRTUAL_BASE))
	{
		pde = GET_PAGE_DIRECTORY->entries + i;
		if (pde->present) {
			pa = pde->frame << 12u;
			free_frame(pa);
		}
		pde->value = 0;
		++i;
	}
}

/*
** Called by thread_waitpid() when waiting a zombie thread.
** Used to finish cleaning up this thread, by freeing it's kernel memory.
*/
void
arch_free_zombie_thread(struct thread *t)
{
	kfree(t->arch.kernel_stack);
	t->arch.kernel_stack = NULL;

	free_frame(t->vaspace->arch.pagedir);
}

