#include "record.h"

struct addRecordBuffer
{
   long mtype;
   int clientNumber;
   char name[20];
   int salary;
};

enum modifySalaryOperation {
   increase,
   decrease
};

struct modifyRecordBuffer
{
   long mtype;
   int recordKey;
   enum modifySalaryOperation salaryOperation;
   int value;
};

struct acquireRecordBuffer
{
   long mtype;
   int keyOfRecordToBeAcquired;
};

struct releaseRecordBuffer
{
   long mtype;
   int keyOfRecordToBeReleased;
};

struct logMsgBuffer
{
   long mtype;
   int keyOfRecordToBeReleased;
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
   retrieve
};

struct AllOperations
{
   enum operation operationNeeded;
   struct addRecordBuffer  addBuffer;
   struct modifyRecordBuffer  modifyBuffer;
   struct retrieveBuffer  retrieveBuffer;
};

struct clientManagerMsgBuffer
{
   long mtype;
   struct AllOperations operationMessage;
};