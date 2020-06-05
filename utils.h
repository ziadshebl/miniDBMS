
#define NAME_LENGTH 20
#define maxCharactersOfLogMessage   100
#define MAX_RECORDS  100

//Record struct
struct record
{
   int key;
   char name [NAME_LENGTH];
   int salary; 
};

///////////////////////////////////////////////////////////////////////////////
//Message Buffers
///Client-Manager message Buffers
struct addRecordBuffer
{
   int clientPID;
   char name[20];
   int salary;
};

struct semaphoreOperationsBuffer
{
   int clientPID;
   int recordKey;
};

struct logMsgBuffer
{
   long mtype;
   char messageToLog[maxCharactersOfLogMessage];
};


enum salaryRetrieveOperation {
   smallerThan,
   biggerThan,
   equal,
   smallerThanOrEqual,
   biggerThanOrEqual,
   salaryNone
};

enum retrieveOperation {
   nameAndSalary,
   nameOnly,
   nameContainsAndSalary,
   nameContainsOnly,
   salaryOnly,
   fullTable
};

enum nameRetrieveOperation {
   fullName,
   nameContains,
   nameNone
};
struct retrieveBuffer
{
   int clientPID;
   char name[NAME_LENGTH];
   int salary;
   enum retrieveOperation operation;
   enum salaryRetrieveOperation salaryOperation;
   enum nameRetrieveOperation nameOperation;
};

enum modifySalaryOperation {
   increase,
   decrease
};

struct modifyRecordBuffer
{
   int clientPID;
   int recordKey;
   enum modifySalaryOperation salaryOperation;
   int value;
};

enum operation{
   add,
   modify,
   retrieve,
   acquire,
   release
};

struct AllOperations
{
   enum operation operationNeeded;
   struct addRecordBuffer  addBuffer;
   struct semaphoreOperationsBuffer  semaphoreOperationsBuffer;
   struct modifyRecordBuffer  modifyBuffer;
   struct retrieveBuffer retrieveBuffer;  
   //struct releaseRecordBuffer releaseBuffer;
   
};

struct clientManagerMsgBuffer
{
   long mtype;
   struct AllOperations operationMessage;
};

struct additionSuccessMessageBuffer
{
   long mtype;
   int key;
};

//TODO:: add number of records
struct operationSuccessMessageBuffer
{
   long mtype;
   int isOperationDone;    //0 for failure, 1 for success
   //int numberOfRecords;
};

/*
struct acquireRecordBuffer
{
   int clientPID;
   int keyOfRecordToBeAcquired;
};
*/
//Query Logger Message Buffers
struct queryLoggerMsgBuffer
{
    long mtype;
    enum operation neededoperation;
    int senderPID;
};

///////////////////////////////////////////////////////////////////////////////
//Function declerations
void dumbMemory(struct record* memoryStartAddress,int numberOfEntries);
void queryLog(int numberOfRecords, struct retrieveBuffer requiredRetrieval,struct record records[100]);  
