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
#include <libgen.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <assert.h>

#include "shared.h"


static char *progname;  // File visible program name string. Set in main() from argv[0]

static void usage(void)
{
  fprintf(stderr, "Usage: %s <msg>\n", progname);
  exit(1);
}

// Sanity check that shared memory is of the correct size and type
static void check(int fd)
{
  struct stat statbuf;
  int magic;

  fstat(fd, &statbuf);
  if (statbuf.st_size != sizeof(shared_t)) {
    fprintf(stderr, "Bad shared segment size\n");
    exit(1);
  }
  lseek(fd, SEEK_SET, 0);
  if (read(fd, &magic, sizeof(magic)) != sizeof(magic) ||
      magic != SHM_MAGIC) {
    fprintf(stderr, "Bad magic number\n");
    exit(1);
  }
}

int main(int argc, char *argv[])
{
  progname = strdup(basename(argv[0]));
  shared_t *sptr;

  if (argc != 2) {
    usage();
  }
  char *msg = argv[1];

  int fd = shm_open(SHM_NAME, O_RDWR, FILE_MODE);
  if (fd < 0) {
    // Means that the server is not likely not yet running
    perror("shm_open");
    usage();
  }
  check(fd);

  sptr = mmap(NULL, sizeof(shared_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (!sptr) {
    perror("mmap");
    exit(1);
  }
  close(fd);

  /*
   * 1. Wait until a free buffer slot becomes available
   */
  sem_wait(&sptr->nempty);

  /*
   * 2. When one does, write the message into the buffer list, advancing
   *    the write index and accounting for write wrap around
   */
  sem_wait(&sptr->mutex);
  strncpy(sptr->msgdata[sptr->windex], msg, sizeof sptr->msgdata[0] - 1);
  sptr->msgdata[sptr->windex][sizeof sptr->msgdata[0]  - 1] = '\0';
  if (++sptr->windex == SHM_NMSG) {
    sptr->windex = 0;
  }
  sem_post(&sptr->mutex);

  /*
   * 3. Signal that there is one more message to read.
   */
  sem_post(&sptr->nstored);

  exit(0);
}
