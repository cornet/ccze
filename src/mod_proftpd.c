/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_proftpd.c -- proftpd-related colorizers for CCZE
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
#include <string.h>

static void ccze_proftpd_setup (void);
static void ccze_proftpd_shutdown (void);
static int ccze_proftpd_handle (const char *str, size_t length, char **rest);

static pcre *reg_proftpd_access, *reg_proftpd_auth;
static pcre_extra *hints_proftpd_access, *hints_proftpd_auth;

static char *
ccze_proftpd_access_log_process (const char *str, int *offsets, int match)
{
  char *host, *user, *auser, *date, *command, *file, *ftpcode, *size;

  pcre_get_substring (str, offsets, match, 1, (const char **)&host);
  pcre_get_substring (str, offsets, match, 2, (const char **)&user);
  pcre_get_substring (str, offsets, match, 3, (const char **)&auser);
  pcre_get_substring (str, offsets, match, 4, (const char **)&date);
  pcre_get_substring (str, offsets, match, 5, (const char **)&command);
  pcre_get_substring (str, offsets, match, 6, (const char **)&file);
  pcre_get_substring (str, offsets, match, 7, (const char **)&ftpcode);
  pcre_get_substring (str, offsets, match, 8, (const char **)&size);

  ccze_addstr (CCZE_COLOR_HOST, host);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_USER, user);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_USER, auser);
  ccze_space ();

  ccze_addstr (CCZE_COLOR_DEFAULT, "[");
  ccze_print_date (date);
  ccze_addstr (CCZE_COLOR_DEFAULT, "]");
  ccze_space ();

  ccze_addstr (CCZE_COLOR_DEFAULT, "\"");
  ccze_addstr (CCZE_COLOR_KEYWORD, command);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_URI, file);
  ccze_addstr (CCZE_COLOR_DEFAULT, "\"");
  ccze_space ();

  ccze_addstr (CCZE_COLOR_FTPCODES, ftpcode);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_GETSIZE, size);
  
  ccze_newline ();

  free (size);
  free (ftpcode);
  free (file);
  free (command);
  free (date);
  free (auser);
  free (user);
  free (host);

  return NULL;
}

static char *
ccze_proftpd_auth_log_process (const char *str, int *offsets, int match)
{
  char *servhost, *pid, *remhost, *date, *cmd, *value, *ftpcode;
  
  pcre_get_substring (str, offsets, match, 1, (const char **)&servhost);
  pcre_get_substring (str, offsets, match, 2, (const char **)&pid);
  pcre_get_substring (str, offsets, match, 3, (const char **)&remhost);
  pcre_get_substring (str, offsets, match, 4, (const char **)&date);
  pcre_get_substring (str, offsets, match, 5, (const char **)&cmd);
  pcre_get_substring (str, offsets, match, 6, (const char **)&value);
  pcre_get_substring (str, offsets, match, 7, (const char **)&ftpcode);

  ccze_addstr (CCZE_COLOR_HOST, servhost);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_DEFAULT, "ftp server");
  ccze_space ();
  ccze_addstr (CCZE_COLOR_PIDB, "[");
  ccze_addstr (CCZE_COLOR_PID, pid);
  ccze_addstr (CCZE_COLOR_PIDB, "]");
  ccze_space ();
  ccze_addstr (CCZE_COLOR_HOST, remhost);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_DEFAULT, "[");
  ccze_print_date (date);
  ccze_addstr (CCZE_COLOR_DEFAULT, "]");
  ccze_space ();
  ccze_addstr (CCZE_COLOR_DEFAULT, "\"");
  ccze_addstr (CCZE_COLOR_KEYWORD, cmd);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_DEFAULT, value);
  ccze_addstr (CCZE_COLOR_DEFAULT, "\"");
  ccze_space ();
  ccze_addstr (CCZE_COLOR_FTPCODES, ftpcode);
  
  ccze_newline ();
    
  free (ftpcode);
  free (value);
  free (cmd);
  free (date);
  free (remhost);
  free (pid);
  free (servhost);

  return NULL;
}

static void
ccze_proftpd_setup (void)
{
  const char *error;
  int errptr;

  reg_proftpd_access = pcre_compile
    ("^(\\d+\\.\\d+\\.\\d+\\.\\d+) (\\S+) (\\S+) "
     "\\[(\\d{2}/.{3}/\\d{4}:\\d{2}:\\d{2}:\\d{2} [\\-\\+]\\d{4})\\] "
     "\"([A-Z]+) ([^\"]+)\" (\\d{3}) (-|\\d+)", 0, &error, &errptr, NULL);
  hints_proftpd_access = pcre_study (reg_proftpd_access, 0, &error);

  reg_proftpd_auth = pcre_compile
    ("^(\\S+) ftp server \\[(\\d+)\\] (\\d+\\.\\d+\\.\\d+\\.\\d+) "
     "\\[(\\d{2}/.{3}/\\d{4}:\\d{2}:\\d{2}:\\d{2} [\\-\\+]\\d{4})\\] "
     "\"([A-Z]+) ([^\"]+)\" (\\d{3})", 0, &error, &errptr, NULL);
  hints_proftpd_auth = pcre_study (reg_proftpd_auth, 0, &error);
}

static void
ccze_proftpd_shutdown (void)
{
  free (reg_proftpd_auth);
  free (hints_proftpd_auth);
  free (reg_proftpd_access);
  free (hints_proftpd_access);
}

static int
ccze_proftpd_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];

  if ((match = pcre_exec (reg_proftpd_access, hints_proftpd_access,
			  str, length, 0, 0, offsets, 99)) >= 0)
    {
      *rest = ccze_proftpd_access_log_process (str, offsets, match);
      return 1;
    }
  if ((match = pcre_exec (reg_proftpd_auth, hints_proftpd_auth,
			  str, length, 0, 0, offsets, 99)) >= 0)
    {
      *rest = ccze_proftpd_auth_log_process (str, offsets, match);
      return 1;
    }

  return 0;
}

CCZE_DEFINE_PLUGIN (proftpd, FULL,
		    "Coloriser for proftpd access and auth logs.");
