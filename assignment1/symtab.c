// vim: set ts=2 sw=2 expandtab:

/*
 * Simple symbol table for the learning shell
 *
 * Copyright (C) 2012  Brian Gillespie
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as publisheod by
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
#include <string.h>

#include "symtab.h"

/*
 * Implemented as push down linked-list. Add new entries at the head
 */

symbol_t *
symtab_set(symbol_t *symtab, char *name, stype_t type, void *value)
{
  symbol_t *symbol = symtab_lookup(symtab, name);
  if (symbol == NULL) {
    symbol = (symbol_t *)malloc(sizeof(symbol_t));
    symbol->name = strdup(name);
    symbol->next = symtab;
    symbol->value = NULL;
    symtab = symbol;
  }
  if (type == SYM_VAR) {
    value = strdup(value);
  }
  if (symbol->value) {
    free(symbol->value);
  }
  symbol->type = type;
  symbol->value = value;
  return symtab;
}

symbol_t *
symtab_remove(symbol_t *symtab, char *name)
{
  symbol_t *symbol = symtab, *prev = NULL;

  while (symbol) {
    if (strcmp(symbol->name, name) == 0) {
      if (prev) {
        prev->next = symbol->next;
        free(symbol);
        break;
      } else {
        symtab = symbol->next;
        free(symbol);
        break;
      }
    }
    prev = symbol;
    symbol = symbol->next;
  }
  return symtab;
}
 
symbol_t *
symtab_lookup(symbol_t *symtab, char *name)
{
  symbol_t *symbol = symtab;

  while (symbol) {
    if (strcmp(symbol->name, name) == 0) {
      return symbol;
    }
    symbol = symbol->next;
  }
  return NULL;
}

void *
symtab_fetch(symbol_t *symtab, char *name, void *value)
{
  symbol_t *symbol = symtab_lookup(symtab, name);
  return symbol ? symbol->value : value;
}

int
symtab_size(symbol_t *symtab)
{
  symbol_t *symbol = symtab;
  int size = 0;

  while (symbol) {
    size++;
    symbol = symbol->next;
  }
  return size;
}

void
symtab_free(symbol_t *symtab)
{
  symbol_t *symbol = symtab;

  while (symbol) {
    free(symbol->name);
    if (symbol-> type == SYM_VAR) {
      free(symbol->value);
    }
    symbol_t *next = symbol->next;
    free(symbol);
    symbol = next;
  }
}
 
void
symtab_print(symbol_t *symtab)
{
  symbol_t *symbol = symtab;

  while (symbol) {
    printf("%s", symbol->name);
    if (symbol->type == SYM_VAR) {
      printf(" => '%s'", (char *)symbol->value);
    }
    printf("\n");
    symbol = symbol->next;
  }
}

#ifdef SYMTAB_TEST

symbol_t *symtab;
#include <stdarg.h>

static void fail(const char *fmt, ...)
{
  va_list argp;
  va_start(argp, fmt);
  vfprintf(stderr, fmt, argp);
  va_end(argp);
  exit(1);
}

int main(int argc, char *argv[])
{
  // Add a symbol
  char *name = "PATH";
  char *value;
  int size;

  symtab = symtab_set(symtab, name, SYM_VAR, "/bin:/usr/bin:/usr/local/bin");

  // Get it back
  symbol_t *symbol = symtab_lookup(symtab, name);
  if (symbol == NULL) {
    fail("Cound not lookup symbol name='%s'\n", name);
  }

  name = "PS1";
  value = "lsh>> ";
  symtab = symtab_set(symtab, name, SYM_VAR, value);
  // Fetch a value value directly
  char *ps1 = symtab_fetch(symtab, name, "notfound");
  if (strcmp(ps1, value) != 0) {
    fail("Cound not fetch symbol value='%s'\n", value);
  }

  // Check the size
  size = symtab_size(symtab);
  if (size != 2) {
    fail("Expected size to be 2 but got '%d'\n", size);
  }

  // Remove a symbol
  symtab = symtab_remove(symtab, name);
  value = symtab_fetch(symtab, name, "notfound");
  if (strcmp(value, "notfound") != 0) {
    fail("Remove of '%s' failed\n", name);
  }
  size = symtab_size(symtab);
  if (size != 1) {
    fail("Expected size to be 1 but got '%d'\n", size);
  }

  symtab_print(symtab);
  exit(0);
}
#endif
