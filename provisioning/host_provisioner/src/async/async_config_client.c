/*************************************************************************
    > File Name: async_config_client.c
    > Author: Kevin
    > Created Time: 2019-04-17
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include <signal.h>
#include <time.h>
#include <errno.h>

#include "async_config_client.h"
#include "err_code.h"
#include "conf_generic_parser.h"
#include "generic_main.h"
#include "action.h"
#include "network_manage.h"

/* Defines  *********************************************************** */
#define TIMER_SIG SIGRTMAX
#define INVALID_NODE_INDEX  (-1)
#define DEBUG_ACC_VERBOSE  0

/* Global Variables *************************************************** */
const char *stateNames[] = {
  "Provisioning",
  "Provisioned",
  "Getting DCD",
  "Adding App Key(s)",
  "Binding App Key(s)",
  "Set Model Publication Address",
  "Add Model Subscription Address",
  "Set TTL/Proxy/Friend/Relay/Nettx",
  "Configuration End",
  "Remove Node",
  "Remove Node End"
};

/* Static Variables *************************************************** */
static accInitConfig_t *accconfig = NULL;
/* static tbcStatus_t pTbcStatus; */
static int configFailedList[DEFAULT_MAX_NODES];

static struct __cache{
  int inUse;
  int timerCreated;
  timer_t timerid;
  struct sigevent sev;

  tbcCache_t cache;
} caches[MAX_CONCURRENT_CONFIG_NODES];

static struct __stateList{
  size_t statesNum;
  stateInstance_t *pStates;
} stateList;

/* static stateInstance_t *pStates = NULL; */
static volatile int stateMachineStarted = false;
static volatile int newDeviceAdded = false;

/* *************************************************** */
static int getFreeCache(void);
static stateInstance_t *getStateInsByState(int state);

static stateInstance_t getDcdIns = {
  getDcd_em,
  getDcdEntryGuard,
  getDcdStateEntry,
  getDcdStateInprogress,
  getDcdStateRetry,
  getDcdStateExit,
  isGetDcdRelatedPacket,
  NULL
};

static stateInstance_t addAppKeyIns = {
  addAppKey_em,
  addAppKeyEntryGuard,
  addAppKeyStateEntry,
  addAppKeyStateInprogress,
  addAppKeyStateRetry,
  addAppKeyStateExit,
  isAddAppKeyRelatedPacket,
  NULL
};

static stateInstance_t bindAppKeyIns = {
  bindAppKey_em,
  bindAppKeyEntryGuard,
  bindAppKeyStateEntry,
  bindAppKeyStateInprogress,
  bindAppKeyStateRetry,
  bindAppKeyStateExit,
  isBindAppKeyRelatedPacket,
  NULL
};

static stateInstance_t setPubIns = {
  setPub_em,
  setPubEntryGuard,
  setPubStateEntry,
  setPubStateInprogress,
  setPubStateRetry,
  setPubStateExit,
  isSetPubRelatedPacket,
  NULL
};

static stateInstance_t addSubIns = {
  addSub_em,
  addSubEntryGuard,
  addSubStateEntry,
  addSubStateInprogress,
  addSubStateRetry,
  addSubStateExit,
  isAddSubRelatedPacket,
  NULL
};

static stateInstance_t setConfigsIns = {
  setConfig_em,
  setConfigsEntryGuard,
  setConfigsStateEntry,
  setConfigsStateInprogress,
  setConfigsStateRetry,
  setConfigsStateExit,
  isSetConfigsRelatedPacket,
  NULL
};

static stateInstance_t endIns = {
  end_em,
  NULL,
  endStateEntry,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

static stateInstance_t removeNodeIns = {
  resetNode_em,
  removeNodeEntryGuard,
  removeNodeStateEntry,
  removeNodeStateInprogress,
  removeNodeStateRetry,
  removeNodeStateExit,
  isRemoveNodeRelatedPacket,
  NULL
};

static stateInstance_t removeEndIns = {
  resetNodeEnd_em,
  NULL,
  removeEndStateEntry,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

/* Static Functions Declaractions ************************************* */
static int nodeInFailList(int nodeId);
static void addOneToFailList(int nodeId);
#if 0
static int delegateConfigPackets(uint32_t evtId);
#endif
static void registerDefaultStates(void);
static void resetCacheByIndex(int index);
static void resetCache(struct __cache *cache, int index);
static int loadNodes(int type);
static int tbcStateEngine(void);
static void accResetAll(void);
static void createTimer(tbcCache_t *tbc);
static void sigInit(void);
#if 0
static void deleteTimer(tbcCache_t *tbc);
#endif
static void timerInit(void);

void accInit(void *p)
{
  accconfig = p;
  sigInit();
  accResetAll();
  timerInit();

  enOrDisConfigStateMachine(p ? accconfig->enableStateMachine : 1);
}

void startTimer(tbcCache_t *tbc, int start)
{
#if (DEBUG_GUARD_TIMER_OFF == 0)
  struct itimerspec ts;

  memset(&ts, 0, sizeof(struct itimerspec));
  if (start) {
    ts.it_value.tv_sec = DEFAULT_GUARD_SEC;
  }

  if (-1 == timer_settime(caches[tbc->selfId].timerid, 0, &ts, NULL)) {
    errExit("timer_settime");
  }
#endif
  /* CS_LOG("Node[%d]: Guard Timer %s\n", */
  /* tbc->nodeIndex, */
  /* start ? "Started" : "Stopped"); */
}

int accMainLoop(void *p)
{
  int cnt = 0, busy = 0, action;

  switch (stateMachineStarted) {
    case not_start_em:
      break;
    case starting_em:
      stateMachineStarted = started_em;
      break;
    case started_em:
      action = getLoadedActions();
      if (IS_ACTION_BIT_SET(action, actionTBR)) {
        if (0 != (cnt = loadNodes(actionTBR))) {
          CS_MSG("Load %d Node%s to Remove.\n", cnt, cnt > 1 ? "s" : "");
        }
      } else {
        if (0 != (cnt = loadNodes(actionTBC))) {
          CS_MSG("Load %d Node%s to Configure.\n", cnt, cnt > 1 ? "s" : "");
        }
      }
      busy |= tbcStateEngine();
      break;
    case stopping_em:
      /* TODO - Update the latest states to the config files */
      stateMachineStarted = not_start_em;
      break;
    default:
      hardASSERT(0);
      break;
  }

  return busy;
}

void enOrDisConfigStateMachine(bool enable)
{
  if (stateMachineStarted == started_em && !enable) {
    /* Stop  */
    stateMachineStarted = stopping_em;
  } else if (stateMachineStarted == not_start_em && enable) {
    /* Start  */
    stateMachineStarted = starting_em;
  }
}

void setNewDeviceAddedFlag(void)
{
  newDeviceAdded = 1;
}

void asyncConfigClientReset(void)
{
  accResetAll();
}

size_t getConfigStateNum(void)
{
  return stateList.statesNum;
}
/**
 * @brief addStateAfter add an item to the states
 *
 * @param ps - state instance
 * @param state - after which state the new state should be added, -1 to add to
 * the head
 *
 * @return
 */
errcode_t addStateAfter(const stateInstance_t *ps, int state)
{
  bool found = false;
  stateInstance_t *p = stateList.pStates, *pn;

  /* Add to the head */
  if (state == -1) {
    NEW_STATE_INS(insert);
    memcpy(pnewinsert, ps, sizeof(stateInstance_t));
    pnewinsert->next = stateList.pStates;
    stateList.pStates = pnewinsert;
    stateList.statesNum++;
    return E_SUC;
  }

  while (p) {
    if (p->state == state) {
      found = true;
      break;
    }
    p = p->next;
  }

  if (!found) {
    return E_NFND;
  }

  NEW_STATE_INS(insert);
  memcpy(pnewinsert, ps, sizeof(stateInstance_t));
  pn = p->next;
  p->next = pnewinsert;
  pnewinsert->next = pn;
  stateList.statesNum++;

  return E_SUC;
}

/**
 * @brief getFreeCache get cache ID which is not in use
 *
 * @return
 */
static int getFreeCache(void)
{
  int i;
  for (i = 0; i < MAX_CONCURRENT_CONFIG_NODES; i++) {
    if (!caches[i].inUse) {
      return i;
    }
  }
  return -1;
}

static int getFreeCacheCnt(void)
{
  int i, ret = 0;
  for (i = 0; i < MAX_CONCURRENT_CONFIG_NODES; i++) {
    if (!caches[i].inUse) {
      ret++;
    }
  }

  return ret;
}

/**
 * @brief freeStates Free all state instances
 */
static void freeStates(void)
{
  stateInstance_t *p = stateList.pStates, *pn;
  if (p) {
    pn = p->next;
    free(p);
    p = pn;
  }
  stateList.statesNum = 0;
  stateList.pStates = NULL;
}

static void accResetAll(void)
{
  int i;

  /* memset(&pTbcStatus, 0, sizeof(tbcStatus_t)); */
  clearFailList();
  for (i = 0; i < MAX_CONCURRENT_CONFIG_NODES; i++) {
    resetCacheByIndex(i);
  }

  stateMachineStarted = false;
  newDeviceAdded = false;
  freeStates();
  addStateAfter(&getDcdIns, -1);
  addStateAfter(&endIns, getDcd_em);
  addStateAfter(&removeEndIns, end_em);

  if (accconfig && accconfig->useDefaultStates) {
    registerDefaultStates();
  }
}

static void registerDefaultStates(void)
{
#if (STATE_TEST == 1)
  addStateAfter(&setConfigsIns, getDcd_em);
#else
  addStateAfter(&addAppKeyIns, getDcd_em);
  addStateAfter(&bindAppKeyIns, addAppKey_em);
  addStateAfter(&setPubIns, bindAppKey_em);
  addStateAfter(&addSubIns, setPub_em);
  addStateAfter(&setConfigsIns, addSub_em);
#endif

  addStateAfter(&removeNodeIns, end_em);
}

static stateInstance_t *getStateInsBySeq(int whichOne)
{
  int i;
  stateInstance_t *p = stateList.pStates;

  if (whichOne < 0 || whichOne >= stateList.statesNum) {
    return NULL;
  }

  for (i = 0; i < whichOne; i++) {
    if (!p) {
      return NULL;
    }
    p = p->next;
  }

  return p;
}

static tbcCache_t *getCacheByHandle(const struct gecko_cmd_packet *e)
{
  int i, handle;
  struct __cache *p = NULL;

  switch (BGLIB_MSG_ID(e->header)) {
    case gecko_evt_mesh_config_client_dcd_data_id:
      handle = e->data.evt_mesh_config_client_dcd_data.handle;
      break;
    case gecko_evt_mesh_config_client_dcd_data_end_id:
      handle = e->data.evt_mesh_config_client_dcd_data_end.handle;
      break;
    case gecko_evt_mesh_config_client_appkey_status_id:
      handle = e->data.evt_mesh_config_client_appkey_status.handle;
      break;
    case gecko_evt_mesh_config_client_binding_status_id:
      handle = e->data.evt_mesh_config_client_binding_status.handle;
      break;
    case gecko_evt_mesh_config_client_model_pub_status_id:
      handle = e->data.evt_mesh_config_client_model_pub_status.handle;
      break;
    case gecko_evt_mesh_config_client_model_sub_status_id:
      handle = e->data.evt_mesh_config_client_model_sub_status.handle;
      break;
    case gecko_evt_mesh_config_client_relay_status_id:
      handle = e->data.evt_mesh_config_client_relay_status.handle;
      break;
    case gecko_evt_mesh_config_client_friend_status_id:
      handle = e->data.evt_mesh_config_client_friend_status.handle;
      break;
    case gecko_evt_mesh_config_client_gatt_proxy_status_id:
      handle = e->data.evt_mesh_config_client_gatt_proxy_status.handle;
      break;
    case gecko_evt_mesh_config_client_default_ttl_status_id:
      handle = e->data.evt_mesh_config_client_default_ttl_status.handle;
      break;
    case gecko_evt_mesh_config_client_network_transmit_status_id:
      handle = e->data.evt_mesh_config_client_network_transmit_status.handle;
      break;
    case gecko_evt_mesh_config_client_reset_status_id:
      handle = e->data.evt_mesh_config_client_reset_status.handle;
      break;
    case gecko_evt_mesh_config_client_beacon_status_id:
      handle = e->data.evt_mesh_config_client_beacon_status.handle;
      break;

    default:
      CS_ERR("NEED ADD a case to %s\n", __FUNCTION__);
      hardASSERT(0);
      break;
  }

  for (i = 0; i < MAX_CONCURRENT_CONFIG_NODES; i++) {
    p = &caches[i];
    if (p->inUse && p->cache.handle == handle) {
#if (DEBUG_ACC_VERBOSE == 1)
      CS_LOG("Packet to %d cache\n", i);
#endif
      return &p->cache;
    }
  }

  CS_ERR("No Cache Found by handle\n");
  hardASSERT(0);
  return NULL;
}

/**
 * @brief identifyCacheFromGeneralPayload Get which tbcCache_t should be used
 * from generalPayload_t.
 *
 * @param payload
 *
 * @return
 */
#if 0
static tbcCache_t *identifyCacheFromGeneralPayload(generalPayload_t *payload)
{
  /* if (PACKET_TYPE_RSP(payload->bgCmd.in->header)) { */
  /* [> return getCacheByLocalHandle(payload->localHandle); <] */
  /* hardASSERT(caches[payload->selfId].inUse); */
  /* return &caches[payload->selfId].cache; */
  /* } else { */
  return getCacheByHandle(payload->bgCmd.in);
  /* } */
  /* return NULL; */
}
#endif

static int is_config_device_events(const struct gecko_cmd_packet *e)
{
  uint32_t evt_id = BGLIB_MSG_ID(e->header);
  return ((evt_id & 0x000000ff) == 0x000000a0
          && (evt_id & 0x00ff0000) == 0x00270000);
}

int config_devices_event_handler(struct gecko_cmd_packet *e)
{
  int ret = 0;
  tbcCache_t *tbc = NULL;
  stateInstance_t *state = NULL;

  hardASSERT(e);
  if (!is_config_device_events(e)) {
    /* CS_LOG("Event [0x%08x] is not config event\n", BGLIB_MSG_ID(e->header)); */
    return 0;
  }

  tbc = getCacheByHandle(e);
  hardASSERT(tbc);
  state = getStateInsByState(tbc->state);
  hardASSERT(state);

  if (WAIT_RESPONSE(tbc) && state->onStateInProgress) {
#if (DEBUG_ACC_VERBOSE == 1)
    CS_LOG("onStateInProgress once.\n");
#endif
    ret = state->onStateInProgress(e, tbc);
  }

  if (!ret && !WAIT_RESPONSE(tbc) && EVER_RETRIED(tbc) && state->onStateRetry) {
#if (DEBUG_ACC_VERBOSE == 1)
    CS_LOG("onStateRetry once.\n");
#endif
    ret |= state->onStateRetry(tbc, on_timeout_em);
  } else if (ret == E_UNSPEC) {
    return 0;
  }

  return 1;
}

static void handleException(void)
{
  int i = 0, ret = 0;
  tbcCache_t *tbc = NULL;
  stateInstance_t *state = NULL;

  for (i = 0; i < MAX_CONCURRENT_CONFIG_NODES; i++) {
    if (caches[i].inUse) {
      tbc = &caches[i].cache;
      state = getStateInsByState(tbc->state);
      if (tbc->expired && state->onStateRetry) {
        ret = state->onStateRetry(tbc, on_guard_timer_expired_em);
        if (ret != E_SUC) {
          CS_ERR("Expired Return %d\n", ret);
        }
      } else if (OOM(tbc) && state->onStateRetry) {
        ret = state->onStateRetry(tbc, on_oom_em);
        CS_LOG("Node[%d]: OOM Recovery Once.\n", tbc->nodeIndex);
        if (ret != E_SUC && ret != E_OOM) {
          CS_ERR("OOM Return %d - %s\n", ret, ERR_NAME[ret]);
        }
      }
    }
  }
  return;
}

#if 0
/**
 * @brief stateHandleEvents Handle events in specific state
 *
 * @param in generalPayload_t
 *
 * @return 0 - normally, 1 - error
 */
static int stateHandleEvents(void *in)
{
  int ret = 0;
  generalPayload_t *payload = (generalPayload_t *)in;
  tbcCache_t *tbc = NULL;
  stateInstance_t *state = NULL;

  tbc = identifyCacheFromGeneralPayload(payload);
  hardASSERT(tbc);
  state = getStateInsByState(tbc->state);
  hardASSERT(state);

  if (WAIT_RESPONSE(tbc) && state->onStateInProgress) {
#if (DEBUG_ACC_VERBOSE == 1)
    CS_LOG("onStateInProgress once.\n");
#endif
    ret |= state->onStateInProgress(in, tbc);
  }

  if (!ret && !WAIT_RESPONSE(tbc) && EVER_RETRIED(tbc) && state->onStateRetry) {
#if (DEBUG_ACC_VERBOSE == 1)
    CS_LOG("onStateRetry once.\n");
#endif
    ret |= state->onStateRetry(tbc, on_timeout_em);
  }

  return ret;
}
#endif

int stateToNextState(void *in)
{
  int ret, transitioned = 0;
  stateInstance_t *psi = NULL, *pnsi = NULL;
  tbcCache_t *cache = (tbcCache_t *)in;
  /* If current state exit callback exist, exist first */
  psi = getStateInsByState(cache->state);
  if (cache->nextState == -1) {
    pnsi = psi->next;
  } else {
    pnsi = getStateInsByState(cache->nextState);
  }

  accDEBUG("Node[%d]: Exiting from %s state\n",
           cache->nodeIndex,
           stateNames[cache->state]);
  if (psi && psi->onStateExit) {
    ret = psi->onStateExit(cache);
    accDEBUG("Node[%d]: Exiting CB called.\n", cache->nodeIndex);
  }

  if (!pnsi) {
    /* If specified next state doesn't exist, load the next of the current */
    /* state */
    pnsi = psi->next;
  }

  /* Find next state with valid entry */
  while (pnsi && !pnsi->onStateEntry) {
    pnsi = pnsi->next;
  }

  while (pnsi) {
    accDEBUG("Node[%d]: Try to enter %s state\n",
             cache->nodeIndex,
             stateNames[pnsi->state]);
    ret = pnsi->onStateEntry(cache, pnsi->entryGuard);
    switch (ret) {
      case E_SUC:
      case E_OOM:
        cache->state = pnsi->state;
        cache->nextState = pnsi->state;
        transitioned = 1;
        accDEBUG("Node[%d]: Enter %s state Success\n",
                 cache->nodeIndex,
                 stateNames[pnsi->state]);
        break;
      /* Implementation of the callback should make sure that won't return  */
      /* this if not more states to load */
      case E_TONEXT:
        pnsi = pnsi->next;
        break;
      default:
        pnsi = getStateInsByState(end_em);
        break;
    }
    if (transitioned) {
      break;
    }
  }

  return transitioned;
}

static void onOneCacheEnd(struct __cache *cache)
{
  GET_HL_STR(finish, "%s", "Configuration Finished.");
  CS_MSG("Node[%d]: %s\n", cache->cache.nodeIndex, hlstr_finish);
  resetCache(cache, cache->cache.selfId);
}

static int tbcStateEngine(void)
{
  int i, busy = 0;
  struct __cache *pc = NULL;
  /* stateInstance_t *pStateIns = NULL; */
  /* int targetState; */
  tbcCache_t *cache = NULL;

  /* 1. Check if any Exception (OOM | Guard timer expired) happened in last round */
  handleException();

#if 0
  /* 2. Check the queue and handle the received events */
  cnt = MIN(itemsInQueue(send_to_tbc_q_em), MAX_CONCURRENT_CONFIG_NODES);
  for (i = 0; i < cnt; i++) {
    payload = receiveOneItemFromQueue(send_to_tbc_q_em);
    if (payload) {
      /* CS_LOG("One packet to tbc\n"); */
      ret = stateHandleEvents(payload);
      softASSERT(ret == 0);
      busy |= 1;
    } else {
      hardASSERT(0);
      break;
    }
  }
#endif

  /* 4. Check if any state needs to update */
  for (i = 0; i < MAX_CONCURRENT_CONFIG_NODES; i++) {
    pc = &caches[i];
    cache = &pc->cache;
    if (!pc->inUse || WAIT_RESPONSE(cache)) {
      continue;
    }

    if (cache->state != cache->nextState) {
      busy |= stateToNextState(cache);
      if (cache->state == end_em || cache->state == resetNodeEnd_em) {
        if (cache->errCache.callErrBits != 0
            || cache->errCache.evtErrBits != 0) {
          addOneToFailList(cache->nodeIndex);
        }
        onOneCacheEnd(pc);
      }
    }
  }

  /* Check if the entry is success */
  return busy;
}

#if 0
static int delegateConfigPackets(uint32_t evtId)
{
  int i;
  stateInstance_t *state = stateList.pStates;
  for (i = 0; i < stateList.statesNum; i++) {
    if (state && state->isRelatedPacket) {
      if (state->isRelatedPacket(evtId)) {
        return 1;
      }
    }
    state = state->next;
  }
  return 0;
}
#endif

static stateInstance_t *getStateInsByState(int state)
{
  int i;
  stateInstance_t *p = NULL;

  if (state < CONFIGURATION_STATE_MIN || state > CONFIGURATION_STATE_MAX) {
    return NULL;
  }

  for (i = 0; i < stateList.statesNum; i++) {
    p = getStateInsBySeq(i);
    if (!p) {
      return NULL;
    }
    if (state == p->state) {
      return p;
    }
  }

  return NULL;
}

static void resetCache(struct __cache *cache, int index)
{
  hardASSERT(cache);

  cache->inUse = 0;

  memset(&cache->cache, 0, sizeof(tbcCache_t));
  cache->cache.state = provisioned_em;
  cache->cache.nextState = getDcd_em;
  cache->cache.nodeIndex = INVALID_NODE_INDEX;
  cache->cache.selfId = index;
}

static void resetCacheByIndex(int index)
{
  hardASSERT(index >= 0 && index <= MAX_CONCURRENT_CONFIG_NODES);
  resetCache(&caches[index], index);
}

static bool needsSetTrans(int nettxCnt,
                          int nettxInterval)
{
  if (nettxCnt > 8) {
    nettxCnt = 0;
    CS_ERR("Network Transmission invalid, valid range 1-8.\n");
    return false;
  } else if (nettxCnt == 0) {
    return false;
  }

  if (nettxInterval < 10 || nettxInterval > 320 || nettxInterval % 10) {
    CS_ERR("Network Transmission interval invalid, valid range 10-320, step by 10.\n");
    return false;
  }
  return true;
}

static void parseFeatures(uint32_t *pFeatureBit,
                          const nodeConf_t *pnconfig)
{
  const char *pFeatureString = pnconfig->feature;
  /* If need to set default TTL */
  if (!pnconfig->features) {
    if (pnconfig->ttl) {
      *pFeatureBit |= FBIT(ttl_em);
      *pFeatureBit |= FBIT(ttl_em + 16);
    }
    /* If need to set the relay, proxy, friend and lpn */
    for (int i = 0; i < 4; i++) {
      if (pFeatureString[i] != 0) {
        /* *pFeatureBit |= FBIT(bits_valid_em); */
        if (featureIsEnabled(pFeatureString, 'r')) {
          *pFeatureBit |= FBIT(relay_em);
          *pFeatureBit |= FBIT(relay_em + 16);
        }
        if (featureIsEnabled(pFeatureString, 'p')) {
          *pFeatureBit |= FBIT(proxy_em);
          *pFeatureBit |= FBIT(proxy_em + 16);
        }
        if (featureIsEnabled(pFeatureString, 'f')) {
          *pFeatureBit |= FBIT(friend_em);
          *pFeatureBit |= FBIT(friend_em + 16);
        }
        if (featureIsEnabled(pFeatureString, 'l')) {
          *pFeatureBit |= FBIT(lpn_em);
        }
        break;
      }
    }
    /* If need to set the network transmission */
    if (needsSetTrans(pnconfig->transCnt, pnconfig->transIntv)) {
      *pFeatureBit |= FBIT(net_trans_em);
      *pFeatureBit |= FBIT(net_trans_em + 16);
    }
#define SET_CONFIGS_DEBUG 1
#if (SET_CONFIGS_DEBUG == 1)
    *pFeatureBit |= FBIT(relay_em + 16);
    *pFeatureBit |= FBIT(proxy_em + 16);
    *pFeatureBit |= FBIT(friend_em + 16);
#endif
  } else {
    *pFeatureBit = pnconfig->features;
  }
}

static void loadCacheByNodeId(int cacheId,
                              int nodeId,
                              networkConfig_t *pconfig,
                              int type)
{
  struct __cache *cache = &caches[cacheId];
  tbcCache_t *tbc = &cache->cache;
  /* memset(cache, 0, sizeof(struct __cache)); */
  resetCacheByIndex(cacheId);

  cache->inUse = 1;
  if (type == actionTBC) {
    tbc->state = provisioned_em;
    tbc->nextState = getDcd_em;
    /* TODO - Should move to xml parser */
    parseFeatures(&tbc->features,
                  &pconfig->pNodes[nodeId]);
    GET_HL_STR(finish, "%s", "Configuration Started.");
    CS_MSG("Node[%d]: %s\n", nodeId, hlstr_finish);
  } else if (type == actionTBR) {
    tbc->state = end_em;
    tbc->nextState = resetNode_em;
    GET_HL_STR(finish, "%s", "Removal Started.");
    CS_MSG("Node[%d]: %s\n", nodeId, hlstr_finish);
  }

  tbc->pconfig = pconfig;
  tbc->nodeIndex = nodeId;
  tbc->selfId = cacheId;
}

static int nodeInCaches(int nodeId)
{
  int i = 0;
  for (i = 0; i < MAX_CONCURRENT_CONFIG_NODES; i++) {
    if (caches[i].inUse && caches[i].cache.nodeIndex == nodeId) {
      return 1;
    }
  }
  return 0;
}

static int getNodeIdNotInCaches(int type)
{
  int tbcNodeCnt = 0, i, nodeId;
  tbcNodeCnt = getCnt(type);
  for (i = 0; i < tbcNodeCnt; i++) {
    nodeId = getNodeByOffset(type, i);
    if (!nodeInCaches(nodeId) && !nodeInFailList(nodeId)) {
      return nodeId;
    }
  }
  return -1;
}

static int loadNodes(int type)
{
  networkConfig_t *pconfig = NULL;
  int i, actions, freeCacheId, nodeId, cnt, usedCacheCnt, notUsedCacheCnt,
      tbcNodesCnt;

  if (type == actionTBC) {
    if (newDeviceAdded) {
      getAction(actionTBC, 1);
      newDeviceAdded = 0;
      CS_LOG("Reload Action since new device added.\n");
    }
  }

  actions = getLoadedActions();
  if (!IS_ACTION_BIT_SET(actions, type)) {
    return 0;
  }

  getNetworkConfig((void **)&pconfig);
  hardASSERT(pconfig);

  notUsedCacheCnt = getFreeCacheCnt();
  usedCacheCnt = MAX_CONCURRENT_CONFIG_NODES - notUsedCacheCnt;
  tbcNodesCnt = getCnt(type);

  if (tbcNodesCnt <= usedCacheCnt) {
    return 0;
  }

  cnt = MIN(tbcNodesCnt - usedCacheCnt, getFreeCacheCnt());

  for (i = 0; i < cnt; i++) {
    freeCacheId = getFreeCache();
    /* nodeId = getNodeByOffset(actionTBC, usedCacheCnt + i); */
    nodeId = getNodeIdNotInCaches(type);
    if (nodeId == -1) {
      break;
    }
    CS_LOG("NODE ID %d Loaded\n", nodeId);
    loadCacheByNodeId(freeCacheId, nodeId, pconfig, type);
  }

  return i;
}

void clearFailList(void)
{
  memset(configFailedList, -1, DEFAULT_MAX_NODES * sizeof(int));
}

static void addOneToFailList(int nodeId)
{
  int i;
  for (i = 0; i < DEFAULT_MAX_NODES; i++) {
    if (configFailedList[i] == -1) {
      configFailedList[i] = nodeId;
      CS_ERR("Added Node[%d] to the fail list.\n", nodeId);
      return;
    }
  }
}

static int nodeInFailList(int nodeId)
{
  int i;
  for (i = 0; i < DEFAULT_MAX_NODES; i++) {
    if (configFailedList[i] == nodeId) {
      return 1;
    }
  }
  return 0;
}

static void handler(int sig, siginfo_t *si, void *uc)
{
  CS_ERR("Guard Timer Expired\n"); /* NOTE - Unsafe */
  tbcCache_t *tbc = si->si_value.sival_ptr;

  hardASSERT(tbc);
  tbc->expired = 1;
}

#if 0
static void deleteTimer(tbcCache_t *tbc)
{
  if (0 != timer_delete(caches[tbc->selfId].timerid)) {
    errExit("timer_delete");
  }
}
#endif

static void sigInit(void)
{
  struct sigaction sa;

  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = handler;
  sigemptyset(&sa.sa_mask);
  if (-1 == sigaction(TIMER_SIG, &sa, NULL)) {
    errExit("sigaction");
  }
}

static void timerInit(void)
{
  int i = 0;
  for (i = 0; i < MAX_CONCURRENT_CONFIG_NODES; i++) {
    createTimer(&caches[i].cache);
  }
  CS_LOG("***Timers*** created\n");
}

static void createTimer(tbcCache_t *tbc)
{
  int ret = 0;
  timer_t *tid = NULL;
  struct sigevent *e = NULL;

  if (caches[tbc->selfId].timerCreated) {
    return;
  }

  tid = &caches[tbc->selfId].timerid;
  e = &caches[tbc->selfId].sev;

  e->sigev_notify = SIGEV_SIGNAL;
  e->sigev_signo = TIMER_SIG;
  e->sigev_value.sival_ptr = tbc;

  if (0 != (ret = timer_create(CLOCK_REALTIME, e, tid))) {
    errExit("timer_create");
  }
  caches[tbc->selfId].timerCreated = 1;
}
