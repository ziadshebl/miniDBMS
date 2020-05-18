#include "record.h"

struct addRecordMsgBuffer
{
   long mtype;
   struct record recordToBeAdded;
};

struct modifyRecordMsgBuffer
{
   long mtype;
   int recordKey;
   int isAddition;   //0 for subtracting a specific value, 1 for adding a specific value
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