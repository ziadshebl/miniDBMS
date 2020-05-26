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


struct clientManagerMsgBuffer message;
//MAIN Function.
int main(int argc, char*argv[]){
     
    //Getting the message q id and the shared memory id.
    int sharedMemoryId = atoi(argv[1]);
    int ManagerClientMessageQid = atoi(argv[2]);
    printf("I am the dbManager\n");
    printf("The shared memory id is: %d \n",sharedMemoryId);
    printf("The message Q id is: %d \n",ManagerClientMessageQid);
    while(1)
    {

    }
    
    

}
void Recieve()
{
    while(msgrcv(ManagerClientMessageQid, &message, sizeof(message.operationMessage), getpid(), !IPC_NOWAIT)!=-1)
    {
    }
    
}