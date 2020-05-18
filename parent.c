#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include "msgbuffers.h"



//Signal Handler Functions.
void handler(int signum);
void handler2(int signum);


//PIDs of all processes.
int  PIDs [5];
int leader=-1;







//Message queue id.
key_t msgqid;

//Received value.
int rec_val;

//Received message.
//struct msgbuff messageRCVD;


//Indicates that the process has the IDs of other processes.
int IDsRCVD=0;

//MAIN Function.
int main(){
   
    int send_val;
    int pid, stat_loc;
    int childNum=1;


    //Overwriting the handlers.
    signal(SIGUSR1, handler);
    signal(SIGUSR2, handler2);

    

    //Creating a new message queue.
    msgqid = msgget(IPC_PRIVATE, 0644); // or msgget(12613, IPC_CREATE | 0644)
    if(msgqid == -1)
    {	
        perror("Error in create");
        exit(-1);
    }
    printf("The msgqid of the queue used in this program is %d\n", msgqid);



    //Forking 4 more processes and filling the PIDs array.
    PIDs[0]=getpid();

    printf("Child %d forked\n", childNum);
    childNum=childNum+1;
    pid = fork();
    PIDs[1]=pid;


    for(int i=0; i<3; i++){
        if(pid!=0){ 
            printf("Child %d forked\n", childNum);
            childNum=childNum+1;   
            pid=fork();
                if(pid!=0){
                    PIDs[i+2]=pid;
                }
        }
    }


    //As a parent, making sure that all the children have the full PIDs array.
    if(pid!=0){
        struct msgbuff message;
        IDsRCVD=1;

        for(int i=0; i<5;i++){
            message.messagePID[i]=PIDs[i];
        }

        for(int i=0; i<4; i++){
            sleep(1);
            send_val = msgsnd(msgqid, &message, sizeof(message.messagePID), !IPC_NOWAIT);
            kill((PIDs[i+1]), SIGUSR1);
        }
    }
    else{
        while(IDsRCVD==0){
            
        }
        
    }

}



//A handler that makes sure that all processes know the full PIDs array.
void handler(int signum)
{
     
    rec_val = msgrcv(msgqid, &messageRCVD, sizeof(messageRCVD.messagePID), 0, !IPC_NOWAIT);  
    for(int i =0;i<5;i++){
        PIDs[i]=messageRCVD.messagePID[i];
    }
    IDsRCVD=1;
    
}




//A handler called when the previous leader died.
void handler2(int signum)
{ 
    //The non-leaders received the signals.
    printf("\nI am %d and I know that the previous leader is dying :(\n", getpid());


    //Assigning -1 to the place of the previous leader. 
    int i=0;
    while(PIDs[i]!=-1 && i<sizeof(PIDs)/sizeof(int)){
        i++;
    }
    i--;
    PIDs[i]=-1;

    

    //Getting my own index in the PIDs array.
    i=0;
    while(PIDs[i]!=getpid()){
        i++;
        sleep(1);
        
    }
    



    //Creating a new message.
    struct msgbuff message;
    

        //PUT MYSELF IN A MESSAGE.
        //Selecting the next process.
        if (PIDs[i+1]!=-1){
            message.mtype=PIDs[i+1];
        }
        else{
            message.mtype=PIDs[0];
        }

        //Putting myself as the first element of the array.
        message.messagePID[0]=getpid();
        message.messagePID[1]=-1;
        message.messagePID[2]=-1;
        message.messagePID[3]=-1;
        message.messagePID[4]=-1;


        //Sending the message to the next process 
        int send_val = msgsnd(msgqid, &message, sizeof(message.messagePID), !IPC_NOWAIT);
        

        //Receive from other processes, until I am the first element in the received message.
        while(1){
            int rec_val = msgrcv(msgqid, &messageRCVD, sizeof(messageRCVD.messagePID), getpid(), !IPC_NOWAIT);

            //If the first element of the received message isn't my PID, I will put myself in the array and pass it to the next one.
            if(messageRCVD.messagePID[0]!=getpid()){
                
                int c=0;
                while(messageRCVD.messagePID[c]!=-1){
                    c++;
                }
                messageRCVD.messagePID[c]=getpid();
                for(int i=0;i<5;i++){
                    message.messagePID[i]=messageRCVD.messagePID[i];
                }
                send_val = msgsnd(msgqid, &message, sizeof(message.messagePID), !IPC_NOWAIT);
            }
            //I am the first element in the received array, so I will choose the greatest PID to be the leader.
            else{
                int max=-1;
                int z=0;
                while(PIDs[z]!=-1){
                    if(PIDs[z]>max){
                        max=PIDs[z];
                    }
                    z++;
                }
                printf("\nI am %d and I think %d should be our new leader\n", getpid(), max);
                leader=max;
                break;
            } 
        }
    
}