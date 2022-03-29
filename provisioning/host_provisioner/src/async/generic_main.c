/*************************************************************************
    > File Name: generic_main.c
    > Author: Kevin
    > Created Time: 2019-04-22
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include <pthread.h>
#include "generic_main.h"
#include "app_tasks.h"

#include "utils.h"
#include "console_rx.h"
#include "conf_generic_parser.h"
#include "prov_assert.h"
#include "network_manage.h"
#include "action.h"
#include "scanner.h"
#include "async_config_client.h"
/* Defines  *********************************************************** */

/* Global Variables *************************************************** */
extern char rxBuf[MAX_BUF_NUM][CONSOLE_RX_BUF_SIZE];
extern int  rID, wID;
extern pthread_mutex_t cmdMutex;

typedef int (*tbxMainLoop)(void *);
/* Static Variables *************************************************** */
struct act {
  int action;
  char *str;
  tbxMainLoop func;
};

static struct act tbx[] = {
  { actionTBA, "Adding", addingDevicesMainLoop },
  { actionTBR, "Removing", accMainLoop },
  { actionTBBL, "Blacklisting", blacklistingDevicesMainLoop }
};

#define TBX_NUM() sizeof(tbx) / sizeof(struct act)
#define WRAP_ACT(x) (x) = ((x) + 1) % (TBX_NUM())
#define NEXT_ACT_BY_INDEX(x, n) ((x) + (n)) % (TBX_NUM())
#define ACTION_LOADING_MODE_PRT                 "   Action Sequence"
#define ACTION_LOADING_ITEM_PRT                 "      - %d> %s\n"

static char *configFP = NULL;
static provisioner_t provisioner;

/* static tbxMainLoop actionHandles[actionMAX] = { */
/* NULL, */
/* addingDevicesMainLoop, */
/* }; */

static accInitConfig_t accConfig = {
  .useDefaultStates = 1,
  .enableStateMachine = 1
};

/* Static Functions Declaractions ************************************* */
static int execCMD(void);
static int checkNextAction(void);
#if 0
static void filtersToBglibInterface(void);
#endif
static int actionMainLoops(void);

provisioner_t *getProvisioner(void)
{
  return &provisioner;
}

int getLoadedActions(void)
{
  return provisioner.runningStatus.actions;
}

void forceGenericReloadActions(void)
{
  provisioner.runningStatus.forceReloadActions = 1;
}

int getFreeModeStatus(void)
{
  return provisioner.runningStatus.freeMode;
}

void invalidProvisionerState(void)
{
  provisioner.state = not_sync_up_em;
}

int getProvState(void)
{
  return provisioner.state;
}

int getScanningStatus(void)
{
  return provisioner.runningStatus.isScanning;
}

int getProgressStatus(void)
{
  return provisioner.runningStatus.inProgress;
}

int getActionPos(void)
{
  return provisioner.runningStatus.actionPos;
}

tbaStatus_t *gettbaStatus(void)
{
  return &provisioner.tbaStatus;
}

tbblStatus_t *gettbblStatus(void)
{
  return &provisioner.tbblStatus;
}

void setFreeMode(int enable)
{
  provisioner.runningStatus.freeMode = !!enable;
}

void wrapActions(void)
{
  WRAP_ACT(provisioner.runningStatus.actionPos);
}

void printActionSeq(void)
{
  int i = 0;
  CS_MSG(ACTION_LOADING_MODE_PRT);
  for (; i < TBX_NUM(); i++) {
    CS_MSG(ACTION_LOADING_ITEM_PRT, i + 1, tbx[NEXT_ACT_BY_INDEX(getActionPos(), i)].str);
  }
}

void startOrStopProgress(int start)
{
  if (provisioner.runningStatus.inProgress == started_em && !start) {
    /* Stop  */
    provisioner.runningStatus.inProgress = stopping_em;
  } else if (provisioner.runningStatus.inProgress == not_start_em && start) {
    /* Start  */
    provisioner.runningStatus.inProgress = starting_em;
  }
  if (start) {
    reloadNetworkConfig(NULL);
    clearFailList();
    provisioner.runningStatus.forceReloadActions = 1;
    provisioner.runningStatus.freeMode = 0;
  }
}

void preForceReSyncUpHostAndTarget(void)
{
  asyncConfigClientReset();
  addingDevicesResetAll();
  /* networkManageResetAll();  */

  invalidProvisionerState();
  provisioner.runningStatus.forceReloadActions = 1;
  reloadNetworkConfig(NULL);
}

void *pGenericMain(void *pIn)
{
  int busy = 0;
  /* ret =  */
  configFP = pIn;
  genericInit();

  for (;;) {
    busy = 0;
    execCMD();
    event_dispenser();
    busy |= genericMainLoop();
    checkNextAction();
    switch (provisioner.runningStatus.inProgress) {
      case not_start_em:
        if (provisioner.runningStatus.freeMode) {
          busy |= addingDevicesMainLoop(NULL);
        }
        break;
      case starting_em:
        /* TODO - Any preparation ? */
        provisioner.runningStatus.forceReloadActions = 1;
        provisioner.runningStatus.inProgress = started_em;
        break;
      case started_em:
        /* addingDevicesMainLoop(provisioner.runningStatus.actions, &accConfig); */
        busy = actionMainLoops();
        if (busy == -1) {
          provisioner.runningStatus.inProgress = stopping_em;
        }
        break;
      case stopping_em:
        /* TODO - Update the latest states to the config files */
        provisioner.runningStatus.inProgress = not_start_em;
        break;
      default:
        hardASSERT(0);
        break;
    }
    if (!busy) {
      usleep(10 * 1000);
    }
  }
}

static int actionMainLoops(void)
{
  int busy = 0, state = actionTBC, i = 0;
  struct act *pact = NULL;
  if (!provisioner.runningStatus.actions) {
    if (!provisioner.runningStatus.freeMode) {
      if (provisioner.runningStatus.isScanning) {
        enOrDisUnprovBeaconScanning(0);
      }
      return -1;
    }
    return 0;
  }

  /* Handle Adding | Removing | Blacklisting */
  for (; i < TBX_NUM(); i++) {
    pact = &tbx[NEXT_ACT_BY_INDEX(provisioner.runningStatus.actionPos, i)];
    if (IS_ACTION_BIT_SET(provisioner.runningStatus.actions, pact->action)) {
      busy |= pact->func(NULL);
      state = pact->action;
    }
  }
  if (state == actionTBC
      && !provisioner.runningStatus.freeMode
      && provisioner.runningStatus.isScanning) {
    enOrDisUnprovBeaconScanning(0);
  }

#if 0
  if (IS_ACTION_BIT_SET(provisioner.runningStatus.actions, actionTBA)) {
    busy |= addingDevicesMainLoop(NULL);
    state = actionTBA;
  } else if (IS_ACTION_BIT_SET(provisioner.runningStatus.actions, actionTBR)) {
    state = actionTBR;
    busy |= accMainLoop(NULL);
  } else if (IS_ACTION_BIT_SET(provisioner.runningStatus.actions, actionTBBL)) {
    busy |= blacklistingDevicesMainLoop(NULL);
    state = actionTBBL;
  } else if (!provisioner.runningStatus.freeMode
             && provisioner.runningStatus.isScanning) {
    enOrDisUnprovBeaconScanning(0);
  }
#endif

  if ((IS_ACTION_BIT_SET(provisioner.runningStatus.actions, actionTBC))
      && (state != actionTBR && state != actionTBBL)) {
    busy |= accMainLoop(NULL);
  }

  return busy;
}

static int provisionDone(void *p)
{
  onDeviceDone(actionTBA, *(int *)p);
  forceGenericReloadActions();
  return 0;
}

int genericInit(void)
{
  int ret = E_SUC;
  hardASSERT(configFP);
  /* if (E_SUC != (ret = parserInit(typeXML_e))) { */
  if (E_SUC != (ret = parserInit(typeJSON_e))) {
    CS_ERR("Parser Init failed, errcode %d, Exiting.. \n", ret);
    exit(EXIT_FAILURE);
  }
  ret = loadNetworkConfig(NULL, configFP, INPUT_TMPL_FILENAME, true, false);
  CS_MSG("Load Template File            <<<%s>>> - <<<%s>>>\n",
         INPUT_TMPL_FILENAME,
         ret == E_SUC ? "SUCCESs" : "FAILED");
  CS_MSG("Load Node Configuration File  <<<%s>>> - <<<%s>>> - <errcode: %d>\n",
         configFP,
         ret == E_SUC ? "SUCCESs" : "FAILED",
         ret);
  hardASSERT(ret == E_SUC);

  memset(&provisioner, 0, sizeof(provisioner_t));
  provisioner.connHandle = 0xFF;

  ret = networkInit();
  hardASSERT(ret == E_SUC);

#if 0
  filtersToBglibInterface();
#endif
  actionInit(removeDeviceFromLDDB, isNodeIndexInDeviceDB);
  addingDevicesInit(unprovisionedBeaconFoundCB, provisionDone);
  blacklistingDevicesInit();
  provisioner.runningStatus.forceReloadActions = 0;

  accInit(&accConfig);
  return ret;
}

static int checkNextAction(void)
{
  int state = getProvState();
  if (state < configured_idle_em) {
    return 0;
  }

  if (provisioner.runningStatus.forceReloadActions) {
    dbgPrint("Force reload actions\n");
    provisioner.runningStatus.actions = getActions(1);
    provisioner.runningStatus.forceReloadActions = 0;
  } else {
    if (state == configured_idle_em) {
      provisioner.runningStatus.actions = getActions(0);
    }
  }
  return provisioner.runningStatus.actions ? 1 : 0;
}

static char **splitCmd(char *pIn, int *argc)
{
  int pos = 0;
  char **buf = (char **)malloc(MAX_CONSOLE_PARAM_NUM * sizeof(char *));
  memset(buf, 0, MAX_CONSOLE_PARAM_NUM * sizeof(char *));

  char *p = strtok(pIn, SPLIT_TOKENS);

  while (p) {
    buf[pos++] = p;
    if (pos == MAX_CONSOLE_PARAM_NUM - 1) {
      break;
    }
    p = strtok(NULL, SPLIT_TOKENS);
  }

  *argc = pos;
  return buf;
}

static int findCmdLoc(uint8_t argc, const char *argv[])
{
  int pos = 0;
  CmdItem_t *pCmds = getCommands();
  for (; pos < getConsoleCmdCount(); pos++) {
    if (!strcmp(argv[0], pCmds[pos].command)) {
      break;
    }
  }

  return pos;
}

static int execCMD(void)
{
  char locBuf[CONSOLE_RX_BUF_SIZE];
  char **argv = NULL;
  int valid = 0, argc, err = 0, ret = 0;
  CmdItem_t *pCmds = getCommands();

  if ((ret = pthread_mutex_lock(&cmdMutex)) != 0) {
    errExitEN(ret, "pthread_mutex_lock");
  }
  if (BUF_PENDING()) {
    strcpy(locBuf, rxBuf[rID]);
    /* dbgPrint("Echo CMD [%s]\n", */
    /* rxBuf[rID]); */
    ID_INCREMENT(rID);
    valid = 1;
  }
  if ((ret = pthread_mutex_unlock(&cmdMutex)) != 0) {
    errExitEN(ret, "pthread_mutex_unlock");
  }

  if (!valid) {
    return 0;
  }

  argv = splitCmd(locBuf, &argc);

  if (!argc) {
    err = E_SUC;
    goto out;
  }

  int pos = findCmdLoc(argc, (const char **)argv);

  if (pos == getConsoleCmdCount()) {
    /* CMD not supported */
    CS_ERR("CMD not supported.\n");
    outputUsage();
    err = E_NSPT;
    goto out;
  }

  ret = pCmds[pos].pCb(argc, (const char **)argv);

  if (ret) {
    CS_ERR("[[%s]] CMD Failed With Error Code [0x%04x]\n", argv[0], ret);
    err = ret;
    goto out;
  }

  CS_MSG("[[%s]] CMD Success\n", argv[0]);

  out:
  if (argv) {
    free(argv);
    argv = NULL;
  }
  return err;
}

#if 0
static void filtersToBglibInterface(void)
{
  registerFilter(gecko_evt_le_gap_adv_timeout_id, NULL);
}
#endif
