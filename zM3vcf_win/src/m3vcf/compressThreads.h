#ifndef _COMPRESS_THREAD_H_
#define _COMPRESS_THREAD_H_

#include <pthread.h>
#include <semaphore.h>

#include "threeLinkQueue.h"


#define COMPRESS_THREAD_SIZE			(6)
#define WRITE_THREAD_SIZE				(1)
#define READ_THREAD_SIZE				(1)
//#define MAX_WAITING_LINK_SIZE 			(4)
#define MAX_WAITING_LINK_SIZE 			(0)
#define ADDITIONAL_LINK_SIZE (READ_THREAD_SIZE+WRITE_THREAD_SIZE+MAX_WAITING_LINK_SIZE)

//read and write tlink queue
typedef struct
{
	TLink_Queue queue;
	pthread_mutex_t ready_mutex;
	pthread_mutex_t raw_data_mutex;
	pthread_mutex_t data_mutex;
	volatile int block_endFlag;
	volatile int write_endFlag;
	sem_t ready_full;
	sem_t ready_empty;
	sem_t raw_data_full;
	sem_t raw_data_empty;
	sem_t data_full;
	sem_t data_empty;
	int numRecords;
}tlqueue_args;

typedef struct
{
	pthread_mutex_t outReadQueueMutex;
	pthread_mutex_t inWriteQueueMutex;
}block_records_thread_args;


//create thread in turn
extern pthread_mutex_t gCreateMutex;

//read and write queue
extern tlqueue_args gBlockQueue;

//compress thread number
extern int gCompressThreadNum;

//queue data in turn 
extern block_records_thread_args *gBRThreadArgs;



//read and write queue
void initTLQueueAndThreadArgs(tlqueue_args *qap,int numRecords);
void unInitTLQueueAndThreadArgs(tlqueue_args *qap);
void inReadyQueue(tlqueue_args *qap,TQNode *p);
void outReadyQueue(tlqueue_args *qap,TQNode **p);
void inRawDataQueue(tlqueue_args *qap,TQNode *p);
void outRawDataQueue(tlqueue_args *qap,TQNode **p);
void inDataQueue(tlqueue_args *qap,TQNode *p);
void outDataQueue(tlqueue_args *qap,TQNode **p);

#endif
