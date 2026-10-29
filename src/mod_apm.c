/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_apm.c -- apm colorizers for CCZE
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

static void ccze_apm_setup (void);
static void ccze_apm_shutdown (void);
static int ccze_apm_handle (const char *str, size_t length, char **rest);

static ccze_regex_t *reg_apm;

static char *
ccze_apm_process (const char *str, int *offsets, int match)
{
  char *battery, *charge, *rate, *stuff1, *elapsed, *remain;
  char *stuff2;
  
  ccze_regex_substring (str, offsets, match, 1, (const char **)&battery);
  ccze_regex_substring (str, offsets, match, 2, (const char **)&charge);
  ccze_regex_substring (str, offsets, match, 4, (const char **)&rate);
  ccze_regex_substring (str, offsets, match, 5, (const char **)&stuff1);
  ccze_regex_substring (str, offsets, match, 6, (const char **)&elapsed);
  ccze_regex_substring (str, offsets, match, 7, (const char **)&remain);
  ccze_regex_substring (str, offsets, match, 8, (const char **)&stuff2);
        
  ccze_addstr (CCZE_COLOR_DEFAULT, "Battery:");
  ccze_space ();
  ccze_addstr (CCZE_COLOR_PERCENTAGE, battery);
  ccze_addstr (CCZE_COLOR_DEFAULT, "%,");
  ccze_space ();
  ccze_addstr (CCZE_COLOR_SYSTEMWORD, charge);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_DEFAULT, "(");
  ccze_addstr (CCZE_COLOR_PERCENTAGE, rate);
  ccze_addstr (CCZE_COLOR_DEFAULT, "%");
  ccze_space ();
  ccze_addstr (CCZE_COLOR_DEFAULT, stuff1);
  ccze_space ();
  ccze_addstr (CCZE_COLOR_DATE, elapsed);
  ccze_addstr (CCZE_COLOR_DEFAULT, "),");
  ccze_space ();
  ccze_addstr (CCZE_COLOR_DATE, remain);
  ccze_space ();

  free (battery);
  free (charge);
  free (rate);
  free (stuff1);
  free (elapsed);
  free (remain);
  
  return stuff2;
}

static void
ccze_apm_setup (void)
{


  reg_apm = ccze_regex_compile
    ("Battery: (-?\\d*)%, ((.*)charging) \\((-?\\d*)% ([^ ]*) "
     "(\\d*:\\d*:\\d*)\\), (\\d*:\\d*:\\d*) (.*)");
}

static void
ccze_apm_shutdown (void)
{
  ccze_regex_free (reg_apm);
}

static int
ccze_apm_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];
  
  if ((match = ccze_regex_match (reg_apm, str, length, offsets, 99)) >= 0)
    {
      if (rest)
	*rest = ccze_apm_process (str, offsets, match);
      else
	ccze_apm_process (str, offsets, match);
      
      return 1;
    }
    
  return 0;
}

CCZE_DEFINE_PLUGIN (apm, PARTIAL, "Coloriser for APM sub-logs.");
