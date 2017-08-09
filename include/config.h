/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#ifndef _CONFIG_
# define _CONFIG_

/*
** This file is used as a configuration file to
** let you customize ChaOS.
*/

/* Maximum number of processes running at the same time */
# define MAX_PID		32

/* Default size of a thread's stack */
# define DEFAULT_STACK_SIZE	0x1000 * 16u

/* [X86] Comment to disable SSE instructions (floating points) */
/* TODO Not implemented yet */
# define ENABLE_SSE

/*
** Ensure configuration is valid
*/
# if MAX_PID < 1
#  error "MAX_PID is less than one"
# endif /* MAX_PID < 1 */

#endif /* !_CONFIG_ */
