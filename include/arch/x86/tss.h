/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#ifndef _ARCH_X86_TSS_H_
# define _ARCH_X86_TSS_H_

# include <chaosdef.h>

/*
** The Task State Segment.
**
** As we're doing software multitasking, most of the TSS is useless.
*/
struct tss
{
	uint32 prev_tss;	/* Previous TSS */
	uint32 esp0;		/* The stack pointer to load when we switch to kernel mode. */
	uint32 ss0;		/* The stack segment to load when we switch to kernel mode. */
	uint32 esp1;		/* Everything this point is unusued */
	uint32 ss1;
	uint32 esp2;
	uint32 ss2;
	uint32 cr3;
	uint32 eip;
	uint32 eflags;
	uint32 eax;
	uint32 ecx;
	uint32 edx;
	uint32 ebx;
	uint32 esp;
	uint32 ebp;
	uint32 esi;
	uint32 edi;
	uint32 es;
	uint32 cs;
	uint32 ss;
	uint32 ds;
	uint32 fs;
	uint32 gs;
	uint32 ldt;
	uint16 trap;
	uint16 iomap_base;
} __packed;

static_assert(sizeof(struct tss) == 26 * sizeof(uintptr));

struct gdt_tss_entry
{
	uint limit_low		: 16;
	uint base_low		: 24;

	uint _one		: 1;
	uint busy		: 1;
	uint type		: 3;
	uint dpl		: 2;
	uint present		: 1;

	uint limit_high		: 4;
	uint available		: 1;
	uint _zero		: 2;
	uint granularity	: 1;
	uint base_high		: 8;
} __packed;

static_assert(sizeof(struct gdt_tss_entry) == 2 * sizeof(uintptr));

void			set_kernel_stack(uintptr stack);

#endif /* !_ARCH_X86_TSS_H_ */
