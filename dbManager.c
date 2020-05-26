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



//MAIN Function.
int main(int argc, char*argv[]){

    int sharedMemoryId = atoi(argv[1]);
    int ManagerClientMessageQid = atoi(argv[2]);
    printf("%d\n", argc);
    printf("%s\n",argv[1]); 
    printf("The shared memory id is: %d \n",sharedMemoryId);
    printf("The message Q id id is: %d \n",ManagerClientMessageQid);
    
    printf("I am the dbManager\n");

}
