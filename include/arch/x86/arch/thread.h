/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#ifndef _ARCH_X86_ARCH_THREAD_H_
# define _ARCH_X86_ARCH_THREAD_H_

# include <arch/x86/interrupts.h>

struct		arch_thread
{
	/* Stack pointer of the thread, belongs to kernel stack */
	virt_addr_t sp;

	/* Pointer to the kerenl stack of this thread */
	void *kernel_stack;

	/* Size of the kernel stack */
	size_t kernel_stack_size;

	/* Interrupt frame, set when a syscall is trigger */
	struct iframe *iframe;
};

struct		context_switch_frame
{
	uintptr edi;
	uintptr esi;
	uintptr ebp;
	uintptr esp;
	uintptr ebx;
	uintptr edx;
	uintptr ecx;
	uintptr eax;
	uintptr eflags;
	uintptr eip;
};

extern void 	x86_context_switch(void **old_esp, void *new_esp);
extern void	x86_jump_userspace(void *) __noreturn;
extern void	x86_return_userspace(void *) __noreturn;

#endif /* _ARCH_X86_ARCH_THREAD_H_ */
