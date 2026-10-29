/* PCRE2 matching with the offset arrays used by CCZE's parsers. */
#include "ccze-regex.h"

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ccze_regex
{
  pcre2_code *code;
  pcre2_match_data *match;
};

ccze_regex_t *
ccze_regex_compile (const char *pattern)
{
  ccze_regex_t *regex = malloc (sizeof (*regex));
  PCRE2_SIZE offset;
  int error;

  if (!regex)
    exit (2);
  regex->code = pcre2_compile ((PCRE2_SPTR)pattern, PCRE2_ZERO_TERMINATED,
                               0, &error, &offset, NULL);
  if (!regex->code)
    {
      PCRE2_UCHAR message[256];
      pcre2_get_error_message (error, message, sizeof (message));
      fprintf (stderr, "ccze: invalid regex at offset %zu: %s\n  %s\n",
               (size_t)offset, (char *)message, pattern);
      free (regex);
      exit (2);
    }
  /* CCZE is single-threaded; reuse each pattern's match buffer per line. */
  regex->match = pcre2_match_data_create_from_pattern (regex->code, NULL);
  if (!regex->match)
    {
      pcre2_code_free (regex->code);
      free (regex);
      exit (2);
    }
  return regex;
}

void
ccze_regex_free (ccze_regex_t *regex)
{
  if (!regex)
    return;
  pcre2_match_data_free (regex->match);
  pcre2_code_free (regex->code);
  free (regex);
}

int
ccze_regex_match (ccze_regex_t *regex, const char *subject, size_t length,
                  int *offsets, size_t count)
{
  PCRE2_SIZE *vector;
  size_t i, copied;
  int matches;

  if (length > INT_MAX)
    return PCRE2_ERROR_BADDATA;
  matches = pcre2_match (regex->code, (PCRE2_SPTR)subject, length, 0, 0,
                         regex->match, NULL);
  if (matches < 0)
    return matches;
  vector = pcre2_get_ovector_pointer (regex->match);
  copied = (size_t)matches * 2;
  if (copied > count)
    copied = count;
  for (i = 0; i < copied; i++)
    offsets[i] = vector[i] == PCRE2_UNSET ? -1 : (int)vector[i];
  return (size_t)matches * 2 > count ? 0 : matches;
}

int
ccze_regex_substring (const char *subject, const int *offsets, int matches,
                      int group, const char **result)
{
  int start, end;
  char *copy;

  *result = NULL;
  if (group < 0 || group >= matches)
    return PCRE2_ERROR_NOSUBSTRING;
  start = offsets[group * 2];
  end = offsets[group * 2 + 1];
  if (start < 0 || end < 0)
    start = end = 0;
  copy = malloc ((size_t)(end - start) + 1);
  if (!copy)
    exit (2);
  memcpy (copy, subject + start, (size_t)(end - start));
  copy[end - start] = '\0';
  *result = copy;
  return end - start;
}
