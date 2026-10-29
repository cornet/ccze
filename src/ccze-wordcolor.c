/* -*- mode: c; c-file-style: "gnu" -*-
 * ccze-wordcolor.c -- Word-coloriser functions
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
#include <ctype.h>
#include <netdb.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>

#include "ccze-private.h"
#include "ccze-compat.h"

static ccze_regex_t *reg_pre, *reg_post, *reg_host, *reg_mac, *reg_email;
static ccze_regex_t *reg_uri, *reg_size, *reg_ver, *reg_time, *reg_addr;
static ccze_regex_t *reg_num, *reg_sig, *reg_email2, *reg_hostip, *reg_msgid;

static char *words_bad[] = {
  "warn", "restart", "exit", "stop", "end", "shutting", "down", "close",
  "unreach", "can't", "cannot", "skip", "deny", "disable", "ignored",
  "miss", "oops", "not", "backdoor", "blocking", "ignoring",
  "unable", "readonly", "offline", "terminate", "empty", "virus"
};

static char *words_good[] = {
  "activ", "start", "ready", "online", "load", "ok", "register", "detected",
  "configured", "enable", "listen", "open", "complete", "attempt", "done",
  "check", "listen", "connect", "finish", "clean"
};

static char *words_error[] = {
  "error", "crit", "invalid", "fail", "false", "alarm", "fatal"
};

static char *words_system[] = {
  "ext2-fs", "reiserfs", "vfs", "iso", "isofs", "cslip", "ppp", "bsd",
  "linux", "tcp/ip", "mtrr", "pci", "isa", "scsi", "ide", "atapi",
  "bios", "cpu", "fpu", "discharging", "resume"
};

static char *
_stolower (const char *str)
{
  char *newstr = strdup (str);
  size_t i;

  for (i = 0; i < strlen (newstr); i++)
    newstr[i] = tolower (str[i]);

  return newstr;
}

void
ccze_wordcolor_process_one (char *word, int slookup)
{
  size_t wlen;
  int offsets[99];
  ccze_color_t col;
  int match, printed = 0;
  char *pre = NULL, *post = NULL, *tmp, *lword;

  col = CCZE_COLOR_DEFAULT;

  /** prefix **/
  if ((match = ccze_regex_match (reg_pre, word, strlen (word), offsets, 99)) >= 0)
    {
      ccze_regex_substring (word, offsets, match, 1, (const char **)&pre);
      ccze_regex_substring (word, offsets, match, 2, (const char **)&tmp);
      free (word);
      word = tmp;
    }
  else
    pre = NULL;

  /** postfix **/
  if ((match = ccze_regex_match (reg_post, word, strlen (word), offsets, 99)) >= 0)
    {
      ccze_regex_substring (word, offsets, match, 1, (const char **)&tmp);
      ccze_regex_substring (word, offsets, match, 2, (const char **)&post);
      free (word);
      word = tmp;
    }
  else
    post = NULL;

  wlen = strlen (word);
  lword = _stolower (word);
      
  /** Host **/
  if (ccze_regex_match (reg_host, lword, wlen, offsets, 99) >= 0)
    col = CCZE_COLOR_HOST;
  /** MAC address **/
  else if (ccze_regex_match (reg_mac, lword, wlen, offsets, 99) >= 0)
    col = CCZE_COLOR_MAC;
  /** Directory **/
  else if (lword[0] == '/')
    col = CCZE_COLOR_DIR;
  /** E-mail **/
  else if (ccze_regex_match (reg_email, lword, wlen, offsets, 99)
	   >= 0 && ccze_regex_match (reg_email2, lword, wlen, offsets,99) >= 0)
    col = CCZE_COLOR_EMAIL;
  /** Message-ID **/
  else if (ccze_regex_match (reg_msgid, lword, wlen, offsets, 99) >= 0)
    col = CCZE_COLOR_EMAIL;
  /** URI **/
  else if (ccze_regex_match (reg_uri, lword, wlen, offsets, 99) >= 0)
    col = CCZE_COLOR_URI;
  /** Size **/
  else if (ccze_regex_match (reg_size, lword, wlen, offsets, 99) >= 0)
    col = CCZE_COLOR_SIZE;
  /** Version **/
  else if (ccze_regex_match (reg_ver, lword, wlen, offsets, 99) >= 0)
    col = CCZE_COLOR_VERSION;
  /** Time **/
  else if (ccze_regex_match (reg_time, lword, wlen, offsets, 99) >= 0)
    col = CCZE_COLOR_DATE;
  /** Address **/
  else if (ccze_regex_match (reg_addr, lword, wlen, offsets, 99) >= 0)
    col = CCZE_COLOR_ADDRESS;
  /** Number **/
  else if (ccze_regex_match (reg_num, lword, wlen, offsets, 99) >= 0)
    col = CCZE_COLOR_NUMBERS;
  /** Signal **/
  else if (ccze_regex_match (reg_sig, lword, wlen, offsets, 99) >= 0)
    col = CCZE_COLOR_SIGNAL;
  /* Host + IP (postfix) */
  else if (ccze_regex_match (reg_hostip, lword, wlen, offsets, 99) >= 0)
    {
      char *host, *ip;
      size_t hostlen, iplen;
      
      host = strndup (word, strchr (word, '[') - (word));
      hostlen = strlen (host);
      iplen = strlen (word) - hostlen - 1;
      ip = strndup (&word[strlen (host) + 1], iplen);
      ccze_addstr (CCZE_COLOR_HOST, host);
      ccze_addstr (CCZE_COLOR_PIDB, "[");
      ccze_addstr (CCZE_COLOR_HOST, ip);
      ccze_addstr (CCZE_COLOR_PIDB, "]");

      free (host);
      free (ip);
      printed = 1;
    }
  /* Service */
  else if (slookup && getservbyname (lword, NULL))
    col = CCZE_COLOR_SERVICE;
  /* Protocol */
  else if (slookup && getprotobyname (lword))
    col = CCZE_COLOR_PROT;
  /* User */
  else if (slookup && getpwnam (lword))
    col = CCZE_COLOR_USER;
  else
    { /* Good/Bad/System words */
      size_t i;
      
      for (i = 0; i < sizeof (words_bad) / sizeof (char *); i++)
	{
	  if (strstr (lword, words_bad[i]) == lword)
	    col = CCZE_COLOR_BADWORD;
	}
      for (i = 0; i < sizeof (words_good) / sizeof (char *); i++)
	{
	  if (strstr (lword, words_good[i]) == lword)
	    col = CCZE_COLOR_GOODWORD;
	}
      for (i = 0; i < sizeof (words_error) / sizeof (char *); i++)
	{
	  if (strstr (lword, words_error[i]) == lword)
	    col = CCZE_COLOR_ERROR;
	}
      for (i = 0; i < sizeof (words_system) / sizeof (char *); i++)
	{
	  if (strstr (lword, words_system[i]) == lword)
	    col = CCZE_COLOR_SYSTEMWORD;
	}
    }

  if (!printed)
    {
      ccze_addstr (CCZE_COLOR_DEFAULT, pre);
      ccze_addstr (col, word);
      ccze_addstr (CCZE_COLOR_DEFAULT, post);
    }
  
  free (lword);
  free (word);
  free (post);
  free (pre);
}

void
ccze_wordcolor_process (const char *msg, int wcol, int slookup)
{
  char *word;
  char *msg2;

  if (msg)
    msg2 = strdup (msg);
  else
    return;

  if (!wcol)
    {
      ccze_addstr (CCZE_COLOR_DEFAULT, msg);
      free (msg2);
      return;
    }

  if ((strstr (msg, "last message repeated") && strstr (msg, "times")) ||
      (strstr (msg, "-- MARK --")))
    {
      ccze_addstr (CCZE_COLOR_REPEAT, msg);
      free (msg2);
      return;
    }

  word = xstrdup (ccze_strbrk (msg2, ' '));
  if (!word)
    {
      ccze_addstr (CCZE_COLOR_DEFAULT, msg);
      free (msg2);
      free (word);
      return;
    }
  
  do
    {
      ccze_wordcolor_process_one (word, slookup);
      ccze_space ();
    } while ((word = xstrdup (ccze_strbrk (NULL, ' '))) != NULL);

  free (msg2);
  
  return;
}

void
ccze_wordcolor_setup (void)
{


  reg_pre = ccze_regex_compile ("^([`'\".,!?:;(\\[{<]+)([^`'\".,!?:;(\\[{<]\\S*)$");
  reg_post = ccze_regex_compile ("^(\\S*[^`'\".,!?:;)\\]}>])([`'\".,!?:;)\\]}>]+)$");
  reg_host = ccze_regex_compile ("^(((\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})|"
			   "(([a-z0-9-_]+\\.)+[a-z]{2,3})|(localhost)|"
			   "(\\w*::\\w+)+)(:\\d{1,5})?)$");
  reg_hostip = ccze_regex_compile ("^(((\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})|"
			     "(([a-z0-9-_\\.]+)+)|(localhost)|"
			     "(\\w*::\\w+)+)(:\\d{1,5})?)"
			     "\\[");
  reg_mac = ccze_regex_compile ("^([0-9a-f]{2}:){5}[0-9a-f]{2}$");
  reg_email = ccze_regex_compile
    ("^[a-z0-9-_=\\+]+@([a-z0-9-_\\.]+)+(\\.[a-z]{2,4})+");
  reg_email2 = ccze_regex_compile ("(\\.[a-z]{2,4})+$");
  reg_uri = ccze_regex_compile ("^\\w{2,}:\\/\\/(\\S+\\/?)+$");
  reg_size = ccze_regex_compile ("^\\d+(\\.\\d+)?[k|m|g|t]i?b?(ytes?)?");
  reg_ver = ccze_regex_compile ("^v?(\\d+\\.){1}((\\d|[a-z])+\\.)*(\\d|[a-z])+$");
  reg_time = ccze_regex_compile ("\\d{1,2}:\\d{1,2}(:\\d{1,2})?");
  reg_addr = ccze_regex_compile ("^0x(\\d|[a-f])+$");
  reg_num = ccze_regex_compile ("^[+-]?\\d+$");
  reg_sig = ccze_regex_compile ("^sig(hup|int|quit|ill|abrt|fpe|kill|segv|pipe|"
			  "alrm|term|usr1|usr2|chld|cont|stop|tstp|tin|tout|"
			  "bus|poll|prof|sys|trap|urg|vtalrm|xcpu|xfsz|iot|"
			  "emt|stkflt|io|cld|pwr|info|lost|winch|unused)");
  reg_msgid = ccze_regex_compile
    ("^[a-z0-9-_\\.\\$=\\+]+@([a-z0-9-_\\.]+)+(\\.?[a-z]+)+");
}

void
ccze_wordcolor_shutdown (void)
{
  ccze_regex_free (reg_pre);
  ccze_regex_free (reg_post);
  ccze_regex_free (reg_host);
  ccze_regex_free (reg_mac);
  ccze_regex_free (reg_email);
  ccze_regex_free (reg_email2);
  ccze_regex_free (reg_uri);
  ccze_regex_free (reg_size);
  ccze_regex_free (reg_ver);
  ccze_regex_free (reg_time);
  ccze_regex_free (reg_addr);
  ccze_regex_free (reg_num);
  ccze_regex_free (reg_sig);
  ccze_regex_free (reg_hostip);
  ccze_regex_free (reg_msgid);
}
