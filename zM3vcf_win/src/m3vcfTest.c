#include "m3vcf/m3vcf.h"

int default_block_size = 1000;
const char *vcf_file_path = "testFile/ALL.chr22.10000Markers.vcf.gz";
const char *m3vcf_flie_path = "testFile/ALL.chr22.10000Markers.m3vcf.gz";
const char *if_vcf_file_path = "testFile/ALL.chr22.10000Markers.IF.vcf.gz";

int main(int argc,char** argv)
{
	//vcf to m3vcf
	M3VCF_COMPRESS_ARGS com_args;
	com_args.vcfFileName = vcf_file_path;
	com_args.vcfFileType = FILE_MODE_GZ; 
	com_args.m3vcfFileName = m3vcf_flie_path; 
	com_args.m3vcfFileType = FILE_MODE_GZ; 
	com_args.bufferSize = default_block_size;
	com_args.thread_num=0; 
	com_args.memory_limit=0;
	if(M3VCF_OK!=vcfCompressToM3vcf(&com_args))
	{
		fprintf(stderr,"vcffile:[%s] compress to m3vcffile:[%s] error!\n",com_args.vcfFileName,com_args.m3vcfFileName);
		return 1;
	}
	//m3vcf to vcf
	M3VCF_CONVERT_ARGS con_args;
	con_args.m3vcfFileName = m3vcf_flie_path; 
	con_args.m3vcfFileType = FILE_MODE_GZ; 
	con_args.vcfFileName = if_vcf_file_path; 
	con_args.vcfFileType = FILE_MODE_GZ; 
	if(M3VCF_OK!=m3vcfConvertToVcf(&con_args))
	{
		fprintf(stderr,"m3vcffile:[%s] convert to vcffile:[%s] error!\n",con_args.m3vcfFileName,con_args.vcfFileName);
		return 1;
	}
	return 0;
}

