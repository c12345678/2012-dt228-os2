
// vim: set ts=2 sw=2 expandtab:

/*
 * Example code for implementing standard output redirection to a file
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
 * This is the sort of code required to implement output redirection to
 * a file for commands of the following type entereed at the shell:
 *
 * $ echo "Hello, world" > output.txt
 *
 */
int main(int argc, char *argv[])
{
  char *progname = basename(strdup(argv[0]));

  /*
   * The user of this program can specify the message text and the 
   * output file name as a command line arguments
   */
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <message> <filename>\n", progname);
    exit (1);
  }
  char *message = argv[1];
  char *outfile = argv[2];

  /*
   * Step 1 - Open the file for writing. Truncate it it already exists
   * but create it anew if it doesn't. Make it readable and writeable
   * by the owner
   */
  int fd = open(outfile, O_CREAT|O_RDWR|O_TRUNC, S_IWUSR|S_IRUSR);
  if (fd < 0) {
    perror("open");
    exit (1);
  }

  /*
   * Step 2 - Duplicate the standard output onto this new descriptor
   * just opened
   */
  if (dup2(fd, STDOUT_FILENO) < 0) {
    perror("dup2");
    exit (1);
  }

  /*
   * Step 3 - We can close the original file descriptor now that it has
   * standard output has been duplicated onto the associated file
   */
  close(fd);

  /*
   * Step 4 - Now just display the message. It will go to the file rather
   * than the screen (which would ordinarily be the default)
   */
  printf("%s\n", message);

  /*
   * When this program exits, the standard output will be closed
   * automatically, this closing our file too
   */

  exit (0);
} 
