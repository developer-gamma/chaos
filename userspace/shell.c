/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <unistd.h>

static size_t
strlen(char const *str)
{
	char const *s;

	s = str;
	while (*s)
		++s;
	return (s - str);
}

static void
puts(char const *str)
{
	write(1, str, strlen(str));
}

static char *
strndup(char const *str, size_t n)
{
	size_t len;
	char *out;

	len = 0;
	while (len < n && str[len]) {
		++len;
	}
	out = sbrk(len + 1);
	if (out == NULL) {
		return (NULL);
	}
	len = 0;
	while (len < n && str[len]) {
		out[len] = str[len];
		++len;
	}
	out[len] = '\0';
	return (out);
}

static void
prompt(void)
{
	puts("$> ");
}

static char *
read_command(void)
{
	char buffer[PAGE_SIZE];
	size_t buff_size;
	int ret;

	buff_size = 0;
	while (1)
	{
		ret = read(0, buffer + buff_size, 1);
		if (ret <= 0) {
			return (NULL);
		}
		if (buffer[buff_size] == '\n' || buff_size + 1 == PAGE_SIZE)
			break;
		++buff_size;
	}
	return (strndup(buffer, buff_size));
}

static void
parse_command(char const *cmd)
{
	puts("Unknown command \"");
	puts(cmd);
	puts("\"\n");
}

int
shell_main(void)
{
	char *cmd;

	while (42)
	{
		prompt();
		cmd = read_command();
		if (cmd) {
			parse_command(cmd);
		}
	}
	return (0);
}
