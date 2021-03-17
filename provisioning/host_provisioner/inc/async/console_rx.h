/*************************************************************************
    > File Name: console_rx.h
    > Author: Kevin
    > Created Time: 2019-04-22
    >Description:
 ************************************************************************/

#ifndef CONSOLE_RX_H
#define CONSOLE_RX_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>

#define SPLIT_TOKENS                            "\t\r\n\a "
#define MAX_BUF_NUM                             10
#define CONSOLE_RX_BUF_SIZE                     32
#define MAX_CONSOLE_PARAM_NUM                   6

#define BUF_PENDING()                           ((rID) == (wID) ? 0 : 1)
#define BUF_FULL()                              ((rID) == (((wID) + 1) % MAX_BUF_NUM) ? 1 : 0)
#define ID_INCREMENT(x)            \
  do {                             \
    (x) = ((x) + 1) % MAX_BUF_NUM; \
  } while (0)
typedef int (*pPrmCheckAndExecCB_t)(int argc, const char *argv[]);

typedef struct CmdItem{
  char *command;
  char *p1;
  char *usage;
  char *p2;
  char *desc;
  pPrmCheckAndExecCB_t pCb;
}CmdItem_t;

CmdItem_t *getCommands(void);
size_t getConsoleCmdCount(void);

void outputUsage(void);

int factoryResetExecCB(int argc, const char *argv[]);
int recExec(int argc, const char *argv[]);
int getOptExec(int argc, const char *argv[]);
int setOptExec(int argc, const char *argv[]);
int goExec(int argc, const char *argv[]);
int listExec(int argc, const char *argv[]);
int swActionExec(int argc, const char *argv[]);
int passExec(int argc, const char *argv[]);
int usageExec(int argc, const char *argv[]);
int exitExec(int argc, const char *argv[]);
#ifdef __cplusplus
}
#endif
#endif //CONSOLE_RX_H
