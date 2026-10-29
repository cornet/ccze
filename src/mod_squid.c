/* -*- mode: c; c-file-style: "gnu" -*-
 * mod_squid.c -- Squid-related colorizers for CCZE
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

static void ccze_squid_setup (void);
static void ccze_squid_shutdown (void);
static int ccze_squid_handle (const char *str, size_t length, char **rest);

static ccze_regex_t *reg_squid_access, *reg_squid_store, *reg_squid_cache;


static ccze_color_t
_ccze_proxy_action (const char *action)
{
  if (strstr (action, "ERR") == action)
    return CCZE_COLOR_ERROR;
  if (strstr (action, "MISS"))
    return CCZE_COLOR_PROXY_MISS;
  if (strstr (action, "HIT"))
    return CCZE_COLOR_PROXY_HIT;
  if (strstr (action, "DENIED"))
    return CCZE_COLOR_PROXY_DENIED;
  if (strstr (action, "REFRESH"))
    return CCZE_COLOR_PROXY_REFRESH;
  if (strstr (action, "SWAPFAIL"))
    return CCZE_COLOR_PROXY_SWAPFAIL;
  if (strstr (action, "NONE"))
    return CCZE_COLOR_DEBUG;

  return CCZE_COLOR_UNKNOWN;
}

static ccze_color_t
_ccze_proxy_hierarch (const char *hierar)
{
  if (strstr (hierar, "NO") == hierar)
    return CCZE_COLOR_WARNING;
  if (strstr (hierar, "DIRECT"))
    return CCZE_COLOR_PROXY_DIRECT;
  if (strstr (hierar, "PARENT"))
    return CCZE_COLOR_PROXY_PARENT;
  if (strstr (hierar, "MISS"))
    return CCZE_COLOR_PROXY_MISS;

  return CCZE_COLOR_UNKNOWN;
}

static ccze_color_t
_ccze_proxy_tag (const char *tag)
{
  if (strstr (tag, "CREATE"))
    return CCZE_COLOR_PROXY_CREATE;
  if (strstr (tag, "SWAPIN"))
    return CCZE_COLOR_PROXY_SWAPIN;
  if (strstr (tag, "SWAPOUT"))
    return CCZE_COLOR_PROXY_SWAPOUT;
  if (strstr (tag, "RELEASE"))
    return CCZE_COLOR_PROXY_RELEASE;

  return CCZE_COLOR_UNKNOWN;
}

static char *
ccze_squid_access_log_process (const char *str, int *offsets, int match)
{
  char *date, *espace, *elaps, *host, *action, *httpc, *gsize;
  char *method, *uri, *ident, *hierar, *fhost, *ctype;

  ccze_regex_substring (str, offsets, match, 1, (const char **)&date);
  ccze_regex_substring (str, offsets, match, 2, (const char **)&espace);
  ccze_regex_substring (str, offsets, match, 3, (const char **)&elaps);
  ccze_regex_substring (str, offsets, match, 4, (const char **)&host);
  ccze_regex_substring (str, offsets, match, 5, (const char **)&action);
  ccze_regex_substring (str, offsets, match, 6, (const char **)&httpc);
  ccze_regex_substring (str, offsets, match, 7, (const char **)&gsize);
  ccze_regex_substring (str, offsets, match, 8, (const char **)&method);
  ccze_regex_substring (str, offsets, match, 9, (const char **)&uri);
  ccze_regex_substring (str, offsets, match, 10, (const char **)&ident);
  ccze_regex_substring (str, offsets, match, 11, (const char **)&hierar);
  ccze_regex_substring (str, offsets, match, 12, (const char **)&fhost);
  ccze_regex_substring (str, offsets, match, 13, (const char **)&ctype);

  ccze_print_date (date);
  ccze_addstr (CCZE_COLOR_DEFAULT, espace);
  ccze_addstr (CCZE_COLOR_GETTIME, elaps);
  ccze_space ();

  ccze_addstr (CCZE_COLOR_HOST, host);
  ccze_space ();

  ccze_addstr (_ccze_proxy_action (action), action);
  ccze_addstr (CCZE_COLOR_DEFAULT, "/");
  ccze_addstr (CCZE_COLOR_HTTPCODES, httpc);
  ccze_space ();

  ccze_addstr (CCZE_COLOR_GETSIZE, gsize);
  ccze_space ();

  ccze_addstr (ccze_http_action (method), method);
  ccze_space ();

  ccze_addstr (CCZE_COLOR_URI, uri);
  ccze_space ();

  ccze_addstr (CCZE_COLOR_IDENT, ident);
  ccze_space ();

  ccze_addstr (_ccze_proxy_hierarch (hierar), hierar);
  ccze_addstr (CCZE_COLOR_DEFAULT, "/");
  ccze_addstr (CCZE_COLOR_HOST, fhost);
  ccze_space ();

  ccze_addstr (CCZE_COLOR_CTYPE, ctype);

  ccze_newline ();

  free (date);
  free (espace);
  free (elaps);
  free (host);
  free (action);
  free (httpc);
  free (gsize);
  free (method);
  free (uri);
  free (ident);
  free (hierar);
  free (fhost);
  free (ctype);
  
  return NULL;
}

static char *
ccze_squid_cache_log_process (const char *str, int *offsets, int match)
{
  char *date, *other;

  ccze_regex_substring (str, offsets, match, 1, (const char **)&date);
  ccze_regex_substring (str, offsets, match, 3, (const char **)&other);

  ccze_addstr (CCZE_COLOR_DATE, date);
  ccze_space();

  free (date);
  return other;
}

static char *
ccze_squid_store_log_process (const char *str, int *offsets, int match)
{
  char *date, *tag, *swapnum, *swapname, *swapsum, *space1, *hcode;
  char *hdate, *lmdate, *expire, *ctype, *size, *read, *method;
  char *uri, *space2, *space3, *space4;
    
  ccze_regex_substring (str, offsets, match, 1, (const char **)&date);
  ccze_regex_substring (str, offsets, match, 2, (const char **)&tag);
  ccze_regex_substring (str, offsets, match, 3, (const char **)&swapnum);
  ccze_regex_substring (str, offsets, match, 4, (const char **)&swapname);
  ccze_regex_substring (str, offsets, match, 5, (const char **)&swapsum);
  ccze_regex_substring (str, offsets, match, 6, (const char **)&space1);
  ccze_regex_substring (str, offsets, match, 7, (const char **)&hcode);
  ccze_regex_substring (str, offsets, match, 8, (const char **)&space2);
  ccze_regex_substring (str, offsets, match, 9, (const char **)&hdate);
  ccze_regex_substring (str, offsets, match, 10, (const char **)&space3);
  ccze_regex_substring (str, offsets, match, 11, (const char **)&lmdate);
  ccze_regex_substring (str, offsets, match, 12, (const char **)&space4);
  ccze_regex_substring (str, offsets, match, 13, (const char **)&expire);
  ccze_regex_substring (str, offsets, match, 14, (const char **)&ctype);
  ccze_regex_substring (str, offsets, match, 15, (const char **)&size);
  ccze_regex_substring (str, offsets, match, 16, (const char **)&read);
  ccze_regex_substring (str, offsets, match, 17, (const char **)&method);
  ccze_regex_substring (str, offsets, match, 18, (const char **)&uri);

  ccze_print_date (date);
  ccze_space();
  ccze_addstr (_ccze_proxy_tag (tag), tag);
  ccze_space();
  ccze_addstr (CCZE_COLOR_SWAPNUM, swapnum);
  ccze_space();
  ccze_addstr (CCZE_COLOR_SWAPNUM, swapname);
  ccze_space();
  ccze_addstr (CCZE_COLOR_SWAPNUM, swapsum);
  ccze_addstr (CCZE_COLOR_DEFAULT, space1);
  ccze_addstr (CCZE_COLOR_HTTPCODES, hcode);
  ccze_addstr (CCZE_COLOR_DEFAULT, space2);
  ccze_print_date (hdate);
  ccze_addstr (CCZE_COLOR_DEFAULT, space3);
  ccze_print_date (lmdate);
  ccze_addstr (CCZE_COLOR_DEFAULT, space4);
  ccze_print_date (expire);
  ccze_space();
  ccze_addstr (CCZE_COLOR_CTYPE, ctype);
  ccze_space();
  ccze_addstr (CCZE_COLOR_GETSIZE, size);
  ccze_addstr (CCZE_COLOR_DEFAULT, "/");
  ccze_addstr (CCZE_COLOR_GETSIZE, read);
  ccze_space();
  ccze_addstr (ccze_http_action (method), method);
  ccze_space();
  ccze_addstr (CCZE_COLOR_URI, uri);

  ccze_newline ();

  free (date);
  free (tag);
  free (swapnum);
  free (swapname);
  free (swapsum);
  free (space1);
  free (hcode);
  free (hdate);
  free (lmdate);
  free (expire);
  free (ctype);
  free (size);
  free (read);
  free (method);
  free (uri);
  free (space2);
  free (space3);
  free (space4);
  
  return NULL;
}

static void
ccze_squid_setup (void)
{


  reg_squid_access = ccze_regex_compile
    ("^(\\d{9,10}\\.\\d{3})(\\s+)(\\d+)\\s(\\S+)\\s(\\w+)\\/(\\d{3})"
     "\\s(\\d+)\\s(\\w+)\\s(\\S+)\\s(\\S+)\\s(\\w+)\\/([\\d\\.]+|-)\\s(.*)");


  reg_squid_cache = ccze_regex_compile
    ("^(\\d{4}\\/\\d{2}\\/\\d{2}\\s(\\d{2}:){2}\\d{2}\\|)\\s(.*)$");


  reg_squid_store = ccze_regex_compile
    ("^([\\d\\.]+)\\s(\\w+)\\s(\\-?[\\dA-F]+)\\s+(\\S+)\\s([\\dA-F]+)"
     "(\\s+)(\\d{3}|\\?)(\\s+)(\\-?[\\d\\?]+)(\\s+)(\\-?[\\d\\?]+)(\\s+)"
     "(\\-?[\\d\\?]+)\\s(\\S+)\\s(\\-?[\\d|\\?]+)\\/(\\-?[\\d|\\?]+)\\s"
     "(\\S+)\\s(.*)");
}

static void
ccze_squid_shutdown (void)
{
  ccze_regex_free (reg_squid_access);
  ccze_regex_free (reg_squid_cache);
  ccze_regex_free (reg_squid_store);
}

static int
ccze_squid_handle (const char *str, size_t length, char **rest)
{
  int match, offsets[99];

  if ((match = ccze_regex_match (reg_squid_access, str,
			  length, offsets, 99)) >= 0)
    {
      *rest = ccze_squid_access_log_process (str, offsets, match);
      return 1;
    }
  
  if ((match = ccze_regex_match (reg_squid_store, str,
			  length, offsets, 99)) >= 0)
    {
      *rest = ccze_squid_store_log_process (str, offsets, match);
      return 1;
    }
  
  if ((match = ccze_regex_match (reg_squid_cache, str,
			  length, offsets, 99)) >= 0)
    {
      *rest = ccze_squid_cache_log_process (str, offsets, match);
      return 1;
    }

  return 0;
}

CCZE_DEFINE_PLUGIN (squid, FULL,
		    "Coloriser for squid access, store and cache logs.");
