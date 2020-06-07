#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/shm.h>

#define AcquireSemaphore 1
#define ReleaseSemaphore 0
#define EMPTY 0
#define FULL 1
#define LOCK 2
#define DEFAULT 3

void dumbMemory(struct record* memoryStartAddress,int numberOfEntries)
{
    struct record* currentAddress;
    printf("Current State of Memory:\n");
    for(int index=numberOfEntries;index<numberOfEntries;index++)
    {
        currentAddress=memoryStartAddress+index*sizeof(struct record);
        printf("........................................................\n");
        printf("Index is: %d\n Name is: %s \nSalary is: %d\n",currentAddress->key,currentAddress->name,currentAddress->salary);
    }
}

///////////////////////////////////////////////////////////////////////////////
//Query Logging Function
void queryLog(int numberOfRecords, struct retrieveBuffer requiredRetrieval,struct record records[100])
{
    char message[300]="The output of the query ";
    int name=0;
    int salary=0;
    //printf("REQUIRED # of records %d ,REQUIRED querlog operation is: %d \n",numberOfRecords,requiredRetrieval.operation);
    printf("Required Operation %d\n",requiredRetrieval.operation);

    switch(requiredRetrieval.operation)
    {
        case(nameAndSalary):
        {
            name=1;
            salary=1;
            break;
        }
        case(nameOnly):
        {
            name=1;
            break;
        }
        case(nameContainsOnly):
        {
            name=1;
            break;
        }
        case(nameContainsAndSalary):
        {
            name=1;
            salary=1;
            break;
        }
        case(salaryOnly):
        {
            printf("Helloooo.............................\n");
            salary=1;
            break;
        }
    }

    if(salary)
    {
        char salaryMessage[20]="on salary ";
        switch(requiredRetrieval.salaryOperation)
        {
            case(smallerThan):
            {
                strcat(salaryMessage,"smaller than ");
                break;
            }
            case(biggerThan):
            {
                strcat(salaryMessage,"bigger than ");
                break;
            }
            case(equal):
            {
                strcat(salaryMessage,"equals ");
                break;
            }
            case(smallerThanOrEqual):
            {
                strcat(salaryMessage,"smaller than or equal ");
                break;
            }
            case(biggerThanOrEqual):
            {
                strcat(salaryMessage,"bigger than or equal ");
                break;
            }
        }
        char salary[10];
        sprintf(salary,"%d",requiredRetrieval.salary);
        strcat(salaryMessage,salary);
        strcat(message,salaryMessage);
    }

    if(name & strcmp(requiredRetrieval.name,"Any")!=0)
    {
        if(salary)
            strcat(message," and ");

        char nameMessage[40]="";
        switch(requiredRetrieval.nameOperation)
        {
            
            case(fullName):
            {
                strcpy(nameMessage,"name equals: ");
                break;
            }
            case(nameContains):
            {
                strcpy(nameMessage,"name contains: ");
            }
        }
        strcat(nameMessage,requiredRetrieval.name);
        strcat(message,nameMessage);
    }

    if(!name & !salary)
        strcat(message,"to retrieve full table ");
    
    strcat(message," is: \n");

    FILE * queryLoggingFile = fopen("queryLoggingFile.txt","a");

    //outputting Query
    fflush(queryLoggingFile);
    fputs(message,queryLoggingFile);

    for(int index=0;index<numberOfRecords;index++)
    {
        if(records[index].key ==-1)
            continue;
            
        char recordMessage[100]="KEY: ";
        char key[5];
        sprintf(key,"%d",records[index].key);
        strcat(recordMessage,key);
        strcat(recordMessage," NAME: ");
        strcat(recordMessage,records[index].name);
        char salary[10];
        strcat(recordMessage,"  SALARY: ");
        sprintf(salary,"%d",records[index].salary);
        strcat(recordMessage,salary);
        strcat(recordMessage,"\n");

        fputs(recordMessage,queryLoggingFile);
    }
    fclose(queryLoggingFile);    
}

///////////////////////////////////////////////////////////////////////////////
//Logger functions
int SendMessageToAcquireSemaphore(int MsgQid, int RecieverPID,int SemaphoreType){

    struct msgbuff message;

    message.mtype = RecieverPID;
    message.SemaphoreStat = AcquireSemaphore;
    message.SenderPID = getpid();
    message.SemaphoreType = SemaphoreType;

    int send_val = msgsnd(MsgQid, &message, sizeof(message.SemaphoreStat)+sizeof(message.SenderPID)+sizeof(message.SemaphoreType), IPC_NOWAIT);

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

int SendMessageToReleaseSemaphore(int MsgQid, int RecieverPID, int SemaphoreType){

    struct msgbuff message;

    message.mtype = RecieverPID;
    message.SemaphoreStat = ReleaseSemaphore;
    message.SenderPID = getpid();
    message.SemaphoreType = SemaphoreType;

    int send_val = msgsnd(MsgQid, &message, sizeof(message.SemaphoreStat)+sizeof(message.SenderPID)+sizeof(message.SemaphoreType), IPC_NOWAIT);

    if (send_val == -1)
    {
        perror("Error in send");
        return -1;
    }else
    {
        return 0;
    }
    
}
int RecieveMessage(int MsgQid){

    struct msgbuff message;

    int rec_val = msgrcv(MsgQid, &message, sizeof(message.SemaphoreStat)+sizeof(message.SenderPID)+sizeof(message.SemaphoreType), getpid(), !IPC_NOWAIT);

    if (rec_val == -1)
    {
        
        return -1;
    }else
    {
       // printf("This is process %d. Message Recieved From Logger\n", getpid());
        return 0;
    }
    
}
void Log(char* SentLogMessage, int MsgQid, int LoggerPID, int loggerSharedMemoryID ){

    
    SendMessageToAcquireSemaphore(MsgQid, LoggerPID, EMPTY);
    RecieveMessage(MsgQid);

    
    SendMessageToAcquireSemaphore(MsgQid, LoggerPID, LOCK);
    RecieveMessage(MsgQid);

    //Write in shared memory
    struct loggerMsg* MemoryAddress =(struct loggerMsg*) shmat(loggerSharedMemoryID,NULL,0);
    strcpy(MemoryAddress->Msg,SentLogMessage);
    MemoryAddress->senderPID = getpid();
    //printf("this is process %d and written in memory is: %s\n",getpid() ,MemoryAddress->Msg);

    SendMessageToReleaseSemaphore(MsgQid,LoggerPID, LOCK);
    SendMessageToReleaseSemaphore(MsgQid,LoggerPID, FULL);
    RecieveMessage(MsgQid);
    shmdt(MemoryAddress);
}

////////////////////////////////////////////////////////////////////////////////
