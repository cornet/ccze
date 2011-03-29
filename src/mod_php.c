/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_php.c -- php.log colorizers for CCZE
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

static void ccze_php_setup (void);
static void ccze_php_shutdown (void);
static int ccze_php_handle (const char *str, size_t length, char **rest);

static pcre *reg_php;
static pcre_extra *hints_php;

static char *
ccze_php_process (const char *str, int *offsets, int match)
{
  char *date = NULL, *rest = NULL;
  
  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
  pcre_get_substring (str, offsets, match, 2, (const char **)&rest);
  
  ccze_addstr (CCZE_COLOR_DATE, date);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_KEYWORD, "PHP");
  ccze_space ();

  free (date);

  return rest;
}

static void
ccze_php_setup (void)
{
  const char *error;
  int errptr;

  reg_php = pcre_compile ("^(\\[\\d+-...-\\d+ \\d+:\\d+:\\d+\\]) PHP (.*)$",
			  0, &error, &errptr, NULL);
  hints_php = pcre_study (reg_php, 0, &error);
}

static void
ccze_php_shutdown (void)
{
  free (reg_php);
  free (hints_php);
}

static int
ccze_php_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];
  
  if ((match = pcre_exec (reg_php, hints_php, str, length,
			  0, 0, offsets, 99)) >= 0)
    {
      *rest = ccze_php_process (str, offsets, match);
      return 1;
    }
  
  return 0;
}

CCZE_DEFINE_PLUGIN (php, FULL, "Coloriser for PHP logs.");
