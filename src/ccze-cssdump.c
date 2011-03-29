/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-cssdump.c -- Dump internal color table into css format
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
#ifdef HAVE_ARGP_H
# include <argp.h>
#endif

#include "ccze-compat.h"
#include "ccze-private.h"
#include "ccze-color.c"

const char *argp_program_name = "ccze-cssdump";
const char *argp_program_version = "ccze-cssdump (ccze 0.1." PATCHLEVEL ")";
const char *argp_program_bug_address = "<algernon@bonehunter.rulez.org>";
static struct argp_option options[] = {
  {NULL, 0, NULL, 0, "", 1},
  {"rcfile", 'F', "FILE", 0, "Read configuration from FILE", 1},
  {"load", 'l', NULL, 0, "Load default configuration files", 1},
  {NULL, 0, NULL, 0,  NULL, 0}
};

ccze_config_t ccze_config = {
  .scroll = 1,
  .convdate = 0,
  .remfac = 0,
  .wcol = 1,
  .slookup = 1,
  .rcfile = NULL,
  .cssfile = NULL,
  .transparent = 1,
  .pluginlist_len = 0,
  .pluginlist_alloc = 10,
  .color_argv_len = 0,
  .color_argv_alloc = 10,
  .mode = CCZE_MODE_HTML
};

static error_t parse_opt (int key, char *arg, struct argp_state *state);
static struct argp argp =
  {options, parse_opt, 0, "ccze -- cheer up 'yer logs.", NULL, NULL, NULL};

static int ccze_loaddefs = 0;

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  switch (key)
    {
    case 'F':
      ccze_config.rcfile = arg;
      break;
    case 'l':
      ccze_loaddefs = 1;
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

int
main (int argc, char *argv[])
{
  argp_parse (&argp, argc, argv, 0, 0, NULL);
  
  ccze_color_init ();

  if (ccze_config.rcfile)
    ccze_color_load (ccze_config.rcfile);
  else if (ccze_loaddefs)
    {
      char *home, *homerc;
      
      ccze_color_load (SYSCONFDIR "/colorizerc");
      ccze_color_load (SYSCONFDIR "/cczerc");
      home = getenv ("HOME");
      if (home)
	{
	  asprintf (&homerc, "%s/.colorizerc", home);
	  ccze_color_load (homerc);
	  free (homerc);
	  asprintf (&homerc, "%s/.cczerc", home);
	  ccze_color_load (homerc);
	  free (homerc);
	}
    }

  ccze_colors_to_css ();
  return 0;
}
