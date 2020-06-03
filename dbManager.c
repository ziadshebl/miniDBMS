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
#include "utils.h"
#include "semaphore.h"

int key=0;//Key accumlator.
int temp;

struct clientManagerMsgBuffer message;//The message struct sent from the client to the manager to add a record.
struct record *tuple;// A pointer of type record.
struct record *startOfTheSharedMemory;
struct additionSuccessMessageBuffer onAdditionSuccess;//The message struct sent from the manager to the client to confirm addition.
struct operationSuccessMessageBuffer onSuccessMessage;//The message struct sent to the manager when acquire and modification is successful.
struct semaphore recordsSemaphores[MAX_RECORDS];

int ManagerClientMessageQid;//A variable to recieve queue id.
int sharedMemoryId;//A variable to recieve shared memory.
int messageRecieveStatus;//A variable to check if a message is recieved or not.
int messageSentStatus;
int loggerMsgQid;
int loggerPID;
int loggerSharedMemoryID;

void addNewRecord();
void acquireRecord();
//void modifyRecord();
void releaseRecord();

//MAIN Function.
int main(int argc, char*argv[])
{
    loggerSharedMemoryID = atoi(argv[5]);
    loggerMsgQid = atoi(argv[3]);
    loggerPID = atoi(argv[4]);
    printf("I am the manager and logger shared memory ID is %d\n",loggerSharedMemoryID);
    ManagerClientMessageQid = atoi(argv[2]);//Recieve the message queue id between client and manager from parent process.  
    printf("MsgQ PID is: %d\n",ManagerClientMessageQid);  
    sharedMemoryId = atoi(argv[1]);//Recieve the shared memory id from the parent process.  
    tuple =shmat(sharedMemoryId,NULL,0);//Attchment to the shared memory to the record pointer.
    startOfTheSharedMemory=tuple;
    printf("memory id is: %d,address is:%d\n",sharedMemoryId,tuple);

    //Intializing semaphores
    for (int index=0;index<MAX_RECORDS;index++)
    {
        recordsSemaphores[index].semaphoreValue=1;
        recordsSemaphores[index].sleepingProcesses.rear=-1;
    }

    while(1)
    {
        messageRecieveStatus=msgrcv(ManagerClientMessageQid, &message, sizeof(message.operationMessage), getpid(), IPC_NOWAIT);//Recieving a message 
        if(messageRecieveStatus>-1)
        {
            printf("message soperation is %d \n",message.operationMessage.operationNeeded);
            if(message.operationMessage.operationNeeded==add)
            {
                addNewRecord();//Adds a new record from the last message sent to the shared memory.
            }
            else if(message.operationMessage.operationNeeded==acquire)
            {
                acquireRecord();//Aquire a record from the last message sent to the shared memory.
            }
            /*
            else if(message.operationMessage.operationNeeded==modify)
            {
                modifyRecord();
            }
            */
            else if(message.operationMessage.operationNeeded==release)
            {
                releaseRecord();
            }
            
        }
    }
}

void addNewRecord()
{
    //Adding the last message sent data to the shared memory.
    printf("Now adding new record in Manager\n");
    tuple->key=key;
    tuple->salary=message.operationMessage.addBuffer.salary;
    strcpy(tuple->name,message.operationMessage.addBuffer.name);


    printf("............................................................... \n");
    printf("The key  is: %d \n",tuple->key);
    printf("The salary is: %d \n",tuple->salary);
    printf("The name is: %s \n",tuple->name);
    printf("............................................................... \n");

    

    //Adding data to message to be sent on addition success.
    onAdditionSuccess.mtype=message.operationMessage.addBuffer.clientPID;
    onAdditionSuccess.key=key;

    messageSentStatus=msgsnd(ManagerClientMessageQid, &onAdditionSuccess, sizeof(onAdditionSuccess.key), !IPC_NOWAIT);//Sending a message to the dbmanager with the key of the tuple added.
    if(messageSentStatus>-1){
        //printf("Addition Message sent successfully... \n");
    }
    else{
        //printf("Error in sending... \n");
    }

    if(key<999)
    {
        key++; // to be handled 
        tuple+=sizeof(struct record);//Incrementing the pointer pointing to the shared memory by the size of the struct added.
    }
}

void sendReleaseMessage(int pid)
{
    onSuccessMessage.mtype=pid;
    onSuccessMessage.isOperationDone=1;
    //onSuccessMessage.numberOfRecords=key-1;
    messageSentStatus=msgsnd(ManagerClientMessageQid, &onSuccessMessage, sizeof(onSuccessMessage.isOperationDone), !IPC_NOWAIT);//Sending a message to the dbmanager with the status of the acquire  of the tuple requested.
    if(messageSentStatus>-1){
    //printf("Acquire Message sent successfully... \n");
    }
    else{
        printf("Error in sending.... \n");
    }
}
void acquireRecord()
{
    printf("An acquire request is recieved..... \n");
    int reqsemaphore=message.operationMessage.semaphoreOperationsBuffer.recordKey;
    int returnValue=acquireSemaphore(&recordsSemaphores[reqsemaphore],message.operationMessage.semaphoreOperationsBuffer.clientPID);
    if(returnValue==0)
    {
        sendReleaseMessage(message.operationMessage.semaphoreOperationsBuffer.clientPID);
        printf("ASemaphore aquired\n");
    }
        
}

void releaseRecord()
{
    int reqsemaphore=message.operationMessage.semaphoreOperationsBuffer.recordKey;
    int awakenedProcess=releaseSemaphore(&recordsSemaphores[reqsemaphore]);
    //if process is awakened send release record
    if(awakenedProcess!=0)
        sendReleaseMessage(awakenedProcess);
}   

/*
void modifyRecord()
{
    // printf("A modification request is recieved...................... \n");
     temp=message.operationMessage.modifyBuffer.recordKey*sizeof(struct record);
     if(message.operationMessage.modifyBuffer.salaryOperation==increase) 
     {(startOfTheSharedMemory+temp)->salary+=message.operationMessage.modifyBuffer.value;}
     if(message.operationMessage.modifyBuffer.salaryOperation==decrease) 
     {(startOfTheSharedMemory+temp)->salary-=message.operationMessage.modifyBuffer.value;}
     printf("\n");
     printf("The key to be edited  is: %d \n",(startOfTheSharedMemory+temp)->key);
     printf("The salary  after editing is: %d \n",(startOfTheSharedMemory+temp)->salary);
     printf("The name is: %s \n",(startOfTheSharedMemory+temp)->name);
     printf("Edited...\n");
     printf("\n");
    onSuccessMessage.mtype=message.operationMessage.modifyBuffer.clientPID;
    onSuccessMessage.isOperationDone=1;
    messageSentStatus=msgsnd(ManagerClientMessageQid, &onSuccessMessage, sizeof(onSuccessMessage.isOperationDone), !IPC_NOWAIT);//Sending a message to the dbmanager with the status of the modify  of the tuple requested.
    if(messageSentStatus>-1){
        //printf("Modify Message sent successfully... \n");
    }
    else{
        printf("Error in sending.... \n");
    }
 }
*/