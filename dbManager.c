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



//MAIN Function.
int main(int argc, char*argv[]){
     struct clientManagerMsgBuffer message;
    //Getting the message q id and the shared memory id.
    int sharedMemoryId = atoi(argv[1]);
    int ManagerClientMessageQid = atoi(argv[2]);
    int messageRecieveStatus;
    // printf("I am the dbManager\n");
    // printf("The shared memory id is: %d \n",sharedMemoryId);
    // printf("The message Q id is: %d \n",ManagerClientMessageQid);
    // printf("I am the dbManager and my id is: %d\n",getpid());
    while(1)
    {
        messageRecieveStatus=msgrcv(ManagerClientMessageQid, &message, sizeof(message.operationMessage), getpid(), IPC_NOWAIT);
        if(messageRecieveStatus>-1)
        {
            printf("The Name is %s \n",message.operationMessage.addMsgBuffer.name);
        }
    }
    

    


    
}
