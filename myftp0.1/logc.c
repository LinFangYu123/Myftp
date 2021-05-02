#include <time.h>
#include "logc.h"
#include <stdlib.h>

FILE *pFile;

static void get_current_time(char *buf, int len)
{
    time_t timep;
    struct tm *p;
    
    timep = time(NULL);
    p = localtime(&timep);
    snprintf(buf, len - 1,\
        "%d/%d/%d %d:%d:%d", \
        (1900 + p->tm_year), \
        (1 + p->tm_mon), \
        p->tm_mday, \
        p->tm_hour, \
        p->tm_min, \
        p->tm_sec);
}

void log_create(char *filename){
    pFile = fopen(filename, "a+");
    if(pFile == NULL){
        perror("fopen");
        exit(1);
    }
}

static void vlog_write(const char* fmt, va_list args)
{
    char buf[MAX_LOG_LEN+1] = {0};
    char time[128] = {0};
    get_current_time(time, sizeof(time) - 1);
    vsnprintf(buf, sizeof(buf), fmt, args);
    fprintf(pFile, "[%s] %s", time, buf);
    fflush(pFile);
}
 

void log_write(const char *fmt, ...)
{
    va_list args;
 
    va_start(args, fmt);
    vlog_write(fmt, args);
    va_end(args);
}

void log_close(){
    if(pFile!=NULL){
        fclose(pFile);
        pFile = NULL;
    }
}
