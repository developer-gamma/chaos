/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#ifndef _KERNEL_UNIT_TESTS_H_
# define _KERNEL_UNIT_TESTS_H_

# include <kernel/init.h>

enum unit_test_level
{
	UNIT_TEST_LEVEL_LIBC		= 0,
	UNIT_TEST_LEVEL_PMM,
	UNIT_TEST_LEVEL_VMM,
};

typedef void(*unit_test_hook_funcptr)(void);

struct unit_test_hook
{
	enum unit_test_level level;
	unit_test_hook_funcptr hook;
	char const *name;
};

# define NEW_UNIT_TEST(n, h, l)						\
	__aligned(sizeof(void*)) __used __section("chaos_unit_tests")	\
	static const struct unit_test_hook _utest_hook_struct_##n = {	\
		.level = l,						\
		.hook = h,						\
		.name = #n,						\
	}

void			trigger_unit_tests(enum unit_test_level utl);

#endif /* !_KERNEL_UNIT_TESTS */
