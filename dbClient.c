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


#define configfileName "config.txt"
#define maxNumberOfCharToBeRead 1024
#define maxOperationsNumber 100


int searchForAWord(char*wordToBeSearched);
void readFromALine(int lineNeeded, char*characterFound);



//MAIN Function.
int main(int argc, char*argv[])
{
    int clientNumber = atoi(argv[1]);
    int clientManagerMsgQ = atoi(argv[3]);
    int dbManagerPID = atoi(argv[4]);
    int databaseSharedMemory = atoi (argv[2]);
    char clientStart[8]="client";
    char clientEnd[11]="endClient";
    char clientNumberChar[2];
    int startingLineNumber;
    int endingLineNumber;
    char textBuffer[maxNumberOfCharToBeRead];
    struct AllOperations clientOperations[maxOperationsNumber];
    int operationCounter=0;
    strcpy(clientNumberChar,argv[1]);
    strcat(clientStart,clientNumberChar);
    strcat(clientEnd,clientNumberChar);
    printf("dbID: %d\n", dbManagerPID);
    

    startingLineNumber = searchForAWord(clientStart);
    endingLineNumber = searchForAWord(clientEnd);

    for (int lineCounter=startingLineNumber+1; lineCounter<endingLineNumber; lineCounter++)
    {
        readFromALine(lineCounter,textBuffer);
        printf("Client%s: %s",clientNumberChar,textBuffer);
        char empName[15];
        char empSalaryChar[8];
        char empIdChar[6];
        char salaryIncreaseOrDecrease;
        int empSalary;
        int empId;
        if(sscanf(textBuffer,"Add  %s   %s",empName, empSalaryChar)!=0)
        {
            struct addRecordBuffer toAddBuffer;
            empSalary=atoi(empSalaryChar);
            
            strcpy(toAddBuffer.name,empName);
            toAddBuffer.salary=empSalary;
            toAddBuffer.clientNumber=clientNumber;
            toAddBuffer.mtype=dbManagerPID;
            
            clientOperations[operationCounter].addBuffer=toAddBuffer;
            clientOperations[operationCounter].operationNeeded = add;
            operationCounter++;

            printf("EmpName: %s with %d will be added from client %d\n", empName, empSalary, clientNumber);

        }
        else if(sscanf(textBuffer,"Modify  %s  %c%s",empIdChar,&salaryIncreaseOrDecrease ,empSalaryChar)!=0)
        {
            if(salaryIncreaseOrDecrease=='+')
            {
                struct modifyRecordBuffer toModifyBuffer;
                empSalary=atoi(empSalaryChar);
                empId=atoi(empIdChar);

                toModifyBuffer.recordKey=empId;
                toModifyBuffer.salaryOperation=increase;
                toModifyBuffer.value=empSalary;

                 clientOperations[operationCounter].modifyBuffer=toModifyBuffer;
                 operationCounter++;
                printf("Employe %s will be increased by %d\n", empIdChar, empSalary);
            }
            else 
            {
                struct modifyRecordBuffer toModifyBuffer;
                empSalary=atoi(empSalaryChar);
                empId=atoi(empIdChar);

                toModifyBuffer.recordKey=empId;
                toModifyBuffer.salaryOperation=decrease;
                toModifyBuffer.value=empSalary;

                clientOperations[operationCounter].modifyBuffer=toModifyBuffer;
                operationCounter++;
                printf("Employe %s will be decreased by %d\n", empIdChar, empSalary);
            }

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
                    printf("SALARY ONLY\n");
                }
                else
                {
                    toRetrieveBuffer.operation=nameAndSalary;
                    printf("SALARY AND NAME=%s\n", toRetrieveBuffer.name);
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
                    //printf("SALARY ONLY\n");
                }
                else
                {
                    toRetrieveBuffer.operation=nameAndSalary;
                    //printf("SALARY AND NAME=%s\n", toRetrieveBuffer.name);
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
                    //printf("SALARY ONLY\n");
                }
                else
                {
                    toRetrieveBuffer.operation=nameAndSalary;
                    //printf("SALARY AND NAME=%s\n", toRetrieveBuffer.name);
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
                    //printf("SALARY ONLY\n");
                }
                else
                {
                    toRetrieveBuffer.operation=nameAndSalary;
                    //printf("SALARY AND NAME=%s\n", toRetrieveBuffer.name);
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
            
            printf("Employee starting with %s with salary %s will be retrieved\n", empName, empSalaryChar);

        }
    }


    for (int operation; operation< operationCounter; operation++)
    {
        struct clientManagerMsgBuffer toSend;
        int send_val;
        toSend.mtype = dbManagerPID;
        toSend.operationMessage=clientOperations[operation];
        send_val = msgsnd(clientManagerMsgQ, &toSend, sizeof(toSend.operationMessage), !IPC_NOWAIT);
        if(send_val < 0)
        {

        }
        else
        {
            printf("MESSAGE SENT SUCCESSFULLY\n");
        }
        
    }
    //printf("Name: %s    Salary:  %d     Client: %d\n",clientOperations[0].addBuffer.name, clientOperations[0].addBuffer.salary, clientOperations[0].addBuffer.clientNumber);





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
