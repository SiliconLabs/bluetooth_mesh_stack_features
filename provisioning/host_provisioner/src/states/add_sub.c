/*************************************************************************
    > File Name: add_sub.c
    > Author: Kevin
    > Created Time: 2019-04-29
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include "async/async_config_client.h"

#include "utils.h"

#include "prov_assert.h"
#include "conf_generic_parser.h"
#include "network_manage.h"

#include "generic_main.h" /* TODO - rework to remove getnetwork key */
/* Defines  *********************************************************** */
#define SIG_VENDOR_ID 0xffff

#define ADD_SUB_MSG \
  "Node[%d]: Add Address[0x%04x] to Model[%04x:%04x] Subscription List on Element[%d]\n"
#define ADD_SUB_SUC_MSG                                                                      \
  "Node[%d]:  --- Add Address[0x%04x] to Model[%04x:%04x] Subscription List on Element[%d] " \
  HL_SUCCESS "\n"
#define ADD_SUB_FAIL_MSG                                                                     \
  "Node[%d]:  --- Add Address[0x%04x] to Model[%04x:%04x] Subscription List on Element[%d] " \
  HL_FAILED ", Err <0x%04x>\n"

#define ADD_SUB_RETRY_TIMES 5

#define ELEMENT_ITERATOR_INDEX  0
#define MODEL_ITERATOR_INDEX  1
#define SUB_ADDR_ITERATOR_INDEX  2

#if 1
#define ONCE_P(cache, config)
#else
#define ONCE_P(cache, config)                                                               \
  do {                                                                                      \
    CS_LOG(                                                                                 \
      ADD_SUB_MSG,                                                                          \
      cache->nodeIndex,                                                                     \
      config->pNodes[cache->nodeIndex].subAddrs[cache->iterators[SUB_ADDR_ITERATOR_INDEX]], \
      cache->vendorId,                                                                      \
      cache->modelId,                                                                       \
      cache->iterators[ELEMENT_ITERATOR_INDEX]);                                            \
  } while (0)
#endif

#define SUC_P(cache, config)                                                                \
  do {                                                                                      \
    CS_LOG(                                                                                 \
      ADD_SUB_SUC_MSG,                                                                      \
      cache->nodeIndex,                                                                     \
      config->pNodes[cache->nodeIndex].subAddrs[cache->iterators[SUB_ADDR_ITERATOR_INDEX]], \
      cache->vendorId,                                                                      \
      cache->modelId,                                                                       \
      cache->iterators[ELEMENT_ITERATOR_INDEX]);                                            \
  } while (0)

#define FAIL_P(cache, config, err)                                                          \
  do {                                                                                      \
    CS_LOG(                                                                                 \
      ADD_SUB_FAIL_MSG,                                                                     \
      cache->nodeIndex,                                                                     \
      config->pNodes[cache->nodeIndex].subAddrs[cache->iterators[SUB_ADDR_ITERATOR_INDEX]], \
      cache->vendorId,                                                                      \
      cache->modelId,                                                                       \
      cache->iterators[ELEMENT_ITERATOR_INDEX],                                             \
      err);                                                                                 \
  } while (0)

/* Global Variables *************************************************** */
extern const char *stateNames[];

static const uint32_t events[] = {
  gecko_evt_mesh_config_client_model_sub_status_id
};

#define RELATE_EVENTS_NUM() (sizeof(events) / sizeof(uint32_t))
/* Static Variables *************************************************** */
typedef int (*funcPack)(void *, void *, void *);

/* Static Functions Declaractions ************************************* */
static int iterateAddSub(tbcCache_t *tbc, networkConfig_t *pconfig);

static int addSubOnce(tbcCache_t *tbc, networkConfig_t *pconfig)
{
  struct gecko_msg_mesh_config_client_add_model_sub_rsp_t *arsp;
  struct gecko_msg_mesh_config_client_set_model_sub_rsp_t *srsp;
  uint16_t *retval = NULL;
  uint32_t *handle = NULL;

  if (tbc->iterators[SUB_ADDR_ITERATOR_INDEX] == 0) {
    tbc->vendorId =
      tbc->iterators[MODEL_ITERATOR_INDEX] >= tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].numS
      ? tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].pVd[tbc->iterators[MODEL_ITERATOR_INDEX] - tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].numS]
      : 0xFFFF;
    tbc->modelId =
      tbc->iterators[MODEL_ITERATOR_INDEX] >= tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].numS
      ? tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].pVm[tbc->iterators[MODEL_ITERATOR_INDEX] - tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].numS]
      : tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].pSm[tbc->iterators[MODEL_ITERATOR_INDEX]];

    srsp = gecko_cmd_mesh_config_client_set_model_sub(
      getNetworkIdByNodeId(tbc->nodeIndex),
      pconfig->pNodes[tbc->nodeIndex].uniAddr,
      tbc->iterators[ELEMENT_ITERATOR_INDEX],
      tbc->vendorId,
      tbc->modelId,
      pconfig->pNodes[tbc->nodeIndex].subAddrs[tbc->iterators[SUB_ADDR_ITERATOR_INDEX]]);
    retval = &srsp->result;
    handle = &srsp->handle;
  } else {
    tbc->vendorId =
      tbc->iterators[MODEL_ITERATOR_INDEX] >= tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].numS
      ? tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].pVd[tbc->iterators[MODEL_ITERATOR_INDEX] - tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].numS]
      : 0xFFFF;
    tbc->modelId =
      tbc->iterators[MODEL_ITERATOR_INDEX] >= tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].numS
      ? tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].pVm[tbc->iterators[MODEL_ITERATOR_INDEX] - tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].numS]
      : tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].pSm[tbc->iterators[MODEL_ITERATOR_INDEX]];
    arsp = gecko_cmd_mesh_config_client_add_model_sub(
      getNetworkIdByNodeId(tbc->nodeIndex),
      pconfig->pNodes[tbc->nodeIndex].uniAddr,
      tbc->iterators[ELEMENT_ITERATOR_INDEX],
      tbc->vendorId,
      tbc->modelId,
      pconfig->pNodes[tbc->nodeIndex].subAddrs[tbc->iterators[SUB_ADDR_ITERATOR_INDEX]]);
    retval = &arsp->result;
    handle = &arsp->handle;
  }

  if (*retval != bg_err_success) {
    if (*retval == bg_err_out_of_memory) {
      OOM_SET(tbc);
      return E_OOM;
    }
    FAIL_P(tbc, pconfig, *retval);
    err_set_to_end(tbc, *retval, bgapi_em);
    CS_LOG("Node[%d]: To <<st_end>> State\n", tbc->nodeIndex);
    return E_BGAPI;
  } else {
    ONCE_P(tbc, pconfig);
    WAIT_RESPONSE_SET(tbc);
    tbc->handle = *handle;
    /* CS_LOG("Node[%d] add sub handle [%d]\n", tbc->nodeIndex, tbc->handle); */
    startTimer(tbc, 1);
  }

  return E_SUC;
}

int addSubEntryGuard(const void *in)
{
  tbcCache_t *tbc = (tbcCache_t *)in;
  networkConfig_t *pconfig = (networkConfig_t *)tbc->pconfig;

  if (pconfig->pNodes[tbc->nodeIndex].subCnt != 0) {
    return true;
  }
  return false;
}

/* Design notes: */
/* 1. Needs to start the alarm in engine */
int addSubStateEntry(void *in, funcGuard guard)
{
  /* Alarm SHOULD be set in the main engine */
  tbcCache_t *tbc = (tbcCache_t *)in;
  networkConfig_t *pconfig = (networkConfig_t *)tbc->pconfig;

  if (guard) {
    if (!guard(in)) {
      CS_MSG("To Next State Since %s Guard Not Passed\n",
             stateNames[tbc->state]);
      /* err_set_to_end(tbc, 0, bg_err_invalid_em); */
      /* CS_MSG("Node[%d]: To <<st_end>> State\n", tbc->nodeIndex); */
      /* tbc->nextState = -1; */
      return E_TONEXT;
    }
  }

  return addSubOnce(tbc, pconfig);
}

/* Design notes: */
/* 1. Needs to stop the alarm in engine */
int addSubStateInprogress(void *in, void *cache)
{
  uint32_t evtId;
  struct gecko_cmd_packet *e = NULL;
  tbcCache_t *tbc = (tbcCache_t *)cache;
  networkConfig_t *pconfig = (networkConfig_t *)tbc->pconfig;

  hardASSERT(pconfig);
  hardASSERT(tbc);
  hardASSERT(in);
  e = in;

  evtId = BGLIB_MSG_ID(e->header);
  startTimer(tbc, 0);
  switch (evtId) {
    case gecko_evt_mesh_config_client_model_sub_status_id:
    {
      WAIT_RESPONSE_CLEAR(tbc);
      switch (e->data.evt_mesh_config_client_model_sub_status.result) {
        case bg_err_success:
          RETRY_CLEAR(tbc);
          SUC_P(tbc, pconfig);
          break;
        case bg_err_timeout:
          /* bind any retry case here */
          if (!EVER_RETRIED(tbc)) {
            tbc->retry = ADD_SUB_RETRY_TIMES;
            EVER_RETRIED_SET(tbc);
          } else if (tbc->retry <= 0) {
            RETRY_CLEAR(tbc);
            RETRY_OUT_PRINT(tbc);
            err_set_to_end(tbc, bg_err_timeout, bgevent_em);
            CS_LOG("Node[%d]: To <<st_end>> State\n", tbc->nodeIndex);
          }
          return E_SUC;
          break;
        default:
          FAIL_P(tbc,
                 pconfig,
                 e->data.evt_mesh_config_client_model_sub_status.result);
          __ERR_P(e->data.evt_mesh_config_client_model_sub_status.result,
                  tbc,
                  stateNames[tbc->state]);
          err_set_to_end(tbc, bg_err_timeout, bgevent_em);
          CS_LOG("Node[%d]: To <<st_end>> State\n", tbc->nodeIndex);
          return E_SUC;
      }

      if (iterateAddSub(tbc, pconfig) == 1) {
        tbc->nextState = -1;
        return E_SUC;
      }

      return addSubOnce(tbc, pconfig);
    }
    break;

    default:
      CS_ERR("Unexpected event [0x%08x] happend in %s state.\n",
             evtId,
             stateNames[tbc->state]);
      return E_UNSPEC;
  }

  return E_SUC;
}

int addSubStateRetry(void *p, int reason)
{
  int ret = 0;
  hardASSERT(p);
  tbcCache_t *tbc = (tbcCache_t *)p;
  networkConfig_t *pconfig = (networkConfig_t *)tbc->pconfig;
  hardASSERT(pconfig);
  hardASSERT(reason < retry_on_max_em);

  ret = addSubOnce(tbc, pconfig);

  if (ret == E_SUC) {
    switch (reason) {
      case on_timeout_em:
        if (!EVER_RETRIED(tbc) || tbc->retry-- <= 0) {
          hardASSERT(0);
        }
        RETRY_ONCE_PRINT(tbc);
        break;
      case on_oom_em:
        hardASSERT(OOM(tbc));
        OOM_ONCE_PRINT(tbc);
        OOM_CLEAR(tbc);
        break;
      case on_guard_timer_expired_em:
        hardASSERT(tbc->expired);
        EXPIRED_ONCE_PRINT(tbc);
        tbc->expired = 0;
        break;
    }
  }
  return ret;
}

int addSubStateExit(void *p)
{
  return E_SUC;
}

int isAddSubRelatedPacket(uint32_t evtId)
{
  int i;
  for (i = 0; i < RELATE_EVENTS_NUM(); i++) {
    if (BGLIB_MSG_ID(evtId) == events[i]) {
      return 1;
    }
  }
  return 0;
}

static int iterateAddSub(tbcCache_t *tbc, networkConfig_t *pconfig)
{
  if (++tbc->iterators[SUB_ADDR_ITERATOR_INDEX]
      == pconfig->pNodes[tbc->nodeIndex].subCnt) {
    tbc->iterators[SUB_ADDR_ITERATOR_INDEX] = 0;
    if (++tbc->iterators[MODEL_ITERATOR_INDEX]
        == tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].numS
        + tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].numV) {
      tbc->iterators[MODEL_ITERATOR_INDEX] = 0;
      if (++tbc->iterators[ELEMENT_ITERATOR_INDEX] == tbc->dcdP0.eleCnt) {
        tbc->iterators[ELEMENT_ITERATOR_INDEX] = 0;
        return 1;
      }
    }
  }
  return 0;
}
