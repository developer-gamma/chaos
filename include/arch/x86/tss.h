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
	uint32_t prev_tss;	/* Previous TSS */
	uint32_t esp0;		/* The stack pointer to load when we switch to kernel mode. */
	uint32_t ss0;		/* The stack segment to load when we switch to kernel mode. */
	uint32_t esp1;		/* Everything this point is unusued */
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t fs;
	uint32_t gs;
	uint32_t ldt;
	uint16_t trap;
	uint16_t iomap_base;
} __packed;

#endif /* !_ARCH_X86_TSS_H_ */
