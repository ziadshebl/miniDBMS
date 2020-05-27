#include "record.h"
#define maxCharactersOfLogMessage   100

struct addRecordBuffer
{
   int clientPID;
   char name[20];
   int salary;
};

enum modifySalaryOperation {
   increase,
   decrease
};

struct modifyRecordBuffer
{
   int recordKey;
   enum modifySalaryOperation salaryOperation;
   int value;
};

struct acquireRecordBuffer
{
   int keyOfRecordToBeAcquired;
};

struct releaseRecordBuffer
{
   int keyOfRecordToBeReleased;
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
   struct modifyRecordBuffer  modifyBuffer;
   struct retrieveBuffer  retrieveBuffer;
   struct acquireRecordBuffer acquireBuffer;
   struct releaseRecordBuffer releaseBuffer;
   
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