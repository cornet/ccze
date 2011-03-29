/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_xferlog -- Xferlog colorizer for CCZE.
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
#include <stdlib.h>

static void ccze_xferlog_setup (void);
static void ccze_xferlog_shutdown (void);
static int ccze_xferlog_handle (const char *str, size_t length, char **rest);

static pcre *reg_xferlog;
static pcre_extra *hints_xferlog;

static char *
ccze_xferlog_log_process (const char *str, int *offsets, int match)
{
  char *curtime, *transtime, *host, *fsize, *fname, *transtype;
  char *actionflag, *direction, *amode, *user, *service, *amethod;
  char *auid, *status;

  pcre_get_substring (str, offsets, match, 1, (const char **)&curtime);
  pcre_get_substring (str, offsets, match, 2, (const char **)&transtime);
  pcre_get_substring (str, offsets, match, 3, (const char **)&host);
  pcre_get_substring (str, offsets, match, 4, (const char **)&fsize);
  pcre_get_substring (str, offsets, match, 5, (const char **)&fname);
  pcre_get_substring (str, offsets, match, 6, (const char **)&transtype);
  pcre_get_substring (str, offsets, match, 7, (const char **)&actionflag);
  pcre_get_substring (str, offsets, match, 8, (const char **)&direction);
  pcre_get_substring (str, offsets, match, 9, (const char **)&amode);
  pcre_get_substring (str, offsets, match, 10, (const char **)&user);
  pcre_get_substring (str, offsets, match, 11, (const char **)&service);
  pcre_get_substring (str, offsets, match, 12, (const char **)&amethod);
  pcre_get_substring (str, offsets, match, 13, (const char **)&auid);
  pcre_get_substring (str, offsets, match, 14, (const char **)&status);
  
  ccze_addstr (CCZE_COLOR_DATE, curtime);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_GETTIME, transtime);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_HOST, host);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_GETSIZE, fsize);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_DIR, fname);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_PIDB, transtype);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_FTPCODES, actionflag);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_FTPCODES, direction);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_FTPCODES, amode);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_USER, user);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_SERVICE, service);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_FTPCODES, amethod);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_USER, auid);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_FTPCODES, status);

  ccze_newline ();

  free (curtime);
  free (transtime);
  free (host);
  free (fsize);
  free (fname);
  free (transtype);
  free (actionflag);
  free (direction);
  free (amode);
  free (user);
  free (service);
  free (amethod);
  free (auid);
  free (status);

  return NULL;
}

static void
ccze_xferlog_setup (void)
{
  const char *error;
  int errptr;

  /* FIXME: Does not handle spaces in filenames! */
  reg_xferlog = pcre_compile
    ("^(... ... +\\d{1,2} +\\d{1,2}:\\d{1,2}:\\d{1,2} \\d+) (\\d+) ([^ ]+) "
     "(\\d+) (\\S+) (a|b) (C|U|T|_) (o|i) (a|g|r) ([^ ]+) ([^ ]+) " 
     "(0|1) ([^ ]+) (c|i)", 0, &error,
     &errptr, NULL);
  hints_xferlog = pcre_study (reg_xferlog, 0, &error);
}

static void
ccze_xferlog_shutdown (void)
{
  free (reg_xferlog);
  free (hints_xferlog);
}

static int
ccze_xferlog_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];
  
  if ((match = pcre_exec (reg_xferlog, hints_xferlog, str, length,
			  0, 0, offsets, 99)) >= 0)
    {
      *rest = ccze_xferlog_log_process (str, offsets, match);
      return 1;
    }
  
  return 0;
}

CCZE_DEFINE_PLUGIN (xferlog, FULL, "Generic xferlog coloriser.");
