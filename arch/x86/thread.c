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
#include <string.h>

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
	//x86_jump_userspace(current_thread->entry, current_thread->stack);

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
** Set default register values and pushes the arguments on the stack
*/
void
arch_thread_execve(int argc, char *argv[])
{
	int i;
	void *stack;
	struct iframe *iframe;
	char **argv2;
	size_t len;

	argv2 = kalloc(sizeof(char *) * argc);
	assert_neq(argv2, NULL);

	/* Paste argv on thread's stack */
	i = 0;
	stack = current_thread->stack;
	while (i < argc)
	{
		len = strlen(argv[i]);
		stack -= (len - 1) * sizeof(char);
		strcpy(stack, argv[i]);
		argv2[i] = stack;
		++i;
	}
	stack -= (argc + 1) * sizeof(char *);
	memcpy(stack, argv2, (argc + 1) * sizeof(char *));
	kfree(argv2);
	argv2 = stack;
	stack = (void *)ROUND_DOWN((uintptr)stack, sizeof(void *));

	stack -= sizeof(char **); /* Push argv*/
	*(char ***)stack = argv2;
	stack -= sizeof(int); /* Push argc */
	*(int *)stack = argc;
	stack -= sizeof(void (*)()); /* Push dumb return value */
	*(void (**)())stack = (void (*)())0x0;

	iframe = get_current_thread()->arch.iframe;
	iframe->ecx = 0;
	iframe->edx = 0;
	iframe->ebx = 0;
	iframe->esi = 0;
	iframe->edi = 0;
	iframe->eip = (uintptr)current_thread->entry;
	iframe->esp = (uintptr)stack;
	iframe->ebp = iframe->esp;
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

/*
** Do the arch-dependant part of context switching.
** Calls an asm routine, defined in context.asm.
*/
void
arch_context_switch(struct thread *old, struct thread *new)
{
	uintptr kernel_stack;

	if (old->vaspace != new->vaspace) {
		set_cr3(new->vaspace->arch.pagedir);
	}
	kernel_stack = (uintptr)new->arch.kernel_stack + new->arch.kernel_stack_size;
	kernel_stack = ROUND_DOWN(kernel_stack, sizeof(void *));
	set_kernel_stack(kernel_stack);
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
