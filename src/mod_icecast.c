/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_icecast.c -- icecast/{usage,icecast}.log colorizers for CCZE
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

static void ccze_icecast_setup (void);
static void ccze_icecast_shutdown (void);
static int ccze_icecast_handle (const char *str, size_t length, char **rest);

static pcre *reg_icecast, *reg_icecast_usage;
static pcre_extra *hints_icecast, *hints_icecast_usage;

static char *
ccze_icecast_process (const char *str, int *offsets, int match)
{
  char *date = NULL, *admin = NULL, *threadno = NULL, *thread = NULL;
  char *rest = NULL;
  
  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
  pcre_get_substring (str, offsets, match, 2, (const char **)&admin);
  pcre_get_substring (str, offsets, match, 4, (const char **)&threadno);
  pcre_get_substring (str, offsets, match, 5, (const char **)&thread);
  pcre_get_substring (str, offsets, match, 6, (const char **)&rest);
  
  ccze_addstr (CCZE_COLOR_DATE, date);
  ccze_space ();

  if (admin && admin[0] != '\0')
    {
      ccze_addstr (CCZE_COLOR_KEYWORD, admin);
      ccze_space ();
      ccze_addstr (CCZE_COLOR_PIDB, "[");
      ccze_addstr (CCZE_COLOR_HOST, thread);
      ccze_addstr (CCZE_COLOR_PIDB, "]");
    }
  else
    {
      ccze_addstr (CCZE_COLOR_PIDB, "[");
      ccze_addstr (CCZE_COLOR_NUMBERS, threadno);
      ccze_addstr (CCZE_COLOR_DEFAULT, ":");
      ccze_addstr (CCZE_COLOR_KEYWORD, thread);
      ccze_addstr (CCZE_COLOR_PIDB, "]");
    }
  ccze_space ();

  free (date);
  free (admin);
  free (threadno);
  free (thread);
  
  return rest;
}

static char *
ccze_icecast_usage_process (const char *str, int *offsets, int match)
{
  char *date, *threadno, *thread, *date2, *bw, *src;
  char *unit, *clients, *admins;
    
  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
  pcre_get_substring (str, offsets, match, 3, (const char **)&threadno);
  pcre_get_substring (str, offsets, match, 4, (const char **)&thread);
  pcre_get_substring (str, offsets, match, 5, (const char **)&date2);
  pcre_get_substring (str, offsets, match, 6, (const char **)&bw);
  pcre_get_substring (str, offsets, match, 7, (const char **)&unit);
  pcre_get_substring (str, offsets, match, 8, (const char **)&src);
  pcre_get_substring (str, offsets, match, 9, (const char **)&clients);
  pcre_get_substring (str, offsets, match, 10, (const char **)&admins);
  
  
  ccze_addstr (CCZE_COLOR_DATE, date);
  ccze_space ();

  ccze_addstr (CCZE_COLOR_PIDB, "[");
  ccze_addstr (CCZE_COLOR_NUMBERS, threadno);
  ccze_addstr (CCZE_COLOR_DEFAULT, ":");
  ccze_addstr (CCZE_COLOR_KEYWORD, thread);
  ccze_addstr (CCZE_COLOR_PIDB, "]");
  ccze_space ();

  ccze_addstr (CCZE_COLOR_DATE, date2);
  ccze_space ();

  ccze_addstr (CCZE_COLOR_KEYWORD, "Bandwidth:");
  ccze_addstr (CCZE_COLOR_NUMBERS, bw);
  ccze_addstr (CCZE_COLOR_DEFAULT, unit);
  ccze_space ();

  ccze_addstr (CCZE_COLOR_KEYWORD, "Sources:");
  ccze_addstr (CCZE_COLOR_NUMBERS, src);
  ccze_space ();

  ccze_addstr (CCZE_COLOR_KEYWORD, "Clients:");
  ccze_addstr (CCZE_COLOR_NUMBERS, clients);
  ccze_space ();

  ccze_addstr (CCZE_COLOR_KEYWORD, "Admins:");
  ccze_addstr (CCZE_COLOR_NUMBERS, admins);

  ccze_newline ();
  
  free (date);
  free (threadno);
  free (thread);
  free (date2);
  free (bw);
  free (unit);
  free (src);
  free (clients);
  free (admins);
  
  return NULL;
}

static void
ccze_icecast_setup (void)
{
  const char *error;
  int errptr;

  reg_icecast = pcre_compile ("^(\\[\\d+/.../\\d+:\\d+:\\d+:\\d+\\]) "
			      "(Admin)? *(\\[(\\d+)?:?([^\\]]*)\\]) (.*)$",
			  0, &error, &errptr, NULL);
  hints_icecast = pcre_study (reg_icecast, 0, &error);

  reg_icecast_usage = pcre_compile ("^(\\[\\d+/.../\\d+:\\d+:\\d+:\\d+\\]) "
				    "(\\[(\\d+):([^\\]]*)\\]) "
				    "(\\[\\d+/.../\\d+:\\d+:\\d+:\\d+\\]) "
				    "Bandwidth:([\\d\\.]+)([^ ]*) "
				    "Sources:(\\d+) "
				    "Clients:(\\d+) Admins:(\\d+)",
				    0, &error, &errptr, NULL);
  hints_icecast_usage = pcre_study (reg_icecast_usage, 0, &error);
}

static void
ccze_icecast_shutdown (void)
{
  free (reg_icecast);
  free (hints_icecast);
  free (reg_icecast_usage);
  free (hints_icecast_usage);
}

static int
ccze_icecast_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];

  if ((match = pcre_exec (reg_icecast_usage, hints_icecast_usage,
			  str, length, 0, 0, offsets, 99)) >= 0)
    {
      *rest = ccze_icecast_usage_process (str, offsets, match);
      return 1;
    }
  
  if ((match = pcre_exec (reg_icecast, hints_icecast, str, length,
			  0, 0, offsets, 99)) >= 0)
    {
      *rest = ccze_icecast_process (str, offsets, match);
      return 1;
    }
  
  return 0;
}

CCZE_DEFINE_PLUGIN (icecast, FULL, "Coloriser for Icecast(8) logs.");
