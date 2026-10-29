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

static ccze_regex_t *reg_xferlog;


static char *
ccze_xferlog_log_process (const char *str, int *offsets, int match)
{
  char *curtime, *transtime, *host, *fsize, *fname, *transtype;
  char *actionflag, *direction, *amode, *user, *service, *amethod;
  char *auid, *status;

  ccze_regex_substring (str, offsets, match, 1, (const char **)&curtime);
  ccze_regex_substring (str, offsets, match, 2, (const char **)&transtime);
  ccze_regex_substring (str, offsets, match, 3, (const char **)&host);
  ccze_regex_substring (str, offsets, match, 4, (const char **)&fsize);
  ccze_regex_substring (str, offsets, match, 5, (const char **)&fname);
  ccze_regex_substring (str, offsets, match, 6, (const char **)&transtype);
  ccze_regex_substring (str, offsets, match, 7, (const char **)&actionflag);
  ccze_regex_substring (str, offsets, match, 8, (const char **)&direction);
  ccze_regex_substring (str, offsets, match, 9, (const char **)&amode);
  ccze_regex_substring (str, offsets, match, 10, (const char **)&user);
  ccze_regex_substring (str, offsets, match, 11, (const char **)&service);
  ccze_regex_substring (str, offsets, match, 12, (const char **)&amethod);
  ccze_regex_substring (str, offsets, match, 13, (const char **)&auid);
  ccze_regex_substring (str, offsets, match, 14, (const char **)&status);
  
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


  /* FIXME: Does not handle spaces in filenames! */
  reg_xferlog = ccze_regex_compile
    ("^(... ... +\\d{1,2} +\\d{1,2}:\\d{1,2}:\\d{1,2} \\d+) (\\d+) ([^ ]+) "
     "(\\d+) (\\S+) (a|b) (C|U|T|_) (o|i) (a|g|r) ([^ ]+) ([^ ]+) " 
     "(0|1) ([^ ]+) (c|i)");
}

static void
ccze_xferlog_shutdown (void)
{
  ccze_regex_free (reg_xferlog);
}

static int
ccze_xferlog_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];
  
  if ((match = ccze_regex_match (reg_xferlog, str, length, offsets, 99)) >= 0)
    {
      *rest = ccze_xferlog_log_process (str, offsets, match);
      return 1;
    }
  
  return 0;
}

CCZE_DEFINE_PLUGIN (xferlog, FULL, "Generic xferlog coloriser.");
