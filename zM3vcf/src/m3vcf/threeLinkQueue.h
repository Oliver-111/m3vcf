#ifndef _THREE_LINK_QUEUE_H_
#define _THREE_LINK_QUEUE_H_

#include "common.h"

#define MIN_TNODE_SIZE (3)

typedef enum
{
	TLINK_QUEUE_OK=0,
	TLINK_QUEUE_ERROR,
	TLINK_QUEUE_MAX_SIZE
}TLINK_QUEUE_STATUS;

typedef enum
{
	TQ_READY=0,
	TQ_RAWDATA,
	TQ_DATA,
	TQ_MAX_SIZE
}TQUEUE_MODE;

typedef TLINK_QUEUE_STATUS TQStatus;
typedef RECORD_BLOCK TQElemType;
typedef struct TQnode
{
	TQElemType data;
	struct TQnode *next;
}TQNode;

typedef struct
{
	TQNode *readyQueueFront;
	TQNode *readyQueueRear;
	
	TQNode *rawDataQueueFront;
	TQNode *rawDataQueueRear;

	TQNode *dataQueueFront;
	TQNode *dataQueueRear;
}TLink_Queue;

TQStatus init_TLinkQueue(TLink_Queue *Q,int nodeNum);
TQNode* out_TQueue(TLink_Queue *Q,TQUEUE_MODE mode);
TQStatus in_TQueue(TLink_Queue *Q,TQUEUE_MODE mode,TQNode *node);
TQStatus is_TQueue_Empty(TLink_Queue *Q,TQUEUE_MODE mode);
TQStatus uninit_TLinkQueue(TLink_Queue *Q);
#endif

