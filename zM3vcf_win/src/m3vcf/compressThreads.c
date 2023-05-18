#include "compressThreads.h"



pthread_mutex_t gCreateMutex=PTHREAD_MUTEX_INITIALIZER;
tlqueue_args gBlockQueue;
block_records_thread_args *gBRThreadArgs=NULL; 
int gCompressThreadNum = COMPRESS_THREAD_SIZE;


void initTLQueueAndThreadArgs(tlqueue_args *qap,int numRecords)
{
	init_TLinkQueue(&(qap->queue),gCompressThreadNum+ADDITIONAL_LINK_SIZE);
	
	qap->numRecords=numRecords;	
	
	pthread_mutex_init(&(qap->ready_mutex),NULL);
	pthread_mutex_init(&(qap->raw_data_mutex),NULL);
	pthread_mutex_init(&(qap->data_mutex),NULL);
	
	qap->block_endFlag=0;
	qap->write_endFlag=0;
	
	sem_init(&(qap->ready_full),0,0);
	sem_init(&(qap->ready_empty),0,gCompressThreadNum+ADDITIONAL_LINK_SIZE);

	sem_init(&(qap->raw_data_full),0,gCompressThreadNum+ADDITIONAL_LINK_SIZE);
	sem_init(&(qap->raw_data_empty),0,0);

	sem_init(&(qap->data_full),0,gCompressThreadNum+ADDITIONAL_LINK_SIZE);
	sem_init(&(qap->data_empty),0,0);

	int i;
	gBRThreadArgs=(block_records_thread_args *)malloc(sizeof(block_records_thread_args)*gCompressThreadNum);
	if(NULL==gBRThreadArgs)
	{
		zSLMessage("gBRThreadArgs malloc error\n");
		exit(1);
	}
	for(i=0;i<gCompressThreadNum;i++)
	{
		pthread_mutex_init(&(gBRThreadArgs[i].outReadQueueMutex),NULL);
		pthread_mutex_init(&(gBRThreadArgs[i].inWriteQueueMutex),NULL);
		pthread_mutex_lock(&(gBRThreadArgs[i].outReadQueueMutex));
		pthread_mutex_lock(&(gBRThreadArgs[i].inWriteQueueMutex));
	}
	pthread_mutex_unlock(&(gBRThreadArgs[0].outReadQueueMutex));
	pthread_mutex_unlock(&(gBRThreadArgs[0].inWriteQueueMutex));
}
void unInitTLQueueAndThreadArgs(tlqueue_args *qap)
{
	uninit_TLinkQueue(&(qap->queue));
	
	pthread_mutex_destroy(&(qap->ready_mutex));
	pthread_mutex_destroy(&(qap->raw_data_mutex));
	pthread_mutex_destroy(&(qap->data_mutex));
	
	sem_destroy(&(qap->ready_full));
	sem_destroy(&(qap->ready_empty));
	
	sem_destroy(&(qap->raw_data_full));
	sem_destroy(&(qap->raw_data_empty));
	
	sem_destroy(&(qap->data_full));
	sem_destroy(&(qap->data_empty));
	
	int i;
	for(i=0;i<gCompressThreadNum;i++)
	{
		pthread_mutex_destroy(&(gBRThreadArgs[i].outReadQueueMutex));
		pthread_mutex_destroy(&(gBRThreadArgs[i].inWriteQueueMutex));
	}
	free(gBRThreadArgs);
}

void inReadyQueue(tlqueue_args *qap,TQNode *p)
{
	sem_wait(&(qap->ready_full));
	pthread_mutex_lock(&(qap->ready_mutex));
	in_TQueue(&(qap->queue),TQ_READY,p);
	sem_post(&(qap->ready_empty));
	pthread_mutex_unlock(&(qap->ready_mutex));
}

void outReadyQueue(tlqueue_args *qap,TQNode **p)
{
	sem_wait(&(qap->ready_empty));
	pthread_mutex_lock(&(qap->ready_mutex));
	(*p)=out_TQueue(&(qap->queue),TQ_READY);
	sem_post(&(qap->ready_full));
	pthread_mutex_unlock(&(qap->ready_mutex));
}	

void inRawDataQueue(tlqueue_args *qap,TQNode *p)
{
	sem_wait(&(qap->raw_data_full));
	pthread_mutex_lock(&(qap->raw_data_mutex));
	in_TQueue(&(qap->queue),TQ_RAWDATA,p);
	sem_post(&(qap->raw_data_empty));
	pthread_mutex_unlock(&(qap->raw_data_mutex));
}

void outRawDataQueue(tlqueue_args *qap,TQNode **p)
{
	sem_wait(&(qap->raw_data_empty));
	pthread_mutex_lock(&(qap->raw_data_mutex));
	(*p)=out_TQueue(&(qap->queue),TQ_RAWDATA);
	sem_post(&(qap->raw_data_full));
	pthread_mutex_unlock(&(qap->raw_data_mutex));
}	

void inDataQueue(tlqueue_args *qap,TQNode *p)
{
	sem_wait(&(qap->data_full));
	pthread_mutex_lock(&(qap->data_mutex));
	in_TQueue(&(qap->queue),TQ_DATA,p);
	sem_post(&(qap->data_empty));
	pthread_mutex_unlock(&(qap->data_mutex));
}
void outDataQueue(tlqueue_args *qap,TQNode **p)
{
	sem_wait(&(qap->data_empty));
	pthread_mutex_lock(&(qap->data_mutex));
	(*p)=out_TQueue(&(qap->queue),TQ_DATA);
	sem_post(&(qap->data_full));
	pthread_mutex_unlock(&(qap->data_mutex));
}	


