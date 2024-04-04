#ifndef _VCF_LIB_H_
#define _VCF_LIB_H_

#include <stdio.h>
#include <zlib.h>


#define P_GT	0x00000000
#define P_DS	0x00000002


//#define MAX_SUPPORT_SAMPLE_NUM 	1024*1024*1
#define BT_MAX_LINE_SIZE 		1024*1024*6

#define OPEN_MP_THREAD_NUM 		10


//status modes
typedef enum
{
	VCF_OK=0,
	VCF_ERROR,
	VCF_WITHOUT_GT,
	//VCF_WITHOUT_ITEM,
	VCF_READING_UNFULL,
	VCF_EOF,
	VCF_MAX_SIZE
}VCF_STATUS;

//file format type
typedef enum 
{
	FILE_MODE_NORMAL=0,
	FILE_MODE_GZ,
	FILE_MODE_MAX_SIZE
}FILE_MODE;

//vcf file head part structrue
typedef struct
{
	char **metaInfoLines;
	int numMetaInfoLines;
	char *headerLine;
	//int numSamples;
}FILE_HEAD;

//data line infomation part structrue
typedef struct
{
	char *chrom;
    char *pos;
    char *ID;
    char *ref;
    char *alt;
    char *qual;
    char *filter;
    char *info;
	char *format;
}DATA_INFO;

//data line structrue
typedef struct
{
	char *rawDataLine;
	DATA_INFO dataInfo;
	char *samplesRawString;
	char *gtData;
	float *dsData;
	int numSamples;
}DATA_LINE;

//data block structrue
typedef struct
{
	DATA_LINE *dataLines;
	int numDataLines;
}DATA_BLOCK;

typedef struct
{
	char *format;
	char **dataStr;
}DATA_FORMAT_STR;

//all format data line structrue
typedef struct
{
	char *rawDataLine;
	DATA_INFO dataInfo;
	int numFormats;
	DATA_FORMAT_STR *dataFormatStr;
	int numSamples;
}DATA_LINE_ALL_FORMAT;

//all format data block structrue
typedef struct
{
	DATA_LINE_ALL_FORMAT *dataLines;
	int numDataLines;
}DATA_BLOCK_ALL_FORMAT;


//file_handle
typedef struct
{
	union
	{
		FILE * fp;
		gzFile gfp;
	}fp;
	FILE_MODE mode;
	FILE_HEAD head;
	int numSamples;
	unsigned int parsingItems; 	
}VCF_FILE;


//////////////////////////////open and close file interface////////////////////////////////////
//open a exist vcf file for read
VCF_STATUS vcfFileOpen(VCF_FILE *fp,const char *fileName,FILE_MODE fileMode,unsigned int parseItem);
//create a new vcf file for write, or open a exist vcf file and clear the content for write
VCF_STATUS vcfFileCreate(VCF_FILE *fp,const char *fileName,FILE_MODE fileMode);
//create a new vcf file for write, or open a exist vcf file for append the content
VCF_STATUS vcfFileAppend(VCF_FILE *fp,const char *fileName,FILE_MODE fileMode);
//close a opened vcf file
VCF_STATUS vcfFileClose(VCF_FILE *fp);

//////////////////////////////////read file interface/////////////////////////////////////////
//read a string line from a vcf file
VCF_STATUS vcfFileReadLine(VCF_FILE *fp,char *lineStr,int lineSize);
//read a vcf file head part in FILE_HEAD structure
VCF_STATUS vcfFileReadHead(VCF_FILE *fp);
//get the meta-informaiton lines number in the vcf file, from the vcf header
VCF_STATUS getNumMetaInfoLines(FILE_HEAD *fhp,int *NumMetaInfoLines);
//get the samples' number in the vcf file, from the vcf header
VCF_STATUS getNumSamples(VCF_FILE *fp,int *NumSamples);
//parse a data line string to DATA_INFO structure, and return rest string of data line
char* vcfFileParseDataLineInfo(char *lineStr,DATA_INFO *dataInfo);
//parse a data line string to DATA_LINE structure
VCF_STATUS vcfFileParseDataLine(VCF_FILE *fp,char *lineStr,DATA_LINE *dlp);
//read a data line to DATA_LINE structure
VCF_STATUS vcfFileReadDataLine(VCF_FILE *fp,DATA_LINE *dlp);
//read numLines data lines to DATA_BLOCK structure(multilthreads)
VCF_STATUS vcfFileReadDataBlock(VCF_FILE *fp,DATA_BLOCK *dbp,int numLines);
//read numLines data lines with overlap 1 line mode(for m3vcf require), to DATA_BLOCK structure(multilthreads)
VCF_STATUS vcfFileReadDataBlockOverlap1Line(VCF_FILE *fp,DATA_BLOCK *dbp,int numLines);

//////////////////////////////////write file interface/////////////////////////////////////////
//write a string line to a vcf file
VCF_STATUS vcfFileWriteLine(VCF_FILE *fp,char *lineStr);
//write a vcf file head part from FILE_HEAD structure
VCF_STATUS vcfFileWriteHead(VCF_FILE *fp,FILE_HEAD *fhp);
//add a meta-informaiton line to FILE_HEAD structure at posIndex postion
VCF_STATUS vcfFileAddMetaInfoLine(FILE_HEAD *fhp,int posIndex,char *MetaInfoLine);
//move a meta-information line to FILE_HEAD structure at posIndex postion
VCF_STATUS vcfFileRemoveMetaInfoLine(FILE_HEAD *fhp,int posIndex);


//////////////////////////////////other interface/////////////////////////////////////////
void clearFileHead(FILE_HEAD *fhp);
void clearDataLine(DATA_LINE *dlp);
void clearDataBlock(DATA_BLOCK *dbp);
VCF_STATUS vcfPopSubString(char **lineStr,char *subStr);
void printDataLine(DATA_LINE *dlp);

//////////////////////////parse all format Data interface////////////////////////////////
//parse a data line string to all format DATA_LINE structure
VCF_STATUS vcfFileParseDataLine_allFormat(VCF_FILE *fp,char *lineStr,DATA_LINE_ALL_FORMAT *dlafp);
//read a data line to DATA_LINE structure
VCF_STATUS vcfFileReadDataLine_allFormat(VCF_FILE *fp,DATA_LINE_ALL_FORMAT *dlafp);
//read numLines data lines to DATA_BLOCK structure(multilthreads)
VCF_STATUS vcfFileReadDataBlock_allFormat(VCF_FILE *fp,DATA_BLOCK_ALL_FORMAT *dbafp,int numLines);

void clearDataLine_allFormat(DATA_LINE_ALL_FORMAT *dlafp);
void clearDataBlock_allFormat(DATA_BLOCK_ALL_FORMAT *dbafp);
void printDataLine_allFormat(DATA_LINE_ALL_FORMAT *dlafp);



#endif 
