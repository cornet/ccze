/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-dump.c -- Dump internal color table
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

#define CCZE_DUMP 1

#include <ccze.h>
#ifdef HAVE_ARGP_H
# include <argp.h>
#endif

#include "ccze-color.c"
#include "ccze-compat.h"

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
  .mode = CCZE_MODE_CURSES
};

const char *argp_program_name = "ccze-dump";
const char *argp_program_version = "ccze-dump (ccze 0.1." PATCHLEVEL ")";
const char *argp_program_bug_address = "<algernon@bonehunter.rulez.org>";
static struct argp_option options[] = {
  {NULL, 0, NULL, 0, "", 1},
  {"rcfile", 'F', "FILE", 0, "Read configuration from FILE", 1},
  {"load", 'l', NULL, 0, "Load default configuration files", 1},
  {NULL, 0, NULL, 0,  NULL, 0}
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

static char *
ccze_dump_color_get_attrib (int color)
{
  char *str = (char *)ccze_calloc (100, sizeof (char));
  
  if (color & A_BOLD)
    strcat (str, "bold ");
  if (color & A_UNDERLINE)
    strcat (str, "underline ");
  if (color & A_REVERSE)
    strcat (str, "reverse ");
  if (color & A_BLINK)
    strcat (str, "blink ");
  
  return str;
}

static char *
ccze_dump_color_to_name (int color)
{
  int my_color = ccze_color_strip_attrib (color);

  if (my_color < COLOR_PAIR (8))
    return ccze_color_to_name_simple (my_color);
  else
    {
      int i,j;
      char *str, *cj, *ci;

      j = (my_color >> 8) % 8;
      i = (my_color >> 8) / 8;
      cj = ccze_color_to_name_simple (COLOR_PAIR (j));
      ci = ccze_color_to_name_simple (COLOR_PAIR (i));
      asprintf (&str, "%s on_%s", cj, ci);
      return str;
    }
}

static char *
ccze_dump_color_comment (int cidx)
{
  return ccze_color_keyword_map[cidx].comment;
}

static int
ccze_dump_color_hidden (int cidx)
{
  return !ccze_color_keyword_map[cidx].settable;
}

static int
ccze_dump_color_to_idx (ccze_color_t color)
{
  size_t cidx;
  
  for (cidx = 0; cidx < sizeof (ccze_color_keyword_map); cidx++)
    if (ccze_color_keyword_map[cidx].idx == color)
      return cidx;
  return 0;
}

int
main (int argc, char *argv[])
{
  ccze_color_t cidx;
  char line[256];
  int color;
  size_t llen;
  
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

  printf ("# Configuration file for ccze\n#\n");
  printf ("# Available 'pre' attributes: bold, underline, underscore, "
	  "blink, reverse\n");
  printf ("# Available colors:  black, red, green, yellow, blue, magenta, "
	  "cyan, white\n");
  printf ("# Available bgcolors: on_black, on_red, on_green, on_yellow, "
	  "on_blue, on_magenta, on_cyan, on_white\n#\n");
  printf ("# You can also use item names in color definition, like:\n#\n");
  printf ("# default   blue\n# date      'default'\n#\n");
  printf ("# Here you defined default color to blue, and date color to "
	  "default value's color, so\n");
  printf ("# your date color is blue. (You can only use predefined item "
	  "names!)\n\n");
  printf ("# item          color                   # comment (what is "
	  "color, or why it's that ;)\n\n");

  /* Dump colors */
  for (cidx = CCZE_COLOR_DATE; cidx < CCZE_COLOR_LAST; cidx++)
    {
      if (ccze_dump_color_hidden (cidx))
	continue;
      
      color = ccze_color (cidx);
      
      strcpy (line, ccze_color_lookup_name (cidx));
      llen = strlen (line);
      memset (&line[llen], ' ', 16 - llen);
      line[16]='\0';
      strcat (line, ccze_dump_color_get_attrib (color));
      strcat (line, ccze_dump_color_to_name (color));
      llen = strlen (line);
      memset (&line[llen], ' ', 42 - llen);
      line[40]='#';
      line[42]='\0';
      strcat (line, ccze_dump_color_comment (ccze_dump_color_to_idx (cidx)));
      
      printf ("%s\n", line);      
    }

  /* CSS codes */
  printf ("\n# CSS codes for the HTML output\n");
  for (color = 0; color < 8; color++)
    {
      strcpy (line, "css");
      strcat (line, ccze_colorname_map[color].name);
      llen = strlen (line);
      memset (&line[llen], ' ', 16 - llen);
      line[16]='\0';
      strcat (line, ccze_csscolor_normal_map[color]);
      printf ("%s\n", line);

      strcpy (line, "cssbold");
      strcat (line, ccze_colorname_map[color].name);
      llen = strlen (line);
      memset (&line[llen], ' ', 16 - llen);
      line[16]='\0';
      strcat (line, ccze_csscolor_bold_map[color]);
      printf ("%s\n", line);
    }

  strcpy (line, "cssbody");
  llen = strlen (line);
  memset (&line[llen], ' ', 16 - llen);
  line[16]='\0';
  strcat (line, ccze_cssbody_color ());
  printf ("%s\n", line);
  
  return 0;
}
