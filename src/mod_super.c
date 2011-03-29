/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_super.c -- super-related colorizers for CCZE
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
#include <stdlib.h>

static void ccze_super_setup (void);
static void ccze_super_shutdown (void);
static int ccze_super_handle (const char *str, size_t length, char **rest);

static pcre *reg_super;
static pcre_extra *hints_super;

static char *
ccze_super_process (const char *str, int *offsets, int match)
{
  char *email, *date, *space, *suptag, *other;

  pcre_get_substring (str, offsets, match, 1, (const char **)&email);
  pcre_get_substring (str, offsets, match, 2, (const char **)&date);
  pcre_get_substring (str, offsets, match, 3, (const char **)&space);
  pcre_get_substring (str, offsets, match, 4, (const char **)&suptag);
  pcre_get_substring (str, offsets, match, 5, (const char **)&other);

  ccze_addstr (CCZE_COLOR_EMAIL, email);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_DATE, date);
  ccze_addstr (CCZE_COLOR_DEFAULT, space);
  ccze_addstr (CCZE_COLOR_PROC, suptag);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_PIDB, "(");
  ccze_addstr (CCZE_COLOR_DEFAULT, other);
  ccze_addstr (CCZE_COLOR_PIDB, ")");

  ccze_newline ();
  
  return NULL;
}

static void
ccze_super_setup (void)
{
  const char *error;
  int errptr;

  reg_super = pcre_compile
    ("^(\\S+)\\s(\\w+\\s+\\w+\\s+\\d+\\s+\\d+:\\d+:\\d+\\s+\\d+)"
     "(\\s+)(\\S+)\\s\\(([^\\)]+)\\)", 0, &error, &errptr, NULL);
  hints_super = pcre_study (reg_super, 0, &error);
}

static void
ccze_super_shutdown (void)
{
  free (reg_super);
  free (hints_super);
}

static int
ccze_super_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];
  
  if ((match = pcre_exec (reg_super, hints_super, str, length,
			  0, 0, offsets, 99)) >= 0)
    {
      *rest = ccze_super_process (str, offsets, match);
      return 1;
    }
  
  return 0;
}

CCZE_DEFINE_PLUGIN (super, FULL, "Coloriser for super(1) logs.");
