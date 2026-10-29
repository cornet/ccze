/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_distcc.c -- distcc.log colorizers for CCZE
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

static void ccze_distcc_setup (void);
static void ccze_distcc_shutdown (void);
static int ccze_distcc_handle (const char *str, size_t length, char **rest);

static ccze_regex_t *reg_distcc;


static char *
ccze_distcc_process (const char *str, int *offsets, int match)
{
  char *pid, *func, *rest;
    
  ccze_regex_substring (str, offsets, match, 1, (const char **)&pid);
  ccze_regex_substring (str, offsets, match, 2, (const char **)&func);
  ccze_regex_substring (str, offsets, match, 3, (const char **)&rest);
  
  ccze_addstr (CCZE_COLOR_PROC, "distccd");
  ccze_addstr (CCZE_COLOR_PIDB, "[");
  ccze_addstr (CCZE_COLOR_PID, pid);
  ccze_addstr (CCZE_COLOR_PIDB, "]");
  ccze_space ();

  if (func && func[0] != '\0')
    {
      ccze_addstr (CCZE_COLOR_KEYWORD, func);
      ccze_space ();
    }

  free (pid);
  free (func);
  
  return rest;
}

static void
ccze_distcc_setup (void)
{


  reg_distcc = ccze_regex_compile ("^distccd\\[(\\d+)\\] (\\([^\\)]+\\))? ?(.*)");
}

static void
ccze_distcc_shutdown (void)
{
  ccze_regex_free (reg_distcc);
}

static int
ccze_distcc_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];
  
  if ((match = ccze_regex_match (reg_distcc, str, length, offsets, 99)) >= 0)
    {
      *rest = ccze_distcc_process (str, offsets, match);
      return 1;
    }
  
  return 0;
}

CCZE_DEFINE_PLUGIN (distcc, FULL, "Coloriser for distcc(1) logs.");
