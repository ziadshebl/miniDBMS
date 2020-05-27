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
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<errno.h>
#include "msgbuffers.h"

int key=0;//Key accumlator.

struct clientManagerMsgBuffer message;//The message struct sent from the client to the manager to add a record.
struct record *tuple;// A pointer of type record.
struct additionSuccessMessageBuffer onAdditionSuccess;//The message struct sent from the manager to the client to confirm addition.

int ManagerClientMessageQid;//A variable to recieve queue id.
int sharedMemoryId;//A variable to recieve shared memory.
int messageRecieveStatus;//A variable to check if a message is recieved or not.
int messageSentStatus;

void addNewRecord();
void acquireRecord();

//MAIN Function.
int main(int argc, char*argv[])
{

    ManagerClientMessageQid = atoi(argv[2]);//Recieve the message queue id between client and manager from parent process.    
    sharedMemoryId = atoi(argv[1]);//Recieve the shared memory id from the parent process.  
    tuple =shmat(sharedMemoryId,NULL,0);//Attchment to the shared memory ro the record pointer.

    while(1)
    {
        messageRecieveStatus=msgrcv(ManagerClientMessageQid, &message, sizeof(message.operationMessage), getpid(), IPC_NOWAIT);//Recieving a message 
        if(messageRecieveStatus>-1)
        {
            if(message.operationMessage.operationNeeded==add)
            {
                addNewRecord();//Adds a new record from the last message sent to the shared memory.
            }
            else if(message.operationMessage.operationNeeded==acquire)
            {
                acquireRecord();//Aquire a record from the last message sent to the shared memory.
            }
            else if(message.operationMessage.operationNeeded==retrieve)
            {
                printf("A retrieve message rcvd \n");
            }
            
        }
    }
}

void addNewRecord()
{
    //Adding the last message sent data to the shared memory.
    tuple->key=key;
    tuple->salary=message.operationMessage.addBuffer.salary;
    strcpy(tuple->name,message.operationMessage.addBuffer.name);

    // printf("The key is: %d \n",tuple->key);
    // printf("The salary is: %d \n",tuple->salary);
    // printf("The name is: %s \n",tuple->name);
    // printf("Added..... \n");

    

    //Adding data to message to be sent on addition success.
    onAdditionSuccess.mtype=message.operationMessage.addBuffer.clientPID;
    onAdditionSuccess.key=key;

    messageSentStatus=msgsnd(ManagerClientMessageQid, &onAdditionSuccess, sizeof(onAdditionSuccess.key), !IPC_NOWAIT);//Sending a message to the dbmanager with the key of the tuple added.
    if(messageSentStatus>-1){
        //printf("Message sent successfully... \n");
    }
    else{
        printf("Error in sending... \n");
    }

    if(key<999)
    {
        key++; // to be handled 
        tuple+=sizeof(struct record);//Incrementing the pointer pointing to the shared memory by the size of the struct added.
    }
}
void acquireRecord()
{
    printf("An acquire request is recieved... \n");
}

