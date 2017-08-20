/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/init.h>
#include <kernel/multiboot.h>
#include <kernel/unit_tests.h>
#include <stdio.h>

extern struct unit_test_hook const __start_chaos_unit_tests[] __weak;
extern struct unit_test_hook const __stop_chaos_unit_tests[] __weak;

static void
trigger_unit_test_hooks(enum unit_test_level utl)
{
	struct unit_test_hook const *hook;

	for (hook = __start_chaos_unit_tests; hook < __stop_chaos_unit_tests; ++hook)
	{
		if (hook->level == utl) {
			printf("[..]\tUnit tests (%s)", hook->name);
			hook->hook();
			printf("\r[OK]\tUnit tests (%s)\n", hook->name);
		}
	}
}

void
trigger_unit_tests(enum unit_test_level utl)
{
	if (cmd_options.unit_test)
	{
		trigger_unit_test_hooks(utl);
	}
}
