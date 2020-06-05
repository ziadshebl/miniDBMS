#define MAX_PROCESSES 100
#include <stdatomic.h>
#include <stdio.h>
#include <signal.h>

struct queue
{
    int rear;
    int queueElements[MAX_PROCESSES];
};

struct semaphore
{
    atomic_int semaphoreValue;
    struct queue sleepingProcesses;

};

int acquireSemaphore(struct semaphore *_semaphore,int pid);
int releaseSemaphore(struct semaphore *_semaphore);
void printQueue(struct queue  _queue);