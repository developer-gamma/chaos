/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/syscall.h>
#include <kernel/thread.h>
#include <kernel/interrupts.h>
#include <arch/x86/interrupts.h>
#include <stdio.h>

__noreturn static void
x86_unhandled_exception(struct iframe *iframe)
{
	panic("Unhandled exception %#hhx\n"
		"\tError code: %#x", iframe->int_num, iframe->err_code);
}

static status_t
x86_division_by_zero_handler(struct iframe *iframe __unused)
{
	thread_exit(136); /* Boom, headshot! */
	return (OK);
}

static status_t
x86_breakpoint_handler(struct iframe *iframe)
{
	printf("Breakpoint.\n"
		"\tEIP: %p\n"
		"\tESP: %p\n",
		iframe->eip, iframe->esp);
	return (OK);
}

static status_t
x86_pagefault_handler(struct iframe *iframe)
{
	uintptr addr;

	if (unlikely(get_current_thread()->pid <= 1)) /* Usefull for early boot crash */
	{
		addr = get_cr2();
		printf("Page Fault at address %#p.\n"
			"\tAddress: %#p\n"
			"\tPresent: %y\n"
			"\tWrite: %y\n"
			"\tUser-mode: %y\n"
			"\tReserved: %y\n",
			iframe->eip,
			(void *)addr,
			(bool)((iframe->err_code ^ 0x1)),
			(bool)((iframe->err_code & 0x2)),
			(bool)((iframe->err_code & 0x4)),
			(bool)((iframe->err_code & 0x8))
		);
	}
	thread_exit(139); /* Boom, headshot! */
	return (OK);
}

static status_t
x86_gp_fault_handler(struct iframe *iframe __unused)
{
	thread_exit(139); /* Boom, headshot! */
	return (OK);
}

/*
** Common handler for all exceptions.
*/
void
x86_exception_handler(struct iframe *iframe)
{
	switch (iframe->int_num)
	{
	case X86_INT_DIVISION_BY_ZERO:
		x86_division_by_zero_handler(iframe);
		break;
	case X86_INT_BREAKPOINT:
		x86_breakpoint_handler(iframe);
		break;
	case X86_INT_PAGE_FAULT:
		x86_pagefault_handler(iframe);
		break;
	case X86_INT_GP_FAULT:
		x86_gp_fault_handler(iframe);
		break;
	default:
		x86_unhandled_exception(iframe);
		break;
	}
}

/*
** Common handler for all IRQs.
*/
void
x86_irq_handler(struct iframe * iframe)
{
	enum handler_return ret;

	ret = handle_interrupt(iframe->int_num);

	/* Reset the PICs */
	if (iframe->err_code > 7)
		RESET_SLAVE_PIC();
	RESET_MASTER_PIC();

	if (ret == IRQ_RESCHEDULE) {
		thread_yield();
	}
}

/*
** Common handler for all syscalls
**
** Arguments are in iframe->edi, iframe->esi, iframe->edx and iframe->ecx.
*/
void
x86_syscalls_handler(struct iframe *iframe)
{
	get_current_thread()->arch.iframe = iframe;
	switch (iframe->eax)
	{
		case EXIT:
			thread_exit(iframe->edi);
			break;
		case FORK:
			iframe->eax = sys_fork();
			break;
		case WRITE:
			iframe->eax = sys_write((int)iframe->edi, (char const *)iframe->esi, (size_t)iframe->edx);
			break;
		case READ:
			iframe->eax = sys_read((int)iframe->edi, (char *)iframe->esi, (size_t)iframe->edx);
			break;
		case BRK:
			iframe->eax = ubrk((void *)iframe->edi);
			break;
		case SBRK:
			iframe->eax = (uintptr)usbrk((intptr)iframe->edi);
			break;
		case GETPID:
			iframe->eax = get_current_thread()->pid;
			break;
		case WAITPID:
			iframe->eax = thread_waitpid(iframe->edi);
			break;
		case EXECVE:
			iframe->eax = thread_execve((char const *)iframe->edi, (int (*)(void))iframe->esi);
			break;
		case GETCWD:
			iframe->eax = (uintptr)thread_getcwd((char *)iframe->edi, (size_t)iframe->esi);
			break;
		case OPEN:
			iframe->eax = -1; /* TODO */
			break;
		case CLOSE:
			iframe->eax = -1; /* TODO */
			break;
		default:
			panic("Unknown syscall %p\n", iframe->eax);
	}
}
