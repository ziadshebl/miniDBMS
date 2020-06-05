#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include<sys/shm.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include "loggerFunctions.c"
//#include "record.h"


#define configfileName "config.txt"
#define maxNumberOfCharToBeRead 1024
#define maxOperationsNumber 100
#define maxSalaryDigits 6
#define maxNameCharacters 20
#define maxIdDigits 6



int acquireSemaphore(struct clientManagerMsgBuffer toSendMessage,int clientManagerMsgQ);
void releaseSemaphore(struct clientManagerMsgBuffer toSendMessage,int clientManagerMsgQ);
void modifyRecord(struct clientManagerMsgBuffer toSendMessage);
int searchForAWord(char*wordToBeSearched);
void readFromALine(int lineNeeded, char*characterFound);
struct addRecordBuffer createAddRecordBuffer (char empSalaryChar[maxSalaryDigits], char empName[maxNameCharacters]);
struct modifyRecordBuffer createModifyRecordBuffer (char empSalaryChar[maxSalaryDigits], char empName[maxNameCharacters], char empIdChar[maxIdDigits],enum modifySalaryOperation salaryOperation);

int dbManagerPID;
struct record * databaseMemoryBegining;

int main(int argc, char*argv[])
{   int loggerSharedMemoryID = atoi(argv[7]);
    int loggerMsgQid = atoi(argv[5]);
    int loggerPID = atoi(argv[6]);
    int clientNumber = atoi(argv[1]);
    int clientManagerMsgQ = atoi(argv[3]);
    dbManagerPID = atoi(argv[4]);
    int databaseSharedMemory = atoi (argv[2]);
    char clientStart[8]="client";
    char clientEnd[11]="endClient";
    char clientNumberChar[2];
    int startingLineNumber;
    int endingLineNumber;
    char textBuffer[maxNumberOfCharToBeRead];
    struct AllOperations clientOperations[maxOperationsNumber];
    int operationCounter=0;
    char logMsg[100] = "this is client testing log function..";
    strcpy(clientNumberChar,argv[1]);
    strcat(clientStart,clientNumberChar);
    strcat(clientEnd,clientNumberChar);

    printf("I am the client ,manager PID is %d and logger shm ID: %d, datbase shm ID: %d\n",dbManagerPID,loggerSharedMemoryID,databaseSharedMemory);
    //attaching shared memory
    databaseMemoryBegining =shmat(databaseSharedMemory,NULL,0);//Attchment to the shared memory to the record pointer.

    //Log(clientStart, loggerMsgQid, loggerPID);
    printf("I am the client and logger shared memory ID is %d\n",loggerSharedMemoryID);
    printf("I am the client and logger msgqid ID is %d\n",loggerMsgQid);
    printf("I am the client and logger ID is %d\n",loggerPID);
    startingLineNumber = searchForAWord(clientStart);
    endingLineNumber = searchForAWord(clientEnd);
    
    for (int lineCounter=startingLineNumber+1; lineCounter<endingLineNumber; lineCounter++)
    {
        readFromALine(lineCounter,textBuffer);
        char empName[maxNameCharacters];
        char empSalaryChar[maxSalaryDigits];
        char empIdChar[maxIdDigits];
        char salaryIncreaseOrDecrease;
        int empSalary;
        int empId;
        if(sscanf(textBuffer,"Add  %s   %s",empName, empSalaryChar)!=0)
        {
            struct addRecordBuffer toAddBuffer = createAddRecordBuffer (empSalaryChar, empName);
            clientOperations[operationCounter].addBuffer=toAddBuffer;
            clientOperations[operationCounter].operationNeeded = add;
            operationCounter++;

        }
        else if(sscanf(textBuffer,"Modify  %s  %c%s",empIdChar,&salaryIncreaseOrDecrease ,empSalaryChar)!=0)
        {
            struct modifyRecordBuffer toModifyBuffer;
            if(salaryIncreaseOrDecrease=='+')
            {
                 toModifyBuffer = createModifyRecordBuffer (empSalaryChar, empName, empIdChar,increase);
                 
            }
            else 
            {
                 toModifyBuffer = createModifyRecordBuffer (empSalaryChar, empName, empIdChar,decrease);
                
            }
            clientOperations[operationCounter].modifyBuffer=toModifyBuffer;
            clientOperations[operationCounter].operationNeeded = modify;
            operationCounter++;
        }
        else if(sscanf(textBuffer,"Retrieve Name:   %s  Salary: %s",empName, empSalaryChar)!=0)
        {
            if(empSalaryChar[0]=='<' && empSalaryChar[1]!='=')
            {
                struct retrieveBuffer toRetrieveBuffer;
                char empSalaryCharWithoutOperator[7];
                for (int character=0; character<7; character++){
                    empSalaryCharWithoutOperator[character]=empSalaryChar[character+1];
                } 
                empSalary = atoi(empSalaryCharWithoutOperator);
                strcpy(toRetrieveBuffer.name, empName);
                toRetrieveBuffer.salaryOperation=smallerThan;
                toRetrieveBuffer.salary=empSalary;
                if(strcmp(empName, "Any")==0)
                {
                    toRetrieveBuffer.operation=salaryOnly;
                    
                }
                else
                {
                    toRetrieveBuffer.operation=nameAndSalary;
                }
                clientOperations[operationCounter].retrieveBuffer=toRetrieveBuffer;
                operationCounter++;
                
               
            }
            else if(empSalaryChar[0]=='>' && empSalaryChar[1]!='=')
            {
                struct retrieveBuffer toRetrieveBuffer;
                char empSalaryCharWithoutOperator[7];
                for (int character=0; character<7; character++){
                    empSalaryCharWithoutOperator[character]=empSalaryChar[character+1];
                }
                empSalary = atoi(empSalaryCharWithoutOperator);
                strcpy(toRetrieveBuffer.name, empName);
                toRetrieveBuffer.salaryOperation=biggerThan;
                toRetrieveBuffer.salary=empSalary;
                if(strcmp(empName, "Any")==0)
                {
                    toRetrieveBuffer.operation=salaryOnly;
                }
                else
                {
                    toRetrieveBuffer.operation=nameAndSalary;
                }
                clientOperations[operationCounter].retrieveBuffer=toRetrieveBuffer;
                operationCounter++;
                
            }
            else if(empSalaryChar[0]=='=')
            {
                struct retrieveBuffer toRetrieveBuffer;
                char empSalaryCharWithoutOperator[7];
                for (int character=0; character<7; character++){
                    empSalaryCharWithoutOperator[character]=empSalaryChar[character+1];
                }
                empSalary = atoi(empSalaryCharWithoutOperator);
                strcpy(toRetrieveBuffer.name, empName);
                toRetrieveBuffer.salaryOperation=equal;
                toRetrieveBuffer.salary=empSalary;
                if(strcmp(empName, "Any")==0)
                {
                    toRetrieveBuffer.operation=salaryOnly;
                   
                }
                else
                {
                    toRetrieveBuffer.operation=nameAndSalary;
               
                }
                clientOperations[operationCounter].retrieveBuffer=toRetrieveBuffer;
                operationCounter++;
                
                
            }
            else if(empSalaryChar[0]=='>' && empSalaryChar[1]=='=')
            {
                struct retrieveBuffer toRetrieveBuffer;
                char empSalaryCharWithoutOperator[6];
                for (int character=0; character<7; character++){
                    empSalaryCharWithoutOperator[character]=empSalaryChar[character+2];
                }
                empSalary = atoi(empSalaryCharWithoutOperator);
                strcpy(toRetrieveBuffer.name, empName);
                toRetrieveBuffer.salaryOperation=biggerThanOrEqual;
                toRetrieveBuffer.salary=empSalary;
                if(strcmp(empName, "Any")==0)
                {
                    toRetrieveBuffer.operation=salaryOnly;
                    
                }
                else
                {
                    toRetrieveBuffer.operation=nameAndSalary;
     
                }
                clientOperations[operationCounter].retrieveBuffer=toRetrieveBuffer;
                operationCounter++;
                
                
            }
            else if(empSalaryChar[0]=='<' && empSalaryChar[1]=='=')
            {

                struct retrieveBuffer toRetrieveBuffer;
                char empSalaryCharWithoutOperator[6];
                for (int character=0; character<7; character++){
                    empSalaryCharWithoutOperator[character]=empSalaryChar[character+2];
                }
                empSalary = atoi(empSalaryCharWithoutOperator);
                strcpy(toRetrieveBuffer.name, empName);
                toRetrieveBuffer.salaryOperation=smallerThanOrEqual;
                toRetrieveBuffer.salary=empSalary;
                if(strcmp(empName, "Any")==0)
                {
                    toRetrieveBuffer.operation=salaryOnly;
                    
                }
                else
                {
                    toRetrieveBuffer.operation=nameAndSalary;
                  
                }
                clientOperations[operationCounter].retrieveBuffer=toRetrieveBuffer;
                operationCounter++;
                
            }
            else if(strcmp(empName, "Any")==0)
            {
                struct retrieveBuffer toRetrieveBuffer;
                toRetrieveBuffer.salaryOperation=none;
                if(empName=="Any")
                {
                    toRetrieveBuffer.operation=fullTable;
                }
            }

        }
        else if(sscanf(textBuffer,"Retrieve NameStarts:   %s  Salary: %s",empName, empSalaryChar)!=0)
        {
     

        }
    }


    for (int operation; operation< operationCounter; operation++)
    {
        struct clientManagerMsgBuffer toSendMessage;
        int send_val;
        toSendMessage.mtype = dbManagerPID;
        toSendMessage.operationMessage=clientOperations[operation];
        struct additionSuccessMessageBuffer additionSuccessMessage;
        if(toSendMessage.operationMessage.operationNeeded==add)
        {
            printf("Now sending add message: \n");
            send_val = msgsnd(clientManagerMsgQ, &toSendMessage, sizeof(toSendMessage.operationMessage), !IPC_NOWAIT);
            int messageRecieveStatus =msgrcv(clientManagerMsgQ, &additionSuccessMessage, sizeof(additionSuccessMessage.key), getpid(), !IPC_NOWAIT);
            printf("The key of the added record is %d\n", additionSuccessMessage.key);
            printf("id:%d,name:%s,salary:%d\n",databaseMemoryBegining->key,databaseMemoryBegining->name,databaseMemoryBegining->salary);
        }
        else if(toSendMessage.operationMessage.operationNeeded==modify)
        {
            int operationSucceeded=acquireSemaphore(toSendMessage,clientManagerMsgQ);
            if(operationSucceeded)
            {
                modifyRecord(toSendMessage);
                releaseSemaphore(toSendMessage,clientManagerMsgQ);
            }
        }
            
    }
    
Log(logMsg, loggerMsgQid, loggerPID, loggerSharedMemoryID);
}

int acquireSemaphore(struct clientManagerMsgBuffer toSendMessage,int clientManagerMsgQ)
{
    struct semaphoreOperationsBuffer acquireBuffer;
    struct clientManagerMsgBuffer toAcquireMessage;
    struct operationSuccessMessageBuffer operationSuccessMessage;
    //acquiring semaphore
    acquireBuffer.recordKey = toSendMessage.operationMessage.modifyBuffer.recordKey;
    acquireBuffer.clientPID=getpid();
    toAcquireMessage.operationMessage.semaphoreOperationsBuffer=acquireBuffer;
    toAcquireMessage.mtype=dbManagerPID;
    toAcquireMessage.operationMessage.operationNeeded=acquire;
    int send_val = msgsnd(clientManagerMsgQ, &toAcquireMessage, sizeof(toAcquireMessage.operationMessage), !IPC_NOWAIT);
           
    //waiting for sempahore 
    if(send_val > -1){
    //printf("Message to acquire %d sent\n", toSendMessage.operationMessage.modifyBuffer.recordKey);
    }
    int messageRecieveStatus =msgrcv(clientManagerMsgQ, &operationSuccessMessage, sizeof(operationSuccessMessage.isOperationDone), getpid(), !IPC_NOWAIT);
    return operationSuccessMessage.isOperationDone;
}

void modifyRecord(struct clientManagerMsgBuffer toSendMessage)
{
    struct record *addressToBeModified=databaseMemoryBegining+( toSendMessage.operationMessage.modifyBuffer.recordKey * sizeof(struct record));
    if(toSendMessage.operationMessage.modifyBuffer.salaryOperation==increase) 
        addressToBeModified->salary+=toSendMessage.operationMessage.modifyBuffer.value;

    else if(toSendMessage.operationMessage.modifyBuffer.salaryOperation==decrease) 
       addressToBeModified->salary-=toSendMessage.operationMessage.modifyBuffer.value;

    printf("\n");
    printf("The key to be edited  is: %d \n",addressToBeModified->key);
    printf("The salary  after editing is: %d \n",addressToBeModified->salary);
    printf("The name is: %s \n",addressToBeModified->name);
    printf("Edited...\n");
    printf("\n");
}

void releaseSemaphore(struct clientManagerMsgBuffer toSendMessage,int clientManagerMsgQ)
{
    struct semaphoreOperationsBuffer releaseBuffer;
    struct clientManagerMsgBuffer toReleaseMessage;

    //acquiring semaphore
    releaseBuffer.recordKey = toSendMessage.operationMessage.modifyBuffer.recordKey;
    releaseBuffer.clientPID=getpid();
    toReleaseMessage.operationMessage.semaphoreOperationsBuffer=releaseBuffer;
    toReleaseMessage.mtype=dbManagerPID;
    toReleaseMessage.operationMessage.operationNeeded=release;
    int send_val = msgsnd(clientManagerMsgQ, &toReleaseMessage, sizeof(toReleaseMessage.operationMessage), !IPC_NOWAIT);

    //waiting for sempahore 
    if(send_val > -1){
    //printf("Message to acquire %d sent\n", toSendMessage.operationMessage.modifyBuffer.recordKey);
    }
}

int searchForAWord(char*wordToBeSearched)
{
    FILE * configurationFile=fopen(configfileName, "r");
    int lineNumber=1;
    int findedResult;
    char charactersInFile [maxNumberOfCharToBeRead];


    if(configurationFile==NULL)
    {

        printf("File can't be opened\n");

    }
    else
    {
        while(fgets(charactersInFile, maxNumberOfCharToBeRead, configurationFile)!=NULL)
        {
            if(strstr(charactersInFile, wordToBeSearched)!=NULL)
            {
                return lineNumber;
            }
            lineNumber=lineNumber+1;
        }
    }
    

}

void readFromALine(int lineNeeded, char*characterFound)
{
    FILE * configurationFile=fopen(configfileName, "r");
    int lineNumber=1;
    int findedResult;
    char charactersInFile [maxNumberOfCharToBeRead];


    if(configurationFile==NULL)
    {

        printf("Configuration file can't be opened\n");

    }
    else
    {
        while(fgets(charactersInFile, maxNumberOfCharToBeRead, configurationFile)!=NULL)
        {
            if(lineNumber==lineNeeded)
            {
                for(int characterPlace=0; characterPlace < maxNumberOfCharToBeRead; characterPlace++)
                {
                    characterFound[characterPlace] = charactersInFile[characterPlace];
                }
            }
            lineNumber=lineNumber+1;
        }
    }

}


struct addRecordBuffer createAddRecordBuffer (char empSalaryChar[maxSalaryDigits], char empName[maxNameCharacters])
{
    struct addRecordBuffer toAddBuffer;
    int empSalary=atoi(empSalaryChar);      
    strcpy(toAddBuffer.name,empName);
    toAddBuffer.salary=empSalary;
    toAddBuffer.clientPID=getpid();

    return toAddBuffer;

}


struct modifyRecordBuffer createModifyRecordBuffer (char empSalaryChar[maxSalaryDigits], char empName[maxNameCharacters], char empIdChar[maxIdDigits],enum modifySalaryOperation salaryOperation)
{
    struct modifyRecordBuffer toModifyBuffer;
    int empSalary=atoi(empSalaryChar);
    int empId=atoi(empIdChar);


    toModifyBuffer.recordKey=empId;
    toModifyBuffer.salaryOperation=salaryOperation;
    toModifyBuffer.value=empSalary;
    toModifyBuffer.clientPID=getpid();
    return toModifyBuffer;
}