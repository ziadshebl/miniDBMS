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
#include <time.h>
#include <sys/shm.h>
#include "semaphore.h"
#include "utils.h"

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
struct loggerMsg*MemoryAddress;

void loggerReceiveMessage(int MsgQid,struct loggerMsg* MemoryAddress, FILE * LoggingOutputFile, char* timeBuffer);
void loggerSendMessage(int MsgQid, int RecieverPID);
void Consume(struct loggerMsg* MemoryAddress, FILE * LoggingOutputFile, char* timeBuffer);
void WriteToFile(FILE * LoggingOutputFile, char* timeBuffer, char* ProcessID);
void terminateProcess(int sigNum);

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

 MemoryAddress =(struct loggerMsg*) shmat(loggerSharedMemoryID,NULL,0);
  printf("Data written in memory: %s\n", MemoryAddress->Msg);

//Changing SIGUSR2 handler to terminateSignal in order to terminate and detach shared memory when prent sends signal
signal(SIGUSR2,terminateProcess);


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


    loggerReceiveMessage(loggerMsgQid,MemoryAddress,LoggingOutputFile,timeBuffer);
}

}

void loggerReceiveMessage(int MsgQid,struct loggerMsg* MemoryAddress, FILE * LoggingOutputFile, char* timeBuffer){

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
                        
                loggerSendMessage(MsgQid,message.SenderPID);

            }
        }
        else if(message.SemaphoreStat == ReleaseSemaphore){

            if(message.SemaphoreType == EMPTY) ProcessToResume = releaseSemaphore(&empty);
            if(message.SemaphoreType == LOCK) ProcessToResume = releaseSemaphore(&lock);
             if(message.SemaphoreType == FULL) ProcessToResume = releaseSemaphore(&full);

             if(message.SemaphoreType == LOCK){
            Consume(MemoryAddress,LoggingOutputFile,timeBuffer);
            loggerSendMessage(MsgQid,message.SenderPID);
            
            if(ProcessToResume != 0){
            loggerSendMessage(MsgQid,ProcessToResume);
            //raise(SIGSTOP);

            

            }    

            }
        }
    
    
    }   
}
void loggerSendMessage(int MsgQid, int RecieverPID){

struct msgbuff message;
message.mtype = RecieverPID;
message.SemaphoreStat = MsgFromLogger;
message.SemaphoreType = DEFAULT;
message.SenderPID = getpid();
int send_val = msgsnd(MsgQid, &message, sizeof(message.SemaphoreStat)+sizeof(message.SenderPID)+sizeof(message.SemaphoreType), IPC_NOWAIT);


}
void Consume(struct loggerMsg* MemoryAddress, FILE * LoggingOutputFile, char* timeBuffer){
  
     /*int ret_val = acquireSemaphore(&full,-1);
     if(ret_val != 0) return;
     
     ret_val = acquireSemaphore(&lock,-1);
      if(ret_val != 0) return;
      */
     
   if(strcmp(MemoryAddress->Msg,PreviousLog)== 0 && MemoryAddress->senderPID == PreviousID) return;
   printf(".................................inside consume...................... %d\n",MemoryAddress->senderPID);

    char string[300];
    strcpy(string,MemoryAddress->Msg);
    int PID = MemoryAddress->senderPID;
    char LogMsg[350];
    snprintf(LogMsg, 350, "Process of ID: %d ", PID);

    strcat(LogMsg,string);
    WriteToFile(LoggingOutputFile,timeBuffer,LogMsg);
    strcpy(PreviousLog,MemoryAddress->Msg);
    PreviousID = MemoryAddress->senderPID;

    //releaseSemaphore(&lock);
    //releaseSemaphore(&empty);
    
}
void WriteToFile(FILE * LoggingOutputFile, char* timeBuffer, char* LogMsg){
    printf(".................................inside write...................... %s\n", LogMsg);
    LoggingOutputFile= fopen("LoggingOutputFile","a");
    fputs(timeBuffer, LoggingOutputFile);
    fputs(LogMsg,LoggingOutputFile);
    fprintf(LoggingOutputFile,"\n");
    fclose(LoggingOutputFile); 
}

void terminateProcess(int sigNum)
{
    shmdt(MemoryAddress);
    raise(SIGTERM);
}