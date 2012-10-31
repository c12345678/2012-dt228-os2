
// vim: set ts=2 sw=2 expandtab:

/*
 * Example code for implementing a multi-process pipeline using pipe(2)
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
#include <stdbool.h>
#include <sys/wait.h>

/*
 * Fork a child to run the specificied command and pipe our
 * (parent) standard output into its (child) standard input
 */
pid_t run(char *cmd, int writefd, int readfd, int closefd)
{ 
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    exit(1);
  }

  if (pid == 0) {
    /*
     * Child process return after fork.
     */
    close(closefd);

    /*
     * Duplicate the child's standard input onto the other side
     * of the pipe that the previous process will write to and
     * then close the child side of that pipe as it's no longer
     * needed
     */
    if (readfd != STDIN_FILENO) {
      dup2(readfd, STDIN_FILENO);
      close(readfd);
    }

    /*
     * Duplicate the child's standard out onto the writing side
     * so data pops up on the next process's read side
     */
    if (writefd != STDOUT_FILENO) {
      dup2(writefd, STDOUT_FILENO);
      close(writefd);
    }

    /*
     * Execute the specified command which, if successful, would
     * overlay this child process just created and never return
     */
    execl(cmd, cmd, (char *)NULL);

    /*
     * If we get here then something has gone wrong, so exit
     */
    perror("exec");
    _exit(1);
  }
  /*
   * Parent side return after fork.
   */

  if (setpgid(pid, 0) < 0) {
    perror("setpgid");
    return -1;
  }

  return pid;
}

/*
 * This is the sort of code required to implement a process pipeline
 * for commands of the following type entereed at the shell:
 *
 * $ ls -l | grep arch
 *
 * The following approach is more like how real shells work in that
 * all newly created command processes are direct children of the
 * parent
 */
int main(int argc, char *argv[])
{
  char *progname = basename(strdup(argv[0]));
  int i;

  /*
   * The user of this program can specify the two commands to run
   * as command line arguments. For simplicity in this demonstration
   * the commands must be specified by absolute path as we won't
   * be doing any PATH lookups here
   */
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <command1> <command2> [ ... <commandN> ]\n", progname);
    exit (1);
  }
  int count = argc - 1;

  /*
   * A pipe is a uni-directional communcations channel consisting of
   * two file descriptors. Once opened, data written on the write-side
   * descriptpr pipefds[i][1] can be read on read-side pipefds[i][0]
   * We construct an array to hold descriptor pairs, one pair for each
   * pair of command to be run. In other words, for N processes, we
   * require N - 1 pipes to be created
   */
  int pipefds[count - 1][2];

  pid_t pidlist[count];
  for (i = 0; i < count - 1; i++) {
    /*
     * Open the pipe in the parent before forking the child.
     */
    if (pipe(pipefds[i]) < 0) {
      perror("pipe");
      exit(1);
    }


    /*
     * Special case for first process in the pipeline which
     * reads from the parent's standard input
     */
    int readfd = (i == 0) ? STDIN_FILENO : pipefds[i - 1][0];

    /*
     * Do the fork and get the child's pid
     */
    pidlist[i] = run(argv[i + 1], pipefds[i][1], readfd, pipefds[i][0]);

    /*
     * Now that the child processes are forked and wired up, we can
     * close the parent sides of the pipes we no longer need
     */
    close(pipefds[i][1]);
    if (i > 0) {
      close(pipefds[i - 1][0]);
    }
  }
  /*
   * Now run the last process in the sequence and clean up
   */
  pidlist[i] = run(argv[i + 1], STDOUT_FILENO, pipefds[i - 1][0], pipefds[i - 1][1]);
  close(pipefds[i - 1][0]);
  close(pipefds[i - 1][1]);

  /*
   * Wait for all child processes to terminate
   */
  for (i = 0; i < count; i++) {
    pid_t pid = pidlist[i];
    waitpid(pid, NULL, 0);
  }

  exit(0);
} 

