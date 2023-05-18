//#include "../config.h"
#include <sys/time.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "../htslib/hts.h"
#include "../htslib/vcf.h"
#include "../htslib/thread_pool.h"

//#include "../htslib/kstring.h"
//#include "../htslib/kseq.h"

#define COMMAND_LEN 256

int main(int argc, char **argv)
{
//start cmWatcher.sh
	char tmpCommand[COMMAND_LEN];
	sprintf(tmpCommand,"nohup ./cmWatcher.sh %d 10 %s.hts.txt &",getpid(),argv[1]);
	printf("tmpc=%s\n",tmpCommand);
	system(tmpCommand);


	unsigned int timeUse=0;
	struct timeval tStart,tEnd;
	gettimeofday(&tStart,NULL);

	htsFile *fp    = hts_open(argv[1],"r");
	if (!fp) 
	{
		printf("%s open error!\n",argv[1]);
	}
	
////////////////////////////////////thread///////////////////////////////
	htsThreadPool p = {NULL, 0};
    p.pool = hts_tpool_init(10);
    if (!p.pool) 
	{
        printf("Error creating thread pool\n");
    } 
	else 
	{
        hts_set_opt(fp, HTS_OPT_THREAD_POOL, &p);
    }
/////////////////////////////////////////////////////////////////////////    	
	bcf_hdr_t *hdr = bcf_hdr_read(fp);
	if (!hdr) 
	{
		printf("%s read head error!\n",argv[1]);
	}	
    bcf1_t *line = bcf_init();
	if (!line)
	{
		printf("init error!\n");
	}

	while(bcf_read(fp,hdr,line) == 0)
	{
		
	}

	
	bcf_destroy1(line);
    bcf_hdr_destroy(hdr);
	int ret;
    if ((ret = hts_close(fp)))
    {
        printf("close error\n");
    }
////////////////////////////////////thread///////////////////////////////
	if (p.pool)
	{
        hts_tpool_destroy(p.pool);
	}
/////////////////////////////////////////////////////////////////////////    	
    gettimeofday(&tEnd,NULL);
	//timeUse = (tEnd.tv_sec*1000*1000+tEnd.tv_usec)-(tStart.tv_sec*1000*1000+tStart.tv_usec);
	timeUse = (tEnd.tv_sec-tStart.tv_sec);
	printf("%s read use time =%us\n",argv[1],timeUse);
}
