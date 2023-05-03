#include "sem_utils.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

// Reserve semaphore - decrement it by 1
int reserveSem(int semId, int semNum) {
    struct sembuf sops;

    sops.sem_num = semNum;
    sops.sem_op = -1;
    sops.sem_flg = 0;

    return semop(semId, &sops, 1);
}

// Release semaphore - increment it by 1
int releaseSem(int semId, int semNum) {
    struct sembuf sops;

    sops.sem_num = semNum;
    sops.sem_op = 1;
    sops.sem_flg = 0;

    return semop(semId, &sops, 1);
}

int waitZero_sem(int semId, int semNum){
    struct sembuf sops;

    sops.sem_num = semNum;
    sops.sem_op = 0;
    sops.sem_flg = 0;

    return semop(semId, &sops, 1);

}
