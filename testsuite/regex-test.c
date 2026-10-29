#include "ccze-regex.h"

#undef NDEBUG
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int
main (void)
{
  ccze_regex_t *regex = ccze_regex_compile ("^(a)?(b)(c)?$");
  int offsets[12], matches;
  const char *part;

  matches = ccze_regex_match (regex, "b", 1, offsets, 12);
  assert (matches == 3);
  assert (offsets[2] == -1 && offsets[3] == -1);
  assert (ccze_regex_substring ("b", offsets, matches, 1, &part) == 0);
  assert (strcmp (part, "") == 0);
  free ((void *)part);
  assert (ccze_regex_substring ("b", offsets, matches, 2, &part) == 1);
  assert (strcmp (part, "b") == 0);
  free ((void *)part);
  assert (ccze_regex_substring ("b", offsets, matches, 3, &part) < 0);
  assert (part == NULL);

  assert (ccze_regex_match (regex, "abc", 3, offsets, 12) == 4);
  assert (ccze_regex_match (regex, "other", 5, offsets, 12) < 0);
  assert (ccze_regex_match (regex, "b", 1, offsets, 2) == 0);
  ccze_regex_free (regex);
  ccze_regex_free (NULL);

  regex = ccze_regex_compile ("^$");
  assert (ccze_regex_match (regex, "", 0, offsets, 12) == 1);
  ccze_regex_free (regex);
  return 0;
}
