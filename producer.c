/* Producer file: the producers job in the producer consumer problem is to generate data
to then add into the shared buffer for the consumer to consumer once it has procceeded to 
the next open spot in the buffer and repeat the process over again.
*/


#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                            } while (0)

#define BUF_SIZE 1024   //Maximum size that the exchanged string can be

//Defining the Stucture
       struct shmbuf {
               sem_t  sem1;            //POSIX unnamed semaphore
               sem_t  sem2;            //POSIX unnamed semaphore 
               size_t cnt;             //Number of bytes used in 'buf' 
               char   buf[BUF_SIZE];   //Data being transferred 
               int itemCT;             //Contains the count of the number of items in the table 
       };

#include <ctype.h>
#include "pshm_ucase.h"

       
        int main(int argc, char *argv[]){
              if (argc != 2) {
                     fprintf(stderr, "Usage: %s /shm-path\n", argv[0]);
              exit(EXIT_FAILURE);
              }

              char *shmpath = argv[1];

              //Creates a shared memory object and set its size to the size of the structure

              int fd = shm_open(shmpath, O_CREAT | O_EXCL | O_RDWR,S_IRUSR | S_IWUSR);
              if (fd == -1)errExit("shm_open");

              if (ftruncate(fd, sizeof(struct shmbuf)) == -1)errExit("ftruncate");

              //Maps the object into the caller's address space

              struct shmbuf *shmp = mmap(NULL, sizeof(*shmp),PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);
              if (shmp == MAP_FAILED)errExit("mmap");

              //Initialize semaphores as process-shared using a value 0

              if (sem_init(&shmp->sem1, 1, 0) == -1)errExit("sem_init-sem1");
              if (sem_init(&shmp->sem2, 1, 0) == -1)errExit("sem_init-sem2");

              //Wait for 'sem1' to be posted by consumer thread before touchin shared memory

              if (sem_wait(&shmp->sem1) == -1)errExit("sem_wait");

              //Convert data in shared memory into upper case

              for (int j = 0; j < shmp->cnt; j++)shmp->buf[j] = toupper((unsigned char) shmp->buf[j]);
              
              //If the producer has found that it has the semaphore but the table is not empty, exit with an item count error

              if(shmp->itemCT > 0){
              errExit("item cnt err");
              }
              //If the item count error is zero, set the table with two more items

              else{           
              shmp->itemCT = 2; //Put two items on the table
              }
              //Post 'sem2' to tell the consumer thread that it can now access the modified data in shared memory

              if (sem_post(&shmp->sem2) == -1)errExit("sem_post");

              //Unlink the shared memory object
              shm_unlink(shmpath);

              exit(EXIT_SUCCESS);
       }