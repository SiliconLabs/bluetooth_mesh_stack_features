/*************************************************************************
    > File Name: logging.c
    > Author: Kevin
    > Created Time: 2019-01-16
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "logging.h"
#include "err_code.h"
#include "prov_assert.h"
/* Defines  *********************************************************** */
#define TIME_FMT                        "[%04d-%02d-%02d %02d:%02d:%02d]"
#define MAX_BUF_SIZE                    512
#define LOG_FILE_NAME                   (SRC_ROOT_DIR "log/prov.log")
#define PROV_LOG_PREFIX                      "PROV LOG >> "
#define logPrint(__fmt, ...)                      \
  do {                                            \
    printf(PROV_LOG_PREFIX __fmt, ##__VA_ARGS__); \
    fflush(stdout);                               \
  } while (0)

#define logASSERT(x)                                                                   \
  do {                                                                                 \
    if ((x) == 0) { logPrint("ASSERT ERROR, %s:%d\n", __FILE__, (uint32_t)__LINE__); } \
  } while (0)

/* Static Variables *************************************************** */
static FILE *fp = NULL;

static const char tooLongData[] = ">>>Data is to long, may missing data from previous item<<<\n";

/* Static Functions Declaractions ************************************* */

/* Global Variables *************************************************** */

int initLog(void)
{
  fp = fopen(LOG_FILE_NAME, "a+");
  if (fp == NULL) {
    logASSERT(0);
    return E_IO;
  }
  setlinebuf(fp);
  return E_SUC;
}

void deinitLog(void)
{
  if (fp) {
    fclose(fp);
    fp = NULL;
  }
}

static int getTimeNow(char *pO, size_t inputLen, size_t *len)
{
  time_t t;
  struct tm p;

  time(&t);
  localtime_r(&t, &p);

  *len = snprintf(pO, inputLen, TIME_FMT, p.tm_year + 1900, p.tm_mon + 1,
                  p.tm_mday, p.tm_hour, p.tm_min, p.tm_sec);
  if (*len > inputLen) {
    return E_INVP;
  }
  return E_SUC;
}

void logToFile(const char *__fmt, ...)
{
  char buf[MAX_BUF_SIZE];
  size_t offs = 0;
  va_list argList;
  if (fp == NULL) {
    if (initLog()) {
      return;
    }
  }

  memset(buf, 0, MAX_BUF_SIZE);

  softASSERT(0 == getTimeNow(buf,
                             MAX_BUF_SIZE,
                             &offs));

  va_start(argList, __fmt);
  size_t len = vsnprintf(buf + offs,
                         MAX_BUF_SIZE - offs,
                         __fmt,
                         argList);
  va_end(argList);

  softASSERT(0 == logData(buf,
                          len + offs));
  if (len + offs >= MAX_BUF_SIZE) {
    logData(tooLongData, sizeof(tooLongData));
  }
}

int logData(const void *data, size_t len)
{
  int ret;
  if (len == 0) {
    return E_SUC;
  }

  if (data == NULL) {
    return E_INVP;
  }

  if (fp == NULL) {
    if ((ret = initLog()) != 0) {
      return ret;
    }
  }

  int count = fwrite((const char *)data, len, 1, fp);
  if (count != 1) {
    return E_IO;
  }

  fflush(fp);
  return E_SUC;
}

int clearLog(void)
{
  if (fp) {
    fclose(fp);
    fp = NULL;
  }

  fp = fopen(LOG_FILE_NAME, "w+");

  if (fp == NULL) {
    logASSERT(0);
    return E_IO;
  }

  return initLog();
}
