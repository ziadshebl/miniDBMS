#include "msgbuffers.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/shm.h>

#define AcquireSemaphore 1
#define ReleaseSemaphore 0
#define EMPTY 0
#define FULL 1
#define LOCK 2
#define DEFAULT 3


int SendMessageToAcquireSemaphore(int MsgQid, int RecieverPID,int SemaphoreType);
int SendMessageToReleaseSemaphore(int MsgQid, int RecieverPID,int SemaphoreType);
int RecieveMessage(int MsgQid);
void Log(char* LogMessage, int MsgQid, int LoggerPID, int loggerSharedMemoryID);
int loggerFunctions(){


}
int SendMessageToAcquireSemaphore(int MsgQid, int RecieverPID,int SemaphoreType){

    struct msgbuff message;

    message.mtype = RecieverPID;
    message.SemaphoreStat = AcquireSemaphore;
    message.SenderPID = getpid();
    message.SemaphoreType = SemaphoreType;

    int send_val = msgsnd(MsgQid, &message, sizeof(message.SemaphoreStat)+sizeof(message.SenderPID)+sizeof(message.SemaphoreType), IPC_NOWAIT);

    if (send_val == -1)
    {
        perror("Error in send");
        return -1;
    }
    else
    {
        return 0;
    }
    
}

int SendMessageToReleaseSemaphore(int MsgQid, int RecieverPID, int SemaphoreType){

    struct msgbuff message;

    message.mtype = RecieverPID;
    message.SemaphoreStat = ReleaseSemaphore;
    message.SenderPID = getpid();
    message.SemaphoreType = SemaphoreType;

    int send_val = msgsnd(MsgQid, &message, sizeof(message.SemaphoreStat)+sizeof(message.SenderPID)+sizeof(message.SemaphoreType), IPC_NOWAIT);

    if (send_val == -1)
    {
        perror("Error in send");
        return -1;
    }else
    {
        return 0;
    }
    
}
int RecieveMessage(int MsgQid){

    struct msgbuff message;

    int rec_val = msgrcv(MsgQid, &message, sizeof(message.SemaphoreStat)+sizeof(message.SenderPID)+sizeof(message.SemaphoreType), getpid(), !IPC_NOWAIT);

    if (rec_val == -1)
    {
        
        return -1;
    }else
    {
        printf("This is process %d. Message Recieved From Logger\n", getpid());
        return 0;
    }
    
}
void Log(char* SentLogMessage, int MsgQid, int LoggerPID, int loggerSharedMemoryID ){

    
    SendMessageToAcquireSemaphore(MsgQid, LoggerPID, EMPTY);
    RecieveMessage(MsgQid);

    
    SendMessageToAcquireSemaphore(MsgQid, LoggerPID, LOCK);
    RecieveMessage(MsgQid);

    //Write in shared memory
    struct loggerMsg* MemoryAddress =(struct loggerMsg*) shmat(loggerSharedMemoryID,NULL,0);
    strcpy(MemoryAddress->Msg,SentLogMessage);
    MemoryAddress->senderPID = getpid();
    printf("this is process %d and written in memory is: %s\n",getpid() ,MemoryAddress->Msg);

    SendMessageToReleaseSemaphore(MsgQid,LoggerPID, LOCK);
    SendMessageToReleaseSemaphore(MsgQid,LoggerPID, FULL);
    RecieveMessage(MsgQid);
    shmdt(MemoryAddress);
}
