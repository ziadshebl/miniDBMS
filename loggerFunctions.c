#define AcquireSemaphore 1
#define ReleaseSemaphore 0

struct msgbuff
{
    long mtype;
    int SemaphoreStat; 
    int SenderPID; 
};

int SendMessageToAcquireSemaphore(int MsgQid, int RecieverPID);
int SendMessageToReleaseSemaphore(int MsgQid, int RecieverPID);
int RecieveMessage(int MsgQid);
void Log(char* LogMessage, int MsgQid, int LoggerPID);
int loggerFunctions(){


}
int SendMessageToAcquireSemaphore(int MsgQid, int RecieverPID){

    struct msgbuff message;

    message.mtype = RecieverPID;
    message.SemaphoreStat = AcquireSemaphore;
    message.SenderPID = getpid();

    int send_val = msgsnd(MsgQid, &message, sizeof(message.SemaphoreStat)+sizeof(message.SenderPID), IPC_NOWAIT);

    if (send_val == -1)
    {
        perror("Error in send");
        return -1;
    }
    else
    {
        return 0;
    }
    
}

int SendMessageToReleaseSemaphore(int MsgQid, int RecieverPID){

    struct msgbuff message;

    message.mtype = RecieverPID;
    message.SemaphoreStat = ReleaseSemaphore;
    message.SenderPID = getpid();

    int send_val = msgsnd(MsgQid, &message, sizeof(message.SemaphoreStat)+sizeof(message.SenderPID), IPC_NOWAIT);

    if (send_val == -1)
    {
        perror("Error in send");
        return -1;
    }else
    {
        return 0;
    }
    
}
int RecieveMessage(int MsgQid){

    struct msgbuff message;

    int rec_val = msgrcv(MsgQid, &message, sizeof(message.SemaphoreStat)+sizeof(message.SenderPID), getpid(), !IPC_NOWAIT);

    if (rec_val == -1)
    {
        
        return -1;
    }else
    {
        printf("This is process %d. Message Recieved From Logger\n", getpid());
        return 0;
    }
    
}
void Log(char* LogMessage, int MsgQid, int LoggerPID ){

    SendMessageToAcquireSemaphore(MsgQid, LoggerPID);
    RecieveMessage(MsgQid);


    //Write in shared memory
    printf("This is Process %d an I am Now Logging..\n", getpid());
    SendMessageToReleaseSemaphore(MsgQid,LoggerPID);
}
