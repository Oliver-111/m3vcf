#include <getopt.h>
#include <unistd.h>

#include "m3vcf/m3vcf.h"


#define DEFAULT_BUFFER_SIZE		1000

static void compressUsage()
{
    fprintf(stderr, "\n");
    fprintf(stderr, " About:   Compress VCF files to M3VCF files.  \n");
    fprintf(stderr, " Usage:   zM3vcf compress [options] <A.vcf.gz> \n");
    fprintf(stderr, "\n");
    fprintf(stderr, " Options:\n");
    fprintf(stderr, "                                  (faster, but use with caution)  \n");
    fprintf(stderr, "   -b, --buffer <int>             Number of variants to compress at a time [1000]\n");
    fprintf(stderr, "   -o, --output <file>            Write output to a file [standard output]\n");
    fprintf(stderr, "   -O, --output-type <m|M>        m: compressed M3VCF, M: uncompressed M3VCF\n");
	fprintf(stderr, "   -t, --threads-limit <int>      Max number of thread (the number must bigger than 3) [%d]\n",READ_THREAD_SIZE+WRITE_THREAD_SIZE+COMPRESS_THREAD_SIZE);
	fprintf(stderr, "   -m, --memory-limit <int>       Max number of memory (unit:GB)\n");
   	fprintf(stderr, "\n");
    exit(1);
}

static void convertUsage()
{
    fprintf(stderr, "\n");
    fprintf(stderr, " About:   Convert M3VCF file to VCF file  \n");
    fprintf(stderr, " Usage:   zM3vcf convert [options] <A.m3vcf.gz> \n");
    fprintf(stderr, "\n");
    fprintf(stderr, " Options:\n");
    fprintf(stderr, "   -o, --output <file>            Write output to a file [standard output]\n");
    fprintf(stderr, "   -O, --output-type <m|M>        m: compressed VCF, M: uncompressed VCF\n");
    fprintf(stderr, "\n");
    exit(1);
}

void compressParsingArg(int argc, char *argv[],M3VCF_COMPRESS_ARGS *args)
{
	int c;
    args->m3vcfFileName = "-";
    args->m3vcfFileType = FILE_MODE_GZ;
	args->vcfFileType = FILE_MODE_NORMAL;
    args->bufferSize = DEFAULT_BUFFER_SIZE;
	args->thread_num = 0;
	args->memory_limit = 0;
    
    static struct option loptions[] =
    {
        {"buffer",required_argument,NULL,'b'},
        {"output",required_argument,NULL,'o'},
        {"output-type",required_argument,NULL,'O'},
        {"thread-num",required_argument,NULL,'t'},
        {"memory-limit",required_argument,NULL,'m'},
        {NULL,0,NULL,0}
    };

    while ((c = getopt_long(argc, argv, "b:o:O:t:m:kfS:",loptions,NULL)) >= 0)
    {
        switch (c) {
            case 'b': args->bufferSize = strtol(optarg, 0, 0); break;
            case 'o': args->m3vcfFileName = optarg; break;
            case 'O':
                switch (optarg[0]) {
                    case 'm': args->m3vcfFileType = FILE_MODE_GZ; break;
                    case 'M': args->m3vcfFileType = FILE_MODE_NORMAL; break;
                    default: fprintf(stderr,"The output type \"%s\" not recognised\n", optarg);
                };
                break;
			case 't': 
				args->thread_num = strtol(optarg, 0, 0);
				if(args->thread_num< (1+READ_THREAD_SIZE+WRITE_THREAD_SIZE))	
				{
					fprintf(stderr,"Invalid thread number (must be greater than %d): %d\n",1+READ_THREAD_SIZE+WRITE_THREAD_SIZE, args->thread_num);
					exit(1);
				}
				break;
			case 'm': 
				args->memory_limit = strtol(optarg, 0, 0); 
				if(args->memory_limit<=0)	
				{
					fprintf(stderr,"Invalid memory number : %d GB\n", args->memory_limit);
					exit(1);
				}
				break;
            case '?': compressUsage(); break;
            default: fprintf(stderr,"Unknown argument: %s\n", optarg);
        }
    }

    args->vcfFileName = NULL;
    if ( optind>=argc )
    {
        if ( !isatty(fileno((FILE *)stdin)) ) 
			args->vcfFileName = "-";  // reading from stdin
        else 
			compressUsage();
    }
    else 
	{
		args->vcfFileName = argv[optind];
    }
    if ( args->bufferSize<10 )
    {
        fprintf(stderr,"Invalid buffer (must be greater than 10): %d\n", args->bufferSize);
		exit(1);
    }

	char *ext=strrchr(args->vcfFileName,'.');
	if (ext)
	{	 
		ext++;
		if(0==strcmp("gz",ext))
		{
			args->vcfFileType= FILE_MODE_GZ;
		}
	}
}

void convertParsingArg(int argc, char *argv[],M3VCF_CONVERT_ARGS *args)
{
	int c;
    args->vcfFileName = "-";
    args->vcfFileType = FILE_MODE_GZ;
	args->m3vcfFileType = FILE_MODE_NORMAL;
    
    static struct option loptions[] =
    {
        {"output",required_argument,NULL,'o'},
        {"output-type",required_argument,NULL,'O'},
        {NULL,0,NULL,0}
    };

    while ((c = getopt_long(argc, argv, "o:O:G",loptions,NULL)) >= 0)
    {
        switch (c) {
            case 'o': args->vcfFileName = optarg; break;
            case 'O':
                switch (optarg[0]) {
                    case 'm': args->vcfFileType = FILE_MODE_GZ; break;
                    case 'M': args->vcfFileType = FILE_MODE_NORMAL; break;
                    default: fprintf(stderr,"The output type \"%s\" not recognised\n", optarg);
                };
                break;
            case '?': convertUsage(); break;
            default: fprintf(stderr,"Unknown argument: %s\n", optarg);
        }
    }

    args->m3vcfFileName = NULL;
    if ( optind>=argc )
    {
        if ( !isatty(fileno((FILE *)stdin)) ) args->m3vcfFileName = "-";  // reading from stdin
        else convertUsage();
    }
    else args->m3vcfFileName = argv[optind];
 
    char *ext=strrchr(args->m3vcfFileName,'.');
	if (ext)
	{	 
		ext++;
		if(0==strcmp("gz",ext))
		{
			args->m3vcfFileType = FILE_MODE_GZ;
		}
	}
}

int zM3vcfCompress(int argc, char *argv[])
{
	M3VCF_COMPRESS_ARGS args;
	compressParsingArg(argc,argv,&args);
	if(M3VCF_OK!=vcfCompressToM3vcf(&args))
	{
		fprintf(stderr,"vcffile:[%s] compress to m3vcffile:[%s] error!\n",args.vcfFileName,args.m3vcfFileName);
		exit(1);
	}
	return 0;
}
int zM3vcfConvert(int argc, char *argv[])
{
	M3VCF_CONVERT_ARGS args;
	convertParsingArg(argc,argv,&args);
	if(M3VCF_OK!=m3vcfConvertToVcf(&args))
	{
		fprintf(stderr,"m3vcffile:[%s] convert to vcffile:[%s] error!\n",args.m3vcfFileName,args.vcfFileName);
		exit(1);
	}
	return 0;
}

int zM3vcfConcat(int argc, char *argv[])
{
	fprintf(stderr,"concat!\n");
	return 0;
}


typedef struct
{
    int (*func)(int, char*[]);
    const char *alias, *help;
}
cmd_t;

static cmd_t cmds[] =
{
    { .func  = NULL,
      .alias = "M3VCF manipulation",
      .help  = NULL
    },

    { .func  = zM3vcfCompress, //main_vcfannotate,
      .alias = "compress",
      .help  = "compress VCF file to M3VCF",
    },
    { .func  = zM3vcfConcat, //main_vcfconcat,
      .alias = "concat",
      .help  = "concatenate M3VCF files from the same set of samples"
    },
    { .func  = zM3vcfConvert, //main_vcfconvert,
      .alias = "convert",
      .help  = "convert M3VCF files to different formats and back"
    },
    { .func  = NULL,
      .alias = NULL,
      .help  = NULL
    }

};

static void usage(FILE *fp)
{
    fprintf(fp, "\n");
    fprintf(fp, " -------------------------------------------------------------------------------- \n");
	fprintf(fp, "                  zM3vcf - A Tool for Manipulating M3VCF Files\n");
	fprintf(fp, " --------------------------------------------------------------------------------\n");
    fprintf(fp, "\n (c) 2022 \n\n");


    fprintf(fp, " Version: %s\n", ZM3VCF_VERSION);
    fprintf(fp, "\n Usage  : zM3vcf [--version|--version-only] [--help] <command> <argument>\n");
    fprintf(fp, "\n");
    fprintf(fp, " Commands:\n");

    int i = 0;
    const char *sep = NULL;
    while (cmds[i].alias)
    {
        if ( !cmds[i].func ) sep = cmds[i].alias;
        if ( sep )
        {
            fprintf(fp, "\n -- %s\n", sep);
            sep = NULL;
        }
        if ( cmds[i].func && cmds[i].help[0]!='-' ) fprintf(fp, "    %-12s %s\n", cmds[i].alias, cmds[i].help);
        i++;
    }
    fprintf(fp,"\n");
    fprintf(fp,"\n");
}

int main(int argc,char** argv)
{	
	if (argc < 2) 
	{ 
		usage(stderr); 
		return 1; 
	}
    if (strcmp(argv[1], "version") == 0 || strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) 
	{
        fprintf(stderr,"zM3vcf %s\n", ZM3VCF_VERSION);
        fprintf(stderr,"This is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n");
        return 0;
    }
    else if (strcmp(argv[1], "--version-only") == 0) 
	{
        fprintf(stderr,"%s\n", ZM3VCF_VERSION);
        return 0;
    }
    else if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) 
	{
		if (argc == 2) 
		{ 
			usage(stdout); 
			return 0; 
		}
        argv++;
        argc = 2;
    }
    	
	int i = 0;
    while (cmds[i].alias)
    {
        if (cmds[i].func && strcmp(argv[1],cmds[i].alias)==0)
        {
            return cmds[i].func(argc-1,argv+1);
        }
        i++;
    }
    fprintf(stderr, "[ERROR:] unrecognized command '%s'\n", argv[1]);
	return 1;
}

