/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_dpkg.c -- Dpkg log-coloriser module for CCZE
 * Copyright (C) 2007 arno. <arno.@no-log.org>
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

static void ccze_dpkg_setup (void);
static void ccze_dpkg_shutdown (void);
static int ccze_dpkg_handle (const char *str, size_t length, char **rest);


static pcre *reg_dpkg_status, *reg_dpkg_action, *reg_dpkg_conffile;

static char *
ccze_dpkg_status_process(const char *str, int *offsets, int match)
{
  char *date, *state, *pkg, *installed_version;

  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
  pcre_get_substring (str, offsets, match, 2, (const char **)&state);
  pcre_get_substring (str, offsets, match, 3, (const char **)&pkg);
  pcre_get_substring (str, offsets, match, 4, (const char **)&installed_version);

  ccze_print_date(date);
  ccze_space();
  ccze_addstr(CCZE_COLOR_KEYWORD, "status");
  ccze_space();
  ccze_addstr(CCZE_COLOR_PKGSTATUS, state);
  ccze_space();
  ccze_addstr(CCZE_COLOR_PKG, pkg);
  ccze_space();
  ccze_addstr(CCZE_COLOR_DEFAULT, installed_version);
  ccze_newline();

  free(date);
  free(state);
  free(pkg);
  free(installed_version);

  return NULL;
}

static char *
ccze_dpkg_action_process(const char *str, int *offsets, int match)
{
  char *date, *action, *pkg, *installed_version, *available_version;

  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
  pcre_get_substring (str, offsets, match, 2, (const char **)&action);
  pcre_get_substring (str, offsets, match, 3, (const char **)&pkg);
  pcre_get_substring (str, offsets, match, 4, (const char **)&installed_version);
  pcre_get_substring (str, offsets, match, 5, (const char **)&available_version);

  ccze_print_date(date);
  ccze_space();
  ccze_addstr(CCZE_COLOR_KEYWORD, action);
  ccze_space();
  ccze_addstr(CCZE_COLOR_PKG, pkg);
  ccze_space();
  ccze_addstr(CCZE_COLOR_DEFAULT, installed_version);
  ccze_space();
  ccze_addstr(CCZE_COLOR_DEFAULT, available_version);
  ccze_newline();

  free(date);
  free(action);
  free(pkg);
  free(installed_version);
  free(available_version);

  return NULL;
}

static char *
ccze_dpkg_conffile_process(const char *str, int *offsets, int match)
{
  char *date, *filename, *decision;
  /* YYYY-MM-DD HH:MM:SS conffile <filename> <decision> */

  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
  pcre_get_substring (str, offsets, match, 2, (const char **)&filename);
  pcre_get_substring (str, offsets, match, 3, (const char **)&decision);

  ccze_print_date(date);
  ccze_space();
  ccze_addstr(CCZE_COLOR_KEYWORD, "conffile");
  ccze_space();
  ccze_addstr(CCZE_COLOR_FILE, filename);
  ccze_space();
  ccze_addstr(CCZE_COLOR_KEYWORD, decision);
  ccze_newline();

  free(date);
  free(filename);
  free(decision);

  return NULL;
}


static void
ccze_dpkg_setup (void)
{
  const char *error;
  int errptr;

  /* YYYY-MM-DD HH:MM:SS status <state> <pkg> <installed-version> */
  reg_dpkg_status = pcre_compile(
      "^([-\\d]{10}\\s[:\\d]{8})\\sstatus\\s(\\S+)\\s(\\S+)\\s(\\S+)$",
      0, &error, &errptr, NULL);

  /* YYYY-MM-DD HH:MM:SS <action> <pkg> <installed-version> <available-version> */
  reg_dpkg_action = pcre_compile(
      "^([-\\d]{10}\\s[:\\d]{8})\\s(install|upgrade|remove|purge)\\s(\\S+)\\s(\\S+)\\s(\\S+)$",
      0, &error, &errptr, NULL);

  /* YYYY-MM-DD HH:MM:SS conffile <filename> <decision> */
  reg_dpkg_conffile = pcre_compile(
      "^([-\\d]{10}\\s[:\\d]{8})\\sconffile\\s(\\S+)\\s(install|keep)$",
      0, &error, &errptr, NULL);

}

static void
ccze_dpkg_shutdown (void)
{
  free (reg_dpkg_status);
  free (reg_dpkg_action);
  free (reg_dpkg_conffile);
}

static int
ccze_dpkg_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];
  
  if ((match = pcre_exec (reg_dpkg_status, NULL, str, length,
                  0, 0, offsets, 99)) >= 0)
  {
      *rest = ccze_dpkg_status_process (str, offsets, match);
      return 1;
  }

  if ((match = pcre_exec (reg_dpkg_action, NULL, str, length,
                  0, 0, offsets, 99)) >= 0)
  {
      *rest = ccze_dpkg_action_process (str, offsets, match);
      return 1;
  }

  if ((match = pcre_exec (reg_dpkg_conffile, NULL, str, length,
                  0, 0, offsets, 99)) >= 0)
  {
      *rest = ccze_dpkg_conffile_process (str, offsets, match);
      return 1;
  }

  return 0;
}

CCZE_DEFINE_PLUGIN (dpkg, FULL, "Coloriser for dpkg logs.");
