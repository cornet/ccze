/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_vstpd.c -- VSFTPd-related colorizers for CCZE
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

static void ccze_vsftpd_setup (void);
static void ccze_vsftpd_shutdown (void);
static int ccze_vsftpd_handle (const char *str, size_t length, char **rest);

static pcre *reg_vsftpd;
static pcre_extra *hints_vsftpd;

static char *
ccze_vsftpd_log_process (const char *str, int *offsets, int match)
{
  char *date, *sspace, *pid, *user, *other;

  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
  pcre_get_substring (str, offsets, match, 2, (const char **)&sspace);
  pcre_get_substring (str, offsets, match, 3, (const char **)&pid);
  pcre_get_substring (str, offsets, match, 5, (const char **)&user);
  pcre_get_substring (str, offsets, match, 6, (const char **)&other);
  
  ccze_addstr (CCZE_COLOR_DATE, date);
  ccze_addstr (CCZE_COLOR_DEFAULT, sspace);

  ccze_addstr (CCZE_COLOR_PIDB, "[");
  ccze_addstr (CCZE_COLOR_DEFAULT, "pid ");
  ccze_addstr (CCZE_COLOR_PID, pid);
  ccze_addstr (CCZE_COLOR_PIDB, "]");
  ccze_space();

  if (*user)
    {
      ccze_addstr (CCZE_COLOR_PIDB, "[");
      ccze_addstr (CCZE_COLOR_USER, user);
      ccze_addstr (CCZE_COLOR_PIDB, "]");
      ccze_space ();
    }

  free (date);
  free (sspace);
  free (pid);
  free (user);
  
  return other;
}

static void
ccze_vsftpd_setup (void)
{
  const char *error;
  int errptr;

  reg_vsftpd = pcre_compile
    ("^(\\S+\\s+\\S+\\s+\\d{1,2}\\s+\\d{1,2}:\\d{1,2}:\\d{1,2}\\s+\\d+)"
     "(\\s+)\\[pid (\\d+)\\]\\s+(\\[(\\S+)\\])?\\s*(.*)$", 0, &error,
     &errptr, NULL);
  hints_vsftpd = pcre_study (reg_vsftpd, 0, &error);
}

static void
ccze_vsftpd_shutdown (void)
{
  free (reg_vsftpd);
  free (hints_vsftpd);
}

static int
ccze_vsftpd_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];
  
  if ((match = pcre_exec (reg_vsftpd, hints_vsftpd, str, length,
			  0, 0, offsets, 99)) >= 0)
    {
      *rest = ccze_vsftpd_log_process (str, offsets, match);
      return 1;
    }
  
  return 0;
}

CCZE_DEFINE_PLUGIN (vsftpd, FULL, "Coloriser for vsftpd(8) logs.");
