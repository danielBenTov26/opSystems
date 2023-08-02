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

#define key 1234
#define MAX_COMMAND_LENGTH 128
typedef enum {
    ADD,
    SUB,
    MUL,
} operate;
typedef struct PolynomOperate {
    int level1;
    int level2;
    int *pol1;
    int *pol2;
    char *op;
} Polynom;

void printAnswer(char *input);

void printAdd(Polynom polynom);

void printSub(Polynom polynom);

void printMul(Polynom polynom);

void printPolinom(int maxDegree, int *result);


int main() {
    char *ptr;
//    key_t key;
    int shm_id;


    if ((shm_id = shmget(key, 0, 0600)) == -1) {
        perror("failed to get memory");
        exit(1);
    }

    if ((ptr = (char *) shmat(shm_id, NULL, 0)) < 0) {
        perror("failed to attach memory");
        exit(1);
    }

    sem_t *mutex;
    mutex = sem_open("/my_mutex1", 0);
    if (mutex == SEM_FAILED) {
        perror("sam mutex open failed");
        exit(EXIT_FAILURE);
    }
    sem_t *full;
    full = sem_open("/my_full1", 0);
    if (full == SEM_FAILED) {
        perror("sam full open failed");
        exit(EXIT_FAILURE);
    }
    sem_t *empty;
    empty = sem_open("/my_empty1", 0);
    if (empty == SEM_FAILED) {
        perror("sam empty open failed");
        exit(EXIT_FAILURE);
    }
    int i = 0;
    char temp[129];
    char input[129];
    while (1) {
        if (i == 10)
            i = 0;

        //
        sem_wait(full);
        sem_wait(mutex);
        strcpy(temp, &ptr[i * MAX_COMMAND_LENGTH]);
        strcpy(input, strtok(temp, "\n"));
        i++;
        sem_post(mutex);
        sem_post(empty);
        //

        if (strcmp(input, "END") == 0)
            break;
        printAnswer(input);
    }
    if (shmdt(ptr) == -1) {
        perror("shmdt");
        return 1;
    }

    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        return 1;
    }


}

void printAnswer(char *input) {
    Polynom polynom;

    char *pollA = strtok(input + 1, ")");
    char *operateP = strtok(NULL, "(");
    char *pollB = strtok(NULL, ")");


    polynom.op = operateP;


    char *token = strtok(pollA, ":");
    polynom.level1 = atoi(token);
    polynom.pol1 = (int *) malloc(sizeof(int) * (polynom.level1 + 1));
    int i = polynom.level1;
    token = strtok(NULL, ",");
    while (token != NULL && i >= 0) {
        polynom.pol1[i] = atoi(token);
        token = strtok(NULL, ",");
        i--;
    }

    token = strtok(pollB, ":");
    polynom.level2 = atoi(token);
    polynom.pol2 = (int *) malloc(sizeof(int) * (polynom.level2 + 1));
    i = polynom.level2;
    token = strtok(NULL, ",");
    while (token != NULL && i >= 0) {
        polynom.pol2[i] = atoi(token);
        token = strtok(NULL, ",");
        i--;
    }

    if (strcmp(polynom.op, "ADD") == 0) {
        printAdd(polynom);
    } else if (strcmp(polynom.op, "SUB") == 0) {
        printSub(polynom);
    } else if (strcmp(polynom.op, "MUL") == 0) {
        printMul(polynom);
    }

    free(polynom.pol1);
    free(polynom.pol2);
}

void printAdd(Polynom polynom) {
    int *poly1 = polynom.pol1;
    int degree1 = polynom.level1;
    int *poly2 = polynom.pol2;
    int degree2 = polynom.level2;
    // Calculate the maximum degree of the connected polynomial
    int maxDegree = (degree1 > degree2) ? degree1 : degree2;

    // Create an array to store the coefficients of the connected polynomial
    int result[maxDegree + 1];

    // Initialize the result array with zeros
    for (int i = 0; i <= maxDegree; i++) {
        result[i] = 0;
    }

    // Copy the coefficients of the first polynomial to the result array
    for (int i = 0; i <= degree1; i++) {
        result[i] += poly1[i];
    }

    // Copy the coefficients of the second polynomial to the result array
    for (int i = 0; i <= degree2; i++) {
        result[i] += poly2[i];
    }
    printPolinom(maxDegree, result);
}

void printSub(Polynom polynom) {
    int *poly1 = polynom.pol1;
    int degree1 = polynom.level1;
    int *poly2 = polynom.pol2;
    int degree2 = polynom.level2;
    // Calculate the maximum degree of the subtracted polynomial
    int maxDegree = (degree1 > degree2) ? degree1 : degree2;

    // Create an array to store the coefficients of the subtracted polynomial
    int result[maxDegree + 1];

    // Initialize the result array with zeros
    for (int i = 0; i <= maxDegree; i++) {
        result[i] = 0;
    }

    // Subtract the coefficients of the second polynomial from the first polynomial
    for (int i = 0; i <= degree1; i++) {
        result[i] += poly1[i];
    }
    for (int i = 0; i <= degree2; i++) {
        result[i] -= poly2[i];
    }

    // Print the subtraction result
    printPolinom(maxDegree, result);

}

void printMul(Polynom polynom) {
    int *poly1 = polynom.pol1;
    int degree1 = polynom.level1;
    int *poly2 = polynom.pol2;
    int degree2 = polynom.level2;

    // Calculate the degree of the multiplied polynomial
    int resultDegree = degree1 + degree2;

    // Create an array to store the coefficients of the multiplied polynomial
    int result[resultDegree + 1];

    // Initialize the result array with zeros
    for (int i = 0; i <= resultDegree; i++) {
        result[i] = 0;
    }

    // Perform polynomial multiplication
    for (int i = 0; i <= degree1; i++) {
        for (int j = 0; j <= degree2; j++) {
            result[i + j] += poly1[i] * poly2[j];
        }
    }

    printPolinom(resultDegree, result);


}


void printPolinom(int maxDegree, int *result) {
    // Print the subtraction result
    int zeroFlag = 0;
    for (int i = maxDegree; i >= 0; i--) {
        if (result[i] == 0)
            continue;
        zeroFlag++;
        printf("%d", result[i]);

        if (i > 0) {
            if (i == 1) { printf("x "); }
            else { printf("x^%d ", i); }
            if (result[i - 1] > 0 && i - 1 >= 0) {
                printf("+ ");
            } else if (result[i - 1] < 0 && i - 1 >= 0) {
                printf("- ");
                result[i - 1] = result[i - 1] * -1;
            } else {
                for (int j = 2; i - j >= 0; ++j) {


                    if (result[i - j] > 0 && i - j >= 0) {
                        printf("+ ");
                        break;
                    } else if (result[i - j] < 0 && i - j >= 0) {
                        printf("- ");
                        result[i - j] = result[i - j] * -1;
                        break;
                    }
                }
            }
        }
    }
    if (zeroFlag == 0)
        printf("0");
    printf("\n");


}