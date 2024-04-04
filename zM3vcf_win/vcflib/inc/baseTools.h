#ifndef _BASE_TOOLS_H_
#define _BASE_TOOLS_H_


#define BT_MAX_STR_SIZE			1024*1024*1
#define BT_MIN_STR_SIZE				32


//status modes
typedef enum
{
	BT_OK=0,
	BT_ERROR,
	BT_MAX_SIZE
}BT_STATUS;

typedef enum 
{
	ANSWER_N=0,
	ANSWER_Y
}BT_AMSWER_FLAG;

//flag :1  malloc
//flag :0  free
typedef enum 
{
	BT_FREE=0,
	BT_MALLOC
}BT_MF_FLAG;

//////////////////////////////////// tools interface///////////////////////////////////////////
// pop first string of a long string line
BT_STATUS popSubString(char **lineStr,char *subStr);
// pop first string address of a long string line
BT_STATUS popSubAddress(char **lineStr,char **subAddr);
// count the number of sub string in a long string line
int countNumSubString(char *lineStr);
// get the position of the str in lineStr with ":" delimiter
int getStrPosition(char *lineStr,char *str);
// get the str string in lineStr at pos position with ":" delimiter
BT_STATUS getPositionStr(char *lineStr,int pos,char *str );
// get the number of the string in lineStr with ":" delimiter
int getSTRNum(char *lineStr);
// pop first string address of the string in lineStr with ":" delimiter
BT_STATUS popStrAddress(char **lineStr,char **firstAddr);

// check user input answer: Y/N
BT_AMSWER_FLAG questionUserYN(char* question);
// malloc and free float* type space
void btMallocFreeFloatP(float **pFlt,int size,BT_MF_FLAG flag);
// malloc and free char* type space
void btMallocFreeCharP(char **pChr,int size,BT_MF_FLAG flag);
// malloc and free char** type space
void btMallocFreeCharPP(char ***pChr,int row,int column,BT_MF_FLAG flag);
// add a char** space and malloc a new size space for new char**
void btAddCharPP(char ***pChr,int newRow,int newSize);
// delete a char** space in new Row
void btDelCharPP(char ***pChr,int newRow);


//log file open
void btLogInit();
//log file cloes
void btLogUInit();

//write message to stdout
void btMessage(const char *format, ...);
//write message to log file without time stamp
void btLogMessage(const char *format, ...);
//write message to stdout and log file with time stamp
void btStdMessage(const char *format, ...);



#endif
