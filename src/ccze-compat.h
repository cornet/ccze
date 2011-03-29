/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-compat.h -- OS compatibility stuff for CCZE, prototypes
 * Copyright (C) 2003 Gergely Nagy <algernon@bonehunter.rulez.org>
 *
 * This file is part of ccze.
 *
 * ccze is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * ccze is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef _CCZE_COMPAT_H
#define _CCZE_COMPAT_H 1

#include "system.h"

#include <sys/types.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>

#ifndef HAVE_STRNDUP
char *strndup (const char *s, size_t size);
#endif
int ccze_getsubopt (char **optionp, char *const *tokens,
		    char **valuep);

#ifndef HAVE_SCANDIR
int scandir (const char *dir, struct dirent ***namelist,
	     int (*select)(const struct dirent *),
	     int (*compar)(const struct dirent **, const struct dirent **));
#endif
#ifndef HAVE_ALPHASORT
int alphasort (const struct dirent **a, const struct dirent **b)
#endif

#ifndef HAVE_ASPRINTF
int
asprintf(char **ptr, const char *fmt, ...);
#endif

#ifndef HAVE_ARGP_PARSE
struct argp_option
{
  char *name;
  int key;
  const char *arg;
  int flags;
  char *doc;
  int group;
};
struct argp;
struct argp_state;
typedef error_t (*argp_parser_t) (int key, char *arg,
				  struct argp_state *state);
#define ARGP_ERR_UNKNOWN        E2BIG
#define OPTION_HIDDEN		0x2
struct argp
{
  const struct argp_option *options;
  argp_parser_t parser;
  const char *args_doc;
  const char *doc;
  /* The rest is ignored */
  const void *children;
  char *(*help_filter) (int key, const char *text, void *input);
  const char *domain;
};

struct argp_state
{
  /* This is deliberately not compatible with glibc's one. We only use
     ->input anyway */
  void *input;
};

extern const char *argp_program_version;
extern const char *argp_program_bug_address;
extern const char *argp_program_name;

error_t argp_parse (const struct argp *argps, int argc, char **argv,
		    unsigned flags, int arg_index, void *input);
error_t argp_error (const struct argp_state *state, char *fmt, ...)
     __attribute__ ((noreturn));
#endif

#ifndef HAVE_GETLINE
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#endif
#ifndef HAVE_GETDELIM
ssize_t
getdelim (char **lineptr, size_t *n, int delim, FILE *stream);
#endif

#ifdef _AIX
char *strndup (const char *s, size_t size);
#endif

#endif /* !_CCZE_COMPAT_H */
