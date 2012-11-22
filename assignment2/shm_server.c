/*
 * Client server implementation of a producer-consumer for non-cooperating
 * processes using POSIX interfaces
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
#include <stdbool.h>
#include <libgen.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>

#include "shared.h"

static char *progname;  // File visible program name string. Set in main() from argv[0]

static void usage(void)
{
  fprintf(stderr, "Usage: %s <waitsecs>\n", progname);
  exit(1);
}

static volatile bool doexit = false;

static void sigusr1_hdl(int sig)
{
  // Causes main server loop to exit
  doexit = true;
}
 
/*
 * Set up SIGUSR1 handler
 */
static void sigusr1_set()
{
  struct sigaction act;
                  
  memset (&act, 0, sizeof(act));
  act.sa_handler = sigusr1_hdl;
  act.sa_flags |= SA_RESTART;
                     
  if (sigaction(SIGUSR1, &act, 0)) {
    perror ("sigaction");
    exit(1);
  }
}

int main(int argc, char *argv[])
{
  progname = strdup(basename(argv[0]));

  shared_t *sptr;
  int magic = SHM_MAGIC;
  int waitsecs = 0;

  if (argc != 2 || sscanf(argv[1], "%d", &waitsecs) != 1) {
    usage();
  }

  shm_unlink(SHM_NAME);
  int fd = shm_open(SHM_NAME, O_CREAT|O_RDWR|O_EXCL, FILE_MODE);
  if (fd < 0) {
    perror("shm_open");
    exit(1);
  }
  ftruncate(fd, sizeof(shared_t));
  write(fd, &magic, sizeof(magic));

  sptr = mmap(NULL, sizeof(shared_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (!sptr) {
    perror("mmap");
    exit(1);
  }
  close(fd);

  /*
   * TODO
   * Initialize the semaphores to their inital values using sem_init(3)
   */

  setbuf(stdout, NULL);

  sigusr1_set();
  while (!doexit) {

    /*
     * TODO
     * 1. Wait until a message is available to process
     * 2. When a message becomes available, remove it from the message list,
     *    advancing the read index and handling read index wrapping
     * 3. Print the message contents to the screen
     */

    /*
     * IMPORTANT: You must keep the following call to sleep here, otherwise
     * the unit tests won't work properly
     */
    sleep(waitsecs);

    /*
     * 4. Signal that a free message slot is available
     */

  }

  exit(0);
}
