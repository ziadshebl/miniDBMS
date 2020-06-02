#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<errno.h>
#include "msgbuffers.h"
//#include "record.h"

#define configfileName "config.txt"
#define maxNumberOfCharToBeRead 1024
#define sizeOfMessageBuffer 28000 
#define KEY 0x1234
#define KEY2 0x5678


int searchForAWord(char*wordToBeSearched);
void readFromALine(int lineNeeded, char*characterFound);



//MAIN Function.
int main(){
    char numberOfClientsCharacter[maxNumberOfCharToBeRead];
    char databaseSharedMemoryChar[10];
    char clientManagerMsgQidChar[10];
    char loggerMsgQidChar[10];
    char dbManagerPIDChar[5];
    char loggerPIDChar[5];
    char loggerSharedMemoryChar[10];
    int numberOfClients;
    int lineNumber;
    int totalNumberOfChildren;
    int pid;
    int clientManagerMsgQid;                    // The id for the buffer between client and the database manager
    int databaseSharedMemory;
    int loggerSharedMemory;
    int dbManagerPID;
    int loggerPID;
    int loggerMsgQid;                           //The id for the buffer between all processes and the logger

    loggerMsgQid = msgget(IPC_PRIVATE, 0644);   //Initalizing the buffer between all processes and the logger
    printf("The Logger Message Buffer Id is:%d \n",loggerMsgQid); 
     sprintf(loggerMsgQidChar,"%d",loggerMsgQid);  

    clientManagerMsgQid = msgget(IPC_PRIVATE, 0644); // Initiallizing the buffer between client and the database manager
    printf("The Client-Manager Message Buffer Id is:%d \n",clientManagerMsgQid);

    databaseSharedMemory = shmget(KEY,sizeOfMessageBuffer,0644|IPC_CREAT); // shmget returns an identifier in shmid 
    printf("The Shared memory Id is: %d \n",databaseSharedMemory);

    loggerSharedMemory = shmget(KEY2, sizeof(struct loggerMsg), 0644| IPC_CREAT );
    printf("The logger shared memory ID is: %d\n",loggerSharedMemory);
    sprintf(loggerSharedMemoryChar,"%d",loggerSharedMemory);

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
            if(child ==0)
            {
                loggerPID = pid;
                sprintf(loggerPIDChar,"%d",loggerPID);
                printf("The Logger ID is: %d \n", pid);
            }
            if(child==2)
            {
                dbManagerPID = pid;
            }
            if(child>2)
            {
                 numberOfClients=numberOfClients-1;
            }
            if(child==0 && pid==0){
                
                char *argv[] = {"logger.o",loggerMsgQidChar,loggerSharedMemoryChar ,0};
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
                char *argv[] = {"dbManager.o", databaseSharedMemoryChar,clientManagerMsgQidChar,loggerMsgQidChar,loggerPIDChar,loggerSharedMemoryChar,0};
                execve(argv[0], &argv[0], NULL);
            }
            else if(pid==0)
            {
                sprintf(databaseSharedMemoryChar,"%d",databaseSharedMemory);
                sprintf(clientManagerMsgQidChar,"%d",clientManagerMsgQid);
                sprintf(dbManagerPIDChar,"%d",dbManagerPID);
                char clientNumber[2];
                sprintf(clientNumber, "%d", numberOfClients+1);
                char *argv[] = {"dbClient.o",clientNumber,databaseSharedMemoryChar,clientManagerMsgQidChar,dbManagerPIDChar,loggerMsgQidChar,loggerPIDChar ,loggerSharedMemoryChar,0};
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