#include"semaphore.h"
//Queue functions
void enqueue(struct queue * _queue,int processID)
{
    if(_queue->rear==MAX_PROCESSES-1)
    {
        //printf("Queue is Full! \n");
        return;
    }

    if(_queue->rear==-1)
    {
        _queue->rear=0;
        _queue->queueElements[_queue->rear]=processID;
    }
    else{
        _queue->rear++;
        _queue->queueElements[_queue->rear]=processID;
    }
}

int dequeue(struct queue * _queue){
    if(_queue->rear==-1)
    {   
        //printf("Queue is empty! \n");
        return -1;
    }
        
    int dequeuedProcess=_queue->queueElements[0];
    for(int index=0;index<_queue->rear;index++)
       _queue->queueElements[index]=_queue->queueElements[index+1];
    _queue->rear--;
    return dequeuedProcess;
}

void printQueue(struct queue  _queue){
    for(int index=0;index<=_queue.rear;index++)
        printf("%d ",_queue.queueElements[index]);
    printf("\n");
}
///////////////////////////////////////////////////////////////////////////////////
//Semaphore Functions
int acquireSemaphore(struct semaphore *_semaphore,int pid)
{
    atomic_int oldVal=atomic_fetch_sub(&(_semaphore->semaphoreValue),1);
    //printf("value before sub:%d\n",oldVal);
    //printf("value after subtract %d\n",_semaphore->semaphoreValue);
    //comparison is made with value returned from subtract atomic operation
    //lock is not acquired
    if(oldVal<=0)
    {
        enqueue(&(_semaphore->sleepingProcesses),pid);
        kill(pid,SIGSTOP);
        return -1;
    }
    //lock is acquired
    else
        return 0;  
}

int releaseSemaphore(struct semaphore *_semaphore)
{
    atomic_int oldVal=atomic_fetch_add(&(_semaphore->semaphoreValue),1);
    //printf("value before add:%d\n",oldVal);
    //printf("value after add %d\n",_semaphore->semaphoreValue);
    //waiting process i queue
    //todo::
    if(oldVal<0)
    {
        int processToWake=dequeue(&(_semaphore->sleepingProcesses));
        kill(processToWake,SIGCONT);
        return processToWake;
    }
    //no waiting process
    return 0;
}
///////////////////////////////////////////////////////////////////////////////////

