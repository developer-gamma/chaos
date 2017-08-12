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

static void
putc(char c)
{
	write(1, &c, 1);
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

static int
strcmp(char const *s1, char const *s2)
{
	while (*s1 == *s2)
	{
		if (*s1 == '\0') {
			return (0);
		}
		++s1;
		++s2;
	}
	return (s1 - s2);
}

static inline uintptr
get_cs(void)
{
	uintptr cs;

	asm volatile("mov %%cs, %0" : "=r"(cs));
	return (cs);
}

static inline uintptr
get_ds(void)
{
	uintptr ds;

	asm volatile("mov %%ds, %0" : "=r"(ds));
	return (ds);
}

static bool
is_in_usermode(void)
{
	uint ds;
	uint cs;

	ds = get_ds();
	cs = get_cs();
	return ((ds & 0b11) && (cs & 0b11));
}

static void
prompt(void)
{
	puts(is_in_usermode() ? "user" : "kernel");
	puts(" $> ");
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
		/* Process each character, one at a time */
		ret = read(0, buffer + buff_size, 1);
		if (ret <= 0) {
			return (NULL);
		}

		/* Handles the special case it's a backspace */
		if (buffer[buff_size] == '\b') {
			if (buff_size > 0) {
				putc('\b'); /* Visually erase the character */
				putc(' ');
				putc('\b');
				--buff_size;
			}
			continue;
		}

		/* Shows the character the user typed. */
		putc(buffer[buff_size]);

		/* If it's a carriage return or we reached the end of buffer, process the command */
		if (buffer[buff_size] == '\n' || buff_size + 1 == PAGE_SIZE)
			break;
		++buff_size;
	}
	return (strndup(buffer, buff_size));
}

struct cmd
{
	char const *name;
	char const *desc;
	int (*func)(void);
};

static struct cmd cmds[];

static int
exec_help(void)
{
	struct cmd *cmd;

	cmd = cmds;
	puts("Available commands:\n");
	while (cmd->desc)
	{
		putc('\t');
		puts(cmd->name);
		puts(":\t\t");
		puts(cmd->desc);
		putc('\n');
		++cmd;
	}
	return (0);
}

static int
exec_ls(void)
{
	puts("file1\tfile2\tfile3\n");
	return (0);
}

__attribute__((optimize("O0")))
static int
exec_sigsev(void)
{
	*(char *)0xDEADBEEF = 'a';
	return (0);
}

static struct cmd cmds[] =
{
	{"help", "print the help", &exec_help},
	{"ls", "list filesystem", &exec_ls},
	{"sigsev", "produces a segmentation fault", &exec_sigsev},

	{NULL, NULL, NULL},
};

static void
parse_command(char const *cmd)
{
	pid_t pid;
	struct cmd *c;

	c= cmds;
	while (c->name) {
		if (!strcmp(c->name, cmd)) {
			pid = fork();
			assert_neq(pid, -1);
			if (pid == 0) {
				c->func();
				exit();
			} else {
				if (waitpid(pid) == 139) {
					puts("Segmentation Fault\n");
				}
			}
			return ;
		}
		++c;
	}
	puts("Unknown command \"");
	puts(cmd);
	puts("\"\n");
}

int
shell_main(void)
{
	char *cmd;

	puts("Type \"help\" to show all commands.\n");
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
