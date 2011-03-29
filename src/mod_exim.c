/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_exim.c -- Exim log-coloriser module for CCZE
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

static void ccze_exim_setup (void);
static void ccze_exim_shutdown (void);
static int ccze_exim_handle (const char *str, size_t length, char **rest);

static pcre *reg_exim, *reg_exim_actiontype, *reg_exim_uniqn;
static pcre_extra *hints_exim;

static char *
ccze_exim_process (const char *str, int *offsets, int match)
{
  char *date, *msg=NULL, *action=NULL, *uniqn=NULL, *msgfull;
  int match2, offsets2[99];
  ccze_color_t color = CCZE_COLOR_UNKNOWN;
  
  pcre_get_substring (str, offsets, match, 1, (const char **)&date);
  pcre_get_substring (str, offsets, match, 2, (const char **)&msgfull);

  if ((match2 = pcre_exec (reg_exim_actiontype, NULL, msgfull,
			   strlen (msgfull), 0, 0, offsets2, 99)) >= 0)
    {
      pcre_get_substring (msgfull, offsets2, match2, 1,
			  (const char **)&uniqn);
      pcre_get_substring (msgfull, offsets2, match2, 2,
			  (const char **)&action);
      pcre_get_substring (msgfull, offsets2, match2, 3,
			  (const char **)&msg);
      if (action[0] == '<')
	color = CCZE_COLOR_INCOMING;
      else if (action[1] == '>')
	color = CCZE_COLOR_OUTGOING;
      else if (action[0] == '=' || action[0] == '*')
	color = CCZE_COLOR_ERROR;
    }
  else if ((match2 = pcre_exec (reg_exim_uniqn, NULL, msgfull,
				strlen (msgfull), 0, 0, offsets2, 99)) >= 0)
    {
      pcre_get_substring (msgfull, offsets2, match2, 1,
			  (const char **)&uniqn);
      pcre_get_substring (msgfull, offsets2, match2, 2,
			  (const char **)&msg);
    }
  else
    msg = strdup (msgfull);
  
  ccze_print_date (date);
  ccze_space ();

  if (uniqn && uniqn[0])
    {
      ccze_addstr (CCZE_COLOR_UNIQN, uniqn);
      ccze_space();
    }
  
  if (action && action[0])
    {
      ccze_addstr (color, action);
      ccze_space();
    }

  return msg;
}

static void
ccze_exim_setup (void)
{
  const char *error;
  int errptr;

  reg_exim = pcre_compile
    ("^(\\d{4}-\\d{2}-\\d{2}\\s\\d{2}:\\d{2}:\\d{2})\\s(.*)$", 0,
     &error, &errptr, NULL);
  hints_exim = pcre_study (reg_exim, 0, &error);

  reg_exim_actiontype = pcre_compile
    ("^(\\S{16})\\s([<=\\*][=>\\*])\\s(\\S+.*)$", 0, &error,
     &errptr, NULL);
  reg_exim_uniqn = pcre_compile ("^(\\S{16})\\s(.*)$", 0, &error,
				 &errptr, NULL);
}

static void
ccze_exim_shutdown (void)
{
  free (reg_exim);
  free (hints_exim);
  free (reg_exim_actiontype);
  free (reg_exim_uniqn);
}

static int
ccze_exim_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];
  
  if ((match = pcre_exec (reg_exim, hints_exim, str, length,
			  0, 0, offsets, 99)) >= 0)
    {
      *rest = ccze_exim_process (str, offsets, match);
      return 1;
    }
  
  return 0;
}

CCZE_DEFINE_PLUGIN (exim, FULL, "Coloriser for exim logs.");
