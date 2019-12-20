#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

const char *FILENAME = "resources.txt";

void quit()
{
  printf("Use \"take\" as the first argument to take resources, and use \"prov\" to provide resources.\n");
  exit(1);
}

/**
 * Open a file, map it to a memory region with mmap(2), and continuously ask whether
 * to allocate/provide more resources.
 */
int main(int argc, char *argv[])
{
  if (argc < 2) {
    quit();
  }

  int is_taker = strcmp("take", argv[1]) == 0;
  int is_provider = strcmp("prov", argv[1]) == 0;
  const char *verb = is_taker ? "Take" : "Provide";

  if (!is_taker && !is_provider) {
    quit();
  }

  int fd;
  struct stat sb;
  off_t pa_offset;
  char *addr;
  char answer;
  int resource;
  int amount;

  int sem_set_id;
  union semun {
    int val;
  } sem_val;
  int rc;
  struct sembuf sem_op;

  /* Create a semaphore set. */
  sem_set_id = semget(1976, 1, IPC_CREAT | 0600);
  if (sem_set_id == -1) {
    perror("main: semget");
    exit(1);
  }

  /* Initialize a semaphore in the set. */
  sem_val.val = 1;
  rc = semctl(sem_set_id, 0, SETVAL, sem_val);
  if (rc == -1) {
    perror("main: semctl");
    exit(1);
  }

  fd = open(FILENAME, O_RDWR);
  if (fd == -1) {
    printf("Error opening %s\n", FILENAME);
    exit(EXIT_FAILURE);
  }

  /* Get information about the file, including its size. */
  if (fstat(fd, &sb) == -1) {
    close(fd);
    perror("fstat");
    exit(EXIT_FAILURE);
  }

  pa_offset = 0 & ~(sysconf(_SC_PAGE_SIZE) - 1);
  addr = mmap(NULL, sb.st_size - pa_offset, PROT_READ | PROT_WRITE, MAP_SHARED, fd, pa_offset);
  if (addr == MAP_FAILED) {
    close(fd);
    perror("Error mmapping the file");
    exit(EXIT_FAILURE);
  }

  while (1) {
    printf("%s more resources? (y/n) ", verb);
    scanf(" %c", &answer);

    if (answer == 'y') {
      const char *prompt = "Enter the resource number and number of";
      if (is_taker) {
        printf("%s resources needed: ", prompt);
      } else {
        printf("%s additional resources to be provided: ", prompt);
      }
      scanf("%d %d", &resource, &amount);

      /* Restart if a negative resource number or negative resource amount
      is entered. */
      if (resource < 0 || amount < 0) {
        continue;
      }

      /* Subtract/add units from/to resource type. Assuming the resources in
      the file are in order, we can multiply the resource number times 4 and
      then add 2 to get the index of the amount of that resource in the
      character array at the address returned by mmap. */
      int result = (addr[(4 * resource) + 2] - '0');

      if (is_taker) {
        result -= amount;
      } else {
        result += amount;
      }

      if (result >= 0 && result <= 9) {
        sem_op.sem_num = 0;
        sem_op.sem_op = -1;
        sem_op.sem_flg = 0;
        semop(sem_set_id, &sem_op, 1);

        addr[(4 * resource) + 2] = result + '0';

        int sync = msync(addr, sb.st_size - pa_offset, MS_SYNC);

        sem_op.sem_num = 0;
        sem_op.sem_op = 1;
        sem_op.sem_flg = 0;
        semop(sem_set_id, &sem_op, 1);

        if (sync == -1) {
          close(fd);
          perror("Error syncing");
          exit(EXIT_FAILURE);
        } else {
          printf("Synced successfully.\n");
        }
      }
    } else {
      int close_status = close(fd);
      if (close_status == -1) {
        perror("Error closing file descriptor");
        exit(EXIT_FAILURE);
      }
      break;
    }
  }

  return 0;
}
