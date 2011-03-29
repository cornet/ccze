/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_postfix.c -- Postfix colorizer for CCZE
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
#include <string.h>
#include <stdlib.h>

#include "ccze-compat.h"

static void ccze_postfix_setup (void);
static void ccze_postfix_shutdown (void);
static int ccze_postfix_handle (const char *str, size_t length, char **rest);

static pcre *reg_postfix;

static int
_ccze_postfix_process_one (const char *s, char **rest)
{
  char *field, *value;
  size_t i;
  
  if (!strchr (s, '='))
    return 1;

  field = strndup (s, strchr (s, '=') - s);
  i = strlen (s);
  i -= strlen (field) + 1;
  value = strndup (&s[strlen (field) + 1], i);
  
  ccze_addstr (CCZE_COLOR_FIELD, field);
  ccze_addstr (CCZE_COLOR_DEFAULT, "=");
  ccze_wordcolor_process_one (value, 1);

  return 0;
}

static char *
ccze_postfix_process (const char *str, int *offsets, int match)
{
  char *spoolid, *s, *rest, *tmp;
  int r;
  
  pcre_get_substring (str, offsets, match, 1, (const char **)&spoolid);
  pcre_get_substring (str, offsets, match, 2, (const char **)&s);
  pcre_get_substring (str, offsets, match, 4, (const char **)&rest);

  ccze_addstr (CCZE_COLOR_UNIQN, spoolid);
  ccze_addstr (CCZE_COLOR_DEFAULT, ": ");

  tmp = ccze_strbrk (s, ',');
  
  do
    {
      r = _ccze_postfix_process_one (tmp, &rest);
      if (r)
	ccze_addstr (CCZE_COLOR_DEFAULT, tmp);
      else
	tmp = ccze_strbrk (NULL, ',');
      if (tmp)
	ccze_addstr (CCZE_COLOR_DEFAULT, ",");
    } while (!r && (tmp != NULL));
  
  return NULL;
}

static void
ccze_postfix_setup (void)
{
  const char *error;
  int errptr;

  reg_postfix = pcre_compile
    ("^([\\dA-F]+): ((client|to|message-id|uid|resent-message-id|from)(=.*))",
     0, &error, &errptr, NULL);
}

static void
ccze_postfix_shutdown (void)
{
  free (reg_postfix);
}

static int
ccze_postfix_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];
  
  if ((match = pcre_exec (reg_postfix, NULL, str, length,
			  0, 0, offsets, 99)) >= 0)
    {
      if (rest)
	*rest = ccze_postfix_process (str, offsets, match);
      else
	ccze_postfix_process (str, offsets, match);
      
      return 1;
    }
    
  return 0;
}

CCZE_DEFINE_PLUGIN (postfix, PARTIAL, "Coloriser for postfix(1) sub-logs.");
