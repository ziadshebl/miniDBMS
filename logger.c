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
#include "msgbuffers.h"

struct msgbuff
{
    long mtype;
    int SemaphoreStat;  
};

int RecieveMessage(int MsgQid);

int main(int argc, char* argv[]){

FILE * LoggingOutputFile= fopen("LoggingOutputFile","w+");

char string[100] = "My Name is Karim Wael";
int loggerMsgQid = atoi(argv[1]);

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

while(1){


    RecieveMessage(loggerMsgQid);
}

}
/*
int SendMessageToAcquireSemaphore(int MsgQid, int RecieverID){

    struct msgbuff message;

    message.mtype = RecieverID;
    message.SemaphoreStat = AcquireSemaphore;

    int send_val = msgsnd(MsgQid, &message, sizeof(message.SemaphoreStat), IPC_NOWAIT);

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

int SendMessageToReleaseSemaphore(int MsgQid, int RecieverID){

    struct msgbuff message;

    message.mtype = RecieverID;
    message.SemaphoreStat = ReleaseSemaphore;

    int send_val = msgsnd(MsgQid, &message, sizeof(message.SemaphoreStat), IPC_NOWAIT);

    if (send_val == -1)
    {
        perror("Error in send");
        return -1;
    }else
    {
        return 0;
    }
    
}
*/

int RecieveMessage(int MsgQid){

    struct msgbuff message;

    int rec_val = msgrcv(MsgQid, &message, sizeof(message.SemaphoreStat), getpid(), IPC_NOWAIT);

    if (rec_val == -1)
    {
        //perror("Error in recieve");
        return -1;
    }else
    {
        printf("Message Recieved\n");
        return 0;
    }
    
}