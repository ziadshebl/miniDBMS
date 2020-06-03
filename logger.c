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
#include <sys/shm.h>
#include "msgbuffers.h"
#include "semaphore.h"

#define MsgFromLogger 2
#define AcquireSemaphore 1
#define ReleaseSemaphore 0
#define MAX_SIZE 100
#define UNLOCKED 1
#define LOCKED 0
#define EMPTY 0
#define FULL 1
#define LOCK 2
#define DEFAULT 3


int PreviousID = 0;
char PreviousLog[110]  = "DEFAULT";
struct semaphore empty;
struct semaphore full;
struct semaphore lock;

void RecieveMessage(int MsgQid);
void SendMessage(int MsgQid, int RecieverPID);
void Consume(struct loggerMsg* MemoryAddress, FILE * LoggingOutputFile, char* timeBuffer);

int main(int argc, char* argv[]){

FILE * LoggingOutputFile= fopen("LoggingOutputFile","w+");


int loggerMsgQid = atoi(argv[1]);
int loggerSharedMemoryID = atoi(argv[2]);

empty.semaphoreValue = MAX_SIZE;
full.semaphoreValue = 0;
lock.semaphoreValue = UNLOCKED;

empty.sleepingProcesses.rear = -1;
full.sleepingProcesses.rear = -1;
lock.sleepingProcesses.rear = -1;

struct loggerMsg* MemoryAddress =(struct loggerMsg*) shmat(loggerSharedMemoryID,NULL,0);
  printf("Data written in memory: %s\n", MemoryAddress->Msg);


time_t rawtime;
struct tm* timeInfo;
char timeBuffer [80];

//Putting the current date in a readable format
time(&rawtime);
timeInfo = localtime(&rawtime);
strftime(timeBuffer,80,"%c:\t", timeInfo);
   
printf("I am the logger and my pid is %d\n",getpid());
printf("I am the logger and my msgQid is %d\n",loggerMsgQid);
printf("I am the logger and my shared memory ID is %d\n",loggerSharedMemoryID);

while(1){


    RecieveMessage(loggerMsgQid);
    Consume(MemoryAddress,LoggingOutputFile,timeBuffer);
}

}

void RecieveMessage(int MsgQid){

    int SemaphoreValue;
    int ProcessToResume;
    struct msgbuff message;

    int rec_val = msgrcv(MsgQid, &message, sizeof(message.SemaphoreStat)+sizeof(message.SenderPID)+sizeof(message.SemaphoreType), getpid(), !IPC_NOWAIT);

    if (rec_val != -1)
    {
        
        printf("Message Recieved From %d\n", message.SenderPID);
        

        if(message.SemaphoreStat == AcquireSemaphore ){
            if(message.SemaphoreType == EMPTY) SemaphoreValue = acquireSemaphore(&empty,message.SenderPID);
            if(message.SemaphoreType == FULL) SemaphoreValue = acquireSemaphore(&full,message.SenderPID);
            if(message.SemaphoreType == LOCK) SemaphoreValue = acquireSemaphore(&lock,message.SenderPID);
            
            if(SemaphoreValue == 0){
                        
                SendMessage(MsgQid,message.SenderPID);
                //raise(SIGSTOP);   

            }
        }
        else if(message.SemaphoreStat == ReleaseSemaphore){

            if(message.SemaphoreType == EMPTY) ProcessToResume = releaseSemaphore(&empty);
            if(message.SemaphoreType == FULL) ProcessToResume = releaseSemaphore(&full);
            if(message.SemaphoreType == LOCK) ProcessToResume = releaseSemaphore(&lock);
            
            if(ProcessToResume != 0){
            SendMessage(MsgQid,ProcessToResume);
            //raise(SIGSTOP);

            }
        }
    
    
    }   
}
void SendMessage(int MsgQid, int RecieverPID){

struct msgbuff message;
message.mtype = RecieverPID;
message.SemaphoreStat = MsgFromLogger;
message.SemaphoreType = DEFAULT;
message.SenderPID = getpid();
int send_val = msgsnd(MsgQid, &message, sizeof(message.SemaphoreStat)+sizeof(message.SenderPID)+sizeof(message.SemaphoreType), IPC_NOWAIT);


}
void Consume(struct loggerMsg* MemoryAddress, FILE * LoggingOutputFile, char* timeBuffer){
  
 // acquireSemaphore(&full,getpid());
 // acquireSemaphore(&lock,getpid());

   if(strcmp(MemoryAddress->Msg,PreviousLog)== 0 && MemoryAddress->senderPID == PreviousID) return;
    printf("%d %s %s %d %d\n",strcmp(MemoryAddress->Msg,PreviousLog),MemoryAddress->Msg,PreviousLog,PreviousID,MemoryAddress->senderPID);
    char string[110];
    strcpy(string,MemoryAddress->Msg);
    int PID = MemoryAddress->senderPID;
    char ProcessID[110];
    snprintf(ProcessID, 110, "Process of ID %d ", PID);

    strcat(ProcessID,string);
    LoggingOutputFile= fopen("LoggingOutputFile","a");
    fputs(timeBuffer, LoggingOutputFile);
    fputs(ProcessID,LoggingOutputFile);
    fprintf(LoggingOutputFile,"\n");
    strcpy(PreviousLog,MemoryAddress->Msg);
    PreviousID = MemoryAddress->senderPID;

  //  releaseSemaphore(&lock);
   // releaseSemaphore(&empty);
    fclose(LoggingOutputFile);  
}