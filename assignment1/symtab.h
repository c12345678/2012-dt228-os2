
// vim: set ts=2 sw=2 expandtab:

/*
 * Sysmbol table interfaces
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
  SYM_VAR,
  SYM_INTERNAL
} stype_t;

typedef struct symbol {
  char *name;
  void *value;
  stype_t type;
  struct symbol *next;
} symbol_t;

symbol_t *
symtab_set(symbol_t *symtab, char *name, stype_t type, void *value);

symbol_t *
symtab_lookup(symbol_t *symtab, char *name);

void *
symtab_fetch(symbol_t *symtab, char *name, void *value);

int
symtab_size(symbol_t *symtab);

void
symtab_free(symbol_t *symtab);
 
void
symtab_print(symbol_t *symtab);
 
