/*************************************************************************
    > File Name: logging.h
    > Author: Kevin
    > Created Time: 2019-03-06
    >Description:
 ************************************************************************/

#ifndef LOGGING_H
#define LOGGING_H
#include <stdio.h>

int initLog(void);
void deinitLog(void);
int logData(const void *data, size_t len);
void logToFile(const char *__fmt, ...);
int clearLog(void);
#endif
