/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#ifndef _KERNEL_INTERRUPTS_H_
# define _KERNEL_INTERRUPTS_H_

# include <chaosdef.h>
# include <chaoserr.h>

# define MAX_IRQ			16

typedef uintptr		int_state_t;

enum			handler_return
{
	IRQ_NO_RESCHEDULE,
	IRQ_RESCHEDULE,
};

typedef enum handler_return (*int_handler)(void);

/*
** Functions to register interrupt handlers in an architecture independant way.
*/
status_t		register_int_handler(uint vector, int_handler handler);
status_t		unregister_int_handler(uint vector);
enum handler_return	handle_interrupt(uint vector);

/*
** All these functions should be reimplemented on every supported architecture.
*/
status_t		mask_interrupt(uint v);
status_t		unmask_interrupt(uint v);
void			enable_interrupts(void);
void			disable_interrupts(void);
void			push_interrupts(int_state_t *);
void			pop_interrupts(int_state_t *);
bool			are_int_enabled(void);

#endif /* !_LIB_INTERRUPTS_H_ */
