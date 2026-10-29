/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_httpd.c -- httpd-related colorizers for CCZE
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
#include <string.h>

static void ccze_httpd_setup (void);
static void ccze_httpd_shutdown (void);
static int ccze_httpd_handle (const char *str, size_t length, char **rest);

static ccze_regex_t *reg_httpd_access, *reg_httpd_error;


static ccze_color_t
_ccze_httpd_error (const char *level)
{
  if (strstr (level, "debug") || strstr (level, "info") ||
      strstr (level, "notice"))
    return CCZE_COLOR_DEBUG;
  if (strstr (level, "warn"))
    return CCZE_COLOR_WARNING;
  if (strstr (level, "error") || strstr (level, "crit") ||
      strstr (level, "alert") || strstr (level, "emerg"))
    return CCZE_COLOR_ERROR;
  return CCZE_COLOR_UNKNOWN;
}

static char *
ccze_httpd_access_log_process (const char *str, int *offsets, int match)
{
  char *host, *vhost, *user, *date, *full_action, *method, *http_code;
  char *gsize, *other;

  ccze_regex_substring (str, offsets, match, 1, (const char **)&vhost);
  ccze_regex_substring (str, offsets, match, 2, (const char **)&host);
  ccze_regex_substring (str, offsets, match, 3, (const char **)&user);
  ccze_regex_substring (str, offsets, match, 4, (const char **)&date);
  ccze_regex_substring (str, offsets, match, 5, (const char **)&full_action);
  ccze_regex_substring (str, offsets, match, 6, (const char **)&method);
  ccze_regex_substring (str, offsets, match, 7, (const char **)&http_code);
  ccze_regex_substring (str, offsets, match, 8, (const char **)&gsize);
  ccze_regex_substring (str, offsets, match, 9, (const char **)&other);

  ccze_addstr (CCZE_COLOR_HOST, vhost);
  ccze_space();
  ccze_addstr (CCZE_COLOR_HOST, host);
  if (host[0])
    ccze_space ();
  ccze_addstr (CCZE_COLOR_DEFAULT, "-");
  ccze_space ();

  ccze_addstr (CCZE_COLOR_USER, user);
  ccze_space ();

  ccze_addstr (CCZE_COLOR_DATE, date);
  ccze_space ();
  
  ccze_addstr (ccze_http_action (method), full_action);
  ccze_space ();

  ccze_addstr (CCZE_COLOR_HTTPCODES, http_code);
  ccze_space ();

  ccze_addstr (CCZE_COLOR_GETSIZE, gsize);
  ccze_space ();

  ccze_addstr (CCZE_COLOR_DEFAULT, other);
  ccze_newline ();
  
  free (host);
  free (user);
  free (date);
  free (method);
  free (full_action);
  free (http_code);
  free (gsize);

  return NULL;
}

static char *
ccze_httpd_error_log_process (const char *str, int *offsets, int match)
{
  char *date, *level, *msg;
  ccze_color_t lcol;
  
  ccze_regex_substring (str, offsets, match, 1, (const char **)&date);
  ccze_regex_substring (str, offsets, match, 2, (const char **)&level);
  ccze_regex_substring (str, offsets, match, 3, (const char **)&msg);

  ccze_addstr (CCZE_COLOR_DATE, date);
  ccze_space ();

  lcol = _ccze_httpd_error (level);
  ccze_addstr (lcol, level);
  ccze_space ();

  ccze_addstr (lcol, msg);

  ccze_newline ();

  free (date);
  free (level);
  free (msg);

  return NULL;
}

static void
ccze_httpd_setup (void)
{


  reg_httpd_access = ccze_regex_compile
    ("^(\\S*)\\s(\\S*)?\\s?-\\s(\\S+)\\s(\\[\\d{1,2}\\/\\S*"
     "\\/\\d{4}:\\d{2}:\\d{2}:\\d{2}.{0,6}[^\\]]*\\])\\s"
     "(\"([^ \"]+)\\s*[^\"]*\")\\s(\\d{3})\\s(\\d+|-)\\s*(.*)$");


  reg_httpd_error = ccze_regex_compile
    ("^(\\[\\w{3}\\s\\w{3}\\s{1,2}\\d{1,2}\\s\\d{2}:\\d{2}:\\d{2}\\s"
     "\\d{4}\\])\\s(\\[\\w*\\])\\s(.*)$");
}

static void
ccze_httpd_shutdown (void)
{
  ccze_regex_free (reg_httpd_access);
  ccze_regex_free (reg_httpd_error);
}

static int
ccze_httpd_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];

  if ((match = ccze_regex_match (reg_httpd_access, str, length, offsets, 99)) >= 0)
    {
      *rest = ccze_httpd_access_log_process (str, offsets, match);
      return 1;
    }
  if ((match = ccze_regex_match (reg_httpd_error, str, length, offsets, 99)) >= 0)
    {
      *rest = ccze_httpd_error_log_process (str, offsets, match);
      return 1;
    }

  return 0;
}

CCZE_DEFINE_PLUGIN (httpd, FULL,
		    "Coloriser for generic HTTPD access and error logs.");
