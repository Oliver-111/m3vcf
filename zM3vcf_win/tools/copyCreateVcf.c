#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "vcflib.h"
#include <time.h>

#define MAX_LINE_BUF			6*1024*1024
#define MAX_SUB_STR			1*1024*1024

void mixOrderRandom(char ** string,int numSamples)
{
	int i;
	long long strlength=0;
	int split = 1+rand()%(numSamples-1);
	char *p=(*string);
	char str[MAX_SUB_STR];
	char *tmp=(char*)malloc(MAX_LINE_BUF*sizeof(char));
	if(NULL==tmp)
	{
		printf("tmp malloc error!\n");
		exit(1);
	}
	//printf("split=%d\n",split);
	
	for(i=0;i<split;i++)
	{
		vcfPopSubString(&p,str);
	}

	strcpy(tmp,p);
	strlength=strlen(tmp)-1;
	*p='\0';
	p--;
	*p='\n';
	snprintf(tmp+strlength,MAX_LINE_BUF-strlength,"\t%s",(*string));
	strcpy((*string),tmp);
	free(tmp);
}

int main(int argc,char** argv )
{
	if(argc != 4 || atoi(argv[2]) <= 1)
	{
		printf("Usage: %s sourceVCFFile sampleTimes targetVCFFile\n",argv[0]);
		return 1;
	}

	VCF_FILE fp1;
	VCF_FILE fp2;
	long long strlength=0;
	int i;
	char str[MAX_SUB_STR];
	srand((unsigned)time(NULL));
	
	char *lineBuf=(char*)malloc(MAX_LINE_BUF*sizeof(char));
	if(NULL==lineBuf)
	{
		printf("lineBuf malloc error!\n");
		return 1;
	}
	char *pRecord;
	char *finalLineBuf=(char*)malloc(MAX_LINE_BUF*sizeof(char));
	if(NULL==finalLineBuf)
	{
		printf("finalLineBuf malloc error!\n");
		return 1;
	}

	//open source vcf file
	if(VCF_ERROR==vcfFileOpen(&fp1,argv[1],FILE_MODE_GZ,P_GT))
	{
		 printf("Can't open vcf file:%s\n", argv[1]);
		 return 1;
	}
	if(VCF_ERROR==vcfFileReadHead(&fp1))
	{
		printf("Read vcf head error\n");
		return 1;
	}

	//create target vcf file
	if(VCF_ERROR==vcfFileCreate(&fp2,argv[3],FILE_MODE_GZ))
	{
		 printf("Can't create vcf file:%s\n", argv[3]);
		 return 1;
	}

	//fix header line and write new vcf head
	memset(finalLineBuf,0,MAX_LINE_BUF);
	strcpy(finalLineBuf,fp1.head.headerLine);
	free(fp1.head.headerLine);
	strlength=strlen(finalLineBuf)-1;
	for(i=0;i<(atoi(argv[2])-1)*fp1.numSamples;i++)
	{
		strlength+=snprintf(finalLineBuf+strlength,MAX_LINE_BUF-strlength,"\t%d",i);
	}
	strlength+=snprintf(finalLineBuf+strlength,MAX_LINE_BUF-strlength,"\n");
	fp1.head.headerLine=(char*)malloc((strlength+1)*sizeof(char));
	if(NULL == fp1.head.headerLine)
	{
		printf("fileHead.headerLine malloc error!\n");
		return 1;
	}
	strcpy(fp1.head.headerLine,finalLineBuf);
	//fp2.numSamples*=(atoi(argv[2]));
	vcfFileWriteHead(&fp2,&(fp1.head));
	
	//read source vcf file, copy sample argv[2] times.
	while(1)
	{
		if(VCF_EOF==vcfFileReadLine(&fp1,lineBuf,MAX_LINE_BUF))
		{
			break;
		}
		pRecord=lineBuf;
		for(i=0;i<9;i++)
		{
			vcfPopSubString(&pRecord,str);
		}
		strcpy(finalLineBuf,lineBuf);
		strlength=strlen(finalLineBuf)-1;
		for(i=1;i<(atoi(argv[2]));i++)
		{
			//mixed order
			mixOrderRandom(&pRecord,fp1.numSamples);
			//connected into marker
			strlength+=snprintf(finalLineBuf+strlength,MAX_LINE_BUF-strlength,"\t%s",pRecord);
			strlength--;
		}
		vcfFileWriteLine(&fp2,finalLineBuf);
	}

	vcfFileClose(&fp1);
	vcfFileClose(&fp2);
	printf("copy sample %s times:%s vcf file(mixed order) done!!!\n",argv[2],argv[3]);
	free(lineBuf);
	free(finalLineBuf);
	return 0;

}


