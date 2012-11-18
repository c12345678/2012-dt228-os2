/*
 * Working implementation of a shared counter between two processes
 * using mmap(). This time we use /dev/zero for anonymous shared
 * memory backing
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

#define SEM_NAME "/mysem"
#define FILE_MODE (S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH)

static char *progname;  // File visible program name string. Set in main() from argv[0]

static void usage(void)
{
  fprintf(stderr, "Usage: %s <#loops>\n", progname);
  exit(1);
}

struct shared {
  int count;
  sem_t mutex;
} shared, *sptr;

int main(int argc, char *argv[])
{
  progname = strdup(basename(argv[0]));

  int i, n;

  if (argc != 2 || sscanf(argv[1], "%d", &n) != 1) {
    usage();
  }

  int fd = open("/dev/zero", O_RDWR);
  if (fd < 0) {
    perror("open");
    usage();
  }
  write(fd, &shared, sizeof(struct shared));

  sptr = mmap(NULL, sizeof(struct shared), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (!sptr) {
    perror("mmap");
    exit(1);
  }
  close(fd);

  sem_init(&sptr->mutex, 1, 1);

  setbuf(stdout, NULL);
  
  pid_t pid = fork();
  if (pid == 0) {
    // Child
    for (i = 0; i < n; i++) {
      sem_wait(&sptr->mutex);
      printf("child: %d\n", sptr->count++);
      sem_post(&sptr->mutex);
    }
    exit(0);
  }

  // Parent
  for (i = 0; i < n; i++) {
    sem_wait(&sptr->mutex);
    printf("parent: %d\n", sptr->count++);
    sem_post(&sptr->mutex);
  }

  waitpid(pid, NULL, 0);
  exit(0);
}
