/*************************************************************************
    > File Name: console_callbacks.c
    > Author: Kevin
    > Created Time: 2019-04-22
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include <stdio.h>
#include <stdlib.h>

#include "console_rx.h"

#include "utils.h"
#include "err_code.h"
#include "generic_main.h"
#include "conf_generic_parser.h"
#include "prov_assert.h"
#include "async_config_client.h"
#include "event_dispenser.h"

/* Defines  *********************************************************** */
#define FREE_MODE_PRT                           "   Free Mode             - %s\n"
#define CURRENT_ACTION_PRT                      "   Current Action        - %s\n"
#define SCANNING_STATE_PRT                      "   Scanning State        - %s\n"
#define PROGRESS_STATE_PRT                      "   Progress State        - %s\n"
#define NC_FILEPATH_STATE_PRT                   "   Config File Path      - %s\n"

#define CONFIG_PRINT_PREFIX  "Config file (%d) devices status:\n"
#define CONFIG_DEV_PADDING   "         "
#define CONFIG_DEV_STATUS    "Node[%03d]: err=0x%08x, done=0x%02x, blacklist=0x%02x\n"
/* Global Variables *************************************************** */
extern const char *stateNames[];

/* Static Variables *************************************************** */

/* Static Functions Declaractions ************************************* */

int factoryResetExecCB(int argc, const char *argv[])
{
  /* Check if parameter(s) valid */
  int reset = 0;

  if (argc >= 2) {
    reset = argv[1][0] - '0';
  }

  if (!getSyncUpStatus()) {
    CS_ERR("NCP host-target is not synchronized yet.\n");
    return E_NOTRD;
  }

  CS_MSG("%s Resetting...\n", reset ? "Factory" : "Normal");

  if (reset) {
    factoryReset();
  } else {
    normalReset();
  }

  return E_SUC;
}

static void printFreeMode(void)
{
  CS_MSG(FREE_MODE_PRT, getFreeModeStatus() ? "Enabled" : "Disabled");
}

static void printCurAction(void)
{
  char *p = NULL;

  switch (getProvState()) {
    case adding_devices_em:
      p = "Adding";
      break;
    case blacklisting_devices_em:
      p = "Blacklisting";
      break;
    case configuring_devices_em:
      p = "Configuring";
      break;
    case removing_devices_em:
      p = "Removing";
      break;
    default:
      p = "Idle";
      break;
  }

  CS_MSG(CURRENT_ACTION_PRT, p);
}

static void printScanningState(void)
{
  CS_MSG(SCANNING_STATE_PRT, getScanningStatus() ? "Enabled" : "Disabled");
}

static void printInProgressState(void)
{
  char *p = NULL;
  int inProgress = getProgressStatus();
  if (inProgress == not_start_em) {
    p = "Not In Progress";
  } else if (inProgress == started_em) {
    p = "In Progress";
  } else if (inProgress == starting_em) {
    p = "Starting Progress";
  } else if (inProgress == stopping_em) {
    p = "Stopping Progress";
  } else {
    p = "Error";
  }
  CS_MSG(PROGRESS_STATE_PRT, p);
}

static void printFilePath(void)
{
  char *p = get_config_file_path();
  CS_MSG(NC_FILEPATH_STATE_PRT, p ? p : "ERROR");
}

static void printActionLoadingMode(void)
{
  printActionSeq();
}

static void printAllConf(void)
{
  printFreeMode();
  printCurAction();
  printScanningState();
  printInProgressState();
  printFilePath();
  printActionLoadingMode();
}

int getOptExec(int argc, const char *argv[])
{
  int ret = E_SUC;

  if (argc > 2) {
    return E_INVP;
  }

  if (argc == 1) {
    printAllConf();
  } else {
    if (!strcmp(argv[1], "fm")) {
      printFreeMode();
    } else if (!strcmp(argv[1], "ca")) {
      printCurAction();
    } else if (!strcmp(argv[1], "sc")) {
      printScanningState();
    } else if (!strcmp(argv[1], "pr")) {
      printInProgressState();
    } else if (!strcmp(argv[1], "fp")) {
      printFilePath();
    } else if (!strcmp(argv[1], "lm")) {
      printActionLoadingMode();
    } else {
      ret = E_NSPT;
    }
  }
  return ret;
}

int setOptExec(int argc, const char *argv[])
{
  int ret = E_SUC, val = -1;
  (void)val;
#if 1
  if (argc < 2) {
    return E_INVP;
  }

  if (!strcmp(argv[1], "fm")) {
    if (argc == 2) {
      val = 1;
    } else {
      val = atoi(argv[2]);
    }
    if (getProgressStatus() != not_start_em) {
      return E_BUSY;
    }
    /* if (val != 0 && val != 1) { */
    /* return E_INVP; */
    /* } */
    setFreeMode(val);
    enOrDisUnprovBeaconScanning(!!val);
  } else if (!strcmp(argv[1], "sc")) {
    if (argc == 2) {
      val = 1;
    } else {
      val = atoi(argv[2]);
    }
    /* if (val != 0 && val != 1) { */
    /* return E_INVP; */
    /* } */
    enOrDisUnprovBeaconScanning(!!val);
  }
  /*
   * else if (!strcmp(argv[1], "lm")) {
   *   if (argc == 2) {
   *     val = 1;
   *   } else {
   *     val = atoi(argv[2]);
   *   }
   *   if (val != 0 && val != 1) {
   *     return E_INVP;
   *   }
   *   loadActionMode = val;
   *   char v = val + '0';
   *   saveKeyValue(KEY_ACTION_LOADING_MODE, &v);
   * }
   */
  else {
    CS_ERR("Unsupported CMD\n");
    ret = E_NSPT;
  }
#endif
  return ret;
}

int goExec(int argc, const char *argv[])
{
  int go = 1;

  /* Check if parameter(s) valid */
  if (argc > 2) {
    return E_INVP;
  } else if (argc == 2) {
    go = argv[1][0] - '0';
    if ((go != 1) && (go != 0)) {
      outputUsage();
      return E_INVP;
    }
  }

  if (!getSyncUpStatus()) {
    CS_ERR("NCP host-target is not synchronized yet.\n");
    return E_NOTRD;
  }

  startOrStopProgress(go);
  CS_MSG("%s Operations...\n", go == 1 ? "Start" : "Stop");
  return E_SUC;
}

int listExec(int argc, const char *argv[])
{
  int i = 0, a = 0;
  networkConfig_t *pconfig = NULL;
  getNetworkConfig((void **)&pconfig);
  hardASSERT(pconfig);
  struct gecko_msg_mesh_prov_ddb_list_devices_rsp_t *rsp;

  if (getLoadedActions()
      && getProgressStatus() == started_em) {
    CS_MSG("Device is Busy, Please Try Again Later.\n");
  } else {
    PRINTF_SPLIT(2);
    printf(CONFIG_PRINT_PREFIX, pconfig->nodeCnt);
    for (i = 0; i < pconfig->nodeCnt; i++) {
      printf(CONFIG_DEV_PADDING);
      printf(CONFIG_DEV_STATUS,
             i,
             pconfig->pNodes[i].errBits,
             pconfig->pNodes[i].done,
             pconfig->pNodes[i].blacklist);
      if (pconfig->pNodes[i].errBits) {
        for (a = provisioning_em; a < end_em; a++) {
          if ((pconfig->pNodes[i].errBits & (1 << a))) {
            dbgPrint("Last failed state: %s\n", stateNames[a]);
            break;
          }
        }
      }
    }
    PRINTF_SPLIT(2);
    rsp = gecko_cmd_mesh_prov_ddb_list_devices();
    if (bg_err_success != rsp->result) {
      CS_ERR("Error happened when retrieve the DDB, error code <<0x%04x>>\n",
             rsp->result);
      return E_BGAPI;
    } else {
      printf("%d nodes in the network.\n", rsp->count);
    }
  }
  return E_SUC;
}

int swActionExec(int argc, const char *argv[])
{
  wrapActions();
  return E_SUC;
}

int passExec(int argc, const char *argv[])
{
  /* TODO - Modify the usage to "p n", in which n is the node index, the async
   * needs to provide a variable and check it */
  return E_NIMPL;
}

int usageExec(int argc, const char *argv[])
{
  outputUsage();
  return E_SUC;
}

int exitExec(int argc, const char *argv[])
{
  CS_MSG("Exit program\n");
  exit(EXIT_SUCCESS);
}

void outputUsage(void)
{
  CmdItem_t *CMDs = getCommands();
  CS_SPLIT(1);
  CS_MSG("Command\tUsage\t\t\tDescription\n");
  for (int i = 0; i < getConsoleCmdCount(); i++) {
    CS_MSG("%s%s%s%s%s\n",
           CMDs[i].command,
           CMDs[i].p1,
           CMDs[i].usage,
           CMDs[i].p2,
           CMDs[i].desc);
  }
  CS_SPLIT(1);
}
