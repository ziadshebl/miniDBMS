#include "record.h"

struct addRecordMsgBuffer
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

struct modifyRecordMsgBuffer
{
   long mtype;
   int recordKey;
   enum modifySalaryOperation salaryOperation;
   int value;
};

struct acquireRecordMsgBuffer
{
   long mtype;
   int keyOfRecordToBeAcquired;
};

struct releaseRecordMsgBuffer
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

struct retrieveMsgBuffer
{
   long mtype;
   char name[20];
   int salary;
   enum retrieveOperation operation;
   enum salaryRetrieveOperation salaryOperation;
};

struct AllOperations
{
   struct addRecordMsgBuffer  addMsgBuffer;
   struct modifyRecordMsgBuffer  modifyBuffer;
   struct retrieveMsgBuffer  retrieveBuffer;
};