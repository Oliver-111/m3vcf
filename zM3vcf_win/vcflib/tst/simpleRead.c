#include "vcflib.h"
const char *file_path="ALL.chr22.20Markers.10Samples.vcf.gz";

int main(int argc,char** argv )
{
	VCF_FILE fp_read;
	DATA_LINE dataLine={0};//Note: the first time use DATA_LINE struct variable, must clear zero
	DATA_BLOCK dataBlock={0};//Note: the first time use DATA_BLOCK struct variable, must clear zero
	if(VCF_ERROR==vcfFileOpen(&fp_read,file_path,FILE_MODE_GZ,P_DS|P_GT))	  
	{
		printf("the vcf file:%s, open error\n", file_path);
		return 1;
	}
	//read vcf head
	vcfFileReadHead(&fp_read);
	//read a data line from vcf
	vcfFileReadDataLine(&fp_read,&dataLine);
	printDataLine(&dataLine);
	//read a data block from vcf, user can give the data lines of the block
	//Note:Recommend this method read, it can be accelerated by openMP!
	vcfFileReadDataBlock(&fp_read,&dataBlock,10);//read 10 data lines to DATA_BLOCK struct
	printf("the first line of data block:\n");
	printDataLine(&(dataBlock.dataLines[0]));
	printf("the last line of data block:\n");
	printDataLine(&(dataBlock.dataLines[dataBlock.numDataLines-1]));
	//clear dataline and data block 
	clearDataLine(&dataLine);
	clearDataBlock(&dataBlock);
	//close vcf file
	vcfFileClose(&fp_read);
	return 0;
}

