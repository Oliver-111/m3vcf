#ifndef _MESSAGE_LOG_H_
#define _MESSAGE_LOG_H_


//log file open
void zLogInit();
//log file cloes
void zLogUInit();

//write message to stdout
void zStdoutMessage(const char *format, ...);
//write message to log file without time stamp
void zLogMessage(const char *format, ...);
//write message to stdout and log file with time stamp
void zSLMessage(const char *format, ...);



#endif
