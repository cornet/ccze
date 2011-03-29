/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_oops.c -- oops/oops.log colorizers for CCZE
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

static void ccze_oops_setup (void);
static void ccze_oops_shutdown (void);
static int ccze_oops_handle (const char *str, size_t length, char **rest);

static pcre *reg_oops;
static pcre_extra *hints_oops;

static char *
ccze_oops_process (const char *str, int *offsets, int match)
{
  char *date, *sp1, *id, *field, *sp2, *value, *etc;
  
  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
  pcre_get_substring (str, offsets, match, 4, (const char **)&sp1);
  pcre_get_substring (str, offsets, match, 5, (const char **)&id);
  pcre_get_substring (str, offsets, match, 6, (const char **)&field);
  pcre_get_substring (str, offsets, match, 7, (const char **)&sp2);
  pcre_get_substring (str, offsets, match, 8, (const char **)&value);
  pcre_get_substring (str, offsets, match, 9, (const char **)&etc);
  
  ccze_addstr (CCZE_COLOR_DATE, date);
  ccze_addstr (CCZE_COLOR_DEFAULT, sp1);

  ccze_addstr (CCZE_COLOR_PIDB, "[");
  ccze_addstr (CCZE_COLOR_PROC, id);
  ccze_addstr (CCZE_COLOR_PIDB, "]");

  ccze_addstr (CCZE_COLOR_KEYWORD, "statistics()");
  ccze_addstr (CCZE_COLOR_DEFAULT, ":");
  ccze_space ();

  ccze_addstr (CCZE_COLOR_FIELD, field);
  ccze_addstr (CCZE_COLOR_DEFAULT, sp2);
  ccze_addstr (CCZE_COLOR_DEFAULT, ":");
  ccze_space ();
  ccze_addstr (CCZE_COLOR_NUMBERS, value);
  ccze_addstr (CCZE_COLOR_DEFAULT, etc);

  ccze_newline ();
  
  free (date);
  free (sp1);
  free (id);
  free (field);
  free (sp2);
  free (value);
  free (etc);
  
  return NULL;
}

static void
ccze_oops_setup (void)
{
  const char *error;
  int errptr;

  reg_oops = pcre_compile
    ("^((Mon|Tue|Wed|Thu|Fri|Sat|Sun) "
     "(Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) "
     "\\d+ \\d+:\\d+:\\d+ \\d+)(\\s+)\\[([\\dxa-fA-F]+)\\]"
     "statistics\\(\\): ([\\S]+)(\\s*): (\\d+)(.*)",
     0, &error, &errptr, NULL);
  hints_oops = pcre_study (reg_oops, 0, &error);
}

static void
ccze_oops_shutdown (void)
{
  free (reg_oops);
  free (hints_oops);
}

static int
ccze_oops_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];
  
  if ((match = pcre_exec (reg_oops, hints_oops, str, length,
			  0, 0, offsets, 99)) >= 0)
    {
      *rest = ccze_oops_process (str, offsets, match);
      return 1;
    }
  
  return 0;
}

CCZE_DEFINE_PLUGIN (oops, FULL, "Coloriser for oops proxy logs.");
