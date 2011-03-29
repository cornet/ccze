/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-color.c -- Color-handling routines for CCZE
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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ccze-private.h"
#include "ccze-compat.h"

#define BOLD A_BOLD |
#define BLACK COLOR_PAIR (0)
#define RED COLOR_PAIR (1)
#define GREEN COLOR_PAIR (2)
#define YELLOW COLOR_PAIR (3)
#define BLUE COLOR_PAIR (4)
#define CYAN COLOR_PAIR (5)
#define MAGENTA COLOR_PAIR (6)
#define WHITE COLOR_PAIR (7)

#define RAW_ANSI_BOLD 0x1000
#define RAW_ANSI_UNDERLINE 0x2000
#define RAW_ANSI_REVERSE 0x4000
#define RAW_ANSI_BLINK 0x8000
#define RAW_ANSI_BLACK 0
#define RAW_ANSI_RED 1
#define RAW_ANSI_GREEN 2
#define RAW_ANSI_YELLOW 3
#define RAW_ANSI_BLUE 4
#define RAW_ANSI_CYAN 5
#define RAW_ANSI_MAGENTA 6
#define RAW_ANSI_WHITE 7

#define CCZE_KEYWORD_R(k,c,s) k, c, s

#if CCZE_DUMP
#define CCZE_KEYWORD(k,c,d) CCZE_KEYWORD_R (k, c, 1), d
#define CCZE_KEYWORD_H(k,c,d) CCZE_KEYWORD_R (k, c, 0), d
#else
#define CCZE_KEYWORD(k,c,d) CCZE_KEYWORD_R (k, c, 1)
#define CCZE_KEYWORD_H(k,c,d) CCZE_KEYWORD_R (k, c, 0)
#endif

static int ccze_color_table [CCZE_COLOR_LAST + 1];

static char *ccze_csscolor_normal_map[] = {
  "black", "darkred", "#00C000", "brown", "blue", "darkcyan",
  "darkmagenta", "grey" };
static char *ccze_csscolor_bold_map[] = {
  "black", "red", "lime", "yellow", "slateblue",
  "cyan", "magenta", "white" };
static char *ccze_cssbody = "#404040";

typedef struct
{
  char *name;
  int value;
} ccze_colorname_t;
static ccze_colorname_t ccze_colorname_map[] = {
  {"black", 0},
  {"red", 1},
  {"green", 2},
  {"yellow", 3},
  {"blue", 4},
  {"cyan", 5},
  {"magenta", 6},
  {"white", 7},
  {"on_black", 0},
  {"on_red", 1},
  {"on_green", 2},
  {"on_yellow", 3},
  {"on_blue", 4},
  {"on_cyan", 5},
  {"on_magenta", 6},
  {"on_white", 7}
};

typedef struct
{
  char *keyword;
  ccze_color_t idx;
  int settable;
#if CCZE_DUMP
  char *comment;
#endif
} ccze_color_keyword_t;
static ccze_color_keyword_t ccze_color_keyword_map[] = {
  {CCZE_KEYWORD ("default", CCZE_COLOR_DEFAULT, "Default (not colorised)")},
  {CCZE_KEYWORD ("unknown", CCZE_COLOR_UNKNOWN, "Unknown message")},
  {CCZE_KEYWORD ("date", CCZE_COLOR_DATE, "Dates and times")},
  {CCZE_KEYWORD ("host", CCZE_COLOR_HOST, "Host names and IP numbers")},
  {CCZE_KEYWORD ("mac", CCZE_COLOR_MAC, "MAC addresses")},
  {CCZE_KEYWORD ("pid", CCZE_COLOR_PID, "PIDs (Process IDs)")},
  {CCZE_KEYWORD ("pid-sqbr", CCZE_COLOR_PIDB, "Brackets around PIDs")},
  {CCZE_KEYWORD ("get", CCZE_COLOR_HTTP_GET, "HTTP GET")},
  {CCZE_KEYWORD ("post", CCZE_COLOR_HTTP_POST, "HTTP POST")},
  {CCZE_KEYWORD ("head", CCZE_COLOR_HTTP_HEAD, "HTTP HEAD")},
  {CCZE_KEYWORD ("put", CCZE_COLOR_HTTP_PUT, "HTTP PUT")},
  {CCZE_KEYWORD ("connect", CCZE_COLOR_HTTP_CONNECT, "HTTP CONNECT")},
  {CCZE_KEYWORD ("trace", CCZE_COLOR_HTTP_TRACE, "HTTP TRACE")},
  {CCZE_KEYWORD ("httpcodes", CCZE_COLOR_HTTPCODES,
		 "HTTP status codes (200, 404, etc)")},
  {CCZE_KEYWORD ("gettime", CCZE_COLOR_GETTIME, "Transfer times")},
  {CCZE_KEYWORD ("getsize", CCZE_COLOR_GETSIZE, "Transfer sizes")},
  {CCZE_KEYWORD ("debug", CCZE_COLOR_DEBUG, "Debug messages")},
  {CCZE_KEYWORD ("error", CCZE_COLOR_ERROR, "Error messages")},
  {CCZE_KEYWORD ("warning", CCZE_COLOR_WARNING, "Warnings")},
  {CCZE_KEYWORD ("bad", CCZE_COLOR_BADWORD, "\"Bad words\"")},
  {CCZE_KEYWORD ("good", CCZE_COLOR_GOODWORD, "\"Good words\"")},
  {CCZE_KEYWORD ("system", CCZE_COLOR_SYSTEMWORD, "\"System words\"")},
  {CCZE_KEYWORD ("process", CCZE_COLOR_PROC, "Sender process")},
  {CCZE_KEYWORD ("dir", CCZE_COLOR_DIR, "Directory names")},
  {CCZE_KEYWORD ("file", CCZE_COLOR_FILE, "File names")},
  {CCZE_KEYWORD ("prot", CCZE_COLOR_PROT, "Protocols")},
  {CCZE_KEYWORD ("service", CCZE_COLOR_SERVICE, "Services")},
  {CCZE_KEYWORD ("email", CCZE_COLOR_EMAIL, "E-mail addresses")},
  {CCZE_KEYWORD ("size", CCZE_COLOR_SIZE, "Sizes")},
  {CCZE_KEYWORD ("version", CCZE_COLOR_VERSION, "Version numbers")},
  {CCZE_KEYWORD ("address", CCZE_COLOR_ADDRESS, "Memory addresses")},
  {CCZE_KEYWORD ("uri", CCZE_COLOR_URI, "URIs (http://, ftp://, etc)")},
  {CCZE_KEYWORD ("miss", CCZE_COLOR_PROXY_MISS, "Proxy MISS")},
  {CCZE_KEYWORD ("parent", CCZE_COLOR_PROXY_PARENT, "Proxy PARENT")},
  {CCZE_KEYWORD ("direct", CCZE_COLOR_PROXY_DIRECT, "Proxy DIRECT")},
  {CCZE_KEYWORD ("hit", CCZE_COLOR_PROXY_HIT, "Proxy HIT")},
  {CCZE_KEYWORD ("deny", CCZE_COLOR_PROXY_DENIED, "Proxy DENIED")},
  {CCZE_KEYWORD ("ident", CCZE_COLOR_IDENT, "Remote user (proxy/http)")},
  {CCZE_KEYWORD ("refresh", CCZE_COLOR_PROXY_REFRESH, "Proxy REFRESH")},
  {CCZE_KEYWORD ("swapfail", CCZE_COLOR_PROXY_SWAPFAIL, "Proxy SWAPFAIL")},
  {CCZE_KEYWORD ("ctype", CCZE_COLOR_CTYPE, "Content type (http/proxy)")},
  {CCZE_KEYWORD ("create", CCZE_COLOR_PROXY_CREATE, "Proxy CREATE")},
  {CCZE_KEYWORD ("swapin", CCZE_COLOR_PROXY_SWAPIN, "Proxy SWAPIN")},
  {CCZE_KEYWORD ("swapout", CCZE_COLOR_PROXY_SWAPOUT, "Proxy SWAPOUT")},
  {CCZE_KEYWORD ("release", CCZE_COLOR_PROXY_RELEASE, "Proxy RELEASE")},
  {CCZE_KEYWORD ("swapnum", CCZE_COLOR_SWAPNUM, "Proxy swap number")},
  {CCZE_KEYWORD ("user", CCZE_COLOR_USER, "Usernames")},
  {CCZE_KEYWORD ("numbers", CCZE_COLOR_NUMBERS, "Numbers")},
  {CCZE_KEYWORD ("subject", CCZE_COLOR_SUBJECT, "Subject lines (procmail)")},
  {CCZE_KEYWORD ("signal", CCZE_COLOR_SIGNAL, "Signal names")},
  {CCZE_KEYWORD ("incoming", CCZE_COLOR_INCOMING, "Incoming mail (exim)")},
  {CCZE_KEYWORD ("outgoing", CCZE_COLOR_OUTGOING, "Outgoing mail (exim)")},
  {CCZE_KEYWORD ("uniqn", CCZE_COLOR_UNIQN, "Unique ID (exim)")},
  {CCZE_KEYWORD ("repeat", CCZE_COLOR_REPEAT,
		 "'last message repeated N times'")},
  {CCZE_KEYWORD ("field", CCZE_COLOR_FIELD, "RFC822 Field")},
  {CCZE_KEYWORD ("chain", CCZE_COLOR_CHAIN, "Chain names (ulogd)")},
  {CCZE_KEYWORD ("percentage", CCZE_COLOR_PERCENTAGE, "Percentages")},
  {CCZE_KEYWORD ("ftpcodes", CCZE_COLOR_FTPCODES, "FTP codes")},
  {CCZE_KEYWORD ("keyword", CCZE_COLOR_KEYWORD,
		 "Various keywords (like PHP in php.log, etc)")},
  {CCZE_KEYWORD ("pkgstatus", CCZE_COLOR_PKGSTATUS, "package status (dpkg)")},
  {CCZE_KEYWORD ("pkg", CCZE_COLOR_PKG, "package name (dpkg)")},
  
  {CCZE_KEYWORD_H ("black", CCZE_COLOR_STATIC_BLACK, "Black")},
  {CCZE_KEYWORD_H ("red", CCZE_COLOR_STATIC_RED, "Red")},
  {CCZE_KEYWORD_H ("green", CCZE_COLOR_STATIC_GREEN, "Green")},
  {CCZE_KEYWORD_H ("yellow", CCZE_COLOR_STATIC_YELLOW, "Yyellow")},
  {CCZE_KEYWORD_H ("blue", CCZE_COLOR_STATIC_BLUE, "Blue")},
  {CCZE_KEYWORD_H ("cyan", CCZE_COLOR_STATIC_CYAN, "Cyan")},
  {CCZE_KEYWORD_H ("magenta", CCZE_COLOR_STATIC_MAGENTA, "Magenta")},
  {CCZE_KEYWORD_H ("white", CCZE_COLOR_STATIC_WHITE, "White")},
  {CCZE_KEYWORD_H ("bold_black", CCZE_COLOR_STATIC_BOLD_BLACK,
		   "Bold black")},
  {CCZE_KEYWORD_H ("bold_red", CCZE_COLOR_STATIC_BOLD_RED, "Bold red")},
  {CCZE_KEYWORD_H ("bold_green", CCZE_COLOR_STATIC_BOLD_GREEN,
		   "Bold green")},
  {CCZE_KEYWORD_H ("bold_yellow", CCZE_COLOR_STATIC_BOLD_YELLOW,
		   "Bold yellow")},
  {CCZE_KEYWORD_H ("bold_blue", CCZE_COLOR_STATIC_BOLD_BLUE, "Bold blue")},
  {CCZE_KEYWORD_H ("bold_cyan", CCZE_COLOR_STATIC_BOLD_CYAN, "Bold cyan")},
  {CCZE_KEYWORD_H ("bold_magenta", CCZE_COLOR_STATIC_BOLD_MAGENTA,
		   "Bold magenta")},
  {CCZE_KEYWORD_H ("bold_white", CCZE_COLOR_STATIC_BOLD_WHITE,
		   "Bold white")},
};

char *
ccze_color_to_name_simple (int color)
{
  switch (color)
    {
    case BLACK:
      return "black";
    case RED:
      return "red";
    case GREEN:
      return "green";
    case YELLOW:
      return "yellow";
    case BLUE:
      return "blue";
    case CYAN:
      return "cyan";
    case MAGENTA:
      return "magenta";
    case WHITE:
      return "white";
    }
  return NULL;
}

static char *
ccze_color_to_name_css (int color, ccze_color_t realcolor)
{
  if (ccze_color (realcolor) & A_BOLD)
    return ccze_csscolor_bold_map[PAIR_NUMBER (color)];
  else
    return ccze_csscolor_normal_map[PAIR_NUMBER (color)];
}

int
ccze_color_strip_attrib (int color)
{
  int mycolor = color;

  if (mycolor & A_BOLD)
    mycolor ^= A_BOLD;
  if (mycolor & A_UNDERLINE)
    mycolor ^= A_UNDERLINE;
  if (mycolor & A_REVERSE)
    mycolor ^= A_REVERSE;
  if (mycolor & A_BLINK)
    mycolor ^= A_BLINK;

  return mycolor;
}

char *
ccze_color_lookup_name (ccze_color_t color)
{
  size_t cidx;
  
  for (cidx = 0; cidx < sizeof (ccze_color_keyword_map); cidx++)
    if (ccze_color_keyword_map[cidx].idx == color)
      return ccze_color_keyword_map[cidx].keyword;
  return NULL;
}

char *
ccze_color_to_css (ccze_color_t cidx)
{
  int my_color = ccze_color_strip_attrib (ccze_color (cidx));
  char *str, *tmp;

  if (my_color < COLOR_PAIR (8))
    asprintf (&tmp, "\tcolor: %s\n", ccze_color_to_name_css (my_color, cidx));
  else
    {
      int i,j;
      char *ci, *cj;

      j = (my_color >> 8) % 8;
      i = (my_color >> 8) / 8;
      ci = ccze_color_to_name_css (COLOR_PAIR (i), cidx);
      cj = ccze_color_to_name_css (COLOR_PAIR (j), cidx);
      asprintf (&tmp, "\tcolor: %s\n\ttext-background: %s\n", cj, ci);
    }

  if (ccze_color (cidx) & A_UNDERLINE)
    asprintf (&str, ".ccze_%s {\n%s\ttext-decoration: underline\n}\n",
	      ccze_color_lookup_name (cidx), tmp);
  else
    asprintf (&str, ".ccze_%s {\n%s}\n",
	      ccze_color_lookup_name (cidx), tmp);
  free (tmp);
  
  return str;
}

void
ccze_colors_to_css (void)
{
  ccze_color_t cidx;

  for (cidx = CCZE_COLOR_DATE; cidx < CCZE_COLOR_LAST; cidx++)
    {
      char *line;

      line = ccze_color_to_css (cidx);
      printf ("%s\n", line);
      free (line);
    }
}

int
ccze_color (ccze_color_t idx)
{
  return ccze_color_table[idx];
}

static int
_ccze_color_keyword_lookup (const char *key, int hiddentoo)
{
  size_t i;

  for (i = 0; i < sizeof (ccze_color_keyword_map) /
	 sizeof (ccze_color_keyword_t); i++)
    if (!strcmp (key, ccze_color_keyword_map[i].keyword)
	&& (hiddentoo || ccze_color_keyword_map[i].settable))
      return ccze_color_keyword_map[i].idx;
  return -1;
}

int
ccze_color_keyword_lookup (const char *key)
{
  return _ccze_color_keyword_lookup (key, 1);
}

static int
_ccze_colorname_map_lookup (const char *color)
{
  size_t i;

  if (color[0] == '\'')
    {
      char *tmp = strndup (&color[1], strlen (color) - 2);
      int rval = ccze_color_table[_ccze_color_keyword_lookup (tmp, 0)];
      free (tmp);
      return rval;
    }
  
  for (i = 0; i < sizeof (ccze_colorname_map) / sizeof (ccze_colorname_t); i++)
    if (!strcmp (color, ccze_colorname_map[i].name))
      return ccze_colorname_map[i].value;
  return -1;
}

void
ccze_color_parse (char *line)
{
  char *tmp, *keyword, *color, *pre = NULL, *bg;
  int ncolor = 0, nkeyword = 0, nbg, rcolor, csskey = 0;
      
  keyword = strtok (line, " \t\n=");
  if (!keyword)
    return;
  if (strstr (keyword, "css") == keyword)
    csskey = 1;
  else
    {
      tmp = strstr (line, "#");
      if (tmp)
	tmp[0]='\0';
    }

  if (!csskey &&
      ((nkeyword = _ccze_color_keyword_lookup (keyword, 0)) == -1))
    return;

  color = strtok (NULL, " \t\n");
  if (!csskey)
    {
      if (color &&
	  (!strcmp (color, "bold") || !strcmp (color, "underline") ||
	   !strcmp (color, "reverse") || !strcmp (color, "blink")))
	{
	  pre = color;
	  color = strtok (NULL, " \t\n");
	}
    }
      
  if (!color)
    return;

  if (!csskey && (ncolor = _ccze_colorname_map_lookup (color)) == -1)
    return;

  bg = strtok (NULL, " \t\n");
  if (bg)
    {
      if (!csskey && (nbg = _ccze_colorname_map_lookup (bg)) != -1)
      {
	if (ccze_config.mode == CCZE_MODE_RAW_ANSI)
	  ncolor += (nbg << 8);
	else
	  ncolor += nbg*8;
      }
    }
      
  if (color[0] == '\'')
    rcolor = ncolor;
  else
  {
    if (ccze_config.mode == CCZE_MODE_RAW_ANSI)
      rcolor = ncolor;
    else
      rcolor = COLOR_PAIR (ncolor);
  }

  if (pre)
    {
      if (!strcmp (pre, "bold"))
	rcolor |= (ccze_config.mode == CCZE_MODE_RAW_ANSI) ?
	  RAW_ANSI_BOLD : A_BOLD;
      else if (!strcmp (pre, "underline"))
	rcolor |= (ccze_config.mode == CCZE_MODE_RAW_ANSI) ?
	  RAW_ANSI_UNDERLINE : A_UNDERLINE;
      else if (!strcmp (pre, "reverse"))
	rcolor |= (ccze_config.mode == CCZE_MODE_RAW_ANSI) ?
	  RAW_ANSI_REVERSE : A_REVERSE;
      else if (!strcmp (pre, "blink"))
	rcolor |= (ccze_config.mode == CCZE_MODE_RAW_ANSI) ?
	  RAW_ANSI_BLINK : A_BLINK;
    }

  if (!csskey)
    ccze_color_table[nkeyword] = rcolor;
  else
    {
      int bold = 0;

      if (!strcmp (keyword, "cssbody"))
	ccze_cssbody = strdup (color);
      else
	{
	  keyword += 3;
	  if (strstr (keyword, "bold") == keyword)
	    {
	      keyword += 4;
	      bold = 1;
	    }
	  ncolor = _ccze_colorname_map_lookup (keyword);
	  if (bold)
	    ccze_csscolor_bold_map[ncolor] = strdup (color);
	  else
	    ccze_csscolor_normal_map[ncolor] = strdup (color);
	}
    }
}

void
ccze_color_load (const char *fn)
{
  FILE *fp;
  char *line = NULL;
  size_t len;
  struct stat stb;

  stat (fn, &stb);
  if (!S_ISREG (stb.st_mode))
    return;
  
  fp = fopen (fn, "r");
  if (!fp)
    return;
  while (getline (&line, &len, fp) != -1)
    ccze_color_parse (line);
  free (line);
  fclose (fp);
}

char *
ccze_cssbody_color (void)
{
  return ccze_cssbody;
}

static void
ccze_color_init_raw_ansi (void)
{
  ccze_color_table[CCZE_COLOR_DATE] = (RAW_ANSI_BOLD | RAW_ANSI_CYAN);
  ccze_color_table[CCZE_COLOR_HOST] = (RAW_ANSI_BOLD | RAW_ANSI_BLUE);
  ccze_color_table[CCZE_COLOR_PROC] = (RAW_ANSI_GREEN);
  ccze_color_table[CCZE_COLOR_PID] = (RAW_ANSI_BOLD | RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_PIDB] = (RAW_ANSI_BOLD | RAW_ANSI_GREEN);
  ccze_color_table[CCZE_COLOR_DEFAULT] = (RAW_ANSI_CYAN);
  ccze_color_table[CCZE_COLOR_EMAIL] = (RAW_ANSI_BOLD | RAW_ANSI_GREEN);
  ccze_color_table[CCZE_COLOR_SUBJECT] = (RAW_ANSI_MAGENTA);
  ccze_color_table[CCZE_COLOR_DIR] = (RAW_ANSI_BOLD | RAW_ANSI_CYAN);
  ccze_color_table[CCZE_COLOR_FILE] = ccze_color_table[CCZE_COLOR_DIR];
  ccze_color_table[CCZE_COLOR_SIZE] = (RAW_ANSI_BOLD | RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_USER] = (RAW_ANSI_BOLD | RAW_ANSI_YELLOW);
  ccze_color_table[CCZE_COLOR_HTTPCODES] = (RAW_ANSI_BOLD | RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_GETSIZE] = (RAW_ANSI_MAGENTA);
  ccze_color_table[CCZE_COLOR_HTTP_GET] = (RAW_ANSI_GREEN);
  ccze_color_table[CCZE_COLOR_HTTP_POST] = (RAW_ANSI_BOLD | RAW_ANSI_GREEN);
  ccze_color_table[CCZE_COLOR_HTTP_HEAD] = (RAW_ANSI_GREEN);
  ccze_color_table[CCZE_COLOR_HTTP_PUT] = (RAW_ANSI_BOLD | RAW_ANSI_GREEN);
  ccze_color_table[CCZE_COLOR_HTTP_CONNECT] = (RAW_ANSI_GREEN);
  ccze_color_table[CCZE_COLOR_HTTP_TRACE] = (RAW_ANSI_GREEN);
  ccze_color_table[CCZE_COLOR_UNKNOWN] = ccze_color_table[CCZE_COLOR_DEFAULT];
  ccze_color_table[CCZE_COLOR_GETTIME] = (RAW_ANSI_BOLD | RAW_ANSI_MAGENTA);
  ccze_color_table[CCZE_COLOR_URI] = (RAW_ANSI_BOLD | RAW_ANSI_GREEN);
  ccze_color_table[CCZE_COLOR_IDENT] = (RAW_ANSI_BOLD | RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_CTYPE] = (RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_ERROR] = (RAW_ANSI_BOLD | RAW_ANSI_RED);
  ccze_color_table[CCZE_COLOR_PROXY_MISS] = (RAW_ANSI_RED);
  ccze_color_table[CCZE_COLOR_PROXY_HIT] = (RAW_ANSI_BOLD | RAW_ANSI_YELLOW);
  ccze_color_table[CCZE_COLOR_PROXY_DENIED] = (RAW_ANSI_BOLD | RAW_ANSI_RED);
  ccze_color_table[CCZE_COLOR_PROXY_REFRESH] = (RAW_ANSI_BOLD | RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_PROXY_SWAPFAIL] = (RAW_ANSI_BOLD | RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_DEBUG] = (RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_WARNING] = (RAW_ANSI_RED);
  ccze_color_table[CCZE_COLOR_PROXY_DIRECT] = (RAW_ANSI_BOLD | RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_PROXY_PARENT] = (RAW_ANSI_BOLD | RAW_ANSI_YELLOW);
  ccze_color_table[CCZE_COLOR_SWAPNUM] = ccze_color_table[CCZE_COLOR_DEFAULT]; /* XXX */
  ccze_color_table[CCZE_COLOR_PROXY_CREATE] = (RAW_ANSI_BOLD | RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_PROXY_SWAPIN] = (RAW_ANSI_BOLD | RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_PROXY_SWAPOUT] = (RAW_ANSI_BOLD | RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_PROXY_RELEASE] = (RAW_ANSI_BOLD | RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_MAC] = (RAW_ANSI_BOLD | RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_VERSION] = (RAW_ANSI_BOLD | RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_ADDRESS] = (RAW_ANSI_BOLD | RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_NUMBERS] = (RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_SIGNAL] = (RAW_ANSI_BOLD | RAW_ANSI_YELLOW);
  ccze_color_table[CCZE_COLOR_SERVICE] = (RAW_ANSI_BOLD | RAW_ANSI_MAGENTA);
  ccze_color_table[CCZE_COLOR_PROT] = (RAW_ANSI_MAGENTA);
  ccze_color_table[CCZE_COLOR_BADWORD] = (RAW_ANSI_BOLD | RAW_ANSI_YELLOW);
  ccze_color_table[CCZE_COLOR_GOODWORD] = (RAW_ANSI_BOLD | RAW_ANSI_GREEN);
  ccze_color_table[CCZE_COLOR_SYSTEMWORD] = (RAW_ANSI_BOLD | RAW_ANSI_CYAN);
  ccze_color_table[CCZE_COLOR_INCOMING] = (RAW_ANSI_BOLD | RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_OUTGOING] = (RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_UNIQN] = (RAW_ANSI_BOLD | RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_REPEAT] = (RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_FIELD] = (RAW_ANSI_GREEN);
  ccze_color_table[CCZE_COLOR_CHAIN] = (RAW_ANSI_CYAN);
  ccze_color_table[CCZE_COLOR_PERCENTAGE] = (RAW_ANSI_BOLD | RAW_ANSI_YELLOW);
  ccze_color_table[CCZE_COLOR_FTPCODES] = (RAW_ANSI_CYAN);
  ccze_color_table[CCZE_COLOR_KEYWORD] = (RAW_ANSI_BOLD | RAW_ANSI_YELLOW);
  ccze_color_table[CCZE_COLOR_PKGSTATUS] = (RAW_ANSI_GREEN);
  ccze_color_table[CCZE_COLOR_PKG] = (RAW_ANSI_BOLD|RAW_ANSI_RED);

  ccze_color_table[CCZE_COLOR_STATIC_BLACK] = (RAW_ANSI_BLACK);
  ccze_color_table[CCZE_COLOR_STATIC_RED] = (RAW_ANSI_RED);
  ccze_color_table[CCZE_COLOR_STATIC_GREEN] = (RAW_ANSI_GREEN);
  ccze_color_table[CCZE_COLOR_STATIC_YELLOW] = (RAW_ANSI_YELLOW);
  ccze_color_table[CCZE_COLOR_STATIC_BLUE] = (RAW_ANSI_BLUE);
  ccze_color_table[CCZE_COLOR_STATIC_CYAN] = (RAW_ANSI_CYAN);
  ccze_color_table[CCZE_COLOR_STATIC_MAGENTA] = (RAW_ANSI_MAGENTA);
  ccze_color_table[CCZE_COLOR_STATIC_WHITE] = (RAW_ANSI_WHITE);
  ccze_color_table[CCZE_COLOR_STATIC_BOLD_BLACK] = (RAW_ANSI_BOLD | RAW_ANSI_BLACK);
  ccze_color_table[CCZE_COLOR_STATIC_BOLD_RED] = (RAW_ANSI_BOLD | RAW_ANSI_RED);
  ccze_color_table[CCZE_COLOR_STATIC_BOLD_GREEN] = (RAW_ANSI_BOLD | RAW_ANSI_GREEN);
  ccze_color_table[CCZE_COLOR_STATIC_BOLD_YELLOW] = (RAW_ANSI_BOLD | RAW_ANSI_YELLOW);
  ccze_color_table[CCZE_COLOR_STATIC_BOLD_BLUE] = (RAW_ANSI_BOLD | RAW_ANSI_BLUE);
  ccze_color_table[CCZE_COLOR_STATIC_BOLD_CYAN] = (RAW_ANSI_BOLD | RAW_ANSI_CYAN);
  ccze_color_table[CCZE_COLOR_STATIC_BOLD_MAGENTA] = (RAW_ANSI_BOLD | RAW_ANSI_MAGENTA);
  ccze_color_table[CCZE_COLOR_STATIC_BOLD_WHITE] = (RAW_ANSI_BOLD | RAW_ANSI_WHITE);
      
  ccze_color_table[CCZE_COLOR_LAST] = (RAW_ANSI_CYAN);
}

void
ccze_color_init (void)
{
  if (ccze_config.mode == CCZE_MODE_RAW_ANSI)
    {
      ccze_color_init_raw_ansi ();
      return;
    }
  
  ccze_color_table[CCZE_COLOR_DATE] = (BOLD CYAN);
  ccze_color_table[CCZE_COLOR_HOST] = (BOLD BLUE);
  ccze_color_table[CCZE_COLOR_PROC] = (GREEN);
  ccze_color_table[CCZE_COLOR_PID] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_PIDB] = (BOLD GREEN);
  ccze_color_table[CCZE_COLOR_DEFAULT] = (CYAN);
  ccze_color_table[CCZE_COLOR_EMAIL] = (BOLD GREEN);
  ccze_color_table[CCZE_COLOR_SUBJECT] = (MAGENTA);
  ccze_color_table[CCZE_COLOR_DIR] = (BOLD CYAN);
  ccze_color_table[CCZE_COLOR_FILE] = ccze_color_table[CCZE_COLOR_DIR];
  ccze_color_table[CCZE_COLOR_SIZE] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_USER] = (BOLD YELLOW);
  ccze_color_table[CCZE_COLOR_HTTPCODES] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_GETSIZE] = (MAGENTA);
  ccze_color_table[CCZE_COLOR_HTTP_GET] = (GREEN);
  ccze_color_table[CCZE_COLOR_HTTP_POST] = (BOLD GREEN);
  ccze_color_table[CCZE_COLOR_HTTP_HEAD] = (GREEN);
  ccze_color_table[CCZE_COLOR_HTTP_PUT] = (BOLD GREEN);
  ccze_color_table[CCZE_COLOR_HTTP_CONNECT] = (GREEN);
  ccze_color_table[CCZE_COLOR_HTTP_TRACE] = (GREEN);
  ccze_color_table[CCZE_COLOR_UNKNOWN] = ccze_color_table[CCZE_COLOR_DEFAULT];
  ccze_color_table[CCZE_COLOR_GETTIME] = (BOLD MAGENTA);
  ccze_color_table[CCZE_COLOR_URI] = (BOLD GREEN);
  ccze_color_table[CCZE_COLOR_IDENT] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_CTYPE] = (WHITE);
  ccze_color_table[CCZE_COLOR_ERROR] = (BOLD RED);
  ccze_color_table[CCZE_COLOR_PROXY_MISS] = (RED);
  ccze_color_table[CCZE_COLOR_PROXY_HIT] = (BOLD YELLOW);
  ccze_color_table[CCZE_COLOR_PROXY_DENIED] = (BOLD RED);
  ccze_color_table[CCZE_COLOR_PROXY_REFRESH] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_PROXY_SWAPFAIL] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_DEBUG] = (WHITE);
  ccze_color_table[CCZE_COLOR_WARNING] = (RED);
  ccze_color_table[CCZE_COLOR_PROXY_DIRECT] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_PROXY_PARENT] = (BOLD YELLOW);
  ccze_color_table[CCZE_COLOR_SWAPNUM] = COLOR_PAIR (4 + 7*8);
  ccze_color_table[CCZE_COLOR_PROXY_CREATE] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_PROXY_SWAPIN] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_PROXY_SWAPOUT] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_PROXY_RELEASE] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_MAC] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_VERSION] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_ADDRESS] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_NUMBERS] = (WHITE);
  ccze_color_table[CCZE_COLOR_SIGNAL] = (BOLD YELLOW);
  ccze_color_table[CCZE_COLOR_SERVICE] = (BOLD MAGENTA);
  ccze_color_table[CCZE_COLOR_PROT] = (MAGENTA);
  ccze_color_table[CCZE_COLOR_BADWORD] = (BOLD YELLOW);
  ccze_color_table[CCZE_COLOR_GOODWORD] = (BOLD GREEN);
  ccze_color_table[CCZE_COLOR_SYSTEMWORD] = (BOLD CYAN);
  ccze_color_table[CCZE_COLOR_INCOMING] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_OUTGOING] = (WHITE);
  ccze_color_table[CCZE_COLOR_UNIQN] = (BOLD WHITE);
  ccze_color_table[CCZE_COLOR_REPEAT] = (WHITE);
  ccze_color_table[CCZE_COLOR_FIELD] = (GREEN);
  ccze_color_table[CCZE_COLOR_CHAIN] = (CYAN);
  ccze_color_table[CCZE_COLOR_PERCENTAGE] = (BOLD YELLOW);
  ccze_color_table[CCZE_COLOR_FTPCODES] = (CYAN);
  ccze_color_table[CCZE_COLOR_KEYWORD] = (BOLD YELLOW);
  ccze_color_table[CCZE_COLOR_PKGSTATUS] = (GREEN);
  ccze_color_table[CCZE_COLOR_PKG] = (BOLD RED);

  ccze_color_table[CCZE_COLOR_STATIC_BLACK] = (BLACK);
  ccze_color_table[CCZE_COLOR_STATIC_RED] = (RED);
  ccze_color_table[CCZE_COLOR_STATIC_GREEN] = (GREEN);
  ccze_color_table[CCZE_COLOR_STATIC_YELLOW] = (YELLOW);
  ccze_color_table[CCZE_COLOR_STATIC_BLUE] = (BLUE);
  ccze_color_table[CCZE_COLOR_STATIC_CYAN] = (CYAN);
  ccze_color_table[CCZE_COLOR_STATIC_MAGENTA] = (MAGENTA);
  ccze_color_table[CCZE_COLOR_STATIC_WHITE] = (WHITE);
  ccze_color_table[CCZE_COLOR_STATIC_BOLD_BLACK] = (BOLD BLACK);
  ccze_color_table[CCZE_COLOR_STATIC_BOLD_RED] = (BOLD RED);
  ccze_color_table[CCZE_COLOR_STATIC_BOLD_GREEN] = (BOLD GREEN);
  ccze_color_table[CCZE_COLOR_STATIC_BOLD_YELLOW] = (BOLD YELLOW);
  ccze_color_table[CCZE_COLOR_STATIC_BOLD_BLUE] = (BOLD BLUE);
  ccze_color_table[CCZE_COLOR_STATIC_BOLD_CYAN] = (BOLD CYAN);
  ccze_color_table[CCZE_COLOR_STATIC_BOLD_MAGENTA] = (BOLD MAGENTA);
  ccze_color_table[CCZE_COLOR_STATIC_BOLD_WHITE] = (BOLD WHITE);
  
  ccze_color_table[CCZE_COLOR_LAST] = (CYAN);
}
