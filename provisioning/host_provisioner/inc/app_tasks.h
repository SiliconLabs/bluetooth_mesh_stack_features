/*************************************************************************
    > File Name: app_tasks.h
    > Author: Kevin
    > Created Time: 2019-04-22
    >Description:
 ************************************************************************/

#ifndef APP_TASKS_H
#define APP_TASKS_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "event_dispenser.h"

void *pConsoleMain(void *pIn);
void startAppTasks(const interfaceArgs_t *connArgs);
#ifdef __cplusplus
}
#endif
#endif //APP_TASKS_H
