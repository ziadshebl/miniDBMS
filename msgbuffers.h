#include "record.h"
#define maxCharactersOfLogMessage   100
#define MAX_RECORDS  100

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
   none
};

enum retrieveOperation {
   nameAndSalary,
   nameOnly,
   nameContainsAndSalary,
   nameContainsOnly,
   salaryOnly,
   fullTable
};

struct retrieveBuffer
{
   long mtype;
   char name[20];
   int salary;
   enum retrieveOperation operation;
   enum salaryRetrieveOperation salaryOperation;
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

struct operationSuccessMessageBuffer
{
   long mtype;
   int isOperationDone;    //0 for failure, 1 for success
};

/*
struct acquireRecordBuffer
{
   int clientPID;
   int keyOfRecordToBeAcquired;
};
*/