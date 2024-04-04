#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include "messageLog.h"

#define ML_MIN_STR_SIZE	32
#define ML_LOG_FILE_NAME			"zM3vcfLog.txt"
#define ML_LOG_START_STR			"zM3vcf start"
#define ML_LOG_END_STR				"zM3vcf end"


static FILE *g_mlLogFp=NULL;
static volatile int g_mlLogRC=0;


// Build a time string like "2017-12-11 12:00:00".
static char* mlDateAndTimeString()
{
	static char dataTimeStr[ML_MIN_STR_SIZE];
	time_t rawtime;
	struct tm *ptminfo;
	time(&rawtime);
	ptminfo = localtime(&rawtime);

	char stringyear[5], stringmonth[3], stringday[3], stringhour[3], stringmin[3], stringsec[3];
	memset(dataTimeStr,0,ML_MIN_STR_SIZE);

	//Format year, month, day, hour, min, sec into string.
	sprintf(stringyear, "%d", (ptminfo->tm_year + 1900));
	sprintf(stringmonth,"%02d",ptminfo->tm_mon + 1);
	sprintf(stringday,"%02d",ptminfo->tm_mday);
	sprintf(stringhour,"%02d",ptminfo->tm_hour);
	sprintf(stringmin,"%02d",ptminfo->tm_min);
	sprintf(stringsec,"%02d",ptminfo->tm_sec);
	strcpy(dataTimeStr, stringyear);
	strcat(dataTimeStr, "-");
	strcat(dataTimeStr, stringmonth);
	strcat(dataTimeStr, "-");
	strcat(dataTimeStr, stringday);
	strcat(dataTimeStr, " ");
	strcat(dataTimeStr, stringhour);
	strcat(dataTimeStr, ":");
	strcat(dataTimeStr, stringmin);
	strcat(dataTimeStr, ":");
	strcat(dataTimeStr, stringsec);
	return dataTimeStr;
}

void zLogInit()
{
	if(g_mlLogFp)
	{
		g_mlLogRC++;
		return;
	}
	g_mlLogFp=fopen(ML_LOG_FILE_NAME,"a");
	if(NULL == g_mlLogFp)
	{
		zStdoutMessage("[ERROR:] create/open %s log file error!\n",ML_LOG_FILE_NAME);
		exit(1);
	}
	zLogMessage("\n\n*********************  %s  ****************************\n",ML_LOG_START_STR);
	g_mlLogRC++;
}

void zLogUInit()
{
	if(--g_mlLogRC)
	{
		return;
	}
	zLogMessage("\n*********************  %s  ****************************\n\n",ML_LOG_END_STR);
	fclose(g_mlLogFp);
	g_mlLogFp=NULL;
}

void zStdoutMessage(const char *format, ...)
{
	va_list argptr;
	va_start(argptr, format);
	vprintf(format, argptr);
	va_end(argptr);
}

void zLogMessage(const char *format, ...)
{
	va_list argptr;
	va_start(argptr, format);	 
	vfprintf(g_mlLogFp,format,argptr);
	fflush(g_mlLogFp);
	va_end(argptr);
}

void zSLMessage(const char *format, ...)
{
	//to log flie
	va_list argptr;
	va_start(argptr, format);
	fprintf(g_mlLogFp,"[%s]\t",mlDateAndTimeString());
	vfprintf(g_mlLogFp,format,argptr);
	fflush(g_mlLogFp);
	va_end(argptr);
	//to screen
	va_start(argptr, format);
	vprintf(format, argptr);
	va_end(argptr);
}

