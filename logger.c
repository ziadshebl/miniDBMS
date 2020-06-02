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
#define MSG_SIZE 100

struct msgbuff
{
    long mtype;
    int SemaphoreStat;  
    int SenderPID;
};

int RecieveMessage(int MsgQid);

int main(int argc, char* argv[]){

FILE * LoggingOutputFile= fopen("LoggingOutputFile","w+");

char string[100] = "My Name is Karim Wael";
int loggerMsgQid = atoi(argv[1]);
int loggerSharedMemoryID = atoi(argv[2]);
printf("I am the logger and my pid is %d\n",getpid());
printf("I am the logger and my msgQid is %d\n",loggerMsgQid);
printf("I am the logger and my shared memory ID is %d\n",loggerSharedMemoryID);

struct loggerMsg* MemoryAddress =(struct loggerMsg*) shmat(loggerSharedMemoryID,NULL,0);
    strcpy(MemoryAddress->Msg,string);
  printf("Data written in memory: %s", MemoryAddress->Msg);


time_t rawtime;
struct tm* timeInfo;
char timeBuffer [80];

//Putting the current date in a readable format
time(&rawtime);
timeInfo = localtime(&rawtime);
strftime(timeBuffer,80,"%c:\t", timeInfo);


//Printing the current time in the database
fputs(timeBuffer, LoggingOutputFile);

fputs(string,LoggingOutputFile);





fclose(LoggingOutputFile);   
printf("I am the logger and my pid is %d\n",getpid());
printf("I am the logger and my msgQid is %d\n",loggerMsgQid);
printf("I am the logger and my shared memory ID is %d\n",loggerSharedMemoryID);

while(1){


    RecieveMessage(loggerMsgQid);
}

}

int RecieveMessage(int MsgQid){

    struct msgbuff message;

    int rec_val = msgrcv(MsgQid, &message, sizeof(message.SemaphoreStat)+sizeof(message.SenderPID), getpid(), IPC_NOWAIT);

    if (rec_val == -1)
    {
        
        return -1;
    }else
    {
        printf("Message Recieved From %d\n", message.SenderPID);
        struct msgbuff message2;

    message2.mtype = message.SenderPID;
    message2.SemaphoreStat = 1;
    message2.SenderPID = getpid();

    int send_val = msgsnd(MsgQid, &message2, sizeof(message2.SemaphoreStat)+sizeof(message2.SenderPID), IPC_NOWAIT);
        return 0;
    }
    
}