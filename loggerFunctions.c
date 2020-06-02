#define AcquireSemaphore 1
#define ReleaseSemaphore 0

struct msgbuff
{
    long mtype;
    int SemaphoreStat;  
};

int loggerFunctions(){


}
int SendMessageToAcquireSemaphore(int MsgQid, int RecieverID){

    struct msgbuff message;

    message.mtype = RecieverID;
    message.SemaphoreStat = AcquireSemaphore;

    int send_val = msgsnd(MsgQid, &message, sizeof(message.SemaphoreStat), IPC_NOWAIT);

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

int SendMessageToReleaseSemaphore(int MsgQid, int RecieverID){

    struct msgbuff message;

    message.mtype = RecieverID;
    message.SemaphoreStat = ReleaseSemaphore;

    int send_val = msgsnd(MsgQid, &message, sizeof(message.SemaphoreStat), IPC_NOWAIT);

    if (send_val == -1)
    {
        perror("Error in send");
        return -1;
    }else
    {
        return 0;
    }
    
}
