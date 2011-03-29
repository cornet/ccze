/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_sulog.c -- su-related colorizers for CCZE
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

static void ccze_sulog_setup (void);
static void ccze_sulog_shutdown (void);
static int ccze_sulog_handle (const char *str, size_t length, char **rest);

static pcre *reg_sulog;
static pcre_extra *hints_sulog;

static char *
ccze_sulog_process (const char *str, int *offsets, int match)
{
  char *date, *islogin, *tty, *fromuser, *touser;

  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
  pcre_get_substring (str, offsets, match, 2, (const char **)&islogin);
  pcre_get_substring (str, offsets, match, 3, (const char **)&tty);
  pcre_get_substring (str, offsets, match, 4, (const char **)&fromuser);
  pcre_get_substring (str, offsets, match, 5, (const char **)&touser);

  ccze_addstr (CCZE_COLOR_DEFAULT, "SU ");
  ccze_addstr (CCZE_COLOR_DATE, date);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_DEFAULT, islogin);
  ccze_space ();
  switch (tty[0])
    {
    case '?':
      ccze_addstr (CCZE_COLOR_UNKNOWN, tty);
      break;
    default:
      ccze_addstr (CCZE_COLOR_DIR, tty);
      break;
    }
  ccze_space ();
  ccze_addstr (CCZE_COLOR_USER, fromuser);
  ccze_addstr (CCZE_COLOR_DEFAULT, "-");
  ccze_addstr (CCZE_COLOR_USER, touser);
  
  ccze_newline ();

  free (date);
  free (islogin);
  free (tty);
  free (fromuser);
  free (touser);

  return NULL;
}

static void
ccze_sulog_setup (void)
{
  const char *error;
  int errptr;

  reg_sulog = pcre_compile ("^SU (\\d{2}\\/\\d{2} \\d{2}:\\d{2}) ([\\+\\-]) "
			    "(\\S+) ([^\\-]+)-(.*)$", 0, &error, &errptr,
			    NULL);
  hints_sulog = pcre_study (reg_sulog, 0, &error);
}

static void
ccze_sulog_shutdown (void)
{
  free (reg_sulog);
  free (hints_sulog);
}

static int
ccze_sulog_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];
  
  if ((match = pcre_exec (reg_sulog, hints_sulog, str, length,
			  0, 0, offsets, 99)) >= 0)
    {
      *rest = ccze_sulog_process (str, offsets, match);
      return 1;
    }
  
  return 0;
}

CCZE_DEFINE_PLUGIN (sulog, FULL, "Coloriser for su(1) logs.");
