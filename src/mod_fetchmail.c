/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_fetchmail.c -- fetchmail-related colorizers for CCZE
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

static void ccze_fetchmail_setup (void);
static void ccze_fetchmail_shutdown (void);
static int ccze_fetchmail_handle (const char *str, size_t length, char **rest);

static pcre *reg_fetchmail;

static char *
ccze_fetchmail_process (const char *str, int *offsets, int match)
{
  char *start, *addy, *current, *full, *rest;
  
  pcre_get_substring (str, offsets, match, 1, (const char **)&start);
  pcre_get_substring (str, offsets, match, 2, (const char **)&addy);
  pcre_get_substring (str, offsets, match, 3, (const char **)&current);
  pcre_get_substring (str, offsets, match, 4, (const char **)&full);
  pcre_get_substring (str, offsets, match, 5, (const char **)&rest);

  ccze_addstr (CCZE_COLOR_DEFAULT, start);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_EMAIL, addy);
  ccze_addstr (CCZE_COLOR_DEFAULT, ":");
  ccze_addstr (CCZE_COLOR_NUMBERS, current);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_DEFAULT, "of");
  ccze_space ();
  ccze_addstr (CCZE_COLOR_NUMBERS, full);
  ccze_space ();
  
  free (start);
  free (addy);
  free (current);
  free (full);
  
  return rest;
}

static void
ccze_fetchmail_setup (void)
{
  const char *error;
  int errptr;

  reg_fetchmail = pcre_compile
    ("(reading message) ([^@]*@[^:]*):([0-9]*) of ([0-9]*) (.*)", 0,
     &error, &errptr, NULL);
}

static void
ccze_fetchmail_shutdown (void)
{
  free (reg_fetchmail);
}

static int
ccze_fetchmail_handle (const char *str, size_t length, char **rest)
{
  int offsets[99], match;
  
  if ((match = pcre_exec (reg_fetchmail, NULL, str, length, 0, 0, offsets, 99)) >= 0)
    {
      if (rest)
	*rest = ccze_fetchmail_process (str, offsets, match);
      else
	ccze_fetchmail_process (str, offsets, match);
      
      return 1;
    }
  
  return 0;
}

CCZE_DEFINE_PLUGIN (fetchmail, PARTIAL,
		    "Coloriser for fetchmail(1) sub-logs.");
