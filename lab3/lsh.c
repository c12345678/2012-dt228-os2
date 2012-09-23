// vim: set ts=2 sw=2 expandtab:

/*
 * A functioning learning shell (lsh) with fork() and a symbol table
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
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <regex.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/wait.h>

#include "symtab.h"
#include "tokenise.h"

#define SZ(t) (sizeof(t) / sizeof(t[0]))

/*
 * You get the following number by running 'getconf ARG_MAX' on the shell
 * command line for your system. This is the value for the Arch Linux 
 * version we're using in dt228/3 OS2
 */
#define ARG_MAX 2097152                

// For simplicity of implementation, this cmd buffer is visible to all functions
static char cmd[ARG_MAX + 1];

static symbol_t *symtab;

// Default prompt string
#define PS1 "lsh>> "

static char *progname;

/*
 * TODO Add internal commands to support the copyright and license commands
 * noted in the following message
 */
static void license()
{
  if (isatty(fileno(stdout))) {
    fprintf(stderr, "%s: Copyright (C) 2012 Brian Gillespie\n"
                    "This program comes with ABSOLUTELY NO WARRANTY; This is free software,\n"
                    "and you are welcome to redistribute it under certain conditions;\n"
                    "Type \"copyright\" or \"license\" for more information.\n", progname);
  }
}

static void init()
{
  /*
   * Initialize the symbol table with some default values
   */
  symtab = symtab_set(symtab, "PS1", SYM_VAR, PS1);
  char cwd[PATH_MAX];
  symtab = symtab_set(symtab, "PWD", SYM_VAR, getcwd(cwd, sizeof(cwd)));
}

// Show the command prompt
static void prompt()
{
  /*
   * We only want to show the prompt string if we are outputting
   * to a terminal and not being redirected to a pipe or file
   */
  if (isatty(fileno(stdout))) {
    fprintf(stdout, symtab_fetch(symtab, "PS1", PS1));
  }
}

// Do exit cleanup
static void exiting()
{
  if (isatty(fileno(stdout))) {
    fprintf(stdout, "\n");
  }
  free(progname);
}

static int external(int argc, char **argv)
{
    int status = -1;
    pid_t pid = fork();
    if (pid < 0) {
      perror("fork");
    } else if (pid == 0) {
      /*
       * Child: Replace this process with the specified command
       * TODO Implement support for backgrounding processes
       */
      execv(argv[0], argv);
      /*
       * Shouldn't get here if above has been successful
       */
      perror(argv[0]);
    } else {
      /*
       * Parent: Wait for the child
       */
      (void)waitpid(pid, &status, 0);
    }
    return status;
}

/*
 * Run an internal or external command
 */
static int dispatch()
{
  int status = -1;
  int count = 0;
  /*
   * TODO Add support for internal commands
   * TODO Add support for relative paths
   * TODO Add support looking up the PATH variable
   * TODO Add support for passing argument list to called commands
   */
  token_t **tokens = tokenise_fetch(cmd, &count);
  if (count) {
    char **argv = (char **)calloc(count + 1, sizeof(char *));
    int i;
    for (i = 0; i < count; i++) {
      argv[i] = strdup(tokens[i]->value);
    }
    argv[count] = NULL;
    status = external(count, argv);
    for (i = 0; i < count; i++) {
      free(argv[i]);
    }
    free(argv);
  }

  tokenise_free(tokens, count);
  return status;
}

/*
 * Check for the valid forms of command input which are:
 *
 * <command> [<arg1> <arg2> ... <argN>]
 * <name>=<value>
 */
static int parse()
{
  regex_t regex;
  const int cflags = REG_EXTENDED|REG_NEWLINE;
  int rc;
  regmatch_t pmatch[4];

  /*
   * Kill the trailing newline character before dispatching
   * TODO Trim any trailing white space
   */
  char *nl = index(cmd, '\n');
  if (nl) {
    *nl = '\0';
  }

  // Look for patterns of the form: <name>=<value>
  (void)regcomp(&regex, "([a-zA-Z]+[a-zA-Z0-9]*)=(.*)", cflags);
  rc = regexec(&regex, cmd, SZ(pmatch), pmatch, 0);
  if (rc == 0) {
    /*
     * Use the tokeniser to get the name and value and add to our symbol table
     */
    int count;
    token_t **tokens = tokenise_fetch(cmd, &count);
    symtab = symtab_set(symtab, tokens[0]->value, SYM_VAR, tokens[2]->value);
    tokenise_free(tokens, count);
  } else {
    /*
     * Not a variable setting so assume it's a command
     */
    dispatch();
  }

  // FIXME
  return 0;
}

static int repl()
{
  int status = 0;

  for (prompt(); fgets(cmd, sizeof(cmd), stdin) != NULL; prompt()) {
    status = parse();

    // TODO Bind to the exit code of the command just run to $?
  }

  return status;
}


int main(int argc, char *argv[]) 
{
  progname = strdup(basename(argv[0]));
  init();
  license();
  repl();
  exiting();
  exit(0);
}
