#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include "vcflib.h"

#define MAX_LINE_BUF 		6*1024*1024
#define MAX_SUB_STR			1*1024*1024

int getItem(char *pLineBuf)
{
	char str[MAX_SUB_STR];
	int i=0;
	while(VCF_OK==vcfPopSubString(&pLineBuf,str))		
	{
		i++;
	}
	return i;
}

int main(int argc,char** argv )
{
	if(argc != 2)
	{
		printf("Usage: %s VCFFile\n",argv[0]);
		return 1;
	}
	VCF_FILE fp1;
	char *lineBuf=(char*)malloc(sizeof(char)*MAX_LINE_BUF);
	int numSamples=0;
	int numMarkers=0;
	int items;

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
	numSamples=fp1.numSamples;
	
	while(1)
	{
		if(VCF_EOF==vcfFileReadLine(&fp1,lineBuf,MAX_LINE_BUF))
		{
			break;
		}
		else
		{
			numMarkers++;
			items=getItem(lineBuf);
			if(numSamples+9!=items)
			{	
				printf("numMarkers = %d items = %d\n",numMarkers,items);
			}
		}
	}
	
	free(lineBuf);
	vcfFileClose(&fp1);
	
	printf("\n");
	printf("%s:  ",argv[1]);
	printf("numSamples=%d\t",numSamples);
	printf("numMarkers=%d\n",numMarkers);
	return 0;
}
