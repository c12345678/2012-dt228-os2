// vim: set ts=2 sw=2 expandtab:

/*
 * Demonstration of the fork(2) system call
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
#include <unistd.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/wait.h>

static char *progname;

static void exiting()
{
  free(progname);
}

int main(int argc, char *argv[]) 
{
  progname = strdup(basename(argv[0]));

  printf("parent: before fork()\n");

  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
  } else if (pid == 0) {
    // Child
    printf("child: after fork()\n");
    printf("child: sleeping...\n");
    sleep(2);
    printf("child: exiting()\n");
  } else {
    // Parent
    printf("parent: after fork(): pid=%d\n", pid);
    // Wait for child to terminate
    int status = 0;
    pid = waitpid(pid, &status, 0);
    printf("parent: after waitpid(): pid=%d status=0x%04x\n", pid, status);
  }

  exiting();
  exit(0);
}
