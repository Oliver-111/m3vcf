#ifndef _M3VCF_H_
#define _M3VCF_H_

#include "compressThreads.h"

//status modes
typedef enum
{
	M3VCF_OK=0,
	M3VCF_ERROR,
	M3VCF_MAX_SIZE
}M3VCF_STATUS;

typedef struct
{
	int bufferSize;
	const char *m3vcfFileName;
	const char *vcfFileName;
	FILE_MODE vcfFileType;
	FILE_MODE m3vcfFileType;
	int thread_num;
	int memory_limit;
}M3VCF_COMPRESS_ARGS;

typedef struct
{
	const char *m3vcfFileName;
	const char *vcfFileName;
	FILE_MODE vcfFileType;
	FILE_MODE m3vcfFileType;
}M3VCF_CONVERT_ARGS;


//compress
M3VCF_STATUS vcfCompressToM3vcf(M3VCF_COMPRESS_ARGS *mcap);
//convert
M3VCF_STATUS m3vcfConvertToVcf(M3VCF_CONVERT_ARGS *mcap);

#endif
