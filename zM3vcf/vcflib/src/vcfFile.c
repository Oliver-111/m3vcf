#include <string.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>
#include "baseTools.h"
#include "vcflib.h"



#define GT_STR			"GT"
#define DS_STR			"DS"

void printDataLine(DATA_LINE *dlp)
{
	int i;
	//printf("rawDataLine:%s\n",dl->rawDataLine);
	printf("chrom:%s\n",dlp->dataInfo.chrom);
	printf("pos:%s\n",dlp->dataInfo.pos);
	printf("ID:%s\n",dlp->dataInfo.ID);
	printf("ref:%s\n",dlp->dataInfo.ref);
	printf("alt:%s\n",dlp->dataInfo.alt);
	printf("qual:%s\n",dlp->dataInfo.qual);
	printf("filter:%s\n",dlp->dataInfo.filter);
	printf("info:%s\n",dlp->dataInfo.info);
	printf("format:%s\n",dlp->dataInfo.format);
	printf("samplesRawString:%s\n",dlp->samplesRawString);
	printf("numSamples:%d\n",dlp->numSamples);
	printf("gtData:%s\n",dlp->gtData);
	printf("dsData:\n");
	for(i=0;i<dlp->numSamples;i++)
	{
		printf("%f\t",dlp->dsData[i]);
	}
	printf("\n");
}

VCF_STATUS vcfPopSubString(char **lineStr,char *subStr)
{
	if(BT_ERROR==popSubString(lineStr,subStr))
	{
		return VCF_ERROR;
	}
	return VCF_OK;
}


void clearFileHead(FILE_HEAD *fhp)
{
	if(fhp->metaInfoLines)
	{
		btMallocFreeCharPP(&(fhp->metaInfoLines),fhp->numMetaInfoLines,0,BT_FREE);
	}
	if(fhp->headerLine)
	{
		btMallocFreeCharP(&(fhp->headerLine),0,BT_FREE);
	}
	memset(fhp,0,sizeof(FILE_HEAD));
}

void clearDataLine(DATA_LINE *dlp)
{
	if(0==dlp->numSamples)
	{
		return;
	}
	//btMallocFreeCharP(&(dlp->dataInfo.chrom),0,BT_FREE);
	//btMallocFreeCharP(&(dlp->dataInfo.pos),0,BT_FREE);
	//btMallocFreeCharP(&(dlp->dataInfo.ID),0,BT_FREE);
	//btMallocFreeCharP(&(dlp->dataInfo.ref),0,BT_FREE);
	//btMallocFreeCharP(&(dlp->dataInfo.alt),0,BT_FREE);
	//btMallocFreeCharP(&(dlp->dataInfo.qual),0,BT_FREE);
	//btMallocFreeCharP(&(dlp->dataInfo.filter),0,BT_FREE);
	//btMallocFreeCharP(&(dlp->dataInfo.info),0,BT_FREE);
	//btMallocFreeCharP(&(dlp->dataInfo.format),0,BT_FREE);
	//btMallocFreeCharP(&(dlp->samplesRawString),0,BT_FREE);
	btMallocFreeCharP(&(dlp->rawDataLine),0,BT_FREE);
	if(dlp->gtData)
	{
		btMallocFreeCharP(&(dlp->gtData),0,BT_FREE);
	}
	if(dlp->dsData)
	{
		btMallocFreeFloatP(&(dlp->dsData),0,BT_FREE);
	}
	dlp->numSamples=0;
}

void clearDataBlock(DATA_BLOCK *dbp)
{
	if(0==dbp->numDataLines)
	{
		return;
	}

	int i;
	for(i=0;i<dbp->numDataLines;i++)
	{
		clearDataLine(dbp->dataLines+i);
	}
	free(dbp->dataLines);
	dbp->dataLines=NULL;
	dbp->numDataLines=0;
}

static VCF_STATUS vcfCommonOpen(VCF_FILE *fp,const char *fileName,FILE_MODE fileMode,char* openMode)
{
	btLogInit();
	memset(fp,0,sizeof(VCF_FILE));
	if(NULL==fp || NULL==fileName)
	{
		btStdMessage("function:%s parameters error!\n",__func__);
		return VCF_ERROR;
	}
	if(FILE_MODE_NORMAL==fileMode)
	{
		fp->mode = FILE_MODE_NORMAL;
		fp->fp.fp=fopen(fileName,openMode);
		if(NULL == fp->fp.fp)
		{
			btStdMessage("can't open %s file!\n",fileName);
			return VCF_ERROR;
		}
	}
	else if(FILE_MODE_GZ==fileMode)
	{
		fp->mode=FILE_MODE_GZ;
		fp->fp.gfp=gzopen(fileName,openMode);
		if(NULL == fp->fp.gfp)
		{
			btStdMessage("can't open %s file!\n",fileName);
			return VCF_ERROR;
		}
	}
	else
	{
		btStdMessage("function:%s FILE_MODE[%d] parameters error!\n",__func__,fileMode);
		return VCF_ERROR;
	}
	fp->numSamples=0;
	return VCF_OK;
}
	
VCF_STATUS vcfFileOpen(VCF_FILE *fp,const char *fileName,FILE_MODE fileMode,unsigned int parseItem)
{
	if(VCF_ERROR==vcfCommonOpen(fp,fileName,fileMode,"rb"))
	{
		btStdMessage("function:%s return failure!\n",__func__);
		return VCF_ERROR;
	}
	
	if(P_GT!=parseItem && (P_GT|P_DS)!=parseItem)
	{
		btStdMessage("function:%s PARSE_ITEM[%u] parameters error!\n",__func__,parseItem);
		return VCF_ERROR;
	}
	else
	{
		fp->parsingItems=parseItem;
	}
		
	return VCF_OK;
}

VCF_STATUS vcfFileCreate(VCF_FILE *fp,const char *fileName,FILE_MODE fileMode)
{
	if(VCF_ERROR==vcfCommonOpen(fp,fileName,fileMode,"wb"))
	{
		btStdMessage("function:%s error!\n",__func__);
		return VCF_ERROR;
	}
	return VCF_OK;
}

VCF_STATUS vcfFileAppend(VCF_FILE *fp,const char *fileName,FILE_MODE fileMode)
{
	if(VCF_ERROR==vcfCommonOpen(fp,fileName,fileMode,"ab"))
	{
		btStdMessage("function:%s error!\n",__func__);
		return VCF_ERROR;
	}
	return VCF_OK;
}

VCF_STATUS vcfFileClose(VCF_FILE *fp)
{
	if(NULL==fp)
	{
		btStdMessage("function:%s parameters error!\n",__func__);
		return VCF_ERROR;
	}
	
	if(FILE_MODE_NORMAL==fp->mode)
	{
		fclose(fp->fp.fp);
		fp->fp.fp=NULL;
	}
	else if(FILE_MODE_GZ==fp->mode)
	{
		gzclose(fp->fp.gfp);
	}
	clearFileHead(&(fp->head));
	btLogUInit();
	return VCF_OK;
}

VCF_STATUS vcfFileReadLine(VCF_FILE *fp,char *lineStr,int lineSize)
{
	if(NULL==fp || NULL==lineStr)
	{
		btStdMessage("function:%s parameters error!\n",__func__);
		return VCF_ERROR;
	}
	
	//memset(lineStr,0,lineSize);
	
	if(FILE_MODE_NORMAL==fp->mode)
	{
		if(NULL==fgets(lineStr,lineSize,fp->fp.fp))
		{
			return VCF_EOF;
		}
	}
	else if(FILE_MODE_GZ==fp->mode)
	{
		if(NULL==gzgets(fp->fp.gfp,lineStr,lineSize))
		{
			return VCF_EOF;
		}
	}
	return VCF_OK;
}

VCF_STATUS vcfFileReadHead(VCF_FILE *fp)
{
	if(NULL==fp)
	{
		btStdMessage("function:%s parameters error!\n",__func__);
		return VCF_ERROR;
	}
	char *tmpLineStr;
	btMallocFreeCharP(&tmpLineStr,BT_MAX_LINE_SIZE,BT_MALLOC);
	//clearFileHead(fhp);
	while(1)
	{
		memset(tmpLineStr,0,BT_MAX_LINE_SIZE);
		if(VCF_EOF==vcfFileReadLine(fp,tmpLineStr,BT_MAX_LINE_SIZE))
		{
			btStdMessage("file error:meet EOF when read head line!\n");
			btMallocFreeCharP(&tmpLineStr,0,BT_FREE);	
			return VCF_EOF;
		}
		if('#'==tmpLineStr[0] && '#'==tmpLineStr[1])
		{
			if(0==fp->head.numMetaInfoLines)
			{
				btMallocFreeCharPP(&(fp->head.metaInfoLines),1+(fp->head.numMetaInfoLines),strlen(tmpLineStr)+1,BT_MALLOC);
			}
			else
			{
				btAddCharPP(&(fp->head.metaInfoLines),1+(fp->head.numMetaInfoLines),strlen(tmpLineStr)+1);
			}
			strcpy(fp->head.metaInfoLines[fp->head.numMetaInfoLines++],tmpLineStr);
			
		}
		else if('#'==tmpLineStr[0] && 'C'==tmpLineStr[1])
		{
			btMallocFreeCharP(&(fp->head.headerLine),strlen(tmpLineStr)+1,BT_MALLOC);
			strcpy(fp->head.headerLine,tmpLineStr);
			break;
		}
		else
		{
			btMallocFreeCharP(&tmpLineStr,0,BT_FREE);	
			return VCF_ERROR;
		}
	}
	//get numSamples
	fp->numSamples=countNumSubString(fp->head.headerLine)-9;
	btMallocFreeCharP(&tmpLineStr,0,BT_FREE);		
	
	return VCF_OK;
}

VCF_STATUS getNumMetaInfoLines(FILE_HEAD *fhp,int *NumMetaInfoLines)
{
	if(NULL==fhp || NULL==NumMetaInfoLines)
	{
		btStdMessage("function:%s parameters error!\n",__func__);
		return VCF_ERROR;
	}
	(*NumMetaInfoLines)=fhp->numMetaInfoLines;
	return VCF_OK;
}

VCF_STATUS getNumSamples(VCF_FILE *fp,int *NumSamples)
{
	if(NULL==fp || NULL==NumSamples)
	{
		btStdMessage("function:%s parameters error!\n",__func__);
		return VCF_ERROR;
	}
	if(0==fp->numSamples)
	{
		btStdMessage("getNumSamples function must called after vcfFileReadHead function!\n");
		btStdMessage("function:%s use error!\n",__func__);
		return VCF_ERROR;
	}
	
	(*NumSamples)=fp->numSamples;
	return VCF_OK;
}

char* vcfFileParseDataLineInfo(char *lineStr,DATA_INFO *dataInfo)
{
	//char str[BT_MAX_SUB_STR_SIZE];
	//memset(str,0,BT_MAX_SUB_STR_SIZE);
	//popSubString(&lineStr,str);
	//btMallocFreeCharP(&(dlp->dataInfo.chrom),strlen(str)+1,BT_MALLOC);
	//strcpy(dlp->dataInfo.chrom,str);
	//dataInfo->chrom=strdup(str);
	popSubAddress(&lineStr,&(dataInfo->chrom));

	//memset(str,0,BT_MAX_SUB_STR_SIZE);
	//popSubString(&lineStr,str);
	//btMallocFreeCharP(&(dlp->dataInfo.pos),strlen(str)+1,BT_MALLOC);
	//strcpy(dlp->dataInfo.pos,str);
	//dataInfo->pos=strdup(str);
	popSubAddress(&lineStr,&(dataInfo->pos));
	
	//memset(str,0,BT_MAX_SUB_STR_SIZE);
	//popSubString(&lineStr,str);
	//btMallocFreeCharP(&(dlp->dataInfo.ID),strlen(str)+1,BT_MALLOC);
	//strcpy(dlp->dataInfo.ID,str);
	//dataInfo->ID=strdup(str);
	popSubAddress(&lineStr,&(dataInfo->ID));

	//memset(str,0,BT_MAX_SUB_STR_SIZE);
	//popSubString(&lineStr,str);
	//btMallocFreeCharP(&(dlp->dataInfo.ref),strlen(str)+1,BT_MALLOC);
	//strcpy(dlp->dataInfo.ref,str);
	//dataInfo->ref=strdup(str);
	popSubAddress(&lineStr,&(dataInfo->ref));

	//memset(str,0,BT_MAX_SUB_STR_SIZE);
	//popSubString(&lineStr,str);
	//btMallocFreeCharP(&(dlp->dataInfo.alt),strlen(str)+1,BT_MALLOC);
	//strcpy(dlp->dataInfo.alt,str);
	//dataInfo->alt=strdup(str);
	popSubAddress(&lineStr,&(dataInfo->alt));

	//memset(str,0,BT_MAX_SUB_STR_SIZE);
	//popSubString(&lineStr,str);
	//btMallocFreeCharP(&(dlp->dataInfo.qual),strlen(str)+1,BT_MALLOC);
	//strcpy(dlp->dataInfo.qual,str);
	//dataInfo->qual=strdup(str);
	popSubAddress(&lineStr,&(dataInfo->qual));

	//memset(str,0,BT_MAX_SUB_STR_SIZE);
	//popSubString(&lineStr,str);
	//btMallocFreeCharP(&(dlp->dataInfo.filter),strlen(str)+1,BT_MALLOC);
	//strcpy(dlp->dataInfo.filter,str);
	//dataInfo->filter=strdup(str);
	popSubAddress(&lineStr,&(dataInfo->filter));

	//memset(str,0,BT_MAX_SUB_STR_SIZE);
	//popSubString(&lineStr,str);
	//btMallocFreeCharP(&(dlp->dataInfo.info),strlen(str)+1,BT_MALLOC);
	//strcpy(dlp->dataInfo.info,str);
	//dataInfo->info=strdup(str);
	popSubAddress(&lineStr,&(dataInfo->info));

	//memset(str,0,BT_MAX_SUB_STR_SIZE);
	//popSubString(&lineStr,str);
	//btMallocFreeCharP(&(dlp->dataInfo.format),strlen(str)+1,BT_MALLOC);
	//strcpy(dlp->dataInfo.format,str);
	//dataInfo->format=strdup(str);
	popSubAddress(&lineStr,&(dataInfo->format));

	return lineStr;
}


VCF_STATUS vcfFileParseDataLine(VCF_FILE *fp,char *lineStr,DATA_LINE *dlp)
{
	if(NULL==lineStr || NULL==dlp)
	{
		btStdMessage("function:%s parameters error!\n",__func__);
		return VCF_ERROR;
	}
	char str[BT_MAX_STR_SIZE];
	int gtPos,dsPos;
	int gtArrPos=0;
	int dsArrPos=0;
	char itemStr[BT_MIN_STR_SIZE];
	
	dlp->rawDataLine=strdup(lineStr);
	
	//fill datainfo
	lineStr=vcfFileParseDataLineInfo(dlp->rawDataLine,&(dlp->dataInfo));
	
	//fill data part
	dlp->samplesRawString=lineStr;
	
	//check gt 
	//if(0==fp->parsingItems & P_GT)
	{
		if(0 == (gtPos=getStrPosition(dlp->dataInfo.format,GT_STR)))
		{
			btStdMessage("Vcf file without GT format,system quit\n");
			exit(1);
		}
		btMallocFreeCharP(&(dlp->gtData),fp->numSamples*2+1,BT_MALLOC);
	}
	//check ds 
	if(fp->parsingItems & P_DS)
	{
		if(0 == (dsPos=getStrPosition(dlp->dataInfo.format,DS_STR)))
		{
			btStdMessage("Vcf file without DS format,system quit\n");
			exit(1);
		}
		btMallocFreeFloatP(&(dlp->dsData),fp->numSamples,BT_MALLOC);
	}
	
	while(1)
	{
		
		if(BT_ERROR==popSubString(&lineStr,str))
		{
			break;
		}
		
		//parsing gt
		//if(0==fp->parsingItems & P_GT)
		{
			dlp->gtData[gtArrPos++]=str[0];
			dlp->gtData[gtArrPos++]=str[2];
		}
		
		//parsing ds
		if(fp->parsingItems & P_DS)
		{
			if(BT_OK==getPositionStr(str,dsPos,itemStr))
			{
				dlp->dsData[dsArrPos++]=atof(itemStr);
			}
			else
			{
				btStdMessage("###rs:%s\tchr:%s\tpos:%s### DS item wrong!\n",dlp->dataInfo.ID,dlp->dataInfo.chrom,dlp->dataInfo.pos);
			}
		}

		dlp->numSamples++;
	}
	
	if(dlp->numSamples!=fp->numSamples)
	{
		btStdMessage("###rs:%s\tchr:%s\tpos:%s### marker samples number wrong!\n",dlp->dataInfo.ID,dlp->dataInfo.chrom,dlp->dataInfo.pos);
		btStdMessage("file sample number:%d marker sample number:%d\n",fp->numSamples,dlp->numSamples);
	}
	return VCF_OK;
}

VCF_STATUS vcfFileReadDataLine(VCF_FILE *fp,DATA_LINE *dlp)
{
	if(NULL==fp || NULL==dlp)
	{
		btStdMessage("function:%s parameters error!\n",__func__);
		return VCF_ERROR;
	}
	char *tmpLineStr;
	btMallocFreeCharP(&tmpLineStr,BT_MAX_LINE_SIZE,BT_MALLOC);
	//memset(tmpLineStr,0,BT_MAX_LINE_SIZE);

	//clear dataline
	clearDataLine(dlp);
	
	if(VCF_EOF==vcfFileReadLine(fp,tmpLineStr,BT_MAX_LINE_SIZE))
	{
		btMallocFreeCharP(&tmpLineStr,0,BT_FREE);	
		return VCF_EOF;
	}
	vcfFileParseDataLine(fp,tmpLineStr,dlp);
	btMallocFreeCharP(&tmpLineStr,0,BT_FREE);

	//check number of head line items and number of samples for equality
	if(dlp->numSamples!=fp->numSamples)
	{
		btStdMessage("function:%s Vcf data line samples number error,system quit\n",__func__);
		exit(1);
	}
	return VCF_OK;
}

VCF_STATUS vcfFileReadDataBlock(VCF_FILE *fp,DATA_BLOCK *dbp,int numLines)
{
	if(NULL==fp || NULL==dbp)
	{
		btStdMessage("function:%s parameters error!\n",__func__);
		return VCF_ERROR;
	}
	int i,nLines=numLines;
	char *tmpLineStr;
	char **tmpBlockStrs;
	
	btMallocFreeCharP(&tmpLineStr,BT_MAX_LINE_SIZE,BT_MALLOC);
	if(VCF_EOF==vcfFileReadLine(fp,tmpLineStr,BT_MAX_LINE_SIZE))
	{
		btMallocFreeCharP(&tmpLineStr,0,BT_FREE);
		return VCF_EOF;
	}
	btMallocFreeCharPP(&(tmpBlockStrs),1,strlen(tmpLineStr)+1,BT_MALLOC);
	strcpy(tmpBlockStrs[0],tmpLineStr);
	
	//free dataBlock;
	clearDataBlock(dbp);

	//read data block
	for(i=1;i<nLines;i++)
	{
		//memset(tmpLineStr,0,BT_MAX_LINE_SIZE);
		if(VCF_EOF==vcfFileReadLine(fp,tmpLineStr,BT_MAX_LINE_SIZE))
		{
			break;
		}
		btAddCharPP(&(tmpBlockStrs),1+i,strlen(tmpLineStr)+1);
		strcpy(tmpBlockStrs[i],tmpLineStr);
	}
	if(i<nLines)
	{
		nLines=i;
	}
	btMallocFreeCharP(&tmpLineStr,0,BT_FREE);

	//parsing data block
	dbp->numDataLines=nLines;
	dbp->dataLines=(DATA_LINE*)malloc(nLines*sizeof(DATA_LINE));
	if(NULL==dbp->dataLines)
	{
		btStdMessage("DATA_LINE* malloc error!\n");
		exit(1);
	}
	//#pragma omp parallel for num_threads(30) private(pRecord,str,pStr,p,k,j)
	#pragma omp parallel for num_threads(OPEN_MP_THREAD_NUM)
	for(i=0;i<nLines;i++)
	{
		memset((dbp->dataLines+i),0,sizeof(DATA_LINE));
		vcfFileParseDataLine(fp,tmpBlockStrs[i],(dbp->dataLines+i));
		//check number of head line items and number of samples for equality
		if(dbp->dataLines[i].numSamples!=fp->numSamples)
		{
			btStdMessage("function:%s Vcf data line samples number error,system quit\n",__func__);
			exit(1);
		}
	}
	btMallocFreeCharPP(&(tmpBlockStrs),i,0,BT_FREE);
	if(nLines!=numLines)
	{
		return VCF_READING_UNFULL;
	}
	return VCF_OK;
}

VCF_STATUS vcfFileReadDataBlockOverlap1Line(VCF_FILE *fp,DATA_BLOCK *dbp,int numLines)
{
	if(NULL==fp || NULL==dbp)
	{
		btStdMessage("function:%s parameters error!\n",__func__);
		return VCF_ERROR;
	}
	static char *tmpLineStr=NULL;
	int i,nLines=numLines;
	char **tmpBlockStrs;
	
	//get cost time
	//unsigned int timeUse=0;
	//struct timeval tStart,tEnd;
	//gettimeofday(&tStart,NULL);
	
	if(NULL==tmpLineStr)
	{
		btMallocFreeCharP(&tmpLineStr,BT_MAX_LINE_SIZE,BT_MALLOC);
		if(VCF_EOF==vcfFileReadLine(fp,tmpLineStr,BT_MAX_LINE_SIZE))
		{
			btMallocFreeCharP(&tmpLineStr,0,BT_FREE);
			return VCF_EOF;
		}
	}
	btMallocFreeCharPP(&(tmpBlockStrs),1,strlen(tmpLineStr)+1,BT_MALLOC);
	strcpy(tmpBlockStrs[0],tmpLineStr);
	
	//free dataBlock;
	clearDataBlock(dbp);
	
	//read data block
	for(i=1;i<nLines;i++)
	{
		//memset(tmpLineStr,0,BT_MAX_LINE_SIZE);
		if(VCF_EOF==vcfFileReadLine(fp,tmpLineStr,BT_MAX_LINE_SIZE))
		{
			break;
		}
		btAddCharPP(&(tmpBlockStrs),1+i,strlen(tmpLineStr)+1);
		strcpy(tmpBlockStrs[i],tmpLineStr);
	}
	if(i<nLines)
	{
		if(1==i)
		{
			btMallocFreeCharPP(&(tmpBlockStrs),i,0,BT_FREE);
			btMallocFreeCharP(&tmpLineStr,0,BT_FREE);
			return VCF_EOF;
		}
		nLines=i;
	}
	//gettimeofday(&tEnd,NULL);
	//timeUse = (tEnd.tv_sec)-(tStart.tv_sec);
	//btStdMessage("read records\t=\t%u s\n",timeUse);

	
	//parsing data block
	dbp->numDataLines=nLines;
	dbp->dataLines=(DATA_LINE*)malloc(nLines*sizeof(DATA_LINE));
	if(NULL==dbp->dataLines)
	{
		btStdMessage("DATA_LINE* malloc error!\n");
		exit(1);
	}
	
	//#pragma omp parallel for num_threads(30) private(pRecord,str,pStr,p,k,j)
	#pragma omp parallel for num_threads(OPEN_MP_THREAD_NUM)
	for(i=0;i<nLines;i++)
	{
		memset((dbp->dataLines+i),0,sizeof(DATA_LINE));
		vcfFileParseDataLine(fp,tmpBlockStrs[i],(dbp->dataLines+i));
		//check number of head line items and number of samples for equality
		if(dbp->dataLines[i].numSamples!=fp->numSamples)
		{
			btStdMessage("function:%s Vcf data line samples number error,system quit\n",__func__);
			exit(1);
		}
	}
	btMallocFreeCharPP(&(tmpBlockStrs),i,0,BT_FREE);
	if(nLines!=numLines)
	{
		btMallocFreeCharP(&tmpLineStr,0,BT_FREE);
		return VCF_READING_UNFULL;
	}
	
	return VCF_OK;
}

VCF_STATUS vcfFileWriteLine(VCF_FILE *fp,char *lineStr)
{
	if(NULL==fp || NULL==lineStr)
	{
		btStdMessage("function:%s parameters error!\n",__func__);
		return VCF_ERROR;
	}

	if(FILE_MODE_NORMAL==fp->mode)
	{
		fputs(lineStr,fp->fp.fp);
	}
	else if(FILE_MODE_GZ==fp->mode)
	{
		gzputs(fp->fp.gfp,lineStr);
	}
	return VCF_OK;
}

VCF_STATUS vcfFileWriteHead(VCF_FILE *fp,FILE_HEAD *fhp)
{
	if(NULL==fp || NULL==fhp)
	{
		btStdMessage("function:%s parameters error!\n",__func__);
		return VCF_ERROR;
	}

	int i;
	for(i=0;i<fhp->numMetaInfoLines;i++)
	{
		vcfFileWriteLine(fp,fhp->metaInfoLines[i]);
	}
	vcfFileWriteLine(fp,fhp->headerLine);
	
	return VCF_OK;
}

VCF_STATUS vcfFileAddMetaInfoLine(FILE_HEAD *fhp,int posIndex,char *MetaInfoLine)
{
	if(NULL==MetaInfoLine || NULL==fhp || posIndex<0)
	{
		btStdMessage("function:%s parameters error!\n",__func__);
		return VCF_ERROR;
	}
	int numML=0;
	int i;
	char *tmpChr;
	getNumMetaInfoLines(fhp,&numML);
	
	// add metainfoline	
	i=strlen(MetaInfoLine);
	if('\n'!=MetaInfoLine[i-1])
	{
		btAddCharPP(&(fhp->metaInfoLines),1+numML,i+2);
		sprintf(fhp->metaInfoLines[numML],"%s\n",MetaInfoLine);
	}
	else
	{
		btAddCharPP(&(fhp->metaInfoLines),1+numML,i+1);
		strcpy(fhp->metaInfoLines[numML],MetaInfoLine);
	}
	fhp->numMetaInfoLines++;
	//sort
	if(posIndex >= numML)
	{
		return VCF_OK;
	}
	tmpChr=fhp->metaInfoLines[numML];
	for(i=numML;i>posIndex;i--)
	{
		fhp->metaInfoLines[i]=fhp->metaInfoLines[i-1];
	}
	fhp->metaInfoLines[i]=tmpChr;
	return VCF_OK;
}

VCF_STATUS vcfFileRemoveMetaInfoLine(FILE_HEAD *fhp,int posIndex)
{
	if(posIndex<0 || NULL==fhp)
	{
		btStdMessage("function:%s parameters error!\n",__func__);
		return VCF_ERROR;
	}
	int numML=0;
	int i;
	getNumMetaInfoLines(fhp,&numML);
	if(posIndex<numML)
	{
		for(i=posIndex;i<numML-1;i++)
		{
			fhp->metaInfoLines[i]=fhp->metaInfoLines[i+1];
		}
	}
	btDelCharPP(&(fhp->metaInfoLines),numML-1);
	fhp->numMetaInfoLines--;
	return VCF_OK;
}



//all format interface
//parse a data line string to all format DATA_LINE structure
VCF_STATUS vcfFileParseDataLine_allFormat(VCF_FILE *fp,char *lineStr,DATA_LINE_ALL_FORMAT *dlafp)
{
	if(NULL==lineStr || NULL==dlafp)
	{
		btStdMessage("function:%s parameters error!\n",__func__);
		return VCF_ERROR;
	}
	char itemStr[BT_MIN_STR_SIZE];
	char *tmpStr;
	int i,j;
	
	dlafp->rawDataLine=strdup(lineStr);
	
	//fill datainfo
	lineStr=vcfFileParseDataLineInfo(dlafp->rawDataLine,&(dlafp->dataInfo));
	
	//fill data part
	dlafp->numFormats=getSTRNum(dlafp->dataInfo.format);
	dlafp->numSamples=countNumSubString(lineStr);
	if(dlafp->numSamples!=fp->numSamples)
	{
		btStdMessage("###rs:%s\tchr:%s\tpos:%s### marker samples number wrong!\n",dlafp->dataInfo.ID,dlafp->dataInfo.chrom,dlafp->dataInfo.pos);
		btStdMessage("file sample number:%d marker sample number:%d\n",fp->numSamples,dlafp->numSamples);
		exit(1);
	}
	dlafp->dataFormatStr=(DATA_FORMAT_STR*)malloc(dlafp->numFormats*sizeof(DATA_FORMAT_STR));
	if(NULL == (dlafp->dataFormatStr))
	{
		btStdMessage("DATA_FORMAT_STR* malloc error!\n");
		exit(1);
	}
	
	for(i=0;i<dlafp->numFormats;i++)
	{
		getPositionStr(dlafp->dataInfo.format,i+1,itemStr);
		dlafp->dataFormatStr[i].format=strdup(itemStr);
		dlafp->dataFormatStr[i].dataStr=(char**)malloc(dlafp->numSamples*sizeof(char*));
		if(NULL==dlafp->dataFormatStr[i].dataStr)
		{
			btStdMessage("dataStr malloc error!\n");
			exit(1);
		}		
	}

	for(i=0;i<dlafp->numSamples;i++)
	{
		if(BT_ERROR==popSubAddress(&lineStr,&tmpStr))
		{
			btStdMessage("pop sub string error!\n");
			exit(1);
		}
		
		for(j=0;j<dlafp->numFormats;j++)
		{
			popStrAddress(&tmpStr,&(dlafp->dataFormatStr[j].dataStr[i]));
		}
	}

	return VCF_OK;
}

//read a data line to DATA_LINE structure
VCF_STATUS vcfFileReadDataLine_allFormat(VCF_FILE *fp,DATA_LINE_ALL_FORMAT *dlafp)
{
	if(NULL==fp || NULL==dlafp)
	{
		btStdMessage("function:%s parameters error!\n",__func__);
		return VCF_ERROR;
	}
	char *tmpLineStr;
	btMallocFreeCharP(&tmpLineStr,BT_MAX_LINE_SIZE,BT_MALLOC);
	//memset(tmpLineStr,0,BT_MAX_LINE_SIZE);

	//clear dataline
	clearDataLine_allFormat(dlafp);
	
	if(VCF_EOF==vcfFileReadLine(fp,tmpLineStr,BT_MAX_LINE_SIZE))
	{
		btMallocFreeCharP(&tmpLineStr,0,BT_FREE);	
		return VCF_EOF;
	}
	vcfFileParseDataLine_allFormat(fp,tmpLineStr,dlafp);
	btMallocFreeCharP(&tmpLineStr,0,BT_FREE);

	//check number of head line items and number of samples for equality
	if(dlafp->numSamples!=fp->numSamples)
	{
		btStdMessage("function:%s Vcf data line samples number error,system quit\n",__func__);
		exit(1);
	}
	return VCF_OK;
}

//read numLines data lines to DATA_BLOCK structure(multilthreads)
VCF_STATUS vcfFileReadDataBlock_allFormat(VCF_FILE *fp,DATA_BLOCK_ALL_FORMAT *dbafp,int numLines)
{
	if(NULL==fp || NULL==dbafp)
	{
		btStdMessage("function:%s parameters error!\n",__func__);
		return VCF_ERROR;
	}
	int i,nLines=numLines;
	char *tmpLineStr;
	char **tmpBlockStrs;
	
	btMallocFreeCharP(&tmpLineStr,BT_MAX_LINE_SIZE,BT_MALLOC);
	if(VCF_EOF==vcfFileReadLine(fp,tmpLineStr,BT_MAX_LINE_SIZE))
	{
		btMallocFreeCharP(&tmpLineStr,0,BT_FREE);
		return VCF_EOF;
	}
	btMallocFreeCharPP(&(tmpBlockStrs),1,strlen(tmpLineStr)+1,BT_MALLOC);
	strcpy(tmpBlockStrs[0],tmpLineStr);
	
	//free dataBlock;
	clearDataBlock_allFormat(dbafp);

	//read data block
	for(i=1;i<nLines;i++)
	{
		//memset(tmpLineStr,0,BT_MAX_LINE_SIZE);
		if(VCF_EOF==vcfFileReadLine(fp,tmpLineStr,BT_MAX_LINE_SIZE))
		{
			break;
		}
		btAddCharPP(&(tmpBlockStrs),1+i,strlen(tmpLineStr)+1);
		strcpy(tmpBlockStrs[i],tmpLineStr);
	}
	if(i<nLines)
	{
		nLines=i;
	}
	btMallocFreeCharP(&tmpLineStr,0,BT_FREE);

	//parsing data block
	dbafp->numDataLines=nLines;
	dbafp->dataLines=(DATA_LINE_ALL_FORMAT*)malloc(nLines*sizeof(DATA_LINE_ALL_FORMAT));
	if(NULL==dbafp->dataLines)
	{
		btStdMessage("DATA_LINE* malloc error!\n");
		exit(1);
	}
	//#pragma omp parallel for num_threads(30) private(pRecord,str,pStr,p,k,j)
	#pragma omp parallel for num_threads(OPEN_MP_THREAD_NUM)
	for(i=0;i<nLines;i++)
	{
		memset((dbafp->dataLines+i),0,sizeof(DATA_LINE_ALL_FORMAT));
		vcfFileParseDataLine_allFormat(fp,tmpBlockStrs[i],(dbafp->dataLines+i));
		//check number of head line items and number of samples for equality
		if(dbafp->dataLines[i].numSamples!=fp->numSamples)
		{
			btStdMessage("function:%s Vcf data line samples number error,system quit\n",__func__);
			exit(1);
		}
	}
	btMallocFreeCharPP(&(tmpBlockStrs),i,0,BT_FREE);
	if(nLines!=numLines)
	{
		return VCF_READING_UNFULL;
	}
	return VCF_OK;
}

void clearDataLine_allFormat(DATA_LINE_ALL_FORMAT *dlafp)
{
	if(0==dlafp->numSamples)
	{
		return;
	}
	
	btMallocFreeCharP(&(dlafp->rawDataLine),0,BT_FREE);
	if(dlafp->numFormats)
	{
		int i;
		for(i=0;i<dlafp->numFormats;i++)
		{
			btMallocFreeCharP(&(dlafp->dataFormatStr[i].format),0,BT_FREE);
			free(dlafp->dataFormatStr[i].dataStr);
		}
		free(dlafp->dataFormatStr);
	}
	dlafp->numSamples=0;
	dlafp->numFormats=0;
}
void clearDataBlock_allFormat(DATA_BLOCK_ALL_FORMAT *dbafp)
{
	if(0==dbafp->numDataLines)
	{
		return;
	}

	int i;
	for(i=0;i<dbafp->numDataLines;i++)
	{
		clearDataLine_allFormat(dbafp->dataLines+i);
	}
	free(dbafp->dataLines);
	dbafp->dataLines=NULL;
	dbafp->numDataLines=0;
}

void printDataLine_allFormat(DATA_LINE_ALL_FORMAT *dlafp)
{
	int i,j;
	//printf("rawDataLine:%s\n",dl->rawDataLine);
	printf("chrom:%s\n",dlafp->dataInfo.chrom);
	printf("pos:%s\n",dlafp->dataInfo.pos);
	printf("ID:%s\n",dlafp->dataInfo.ID);
	printf("ref:%s\n",dlafp->dataInfo.ref);
	printf("alt:%s\n",dlafp->dataInfo.alt);
	printf("qual:%s\n",dlafp->dataInfo.qual);
	printf("filter:%s\n",dlafp->dataInfo.filter);
	printf("info:%s\n",dlafp->dataInfo.info);
	printf("format:%s\n",dlafp->dataInfo.format);
	printf("numSamples:%d\n",dlafp->numSamples);
	printf("numFormats:%d\n",dlafp->numFormats);
		
	for(i=0;i<dlafp->numFormats;i++)
	{
		printf("%s\n",dlafp->dataFormatStr[i].format);
		for(j=0;j<dlafp->numSamples;j++)
		{
			printf("%s\t",dlafp->dataFormatStr[i].dataStr[j]);
		}
		printf("\n");
	}
	printf("\n");
}


