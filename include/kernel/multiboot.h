/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#ifndef _KERNEL_MULTIBOOT_H_
# define _KERNEL_MULTIBOOT_H_

# include <chaosdef.h>

struct cmd_options
{
	bool unit_test;
};

struct multiboot_info
{
	char const *args;
};

struct cmd_options cmd_options;
struct multiboot_info multiboot_info;

#endif /* !_KERNEL_MULTIBOOT_H_ */
