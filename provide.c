/**
 * provide.c: Prompts the user to provide more resources in the parent
 * process and prints out status information every ten seconds in a child
 * process.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

const char *FILENAME = "resources.txt";

/**
 * Print out the page size, current state of resources, and current status of
 * pages in the mapped memory region.
 *
 * @param *addr - the address of the memory mapping
 * @param length - the size of the mapped memory space
 */
void report(char *addr, size_t length)
{
  int i;
  int page_size = getpagesize();
  int num_pages = (length + page_size - 1) / page_size;
  unsigned char *resident = malloc(num_pages);
  int mincore_status = mincore(addr, length, resident);
  if (mincore_status == -1) {
    perror("mincore");
    exit(EXIT_FAILURE);
  }
  printf("\n\nREPORT\n");
  printf("Page size is: %d\n", page_size);
  printf("Number of pages needed: %d\n", num_pages);
  printf("Current state of resources:\n%s", addr);
  printf("Status of pages in main memory:\n");
  for (i = 0; i < num_pages; i++) {
    char *location = resident[i] == 1 ? "Resident" : "Non-resident";
    printf("Page %d: %s\n", i, location);
  }
  free(resident);
}

/**
 * Open a file, map it to a memory region with mmap, and create the parent
 * and child processes.
 */
int main(void) {
  int fd;
  struct stat sb;
  off_t pa_offset;
  char *addr;
  char provide_more;
  int resource;
  int num_to_add;

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

  if (fork() == 0) {
    /* The child is the reporter. */
    while (1) {
      sleep(10);
      report(addr, sb.st_size - pa_offset);
    }
  } else {
    while (1) {
      printf("Provide more resources? (y/n) ");
      scanf(" %c", &provide_more);
      if (provide_more == 'y') {
        printf("Enter the resource number and number of additional resources to be provided: ");
        scanf("%d %d", &resource, &num_to_add);

        /* Restart if a negative resource number or negative resource amount
        is entered. */
        if (resource < 0 || num_to_add < 0) {
          continue;
        }

        /* Add units to resources. Assuming the resources in the file are in
        order, we can multiply the resource number times 4 and then add 2 to
        get the index of the amount of that resource in the character array at
        the address returned by mmap. */
        int num_resources_after_adding = (addr[(4 * resource) + 2] - '0') + num_to_add;
        if (num_resources_after_adding <= 9) {

          sem_op.sem_num = 0;
          sem_op.sem_op = -1;
          sem_op.sem_flg = 0;
          semop(sem_set_id, &sem_op, 1);

          addr[(4 * resource) + 2] = num_resources_after_adding + '0';

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
  }

  return 0;
}
