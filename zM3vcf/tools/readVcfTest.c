#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#include "vcflib.h"

#define COMMAND_LEN 256

int main(int argc,char** argv )
{
//start cmWatcher.sh
	char tmpCommand[COMMAND_LEN];
	sprintf(tmpCommand,"nohup ./cmWatcher.sh %d 10 %s.zz.txt &",getpid(),argv[1]);
	printf("tmpc=%s\n",tmpCommand);
	system(tmpCommand);
	
	
	unsigned int timeUse=0;
	struct timeval tStart,tEnd;
	gettimeofday(&tStart,NULL);
	VCF_STATUS status;
	VCF_FILE fp;
	int numRecords=1000;
	DATA_BLOCK dataBlock;
	//open vcf
	if(VCF_ERROR==vcfFileOpen(&fp,argv[1],FILE_MODE_GZ,P_DS|P_GT))
	{
		printf("Can't open vcf file:%s\n", argv[1]);
		return 1;
	}
	//read vcf head
	if(VCF_ERROR==vcfFileReadHead(&fp))
	{
		printf("Read vcf head error\n");
		return 1;
	}
	memset(&dataBlock,0,sizeof(dataBlock));
	while(1)
	{
		status=vcfFileReadDataBlock(&fp,&dataBlock,numRecords);
		if(VCF_EOF == status || VCF_READING_UNFULL==status)
		{
			break;
		}
	}
	//close vcf
	vcfFileClose(&fp);
	gettimeofday(&tEnd,NULL);
	//timeUse = (tEnd.tv_sec*1000*1000+tEnd.tv_usec)-(tStart.tv_sec*1000*1000+tStart.tv_usec);
	timeUse = (tEnd.tv_sec-tStart.tv_sec);
	printf("%s read use time =%us\n",argv[1],timeUse);
	return 0;
}
