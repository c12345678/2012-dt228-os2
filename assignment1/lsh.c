// vim: set ts=2 sw=2 expandtab:

/*
 * A functioning learning shell (lsh)
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
#include <unistd.h>
#include <libgen.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <regex.h>

#include "symtab.h"
#include "tokenise.h"
#include "internal.h"

#define SZ(t) (sizeof(t) / sizeof(t[0]))

/*
 * You get the following number by running 'getconf ARG_MAX' on the shell
 * command line for your system. This is the value for the Arch Linux 
 * version we're using in this module
 */
#ifndef ARG_MAX
#define ARG_MAX 2097152                
#endif

/*
 * For simplicity of implementation, this cmd buffer is visible to all functions
 * but you should use the macros below to update and read from the buffer itself
 */
static char buffer[ARG_MAX + 1];
static char *cmd = buffer;        // Initially the same as buffer but can change
#define TRIM(bp)   bp = trim(bp)
#define RESET(bp)  bp = buffer
#define READ(bp)   fgets(bp, sizeof(buffer), stdin)

/*
 * The 'current' head of the symbol table
 */
static symbol_t *symtab;

#ifndef PS1
// Default prompt string if not provided
#define PS1 "lsh>> "
#endif

#ifdef LSH_ENABLE_EXTERNAL
// Default PATH
#ifndef PATH
#define PATH "/usr/local/bin:/bin:/usr/bin"
#endif
#endif

static char *progname;

/*
 * TODO Add internal commands to support the copyright and license commands
 * noted in the following message
 */
static void license()
{
  if (isatty(STDOUT_FILENO)) {
    fprintf(stderr, "%s: Copyright (C) 2012 Brian Gillespie\n"
                    "This program comes with ABSOLUTELY NO WARRANTY; This is free software,\n"
                    "and you are welcome to redistribute it under certain conditions;\n"
                    "Type \"copyright\" or \"license\" for more information.\n", progname);
  }
}

// Do exit cleanup
static void exiting(void)
{
  if (isatty(STDOUT_FILENO)) {
    fprintf(stdout, "Bye\n");
  }
  free(progname);
}

static void halt(int argc, char **argv)
{
  exiting();
  exit(0);
}

static void init(void)
{
  symtab = symtab_set(symtab, "PS1", SYM_VAR, PS1);
#ifdef LSH_ENABLE_EXTERNAL
  symtab = symtab_set(symtab, "PATH", SYM_VAR, PATH);
#endif /* LSH_ENABLE_EXTERNAL */

  symtab = symtab_set(symtab, "exit", SYM_INTERNAL, halt);
#ifdef LSH_ENABLE_CD
  symtab = symtab_set(symtab, "cd", SYM_INTERNAL, lsh_cd);
#endif /* LSH_ENABLE_CD */
}

// Show the command prompt
static void prompt(void)
{
  /*
   * We only want to show the prompt string if we are outputting
   * to a terminal and not being redirected to a pipe or file
   */
  if (isatty(STDOUT_FILENO)) {
    fprintf(stdout, "%s", (char *)symtab_fetch(symtab, "PS1", PS1));
  }
}

/*
 * Trim (in place) white space from the start and end of a string
 */
static char *
trim(char *str)
{
  char *start, *end;
  for (start = str; isspace(*start); start++)
    ;
  for (end = &str[strlen(str) - 1]; isspace(*end); end--)
    *end = '\0';
  return start;
}

static int
internal(int argc, char *argv[])
{
  char *name = argv[0];
  symbol_t *symbol = symtab_lookup(symtab, name);
  int status = -1;

  if (symbol && symbol->type == SYM_INTERNAL) {
    status = ((internal_t)symbol->value)(argc, argv);
  } else {
#if !defined(LSH_ENABLE_EXTERNAL)
    lsh_not_impl(argv[0]);
#endif
  }
  return status;
}

#ifdef LSH_ENABLE_EXTERNAL
/*
 * Search the specified path to locate an executable binary
 */
static char
*path_lookup(const char *path, const char *binary)
{
  char *p = strdup(path);
  char *saveptr;
  char *token;
  int binlen = strlen(binary);
  char *which = (char *)path;

  for (token = strtok_r(p, ":", &saveptr);
       token != NULL;
       token = strtok_r(NULL, ":", &saveptr)) {
    token = trim(token);
    char *execpath = malloc(strlen(token) + binlen + 2);
    strcpy(execpath, token);
    strcat(execpath, "/");
    strcat(execpath, binary);
    struct stat statbuf;
    /*
     * Although we check for the file's existance here, it is
     * not a guarantee, if found, that it will still exist by
     * the time we attempt to execute it. For the same reason,
     * there is no point in checking whether it is executable
     * as that might change too.
     */
    if (stat(execpath, &statbuf) == 0) {
      which = execpath;
      break;
    }
    free(execpath);
  }
  free(p);
  return which;
}

static int
external(int argc, char **argv)
{
    int rc = -1;
    char *path;
    extern char **environ;

    pid_t pid = fork();
    if (pid < 0) {
      perror("fork");
    } else if (pid == 0) {
      /*
       * If argc[0] contains no '/' character then we must lookup
       * the PATH variable to find its path. Otherwise we can just
       * call exec
       */
      if (index(argv[0], '/') == NULL) {
        path = path_lookup(symtab_fetch(symtab, "PATH", PATH), argv[0]);
      } else {
        path = argv[0];
      }
      execve(path, argv, environ);
      /*
       * Shouldn't get here if above has been successful
       */
      if (path && path != argv[0]) {
        free(path);
      }
      perror(argv[0]);
      /*
       * The child must exit here if the exec failed, otherwise we would
       * have another shell running
       */
      exit(errno);
    } else {
      int stat_loc;
      pid = waitpid(pid, &stat_loc, 0);
      if (pid != -1 ) {
        // Get the child exit code
        rc = WEXITSTATUS(stat_loc);
      }
    }

    return rc;
}
#endif /* LSH_ENABLE_EXTERNAL */

/*
 * Run an internal or external command
 */
static int
dispatch(void)
{
  int status = -1;
 
  if (*cmd) {
    /*
     * Build the argument vector argv
     */
    int argc;
    token_t **tokens = tokenise_fetch(cmd, &argc);
    char **argv = calloc(argc + 1, sizeof(char *));
    int i;
    for (i = 0; i < argc; i++) {
      argv[i] = strdup(tokens[i]->value);
    }
    argv[argc] = NULL;
    // See if there is an internal command of this name and run that
    status = internal(argc, argv);
#ifdef LSH_ENABLE_EXTERNAL
    /*
     * If an internal command of that name does not exist then see if
     * we could run an external command of the same name
     */ 
    if (status < 0) {
      status = external(argc, argv);
    }
#endif /* LSH_ENABLE_EXTERNAL */
    for (i = 0; i < argc; i++) {
      free(argv[i]);
    }
    free(argv);
  }

  return status;
}

/*
 * Check for the valid forms of command input which are:
 *
 * <command> [<arg1> <arg2> ... <argN>]
 * <name>=<value>
 */
static int
parse(void)
{
  TRIM(cmd);

#ifdef LSH_ENABLE_USERVARS
  regex_t regex;
  const int cflags = REG_EXTENDED|REG_NEWLINE;
  int rc;
  regmatch_t pmatch[4];

  // Look for patterns of the form: <name>=<value>
  (void)regcomp(&regex, "([a-zA-Z]+[a-zA-Z0-9]*)=(.*)", cflags);
  rc = regexec(&regex, cmd, SZ(pmatch), pmatch, 0);
  if (rc == 0) {
    int count = 0;
    token_t **tokens = tokenise_fetch(cmd, &count);
    symtab = symtab_set(symtab, tokens[0]->value, SYM_VAR, tokens[2]->value);
    tokenise_free(tokens, count);
  } else {
    /*
     * Not a variable setting so assume it's a command
     */
#endif
    dispatch();
#ifdef LSH_ENABLE_USERVARS
  }
#endif

  RESET(cmd);

  // FIXME
  return 0;
}

static int repl(void)
{
  int status = 0;

  for (prompt(); READ(cmd) != NULL; prompt()) {
    status = parse();

    // TODO Bind to the exit code of the command just run to $?
  }
  if (isatty(STDOUT_FILENO)) fprintf(stdout, "\n");

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
