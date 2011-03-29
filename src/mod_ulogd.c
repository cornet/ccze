/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_ulogd.c -- ulogd-related colorizers for CCZE
 * Copyright (C) 2002, 2003 Gergely Nagy <algernon@bonehunter.rulez.org>
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
#include <string.h>
#include <stdlib.h>

#include "ccze-compat.h"

static void ccze_ulogd_setup (void);
static void ccze_ulogd_shutdown (void);
static int ccze_ulogd_handle (const char *str, size_t length, char **rest);

static pcre *reg_ulogd;

static char *
ccze_ulogd_process (const char *msg)
{
  char *word, *tmp, *field, *value;
  char *msg2 = xstrdup (msg);
  
  word = xstrdup (ccze_strbrk (msg2, ' '));
  do
    {
      if ((tmp = strchr (word, '=')) != NULL)
	{
	  field = strndup (word, tmp - word);
	  value = strdup (tmp + 1);
	  ccze_addstr (CCZE_COLOR_FIELD, field);
	  ccze_addstr (CCZE_COLOR_DEFAULT, "=");
	  ccze_wordcolor_process_one (value, 1);
	  free (field);
	  ccze_space ();
	}
      else
	{
	  ccze_addstr (CCZE_COLOR_FIELD, word);
	  ccze_space ();
	}
    } while ((word = xstrdup (ccze_strbrk (NULL, ' '))) != NULL);
  free (msg2);
  
  return NULL;
}

static void
ccze_ulogd_setup (void)
{
  const char *error;
  int errptr;

  reg_ulogd = pcre_compile
    ("(IN|OUT|MAC|TTL|SRC|TOS|PREC|SPT)=", 0, &error,
     &errptr, NULL);
}

static void
ccze_ulogd_shutdown (void)
{
  free (reg_ulogd);
}

static int
ccze_ulogd_handle (const char *str, size_t length, char **rest)
{
  int offsets[10];
  
  if (pcre_exec (reg_ulogd, NULL, str, length, 0, 0, offsets, 2) >= 0)
    {
      if (rest)
	*rest = ccze_ulogd_process (str);
      else
	ccze_ulogd_process (str);
      
      return 1;
    }
  
  return 0;
}

CCZE_DEFINE_PLUGIN (ulogd, PARTIAL, "Coloriser for ulogd sub-logs.");
