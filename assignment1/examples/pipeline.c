
// vim: set ts=2 sw=2 expandtab:

/*
 * Example code for implementing a two process pipeline using pipe(2)
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
pid_t run(char **cmdlist, bool inpipeline)
{ 
  pid_t pid = -1;

  if (cmdlist[0] == NULL) {
    /*
     * No more commands in the pipeline
     */
    return pid;
  }

  /*
   * A pipe is a uni-directional communcations channel consisting of
   * two file descriptors. Once opened, data written on the write-side
   * descriptpr pipefds[1] can be read on read-side pipefds[0]
   */
  int pipefds[2];

  if (inpipeline) {
    /*
     * Open the pipe in the parent before forking the child.
     */
    if (pipe(pipefds) < 0) {
      perror("pipe");
      exit(1);
    }
  }

  pid = fork();
  if (pid < 0) {
    perror("fork");
    exit(1);
  }

  if (pid == 0) {
    /*
     * Child process return after fork.
     */

    if (inpipeline) {
      /*
       * Pipe is still open across the fork call.
       * We can immediately close the side we are not going to use.
       */
      close(pipefds[1]);

      /*
       * Duplicate the child's standard input onto the other side
       * of the pipe that the parent process will write to and
       * then close the child side of that pipe as it's no longer
       * needed
       */
      dup2(pipefds[0], STDIN_FILENO);
      close(pipefds[0]);
    }

    /*
     * Before we execute this command, recursively set up the next
     * command in the pipeline if there is one. In this arrangement,
     * the child of a previous command is the parent of its next
     * command and so on until the sequence is exhausted
     *
     * XXX Note that in a real shell this is not how this would be
     * done in practice. Most implementations would organise every
     * one of the pipeline processes to be a direct child process
     * of the top-level shell.
     */
    run(&cmdlist[1], true);

    /*
     * Execute the specified command which, if successful, would
     * overlay this child process just created
     */
    execl(cmdlist[0], cmdlist[0], (char *)NULL);

    /*
     * If we get here then something has gone wrong, so exit
     */
    perror("exec");
    _exit(1);
  }

  /*
   * Parent side return after fork.
   */
  if (inpipeline) {
    /*
     * In keeping with the above, we can close the unused side
     * of the pipe here immediately
     */
    close(pipefds[0]);

    /*
     * Duplicate the parent's standard out onto the writing side
     * so data pops up on the child's read side
     */
    dup2(pipefds[1], STDOUT_FILENO);
    close(pipefds[1]);
  }

  return pid;
}

/*
 * This is the sort of code required to implement a process pipeline
 * for commands of the following type entereed at the shell:
 *
 * $ ls -l | grep arch
 *
 */
int main(int argc, char *argv[])
{
  char *progname = basename(strdup(argv[0]));

  /*
   * The user of this program can specify the two commands to run
   * as command line arguments. For simplicity in this demonstration
   * the commands must be specified by absolute path as we won't
   * be doing any PATH lookups here
   */
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <command1> <command2> ... <commandN>\n", progname);
    exit (1);
  }

  /*
   * For each command that has to be run, fork a child process to do this
   * This is done recursively by the run() function which starts the ball
   * rolling with the first command argument in the null-terminated argv list
   * The parent of the first command (i.e. this program) is not in the
   * pipeline
   */
  run(&argv[1], false);

  exit(0);
} 

