# Operating-Systems-Programming-Assigment-1
A programming assignment to help students gain a better understanding of coding a Producer-Consumer Programming in Linux. 

Reanna Laurell 
Operating Systems – Fall 2022
Programming Assignment #1

Code Files: 
-	producer.c 
-	consumer.c

Description: 
	For this assignment, we had to code a Producer-Consumer problem, where the producer would generate an item and put it into a table, then the consumer would come and pick up the item. The table was only allowed to hold two items at a time and once the table was completed, the producer would wait. When there were no items in the table the consumer would wait. We had to use semaphores to synchronize the work of the producer and consumer as well as add in mutual exclusion. The problem also required the use of threads and shared memory for the usage of the table. All the code was supposed to be done in either C or C++ in Linux/Unix.

Report: 
	For this project I had tried researched many ways on how to use Linux through a VirtualBox on my Windows computer. I was really struggling for a while with this as I could get VirtualBox downloaded but couldn’t get it to do anything and had tried the substitute method in Visual Studios but again had hit an error that I was unable to find a way around. Through some help I was able to finally get the VirtualBox downloaded open and running, but then got stuck again trying to figure out how to navigate putting files into it and opening the files I needed to complete the assignment in Linux. 
	I was able to still create code for this assignment in Visual Studio, meeting most of the requirements that were needed to theoretically complete a Producer-Consumer style problem but lacked the real ability to test it out to see what errors still needed to be fixed and to ensure that the two files were running correctly together, not creating any deadlocks, or errors. 

Code:
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

Summary: 
	Overall, I was able to learn the concepts of the new topics and how to go about starting to implement them, but I was unable to understand the steps I was missing in the timeline of this assignment. Even without the whole components put together though, I do have a better understanding of how to go about coding and implementing a Producer-Consumer problem.  I have also gained the knowledge of what a VirtualBox is and the steps to go through setting one up on a computer, but just lack the understanding of how to complete the next steps once it is up and running. 
