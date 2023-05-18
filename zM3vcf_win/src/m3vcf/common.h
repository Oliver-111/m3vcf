#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <omp.h>
#include "vcflib.h"
#include "messageLog.h"

#define ZM3VCF_VERSION 			"1.0.0"


#define MIN_STR_SIZE				32
#define MID_STR_SIZE				512
#define MAX_STR_SIZE				1024


//#define ZZ_DEBUG
#ifdef ZZ_DEBUG
	#define POS_DEBUG() fprintf(stderr,"file:%s func:%s  line:%d\n",__FILE__,__func__,__LINE__)
#else
	#define POS_DEBUG()
#endif

//#define ZZ_TIME



typedef struct
{
	//vcf
	DATA_BLOCK dataBlock;
	
	//m3vcf
	int *blockBoundaries;	//size:int * dataBlock.numDataLines
	int posBlockBoundaries;
	int **bestIndex;		//size:int * dataBlock.numDataLines+1*dataBlock.dataLines[0].numSamples*2

	//index
	long long index;
}RECORD_BLOCK;

//flag :1  malloc
//flag :0  free
typedef enum 
{
	MF_FREE=0,
	MF_MALLOC
}MF_FLAG;


void mallocFreeIntP(int**pInt,int size,MF_FLAG flag);
void mallocFreeIntPP(int***pInt,int row,int column,MF_FLAG flag);
void mallocFreeCharP(char**pChr,int size,MF_FLAG flag);
void mallocFreeCharPP(char***pChr,int row,int column,MF_FLAG flag);
void mallocFreeRecordBlock(RECORD_BLOCK *recordBlock,MF_FLAG flag);

#endif
