/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_procmail.c -- procmail-related colorizers for CCZE
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

static void ccze_procmail_setup (void);
static void ccze_procmail_shutdown (void);
static int ccze_procmail_handle (const char *str, size_t length, char **rest);

static pcre *reg_procmail;
static pcre_extra *hints_procmail;

static char *
ccze_procmail_process (const char *str, int *offsets, int match)
{
  char *header = NULL, *value = NULL, *space1 = NULL;
  char *space2 = NULL, *extra = NULL;
  int handled = 0;
  ccze_color_t col = CCZE_COLOR_UNKNOWN;

  pcre_get_substring (str, offsets, match, 1, (const char **)&space1);
  pcre_get_substring (str, offsets, match, 2, (const char **)&header);
  pcre_get_substring (str, offsets, match, 3, (const char **)&value);
  pcre_get_substring (str, offsets, match, 4, (const char **)&space2);
  pcre_get_substring (str, offsets, match, 5, (const char **)&extra);
  
  if (!strcasecmp ("from", header) || !strcasecmp (">from", header))
    {
      col = CCZE_COLOR_EMAIL;
      handled = 1;
    }
  if (!strcasecmp ("subject:", header))
    {
      col = CCZE_COLOR_SUBJECT;
      handled = 1;
    }
  if (!strcasecmp ("folder:", header))
    {
      col = CCZE_COLOR_DIR;
      handled = 1;
    }

  if (!handled)
    {
      free (header);
      free (value);
      free (extra);
      return strdup (str);
    }

  ccze_addstr (CCZE_COLOR_DEFAULT, space1);
  ccze_addstr (CCZE_COLOR_DEFAULT, header);
  ccze_space ();

  ccze_addstr (col, value);
  if (col == CCZE_COLOR_EMAIL)
    col = CCZE_COLOR_DEFAULT;
  ccze_addstr (col, space2);

  if (!strcasecmp ("folder:", header))
    col = CCZE_COLOR_SIZE;
  else if (!strcasecmp ("from", header))
    col = CCZE_COLOR_DATE;

  ccze_addstr (col, extra);
  ccze_newline();
    
  free (extra);
  free (header);
  free (value);

  return NULL;
}

static void
ccze_procmail_setup (void)
{
  const char *error;
  int errptr;

  reg_procmail = pcre_compile
    ("^(\\s*)(>?From|Subject:|Folder:)?\\s(\\S+)(\\s+)?(.*)", 0,
     &error, &errptr, NULL);
  hints_procmail = pcre_study (reg_procmail, 0, &error);
}

static void
ccze_procmail_shutdown (void)
{
  free (reg_procmail);
  free (hints_procmail);
}

static int
ccze_procmail_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];
  
  if ((match = pcre_exec (reg_procmail, hints_procmail, str, length,
			  0, 0, offsets, 99)) >= 0)
    {
      *rest = ccze_procmail_process (str, offsets, match);
      return 1;
    }
  
  return 0;
}

CCZE_DEFINE_PLUGIN (procmail, FULL, "Coloriser for procmail(1) logs.");
