#include "utils.h"
#include <stdio.h>

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
