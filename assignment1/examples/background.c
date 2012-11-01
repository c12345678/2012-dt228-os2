
// vim: set ts=2 sw=2 expandtab:

/*
 * Example code for implementing a child termination cleanup handler
 * for dealing with zombies when running background processes
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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <libgen.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdbool.h>

/*
 * A flag to be set when the child exits
 */
static volatile int terminated = false;

/*
 * The following function will be called asynchronously when a child
 * process terminates
 */
static void sigchld_hdl(int sig)
{
    // Wait for all terminated processes
    while (waitpid(-1, NULL, WNOHANG) > 0)
      ;
    terminated = true;
}

/*
 * Set up the SIGCHLD signal handler
 */
static void sigchld_setup()
{
  struct sigaction act;
     
  memset (&act, 0, sizeof(act));
  act.sa_handler = sigchld_hdl;
         
  if (sigaction(SIGCHLD, &act, 0)) {
     perror ("sigaction");
     exit(1);
  }
}

/*
 * This is the sort of code required to implement a child termination
 * cleanup when the child was originally run in the background but
 * for which a waitpid() was not done. When the child process terminates
 * at a future time, the parent needs to be alerted and do the
 * waitpid() to a cleanup. Otherwise the child process becomes a zombie
 * which is shown as <defunct> in the process listing
 *
 */
int main(int argc, char *argv[])
{
  char *progname = basename(strdup(argv[0]));

  /*
   * The user of this program can specify the command to be run
   * as a command line argument
   */
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <command>\n", progname);
    exit (1);
  }
  char *cmd = argv[1];

  /*
   * Step 1 - Set up to be notified of child termination asynchronous
   * events
   */
  sigchld_setup();

  /*
   * Step 2 - Create a new process to run the command
   */
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    exit(1);
  }

  if (pid == 0) {
    /*
     * Child return after fork. Execute the command
     */
    execl(cmd, cmd, (char *)NULL);

    /*
     * Something went wrong, so exit
     */
    perror("fork");
    _exit(1);
  }

  /*
   * Parent return after fork. Don't do a waitpid to simulate having
   * run the command in the backgroud. Do something else and wait
   * for the asyncrhonous notification
   */
  do {
    sleep(1);
  } while (!terminated);

  /*
   * If we are here, that means the signal handler has run and the
   * child process has been cleaned up
   */

  exit (0);
} 
