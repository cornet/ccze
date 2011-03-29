/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-private.h -- Internal CCZE function prototypes
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

#ifndef _CCZE_PRIVATE_H
#define _CCZE_PRIVATE_H 1

#include <ccze.h>

/* ccze-color.c */
void ccze_color_init (void);
void ccze_color_parse (char *line);
void ccze_color_load (const char *fn);

int ccze_color_strip_attrib (int color);
char *ccze_color_to_name_simple (int color);
char *ccze_color_lookup_name (ccze_color_t color);
char *ccze_color_to_css (ccze_color_t cidx);
void ccze_colors_to_css (void);
char *ccze_cssbody_color (void);

/* ccze-plugin.c */
void ccze_plugin_init (void);
void ccze_plugin_argv_init (void);
int ccze_plugin_argv_set (const char *name, const char *args);
void ccze_plugin_argv_finalise (void);
ccze_plugin_t **ccze_plugins (void);
void ccze_plugin_load_all (void);
void ccze_plugin_load (const char *name);
void ccze_plugin_add (ccze_plugin_t *plugin);
void ccze_plugin_setup (void);
void ccze_plugin_shutdown (void);
void ccze_plugin_finalise (void);
void ccze_plugin_run (ccze_plugin_t **pluginset, char *subject,
		      size_t subjlen, char **rest,
		      ccze_plugin_type_t type, int *handled,
		      int *status);
void ccze_plugin_load_all_builtins (void);
int ccze_plugin_list_fancy (void);

/* ccze-wordcolor.c */
void ccze_wordcolor_process (const char *msg, int wcol, int slookup);
void ccze_wordcolor_setup (void);
void ccze_wordcolor_shutdown (void);

/* ccze.c */
typedef enum
{
  CCZE_MODE_CURSES,
  CCZE_MODE_RAW_ANSI,
  CCZE_MODE_HTML,
  CCZE_MODE_DEBUG,
  CCZE_MODE_PLUGIN_LIST
} ccze_mode_t;

typedef struct
{
  int scroll;
  int convdate;
  int wcol;
  int slookup;
  int remfac;
  int transparent;
  char *rcfile;
  char *cssfile;
  char **pluginlist;
  int pluginlist_alloc, pluginlist_len;
  char **color_argv;
  int color_argv_alloc, color_argv_len;

  ccze_mode_t mode;
} ccze_config_t;

extern ccze_config_t ccze_config;

#endif /* !_CCZE_PRIVATE_H */
