/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/interrupts.h>
#include <chaosdef.h>
#include <stdio.h>

extern void			ret_kernel_main(void);

# define			PRINT_STACK_FRAME(x)				\
	ptr = __builtin_return_address(x);					\
	printf("\tCalled by %p\n", __builtin_extract_return_addr(ptr));		\
	if (ptr == &ret_kernel_main						\
		|| ptr == NULL)							\
		break;								\

/*
** Make the kernel panic with the given error message.
** Never returns.
*/
void
panic(const char *fmt, ...)
{
	va_list va;
	void *ptr;

	arch_disable_interrupts();
	va_start(va, fmt);
	printf("\nKernel panicked: ");
	vprintf(fmt, va);
	va_end(va);

	printf("\n");

	/* __builtin_return_address(x) needs x to be known compile time */
	do
	{
		PRINT_STACK_FRAME(0);
		//PRINT_STACK_FRAME(1);
		//PRINT_STACK_FRAME(2);
		//PRINT_STACK_FRAME(3);
		//PRINT_STACK_FRAME(4);
		//PRINT_STACK_FRAME(5);
		//PRINT_STACK_FRAME(6);
		//PRINT_STACK_FRAME(7);
		//PRINT_STACK_FRAME(8);
		//PRINT_STACK_FRAME(9);
	}
	while (0);

	for (;;)
		;
}

