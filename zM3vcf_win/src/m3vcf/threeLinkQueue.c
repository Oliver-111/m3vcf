#include "threeLinkQueue.h"

TQStatus init_TLinkQueue(TLink_Queue *Q,int nodeNum)
{
	if(!Q)
	{
		return TLINK_QUEUE_ERROR;
	}
	
	int i;
	TQNode *p;
	
	if(nodeNum <= 0)
	{
		nodeNum=MIN_TNODE_SIZE;
	}
	
	p=(TQNode*)malloc(sizeof(TQNode));
	if(!p)
	{
		return TLINK_QUEUE_ERROR;
	}
	p->next=NULL;
	Q->dataQueueFront=p;
	Q->dataQueueRear=p;

	p=(TQNode*)malloc(sizeof(TQNode));
	if(!p)
	{
		return TLINK_QUEUE_ERROR;
	}
	p->next=NULL;
	Q->readyQueueFront=p;
	Q->readyQueueRear=p;

	p=(TQNode*)malloc(sizeof(TQNode));
	if(!p)
	{
		return TLINK_QUEUE_ERROR;
	}
	p->next=NULL;
	Q->rawDataQueueFront=p;
	Q->rawDataQueueRear=p;
	
	for(i=0;i<nodeNum;i++)
	{
		p=(TQNode*)malloc(sizeof(TQNode));
		if(!p)
		{
			return TLINK_QUEUE_ERROR;
		}
		memset(p,0,sizeof(TQNode));
		in_TQueue(Q,TQ_READY,p);
	}
	return TLINK_QUEUE_OK;
}


TQNode* out_TQueue(TLink_Queue *Q,TQUEUE_MODE mode)
{
	if(!Q)
	{
		return NULL;
	}
	if(TLINK_QUEUE_OK==is_TQueue_Empty(Q,mode))
	{
		return NULL;
	}
	
	TQNode *p;
	TQNode **f, **r;
	if(TQ_DATA==mode)
	{
		f=&(Q->dataQueueFront);
		r=&(Q->dataQueueRear);
	}
	else if(TQ_RAWDATA==mode)
	{
		f=&(Q->rawDataQueueFront);
		r=&(Q->rawDataQueueRear);
	}
	else if(TQ_READY==mode)
	{
		f=&(Q->readyQueueFront);
		r=&(Q->readyQueueRear);
	}
	else
	{
		printf("TQ mode out error!\n");
		exit(1);
	}
	
	p=(*f)->next;
	(*f)->next=p->next;
	if(*r==p)
	{
		*r=*f;
	}
	return p;
}
TQStatus in_TQueue(TLink_Queue *Q,TQUEUE_MODE mode,TQNode *node)
{
	if(!Q || !node)
	{
		return TLINK_QUEUE_ERROR;
	}
	TQNode **r;
	if(TQ_DATA==mode)
	{
		r=&(Q->dataQueueRear);
	}
	else if(TQ_RAWDATA==mode)
	{
		r=&(Q->rawDataQueueRear);
	}
	else if(TQ_READY==mode)
	{
		r=&(Q->readyQueueRear);
	}
	else
	{
		printf("TQ mode in error!\n");
		exit(1);
	}
	node->next=NULL;
	(*r)->next=node;
	(*r)=node;
	return TLINK_QUEUE_OK;
}

TQStatus is_TQueue_Empty(TLink_Queue *Q,TQUEUE_MODE mode)
{
	if(!Q)
	{
		return TLINK_QUEUE_ERROR;
	}
	TQNode **f, **r;
	if(TQ_DATA==mode)
	{
		f=&(Q->dataQueueFront);
		r=&(Q->dataQueueRear);
	}
	else if(TQ_RAWDATA==mode)
	{
		f=&(Q->rawDataQueueFront);
		r=&(Q->rawDataQueueRear);
	}
	else if(TQ_READY==mode)
	{
		f=&(Q->readyQueueFront);
		r=&(Q->readyQueueRear);
	}
	else
	{
		printf("TQ mode empty error!\n");
		exit(1);
	}
	if((*f) == (*r))
	{
		return TLINK_QUEUE_OK;
	}
	return TLINK_QUEUE_ERROR;
}

TQStatus uninit_TLinkQueue(TLink_Queue *Q)
{
	if(!Q)
	{
		return TLINK_QUEUE_ERROR;
	}
	TQNode *p;
	
	while(TLINK_QUEUE_OK!=is_TQueue_Empty(Q,TQ_DATA))
	{
		p=Q->dataQueueFront->next;
		Q->dataQueueFront->next=p->next;
		if(Q->dataQueueRear==p)
		{
			Q->dataQueueRear=Q->dataQueueFront;
		}
		free(p);
		p=NULL;
	}
	free(Q->dataQueueFront);
	while(TLINK_QUEUE_OK!=is_TQueue_Empty(Q,TQ_READY))
	{
		p=Q->readyQueueFront->next;
		Q->readyQueueFront->next=p->next;
		if(Q->readyQueueRear==p)
		{
			Q->readyQueueRear=Q->readyQueueFront;
		}
		free(p);
		p=NULL;
	}
	free(Q->readyQueueFront);
	while(TLINK_QUEUE_OK!=is_TQueue_Empty(Q,TQ_RAWDATA))
	{
		p=Q->rawDataQueueFront->next;
		Q->rawDataQueueFront->next=p->next;
		if(Q->rawDataQueueRear==p)
		{
			Q->rawDataQueueRear=Q->rawDataQueueFront;
		}
		free(p);
		p=NULL;
	}
	free(Q->rawDataQueueFront);
	return TLINK_QUEUE_OK;
}

