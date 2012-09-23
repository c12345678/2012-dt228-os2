
// vim: set ts=2 sw=2 expandtab:

/*
 * Tokenise a string into a list of strings suitable for use as an
 * argv[] vector
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

typedef enum {
  T_UNDEF,
  T_ARG,
  T_PIPE,
  T_BACKGROUND,
  T_FROMFILE,
  T_TOFILE,
  T_ASSIGN,
  T_ENDSTMT,
} ttype_t;

typedef struct {
  char *value;
  ttype_t type;
} token_t;

token_t **
tokenise_fetch(const char *src, int *count);

void
tokenise_free(token_t *tokens[], int count);

void
tokenise_print(token_t *tokens[], int count);
