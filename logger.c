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
int main(){


FILE * LoggingOutputFile= fopen("LoggingOutputFile","w+");

time_t rawtime;
struct tm* timeInfo;
char timeBuffer [80];

//Putting the current date in a readable format
time(&rawtime);
timeInfo = localtime(&rawtime);
strftime(timeBuffer,80,"%c:\t", timeInfo);


//Printing the current time in the database
fputs(timeBuffer, LoggingOutputFile);



fclose(LoggingOutputFile);   
printf("I am the logger\n");

}
