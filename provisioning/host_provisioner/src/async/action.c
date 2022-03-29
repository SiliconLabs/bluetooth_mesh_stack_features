/*************************************************************************
    > File Name: /home/zhfu/work/projs/provisioner/src/async/action.c
    > Author: Kevin
    > Created Time: 2019-04-23
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include "action.h"

#include "network_manage.h"
#include "conf_generic_parser.h"
#include "prov_assert.h"
#include "utils.h"

#include <pthread.h>
/* Defines  *********************************************************** */
#ifndef ACT_DEBUG
#define ACT_DEBUG                               0
#endif
#if (ACT_DEBUG == 1)
#define actDEBUG(__fmt__, ...)                  dbgPrint(__fmt__, ##__VA_ARGS__)
#else
#define actDEBUG(__fmt__, ...)
#endif

#define PROVISIONED_BIT_MASK                    0x01
#define CONFIGURED_BIT_MASK                     0x10

#define BLACKLIST_BIT_MASK                      0x01
#define REMOVE_BIT_MASK                         0x10

/* Global Variables *************************************************** */

/* Static Variables *************************************************** */
static actionCache_t cache;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static funcRemoveDeviceFromDDB fnRmDev = NULL;
static funcIsDeviceInDDB fnIsDevInDDB = NULL;

#if 0
/* TODO - still needed ? */
static const char *actionSt[] = {
  "None",
  "Add",
  "Remove",
  "Blacklist",
  "Re-config",
  "MAX - invalid"
};
#endif

/* Static Functions Declaractions ************************************* */
static bool nodeNeedTBA(int i, const networkConfig_t *pconfig);
static bool nodeNeedTBR(int i, const networkConfig_t *pconfig);
static bool nodeNeedTBBL(int i, const networkConfig_t *pconfig);
static bool nodeNeedTBC(int i, const networkConfig_t *pconfig);

static int loadList(nodeAction_t which, const networkConfig_t *pconfig);
static void loadActions(void);

void actionInit(funcRemoveDeviceFromDDB fn1,
                funcIsDeviceInDDB fn2)
{
  hardASSERT(fn2);
  memset(&cache, 0, sizeof(actionCache_t));
  fnRmDev = fn1;
  fnIsDevInDDB = fn2;
}

int getNodeByOffset(nodeAction_t which, int offset)
{
  switch (which) {
    case actionTBA:
      if (offset < 0 || offset >= cache.addCnt) {
        return -1;
      }
      return cache.addList[offset];
    case actionTBC:
      if (offset < 0 || offset >= cache.reConfigCnt) {
        return -1;
      }
      return cache.reConfigList[offset];
    case actionTBBL:
      if (offset < 0 || offset >= cache.blacklistCnt) {
        return -1;
      }
      return cache.blacklistList[offset];
    case actionTBR:
      if (offset < 0 || offset >= cache.removeCnt) {
        return -1;
      }
      return cache.removeList[offset];
    default:
      return -1;
  }
}

int getCnt(int which)
{
  switch (which) {
    case actionTBA:
      return cache.addCnt;
    case actionTBC:
      return cache.reConfigCnt;
    case actionTBBL:
      return cache.blacklistCnt;
    case actionTBR:
      return cache.removeCnt;
    default:
      return -1;
  }
}

#define REMOVE_DEBUG  0
void onDeviceDone(nodeAction_t which, int nodeId)
{
  int *pCnt = NULL, *pList = NULL;
  int i, found = 0;
#if (REMOVE_DEBUG == 1)
  char actSt[30];
#endif
  switch (which) {
    case actionTBA:
      pList = cache.addList;
      pCnt = &cache.addCnt;
#if (REMOVE_DEBUG == 1)
      memcpy(actSt, "Adding", sizeof("Adding"));
#endif
      break;
    case actionTBC:
      pList = cache.reConfigList;
      pCnt = &cache.reConfigCnt;
#if (REMOVE_DEBUG == 1)
      memcpy(actSt, "Configuring", sizeof("Configuring"));
#endif
      break;
    case actionTBBL:
      pList = cache.blacklistList;
      pCnt = &cache.blacklistCnt;
#if (REMOVE_DEBUG == 1)
      memcpy(actSt, "Blacklisting", sizeof("Blacklisting"));
#endif
      break;
    case actionTBR:
      pList = cache.removeList;
      pCnt = &cache.removeCnt;
#if (REMOVE_DEBUG == 1)
      memcpy(actSt, "Removing", sizeof("Removing"));
#endif
      break;
    default:
      return;
  }
#if (REMOVE_DEBUG == 1)
  printf("%s list, original:\n", actSt);
  for (i = 0; i < *pCnt; i++) {
    printf("%d, ", pList[i]);
  }
  printf("\n");
#endif

  int ret, j;
  for (i = 0; i < *pCnt; i++) {
    if (pList[i] == nodeId) {
#if (REMOVE_DEBUG == 1)
      CS_LOG("Removing Node[%d] from %s list", i, actSt);
#endif
      /* TODO - WHY memmove has PROBLEM? */
      LOCK_MUTEX(mutex);
#if 0
      memmove(&pList[i], &pList[i + 1], *pCnt - i - 1);
#else
      for (j = i; j < *pCnt; j++) {
        pList[j] = pList[j + 1];
      }
#endif
      (*pCnt)--;
      UNLOCK_MUTEX(mutex);
      found = 1;
      break;
    }
  }
#if (REMOVE_DEBUG == 1)
  printf("%s list, After:\n", actSt);
  for (i = 0; i < *pCnt; i++) {
    printf("%d, ", pList[i]);
  }
  printf("\n");
#endif
  hardASSERT(found);
  getActions(0);
}

static int isActionReloadNeeded(void)
{
  int allPollDone = cache.addPollDone && cache.blacklistPollDone
                    && cache.recPollDone && cache.removePollDone;
  int allCntZero = !cache.addCnt && !cache.blacklistCnt
                   && !cache.reConfigCnt && !cache.removeCnt;
  if (!allPollDone && allCntZero) {
    return true;
  }
  return false;
}

int getAction(int which, int forceReload)
{
  int *cnt = NULL, *pollDone = NULL;
  switch (which) {
    case actionTBA:
      pollDone = &cache.addPollDone;
      cnt = &cache.addCnt;
    case actionTBC:
      pollDone = &cache.recPollDone;
      cnt = &cache.reConfigCnt;
    case actionTBBL:
      pollDone = &cache.blacklistPollDone;
      cnt = &cache.blacklistCnt;
    case actionTBR:
      pollDone = &cache.removePollDone;
      cnt = &cache.removeCnt;
    default:
      return 0;
  }
  if (forceReload || !(*pollDone) || *cnt) {
    loadActions();
    return 1;
  }
  return 0;
}

int getActions(int forceReloadAll)
{
  if (forceReloadAll) {
    forceReloadAction();
    loadActions();
    forceReloadAll = 0;
  } else if (isActionReloadNeeded()) {
    loadActions();
  }
  return cache.nodeAction;
}

void forceReloadAction(void)
{
  int ret;
  LOCK_MUTEX(mutex);
  cache.addPollDone = false;
  cache.recPollDone = false;
  cache.removePollDone = false;
  cache.blacklistPollDone = false;
  cache.nodeAction = 0;
  UNLOCK_MUTEX(mutex);
}

static void loadSpecificAction(int which, networkConfig_t *pconfig)
{
  if (loadList(which, pconfig)) {
    SET_ACTION_BIT(cache.nodeAction, which);
  } else {
    CLEAR_ACTION_BIT(cache.nodeAction, which);
  }
}

static void listPrint(uint32_t bitMask)
{
  int i;

  if (IS_ACTION_BIT_SET(bitMask, actionTBA)) {
    printf("Add       List: -- ");
    for (i = 0; i < cache.addCnt; i++) {
      printf("%02d%s", cache.addList[i], i == cache.addCnt - 1 ? "" : ", ");
    }
    printf("\n");
  }

  if (IS_ACTION_BIT_SET(bitMask, actionTBC)) {
    printf("Configure List: -- ");
    for (i = 0; i < cache.reConfigCnt; i++) {
      printf("%02d%s",
             cache.reConfigList[i],
             i == cache.reConfigCnt - 1 ? "" : ", ");
    }
    printf("\n");
  }

  if (IS_ACTION_BIT_SET(bitMask, actionTBR)) {
    printf("Remove    List: -- ");
    for (i = 0; i < cache.removeCnt; i++) {
      printf("%02d%s",
             cache.removeList[i],
             i == cache.removeCnt - 1 ? "" : ", ");
    }
    printf("\n");
  }

  if (IS_ACTION_BIT_SET(bitMask, actionTBBL)) {
    printf("Blacklist List: --");
    for (i = 0; i < cache.blacklistCnt; i++) {
      printf("%02d%s",
             cache.blacklistList[i],
             i == cache.blacklistCnt - 1 ? "" : ", ");
    }
    printf("\n");
  }
}

#define PRINT_LIST 0
static void loadActions(void)
{
  int ret;
  nodeAction_t n = actionTBA;
  networkConfig_t *pconfig = NULL;
  getNetworkConfig((void **)&pconfig);
  hardASSERT(pconfig);

  LOCK_MUTEX(mutex);
  for (; n < actionMAX; n++) {
    loadSpecificAction(n, pconfig);
  }
  UNLOCK_MUTEX(mutex);
  listPrint(0xFFFFFFFF);

  GET_HL_STR(add, "%d", cache.addCnt);
  GET_HL_STR(bl, "%d", cache.blacklistCnt);
  GET_HL_STR(rm, "%d", cache.removeCnt);
  GET_HL_STR(conf, "%d", cache.reConfigCnt);
  CS_MSG("| Add - [%s] | Blacklist - [%s] | Remove - [%s] | Configure - [%s]|\n",
         hlstr_add,
         hlstr_bl,
         hlstr_rm,
         hlstr_conf);
}

void clearList(int which)
{
  switch (which) {
    case actionTBA:
      clearAddCache(&cache);
      break;
    case actionTBC:
      clearRecCache(&cache);
      break;
    case actionTBBL:
      clearBlackListCache(&cache);
      break;
    case actionTBR:
      clearRemoveCache(&cache);
      break;
    default:
      return;
  }
}
static int loadList(nodeAction_t which, const networkConfig_t *pconfig)
{
  int ret = 0, i = 0, j = 0;
  int *pollDone;
  int *pList = NULL, *pCnt = NULL, max = 0;
  bool (*nodeNeedTBx)(int i, const networkConfig_t *pconfig);
  hardASSERT(pconfig && which >= actionTBA && which <= actionTBC);

  switch (which) {
    case actionTBA:
      clearAddCache(&cache);
      pollDone = &cache.addPollDone;
      pList = cache.addList;
      pCnt = &cache.addCnt;
      max = MAX_ADD_LIST_SIZE;
      nodeNeedTBx = nodeNeedTBA;
      break;
    case actionTBC:
      clearRecCache(&cache);
      pollDone = &cache.recPollDone;
      pList = cache.reConfigList;
      pCnt = &cache.reConfigCnt;
      max = MAX_RECONFIG_LIST_SIZE;
      nodeNeedTBx = nodeNeedTBC;
      break;
    case actionTBBL:
      clearBlackListCache(&cache);
      pollDone = &cache.blacklistPollDone;
      pList = cache.blacklistList;
      pCnt = &cache.blacklistCnt;
      max = MAX_BLACKLIST_LIST_SIZE;
      nodeNeedTBx = nodeNeedTBBL;
      break;
    case actionTBR:
      clearRemoveCache(&cache);
      pollDone = &cache.removePollDone;
      pList = cache.removeList;
      pCnt = &cache.removeCnt;
      max = MAX_REMOVE_LIST_SIZE;
      nodeNeedTBx = nodeNeedTBR;
      break;
    default:
      return 0;
  }

  if (!(*pollDone)) {
    for (i = 0; i < pconfig->nodeCnt; i++) {
      if (nodeNeedTBx(i, pconfig)) {
        ret = 1;
        if ((which == actionTBR) && featureIsEnabled(pconfig->pNodes[i].feature, 'l')) {
          /* Always add LPN to the head	 */
          /* memmove(pList + 1, pList, *pCnt); */
          for (j = *pCnt; j > 0; j--) {
            pList[j] = pList[j - 1];
          }
          pList[0] = i;
          *pCnt += 1;
        } else {
          pList[*pCnt] = i;
          *pCnt += 1;
        }
        if (*pCnt == max) {
          break;
        }
      }
    }
    if (*pCnt == 0) {
      *pollDone = 1;
    }
  }

  return ret;
}

/******************************************************************
 * ---------------------------------------------------------------
 * ***************************************************************/
void wrapAction(nodeAction_t *a)
{
  *a = (*a + 1) % actionMAX;
  if (*a == actionNone) {
    *a = actionTBA;
  }
}

int loadActionEngine(nodeAction_t na, const networkConfig_t *pconfig, int *inProgress)
{
  int ret = 0;
  nodeAction_t which;

  if (pconfig == NULL) {
    return E_INVP;
  }
  if (cache.nodeAction != actionNone) {
    return E_SUC;
  }

  /* No nodes loaded */
  if (pconfig->nodeCnt == 0) {
    cache.addPollDone = true;
    cache.recPollDone = true;
    cache.removePollDone = true;
    cache.blacklistPollDone = true;
    return E_SUC;
  }

  if (na != actionNone && na != actionMAX) {
    which = na;
  } else {
    which = cache.prioAction;
  }

  do {
    if (E_SUC != (ret = loadList(which, pconfig))) {
      return ret;
    }
    if (cache.nodeAction != actionNone) {
      break;
    }
    wrapAction(&which);
  } while (which != cache.prioAction && (na == actionNone || na == actionMAX));

  /* All nodes are handled properly */
  if ((na == actionTBA) && cache.addPollDone) {
    *inProgress = 0;
  } else if ((na == actionTBC) && cache.recPollDone) {
    *inProgress = 0;
  } else if ((na == actionTBR) && cache.removePollDone) {
    *inProgress = 0;
  } else if ((na == actionTBBL) && cache.blacklistPollDone) {
    *inProgress = 0;
  } else if (cache.addPollDone && cache.blacklistPollDone
             && cache.recPollDone && cache.removePollDone) {
    *inProgress = 0;
  }

  return E_SUC;
}

void reloadListOnClearCache(uint32_t *nodeId)
{
  int n = *nodeId;
  if (cache.nodeAction == actionTBA) {
    for (int i = 0; i < cache.addCnt; i++) {
      if (cache.addList[i] == n) {
        /* pconfig->pNodes[n].done = 1; */
        for (int j = i; j < cache.addCnt - 1; j++) {
          cache.addList[j] = cache.addList[j + 1];
        }
        cache.addCnt--;
        break;
      }
    }

    if (cache.addCnt == 0) {
      cache.nodeAction = actionNone;
    }
  } else if (cache.nodeAction == actionTBC) {
    for (int i = 0; i < cache.reConfigCnt; i++) {
      if (cache.reConfigList[i] == n) {
        for (int j = i; j < cache.reConfigCnt - 1; j++) {
          cache.reConfigList[j] = cache.reConfigList[j + 1];
        }
        cache.reConfigCnt--;
        break;
      }
    }

    if (cache.reConfigCnt == 0) {
      cache.nodeAction = actionNone;
    } else {
      *nodeId = cache.reConfigList[0];
      /* SET_NODE_ID_VALID_BIT(*nodeId); */
    }
  } else {
    for (int i = 0; i < cache.removeCnt; i++) {
      if (cache.removeList[i] == n) {
        for (int j = i; j < cache.removeCnt - 1; j++) {
          cache.removeList[j] = cache.removeList[j + 1];
        }
        cache.removeCnt--;
        break;
      }
    }

    if (cache.removeCnt != 0) {
      *nodeId = cache.removeList[0];
      /* SET_NODE_ID_VALID_BIT(*nodeId); */
    }
  }
}

static bool nodeNeedTBR(int i, const networkConfig_t *pconfig)
{
  /*
   * remove | done | in lddb?| action
   * 0         |  0	  | 0       | Pass, will be handled by TBA
   * 0         |  0	  | 1       | Probably provisioning was failed for some reason
   * 0         |  1	  | 1       | Pass
   * 0         |  1	  | 0       | Assert(0)
   * 1         |  0	  | 0       | Blacklisted already <--------------------------------|
   * 1         |  0	  | 1       |	Probably provisioning was failed for some reason     |
   * 1         |  1	  | 0       | Assert(0) - Blacklisted but done flag write failed.  |
   * 1         |  1	  | 1       | Needs to be blacklisted ----------Should-------------|
   */
  int b = ((pconfig->pNodes[i].blacklist & REMOVE_BIT_MASK) >> 4);
  int d = pconfig->pNodes[i].done & PROVISIONED_BIT_MASK;
  int in = fnIsDevInDDB(i);

  if (d == 0 && in == 1) {
    /* x01 */
    /* Provisioning failed, remove the node from LDDB */
    actDEBUG("Removing one node from LDDB\n");
    if (fnRmDev) {
      fnRmDev(0, i);
    }
  } else if (d != in) {
    /* x10 */
    /* hardASSERT(0); */
    CS_HLE("The Config File and the Device Database is Out Of Sync - %s",
           "Factory Reset Required?");
  } else if (b == 1 && d == 1 && in == 1) {
    /* 111 */
    return true;
  }
  /* 000, 011, 100 */
  return false;
}

static bool nodeNeedTBBL(int i, const networkConfig_t *pconfig)
{
  /*
   * blacklist | done | in lddb?| action
   * 0         |  0	  | 0       | Pass, will be handled by TBA
   * 0         |  0	  | 1       | Probably provisioning was failed for some reason
   * 0         |  1	  | 1       | Pass
   * 0         |  1	  | 0       | Assert(0)
   * 1         |  0	  | 0       | Blacklisted already <--------------------------------|
   * 1         |  0	  | 1       |	Probably provisioning was failed for some reason     |
   * 1         |  1	  | 0       | Assert(0) - Blacklisted but done flag write failed.  |
   * 1         |  1	  | 1       | Needs to be blacklisted ----------Should-------------|
   */
  int b = pconfig->pNodes[i].blacklist & BLACKLIST_BIT_MASK;
  int d = pconfig->pNodes[i].done & PROVISIONED_BIT_MASK;
  int in = fnIsDevInDDB(i);

  if (d == 0 && in == 1) {
    /* x01 */
    /* Provisioning failed, remove the node from LDDB */
    actDEBUG("Removing one node from LDDB");
    if (fnRmDev) {
      fnRmDev(1, i);
    }
  } else if (d != in) {
    /* x10 */
    /* hardASSERT(0); */
    CS_HLE("The Config File and the Device Database is Out Of Sync - %s",
           "Factory Reset Required?");
  } else if (b == 1 && d == 1 && in == 1) {
    /* 111 */
    return true;
  }
  /* 000, 011, 100 */
  return false;
}

static bool nodeNeedTBA(int i, const networkConfig_t *pconfig)
{
  int br = pconfig->pNodes[i].blacklist;
  int d = pconfig->pNodes[i].done & PROVISIONED_BIT_MASK;
  int in = fnIsDevInDDB(i);

  if ( br || d == 1 || in == 1) {
    return false;
  }

  return true;
}

static bool nodeNeedTBC(int i, const networkConfig_t *pconfig)
{
  int br = pconfig->pNodes[i].blacklist;
  int d = pconfig->pNodes[i].done & PROVISIONED_BIT_MASK;
  int c = (pconfig->pNodes[i].done & CONFIGURED_BIT_MASK) >> 4;
  int in = fnIsDevInDDB(i);

  /* blacklisted || unprovisioned || configured || not in ddb */
  if (br  || d == 0 || c == 1 || in == 0) {
    return false;
  }

  return true;
}
