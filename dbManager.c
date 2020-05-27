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
    int key=0;
    struct clientManagerMsgBuffer message;
    struct record record;
    struct additionSuccessMessageBuffer onAdditionSuccess; 
    //Getting the message q id and the shared memory id.
    int sharedMemoryId = atoi(argv[1]);
    int ManagerClientMessageQid = atoi(argv[2]);
    int messageRecieveStatus;
    //struct record * memoryLocationStart = (struct record *) shmat(sharedMemoryId,(void*)0,0); 
     printf("I am the dbManager\n");
     printf("The shared memory id is: %d \n",sharedMemoryId);
    // printf("The message Q id is: %d \n",ManagerClientMessageQid);
    // printf("I am the dbManager and my id is: %d\n",getpid());
    while(1)
    {
        messageRecieveStatus=msgrcv(ManagerClientMessageQid, &message, sizeof(message.operationMessage), getpid(), IPC_NOWAIT);
        if(messageRecieveStatus>-1)
        {
            if(message.operationMessage.operationNeeded==add)
            {
                printf("The Name is %s \n",message.operationMessage.addBuffer.name);
                record.key=key;
                strcpy(record.name,message.operationMessage.addBuffer.name);
                printf("After strcpy... \n");
                record.salary=message.operationMessage.addBuffer.salary;
                //printf("The size of struct is: %ld \n",sizeof(struct record));
                //printf("The memory location is: %x \n",memoryLocationStart);
                //*(memoryLocationStart+(28*key))=record;
                //printf("After adding in shared mem... \n");
                onAdditionSuccess.key=key;
                onAdditionSuccess.mtype=message.operationMessage.addBuffer.clientPID;
                printf("Before sending....\n");
                msgsnd(ManagerClientMessageQid, &onAdditionSuccess, sizeof(onAdditionSuccess.key), !IPC_NOWAIT);
                printf("After sending....\n");
                if(key<999)
                {
                    key++; // to be handled 
                }

               //struct record secondEntry=*(memoryLocationStart+ sizeof(struct record)*(key-1));
                //printf("From the shared memory the name added is: %s \n",secondEntry.name);
            }
            else if(message.operationMessage.operationNeeded==modify)
            {
                printf("A modify message rcvd \n");
            }
            else if(message.operationMessage.operationNeeded==retrieve)
            {
                printf("A retrieve message rcvd \n");
            }
            
        }
    }
    

    


    
}
