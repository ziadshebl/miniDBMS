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


    printf("%d\n", argc);
    printf("%s\n",argv[1]); 
    
    
    printf("I am the dbManager\n");

}
