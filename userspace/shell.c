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
	exit();
	return (0);
}

static int
exec_ls(void)
{
	char cwd[PAGE_SIZE];
	int fd;

	assert_neq(getcwd(cwd, PAGE_SIZE), NULL);
	fd = open(cwd);
	assert_neq(fd, -1);

	/* Do stuff here */
	puts("file1\nfile2\nfile3\n");

	close(fd);
	exit();
	return (0);
}

static int
exec_sigsev(void)
{
	*(char *)0xDEADBEEF = 'a';
	exit();
	return (0);
}

static int
exec_divzero(void)
{
	int j;

	j = 1;
	j = 5 / (j - 1);
	exit();
	return (j);
}

static struct cmd cmds[] =
{
	{"help", "prints this help", &exec_help},
	{"ls", "lists filesystem", &exec_ls},
	{"sigsev", "produces a segmentation fault", &exec_sigsev},
	{"divz","produces a division by zero", &exec_divzero},

	{NULL, NULL, NULL},
};

static void
parse_command(char const *cmd)
{
	pid_t pid;
	struct cmd *c;
	int status;

	c= cmds;
	while (c->name) {
		if (!strcmp(c->name, cmd)) {
			pid = fork();
			assert_neq(pid, -1);
			if (pid == 0) {
				execve(c->name, c->func);
				puts("execve failed\n");
				exit();
			} else {
				status = waitpid(pid);
				if (status == 139) {
					puts("Segmentation Fault\n");
				}
				else if (status == 136) {
					puts("Floating Point exception\n");
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

static void
prompt(void)
{
	char buffer[PAGE_SIZE];

	assert_neq(getcwd(buffer, PAGE_SIZE), NULL);
	puts(buffer);
	puts(" $> ");
}

int
init_routine(void)
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
