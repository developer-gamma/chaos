/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/interrupts.h>

/*
** This file is about handling interrupts requests in an architecture independant way.
** You can register callback for a given interrupt vector, and trigger those callbacks.
*/

static int_handler irq_handlers[MAX_IRQ];

status_t
register_int_handler(uint vec, int_handler handler)
{
	if (vec > MAX_IRQ) {
		return (ERR_OUT_OF_RANGE);
	}
	irq_handlers[vec] = handler;
	return (OK);
}

status_t
unregister_int_handler(uint vec)
{
	if (vec > MAX_IRQ) {
		return (ERR_OUT_OF_RANGE);
	}
	irq_handlers[vec] = NULL;
	return (OK);
}

enum handler_return
handle_interrupt(uint vector)
{
	if (irq_handlers[vector]) {
		return (irq_handlers[vector]());
	}
	return (IRQ_NO_RESCHEDULE);
}
