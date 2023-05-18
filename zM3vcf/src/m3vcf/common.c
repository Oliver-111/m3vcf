#include "common.h"


void mallocFreeIntP(int**pInt,int size,MF_FLAG flag)
{
	if(MF_MALLOC==flag)
	{
		(*pInt)=(int*)malloc(size*sizeof(int));
		if(NULL == (*pInt))
		{
			zSLMessage("int malloc error!\n");
			exit(1);
		}
		memset((*pInt),0,size*sizeof(int));
	}
	else if(MF_FREE==flag)
	{
		free((*pInt));
		(*pInt)=NULL;
	}
	else
	{
		zSLMessage("int malloc flag error!\n");
	}
}

void mallocFreeIntPP(int***pInt,int row,int column,MF_FLAG flag)
{
	int i;
	if(MF_MALLOC==flag)
	{
		(*pInt)=(int**)malloc(row*sizeof(int*));
		if(NULL == (*pInt))
		{
			zSLMessage("int malloc error!\n");
			exit(1);
		}
		for(i=0;i<row;i++)
		{
			(*pInt)[i]=(int*)malloc(column*sizeof(int));
			if(NULL == (*pInt)[i])
			{
				zSLMessage("int malloc error!\n");
				exit(1);
			}
			memset((*pInt)[i],0,column*sizeof(int));
		}
	}
	else if(MF_FREE==flag)
	{
		for(i=0;i<row;i++)
		{
			free((*pInt)[i]);
			(*pInt)[i]=NULL;
		}
		free((*pInt));
		(*pInt)=NULL;
	}
	else
	{
		zSLMessage("int malloc flag error!\n");
	}
}

void mallocFreeCharP(char**pChr,int size,MF_FLAG flag)
{
	if(MF_MALLOC==flag)
	{
		(*pChr)=(char*)malloc(size*sizeof(char));
		if(NULL == (*pChr))
		{
			zSLMessage("char malloc error!\n");
			exit(1);
		}
		memset((*pChr),0,size*sizeof(char));
	}
	else if(MF_FREE==flag)
	{
		free((*pChr));
		(*pChr)=NULL;
	}
	else
	{
		zSLMessage("char malloc flag error!\n");
	}
}

void mallocFreeCharPP(char***pChr,int row,int column,MF_FLAG flag)
{
	int i;
	if(MF_MALLOC==flag)
	{
		(*pChr)=(char**)malloc(row*sizeof(char*));
		if(NULL == (*pChr))
		{
			zSLMessage("char malloc error!\n");
			exit(1);
		}
		for(i=0;i<row;i++)
		{
			(*pChr)[i]=(char*)malloc(column*sizeof(char));
			if(NULL == (*pChr)[i])
			{
				zSLMessage("char malloc error!\n");
				exit(1);
			}
			memset((*pChr)[i],0,column*sizeof(char));
		}
	}
	else if(MF_FREE==flag)
	{
		for(i=0;i<row;i++)
		{
			free((*pChr)[i]);
			(*pChr)[i]=NULL;
		}
		free((*pChr));
		(*pChr)=NULL;
	}
	else
	{
		zSLMessage("char malloc flag error!\n");
	}
}

void mallocFreeRecordBlock(RECORD_BLOCK *recordBlock,MF_FLAG flag)
{
	if(MF_MALLOC==flag)
	{
		mallocFreeIntP(&(recordBlock->blockBoundaries),recordBlock->dataBlock.numDataLines,MF_MALLOC);
		recordBlock->posBlockBoundaries=0;
		
		//malloc **bestIndex
		mallocFreeIntPP(&(recordBlock->bestIndex),recordBlock->dataBlock.numDataLines+1,recordBlock->dataBlock.dataLines[0].numSamples*2,MF_MALLOC);
	}
	else if(MF_FREE==flag)
	{
		//free *blockBoundaries
		mallocFreeIntP(&(recordBlock->blockBoundaries),0,MF_FREE);
		recordBlock->posBlockBoundaries=0;
		//free **bestIndex
		mallocFreeIntPP(&(recordBlock->bestIndex),recordBlock->dataBlock.numDataLines+1,0,MF_FREE);
		//free dataBlock
		clearDataBlock(&(recordBlock->dataBlock));
	}
	else
	{
		zSLMessage("malloc flag error!\n");
	}
}

