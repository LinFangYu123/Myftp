
#ifndef _LOGC_H
#define _LOGC_H
 
#include <stdarg.h>
#include <syslog.h>
#include <unistd.h>
#include <stdio.h> 
#define	MAX_LOG_LEN	1024

/* 添加了打印语句所在的文件、行号、函数信息 */
void log_write(const char *fmt, ...);
void log_create(char *filename);
void log_close();

#endif
