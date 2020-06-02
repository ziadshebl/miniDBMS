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

#define AcquireSemaphore 1
#define ReleaseSemaphore 0


struct msgbuff
{
    long mtype;
    int SemaphoreStat; 
    int SenderPID; 
};

int SendMessageToAcquireSemaphore(int MsgQid, int RecieverPID);
int SendMessageToReleaseSemaphore(int MsgQid, int RecieverPID);
int RecieveMessage(int MsgQid);
void Log(char* LogMessage, int MsgQid, int LoggerPID, int loggerSharedMemoryID);
int loggerFunctions(){


}
int SendMessageToAcquireSemaphore(int MsgQid, int RecieverPID){

    struct msgbuff message;

    message.mtype = RecieverPID;
    message.SemaphoreStat = AcquireSemaphore;
    message.SenderPID = getpid();

    int send_val = msgsnd(MsgQid, &message, sizeof(message.SemaphoreStat)+sizeof(message.SenderPID), IPC_NOWAIT);

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

int SendMessageToReleaseSemaphore(int MsgQid, int RecieverPID){

    struct msgbuff message;

    message.mtype = RecieverPID;
    message.SemaphoreStat = ReleaseSemaphore;
    message.SenderPID = getpid();

    int send_val = msgsnd(MsgQid, &message, sizeof(message.SemaphoreStat)+sizeof(message.SenderPID), IPC_NOWAIT);

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

    int rec_val = msgrcv(MsgQid, &message, sizeof(message.SemaphoreStat)+sizeof(message.SenderPID), getpid(), !IPC_NOWAIT);

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

    kill(LoggerPID,SIGCONT);
    SendMessageToAcquireSemaphore(MsgQid, LoggerPID);
    RecieveMessage(MsgQid);


    //Write in shared memory
    struct loggerMsg* MemoryAddress =(struct loggerMsg*) shmat(loggerSharedMemoryID,NULL,0);
    strcpy(MemoryAddress->Msg,SentLogMessage);

    SendMessageToReleaseSemaphore(MsgQid,LoggerPID);
    shmdt(MemoryAddress);
}
