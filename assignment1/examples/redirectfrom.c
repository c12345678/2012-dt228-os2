
// vim: set ts=2 sw=2 expandtab:

/*
 * Example code for implementing standard input redirection from a file
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

/*
 * This is the sort of code required to implement input redirection from
 * a file for commands of the following type entereed at the shell:
 *
 * $ cat < input.txt
 *
 */
int main(int argc, char *argv[])
{
  char *progname = basename(strdup(argv[0]));

  /*
   * The user of this program can specify the input file name as a
   * command line argument
   */
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <filename>\n", progname);
    exit (1);
  }
  char *infile = argv[1];

  /*
   * Step 1 - Open the file for reading.
   */
  int fd = open(infile, O_RDONLY);
  if (fd < 0) {
    perror("open");
    exit (1);
  }

  /*
   * Step 2 - Duplicate the standard input onto this new descriptor
   * just opened
   */
  if (dup2(fd, STDIN_FILENO) < 0) {
    perror("dup2");
    exit (1);
  }

  /*
   * Step 3 - We can close the original file descriptor now that the
   * standard input has been duplicated onto the associated file
   */
  close(fd);

  /*
   * Step 4 - Now just display the message. It will come from the file
   * rather than the keyboard (which would ordinarily be the default)
   */
  int c;
  while ((c = getchar()) != EOF) {
    putchar(c);
  }

  /*
   * When this program exits, the standard input will be closed
   * automatically, this closing our file too
   */

  exit (0);
} 
