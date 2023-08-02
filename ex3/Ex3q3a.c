#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <semaphore.h>

#define MAX_STRINGS 10
#define key 1234
#define MAX_COMMAND_LENGTH 128

int main() {
    char command[MAX_COMMAND_LENGTH +1];


    // Create a shared memory segment
    int shm_id;

    shm_id = shmget(key, 1280, IPC_CREAT | IPC_EXCL | 0666);
    if (shm_id == -1) {
        perror("shmget failed ");
        exit(EXIT_FAILURE);
    }

    // Attach the shared memory segment to the process's address space
    char *sharedMemory = (char *) shmat(shm_id, NULL, 0); //TODO
    if (sharedMemory == (char *) -1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    sem_t *mutex;
    sem_t *full;
    sem_t *empty;
    sem_close(mutex);
    sem_close(full);
    sem_close(empty);
    sem_unlink("/my_mutex1");
    sem_unlink("/my_full1");
    sem_unlink("/my_empty1");

    mutex = sem_open("/my_mutex1", O_CREAT, 0666, 1);
    if (mutex == SEM_FAILED) {
        perror("sam open failed");
        exit(EXIT_FAILURE);
    }

    full = sem_open("/my_full1", O_CREAT, 0666, 0);
    if (full == SEM_FAILED) {
        perror("sam open failed");
        exit(EXIT_FAILURE);
    }
    empty = sem_open("/my_empty1", O_CREAT, 0666, 10);
    if (empty == SEM_FAILED) {
        perror("sam open failed");
        exit(EXIT_FAILURE);
    }

    // Read strings until "END" is entered
    int i = 0;


    while (1) {
        command[MAX_COMMAND_LENGTH] = '\0';
        if (!fgets(command, sizeof(command), stdin)) {
            perror("fgets failed");
            break;
        }
        if(command[MAX_COMMAND_LENGTH] != '\0')
        {
            printf("ERR");
            continue;
        }

        if (i == 10)
            i = 0;

//
        sem_wait(empty);
        sem_wait(mutex);
        // Copy the command to shared memory
        strcpy(&sharedMemory[i*MAX_COMMAND_LENGTH] , command);
        i++;
        sem_post(mutex);
        sem_post(full);
//
        if (strcmp(command, "END") == 0)
            break;
        if (strcmp(command, "END\n") == 0)
            break;


        usleep(100000);
    }



    if (shmdt(sharedMemory) == -1) {
        perror("shmdt");
        return 1;
    }


    sem_close(mutex);
    sem_close(full);
    sem_close(empty);
    sem_unlink("/my_mutex1");
    sem_unlink("/my_full1");
    sem_unlink("/my_empty1");

    // Detach the shared memory segment


    return 0;
}
