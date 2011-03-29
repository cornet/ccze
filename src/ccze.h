/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze.h -- Public interface to CCZE.
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

#ifndef _CCZE_H
#define _CCZE_H 1

#ifdef HAVE_SYSTEM_H
# include "system.h"
#endif
#include <pcre.h>
#include <ncurses.h>
#include <stddef.h>

/* Compatibility */
void *ccze_malloc (size_t size);
void *ccze_realloc (void *ptr, size_t size);
void *ccze_calloc (size_t nmemb, size_t size);

/* Colors */
typedef enum
{
  CCZE_COLOR_DATE = 0,
  CCZE_COLOR_HOST,
  CCZE_COLOR_PROC,
  CCZE_COLOR_PID,
  CCZE_COLOR_PIDB,
  CCZE_COLOR_DEFAULT,
  CCZE_COLOR_EMAIL,
  CCZE_COLOR_SUBJECT,
  CCZE_COLOR_DIR,
  CCZE_COLOR_FILE,
  CCZE_COLOR_SIZE,
  CCZE_COLOR_USER,
  CCZE_COLOR_HTTPCODES,
  CCZE_COLOR_GETSIZE,
  CCZE_COLOR_HTTP_GET,
  CCZE_COLOR_HTTP_POST,
  CCZE_COLOR_HTTP_HEAD,
  CCZE_COLOR_HTTP_PUT,
  CCZE_COLOR_HTTP_CONNECT,
  CCZE_COLOR_HTTP_TRACE,
  CCZE_COLOR_UNKNOWN,
  CCZE_COLOR_GETTIME,
  CCZE_COLOR_URI,
  CCZE_COLOR_IDENT,
  CCZE_COLOR_CTYPE,
  CCZE_COLOR_ERROR,
  CCZE_COLOR_PROXY_MISS,
  CCZE_COLOR_PROXY_HIT,
  CCZE_COLOR_PROXY_DENIED,
  CCZE_COLOR_PROXY_REFRESH,
  CCZE_COLOR_PROXY_SWAPFAIL,
  CCZE_COLOR_DEBUG,
  CCZE_COLOR_WARNING,
  CCZE_COLOR_PROXY_DIRECT,
  CCZE_COLOR_PROXY_PARENT,
  CCZE_COLOR_SWAPNUM,
  CCZE_COLOR_PROXY_CREATE,
  CCZE_COLOR_PROXY_SWAPIN,
  CCZE_COLOR_PROXY_SWAPOUT,
  CCZE_COLOR_PROXY_RELEASE,
  CCZE_COLOR_MAC,
  CCZE_COLOR_VERSION,
  CCZE_COLOR_ADDRESS,
  CCZE_COLOR_NUMBERS,
  CCZE_COLOR_SIGNAL,
  CCZE_COLOR_SERVICE,
  CCZE_COLOR_PROT,
  CCZE_COLOR_BADWORD,
  CCZE_COLOR_GOODWORD,
  CCZE_COLOR_SYSTEMWORD,
  CCZE_COLOR_INCOMING,
  CCZE_COLOR_OUTGOING,
  CCZE_COLOR_UNIQN,
  CCZE_COLOR_REPEAT,
  CCZE_COLOR_FIELD,
  CCZE_COLOR_CHAIN,
  CCZE_COLOR_PERCENTAGE,
  CCZE_COLOR_FTPCODES,
  CCZE_COLOR_KEYWORD,
  CCZE_COLOR_PKGSTATUS,
  CCZE_COLOR_PKG,

  CCZE_COLOR_STATIC_BLACK,
  CCZE_COLOR_STATIC_RED,
  CCZE_COLOR_STATIC_GREEN,
  CCZE_COLOR_STATIC_YELLOW,
  CCZE_COLOR_STATIC_BLUE,
  CCZE_COLOR_STATIC_CYAN,
  CCZE_COLOR_STATIC_MAGENTA,
  CCZE_COLOR_STATIC_WHITE,
  CCZE_COLOR_STATIC_BOLD_BLACK,
  CCZE_COLOR_STATIC_BOLD_RED,
  CCZE_COLOR_STATIC_BOLD_GREEN,
  CCZE_COLOR_STATIC_BOLD_YELLOW,
  CCZE_COLOR_STATIC_BOLD_BLUE,
  CCZE_COLOR_STATIC_BOLD_CYAN,
  CCZE_COLOR_STATIC_BOLD_MAGENTA,
  CCZE_COLOR_STATIC_BOLD_WHITE,
  
  CCZE_COLOR_LAST
} ccze_color_t;

int ccze_color (ccze_color_t idx);
int ccze_color_keyword_lookup (const char *key);

/* Helpers */
ccze_color_t ccze_http_action (const char *method);
void ccze_print_date (const char *date);
char *ccze_strbrk (char *str, char delim);
char *xstrdup (const char *str);

/* Display */
void ccze_addstr (ccze_color_t col, const char *str);
void ccze_newline (void);
void ccze_space (void);
void ccze_wordcolor_process_one (char *word, int slookup);

/* Plugins */
typedef void (*ccze_plugin_startup_t) (void);
typedef void (*ccze_plugin_shutdown_t) (void);
typedef int (*ccze_plugin_handle_t) (const char *str, size_t length,
				     char **rest);

typedef enum
{
  CCZE_PLUGIN_TYPE_FULL,
  CCZE_PLUGIN_TYPE_PARTIAL,
  CCZE_PLUGIN_TYPE_ANY
} ccze_plugin_type_t;

typedef struct
{
  int abi_version;
  void *dlhandle;
  char *name;
  char **argv;
  ccze_plugin_startup_t startup;
  ccze_plugin_shutdown_t shutdown;
  ccze_plugin_handle_t handler;
  ccze_plugin_type_t type;
  char *desc;
} ccze_plugin_t;

char **ccze_plugin_argv_get (const char *name);
const char *ccze_plugin_name_get (void);

#define CCZE_ABI_VERSION 2

#define CCZE_DEFINE_PLUGINS(plugins...) \
char *ccze_plugin_list[] = { plugins, NULL }

#if !defined(BUILTIN)
#define __default_plugin(name) ; \
char ccze_default_plugin[] = # name
#else
#define __default_plugin(name)
#endif

#define CCZE_DEFINE_PLUGIN(name,type,desc) \
ccze_plugin_t ccze_##name##_info = { CCZE_ABI_VERSION, \
				     NULL, \
				     # name, NULL, \
				     ccze_##name##_setup, \
				     ccze_##name##_shutdown, \
				     ccze_##name##_handle, \
				     CCZE_PLUGIN_TYPE_##type, desc } \
__default_plugin (name)

#endif /* !_CCZE_H */
