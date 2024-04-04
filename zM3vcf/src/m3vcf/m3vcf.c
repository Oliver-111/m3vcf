#include "m3vcf.h"


#define MAXALLELE				2
#define TRANSFACTOR				3
#define CISFACTOR				2

//#define NUM_ALT_HAPLO_INDEX  	512
#define MAX_BLOCK_LINE_SIZE 	6*1024*1024
#define MAX_SUB_STR_SIZE		1*1024*1024

#define M3VCF_VERSION_STR		"##fileformat=M3VCFv2.0\n"


typedef struct
{
	int *examplars;
	int posExamplars;
	int numMarks;
	int *UniqueIndexMap;
	int numHaplotypes;
}M3VCF_BLOCK_HEADER;

typedef struct
{
	char *rawDataLine;
	DATA_INFO dataInfo;
	int numAltHaplo;
	int *altHaploIndex;
}M3VCF_RECORD;

typedef struct
{
	M3VCF_BLOCK_HEADER m3vcfBlockHeader;
	M3VCF_RECORD *m3vcfRecords;
}M3VCF_BLOCK;

typedef struct
{
	int *index;
	int *oldIndex;
	int *previousDifference;
    int *previousPredecessor;
	int *firstDifference;
    int *cost;
    int *bestSlice;
}TEMP_VARIABLE;

typedef struct
{
	M3VCF_BLOCK_HEADER blockHeader;
	int blockStart;
	int blockEnd;
	char *lineStr;
}SUB_BLOCK_INFO;

/*************************************************compress*****************************************************/

static void changeString(char* string)
{
	char tmpStr[MID_STR_SIZE];
	int i,j=0;
	for(i=0;'\0'!=string[i];i++)
	{
		if(string[i]>='0' && string[i]<='9')
		{
			tmpStr[j++]=string[i];
		}
		else if(';'== string[i])
		{
			tmpStr[j++]=' ';
		}
	}
	tmpStr[j]='\0';
	strcpy(string,tmpStr);
}
	
static void initializeTempVariable(TEMP_VARIABLE *tvp,int numRecords,int numHaplotypes)
{
	int i;
	mallocFreeIntP(&(tvp->index),numHaplotypes,MF_MALLOC);
	for(i=0;i<numHaplotypes;i++)
	{
        tvp->index[i]=i;
	}
	mallocFreeIntP(&(tvp->oldIndex),numHaplotypes,MF_MALLOC);
	mallocFreeIntP(&(tvp->previousDifference),numHaplotypes,MF_MALLOC);
	mallocFreeIntP(&(tvp->previousPredecessor),numHaplotypes,MF_MALLOC);
	mallocFreeIntP(&(tvp->firstDifference),numHaplotypes-1,MF_MALLOC);
	mallocFreeIntP(&(tvp->cost),numRecords+1,MF_MALLOC);
   	mallocFreeIntP(&(tvp->bestSlice),numRecords+1,MF_MALLOC);
}

static void unInitializeTempVariable(TEMP_VARIABLE *tvp)
{
	mallocFreeIntP(&(tvp->index),0,MF_FREE);
	mallocFreeIntP(&(tvp->oldIndex),0,MF_FREE);
	mallocFreeIntP(&(tvp->previousDifference),0,MF_FREE);
	mallocFreeIntP(&(tvp->previousPredecessor),0,MF_FREE);
	mallocFreeIntP(&(tvp->firstDifference),0,MF_FREE);
	mallocFreeIntP(&(tvp->cost),0,MF_FREE);
   	mallocFreeIntP(&(tvp->bestSlice),0,MF_FREE);
}

static void initializeSubBlockInfo(SUB_BLOCK_INFO *sbip,int numHaplotypes)
{
	sbip->blockStart=0;
	mallocFreeIntP(&(sbip->blockHeader.examplars),numHaplotypes,MF_MALLOC);
	sbip->blockHeader.posExamplars=0;
	mallocFreeIntP(&(sbip->blockHeader.UniqueIndexMap),numHaplotypes,MF_MALLOC);
	mallocFreeCharP(&(sbip->lineStr),MAX_BLOCK_LINE_SIZE,MF_MALLOC);
}

static void unInitializeSubBlockInfo(SUB_BLOCK_INFO *sbip)
{
	mallocFreeIntP(&(sbip->blockHeader.examplars),0,MF_FREE);
	mallocFreeIntP(&(sbip->blockHeader.UniqueIndexMap),0,MF_FREE);
	mallocFreeCharP(&(sbip->lineStr),0,MF_FREE);
	memset(sbip,0,sizeof(SUB_BLOCK_INFO));
}

static void clearSubBlockInfo(SUB_BLOCK_INFO *sbip,int numHaplotypes)
{
	memset(sbip->blockHeader.examplars,0,numHaplotypes*sizeof(int));
	sbip->blockHeader.posExamplars=0;
	memset(sbip->blockHeader.UniqueIndexMap,0,numHaplotypes*sizeof(int));
	memset(sbip->lineStr,0,MAX_BLOCK_LINE_SIZE);
}

static void FreeM3vcfBlock(M3VCF_BLOCK *mbp)
{
	free(mbp->m3vcfBlockHeader.examplars);
	mbp->m3vcfBlockHeader.posExamplars=0;
	free(mbp->m3vcfBlockHeader.UniqueIndexMap);
	mbp->m3vcfBlockHeader.numHaplotypes=0;

	int i;
	for(i=0;i<mbp->m3vcfBlockHeader.numMarks;i++)
	{
		//free(mbp->m3vcfRecords[i].dataInfo.chrom);
		//free(mbp->m3vcfRecords[i].dataInfo.pos);
		//free(mbp->m3vcfRecords[i].dataInfo.ID);
		//free(mbp->m3vcfRecords[i].dataInfo.ref);
		//free(mbp->m3vcfRecords[i].dataInfo.alt);
		//free(mbp->m3vcfRecords[i].dataInfo.qual);
		//free(mbp->m3vcfRecords[i].dataInfo.filter);
		//free(mbp->m3vcfRecords[i].dataInfo.info);
		//free(mbp->m3vcfRecords[i].dataInfo.format);
		free(mbp->m3vcfRecords[i].rawDataLine);
		if(mbp->m3vcfRecords[i].numAltHaplo)
		{
			free(mbp->m3vcfRecords[i].altHaploIndex);
			mbp->m3vcfRecords[i].numAltHaplo=0;
		}
	}	
	free(mbp->m3vcfRecords);
	mbp->m3vcfBlockHeader.numMarks=0;
}

static void getSubBlockInfo(SUB_BLOCK_INFO *sbip,RECORD_BLOCK *rbp)
{
	int i,j; 
	int previous;
	int current;
	int strlength=0;
	int numHaplotypes=rbp->dataBlock.dataLines[0].numSamples*2;
	DATA_LINE *pDataLine=rbp->dataBlock.dataLines;
	sbip->blockHeader.examplars[sbip->blockHeader.posExamplars++]=rbp->bestIndex[sbip->blockEnd][0];
	sbip->blockHeader.UniqueIndexMap[rbp->bestIndex[sbip->blockEnd][0]]=0;
	for(i=1;i<numHaplotypes;i++)
    {
        previous=rbp->bestIndex[sbip->blockEnd][i-1];
        current=rbp->bestIndex[sbip->blockEnd][i];
        for(j=sbip->blockStart;j<=sbip->blockEnd;j++)
        {
        	if(pDataLine[j].gtData[previous]!= pDataLine[j].gtData[current])
            {
                sbip->blockHeader.examplars[sbip->blockHeader.posExamplars++]=current;
				break;
            }
        }
		sbip->blockHeader.UniqueIndexMap[current]=sbip->blockHeader.posExamplars-1;
    }
	
	strlength=snprintf(sbip->lineStr,MAX_BLOCK_LINE_SIZE,"%s\t%s-%s\t<BLOCK>\t.\t.\t.\t.\tVARIANTS=%d;REPS=%d\t.",
		pDataLine[sbip->blockStart].dataInfo.chrom,
		pDataLine[sbip->blockStart].dataInfo.pos,
		pDataLine[sbip->blockEnd].dataInfo.pos,
		sbip->blockEnd-sbip->blockStart+1,
		sbip->blockHeader.posExamplars);
	for(i=0;i<pDataLine[0].numSamples;i++)
	{
		strlength+=snprintf(sbip->lineStr+strlength,MAX_BLOCK_LINE_SIZE-strlength,"\t%d|%d",sbip->blockHeader.UniqueIndexMap[i*2],sbip->blockHeader.UniqueIndexMap[i*2+1]);
	}
	snprintf(sbip->lineStr+strlength,MAX_BLOCK_LINE_SIZE-strlength,"\n");
}

static void getSubBlockRecord(SUB_BLOCK_INFO *sbip,RECORD_BLOCK *rbp,int posRecord)
{
	int i;
	int strlength=0;
	int *altHaploIndex;
	int numAltHaplo=0;
	DATA_LINE *pDataLine=rbp->dataBlock.dataLines;
	
	mallocFreeIntP(&altHaploIndex,sbip->blockHeader.posExamplars,MF_MALLOC);
	for(i=0;i<sbip->blockHeader.posExamplars;i++)
	{
		if('1'==pDataLine[posRecord].gtData[sbip->blockHeader.examplars[i]])
		{
			altHaploIndex[numAltHaplo++]=i;
		}
	}
	memset(sbip->lineStr,0,MAX_BLOCK_LINE_SIZE);
	strlength=snprintf(sbip->lineStr,MAX_BLOCK_LINE_SIZE,"%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t",
		pDataLine[posRecord].dataInfo.chrom,
		pDataLine[posRecord].dataInfo.pos,
		pDataLine[posRecord].dataInfo.ID,
		pDataLine[posRecord].dataInfo.ref,
		pDataLine[posRecord].dataInfo.alt,
		pDataLine[posRecord].dataInfo.qual,
		pDataLine[posRecord].dataInfo.filter,
		pDataLine[posRecord].dataInfo.info);
	
	if(numAltHaplo>0)
	{
		strlength+=snprintf(sbip->lineStr+strlength,MAX_BLOCK_LINE_SIZE-strlength,"%d",altHaploIndex[0]);
		for(i=1;i<numAltHaplo;i++)
		{
			strlength+=snprintf(sbip->lineStr+strlength,MAX_BLOCK_LINE_SIZE-strlength,",%d",altHaploIndex[i]-altHaploIndex[i-1]);
		}
	}
	else
	{
		strlength+=snprintf(sbip->lineStr+strlength,MAX_BLOCK_LINE_SIZE-strlength,"-");
	}
	snprintf(sbip->lineStr+strlength,MAX_BLOCK_LINE_SIZE-strlength,"\n");
	mallocFreeIntP(&altHaploIndex,0,MF_FREE);
}

static void updateDeltaMatrix(RECORD_BLOCK *rbp,int length,TEMP_VARIABLE *tvp)
{
	int i;
    int blockSize=length;
	int diff;
	int numHaplotypes=rbp->dataBlock.dataLines[0].numSamples*2;
	DATA_LINE *pDataLine=rbp->dataBlock.dataLines;
	
    tvp->previousPredecessor[tvp->oldIndex[0]] = -1;
    for (i=1;i<numHaplotypes;i++)
    {
        tvp->previousPredecessor[tvp->oldIndex[i]]=tvp->oldIndex[i-1];
        tvp->previousDifference[tvp->oldIndex[i]]=tvp->firstDifference[i-1];
    }
    for (i=1;i<numHaplotypes;i++)
    {
        if ( tvp->index[i-1]==tvp->previousPredecessor[tvp->index[i]])
        {
            tvp->firstDifference[i-1] =
			pDataLine[length-1].gtData[tvp->index[i]]==
			pDataLine[length-1].gtData[tvp->index[i-1]] ?
            tvp->previousDifference[tvp->index[i]]+1:0;
            continue;
        }

        diff = 0;
        while (diff<length && diff<blockSize &&
			pDataLine[length-diff-1].gtData[tvp->index[i]] ==
			pDataLine[length-diff-1].gtData[tvp->index[i-1]])
        {
        	diff++;
        }

        tvp->firstDifference[i-1]=diff;
    }
}

static void analyzeBlocks(RECORD_BLOCK *rbp,int length,TEMP_VARIABLE *tvp)
{
	int i,j;
	int distinctHaplos;
	int currentCost;
	int blockSize=length;
	int numHaplotypes=rbp->dataBlock.dataLines[0].numSamples*2;
    for (i=1;i<=blockSize && i<=length;i++)
    {
        distinctHaplos = 1;
        for (j=0;j<numHaplotypes-1;j++)
        {
        	 if (i>tvp->firstDifference[j])
        	 {
        	 	 distinctHaplos++;
        	 }   
        }
        currentCost=1;
		if(i>1)
		{
			currentCost=TRANSFACTOR*(numHaplotypes)+(i)*distinctHaplos*CISFACTOR+tvp->cost[length-i+1];
		}
		
		if(i==2)
		{
			tvp->cost[length] = currentCost;
			tvp->bestSlice[length] = 2;
		}
		else if(tvp->cost[length]>currentCost)
		{
			tvp->cost[length]=currentCost;
			tvp->bestSlice[length]=i;
		}
		else if(tvp->cost[length]+TRANSFACTOR*(numHaplotypes)<currentCost)
		{
        	break;
		}
	}
    memcpy(rbp->bestIndex[length-1],tvp->index,(numHaplotypes)*sizeof(int));
}

static void createBoundaries(RECORD_BLOCK *rbp,TEMP_VARIABLE *tvp)
{
    int where=rbp->dataBlock.numDataLines-1;
	memset(rbp->blockBoundaries,0,rbp->dataBlock.numDataLines);
	rbp->posBlockBoundaries=0;
    while (where != 0)
    {
    	rbp->blockBoundaries[rbp->posBlockBoundaries++]=where;
        where=where-tvp->bestSlice[where+1]+1;
    };
}

void compressBlockRecords(RECORD_BLOCK *rbp)
{
	int i,j;
	int offsets[MAXALLELE+1];
	int numHaplotypes=rbp->dataBlock.dataLines[0].numSamples*2;
	DATA_LINE *pDataLine=rbp->dataBlock.dataLines;
	int tmp;
	//static int g_CheckPhased=1;
	//check genotype unphased
	/*if(g_CheckPhased)
	{
		if('/'==pDataLine[0].samplesRawString[1])
		{
			char answer[MIN_STR_SIZE];
			while(1)
			{
				printf("Separator is '/', genotype unphased, whether continue ?  (y/n) ");
				memset(answer,0,MIN_STR_SIZE);
				fgets(answer,MIN_STR_SIZE,stdin);
				if(0==strcmp(answer,"N\n") || 0==strcmp(answer,"n\n"))
				{
					WRITE_LOG("genotype unphased,system quit!\n");
					printf("genotype unphased,system quit!\n");
					exit(1);
				}
				else if (0==strcmp(answer,"Y\n") || 0==strcmp(answer,"y\n"))
				{
					break;
				}
				else
				{
					printf("Invalid answer,please press 'y' or 'n' key!\n");
				}
			}
		}
		g_CheckPhased=0;
	}
	*/
	mallocFreeRecordBlock(rbp,MF_MALLOC);
	
	//temp variable
	TEMP_VARIABLE tempVarible;
	initializeTempVariable(&tempVarible,rbp->dataBlock.numDataLines,numHaplotypes);
	
	for(i=1;i<=rbp->dataBlock.numDataLines;i++)
	{
		memset(offsets,0,sizeof(offsets));
		
		for (j=0;j<numHaplotypes;j++)
		{
			tmp=pDataLine[i-1].gtData[j]-'0';
			if(0==tmp || 1==tmp)
			{
				offsets[tmp+1]++;
			}
			else
			{
				//check without item
				if('-'==pDataLine[i-1].gtData[j] || '.'==pDataLine[i-1].gtData[j])
				{
					zSLMessage("Vcf file missing GT item,system quit\n");
					exit(1);
				}
				//check marker include 2,3.......
				else if(pDataLine[i-1].gtData[j]>'1')
				{
					//if(ANSWER_N==questionUserYN("More than one allele in ALT,system will delete these markers,whether continue ?  (y/n) "))
					zSLMessage("More than one allele in ALT,system quit!\n");
					exit(1);		
				}
				else
				{
					zSLMessage("ALT data error,system quit!\n");
					exit(1);	
				}
			}
		}

		memcpy(tempVarible.oldIndex,tempVarible.index,numHaplotypes*sizeof(int));
		for(j=0;j<numHaplotypes;j++)
		{
			tempVarible.index[offsets[pDataLine[i-1].gtData[tempVarible.oldIndex[j]]-'0']++]=tempVarible.oldIndex[j];
		}

		updateDeltaMatrix(rbp,i,&tempVarible);

		analyzeBlocks(rbp,i,&tempVarible);

    }
	if(rbp->dataBlock.numDataLines>1)
	{
		createBoundaries(rbp,&tempVarible);
	}
	else
	{
		zSLMessage("record num is too little!\n");
	}

	unInitializeTempVariable(&tempVarible);
}

void m3vcfFileWriteRecords(VCF_FILE *mfp,RECORD_BLOCK *rbp)
{
	int i;
	int numHaplotypes=rbp->dataBlock.dataLines[0].numSamples*2;
	SUB_BLOCK_INFO subBlockInfo;
	initializeSubBlockInfo(&subBlockInfo,numHaplotypes);
	subBlockInfo.blockEnd=rbp->blockBoundaries[--(rbp->posBlockBoundaries)];
	for(i=0;i<rbp->dataBlock.numDataLines;i++)
	{
		if(0==i||subBlockInfo.blockStart<i)
		{
			if(i>0)
			{
				i--;
			}
			//get sub block info
			clearSubBlockInfo(&subBlockInfo,numHaplotypes);
			getSubBlockInfo(&subBlockInfo,rbp);
			//write block head
			vcfFileWriteLine(mfp,subBlockInfo.lineStr);
			//update sub block blockEnd and blockStart
			subBlockInfo.blockStart=subBlockInfo.blockEnd;
			subBlockInfo.blockEnd=rbp->blockBoundaries[--(rbp->posBlockBoundaries)];
		}
		//write record
		getSubBlockRecord(&subBlockInfo,rbp,i);
		vcfFileWriteLine(mfp,subBlockInfo.lineStr);
	}
	unInitializeSubBlockInfo(&subBlockInfo);
	mallocFreeRecordBlock(rbp,MF_FREE);
}

void m3vcfFileWriteHead(VCF_FILE *mfp,FILE_HEAD *fhp)
{
	if(VCF_ERROR==vcfFileAddMetaInfoLine(fhp,0,M3VCF_VERSION_STR))
	{
		zSLMessage("add m3vcf head string error\n");
		exit(1);
	}	
	
	if(VCF_ERROR==vcfFileWriteHead(mfp,fhp))
	{
		zSLMessage("Write m3vcf head error\n");
		exit(1);
	}
}

void threadWriteRecords(void * para)
{
	VCF_FILE *p = (VCF_FILE*)para;
	VCF_FILE mfp=*p;	
	zSLMessage("writeRecord thread create!\n");
#ifdef ZZ_TIME	
	struct timeval tt1,tt2;
	unsigned int timeUse=0;
#endif
	TQNode *pTQNode;
	while(0==gBlockQueue.write_endFlag||TLINK_QUEUE_ERROR==is_TQueue_Empty(&(gBlockQueue.queue),TQ_DATA))
	{
#ifdef ZZ_TIME
		gettimeofday(&tt1,NULL);
#endif
		pTQNode=NULL;
		outDataQueue(&gBlockQueue,&pTQNode);

		m3vcfFileWriteRecords(&mfp,&(pTQNode->data));

		inReadyQueue(&gBlockQueue,pTQNode);
#ifdef ZZ_TIME		
		gettimeofday(&tt2,NULL);
		//timeUse = (tt2.tv_sec*1000*1000+tt2.tv_usec)-(tt1.tv_sec*1000*1000+tt1.tv_usec);
		timeUse = tt2.tv_sec-tt1.tv_sec;
		//printf("write use time\t=\t%u s\n",timeUse);
		zSLMessage("write use time\t=\t%u s\n",timeUse);
		fprintf(stderr,"#");
#endif
	}
}

void threadBlockRecords(void * para)
{
	int *p = (int*)para;
	int num = *p;
	pthread_mutex_unlock(&gCreateMutex);
	zSLMessage("block thread %d create!\n",num);
#ifdef ZZ_TIME
	struct timeval tt1,tt2;
	unsigned int timeUse=0;
#endif
	TQNode *pTQNode;
	int numRecords;
	while(0==gBlockQueue.block_endFlag||TLINK_QUEUE_ERROR==is_TQueue_Empty(&(gBlockQueue.queue),TQ_RAWDATA))
	{
#ifdef ZZ_TIME
		gettimeofday(&tt1,NULL);
#endif		
		pTQNode=NULL;
		
		pthread_mutex_lock(&(gBRThreadArgs[num].outReadQueueMutex));
		outRawDataQueue(&gBlockQueue,&pTQNode);
		pthread_mutex_unlock(&(gBRThreadArgs[(num+1)%gCompressThreadNum].outReadQueueMutex));
		
		compressBlockRecords(&(pTQNode->data));
		numRecords=pTQNode->data.dataBlock.numDataLines;
		
		pthread_mutex_lock(&(gBRThreadArgs[num].inWriteQueueMutex));
		inDataQueue(&gBlockQueue,pTQNode);
		pthread_mutex_unlock(&(gBRThreadArgs[(num+1)%gCompressThreadNum].inWriteQueueMutex));
#ifdef ZZ_TIME		
		gettimeofday(&tt2,NULL);
		//timeUse = (tt2.tv_sec*1000*1000+tt2.tv_usec)-(tt1.tv_sec*1000*1000+tt1.tv_usec);
		timeUse = tt2.tv_sec-tt1.tv_sec;
		zSLMessage("block thread %d block use time\t=\t%u s\n",num,timeUse);
#endif
	}
	
	if(numRecords<gBlockQueue.numRecords)
	{
		gBlockQueue.write_endFlag=1;
	}
}

void adjustCompressThreads(int numThread,int limitMemory,int lines,int numSamples)
{
	if(0==numThread && 0==limitMemory)
	{
		return;
	}
	else if(numThread && 0==limitMemory)
	{
		gCompressThreadNum=numThread-READ_THREAD_SIZE-WRITE_THREAD_SIZE;
	}
	else 
	{
		double tmpElement = 1.25;
		double tmpNodeSize=(sizeof(int)*lines+sizeof(int)*(lines+1)*numSamples*2+(numSamples*2+1)*lines+lines*numSamples*10)*tmpElement/1024/1024/1024;
		//todo :the application self occupied momery
		int tmpNodeNum=limitMemory/tmpNodeSize;
		if(0==numThread)
		{
			gCompressThreadNum=tmpNodeNum-ADDITIONAL_LINK_SIZE;
			
		}
		else
		{
			gCompressThreadNum= (numThread-READ_THREAD_SIZE-WRITE_THREAD_SIZE<tmpNodeNum-ADDITIONAL_LINK_SIZE)?
								(numThread-READ_THREAD_SIZE-WRITE_THREAD_SIZE):(tmpNodeNum-ADDITIONAL_LINK_SIZE) ;
		}
		if(gCompressThreadNum<1)
		{
			zSLMessage("Memory of user arrangement too small to run\n");
			exit(1);
		}
	}
}

M3VCF_STATUS vcfCompressToM3vcf(M3VCF_COMPRESS_ARGS *mcap)
{
	//get cost time
	unsigned int timeUse=0;
	struct timeval tStart,tEnd;
#ifdef ZZ_TIME
	struct timeval tt1,tt2;
#endif
	gettimeofday(&tStart,NULL);

	//pipeline thread
	int i;
	pthread_t idBlockRecords,idWriteRecords;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr,PTHREAD_SCOPE_SYSTEM);
	TQNode *pTQNode=NULL;
	long long index=0;
	
	//init log
	zLogInit();
	zSLMessage("compress vcf file[%s] filetype[%s] to m3vcf file[%s] filetype[%s],user allocates %d threads, %dG memory. beginning...\n", 
		mcap->vcfFileName,
		(FILE_MODE_NORMAL==mcap->vcfFileType)?"vcf":"gz",
		mcap->m3vcfFileName,
		(FILE_MODE_NORMAL==mcap->m3vcfFileType)?"m3vcf":"gz",
		mcap->thread_num,
		mcap->memory_limit);
	
	//m3vcf compress
	VCF_FILE fp;
	VCF_FILE mfp;
	VCF_STATUS status;
	int numRecords;

	//open vcf
	if(VCF_ERROR==vcfFileOpen(&fp,mcap->vcfFileName,mcap->vcfFileType,P_GT))
	{
		zSLMessage("Can't open vcf file:%s\n", mcap->vcfFileName);
		return M3VCF_ERROR;
	}

	//read vcf head
	if(VCF_ERROR==vcfFileReadHead(&fp))
	{
		zSLMessage("Read vcf head error\n");
		return M3VCF_ERROR;
	}

	//create m3vcf
	if(VCF_ERROR==vcfFileCreate(&mfp,mcap->m3vcfFileName,mcap->m3vcfFileType))
	{
		zSLMessage("Can't create m3vcf file:%s\n", mcap->m3vcfFileName);
		return M3VCF_ERROR;
	}
	
	//write m3vcf head
	m3vcfFileWriteHead(&mfp,&(fp.head));
	
	//count the key factor of limit, compress thread numbers
	adjustCompressThreads(mcap->thread_num,mcap->memory_limit,mcap->bufferSize,fp.numSamples);
	//init queues
	numRecords=mcap->bufferSize;
	initTLQueueAndThreadArgs(&gBlockQueue,numRecords);
	
	//create write records thread
	if(0!=pthread_create(&idWriteRecords,&attr,(void*)threadWriteRecords,&mfp))
	{
		zSLMessage("write records thread creates error\n");
		return M3VCF_ERROR;
	}
	
	//create block records threads
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_mutex_init(&gCreateMutex,NULL);
	pthread_mutex_lock(&gCreateMutex);
	for(i=0;i<gCompressThreadNum;i++)
	{
		if(0!=pthread_create(&idBlockRecords,&attr,(void*)threadBlockRecords,&i))
		{
			zSLMessage("block records thread creates error!\n");
			return M3VCF_ERROR;
		}
		pthread_mutex_lock(&gCreateMutex);  
	}

	//to log test
	//zSLMessage("numHaplotypes:%d numRecords:%d\n",fp.numSamples*2,numRecords);

	while(1)
	{
		//read a block records
#ifdef ZZ_TIME
		gettimeofday(&tt1,NULL);
#endif
		
		outReadyQueue(&gBlockQueue,&pTQNode);
		
		status=vcfFileReadDataBlockOverlap1Line(&fp,&(pTQNode->data.dataBlock),numRecords);
		if(VCF_EOF == status)
		{
			gBlockQueue.block_endFlag=1;
			break;
		}
#ifdef ZZ_TIME
		gettimeofday(&tt2,NULL);
		//timeUse = (tt2.tv_sec*1000*1000+tt2.tv_usec)-(tt1.tv_sec*1000*1000+tt1.tv_usec);
		timeUse = (tt2.tv_sec)-(tt1.tv_sec);
		zSLMessage("read and parsing records\t=\t%u s\n",timeUse);
#endif
		
		pTQNode->data.index=index++;
	
		inRawDataQueue(&gBlockQueue,pTQNode);
		pTQNode=NULL;
				
		if(VCF_READING_UNFULL==status)
		{
			gBlockQueue.block_endFlag=1;
			break;
		}
	}
	
	//wait for thread end
	pthread_join(idWriteRecords,NULL);
	//unInitMLQueueAndThreadArgs(&gMarkersQueue);
	unInitTLQueueAndThreadArgs(&gBlockQueue);
	fprintf(stderr,"\n");

	//close vcf
	vcfFileClose(&fp);
	//close m3vcf
	vcfFileClose(&mfp);
			
	gettimeofday(&tEnd,NULL);
	//timeUse = (tEnd.tv_sec*1000*1000+tEnd.tv_usec)-(tStart.tv_sec*1000*1000+tStart.tv_usec);
	timeUse = (tEnd.tv_sec-tStart.tv_sec);
	zSLMessage("compress %s use time =%us\n",mcap->vcfFileName,timeUse);
	
	//close log
	zLogUInit();
	
	return M3VCF_OK;
}



/*************************************************convert*****************************************************/



void m3vcfFileReadHead(VCF_FILE *mfp)
{
	if(VCF_ERROR==vcfFileReadHead(mfp))
	{
		zSLMessage("Read m3vcf head error\n");
		exit(1);
	}
	if(VCF_ERROR==vcfFileRemoveMetaInfoLine(&(mfp->head),0))
	{
		zSLMessage("remove m3vcf head string error\n");
		exit(1);
	}	
}

M3VCF_STATUS m3vcfFileReadBlock(VCF_FILE *mfp,M3VCF_BLOCK *mbp)
{
	//read block header line
	int pos=0;
	int i;
	char *tmpLineStr,*pStr,*p;
	char **tmpBlockStrs;
	char str[MAX_SUB_STR_SIZE];
	mallocFreeCharP(&tmpLineStr,MAX_BLOCK_LINE_SIZE,MF_MALLOC);
	pStr=tmpLineStr;
	if(VCF_EOF==vcfFileReadLine(mfp,pStr,MAX_BLOCK_LINE_SIZE))
	{
		mallocFreeCharP(&tmpLineStr,0,MF_FREE);
		return M3VCF_ERROR;
	}
	
	//parsing and filling mbp->m3vcfBlockHeader
	vcfPopSubString(&pStr,str);
	vcfPopSubString(&pStr,str);
	vcfPopSubString(&pStr,str);
	vcfPopSubString(&pStr,str);
	vcfPopSubString(&pStr,str);
	vcfPopSubString(&pStr,str);
	vcfPopSubString(&pStr,str);
	vcfPopSubString(&pStr,str);
	changeString(str);
	sscanf(str,"%d %d",&(mbp->m3vcfBlockHeader.numMarks),&(mbp->m3vcfBlockHeader.posExamplars));
	vcfPopSubString(&pStr,str);
	
	mallocFreeIntP(&(mbp->m3vcfBlockHeader.UniqueIndexMap),2*(mfp->numSamples),MF_MALLOC);
	for(i=0;i<(mfp->numSamples);i++,pos+=2)
	{
		if(VCF_ERROR==vcfPopSubString(&pStr,str))
		{
			break;
		}
		sscanf(str,"%d|%d",&(mbp->m3vcfBlockHeader.UniqueIndexMap[pos]),&(mbp->m3vcfBlockHeader.UniqueIndexMap[pos+1]));
	}
	if(VCF_ERROR!=vcfPopSubString(&pStr,str)||i<(mfp->numSamples))
	{
		zSLMessage("Read m3vcf block header error\n");
		exit(1);
	}
	mbp->m3vcfBlockHeader.numHaplotypes=2*i;
		
	//read mbp->m3vcfBlockHeader.numMarks record  block records
	pStr=tmpLineStr;
	tmpBlockStrs=(char**)malloc(mbp->m3vcfBlockHeader.numMarks*sizeof(char*));
	
	for(i=0;i<mbp->m3vcfBlockHeader.numMarks;i++)
	{
		if(VCF_EOF==vcfFileReadLine(mfp,pStr,MAX_BLOCK_LINE_SIZE))
		{
			zSLMessage("read m3vcf records error\n");
			exit(1);
		}
		tmpBlockStrs[i]=(char*)malloc((strlen(pStr)+1)*sizeof(char));
		strcpy(tmpBlockStrs[i],pStr);
	}

	//parsing and filling mbp->m3vcfRecords
	mbp->m3vcfRecords=(M3VCF_RECORD*)malloc(mbp->m3vcfBlockHeader.numMarks*sizeof(M3VCF_RECORD));
	if(NULL==mbp->m3vcfRecords)
	{
		zSLMessage("malloc m3vcf records error\n");
		exit(1);
	}
	for(i=0;i<mbp->m3vcfBlockHeader.numMarks;i++)
	{
		mbp->m3vcfRecords[i].rawDataLine=strdup(tmpBlockStrs[i]);
		vcfFileParseDataLineInfo(mbp->m3vcfRecords[i].rawDataLine,&(mbp->m3vcfRecords[i].dataInfo));
		mbp->m3vcfRecords[i].numAltHaplo=0;
		//fprintf(stderr,"%s\n",mbp->m3vcfRecords[i].dataInfo.chrom);
		//fprintf(stderr,"%s\n",mbp->m3vcfRecords[i].dataInfo.pos);
		//fprintf(stderr,"%s\n",mbp->m3vcfRecords[i].dataInfo.ID);
		//fprintf(stderr,"%s\n",mbp->m3vcfRecords[i].dataInfo.ref);
		//fprintf(stderr,"%s\n",mbp->m3vcfRecords[i].dataInfo.alt);
		//fprintf(stderr,"%s\n",mbp->m3vcfRecords[i].dataInfo.qual);
		//fprintf(stderr,"%s\n",mbp->m3vcfRecords[i].dataInfo.filter);
		//fprintf(stderr,"%s\n",mbp->m3vcfRecords[i].dataInfo.info);
		//fprintf(stderr,"%s\n",mbp->m3vcfRecords[i].dataInfo.format);
		if(strcmp(mbp->m3vcfRecords[i].dataInfo.format,"-"))
		{//fprintf(stderr,"i=%d\n",i);
			mallocFreeIntP(&(mbp->m3vcfRecords[i].altHaploIndex),mbp->m3vcfBlockHeader.posExamplars,MF_MALLOC);
			pStr=mbp->m3vcfRecords[i].dataInfo.format;
			pos=0;
			while(1)
			{
				p=strsep(&pStr,",");
				if(NULL == p)
				{
					break;
				}
				if(pos)
				{
					mbp->m3vcfRecords[i].altHaploIndex[pos]=atoi(p)+mbp->m3vcfRecords[i].altHaploIndex[pos-1];
				}
				else
				{
					mbp->m3vcfRecords[i].altHaploIndex[pos]=atoi(p);
				}
				pos++;
			}
			mbp->m3vcfRecords[i].numAltHaplo=pos;
		}
		//free(mbp->m3vcfRecords[i].dataInfo.format);
		//mbp->m3vcfRecords[i].dataInfo.format=(char*)malloc(MIN_STR_SIZE*sizeof(char));
		//strcpy(mbp->m3vcfRecords[i].dataInfo.format,"GT");
	}
	mallocFreeCharP(&tmpLineStr,0,MF_FREE);
	mallocFreeCharPP(&(tmpBlockStrs),mbp->m3vcfBlockHeader.numMarks,0,MF_FREE);
	return M3VCF_OK;
}

void m3vcfBlockWriteVcfFile(VCF_FILE *fp,M3VCF_BLOCK *mbp)
{
	static int firstTimeFlag=0;
	char *tmpLineStr;
	int strlength=0;
	int i,j;
	mallocFreeCharP(&tmpLineStr,MAX_BLOCK_LINE_SIZE,MF_MALLOC);
	mallocFreeIntP(&(mbp->m3vcfBlockHeader.examplars),mbp->m3vcfBlockHeader.posExamplars,MF_MALLOC);
	for(i=firstTimeFlag;i<mbp->m3vcfBlockHeader.numMarks;i++)
	{
		//make a line string from mbp->m3vcfRecords
		strlength=snprintf(tmpLineStr,MAX_BLOCK_LINE_SIZE,"%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\tGT",
		mbp->m3vcfRecords[i].dataInfo.chrom,
		mbp->m3vcfRecords[i].dataInfo.pos,
		mbp->m3vcfRecords[i].dataInfo.ID,
		mbp->m3vcfRecords[i].dataInfo.ref,
		mbp->m3vcfRecords[i].dataInfo.alt,
		mbp->m3vcfRecords[i].dataInfo.qual,
		mbp->m3vcfRecords[i].dataInfo.filter,
		mbp->m3vcfRecords[i].dataInfo.info);
		memset(mbp->m3vcfBlockHeader.examplars,0,mbp->m3vcfBlockHeader.posExamplars*sizeof(int));
		//fprintf(stderr,"i=%d numAltHaplo=%d\n",i,mbp->m3vcfRecords[i].numAltHaplo);
		for(j=0;j<mbp->m3vcfRecords[i].numAltHaplo;j++)
		{
			mbp->m3vcfBlockHeader.examplars[mbp->m3vcfRecords[i].altHaploIndex[j]]=1;
		}
		for(j=0;j<mbp->m3vcfBlockHeader.numHaplotypes;j+=2)
		{
			strlength+=snprintf(tmpLineStr+strlength,MAX_BLOCK_LINE_SIZE-strlength,"\t%d|%d",
				mbp->m3vcfBlockHeader.examplars[mbp->m3vcfBlockHeader.UniqueIndexMap[j]],
				mbp->m3vcfBlockHeader.examplars[mbp->m3vcfBlockHeader.UniqueIndexMap[j+1]]);
		}
		snprintf(tmpLineStr+strlength,MAX_BLOCK_LINE_SIZE-strlength,"\n");
		//fprintf(stderr,"j=%d,num=%d\n",j,mbp->m3vcfBlockHeader.numHaplotypes);
		//write a line into vcf 
		vcfFileWriteLine(fp,tmpLineStr);
		
	}
	firstTimeFlag=1;
	FreeM3vcfBlock(mbp);
	mallocFreeCharP(&tmpLineStr,0,MF_FREE);
}


M3VCF_STATUS m3vcfConvertToVcf(M3VCF_CONVERT_ARGS *mcap)
{
	//get cost time
	unsigned int timeUse=0;
	struct timeval tStart,tEnd;
	//struct timeval tt1,tt2;
	gettimeofday(&tStart,NULL);

	//init log
	zLogInit();
	zSLMessage("convert m3vcf file[%s] filetype[%s] to vcf file[%s] filetype[%s]. \nbeginning...\n", 
		mcap->vcfFileName,
		(FILE_MODE_NORMAL==mcap->m3vcfFileType)?"m3vcf":"gz",
		mcap->m3vcfFileName,
		(FILE_MODE_NORMAL==mcap->vcfFileType)?"vcf":"gz");
	//m3vcf convert
	VCF_FILE fp;
	VCF_FILE mfp;
	M3VCF_BLOCK m3vcfBlock;
		
	//open m3vcf
	if(VCF_ERROR==vcfFileOpen(&mfp,mcap->m3vcfFileName,mcap->m3vcfFileType,P_GT))
	{
		zSLMessage("Can't open m3vcf file:%s\n", mcap->m3vcfFileName);
		return 1;
	}

	//read m3vcf head
	m3vcfFileReadHead(&mfp);
	
	//create vcf
	if(VCF_ERROR==vcfFileCreate(&fp,mcap->vcfFileName,mcap->vcfFileType))
	{
		zSLMessage("Can't create vcf file:%s\n", mcap->vcfFileName);
		return 1;
	}
	
	//write vcf head
	vcfFileWriteHead(&fp,&(mfp.head));
	
	zSLMessage("numSamples=%d\n",mfp.numSamples);
	while(M3VCF_OK==m3vcfFileReadBlock(&mfp,&m3vcfBlock))
	{
		m3vcfBlockWriteVcfFile(&fp,&m3vcfBlock);
	}

	//close vcf
	vcfFileClose(&fp);
	//close m3vcf
	vcfFileClose(&mfp);
			
	gettimeofday(&tEnd,NULL);
	//timeUse = (tEnd.tv_sec*1000*1000+tEnd.tv_usec)-(tStart.tv_sec*1000*1000+tStart.tv_usec);
	timeUse = (tEnd.tv_sec-tStart.tv_sec);
	zSLMessage("use time =%us\n",timeUse);

	//close log
	zLogUInit();
	
	return M3VCF_OK;
}

