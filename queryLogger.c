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
void logAcquiring(int clientNumber);
void logReleasing(int clientNumber);
void logSleeping(int clientNumber);
void logAwakeningProcess(int PID);

struct semaphore queryFileSemaphore;
int queryLoggerMsgQid;
int loggerPID;
int loggerMsgQid;
int loggerSharedMem;

char logMessage[200];
char clientNumberChar[10];

int main(int argc,char* argv[])
{
    //Getting passed Argumments by parent
    queryLoggerMsgQid = atoi(argv[1]);
    loggerMsgQid=atoi(argv[2]);
    loggerPID=atoi(argv[3]);
    loggerSharedMem=atoi(argv[4]);

    //intializing semaphore:
    queryFileSemaphore.semaphoreValue=1;
    queryFileSemaphore.sleepingProcesses.rear=-1;

    //Receiving Message
    struct queryLoggerMsgBuffer receivedMsg;
    int msgReceiveStats;
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
    {
        sendReleaseMessage(message.senderPID);
        logAcquiring(message.clientNumber);
    }
    else
        logSleeping(message.clientNumber);
        
}

void releaseQueryFileSemaphore(struct queryLoggerMsgBuffer message)
{
    int processToAwaken=releaseSemaphore(&queryFileSemaphore);
    logReleasing(message.clientNumber);
    if(processToAwaken)
    {
        sendReleaseMessage(processToAwaken);
        logAwakeningProcess(processToAwaken);
    }     
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

void logAcquiring(int clientNumber)
{
    strcpy(logMessage,"Query logger received an acquire request from client number ");
    sprintf(clientNumberChar,"%d",clientNumber);
    strcat(logMessage,clientNumberChar);
    strcat(logMessage," and the client acquired the Query Logging semaphore successfully! \n");
    Log(logMessage,loggerMsgQid,loggerPID,loggerSharedMem);
}

void logReleasing(int clientNumber)
{
    strcpy(logMessage,"The client number ");
    sprintf(clientNumberChar,"%d",clientNumber);
    strcat(logMessage,clientNumberChar);
    strcat(logMessage," just released the Query Logging semaphore! \n");
    Log(logMessage,loggerMsgQid,loggerPID,loggerSharedMem);
}

void logSleeping(int clientNumber)
{
    strcpy(logMessage,"Query Logger recieved an acquire request from client number ");
    sprintf(clientNumberChar,"%d",clientNumber);
    strcat(logMessage,clientNumberChar);
    strcat(logMessage,", the proccess did not acquire the semaphore and it is now sleeping ZZZZZZZ.....");
    Log(logMessage,loggerMsgQid,loggerPID,loggerSharedMem);
}

void logAwakeningProcess(int PID)
{
     strcpy(logMessage,"Process with PID:");
     sprintf(clientNumberChar,"%d",PID);
     strcat(logMessage,clientNumberChar);
     strcat(logMessage,"has been awaken by the Query Logging semaphore \n");
}