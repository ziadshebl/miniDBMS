#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <limits.h>
#include "msgbuffers.h"
//#include "record.h"

#define configfileName "config.txt"
#define maxNumberOfCharToBeRead 1024
#define sizeOfMessageBuffer 28000 


int searchForAWord(char*wordToBeSearched);
void readFromALine(int lineNeeded, char*characterFound);



//MAIN Function.
int main(){
    char cwd[100];    
    getcwd(cwd, sizeof(cwd));
    char numberOfClientsCharacter[maxNumberOfCharToBeRead];
    char databaseSharedMemoryChar[10];
    char clientManagerMsgQidChar[10];
    char dbManagerPIDChar[5];
    int numberOfClients;
    int lineNumber;
    int totalNumberOfChildren;
    int pid;
    int clientManagerMsgQid;// The id for the buffer between client and the database manager
    int databaseSharedMemory;
    int dbManagerPID;
        //ftok to generate unique key 
    key_t key = ftok(cwd,65); 
    clientManagerMsgQid = msgget(IPC_PRIVATE, 0644); // Initiallizing the buffer between client and the database manager
    printf("The Message Buffer Id is:%d \n",clientManagerMsgQid);
    printf("the key is: %d \n ",key);

    databaseSharedMemory = shmget(key,1024,0666|IPC_CREAT); // shmget returns an identifier in shmid 
    printf("The Shared memory Id is: %d \n",databaseSharedMemory);

    //Reading the number of clients from the configuration file
    lineNumber = searchForAWord("noOfClients");
    readFromALine(lineNumber+1, numberOfClientsCharacter);
    numberOfClients = atoi(numberOfClientsCharacter);

    //Adding number of clients to number of other processes to get total number of children
    totalNumberOfChildren=numberOfClients+3;
    printf("%d\n",numberOfClients);


     for(int child=0; child<totalNumberOfChildren; child++){
        if(pid!=0 || child==0)
        {
            pid=fork(); 
            if(child==2)
            {
                dbManagerPID = pid;
            }
            if(child>2)
            {
                 numberOfClients=numberOfClients-1;
            }
            if(child==0 && pid==0){
                char *argv[] = {"logger.o", 0};
                execve(argv[0], &argv[0], NULL);
            }
            else if(child == 1 && pid==0)
            {
                char *argv[] = {"queryLogger.o", 0};
                execve(argv[0], &argv[0], NULL);
            }
            else if(child == 2 && pid==0)
            {
                dbManagerPID = pid;
                printf("ID: %d\n",dbManagerPID);
                sprintf(databaseSharedMemoryChar,"%d",databaseSharedMemory);
                sprintf(clientManagerMsgQidChar,"%d",clientManagerMsgQid);
                char *argv[] = {"dbManager.o", databaseSharedMemoryChar,clientManagerMsgQidChar,0};
                execve(argv[0], &argv[0], NULL);
            }
            else if(pid==0)
            {
                sprintf(databaseSharedMemoryChar,"%d",databaseSharedMemory);
                sprintf(clientManagerMsgQidChar,"%d",clientManagerMsgQid);
                sprintf(dbManagerPIDChar,"%d",dbManagerPID);
                char clientNumber[2];
                sprintf(clientNumber, "%d", numberOfClients+1);
                char *argv[] = {"dbClient.o",clientNumber,databaseSharedMemoryChar,clientManagerMsgQidChar,dbManagerPIDChar ,0};
                execve(argv[0], &argv[0], NULL);
                
            }
        }
    }


     for (int childNumber=0; childNumber<totalNumberOfChildren; childNumber++){
            int stat_loc;
            pid = wait(&stat_loc);
            //if(!(stat_loc & 0x00FF))
  	       // printf("\nA child with pid %d terminated with exit code %d\n", pid, stat_loc>>8);

      }

    sleep(5);    

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