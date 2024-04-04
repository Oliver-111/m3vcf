#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include "baseTools.h"


#define BT_LOG_FILE_NAME			"vcfLog.txt"
#define BT_LOG_START_STR			"vcflib start"
#define BT_LOG_END_STR				"vcflib end"

static FILE *g_btLogFp=NULL;
static volatile int g_btLogRC=0;


// Build a time string like "2017-12-11 12:00:00".
char* btDateAndTimeString()
{
	static char dataTimeStr[BT_MIN_STR_SIZE];
	time_t rawtime;
	struct tm *ptminfo;
	time(&rawtime);
	ptminfo = localtime(&rawtime);

	char stringyear[5], stringmonth[3], stringday[3], stringhour[3], stringmin[3], stringsec[3];
	memset(dataTimeStr,0,BT_MIN_STR_SIZE);

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

BT_STATUS popSubString(char **lineStr,char *subStr)
{
	int i,j=0;
	unsigned char find=0;
	
	if(!lineStr || '\0'==(*lineStr)[0] || !subStr)
	{
		return BT_ERROR;
	}
	
	for(i=0;'\0'!=(*lineStr)[i];i++)
	{
		if(' '==(*lineStr)[i] || '\t'==(*lineStr)[i] || '\n'==(*lineStr)[i])
		{
			if(0==find)
			{
				continue;
			}
			else
			{
				subStr[j]='\0';
				break;
			}
		}
		else
		{
			subStr[j++]=(*lineStr)[i];
			find=1;
		}	
	}
	(*lineStr)+=i+1;
	return BT_OK;
}

BT_STATUS popSubAddress(char **lineStr,char **subAddr)
{
	int i;
	unsigned char find=0;
	
	if(!lineStr || '\0'==(*lineStr)[0])
	{
		return BT_ERROR;
	}
	
	for(i=0;'\0'!=(*lineStr)[i];i++)
	{
		if(' '==(*lineStr)[i] || '\t'==(*lineStr)[i] || '\n'==(*lineStr)[i])
		{
			if(0==find)
			{
				continue;
			}
			else
			{
				(*lineStr)[i]='\0';
				break;
			}
		}
		else if(0 == find)
		{
			(*subAddr)=(*lineStr+i);
			find=1;
		}	
	}
	(*lineStr)+=i+1;
	return BT_OK;
}


int countNumSubString(char *lineStr)
{
	if(!lineStr)
	{
		return 0;
	}
	int num=0;
	char str[BT_MAX_STR_SIZE];
	
	while(1)
	{
		//memset(str,0,BT_MAX_SUB_STR_SIZE);
		if(BT_ERROR==popSubString(&lineStr,str))
		{
			break;
		}
		num++;
	}
	return num;
}

int getStrPosition(char *lineStr,char *str)
{
/*
	int i=0;
	char *p;
	while(NULL!=(p=strsep(&lineStr,":")))
	{
		i++;
		if(0==strcmp(p,str))
		{
			return i;
		}
	}
	return 0;
*/
	int i,j=0;
	int pos=0;
	char tmpStr[BT_MIN_STR_SIZE];
	for(i=0;'\0'!=lineStr[i];i++)
	{	
		if(':'==lineStr[i])
		{
			pos++;
			tmpStr[j]='\0';
			if(0==strcmp(tmpStr,str))
			{
				return pos;
			}
			j=0;
		}
		else
		{
			tmpStr[j++]=lineStr[i];
		}
	}
	pos++;
	tmpStr[j]='\0';
	if(0==strcmp(tmpStr,str))
	{
		return pos;
	}
	return 0;
}

BT_STATUS getPositionStr(char *lineStr,int pos,char *str )
{
/*	
	int i;
	char *p;
	for(i=0;i<pos;i++)
	{
		p=strsep(&lineStr,":");
	}
	return p;
*/
	int i,j=0;
	int tmpPos=0;
	for(i=0;'\0'!=lineStr[i];i++)
	{	
		if(':'==lineStr[i])
		{
			tmpPos++;
			str[j]='\0';
			if(pos==tmpPos)
			{
				return BT_OK;
			}
			j=0;
		}
		else
		{
			str[j++]=lineStr[i];
		}
	}
	tmpPos++;
	str[j]='\0';
	if(pos==tmpPos)
	{
		return BT_OK;
	}
	return BT_ERROR;
}

int getSTRNum(char *lineStr)
{
	int i=0;
	int num=0;
	for(i=0;'\0'!=lineStr[i];i++)
	{	
		if(':'==lineStr[i])
		{
			num++;
		}
	}
	num++;
	return num;
}

BT_STATUS popStrAddress(char **lineStr,char **firstAddr)
{
	int i;
	
	if(!lineStr || '\0'==(*lineStr)[0])
	{
		return BT_ERROR;
	}

	(*firstAddr)=(*lineStr);
	
	for(i=0;'\0'!=(*lineStr)[i];i++)
	{
		if(':'==(*lineStr)[i] )
		{
			(*lineStr)[i]='\0';
			break;
			
		}
	}
	(*lineStr)+=i+1;
	return BT_OK;

}

BT_AMSWER_FLAG questionUserYN(char* question)
{
	char answer[BT_MIN_STR_SIZE];
	while(1)
	{
		printf("%s",question);
		memset(answer,0,BT_MIN_STR_SIZE);
		fgets(answer,BT_MIN_STR_SIZE,stdin);
		if(0==strcmp(answer,"N\n") || 0==strcmp(answer,"n\n"))
		{
			return ANSWER_N;
		}
		else if (0==strcmp(answer,"Y\n") || 0==strcmp(answer,"y\n"))
		{
			return ANSWER_Y;
		}
		else
		{
			btStdMessage("Invalid answer,please press 'y' or 'n' key!\n");
		}
	}
}

void btMallocFreeFloatP(float **pFlt,int size,BT_MF_FLAG flag)
{
	if(BT_MALLOC==flag)
	{
		(*pFlt)=(float*)malloc(size*sizeof(float));
		if(NULL == (*pFlt))
		{
			btStdMessage("float* malloc error!\n");
			exit(1);
		}
		memset((*pFlt),0,size*sizeof(float));
	}
	else if(BT_FREE==flag)
	{
		free((*pFlt));
		(*pFlt)=NULL;
	}
	else
	{
		btStdMessage("float* malloc flag error!\n");
	}
}

void btMallocFreeCharP(char**pChr,int size,BT_MF_FLAG flag)
{
	if(BT_MALLOC==flag)
	{
		(*pChr)=(char*)malloc(size*sizeof(char));
		if(NULL == (*pChr))
		{
			btStdMessage("char* malloc error!\n");
			exit(1);
		}
		memset((*pChr),0,size*sizeof(char));
	}
	else if(BT_FREE==flag)
	{
		free((*pChr));
		(*pChr)=NULL;
	}
	else
	{
		btStdMessage("char* malloc flag error!\n");
	}
}

void btMallocFreeCharPP(char***pChr,int row,int column,BT_MF_FLAG flag)
{
	int i;
	if(BT_MALLOC==flag)
	{
		(*pChr)=(char**)malloc(row*sizeof(char*));
		if(NULL == (*pChr))
		{
			btStdMessage("char** malloc error!\n");
			exit(1);
		}
		for(i=0;i<row;i++)
		{
			(*pChr)[i]=(char*)malloc(column*sizeof(char));
			if(NULL == (*pChr)[i])
			{
				btStdMessage("*char[%d] malloc error!\n",i);
				exit(1);
			}
			memset((*pChr)[i],0,column*sizeof(char));
		}
	}
	else if(BT_FREE==flag)
	{
		for(i=0;i<row;i++)
		{
			free((*pChr)[i]);
			(*pChr)[i]=NULL;
		}
		free((*pChr));
		(*pChr)=NULL;
	}
	else
	{
		btStdMessage("char** malloc flag error!\n");
	}
}

void btAddCharPP(char ***pChr,int newRow,int newSize)
{
	(*pChr)=(char**)realloc(*pChr,newRow*sizeof(char*));
	if(NULL == (*pChr))
	{
		btStdMessage("char** realloc error!\n");
		exit(1);
	}
	(*pChr)[newRow-1]=(char*)malloc(newSize*sizeof(char));
	if(NULL == (*pChr)[newRow-1])
	{
		btStdMessage("*char[%d] add malloc error!\n",newRow-1);
		exit(1);
	}
	memset((*pChr)[newRow-1],0,newSize*sizeof(char));	
}

void btDelCharPP(char ***pChr,int newRow)
{
	(*pChr)=(char**)realloc(*pChr,newRow*sizeof(char*));
	if(NULL == (*pChr))
	{
		btStdMessage("char** realloc error!\n");
		exit(1);
	}
}

void btLogInit()
{
	if(g_btLogFp)
	{
		g_btLogRC++;
		return;
	}
	g_btLogFp=fopen(BT_LOG_FILE_NAME,"a");
	if(NULL == g_btLogFp)
	{
		btMessage("[ERROR:] create/open log file error!\n");
		exit(1);
	}
	btLogMessage("\n\n*********************  %s  ****************************\n",BT_LOG_START_STR);
	g_btLogRC++;
}

void btLogUInit()
{
	if(--g_btLogRC)
	{
		return;
	}
	btLogMessage("\n*********************  %s  ****************************\n\n",BT_LOG_END_STR);
	fclose(g_btLogFp);
	g_btLogFp=NULL;
}

void btMessage(const char *format, ...)
{
	 va_list argptr;
	 va_start(argptr, format);
	 vprintf(format, argptr);
	 va_end(argptr);
}

void btLogMessage(const char *format, ...)
{
	 va_list argptr;
	 va_start(argptr, format);	 
	 vfprintf(g_btLogFp,format,argptr);
	 fflush(g_btLogFp);
	 va_end(argptr);
}

void btStdMessage(const char *format, ...)
{
	//to log flie
	 va_list argptr;
	 va_start(argptr, format);
	 fprintf(g_btLogFp,"[%s]\t",btDateAndTimeString());
	 vfprintf(g_btLogFp,format,argptr);
	 fflush(g_btLogFp);
	 va_end(argptr);
	//to screen
	 va_start(argptr, format);
	 vprintf(format, argptr);
	 va_end(argptr);
}

