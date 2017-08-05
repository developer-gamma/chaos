/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <chaosdef.h>

extern int (shell_main)(void);

/*
** The first thread launched by the kernel when it has finish booting.
** TODO make this user space
*/
void
init_routine(void)
{
	/*
	** The idea here is to call execve() and execute the init script, or something similar.
	** As execve() isn't implemented yet, i'm directly calling the main of the shell program.
	*/
	shell_main();

	/* Init should never finish. */
	panic("init finished");
}

