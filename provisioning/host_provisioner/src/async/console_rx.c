/*************************************************************************
    > File Name: console.c
    > Author: Kevin
    > Created Time: 2019-04-01
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "console_rx.h"
#include "utils.h"
#include "err_code.h"
/* Defines  *********************************************************** */

/* Static Variables *************************************************** */

const CmdItem_t CMDs[] = {
#if (SWITCH_LEGACY == 1)
  { "l",
    "\t\t",
    "l [1/0]",
    "\t\t",
    "Set light on[1] or off[0]",
    &onOffExec },

  { "ln",
    "\t\t",
    "ln [0-100]",
    "\t",
    "Set lightness [0% - 100%]",
    &lightnessExec },

  { "ct",
    "\t\t",
    "ct [0-100]",
    "\t",
    "Set Color Temperature [0% - 100%]",
    &ctlExec },
#endif
  { "fr",
    "\t\t",
    "fr [1/0]",
    "\t\t",
    "1 - Factory Reset, 0 - normal reset, default is normal reset",
    &factoryResetExecCB },

  { "h",
    "\t\t",
    "h",
    "\t\t\t",
    "help - Print usage",
    &usageExec },

  { "g",
    "\t\t",
    "g [1/0]",
    "\t\t\t",
    "Start[1] or Stop[0] adding/blacklisting devices, default to start",
    &goExec },

  { "p",
    "\t\t",
    "p n",
    "\t\t\t",
    "Pass the #n node configuration",
    &passExec },

  { "s",
    "\t\t",
    "s",
    "\t\t\t",
    "Switch the action (add or blacklist devices)",
    &swActionExec },

  { "list",
    "\t\t",
    "list",
    "\t\t\t",
    "List the devices in the DDB",
    &listExec },

  { "set",
    "\t\t",
    "set option [value]",
    "\t",
    "Set the option to value\n"
    "                fm [1/0] - Enable/disable free mode, in free mode, the provisioner\n"
    "                           will scan for nearby unprovisioned device beacons and\n"
    "                           record them to the target XML file with tmpl field 0xff\n"
    "                           Default - enable\n"
    "                sc [1/0] - Enable/disable scanning for unprovisioned device beacons\n"
    "                           Default - enable\n"
    "                lm [1/0] - Set the action loading mode, when set to 0 (loadModeSingle),\n"
    "                           the loading action engine will only load the current set\n"
    "                           action. If set to 1 (loadModeCircularly), it will load \n"
    "                           all the actions circularly.\n"
    "                           E.g. when current action is adding device, and type 'g' \n"
    "                           0 - add devices if any, do nothing if no device to add.\n"
    "                           1 - add devices if any, then configure/remove/blacklist \n"
    "                               devices if any"
    ,
    &setOptExec },

  { "get",
    "\t\t",
    "get option",
    "\t\t",
    "Get the option value\n"
    "              - If not parameter followed 'get', get all configurations\n"
    "                fm       - Get free mode, 1 - Enabled, 0 - Disabled\n"
    "                ca       - Get current action\n"
    "                pr       - Get device state (CMD in progress or not)\n"
    "                fp       - Get the node configuration file path\n"
    "                sc       - Get scanning state\n"
    "                lm       - Get the action loading mode"
    ,
    &getOptExec },

  { "exit",
    "\t\t",
    "exit",
    "\t\t",
    "Exit program",
    &exitExec },
};

#define CMD_NUM()                                     (sizeof(CMDs) / sizeof(CmdItem_t))
/* Global Variables *************************************************** */
/* extern char rxBuf[MAX_BUF_NUM][CONSOLE_RX_BUF_SIZE]; */
/* extern int  rID, wID; */
/* extern pthread_mutex_t cmdMutex; */

char rxBuf[MAX_BUF_NUM][CONSOLE_RX_BUF_SIZE];
int  rID, wID;
pthread_mutex_t cmdMutex = PTHREAD_MUTEX_INITIALIZER;

/* Static Functions Declaractions ************************************* */
static int getConsoleCmd(void);
static void consoleInit(void);

void *pConsoleMain(void *pIn)
{
  consoleInit();
  for (;;) {
    printf("$ ");
    fflush(stdout);
    getConsoleCmd();
    usleep(100 * 1000);
  }
  return NULL;
}

CmdItem_t *getCommands(void)
{
  return (CmdItem_t *)CMDs;
}

size_t getConsoleCmdCount(void)
{
  return CMD_NUM();
}
static void consoleInit(void)
{
  int ret = 0;
  if ((ret = pthread_mutex_lock(&cmdMutex)) != 0) {
    errExitEN(ret, "pthread_mutex_lock");
  }

  memset(rxBuf, 0, CONSOLE_RX_BUF_SIZE * MAX_BUF_NUM);
  rID = 0;
  wID = 0;

  if ((ret = pthread_mutex_unlock(&cmdMutex)) != 0) {
    errExitEN(ret, "pthread_mutex_unlock");
  }
}

static int getConsoleCmd(void)
{
  char c, locBuf[CONSOLE_RX_BUF_SIZE];
  int err = 0, wOffset = 0, ret = 0;

  memset(locBuf, 0, CONSOLE_RX_BUF_SIZE);
  while ((c = getchar()) != '\n') {
    if (!err) {
      locBuf[wOffset++] = c;
    }

    if ((wOffset == CONSOLE_RX_BUF_SIZE) && !err) {
      /* CMD too long */
      CS_ERR("CMD too long\n");
      err = E_CMDTL;
    }
  }

  if (!wOffset) {
    return E_SUC;
  }

  if (!err) {
    if ((ret = pthread_mutex_lock(&cmdMutex)) != 0) {
      errExitEN(ret, "pthread_mutex_lock");
    }
    if (BUF_FULL()) {
      /* CMD buffer full */
      CS_ERR("CMD buffer full\n");
      err = E_BFFL;
    } else {
      /* Buffer not full, copy the local content to it */
      locBuf[wOffset] = '\0';
      strcpy(rxBuf[wID], locBuf);
      ID_INCREMENT(wID);
    }
    if ((ret = pthread_mutex_unlock(&cmdMutex)) != 0) {
      errExitEN(ret, "pthread_mutex_unlock");
    }
  }

  return err;
}
