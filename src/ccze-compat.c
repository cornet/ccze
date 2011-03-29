/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-compat.c -- OS compatibility stuff for CCZE
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

#include <ccze.h>

#include "system.h"
#include "ccze-compat.h"

#include <ctype.h>
#include <errno.h>
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define XSREALLOC(ptr, type, nmemb) \
	ptr = (type*)ccze_realloc (ptr, nmemb * sizeof (type))

#if malloc == rpl_malloc
#undef malloc
#undef realloc
#endif

void *
ccze_malloc (size_t size)
{
  register void *value = malloc (size);
  if (value == 0)
    exit (2);
  return value;
}

void *
ccze_realloc (void *ptr, size_t size)
{
  register void *value = realloc (ptr, size);
  if (value == 0)
    exit (2);
  return value;
}

void *
ccze_calloc (size_t nmemb, size_t size)
{
  register void *value = calloc (nmemb, size);
  if (value == 0)
    exit (2);
  return value;
}

#ifndef HAVE_STRNDUP
char *
strndup (const char *s, size_t size)
{
  char *ns = (char *)ccze_malloc (size + 1);
  memcpy (ns, s, size);
  ns[size] = '\0';
  return ns;
}
#endif

#ifndef HAVE_ARGP_PARSE
error_t
argp_parse (const struct argp *argps, int argc, char **argv,
	    unsigned flags, int arg_index, void *input)
{
  char *options;
  int optpos = 0, optionspos = 0, optionssize = 30;
  struct argp_state *state;
  int c;
#if HAVE_GETOPT_LONG
  struct option *longopts;
  int longoptionspos = 0;
 
  longopts = (struct option *)ccze_calloc (optionssize,
					   sizeof (struct option));
#endif
  
  state = (struct argp_state *)ccze_malloc (sizeof (struct argp_state));
  state->input = input;

  options = (char *)ccze_calloc (optionssize, sizeof (char *));
  while (argps->options[optpos].name != NULL)
  {
    if (optionspos >= optionssize)
      {
	optionssize *= 2;
	XSREALLOC (options, char, optionssize);
#if HAVE_GETOPT_LONG
	XSREALLOC (longopts, struct option, optionssize);
#endif
      }
    options[optionspos++] = (char) argps->options[optpos].key;
#if HAVE_GETOPT_LONG
    longopts[longoptionspos].name = argps->options[optpos].name;
    if (argps->options[optpos].arg)
      longopts[longoptionspos].has_arg = required_argument;
    else
      longopts[longoptionspos].has_arg = no_argument;
    longopts[longoptionspos].flag = NULL;
    longopts[longoptionspos].val = argps->options[optpos].key;
    longoptionspos++;
#endif
    if (argps->options[optpos].arg)
      options[optionspos++] = ':';
    optpos++;
  }
  if (optionspos + 4 >= optionssize)
    {
      optionssize += 5;
      XSREALLOC (options, char, optionssize);
#if HAVE_GETOPT_LONG
      XSREALLOC (longopts, struct option, optionssize);
#endif
    }
  options[optionspos++] = 'V';
  options[optionspos++] = '?';
  options[optionspos] = '\0';
#if HAVE_GETOPT_LONG
  longopts[longoptionspos].name = "help";
  longopts[longoptionspos].has_arg = no_argument;
  longopts[longoptionspos].flag = NULL;
  longopts[longoptionspos].val = '?';
  longoptionspos++;
  longopts[longoptionspos].name = "version";
  longopts[longoptionspos].has_arg = no_argument;
  longopts[longoptionspos].flag = NULL;
  longopts[longoptionspos].val = 'V';
  longoptionspos++;
  longopts[longoptionspos].name = NULL;
  longopts[longoptionspos].has_arg = 0;
  longopts[longoptionspos].flag = NULL;
  longopts[longoptionspos].val = 0;
#endif

#if HAVE_GETOPT_LONG
  while ((c = getopt_long (argc, argv, options, longopts, NULL)) != -1)
#else
  while ((c = getopt (argc, argv, options)) != -1)
#endif
    {
      switch (c)
	{
	case '?':
	  printf ("Usage: %s [OPTION...]\n%s\n\n", argp_program_name,
		  argps->doc);
	  optpos = 0;
	  while (argps->options[optpos].name != NULL)
	    {
	      if (!(argps->options[optpos].flags & OPTION_HIDDEN))
#if HAVE_GETOPT_LONG
		printf ("  -%c, --%-15s %-12s\t%s\n", 
		        argps->options[optpos].key,
		        argps->options[optpos].name,
			(argps->options[optpos].arg) ?
			 argps->options[optpos].arg : "",
			 argps->options[optpos].doc);
#else
		printf ("  -%c %-12s\t%s\n", argps->options[optpos].key,
			(argps->options[optpos].arg) ?
			argps->options[optpos].arg : "",
			argps->options[optpos].doc);
#endif
	      optpos++;
	    }
	  printf ("\nReport bugs to %s.\n", argp_program_bug_address);
	  exit (0);
	  break;
	case 'V':
	  printf ("%s\n", argp_program_version);
	  exit (0);
	  break;
	default:
	  argps->parser (c, optarg, state);
	  break;
	}
    }
  free (state);
  free (options);
#if HAVE_GETOPT_LONG
  free (longopts);
#endif
  return 0;
}

error_t
argp_error (const struct argp_state *state, char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  fprintf (stderr, "%s: ", argp_program_name);
  vfprintf (stderr, fmt, ap);
  fprintf (stderr, "\nTry `%s -?' for more information.\n", argp_program_name);
  exit (1);
}
#endif

#ifndef HAVE_GETSUBOPT
int ccze_getsubopt (char **optionp, char *const *tokens,
		    char **valuep)
{
  char *endp, *vstart;
  int cnt;

  if (**optionp == '\0')
    return -1;

  /* Find end of next token.  */
  endp = strchr (*optionp, ',');
  if (!endp)
    endp = *optionp + strlen (*optionp);
   
  /* Find start of value.  */
  vstart = memchr (*optionp, '=', endp - *optionp);

  if (vstart == NULL)
    vstart = endp;

  /* Try to match the characters between *OPTIONP and VSTART against
     one of the TOKENS.  */
  for (cnt = 0; tokens[cnt] != NULL; ++cnt)
    if (memcmp (*optionp, tokens[cnt], vstart - *optionp) == 0
	&& tokens[cnt][vstart - *optionp] == '\0')
      {
	/* We found the current option in TOKENS.  */
	*valuep = vstart != endp ? vstart + 1 : NULL;

	if (*endp != '\0')
	  *endp++ = '\0';
	*optionp = endp;

	return cnt;
      }

  /* The current suboption does not match any option.  */
  *valuep = *optionp;

  if (*endp != '\0')
    *endp++ = '\0';
  *optionp = endp;

  return -1;
}
#else
#if HAVE_SUBOPTARG
extern char *suboptarg;
#else
#endif
int
ccze_getsubopt (char **optionp, char *const *tokens,
		char **valuep)
{
  int i = getsubopt (optionp, tokens, valuep);
#if HAVE_SUBOPTARg
  if (!*valuep && suboptarg)
    *valuep = strdup (suboptarg);
#endif
  return i;
}
#endif

#ifndef HAVE_SCANDIR
int
scandir (const char *dir, struct dirent ***namelist,
	 int (*select)(const struct dirent *),
	 int (*compar)(const struct dirent **, const struct dirent **))
{
  DIR *d;
  struct dirent *entry;
  register int i=0;
  size_t entrysize;

  if ((d = opendir (dir)) == NULL)
    return -1;

  *namelist=NULL;
  while ((entry = readdir (d)) != NULL)
    {
      if (select == NULL || (select != NULL && (*select) (entry)))
	{
	  *namelist = (struct dirent **)ccze_realloc
	    ((void *) (*namelist),
	     (size_t)((i + 1) * sizeof (struct dirent *)));
	  if (*namelist == NULL)
	    return -1;

	  entrysize = sizeof (struct dirent) -
	    sizeof (entry->d_name) + strlen (entry->d_name) + 1;
	  (*namelist)[i] = (struct dirent *)ccze_malloc (entrysize);
	  if ((*namelist)[i] == NULL)
	    return -1;
         memcpy ((*namelist)[i], entry, entrysize);
         i++;
	}
    }
  if (closedir (d))
    return -1;
  if (i == 0)
    return -1;
  if (compar != NULL)
    qsort ((void *)(*namelist), (size_t) i, sizeof (struct dirent *),
	   compar);

   return i;
}
#endif

#ifndef HAVE_ALPHASORT
int
alphasort (const struct dirent **a, const struct dirent **b)
{
  return (strcmp ((*a)->d_name, (*b)->d_name));
}
#endif

/* getline() and getdelim() were taken from GNU Mailutils'
   mailbox/getline.c */
/* First implementation by Alain Magloire */
#ifndef HAVE_GETLINE
ssize_t
getline (char **lineptr, size_t *n, FILE *stream)
{
  return getdelim (lineptr, n, '\n', stream);
}
#endif

#ifndef HAVE_GETDELIM
/* Default value for line length.  */
static const int line_size = 128;

ssize_t
getdelim (char **lineptr, size_t *n, int delim, FILE *stream)
{
  size_t indx = 0;
  int c;

  /* Sanity checks.  */
  if (lineptr == NULL || n == NULL || stream == NULL)
    return -1;

  /* Allocate the line the first time.  */
  if (*lineptr == NULL)
    {
      *lineptr = ccze_malloc (line_size);
      if (*lineptr == NULL)
	return -1;
      *n = line_size;
    }

  while ((c = getc (stream)) != EOF)
    {
      /* Check if more memory is needed.  */
      if (indx >= *n)
	{
	  *lineptr = ccze_realloc (*lineptr, *n + line_size);
	  if (*lineptr == NULL)
	    return -1;
	  *n += line_size;
	}

      /* Push the result in the line.  */
      (*lineptr)[indx++] = c;

      /* Bail out.  */
      if (c == delim)
	break;
    }

  /* Make room for the null character.  */
  if (indx >= *n)
    {
      *lineptr = ccze_realloc (*lineptr, *n + line_size);
      if (*lineptr == NULL)
       return -1;
      *n += line_size;
    }

  /* Null terminate the buffer.  */
  (*lineptr)[indx++] = 0;

  /* The last line may not have the delimiter, we have to
   * return what we got and the error will be seen on the
   * next iteration.  */
  return (c == EOF && (indx - 1) == 0) ? -1 : (ssize_t)(indx - 1);
}
#endif

#ifndef HAVE_ASPRINTF
int
asprintf(char **ptr, const char *fmt, ...) 
{
  va_list ap;
  size_t size = 1024;
  int n;
  
  if ((*ptr = ccze_malloc (size)) == NULL)
    return -1;

  while (1)
    {
      va_start (ap, fmt);
      n = vsnprintf (*ptr, size, fmt, ap);
      va_end (ap);

      if (n > -1 && n < (long) size)
	return n;

      if (n > -1)    /* glibc 2.1 */
	size = n+1;
      else           /* glibc 2.0 */
	size *= 2;

      if ((*ptr = ccze_realloc (*ptr, size)) == NULL)
	return -1;
    }
}
#endif
