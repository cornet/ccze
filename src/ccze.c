/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze.c -- CCZE itself
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

#include "system.h"

#ifdef HAVE_ARGP_H
# include <argp.h>
#endif
#include <ccze.h>
#include <dlfcn.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ccze-private.h"
#include "ccze-compat.h"

#define BLACK COLOR_PAIR (0)
#define RED COLOR_PAIR (1)
#define GREEN COLOR_PAIR (2)
#define YELLOW COLOR_PAIR (3)
#define BLUE COLOR_PAIR (4)
#define CYAN COLOR_PAIR (5)
#define MAGENTA COLOR_PAIR (6)
#define WHITE COLOR_PAIR (7)

#define ESC 0x1b

/* ccze somehow swaped cyan and magenta */
static int ccze_raw_ansi_color[] = {30, 31, 32, 33, 34, 36, 35, 37};

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

static short colors[] = {COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW,
			 COLOR_BLUE, COLOR_CYAN, COLOR_MAGENTA, COLOR_WHITE};
static volatile sig_atomic_t sighup_received = 0;

#ifndef HAVE_ARGP_PARSE
const char *argp_program_name = "ccze";
#endif
const char *argp_program_version = "ccze 0.2." PATCHLEVEL;
const char *argp_program_bug_address = "<algernon@bonehunter.rulez.org>";
static struct argp_option options[] = {
  {"rcfile", 'F', "FILE", 0, "Read configuration from FILE", 1},
  {"html", 'h', NULL, 0, "Generate HTML output", 1},
  {"options", 'o', "OPTIONS...", 0, "Toggle some options\n"
   "(such as scroll, wordcolor and lookups, transparent, or cssfile)", 1},
  {"convert-date", 'C', NULL, 0, "Convert UNIX timestamps to readable format", 1},
  {"plugin", 'p', "PLUGIN", 0, "Load PLUGIN", 1},
  {"remove-facility", 'r', NULL, 0,
   "remove syslog-ng's facility from start of the lines", 1},
  {"color", 'c', "KEY=COLOR,...", 0, "Set the color of KEY to COLOR", 1},
  {"argument", 'a', "PLUGIN=ARGS...", 0, "Add ARGUMENTS to PLUGIN", 1},
  {"debug", 'd', NULL, OPTION_HIDDEN, "Turn on debugging.", 1},
  {"raw-ansi", 'A', NULL, 0, "Generate raw ANSI output", 1},
  {"list-plugins", 'l', NULL, 0, "List available plugins", 1},
  {"mode", 'm', "MODE", 0, "Change the output mode\n"
   "(Available modes are curses, ansi and html.)", 1},
  {NULL, 0, NULL, 0,  NULL, 0}
};
static error_t parse_opt (int key, char *arg, struct argp_state *state);
static struct argp argp =
  {options, parse_opt, 0, "ccze -- cheer up 'yer logs.", NULL, NULL, NULL};

enum
{
  CCZE_O_SUBOPT_SCROLL = 0,
  CCZE_O_SUBOPT_NOSCROLL,
  CCZE_O_SUBOPT_WORDCOLOR,
  CCZE_O_SUBOPT_NOWORDCOLOR,
  CCZE_O_SUBOPT_LOOKUPS,
  CCZE_O_SUBOPT_NOLOOKUPS,
  CCZE_O_SUBOPT_CSSFILE,
  CCZE_O_SUBOPT_NOCSSFILE,
  CCZE_O_SUBOPT_TRANSPARENT,
  CCZE_O_SUBOPT_NOTRANSPARENT,
  CCZE_O_SUBOPT_END
};

static char *o_subopts[] = {
  [CCZE_O_SUBOPT_SCROLL] = "scroll",
  [CCZE_O_SUBOPT_NOSCROLL] = "noscroll",
  [CCZE_O_SUBOPT_WORDCOLOR] = "wordcolor",
  [CCZE_O_SUBOPT_NOWORDCOLOR] = "nowordcolor",
  [CCZE_O_SUBOPT_LOOKUPS] = "lookups",
  [CCZE_O_SUBOPT_NOLOOKUPS] = "nolookups",
  [CCZE_O_SUBOPT_CSSFILE] = "cssfile",
  [CCZE_O_SUBOPT_NOCSSFILE] = "nocssfile",
  [CCZE_O_SUBOPT_TRANSPARENT] = "transparent",
  [CCZE_O_SUBOPT_NOTRANSPARENT] = "notransparent",
  [CCZE_O_SUBOPT_END] = NULL
};

enum
{
  CCZE_M_SUBOPT_CURSES,
  CCZE_M_SUBOPT_ANSI,
  CCZE_M_SUBOPT_HTML,
  CCZE_M_SUBOPT_DEBUG,
  CCZE_M_SUBOPT_END
};

static char *m_subopts[] = {
  [CCZE_M_SUBOPT_CURSES] = "curses",
  [CCZE_M_SUBOPT_ANSI] = "ansi",
  [CCZE_M_SUBOPT_HTML] = "html",
  [CCZE_M_SUBOPT_DEBUG] = "debug",
  [CCZE_M_SUBOPT_END] = NULL
};

static char *empty_subopts[] = { NULL };

static char *_strbrk_string;
static size_t _strbrk_string_len;

char *
ccze_strbrk (char *str, char delim)
{
  char *found;
      
  if (str)
    {
      _strbrk_string = str;
      _strbrk_string_len = strlen (str);
      found = str;
    }
  else
    found = _strbrk_string + 1;
  
  if (!_strbrk_string_len)
    return NULL;
  while (_strbrk_string_len >= 1 &&
	 *_strbrk_string != delim)
    {
      _strbrk_string++;
      _strbrk_string_len--;
    }
  if (_strbrk_string_len > 0)
    *_strbrk_string = '\0';
  return found;
}

char *
xstrdup (const char *str)
{
  if (!str)
    return NULL;
  else
    return strdup (str);
}

static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  char *subopts, *value, *plugin;
  
  switch (key)
    {
    case 'c':
      subopts = arg;
      while (*subopts != '\0')
	{
	  ccze_getsubopt (&subopts, empty_subopts, &value);
	  ccze_config.color_argv[ccze_config.color_argv_len++] =
	    strdup (value);
	  if (ccze_config.color_argv_len >= ccze_config.color_argv_alloc)
	    {
	      ccze_config.color_argv_alloc *= 2;
	      ccze_config.color_argv =
		(char **)ccze_realloc (ccze_config.color_argv,
				       ccze_config.color_argv_alloc *
				       sizeof (char *));
	    }
	}
      break;
    case 'a':
      plugin = strtok (arg, "=");
      value = strtok (NULL, "\n");
      ccze_plugin_argv_set (plugin, value);
      break;
    case 'p':
      subopts = arg;
      while (*subopts != '\0')
	{
	  ccze_getsubopt (&subopts, empty_subopts, &value);
	  ccze_config.pluginlist[ccze_config.pluginlist_len++] =
	    strdup (value);
	  if (ccze_config.pluginlist_len >= ccze_config.pluginlist_alloc)
	    {
	      ccze_config.pluginlist_alloc *= 2;
	      ccze_config.pluginlist =
		(char **)ccze_realloc (ccze_config.pluginlist,
				       ccze_config.pluginlist_alloc *
				       sizeof (char *));
	    }
	}
      break;
    case 'h':
      ccze_config.mode = CCZE_MODE_HTML;
      break;
    case 'A':
      ccze_config.mode = CCZE_MODE_RAW_ANSI;
      break;
    case 'd':
      ccze_config.mode = CCZE_MODE_DEBUG;
      break;
    case 'l':
      ccze_config.mode = CCZE_MODE_PLUGIN_LIST;
      break;
    case 'F':
      ccze_config.rcfile = strdup (arg);
      break;
    case 'r':
      ccze_config.remfac = 1;
      break;
    case 'm':
      subopts = arg;
      while (*subopts != '\0')
	{
	  switch (getsubopt (&subopts, m_subopts, &value))
	    {
	    case CCZE_M_SUBOPT_CURSES:
	      ccze_config.mode = CCZE_MODE_CURSES;
	      break;
	    case CCZE_M_SUBOPT_ANSI:
	      ccze_config.mode = CCZE_MODE_RAW_ANSI;
	      break;
	    case CCZE_M_SUBOPT_HTML:
	      ccze_config.mode = CCZE_MODE_HTML;
	      break;
	    case CCZE_M_SUBOPT_DEBUG:
	      ccze_config.mode = CCZE_MODE_DEBUG;
	      break;
	    default:
	      argp_error (state, "unrecognised mode: `%s'", value);
	      break;
	    }
	}
      break;
    case 'o':
      subopts = arg;
      while (*subopts != '\0')
	{
	  switch (getsubopt (&subopts, o_subopts, &value))
	    {
	    case CCZE_O_SUBOPT_SCROLL:
	      ccze_config.scroll = 1;
	      break;
	    case CCZE_O_SUBOPT_NOSCROLL:
	      ccze_config.scroll = 0;
	      break;
	    case CCZE_O_SUBOPT_WORDCOLOR:
	      ccze_config.wcol = 1;
	      break;
	    case CCZE_O_SUBOPT_NOWORDCOLOR:
	      ccze_config.wcol = 0;
	      break;
	    case CCZE_O_SUBOPT_LOOKUPS:
	      ccze_config.slookup = 1;
	      break;
	    case CCZE_O_SUBOPT_NOLOOKUPS:
	      ccze_config.slookup = 0;
	      break;
	    case CCZE_O_SUBOPT_CSSFILE:
	      if (value)
		ccze_config.cssfile = strdup (value);
	      break;
	    case CCZE_O_SUBOPT_NOCSSFILE:
	      ccze_config.cssfile = NULL;
	      break;
	    case CCZE_O_SUBOPT_TRANSPARENT:
	      ccze_config.transparent = 1;
	      break;
	    case CCZE_O_SUBOPT_NOTRANSPARENT:
	      ccze_config.transparent = 0;
	      break;
	    default:
	      argp_error (state, "unrecognised option: `%s'", value);
	      break;
	    }
	}
      break;
    case 'C':
      ccze_config.convdate = 1;
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

ccze_color_t
ccze_http_action (const char *method)
{
  if (!strcasecmp ("GET", method))
    return CCZE_COLOR_HTTP_GET;
  else if (!strcasecmp ("POST", method))
    return CCZE_COLOR_HTTP_POST;
  else if (!strcasecmp ("HEAD", method))
    return CCZE_COLOR_HTTP_HEAD;
  else if (!strcasecmp ("PUT", method))
    return CCZE_COLOR_HTTP_PUT;
  else if (!strcasecmp ("CONNECT", method))
    return CCZE_COLOR_HTTP_CONNECT;
  else if (!strcasecmp ("TRACE", method))
    return CCZE_COLOR_HTTP_TRACE;
  else
    return CCZE_COLOR_UNKNOWN;
}

void
ccze_print_date (const char *date)
{
  time_t ltime;
  char tmp[128];
  
  if (ccze_config.convdate)
    {
      ltime = atol (date);
      if (ltime < 0)
	{
	  ccze_addstr (CCZE_COLOR_DATE, date);
	  return;
	}
      strftime (tmp, sizeof (tmp) - 1, "%b %e %T", gmtime (&ltime));
      ccze_addstr (CCZE_COLOR_DATE, tmp);
    }
  else
    ccze_addstr (CCZE_COLOR_DATE, date);
}

void
ccze_newline (void)
{
  switch (ccze_config.mode)
    {
    case CCZE_MODE_HTML:
      printf ("<br>\n");
      break;
    case CCZE_MODE_DEBUG:
    case CCZE_MODE_RAW_ANSI:
      printf ("\n");
      break;
    case CCZE_MODE_CURSES:
      addstr ("\n");
      break;
    default:
      break;
    }
}

static char *
ccze_str_htmlencode (const char *src)
{
    char *buf, *dest;
    unsigned char c;

    dest = (char *)ccze_malloc (strlen (src) * 5 + 1);

    if (!dest)
      return NULL;

    buf = dest;
    while ((c = *src++))
      {
        switch (c)
	  {
	  case '>':
            *dest++ = '&';
            *dest++ = 'g';
            *dest++ = 't';
            *dest++ = ';';
            break;
	  case '<':
            *dest++ = '&';
            *dest++ = 'l';
            *dest++ = 't';
            *dest++ = ';';
            break;
	  case '&':
            *dest++ = '&';
            *dest++ = 'a';
            *dest++ = 'm';
            *dest++ = 'p';
            *dest++ = ';';
            break;
	  default:
            *dest++ = c;
	  }
      }
    *dest = '\0';
    return buf;
}

static void
ccze_addstr_internal (ccze_color_t col, const char *str, int enc)
{
  switch (ccze_config.mode)
    {
    case CCZE_MODE_HTML:
      if (str)
	{
	  char *d;

	  if (enc)
	    d = ccze_str_htmlencode (str);
	  else
	    d = strdup (str);
	  printf ("<font class=\"ccze_%s\">%s</font>",
		  ccze_color_lookup_name (col), d);
	  free (d);
	}
      break;
    case CCZE_MODE_RAW_ANSI:
      if (str)
	{
	  int c = ccze_color (col);

	  printf("%c[22m", ESC); /* default */

	  if (c & 0x1000) /* Bold */
	    {
	      printf("%c[1m", ESC);
	      c ^= 0x1000;
	    }
	  
	  if (c & 0x2000) /* Underline */
	    {
	      printf("%c[4m", ESC);
	      c ^= 0x2000;
	    }
	  
	  if (c & 0x4000) /* Reverse */
	    {
	      printf("%c[5m", ESC);
	      c ^= 0x4000;
	    }
	  
	  if (c & 0x8000) /* Blink */
	    {
	      printf("%c[7m", ESC);
	      c ^= 0x8000;
	    }

	  if (c >> 8 > 0 || !ccze_config.transparent)
	    printf("%c[%dm", ESC, ccze_raw_ansi_color[c >> 8] + 10);
	  printf ("%c[%dm%s%c[0m", ESC, ccze_raw_ansi_color[c & 0xf], str, ESC);
	}
      break;
    case CCZE_MODE_DEBUG:
      if (str)
	{
	  char *cn = ccze_color_lookup_name (col);
	  printf ("<%s>%s</%s>", cn, str, cn);
	}
      break;
    case CCZE_MODE_CURSES:
      attrset (ccze_color (col));
      addstr (str);
      break;
    default:
      break;
    }
}

void
ccze_addstr (ccze_color_t col, const char *str)
{
  ccze_addstr_internal (col, str, 1);
}

void
ccze_space (void)
{
  switch (ccze_config.mode)
    {
    case CCZE_MODE_HTML:
      ccze_addstr_internal (CCZE_COLOR_DEFAULT, "&nbsp;", 0);
      break;
    default:
      ccze_addstr (CCZE_COLOR_DEFAULT, " ");
      break;
    }
}

static void sigint_handler (int sig) __attribute__ ((noreturn));
static void
sigint_handler (int sig)
{
  switch (ccze_config.mode)
    {
    case CCZE_MODE_CURSES:
      endwin ();
      break;
    case CCZE_MODE_HTML:
      printf ("\n</body>\n</html>\n");
      break;
    case CCZE_MODE_RAW_ANSI:
      printf("%c[0m", ESC);
      break;
    default:
      break;
    }

  if (sig)
    {
      ccze_wordcolor_shutdown ();
      ccze_plugin_shutdown ();
    }
  
  exit (0);
}

static void
sigwinch_handler (int sig)
{
  endwin ();
  refresh ();
  signal (SIGWINCH, sigwinch_handler);
}

static void
sighup_handler (int sig)
{
  sighup_received = 1;
  signal (SIGHUP, sighup_handler);
}

static void
ccze_main (void)
{
  char *subject = NULL;
  size_t subjlen = 0;
  int i, j;
  char *homerc, *home;
  ccze_plugin_t **plugins;

  sighup_received = 0;
  ccze_color_init ();
  ccze_plugin_init ();

  if (ccze_config.rcfile)
    ccze_color_load (ccze_config.rcfile);
  else
    {
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

  while (ccze_config.color_argv_len > 0)
    {
      ccze_color_parse (ccze_config.color_argv[ccze_config.color_argv_len - 1]);
      free (ccze_config.color_argv[--ccze_config.color_argv_len]);
    }
  
  if (ccze_config.mode == CCZE_MODE_CURSES)
    {
      initscr ();
      signal (SIGWINCH, sigwinch_handler);
      nonl ();
      if (ccze_config.scroll)
	{
	  idlok (stdscr, TRUE);
	  scrollok (stdscr, TRUE);
	  leaveok (stdscr, FALSE);
	}
      
      start_color ();
      if (ccze_config.transparent)
	{
	  if (use_default_colors () == OK)
	    {
	      colors[0] = -1;
	    }
	}
      
      for (i = 0; i < 8; i++)
	for (j = 0; j < 8; j++)
	  init_pair (i*8 + j, colors[j], colors[i]);
    }
  else if (ccze_config.mode == CCZE_MODE_HTML)
    {
      printf
	("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//Transitional//EN\">\n"
	 "<html>\n<head>\n<meta name=\"generator\" content=\"%s\">\n",
	 argp_program_version);

      if (ccze_config.cssfile)
	printf ("<link rel=\"stylesheet\" href=\"%s\">\n",
		ccze_config.cssfile);
      else
	{
	  printf ("<style type=\"text/css\">\n"
		  "body { font: 10pt courier; white-space: nowrap }\n");

	  ccze_colors_to_css ();

	  printf ("</style>\n");
	}
      
      
      printf ("<title>Log colorisation generated by %s</title>\n"
	      "</head>\n<body bgcolor=\"%s\">\n\n",
	      argp_program_version, ccze_cssbody_color ());
    }
  
  signal (SIGINT, sigint_handler);
  signal (SIGHUP, sighup_handler);
  
  ccze_wordcolor_setup ();

  if (ccze_config.pluginlist_len == 0)
    ccze_plugin_load_all ();
  else
    {
      int pl = ccze_config.pluginlist_len;
      while (pl-- > 0)
	ccze_plugin_load (ccze_config.pluginlist[pl]);
    }
  ccze_plugin_load_all_builtins ();
  ccze_plugin_finalise ();
      
  i = 0;
  plugins = ccze_plugins ();
  if (!plugins[0])
    {
      endwin ();
      fprintf (stderr, "ccze: No plugins found. Exiting.\n");
      exit (1);
    }

  ccze_plugin_argv_finalise ();

  if (ccze_config.mode == CCZE_MODE_PLUGIN_LIST)
    {
      ccze_plugin_list_fancy ();
      sigint_handler (0);
    }
  
  ccze_plugin_setup ();
  
  while ((getline (&subject, &subjlen, stdin) != -1) && !sighup_received)
    {
      int handled = 0;
      int status = 0;
      char *rest = NULL, *rest2 = NULL;
      char *tmp = strchr (subject, '\n');
      unsigned int remfac_tmp;

      if (tmp)
	tmp[0] = '\0';

      if (ccze_config.remfac && (sscanf (subject, "<%u>", &remfac_tmp) > 0))
	{
	  tmp = strdup (strchr (subject, '>') + 1);
	  free (subject);
	  subject = tmp;
	}

      subjlen = strlen (subject);
      ccze_plugin_run (plugins, subject, subjlen, &rest,
		       CCZE_PLUGIN_TYPE_FULL, &handled, &status);
      
      if (rest)
	{
	  handled = 0;
	  ccze_plugin_run (plugins, rest, strlen (rest), &rest2,
			   CCZE_PLUGIN_TYPE_PARTIAL, &handled, &status);
	  if (handled == 0)
	    ccze_wordcolor_process (rest, ccze_config.wcol,
				    ccze_config.slookup);
	  else
	    ccze_wordcolor_process (rest2, ccze_config.wcol,
				    ccze_config.slookup);
	  ccze_newline ();
	  free (rest);
	  free (rest2);
	}

      if (status == 0)
	{
	  ccze_wordcolor_process (subject, ccze_config.wcol,
				  ccze_config.slookup);
	  ccze_newline ();
	}

      if (ccze_config.mode == CCZE_MODE_CURSES)
	refresh ();
    }

  if (ccze_config.mode == CCZE_MODE_CURSES)
    refresh ();
}

int
main (int argc, char **argv)
{
      
  ccze_config.pluginlist = (char **)ccze_calloc (ccze_config.pluginlist_alloc,
						 sizeof (char *));
  ccze_config.color_argv = (char **)ccze_calloc (ccze_config.color_argv_alloc,
						 sizeof (char *));
  ccze_plugin_argv_init ();
  argp_parse (&argp, argc, argv, 0, 0, NULL);

  do
    {
      ccze_main ();
      ccze_wordcolor_shutdown ();
      ccze_plugin_shutdown ();
    } while (sighup_received);

  sigint_handler (0);
  
  return 0;
}
