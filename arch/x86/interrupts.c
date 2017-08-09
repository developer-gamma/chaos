/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/init.h>
#include <kernel/interrupts.h>
#include <arch/x86/interrupts.h>
#include <string.h>
#include <stdio.h>

/* Defined in idt.asm */
extern struct idt_entry idt[X86_INT_MAX];
extern void (*x86_unhandled_exception_handler)(void);

/*
** Set the present flag for the given idt entry
*/
static inline void
idt_set_present(struct idt_entry *entry, bool p)
{
	entry->flags &= ~(1u << 7u);
	entry->flags |= p << 7u;
}

/*
** Set the ring level for the given idt entry
*/
static inline void
idt_set_dpl(struct idt_entry *entry, enum dpl dpl)
{
	entry->flags &= ~(3u << 5u);
	entry->flags |= dpl << 5u;
}

/*
** Set the interrupt type for the given idt entry
*/
static inline void
idt_set_type(struct idt_entry *entry, enum idt_entry_type type)
{
	entry->flags &= ~0xF;
	entry->flags |= type;
}

/*
** Set the interrupt callback for the given idt entry
*/
static inline void
idt_set_callback(struct idt_entry *entry, uintptr ep)
{
	entry->callback_low = ep & 0x0000FFFFu;
	entry->callback_high = (ep & 0xFFFF0000u) >> 16u;
}

/*
** Set the segment selector for the given idt entry
*/
static inline void
idt_set_segment_sel(struct idt_entry *entry, uint16 sel)
{
	entry->selector = sel;
}

/*
** Updates an IDT entry with the given parameters.
*/
void
idt_set_vector(uint8 vec, uintptr callback, uint16 sel, enum dpl dpl, enum idt_entry_type type)
{
	struct idt_entry *entry;

	entry = idt + vec;
	memset(entry, 0, sizeof(*entry));
	idt_set_present(entry, true);
	idt_set_dpl(entry, dpl);
	idt_set_type(entry, type);
	idt_set_callback(entry, callback);
	idt_set_segment_sel(entry, sel);
}

/*
** Implementation of functions from kernel/interrupts.h
*/

status_t
mask_interrupt(uint vector)
{
	idt_set_present(idt + vector, false);
	return (OK);
}

status_t
unmask_interrupt(uint vector)
{
	idt_set_present(idt + vector, true);
	return (OK);
}

void
enable_interrupts(void)
{
	sti();
}

void
disable_interrupts(void)
{
	cli();
}

void
push_interrupts(int_state_t *save)
{
	*save = get_eflags();
}

void
pop_interrupts(int_state_t *save)
{
	set_eflags(*save);
}

bool
are_int_enabled(void)
{
	uint eflags;

	eflags = get_eflags();
	return ((bool)(eflags & FL_IF));
}

/*
** Sets up a default IDT
** Called by boot.asm
*/
void
setup_default_idt(void)
{
	size_t i;

	i = 0;
	while (i < X86_INT_MAX)
	{
		idt_set_vector(i, (uintptr)&x86_unhandled_exception_handler, KERNEL_CODE_SELECTOR, DPL_RING_0, IDT_INTERRUPT_GATE_32);
		++i;
	}
}

static void
interrupt_init(enum init_level il __unused)
{
	/* Remapping IRQ */
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x0);
	outb(0xA1, 0x0);

	printf("[OK]\tInterrupt Requests\n");
}

NEW_INIT_HOOK(idt_early, &interrupt_init, CHAOS_INIT_LEVEL_ARCH_EARLY);
