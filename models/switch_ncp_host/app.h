/*************************************************************************
    > File Name: app.h
    > Author: Kevin
    > Created Time: 2019-01-09
    >Description:
 ************************************************************************/

#ifndef APP_H
#define APP_H

#include <stdint.h>
#include <string.h>

#ifndef DEBUG_ON
#define DEBUG_ON                                    0
#endif

#define OUTPUT_PREFIX                               "> "
#define DBG_PREFIX                                  "----- >> "
#define ERROR_PREFIX                                "<<ERROR>> : "

#define BASE_DEC                                    0
#define BASE_HEX                                    1

#if (DEBUG_ON == 1)
#define dbgPrint(__fmt, ...)                 \
  do {                                       \
    printf(DBG_PREFIX __fmt, ##__VA_ARGS__); \
    fflush(stdout);                          \
  } while (0)

#else
#define dbgPrint(...)
#endif

#define CS_OUTPUT(__fmt, ...)                   \
  do {                                          \
    printf(OUTPUT_PREFIX __fmt, ##__VA_ARGS__); \
    fflush(stdout);                             \
  } while (0)

#define CS_ERROR(__fmt, ...)                      \
  do {                                            \
    CS_OUTPUT(ERROR_PREFIX __fmt, ##__VA_ARGS__); \
  } while (0)

typedef int (*pPrmCheckAndExecCB_t)(int argc,
                                    const char *argv[]);

typedef struct CmdItem{
  char *command;
  char *p1;
  char *usage;
  char *p2;
  char *desc;
  pPrmCheckAndExecCB_t pCb;
}CmdItem_t;

int str2uint(const char *input,
             size_t length,
             uint32_t *p_ret);

int uint2str(uint64_t input,
             uint8_t base_type,
             size_t length,
             char str[]);

void *pConsoleThread(void *pIn);
void *pAppMainThread(void *pIn);
void on_message_send(uint32_t msg_len,
                     uint8_t* msg_data);
#endif
