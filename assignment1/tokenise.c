// vim: set ts=2 sw=2 expandtab:

/*
 * Tokenise a string into a list of typed tokens suitable for subsequent
 * use as an argv[] vector. There is virtually no input error handling.
 *
 * Copyright (C) 2012  Brian Gillespie
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "tokenise.h"

typedef enum {
  S_START,
  S_DQUOTE,
  S_SQUOTE,
  S_TOKEN,
  S_SPECIAL
} state_t;

#ifdef LSH_ENABLE_USERVARS
#define SPECIAL_CHARS(c)  ((c) == '|' || (c) == '&' || (c) == '<' || (c) == '>' || (c) == '=')  
#else
#define SPECIAL_CHARS(c)  ((c) == '|' || (c) == '&' || (c) == '<' || (c) == '>')  
#endif

#define SPECIAL(c)        ((c) != '\\' && SPECIAL_CHARS(c))

#define TERMINAL(c, ep)   (((c) == '\0' || isspace(c) || SPECIAL(c)) && (ep) == NULL)

/*
 * Return a dynamically allocated array of tokens and update the count
 * with the number of fetched tokens if provided by the caller
 */
token_t **
tokenise_fetch(const char *src, int *count)
{
  token_t **tokens = NULL;
  int tmp_count;
  if (!count) {
    count = &tmp_count;
  }
  *count = 0;
  char *cp = (char *)src, *sp = NULL, *ep = NULL, *value = NULL;
  state_t state = S_START;
  ttype_t type = T_UNDEF;

  while (*cp || state != S_START) {
    switch (state) {
    case S_START:
      if (isspace(*cp)) {
        // Skip over white space
        cp++;
      } else if (*cp == '\\' && ep == NULL) {
        // Escaping next character
        ep = cp++;
      } else if (*cp == '"' && ep == NULL) {
        // Start of a dquote string. Mark the first character
        sp = ++cp;
        state = S_DQUOTE;
      } else if (*cp == '\'' && ep == NULL) {
        // Start of a squote string. Mark the first character
        sp = ++cp;
        state = S_SQUOTE;
      } else if (SPECIAL(*cp)) {
        // Ordinary token
        sp = cp++;
        state = S_SPECIAL;
      } else {
        // Ordinary token
        sp = cp++;
        state = S_TOKEN;
      }
      break;
    case S_DQUOTE:
      // End of a dquote string.
      if ((*cp == '"' && ep == NULL) || *cp == '\0') {
        value = strndup(sp, cp - sp);
        type = T_ARG;
        state = S_START;
      }
      if (*cp) { cp++; }
      break;
    case S_SQUOTE:
      // End of a squote string.
      if ((*cp == '\'' && ep == NULL) || *cp == '\0') {
        value = strndup(sp, cp - sp);
        type = T_ARG;
        state = S_START;
      }
      if (*cp) { cp++; }
      break;
    case S_TOKEN:
      if (TERMINAL(*cp, ep)) {
        // End non-spcecial token processing
        value = strndup(sp, cp - sp);
        type = T_ARG;
        if (SPECIAL(*cp)) {
          // Push it back to be handled in the START state
          if (cp != src) {
            cp--;
          }
        }
        state = S_START;
      }
      if (*cp) { cp++; }
      break;
    case S_SPECIAL:
      value = strndup(sp, 1);
      switch (*sp) {
      case '|': type = T_PIPE; break;
      case '&': type = T_BACKGROUND; break;
      case '<': type = T_FROMFILE; break;
      case '>': type = T_TOFILE; break;
#ifdef LSH_ENABLE_USERVARS
      case '=': type = T_ASSIGN; break;
#endif
      case ';': type = T_ENDSTMT; break;
      default:  type = T_UNDEF; break;  // Error
      }
      state = S_START;
      break;
    }
    if (value) {
      assert(state == S_START);
      assert(type != T_UNDEF);
      int n = *count;
      tokens = (token_t **)realloc(tokens, ++*count * sizeof(token_t *));
      tokens[n] = malloc(sizeof(token_t));
      tokens[n]->value = value;
      tokens[n]->type = type;
      value = NULL;
      type = T_UNDEF;
    }
  }
  return tokens;
}

void
tokenise_free(token_t *tokens[], int count)
{
  if (tokens) {
    int i;
    for (i = 0; i < count; i++) {
      token_t *token = tokens[i];
      if (token) {
        if (token->value) {
          free(token->value);
        }
      }
      free(token);
    }
    free(tokens);
  }
}

void
tokenise_print(token_t *tokens[], int count)
{
  int i;
  for (i = 0; i < count; i++) {
    printf("token[%d]: value='%s', type=%d\n", i, tokens[i]->value, tokens[i]->type);
  }
}



#ifdef TOKENISE_TEST

int main(int argc, char *argv[])
{
  int count;
  token_t **tokens = tokenise_fetch("export VAR='123'; ps -ef|egrep '(root | arch)'|sort -u&", &count);
  tokenise_print(tokens, count);
  tokenise_free(tokens, count);
  exit(0);
}
#endif
