#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include "utils.h"
//#include "loggerFunctions.c"

#define configfileName "config.txt"
#define maxNumberOfCharToBeRead 1024
#define maxOperationsNumber 100
#define maxSalaryDigits 6
#define maxNameCharacters 20
#define maxIdDigits 6

int acquireRecordSemaphore(struct clientManagerMsgBuffer toSendMessage, int clientManagerMsgQ);
void releaseRecordSemaphore(struct clientManagerMsgBuffer toSendMessage, int clientManagerMsgQ);
void modifyRecord(struct clientManagerMsgBuffer toSendMessage);
int searchForAWord(char *wordToBeSearched);
void readFromALine(int lineNeeded, char *characterFound);
struct addRecordBuffer createAddRecordBuffer(char empSalaryChar[maxSalaryDigits], char empName[maxNameCharacters],int clientNumber);
struct modifyRecordBuffer createModifyRecordBuffer(char empSalaryChar[maxSalaryDigits], char empName[maxNameCharacters], char empIdChar[maxIdDigits], enum modifySalaryOperation salaryOperation);
struct retrieveBuffer createRetrievalRecordBuffer(char empSalaryChar[maxSalaryDigits], char empName[maxNameCharacters], enum salaryRetrieveOperation salaryOperation, enum nameRetrieveOperation nameOperation, enum retrieveOperation operation);
void addToQueryOutput(int keyToAdd,  char empNameToAdd[maxNameCharacters],int salaryToAdd);
int  acquireQueryLoggerSemaphore();
void releaseQueryLoggerSemaphore();
void logModification(int key,int clientNumber,int salary,enum modifySalaryOperation salaryOperation);

int dbManagerPID;
struct record *startOfTheSharedMemory;
struct record *startOfTheSharedMemory2;
int numberOfRecords;
int queryLoggerMsqQid;
int queryLoggerPID;
struct record queryOutput[1000];
int queryOutputCounter;
int loggerMsgQid;
int loggerPID;
int loggerSharedMemoryID; 
char loggingMessage[200];
char templateForSalaryAndKeyLogged[20];
int clientNumber;

int main(int argc, char *argv[])
{
    loggerSharedMemoryID = atoi(argv[7]);
    loggerMsgQid = atoi(argv[5]);
    loggerPID = atoi(argv[6]);
    clientNumber = atoi(argv[1]);
    int clientManagerMsgQ = atoi(argv[3]);
    dbManagerPID = atoi(argv[4]);
    int databaseSharedMemory = atoi(argv[2]);
    queryLoggerPID = atoi(argv[8]);
    queryLoggerMsqQid = atoi(argv[9]);
    char clientStart[8] = "client";
    char clientEnd[11] = "endClient";
    char clientNumberChar[2];
    int startingLineNumber;
    int endingLineNumber;
    char textBuffer[maxNumberOfCharToBeRead];
    struct AllOperations clientOperations[maxOperationsNumber];
    int operationCounter=0;

    strcpy(clientNumberChar,argv[1]);
    strcat(clientStart,clientNumberChar);
    strcat(clientEnd,clientNumberChar);
    
    //attaching shared memory
    startOfTheSharedMemory = shmat(databaseSharedMemory, NULL, 0); //Attchment to the shared memory to the record pointer.
    startOfTheSharedMemory2=startOfTheSharedMemory;


    startingLineNumber = searchForAWord(clientStart);
    endingLineNumber = searchForAWord(clientEnd);

    for (int lineCounter = startingLineNumber + 1; lineCounter < endingLineNumber; lineCounter++)
    {
        readFromALine(lineCounter, textBuffer);
        char empName[maxNameCharacters];
        char empSalaryChar[maxSalaryDigits];
        char empIdChar[maxIdDigits];
        char salaryIncreaseOrDecrease;
        int empSalary;
        int empId;
        if (sscanf(textBuffer, "Add  %s   %s", empName, empSalaryChar) != 0)
        {
            struct addRecordBuffer toAddBuffer = createAddRecordBuffer(empSalaryChar, empName,clientNumber);
            clientOperations[operationCounter].addBuffer = toAddBuffer;
            clientOperations[operationCounter].operationNeeded = add;
            operationCounter++;
        }
        else if (sscanf(textBuffer, "Modify  %s  %c%s", empIdChar, &salaryIncreaseOrDecrease, empSalaryChar) != 0)
        {
            struct modifyRecordBuffer toModifyBuffer;
            if (salaryIncreaseOrDecrease == '+')
            {
                toModifyBuffer = createModifyRecordBuffer(empSalaryChar, empName, empIdChar, increase);
            }
            else
            {
                toModifyBuffer = createModifyRecordBuffer(empSalaryChar, empName, empIdChar, decrease);
            }
            clientOperations[operationCounter].modifyBuffer = toModifyBuffer;
            clientOperations[operationCounter].operationNeeded = modify;
            operationCounter++;
        }
        else if((sscanf(textBuffer,"Retrieve Name:   %s  Salary: %s",empName, empSalaryChar)!=0) || (sscanf(textBuffer,"Retrieve NameStarts:   %s  Salary: %s",empName, empSalaryChar)!=0))
        {
            
            struct retrieveBuffer toRetrieveBuffer;
            enum nameRetrieveOperation nameOperation;
            enum retrieveOperation retrievalOperation;
            if (sscanf(textBuffer, "Retrieve Name:   %s  Salary: %s", empName, empSalaryChar) != 0)
            {
                
                if (strcmp(empName, "Any") == 0)
                {
                    nameOperation = nameNone;
                    retrievalOperation = nameAndSalary;
                }
                else if(strcmp(empName, "Any") != 0 & strcmp(empSalaryChar, "Any") == 0)
                {
                    nameOperation = fullName;
                    retrievalOperation = nameOnly;
                }
                else
                {
                    nameOperation = fullName;
                    retrievalOperation = salaryOnly;
                }
            }
            else if (sscanf(textBuffer, "Retrieve NameStarts:   %s  Salary: %s", empName, empSalaryChar) != 0)
            {
                if (strcmp(empName, "Any") == 0)
                {
                    nameOperation = nameNone;
                    retrievalOperation = nameAndSalary;
                }
                else
                {
                    nameOperation = nameContains;
                    retrievalOperation = salaryOnly;
                }
                
            }
      
            if(strcmp(empSalaryChar, "Any") != 0)
            {
            if (empSalaryChar[0] == '<' && empSalaryChar[1] != '=')
            {

                char empSalaryCharWithoutOperator[7];
                for (int character = 0; character < 7; character++)
                {
                    empSalaryCharWithoutOperator[character] = empSalaryChar[character + 1];
                }

                toRetrieveBuffer = createRetrievalRecordBuffer(empSalaryCharWithoutOperator, empName, smallerThan, nameOperation, retrievalOperation);
            }
            else if (empSalaryChar[0] == '>' && empSalaryChar[1] != '=')
            {
                char empSalaryCharWithoutOperator[7];
                for (int character = 0; character < 7; character++)
                {
                    empSalaryCharWithoutOperator[character] = empSalaryChar[character + 1];
                }

                toRetrieveBuffer = createRetrievalRecordBuffer(empSalaryCharWithoutOperator, empName, biggerThan, nameOperation, retrievalOperation);
            }
            else if (empSalaryChar[0] == '=')
            {

                char empSalaryCharWithoutOperator[7];
                for (int character = 0; character < 7; character++)
                {
                    empSalaryCharWithoutOperator[character] = empSalaryChar[character + 1];
                }

                toRetrieveBuffer = createRetrievalRecordBuffer(empSalaryCharWithoutOperator, empName, equal, nameOperation, retrievalOperation);
            }
            else if (empSalaryChar[0] == '>' && empSalaryChar[1] == '=')
            {

                char empSalaryCharWithoutOperator[6];
                for (int character = 0; character < 7; character++)
                {
                    empSalaryCharWithoutOperator[character] = empSalaryChar[character + 2];
                }

                toRetrieveBuffer = createRetrievalRecordBuffer(empSalaryCharWithoutOperator, empName, biggerThanOrEqual, nameOperation, retrievalOperation);
            }
            else if (empSalaryChar[0] == '<' && empSalaryChar[1] == '=')
            {
                char empSalaryCharWithoutOperator[6];
                for (int character = 0; character < 7; character++)
                {
                    empSalaryCharWithoutOperator[character] = empSalaryChar[character + 2];
                }

                toRetrieveBuffer = createRetrievalRecordBuffer(empSalaryCharWithoutOperator, empName, smallerThanOrEqual, nameOperation, retrievalOperation);
            }
            else
            {
                toRetrieveBuffer = createRetrievalRecordBuffer("None", empName, salaryNone, nameOperation, fullTable);
            }
        }
        else
        {
             toRetrieveBuffer = createRetrievalRecordBuffer("None", empName, salaryNone, nameOperation, nameOnly);
        }
        if(strcmp(empSalaryChar, "Any") == 0&strcmp(empName, "Any") == 0)
        {
            toRetrieveBuffer = createRetrievalRecordBuffer("None", empName, salaryNone, nameOperation, fullTable);
        }
        
        
            clientOperations[operationCounter].retrieveBuffer = toRetrieveBuffer;
            clientOperations[operationCounter].operationNeeded = retrieve;
            operationCounter++;   
        }
    }

    for (int operation=0; operation < operationCounter; operation++)
    {
        struct clientManagerMsgBuffer toSendMessage;
        int send_val;
        toSendMessage.mtype = dbManagerPID;
        toSendMessage.operationMessage = clientOperations[operation];
        struct additionSuccessMessageBuffer additionSuccessMessage;
        if (toSendMessage.operationMessage.operationNeeded == add)
        {
            //printf("Now sending add message: \n");
            send_val = msgsnd(clientManagerMsgQ, &toSendMessage, sizeof(toSendMessage.operationMessage), !IPC_NOWAIT);
            //printf("Message sent from child %d\n",getpid());
            int messageRecieveStatus = msgrcv(clientManagerMsgQ, &additionSuccessMessage, sizeof(additionSuccessMessage.key), getpid(), !IPC_NOWAIT);
            //printf("The key of the added record is %d\n", additionSuccessMessage.key);
            //printf("id:%d,name:%s,salary:%d\n", startOfTheSharedMemory->key, startOfTheSharedMemory->name, startOfTheSharedMemory->salary);
        }
    }
    for (int operation =0; operation < operationCounter; operation++)
    {
        struct clientManagerMsgBuffer toSendMessage;
        int send_val;
        toSendMessage.mtype = dbManagerPID;
        toSendMessage.operationMessage = clientOperations[operation];
        struct additionSuccessMessageBuffer additionSuccessMessage;
        if (toSendMessage.operationMessage.operationNeeded == modify)
        {
            int operationSucceeded = acquireRecordSemaphore(toSendMessage, clientManagerMsgQ);
            if (operationSucceeded)
            {
                modifyRecord(toSendMessage);
                releaseRecordSemaphore(toSendMessage, clientManagerMsgQ);
                logModification(toSendMessage.operationMessage.modifyBuffer.recordKey,clientNumber,toSendMessage.operationMessage.modifyBuffer.value,toSendMessage.operationMessage.modifyBuffer.salaryOperation);
            }
        }
    }

    
    for (int operation=0; operation < operationCounter; operation++)
    {
        if(clientOperations[operation].operationNeeded==retrieve)
        {

            int recordNumber=0;
            queryOutputCounter=0;
            while(startOfTheSharedMemory->key!=-1)
            {
                if(clientOperations[operation].retrieveBuffer.nameOperation==nameNone)
                {
                    //printf("Checking name\n");
                    addToQueryOutput(startOfTheSharedMemory->key,  startOfTheSharedMemory->name,startOfTheSharedMemory->salary);
                }
                else if(clientOperations[operation].retrieveBuffer.nameOperation==fullName)
                {
                    if(strcmp(clientOperations[operation].retrieveBuffer.name,startOfTheSharedMemory->name)==0)
                    {
                        addToQueryOutput(startOfTheSharedMemory->key,  startOfTheSharedMemory->name,startOfTheSharedMemory->salary);
                    }
                }
                else if(clientOperations[operation].retrieveBuffer.nameOperation==nameContains)
                {
                    if(strstr(startOfTheSharedMemory->name,clientOperations[operation].retrieveBuffer.name)!=0)
                    {
                        //printf("Found Starts with%s\n",startOfTheSharedMemory->name);
                        addToQueryOutput(startOfTheSharedMemory->key,  startOfTheSharedMemory->name,startOfTheSharedMemory->salary);
                    }
                }
                startOfTheSharedMemory+=sizeof(struct record);//Incrementing the pointer pointing to the shared memory by the size of the struct added.
                //recordNumber++;
            }
            startOfTheSharedMemory=startOfTheSharedMemory2;

            int queryArrayPointer=0;
            while(queryArrayPointer<queryOutputCounter)
            {
                if(clientOperations[operation].retrieveBuffer.salaryOperation==salaryNone)
                {
                    //printf("Checking salary\n");
                }
                else if(clientOperations[operation].retrieveBuffer.salaryOperation==equal)
                {
                    //printf("Checking Salary =\n");
                    if(queryOutput[queryArrayPointer].salary!=clientOperations[operation].retrieveBuffer.salary)
                    {
                        queryOutput[queryArrayPointer].key=-1;
                    }
                }
                else if(clientOperations[operation].retrieveBuffer.salaryOperation==smallerThan)
                {
                    if(queryOutput[queryArrayPointer].salary>=clientOperations[operation].retrieveBuffer.salary)
                    {
                        queryOutput[queryArrayPointer].key=-1;
                    }
                }
                else if(clientOperations[operation].retrieveBuffer.salaryOperation==biggerThan)
                {
                    if(queryOutput[queryArrayPointer].salary<=clientOperations[operation].retrieveBuffer.salary)
                    {
                        queryOutput[queryArrayPointer].key=-1;
                    }
                }
                else if(clientOperations[operation].retrieveBuffer.salaryOperation==smallerThanOrEqual)
                {
                    if(queryOutput[queryArrayPointer].salary>clientOperations[operation].retrieveBuffer.salary)
                    {
                        queryOutput[queryArrayPointer].key=-1;
                    }
                }
                else if(clientOperations[operation].retrieveBuffer.salaryOperation==biggerThanOrEqual)
                {
                    if(queryOutput[queryArrayPointer].salary<clientOperations[operation].retrieveBuffer.salary)
                    {
                        queryOutput[queryArrayPointer].key=-1;
                    }
                }
                queryArrayPointer++;
            }
            
            //Acquiring writing semphore
            int isDone=acquireQueryLoggerSemaphore();

            //Outputting Query
            if(isDone)
                queryLog(queryArrayPointer,clientOperations[operation].retrieveBuffer,queryOutput);
            //releasing semaphore   
            releaseQueryLoggerSemaphore();
        }
    }
    
    //Exiting from the client
    shmdt(startOfTheSharedMemory);
    return clientNumber;
}

void addToQueryOutput(int keyToAdd,  char empNameToAdd[maxNameCharacters],int salaryToAdd)
{
    struct record toAdd;
    toAdd.key=keyToAdd;
    strcpy(toAdd.name,empNameToAdd);
    toAdd.salary=salaryToAdd;
    queryOutput[queryOutputCounter]=toAdd;
    queryOutputCounter++;
}

int acquireRecordSemaphore(struct clientManagerMsgBuffer toSendMessage, int clientManagerMsgQ)
{
    struct semaphoreOperationsBuffer acquireBuffer;
    struct clientManagerMsgBuffer toAcquireMessage;
    struct operationSuccessMessageBuffer operationSuccessMessage;
    //acquiring semaphore
    acquireBuffer.recordKey = toSendMessage.operationMessage.modifyBuffer.recordKey;
    acquireBuffer.clientPID = getpid();
    acquireBuffer.clientNumber=clientNumber;
    toAcquireMessage.operationMessage.semaphoreOperationsBuffer = acquireBuffer;
    toAcquireMessage.mtype = dbManagerPID;
    toAcquireMessage.operationMessage.operationNeeded = acquire;
    int send_val = msgsnd(clientManagerMsgQ, &toAcquireMessage, sizeof(toAcquireMessage.operationMessage), !IPC_NOWAIT);

    //waiting for sempahore
    if (send_val > -1)
    {
        //printf("Message to acquire %d sent\n", toSendMessage.operationMessage.modifyBuffer.recordKey);
    }
    int messageRecieveStatus = msgrcv(clientManagerMsgQ, &operationSuccessMessage, sizeof(operationSuccessMessage.isOperationDone), getpid(), !IPC_NOWAIT);
    return operationSuccessMessage.isOperationDone;
}

void modifyRecord(struct clientManagerMsgBuffer toSendMessage)
{
    struct record *addressToBeModified = startOfTheSharedMemory + (toSendMessage.operationMessage.modifyBuffer.recordKey * sizeof(struct record));
    if (toSendMessage.operationMessage.modifyBuffer.salaryOperation == increase)
        addressToBeModified->salary += toSendMessage.operationMessage.modifyBuffer.value;

    else if (toSendMessage.operationMessage.modifyBuffer.salaryOperation == decrease)
        addressToBeModified->salary -= toSendMessage.operationMessage.modifyBuffer.value;
}

void releaseRecordSemaphore(struct clientManagerMsgBuffer toSendMessage, int clientManagerMsgQ)
{
    struct semaphoreOperationsBuffer releaseBuffer;
    struct clientManagerMsgBuffer toReleaseMessage;

    //acquiring semaphore
    releaseBuffer.recordKey = toSendMessage.operationMessage.modifyBuffer.recordKey;
    releaseBuffer.clientPID = getpid();
    releaseBuffer.clientNumber=clientNumber;
    toReleaseMessage.operationMessage.semaphoreOperationsBuffer = releaseBuffer;
    toReleaseMessage.mtype = dbManagerPID;
    toReleaseMessage.operationMessage.operationNeeded = release;
    int send_val = msgsnd(clientManagerMsgQ, &toReleaseMessage, sizeof(toReleaseMessage.operationMessage), !IPC_NOWAIT);
}

int  acquireQueryLoggerSemaphore()
{
    struct queryLoggerMsgBuffer acquireMessage;
    acquireMessage.neededoperation=acquire;
    acquireMessage.mtype=queryLoggerPID;
    acquireMessage.senderPID=getpid();
    acquireMessage.clientNumber=clientNumber;

    int send_val = msgsnd(queryLoggerMsqQid, &acquireMessage, sizeof(struct queryLoggerMsgBuffer)-sizeof(long), !IPC_NOWAIT);
    
    struct operationSuccessMessageBuffer operationSuccessMessage;
    int messageRecieveStatus = msgrcv(queryLoggerMsqQid, &operationSuccessMessage, sizeof(operationSuccessMessage.isOperationDone), getpid(), !IPC_NOWAIT); 
    return operationSuccessMessage.isOperationDone;
}

void releaseQueryLoggerSemaphore()
{
    struct queryLoggerMsgBuffer releaseMessage;
    releaseMessage.neededoperation=release;
    releaseMessage.mtype=queryLoggerPID;
    releaseMessage.senderPID=getpid();
    releaseMessage.clientNumber=clientNumber;

    int send_val = msgsnd(queryLoggerMsqQid, &releaseMessage, sizeof(struct queryLoggerMsgBuffer)-sizeof(long), !IPC_NOWAIT);
}

int searchForAWord(char *wordToBeSearched)
{
    FILE *configurationFile = fopen(configfileName, "r");
    int lineNumber = 1;
    int findedResult;
    char charactersInFile[maxNumberOfCharToBeRead];

    if (configurationFile == NULL)
    {

        //printf("File can't be opened\n");
    }
    else
    {
        while (fgets(charactersInFile, maxNumberOfCharToBeRead, configurationFile) != NULL)
        {
            if (strstr(charactersInFile, wordToBeSearched) != NULL)
            {
                return lineNumber;
            }
            lineNumber = lineNumber + 1;
        }
    }
}

void readFromALine(int lineNeeded, char *characterFound)
{
    FILE *configurationFile = fopen(configfileName, "r");
    int lineNumber = 1;
    int findedResult;
    char charactersInFile[maxNumberOfCharToBeRead];

    if (configurationFile == NULL)
    {

        //printf("Configuration file can't be opened\n");
    }
    else
    {
        while (fgets(charactersInFile, maxNumberOfCharToBeRead, configurationFile) != NULL)
        {
            if (lineNumber == lineNeeded)
            {
                for (int characterPlace = 0; characterPlace < maxNumberOfCharToBeRead; characterPlace++)
                {
                    characterFound[characterPlace] = charactersInFile[characterPlace];
                }
            }
            lineNumber = lineNumber + 1;
        }
    }
}

struct addRecordBuffer createAddRecordBuffer(char empSalaryChar[maxSalaryDigits], char empName[maxNameCharacters],int clientNumber)
{
    struct addRecordBuffer toAddBuffer;
    int empSalary = atoi(empSalaryChar);
    strcpy(toAddBuffer.name, empName);
    toAddBuffer.salary = empSalary;
    toAddBuffer.clientPID = getpid();
    toAddBuffer.clientNumber=clientNumber;

    return toAddBuffer;
}

struct modifyRecordBuffer createModifyRecordBuffer(char empSalaryChar[maxSalaryDigits], char empName[maxNameCharacters], char empIdChar[maxIdDigits], enum modifySalaryOperation salaryOperation)
{
    struct modifyRecordBuffer toModifyBuffer;
    int empSalary = atoi(empSalaryChar);
    int empId = atoi(empIdChar);

    toModifyBuffer.recordKey = empId;
    toModifyBuffer.salaryOperation = salaryOperation;
    toModifyBuffer.value = empSalary;
    toModifyBuffer.clientPID = getpid();
    return toModifyBuffer;
}
struct retrieveBuffer createRetrievalRecordBuffer(char empSalaryChar[maxSalaryDigits], char empName[maxNameCharacters], enum salaryRetrieveOperation salaryOperation, enum nameRetrieveOperation nameOperation, enum retrieveOperation operation)
{
    struct retrieveBuffer toRetrieveBuffer;
    int empSalary = atoi(empSalaryChar);

    strcpy(toRetrieveBuffer.name, empName);
    toRetrieveBuffer.operation = operation;
    toRetrieveBuffer.salary = empSalary;
    toRetrieveBuffer.salaryOperation = salaryOperation;
    toRetrieveBuffer.nameOperation = nameOperation;
    toRetrieveBuffer.clientPID = getpid();
    return toRetrieveBuffer;
}
void logModification(int key,int clientNumber,int salary,enum modifySalaryOperation salaryOperation)
{
    strcat(loggingMessage,"Client number ");
    sprintf(templateForSalaryAndKeyLogged,"%d",clientNumber);
    strcat(loggingMessage,templateForSalaryAndKeyLogged);
    if(salaryOperation==decrease)
    {
    strcat(loggingMessage," has deducted from the salary of the record: ");
    sprintf(templateForSalaryAndKeyLogged,"%d",key);
    strcat(loggingMessage,templateForSalaryAndKeyLogged);
    }
    else
    {
        strcat(loggingMessage," has added to the salary of the record: ");
    sprintf(templateForSalaryAndKeyLogged,"%d",key);
    strcat(loggingMessage,templateForSalaryAndKeyLogged);
    }

    strcat(loggingMessage," a value of: ");
    sprintf(templateForSalaryAndKeyLogged,"%d",salary);
    strcat(loggingMessage,templateForSalaryAndKeyLogged);

    strcat(loggingMessage,"\n");
            
    Log(loggingMessage,loggerMsgQid,loggerPID,loggerSharedMemoryID);
    strcpy(loggingMessage,"");
}