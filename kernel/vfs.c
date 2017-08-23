/* ------------------------------------------------------------------------ *\
**
**  This file is part of the Chaos Kernel, and is made available under
**  the terms of the GNU General Public License version 2.
**
**  Copyright (C) 2017 - Benjamin Grange <benjamin.grange@epitech.eu>
**
\* ------------------------------------------------------------------------ */

#include <kernel/fs.h>
#include <kernel/init.h>
#include <kernel/multiboot.h>
#include <kernel/kalloc.h>
#include <string.h>

/* Debug */
#include <stdio.h>

static struct fs_file *root;

static struct fs_file *
find_in_dir(struct fs_file *dir, char const *name)
{
	assert(dir->type & FS_DIRECTORY);
	if (dir->mount && dir->mount->api->finddir) {
		return (dir->mount->api->finddir(dir, name));
	}
	return (NULL);
}

/*
** Find the file corresponding to the given path, or NULL if it wasn't found.
**
** THE PATH MUST HAVE BEEN NORMALIZED USING normalize_path() !
** It will also be modified, so you can't count on it after this call.
*/
struct fs_file *
find_file(char *path)
{
	struct fs_file *file;
	size_t path_len;
	char *offset;
	size_t path_depth;

	path_len = strlen(path);
	if (path_len == 1) {
		return (root);
	}

	printf("Looking for %s\n", path);

	/* Tokenize the path */
	offset = path;
	path_depth = 0;
	while (offset < path + path_len) {
		if (*offset == '/') {
			*offset = '\0';
			++path_depth;
		}
		++offset;
	}
	path[path_len] = '\0';
	offset = path + 1;

	/* Go through all tokens */
	file = root;
	while (path_depth)
	{
		printf("  Looking in %s\n", offset);
		file = find_in_dir(file, offset);
		if (!file) {
			printf("404 not found\n");
			return (NULL);
		}
		/* Next token */
		offset += strlen(offset) + 1;
		--path_depth;
	}
	printf("found [%s]\n", file->name);
	return (file);
}

/*
**
** Resolves the given path, by removing double separators, '.' and '..'.
**
** Inspired by lk's 'fs_normalize_path()'
*/
void
resolve_path(char *path)
{
	int outpos;
	int pos;
	bool done;
	char c;

	enum {
		INITIAL,
		FIELD_START,
		IN_FIELD,
		SEPARATOR,
		SEEN_SEPARATOR,
		DOT,
		SEEN_DOT,
		DOTDOT,
		SEEN_DOTDOT,
	} state;

	state = INITIAL;
	pos = 0;
	outpos = 0;
	done = false;

	/* Resolve the given path */
	while (!done)
	{
		c = path[pos];
		switch (state) {
		case INITIAL:
			if (c == '/') {
				state = SEPARATOR;
			}
			else if (c == '.') {
				state = DOT;
			} else {
				state = FIELD_START;
			}
			break;
		case FIELD_START:
			if (c == '.') {
				state = DOT;
			} else if ( c == '\0') {
				done = true;
			} else {
				state = IN_FIELD;
			}
			break;
		case IN_FIELD:
			if (c == '/') {
				state = SEPARATOR;
			} else if (c == '\0') {
				done = true;
			} else {
				path[outpos++] = c;
				pos++;
			}
			break;
		case SEPARATOR:
			path[outpos++] = '/';
			pos++;
			state = SEEN_SEPARATOR;
			break;
		case SEEN_SEPARATOR:
			if (c == '/') {
				++pos;
			} else if (c == '\0') {
				done = true;
			} else {
				state = FIELD_START;
			}
			break;
		case DOT:
			pos++;
			state = SEEN_DOT;
			break;
		case SEEN_DOT:
			if (c == '.') {
				state = DOTDOT;
			} else if (c == '/') { /* Filename == '.', eat it */
				++pos;
				state = SEEN_SEPARATOR;
			} else if (c == '\0') {
				done = true;
			} else { /* Filename starting with '.' */
				path[outpos++] = '.';
				state = IN_FIELD;
			}
			break;
		case DOTDOT:
			pos++;
			state = SEEN_DOTDOT;
			break;
		case SEEN_DOTDOT:
			if (c == '/' || c == '\0') { /* Filename == '..' */
				outpos -= (outpos > 0);
				while (outpos > 0) { /* Walk backward */
					if (path[outpos - 1] == '/') {
						break;
					}
					--outpos;
				}
				pos++;
				state = SEEN_SEPARATOR;
				if (c == '\0') {
					done = true;
				}
			} else { /* Filename starting with '.. */
				path[outpos++] = '.';
				path[outpos++] = '.';
				state = IN_FIELD;
			}
			break;
		}
	}

	/* Remove trailing slash */
	if (outpos == 0) {
		path[outpos++] = '/';
	} else if (outpos > 1 && path[outpos - 1] == '/') {
		--outpos;
	}

	path[outpos] = 0;
}

status_t
init_vfs(void)
{
	root = kalloc(sizeof(*root));
	if (root == NULL) {
		return (ERR_NO_MEMORY);
	}
	memset(root, 0, sizeof(*root));
	root->name[0] = '/';
	root->type |= FS_DIRECTORY;
	return (OK);
}
