/*************************************************************************
    > File Name: app_tasks.c
    > Author: Kevin
    > Created Time: 2019-04-22
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include <pthread.h>

#include "app_tasks.h"
#include "prov_assert.h"
#include "utils.h"

#include "event_dispenser.h"
#include "generic_main.h"

/* Defines  *********************************************************** */

/* Global Variables *************************************************** */

/* Static Variables *************************************************** */
pthread_t consoleRxTid, genericMainTid, tbcTid, bglibInterfaceTid;

/* Static Functions Declaractions ************************************* */

void startAppTasks(const interfaceArgs_t *connArgs)
{
  int ret;

  bglib_interface_init_sync(connArgs);

  CS_SPLIT(1);
  CS_LOG("NCP host and target sync up finished. Creating other threads.\n");
  CS_SPLIT(1);

  if (0 != (ret = pthread_create(&consoleRxTid,
                                 NULL,
                                 pConsoleMain,
                                 NULL))) {
    errExitEN(ret, "pthread_create Console");
  }

  pGenericMain(connArgs->configFilePath);

  if (0 != (ret = pthread_join(consoleRxTid, NULL))) {
    errExitEN(ret, "pthread_join");
  }

  hardASSERT(0);
}
