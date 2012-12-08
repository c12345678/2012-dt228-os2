/*
 * Don't change any of these definitions
 */
#define SHM_NAME    "/shm_dt228_os2"
#define SHM_MAGIC   0xbadcafe
#ifndef SHM_NMSG
#define SHM_NMSG    16
#endif
#ifndef SHM_MSGSIZE
#define SHM_MSGSIZE 256
#endif

#define FILE_MODE (S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH)

#define sleep(x)  (void)usleep(x)

/*
 * You may alter this structure as you require to but
 * leave the magic number as the first field
 */
typedef struct shared {
  int magic;
  sem_t mutex;
  sem_t nempty;
  sem_t nstored;
  int rindex;
  int windex;
  char msgdata[SHM_NMSG][SHM_MSGSIZE];
} shared_t;

