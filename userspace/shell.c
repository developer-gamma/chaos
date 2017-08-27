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
strdup(char const *str)
{
	char *out;
	size_t len;

	len = strlen(str);
	out = sbrk(len + 1);
	if (out == NULL) {
		return (NULL);
	}
	len = 0;
	while (str[len]) {
		out[len] = str[len];
		++len;
	}
	out[len] = '\0';
	return (out);
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

static size_t
nb_args(char const *str)
{
	size_t nb;
	bool old;

	nb = 0;
	old = true;
	while (*str)
	{
		if (*str != ' ' && *str != '\t' && old) {
			++nb;
		}
		old = *str == ' ' || *str == '\t';
		++str;
	}
	return (nb);
}

static void
fill_argv(char *str, char const **argv)
{
	size_t i;
	bool old;

	i = 0;
	old = true;
	while (*str)
	{
		if (*str != ' ' && *str != '\t' && old) {
			argv[i] = str;
			++i;
		}
		old = false;
		if (*str == ' ' || *str == '\t') {
			*str = '\0';
			old = true;
		}
		++str;
	}
	argv[i] = NULL;
}

struct cmd
{
	char const *name;
	char const *desc;
	int (*func)();
};

static struct cmd cmds[];

/* DEBUG */
void printf(char const *, ...);
void *kalloc(size_t);
void kfree(void *);
void strcat(char *, char *);
void strcpy(char *, char *);

static void
print_dir_content(char *pwd)
{
	int fd;
	struct dirent dirent;
	char *path;

	printf("%s:\n", pwd);

	fd = opendir(pwd);
	assert_neq(fd, -1);
	while (readdir(fd, &dirent) == 0) {
		printf("[%c] [%s]\n", dirent.dir ? 'd' : 'f', dirent.name);
		if (strcmp(dirent.name, ".") && strcmp(dirent.name, "..")) {
			path = kalloc(strlen(pwd) + strlen(dirent.name) + 2);
			assert_neq(path, NULL);
			strcpy(path, pwd);
			strcat(path, "/");
			strcat(path, dirent.name);
			if (dirent.dir) {
				printf("\n");
				print_dir_content(path);
				printf("\n");
			} else {
				//printf("(Content of %s here)\n", dirent.name);
			}
			kfree(path);
		}
	}
	closedir(fd);
}

static int
exec_dir(void)
{
	printf("--- DIR STARTING ---\n");
	print_dir_content(".");
	printf("--- DIR FINISHED ---\n");
	exit();
	return (0);
}

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
	struct dirent dirent;
	int fd;

	fd = opendir(".");
	assert_neq(fd, -1);

	/* Do stuff here */
	while (readdir(fd, &dirent) == 0) {
		puts(dirent.name);
		putc('\n');
	}

	closedir(fd);
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
	{"dir", "Test the filesystem", &exec_dir},
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
	char const *argv[nb_args(cmd)]; /* VLA because we don't have a userspace malloc (yet) */

	c = cmds;
	fill_argv(strdup(cmd), argv);
	while (c->name) {
		if (!strcmp(c->name, cmd)) {
			pid = fork();
			assert_neq(pid, -1);
			if (pid == 0) {
				execve(c->name, c->func, argv);
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

#include <kernel/interrupts.h>

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
