#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "vcflib.h"

// cut argv[3] markers from large vcf file to new vcf
#define MAX_LINE_BUF_SIZE	6*1024*1024

int main(int argc,char** argv )
{
	if(argc != 5 || atoi(argv[3]) < 0 || atoi(argv[4]) < 0)
	{
		printf("Usage: %s sourceVCFFile targetVCFFile markersNumber samplesNumber\n",argv[0]);
		printf("       if markersNumber = 0, targetVCFile's markersNumber= sourceVCFFile's markersNumber\n");
		printf("       if samplesNumber = 0, targetVCFile's samplesNumber= sourceVCFFile's samplesNumber\n");
		return 1;
	}

	VCF_FILE fp1;
	VCF_FILE fp2;
	char str[MAX_LINE_BUF_SIZE];
	char *pRecord;

	
	char *lineBuf=(char*)malloc(sizeof(char)*MAX_LINE_BUF_SIZE);
	int i;
	int markers=0;
		
	
	//open source vcf file
	if(VCF_ERROR==vcfFileOpen(&fp1,argv[1],FILE_MODE_GZ,P_GT))
	{
		 printf("Can't open m3vcf file:%s\n", argv[1]);
		 return 1;
	}
	if(VCF_ERROR==vcfFileReadHead(&fp1))
	{
		printf("Read vcf head error\n");
		return 1;
	}

	if(fp1.numSamples < atoi(argv[4]))
	{
		printf("User input sampleNumber is bigger than the sampleNumber of sourceVCFFile!\n");
		printf("tools can't cut!!!");
		return 1;
	}

	pRecord=fp1.head.headerLine;
	if(0 != atoi(argv[4]))
	{
		for(i=0;i<(9+atoi(argv[4]));i++)
		{
			vcfPopSubString(&pRecord,str);
		}
		*pRecord='\0';
		*(--pRecord)='\n';
	}
	
	//creat target vcf file	
	if(VCF_ERROR==vcfFileCreate(&fp2,argv[2],FILE_MODE_GZ))
	{
		 printf("Can't create new m3vcf file:%s\n", argv[2]);
		 return 1;
	}
	vcfFileWriteHead(&fp2,&(fp1.head));
	
	while(1)
	{
		memset(lineBuf,0,MAX_LINE_BUF_SIZE);
		if(VCF_OK != vcfFileReadLine(&fp1,lineBuf,MAX_LINE_BUF_SIZE))
		{
			break;
		}
		if(0 != atoi(argv[4]))
		{
			pRecord=lineBuf;
			for(i=0;i<(9+atoi(argv[4]));i++)
			{
				vcfPopSubString(&pRecord,str);
			}
			*pRecord='\0';
			*(--pRecord)='\n';
		}
		vcfFileWriteLine(&fp2,lineBuf);
		fprintf(stderr,"#");
		markers++;
		if(atoi(argv[3]) == 0 )
		{
			continue;
		}
		else if(atoi(argv[3]) ==  markers)
		{
			break;
		}
	}
	
	fprintf(stderr,"\n");
	free(lineBuf);
	vcfFileClose(&fp1);
	vcfFileClose(&fp2);
	
	return 0;
}

