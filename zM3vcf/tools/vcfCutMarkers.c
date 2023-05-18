#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "vcflib.h"

// cut argv[3] markers from large vcf file to new vcf
#define MAX_LINE_BUF_SIZE	6*1024*1024

int main(int argc,char** argv )
{
	if(argc != 4 || atoi(argv[3]) <= 0)
	{
		printf("Usage: %s sourceVCFFile targetVCFFile markersNumber\n",argv[0]);
		return 1;
	}

	VCF_FILE fp1;
	VCF_FILE fp2;
	char *lineBuf=(char*)malloc(sizeof(char)*MAX_LINE_BUF_SIZE);
	int i;
		
	
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
	
	if(VCF_ERROR==vcfFileCreate(&fp2,argv[2],FILE_MODE_GZ))
	{
		 printf("Can't create new m3vcf file:%s\n", argv[2]);
		 return 1;
	}
	vcfFileWriteHead(&fp2,&(fp1.head));

	for(i=0;i<atoi(argv[3]);i++)
	{
		memset(lineBuf,0,MAX_LINE_BUF_SIZE);
		vcfFileReadLine(&fp1,lineBuf,MAX_LINE_BUF_SIZE);
		vcfFileWriteLine(&fp2,lineBuf);
		fprintf(stderr,"#");
	}
	fprintf(stderr,"\n");
	free(lineBuf);
	vcfFileClose(&fp1);
	vcfFileClose(&fp2);
	
	return 0;
}

