#include "utils.h"
#include <stdio.h>
#include <string.h>

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


void queryLog(int numberOfRecords, struct retrieveBuffer requiredRetrieval,struct record records[100])
{
    char message[5000]="This is the output of the query operation ";
    int name=0;
    int salary=0;
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
                strcat(salaryMessage,"smaller than");
                break;
            }
            case(biggerThan):
            {
                strcat(salaryMessage,"smaller bigger than");
                break;
            }
            case(equal):
            {
                strcat(salaryMessage,"equal");
                break;
            }
            case(smallerThanOrEqual):
            {
                strcat(salaryMessage,"smaller than or equal");
                break;
            }
            case(biggerThanOrEqual):
            {
                strcat(salaryMessage,"bigger than or equal");
                break;
            }
        }
        strcat(message,salaryMessage);
    }

    if(name)
    {
        if(salary)
            strcat(message,"and");

        char nameMessage[40]="";
        switch(requiredRetrieval.nameOperation)
        {
            
            case(fullName):
            {
                strcpy(nameMessage,"name equals: ");
                strcat(nameMessage,requiredRetrieval.name);
                break;
            }
            case(nameContains):
            {
                strcpy(nameMessage,"name contains: ");
                strcat(nameMessage,requiredRetrieval.name);
            }
        }
        strcat(message,nameMessage);
    }

    if(!name & !salary)
        strcat(message,"to retrieve full table ");
    
    strcat(message,"is: \n");

    //either output here or output after message end

    for(int index=0;index<numberOfRecords;index++)
    {
        char recordMessage[100]="Key: ";
        char key[5];
        sprintf(key,"%d",records[index].key);
        strcat(recordMessage,key);
        strcat(recordMessage,", name: ");
        strcat(recordMessage,records[index].name);
        char salary[10];
        sprintf(salary,"%d",records[index].salary);
        strcat(recordMessage,salary);
        strcat(recordMessage,"\n");

        //comment this to output once every record
        strcat(message,recordMessage);
        //either output here or after message end
    }
    //output here:
        
}

