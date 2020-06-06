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
#include "utils.h"

#define configfileName "config.txt"
#define maxNumberOfCharToBeRead 1024
#define sizeOfMessageBuffer 28000 
#define KEY 0x1234
#define KEY2 0x100


int searchForAWord(char*wordToBeSearched);
void readFromALine(int lineNeeded, char*characterFound);
void terminateProcesses();

int numberOfClients;
int dbManagerPID;
int loggerPID;
int queryLoggerPID;
    
//MAIN Function.
int main(){
    char numberOfClientsCharacter[maxNumberOfCharToBeRead];

    char databaseSharedMemoryChar[20];
    char loggerSharedMemoryChar[20];
    char queryLoggerMsqQidChar[20];

    char clientManagerMsgQidChar[20];
    char loggerMsgQidChar[20];

    char dbManagerPIDChar[20];
    char queryLoggerPIDChar[20];
    char loggerPIDChar[20];
  
    int databaseSharedMemory;
    int loggerSharedMemory;
    int loggerMsgQid;
    int queryLoggerMsgQid;
    int clientManagerMsgQid; 
    int shmkey;
      
    int lineNumber;
    int totalNumberOfChildren;
    int pid;

    queryLoggerMsgQid=msgget(IPC_PRIVATE, 0644);
    sprintf(queryLoggerMsqQidChar,"%d",queryLoggerMsgQid);  

    loggerMsgQid = msgget(IPC_PRIVATE, 0644);   //Initalizing the buffer between all processes and the logger
    sprintf(loggerMsgQidChar,"%d",loggerMsgQid);  

    clientManagerMsgQid = msgget(IPC_PRIVATE, 0644); // Initiallizing the buffer between client and the database manager
    sprintf(clientManagerMsgQidChar,"%d",clientManagerMsgQid);

    databaseSharedMemory = shmget(KEY,sizeOfMessageBuffer,0644|IPC_CREAT); // shmget returns an identifier in shmid
    sprintf(databaseSharedMemoryChar,"%d",databaseSharedMemory); 
    fflush(NULL);

    shmkey = ftok("shmfile",65);
    loggerSharedMemory = shmget(KEY2, 10 * sizeof(struct loggerMsg), 0644| IPC_CREAT );
    printf("The logger shared memory ID is: %d\n",loggerSharedMemory);
    sprintf(loggerSharedMemoryChar,"%d",loggerSharedMemory);

    //Reading the number of clients from the configuration file
    lineNumber = searchForAWord("noOfClients");
    readFromALine(lineNumber+1, numberOfClientsCharacter);
    numberOfClients = atoi(numberOfClientsCharacter);
    
    //Adding number of clients to number of other processes to get total number of children
    totalNumberOfChildren=numberOfClients+3;
    printf("Number of clients are%d\n",numberOfClients);

    //Fokring children
    //Forking logger
    pid=fork();
    if(pid== 0)
    {
        char *argv[] = {"logger.o",loggerMsgQidChar,loggerSharedMemoryChar ,0};
        execve(argv[0], &argv[0], NULL);
    }
    else
    {
        loggerPID = pid;
        sprintf(loggerPIDChar,"%d",loggerPID);
        printf("The Logger ID is: %d, logger d in string: %s \n", pid,loggerPIDChar);
    }

    //Forking QueryLogger
    pid=fork();
    if(pid==0)
    {
        char *argv[] = {"queryLogger.o",queryLoggerMsqQidChar,loggerMsgQidChar,loggerPIDChar,loggerSharedMemoryChar,0};
        execve(argv[0], &argv[0], NULL);
    }
    else
    {
        queryLoggerPID = pid;
        sprintf(queryLoggerPIDChar,"%d",queryLoggerPID);
    }

    //Forking DB manager
    pid=fork();
    if(pid==0)
    {
        sprintf(databaseSharedMemoryChar,"%d",databaseSharedMemory); 
        char *argv[] = {"dbManager.o", databaseSharedMemoryChar,clientManagerMsgQidChar,loggerMsgQidChar,loggerPIDChar,loggerSharedMemoryChar,0};
        execve(argv[0], &argv[0], NULL);
    }
    else
    {
        dbManagerPID = pid;
        sprintf(dbManagerPIDChar,"%d",dbManagerPID);
    }

    //Forking clients
    for(int i=0;i<numberOfClients;i++)
    {
        pid=fork();
        if(pid==0)
        {
            char clientNumber[2];
            sprintf(clientNumber, "%d", i+1);
            char *argv[] = {"dbClient.o",clientNumber,databaseSharedMemoryChar,clientManagerMsgQidChar,dbManagerPIDChar,
            loggerMsgQidChar,loggerPIDChar ,loggerSharedMemoryChar,queryLoggerPIDChar,queryLoggerMsqQidChar,0};
            execve(argv[0], &argv[0], NULL);
        }
        else
        {
            //printf("Client %d PID is: %d",i+1,pid);
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

void terminateProcesses()
{
    kill(loggerPID,SIGTERM);
    kill(dbManagerPID,SIGTERM);
    kill(queryLoggerPID,SIGTERM);
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