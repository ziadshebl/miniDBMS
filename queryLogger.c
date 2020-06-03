#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include "utils.h"
#include "semaphore.h"

void acquireQueryFileSemaphore(struct queryLoggerMsgBuffer message);
void releaseQueryFileSemaphore(struct queryLoggerMsgBuffer message);
void sendReleaseMessage(int pid);

struct semaphore queryFileSemaphore;
int queryLoggerMsgQid;


int main(int argc,char* argv[])
{
   // for(int i=0;i<argc;i++)
     //   printf("queryyLogger argv[%d]%s\n",i,argv[i]);
    struct queryLoggerMsgBuffer receivedMsg;
    int msgReceiveStats;
    queryLoggerMsgQid = atoi(argv[1]);

    //printf("I am the queryLogger, Querylogger message Qid is:%d \n",queryLoggerMsgQid);

    //intializing semaphore:
    queryFileSemaphore.semaphoreValue=1;
    queryFileSemaphore.sleepingProcesses.rear=-1;

    while(1)
    {
         msgReceiveStats=msgrcv(queryLoggerMsgQid, &receivedMsg, sizeof(receivedMsg)-sizeof(long), getpid(), IPC_NOWAIT);//Recieving a message 
        if(msgReceiveStats!= -1)
        {
            if(receivedMsg.neededoperation==acquire)
                acquireQueryFileSemaphore(receivedMsg);
            else if(receivedMsg.neededoperation==release)
                releaseQueryFileSemaphore(receivedMsg);
        }
    }

}

void acquireQueryFileSemaphore(struct queryLoggerMsgBuffer message)
{
    int returnValue=acquireSemaphore(&queryFileSemaphore,message.senderPID);
    if(returnValue==0)
        sendReleaseMessage(message.senderPID);
}

void releaseQueryFileSemaphore(struct queryLoggerMsgBuffer message)
{
    int processToAwaken=releaseSemaphore(&queryFileSemaphore);
    if(processToAwaken)
        sendReleaseMessage(processToAwaken);
}

void sendReleaseMessage(int pid)
{
    struct operationSuccessMessageBuffer releaseMsg;
    releaseMsg.isOperationDone=1;
    releaseMsg.mtype=pid;
    //releaseMsg.numberOfRecords=0;
    int msgStatus=msgsnd(queryLoggerMsgQid, &releaseMsg, sizeof(releaseMsg)-sizeof(long), !IPC_NOWAIT);
    if(msgStatus==-1)
        printf("ERROR! Query logger cannot send release message!\n");
}