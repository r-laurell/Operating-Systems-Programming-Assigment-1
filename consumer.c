/*Consumer File : the consumers job in the producer consumer problem is to consuming the data 
that the producer is placing into the buffer one at a time once the producer has moved onto the 
nnext spot in the buffer. Waiting while there is nothing for it to do until the producer has finished
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

    int
    main(int argc, char *argv[]){
            if (argc != 2) {
                fprintf(stderr, "Usage: %s /shm-path\n", argv[0]);
                exit(EXIT_FAILURE);
            }

               char *shmpath = argv[1];

            //Creates a shared memory object and set its size to the size of the structure

            int fd = shm_open(shmpath, O_CREAT | O_EXCL | O_RDWR,S_IRUSR | S_IWUSR);
            if (fd == -1) errExit("shm_open");

            if (ftruncate(fd, sizeof(struct shmbuf)) == -1) errExit("ftruncate");

               //Maps the object into the caller's address space

               struct shmbuf *shmp = mmap(NULL, sizeof(*shmp),PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);
                if (shmp == MAP_FAILED)errExit("mmap");

               //Initialize semaphores as process-shared using a value 0

                if (sem_init(&shmp->sem1, 1, 0) == -1) errExit("sem_init-sem1");
                if (sem_init(&shmp->sem2, 1, 0) == -1)errExit("sem_init-sem2");

               //Waits for 'sem1' to be posted by producer thread before touching shared memory

                if (sem_wait(&shmp->sem1) == -1) errExit("sem_wait");

               //Check to see how many items are on the table and either take one item off the table, ir if no items are left change the semphore to
                if(shmp->itemCT == 0){
                    //Convert data in shared memory into upper case
                    for (int j = 0; j < shmp->cnt; j++)shmp->buf[j] = toupper((unsigned char) shmp->buf[j]);
                    //Post 'sem2' to notify the producer thread that it can now access the modified data in shared memory
                    if (sem_post(&shmp->sem2) == -1)errExit("sem_post");
                    exit(EXIT_SUCCESS);
                }
                //If items remain on the table the consumer takes one item from the table
                else{
                    shmp->itemCT = shmp->itemCT - 1;
                }
            
            //Unlinks the shared memory object
            shm_unlink(shmpath);

            exit(EXIT_SUCCESS);
        }