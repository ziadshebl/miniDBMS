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
struct record *tupleNext; //A pointer of type record.
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


char loggingMessage[200];
char templateForSalaryAndKeyLogged[20];

void addNewRecord();
void logAdding(struct record * tuple);
void acquireRecord();
void logAcquiring(int reqsemaphore);
void logSleeping(int reqsemaphore);
void releaseRecord();
void logReleasing(int reqsemaphore);
void logAwakeningProcess(int processPID,int key);

//MAIN Function.
int main(int argc, char*argv[])
{
 
    // for(int i=0;i<argc;i++)
    //     printf("manager argv[%d]%s\n",i,argv[i]);
    loggerSharedMemoryID = atoi(argv[5]);
    loggerMsgQid = atoi(argv[3]);
    loggerPID = atoi(argv[4]);
    //printf("I am the manager and logger shared memory ID is %d\n",loggerSharedMemoryID);
    ManagerClientMessageQid = atoi(argv[2]);//Recieve the message queue id between client and manager from parent process.  
    //printf("MsgQ PID is: %d\n",ManagerClientMessageQid);  
    sharedMemoryId = atoi(argv[1]);//Recieve the shared memory id from the parent process.  
    tuple =shmat(sharedMemoryId,NULL,0);//Attchment to the shared memory to the record pointer.
    struct loggerMsg* MemoryAddress =(struct loggerMsg*) shmat(loggerSharedMemoryID,NULL,0);
    startOfTheSharedMemory=tuple;
    tupleNext=tuple+sizeof(struct record);
    //printf("memory id is: %d,address is:%d\n",sharedMemoryId,tuple);

    tuple->key=-1;
    tuple->salary=-1;
    strcpy(tuple->name,"");

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
            //printf("message soperation is %d \n",message.operationMessage.operationNeeded);
            if(message.operationMessage.operationNeeded==add)
            {
                addNewRecord();//Adds a new record from the last message sent to the shared memory.
            }
            else if(message.operationMessage.operationNeeded==acquire)
            {
                acquireRecord();//Aquire a record from the last message sent to the shared memory.
            }
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

    tupleNext->key=-1;
    tupleNext->salary=-1;
    strcpy(tupleNext->name,"");
    logAdding(tuple);
    onAdditionSuccess.mtype=message.operationMessage.addBuffer.clientPID;
    onAdditionSuccess.key=key;

    messageSentStatus=msgsnd(ManagerClientMessageQid, &onAdditionSuccess, sizeof(onAdditionSuccess.key), !IPC_NOWAIT);//Sending a message to the dbmanager with the key of the tuple added.
    if(messageSentStatus>-1){
        printf("Addition Message sent successfully... \n");
    }
    else{
        //printf("Error in sending... \n");
    }

    if(key<999)
    {
        key++; // to be handled 
        tuple+=sizeof(struct record);//Incrementing the pointer pointing to the shared memory by the size of the struct added.
        tupleNext+=sizeof(struct record);
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
    //printf("An acquire request is recieved..... \n");
    int reqsemaphore=message.operationMessage.semaphoreOperationsBuffer.recordKey;
    int returnValue=acquireSemaphore(&recordsSemaphores[reqsemaphore],message.operationMessage.semaphoreOperationsBuffer.clientPID);
    if(returnValue==0)
    {
            sendReleaseMessage(message.operationMessage.semaphoreOperationsBuffer.clientPID);
            logAcquiring(reqsemaphore);
    }
    else
    {
            logSleeping(reqsemaphore);
    }
        
}
void releaseRecord()
{
    int reqsemaphore=message.operationMessage.semaphoreOperationsBuffer.recordKey;
    int awakenedProcess=releaseSemaphore(&recordsSemaphores[reqsemaphore]);
    logReleasing(reqsemaphore);
    //if process is awakened send release record
    if(awakenedProcess!=0)
    {
        sendReleaseMessage(awakenedProcess);
        logAwakeningProcess(awakenedProcess,reqsemaphore);

    }
}
void logAdding(struct record * tuple)
{
    strcat(loggingMessage,"The manager added a record with a key of: ");
    sprintf(templateForSalaryAndKeyLogged,"%d",tuple->key);
    strcat(loggingMessage,templateForSalaryAndKeyLogged);

    strcat(loggingMessage," and with name of: ");
    strcat(loggingMessage,tuple->name);

    strcat(loggingMessage," and with a salary of: ");
    sprintf(templateForSalaryAndKeyLogged,"%d",tuple->salary);
    strcat(loggingMessage,templateForSalaryAndKeyLogged);

    strcat(loggingMessage," for client Number: ");
    sprintf(templateForSalaryAndKeyLogged,"%d",message.operationMessage.addBuffer.clientNumber);
    strcat(loggingMessage,templateForSalaryAndKeyLogged);

    strcat(loggingMessage,"\n");
    
    Log(loggingMessage,loggerMsgQid,loggerPID,loggerSharedMemoryID);
    strcpy(loggingMessage,"");
}
void logAcquiring(int reqsemaphore)
{
    strcat(loggingMessage,"The manager recieved an acquire request from client number: ");
    sprintf(templateForSalaryAndKeyLogged,"%d",message.operationMessage.semaphoreOperationsBuffer.clientNumber);
    strcat(loggingMessage,templateForSalaryAndKeyLogged);

    strcat(loggingMessage," and the proccess acquired the semaphore of key: ");
    sprintf(templateForSalaryAndKeyLogged,"%d",reqsemaphore);
    strcat(loggingMessage,templateForSalaryAndKeyLogged);
    strcat(loggingMessage,"\n");
            
    Log(loggingMessage,loggerMsgQid,loggerPID,loggerSharedMemoryID);
    strcpy(loggingMessage,"");
}
void logSleeping(int reqsemaphore)
{
    strcat(loggingMessage,"The manager recieved an acquire request from client number: ");
    sprintf(templateForSalaryAndKeyLogged,"%d",message.operationMessage.semaphoreOperationsBuffer.clientNumber);
    strcat(loggingMessage,templateForSalaryAndKeyLogged);

    strcat(loggingMessage," and the proccess did not acquire the semaphore of key: ");
    sprintf(templateForSalaryAndKeyLogged,"%d",reqsemaphore);
    strcat(loggingMessage,templateForSalaryAndKeyLogged);

    strcat(loggingMessage," and it is now sleeping ZZZZZZZ..... \n");
            
    Log(loggingMessage,loggerMsgQid,loggerPID,loggerSharedMemoryID);
    strcpy(loggingMessage,"");
}
void logReleasing(int reqsemaphore)
{
    strcat(loggingMessage,"The client number: ");
    sprintf(templateForSalaryAndKeyLogged,"%d",message.operationMessage.semaphoreOperationsBuffer.clientNumber);
    strcat(loggingMessage,templateForSalaryAndKeyLogged);

    strcat(loggingMessage," released the semaphore of record with key: ");
    sprintf(templateForSalaryAndKeyLogged,"%d",reqsemaphore);
    strcat(loggingMessage,templateForSalaryAndKeyLogged);
    strcat(loggingMessage,"\n");
            
    Log(loggingMessage,loggerMsgQid,loggerPID,loggerSharedMemoryID);
    strcpy(loggingMessage,"");
}
void logAwakeningProcess(int processPID,int key)
{
    strcat(loggingMessage,"Process with PID: ");
    sprintf(templateForSalaryAndKeyLogged,"%d",processPID);
    strcat(loggingMessage,templateForSalaryAndKeyLogged);

    strcat(loggingMessage," has been awaken by the semaphore of key: ");
    sprintf(templateForSalaryAndKeyLogged,"%d",key);
    strcat(loggingMessage,templateForSalaryAndKeyLogged);
    strcat(loggingMessage,"\n");
            
    Log(loggingMessage,loggerMsgQid,loggerPID,loggerSharedMemoryID);
    strcpy(loggingMessage,"");
}

