/* Regex interface shared by CCZE and its plugins. */
#ifndef CCZE_REGEX_H
#define CCZE_REGEX_H 1

#include <stddef.h>

typedef struct ccze_regex ccze_regex_t;

/* Compile errors and allocation failures terminate CCZE with status 2. */
ccze_regex_t *ccze_regex_compile (const char *pattern);
void ccze_regex_free (ccze_regex_t *regex);
/* A pattern owns reusable match state; do not share it between threads.
   Returns a capture count, 0 for insufficient offsets, or a negative PCRE2
   error code. Substrings may only be read after a positive return value. */
int ccze_regex_match (ccze_regex_t *regex, const char *subject, size_t length,
                      int *offsets, size_t count);
/* The caller frees *result with free(). Unset captures yield an empty string;
   missing groups return a negative error code and set *result to NULL. */
int ccze_regex_substring (const char *subject, const int *offsets, int matches,
                          int group, const char **result);

#endif
