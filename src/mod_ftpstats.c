/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_ftpstats.c -- Stats coloriser for CCZE.
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

static void ccze_ftpstats_setup (void);
static void ccze_ftpstats_shutdown (void);
static int ccze_ftpstats_handle (const char *str, size_t length, char **rest);

static pcre *reg_ftpstats;
static pcre_extra *hints_ftpstats;

static char *
ccze_ftpstats_process (const char *str, int *offsets, int match)
{
  char *date, *sessionid, *user, *host, *type, *size, *duration, *file;

  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
  pcre_get_substring (str, offsets, match, 2, (const char **)&sessionid);
  pcre_get_substring (str, offsets, match, 3, (const char **)&user);
  pcre_get_substring (str, offsets, match, 4, (const char **)&host);
  pcre_get_substring (str, offsets, match, 5, (const char **)&type);
  pcre_get_substring (str, offsets, match, 6, (const char **)&size);
  pcre_get_substring (str, offsets, match, 7, (const char **)&duration);
  pcre_get_substring (str, offsets, match, 8, (const char **)&file);

  ccze_print_date (date);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_UNIQN, sessionid);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_USER, user);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_HOST, host);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_FTPCODES, type);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_GETSIZE, size);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_DATE, duration);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_DIR, file);
  ccze_newline ();

  free (file);
  free (duration);
  free (size);
  free (type);
  free (host);
  free (user);
  free (sessionid);
  free (date);
  
  return NULL;
}

static void
ccze_ftpstats_setup (void)
{
  const char *error;
  int errptr;

  reg_ftpstats = pcre_compile
    ("^(\\d{9,10})\\s([\\da-f]+\\.[\\da-f]+)\\s([^\\s]+)\\s([^\\s]+)"
     "\\s(U|D)\\s(\\d+)\\s(\\d+)\\s(.*)$",
     0, &error, &errptr, NULL);
  hints_ftpstats = pcre_study (reg_ftpstats, 0, &error);
}

static void
ccze_ftpstats_shutdown (void)
{
  free (reg_ftpstats);
  free (hints_ftpstats);
}

static int
ccze_ftpstats_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];
  
  if ((match = pcre_exec (reg_ftpstats, hints_ftpstats, str, length,
			  0, 0, offsets, 99)) >= 0)
    {
      *rest = ccze_ftpstats_process (str, offsets, match);
      return 1;
    }
  
  return 0;
}

CCZE_DEFINE_PLUGIN (ftpstats, FULL, "Coloriser for ftpstats (pure-ftpd) logs.");
