/*************************************************************************
    > File Name: bind_app_key.c
    > Author: Kevin
    > Created Time: 2019-04-28
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

#define BIND_KEY_MSG \
  "Node[%d]: Bind App Key[%d (Ref ID)] to %s Model[%04x:%04x]\n"
#define BIND_KEY_SUC_MSG \
  "Node[%d]:  --- Bind App Key[%d (Ref ID)] to %s Model[%04x:%04x] " HL_SUCCESS "\n"
#define BIND_KEY_FAIL_MSG                                            \
  "Node[%d]:  --- Bind App Key[%d (Ref ID)] to %s Model[%04x:%04x] " \
  HL_FAILED ", Err <0x%04x>\n"

#define BIND_APP_KEY_RETRY_TIMES 5

#define ELEMENT_ITERATOR_INDEX  0
#define MODEL_ITERATOR_INDEX  1
#define APP_KEY_ITERATOR_INDEX  2

#if 1
#define ONCE_P(cache, config)
#else
#define ONCE_P(cache, config)                                                 \
  do {                                                                        \
    CS_LOG(BIND_KEY_MSG,                                                      \
           cache->nodeIndex,                                                  \
           config->keys.appKeys[cache->iterators[APP_KEY_ITERATOR_INDEX]].id, \
           cache->vendorId == SIG_VENDOR_ID ? "SIG" : "Vendor",               \
           cache->vendorId,                                                   \
           cache->modelId);                                                   \
  } while (0)
#endif

#define SUC_P(cache, config)                                                  \
  do {                                                                        \
    CS_LOG(BIND_KEY_SUC_MSG,                                                  \
           cache->nodeIndex,                                                  \
           config->keys.appKeys[cache->iterators[APP_KEY_ITERATOR_INDEX]].id, \
           cache->vendorId == SIG_VENDOR_ID ? "SIG" : "Vendor",               \
           cache->vendorId,                                                   \
           cache->modelId);                                                   \
  } while (0)

#define FAIL_P(cache, config, err)                                            \
  do {                                                                        \
    CS_ERR(BIND_KEY_FAIL_MSG,                                                 \
           cache->nodeIndex,                                                  \
           config->keys.appKeys[cache->iterators[APP_KEY_ITERATOR_INDEX]].id, \
           cache->vendorId == SIG_VENDOR_ID ? "SIG" : "Vendor",               \
           cache->vendorId,                                                   \
           cache->modelId,                                                    \
           err);                                                              \
  } while (0)

/* Global Variables *************************************************** */
extern const char *stateNames[];

static const uint32_t events[] = {
  gecko_evt_mesh_config_client_binding_status_id
};

#define RELATE_EVENTS_NUM() (sizeof(events) / sizeof(uint32_t))
/* Static Variables *************************************************** */

/* Static Functions Declaractions ************************************* */
static int iterateBindAppKeys(tbcCache_t *tbc, networkConfig_t *pconfig);

static int bindAppKeyOnce(tbcCache_t *tbc, networkConfig_t *pconfig)
{
  uint16_t key_id = 0;
  struct gecko_msg_mesh_config_client_bind_model_rsp_t *rsp;

  tbc->vendorId =
    tbc->iterators[MODEL_ITERATOR_INDEX] >= tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].numS
    ? tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].pVd[tbc->iterators[MODEL_ITERATOR_INDEX] - tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].numS]
    : 0xFFFF;
  tbc->modelId =
    tbc->iterators[MODEL_ITERATOR_INDEX] >= tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].numS
    ? tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].pVm[tbc->iterators[MODEL_ITERATOR_INDEX] - tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].numS]
    : tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].pSm[tbc->iterators[MODEL_ITERATOR_INDEX]];

  hardASSERT(
    E_SUC == getAppKeyIdByDummyId(
      pconfig,
      pconfig->pNodes[tbc->nodeIndex].bindings[tbc->iterators[APP_KEY_ITERATOR_INDEX]],
      &key_id));

  rsp = gecko_cmd_mesh_config_client_bind_model(
    getNetworkIdByNodeId(tbc->nodeIndex),
    pconfig->pNodes[tbc->nodeIndex].uniAddr,
    tbc->iterators[ELEMENT_ITERATOR_INDEX],
    key_id,
    tbc->vendorId,
    tbc->modelId
    );

  if (rsp->result != bg_err_success) {
    if (rsp->result == bg_err_out_of_memory) {
      OOM_SET(tbc);
      return E_OOM;
    }
    FAIL_P(tbc, pconfig, rsp->result);
    err_set_to_end(tbc, rsp->result, bgapi_em);
    CS_LOG("Node[%d]: To <<st_end>> State\n", tbc->nodeIndex);
    return E_BGAPI;
  } else {
    ONCE_P(tbc, pconfig);
    WAIT_RESPONSE_SET(tbc);
    tbc->handle = rsp->handle;
    /* CS_LOG("Node[%d] bind key handle [%d]\n", tbc->nodeIndex, tbc->handle); */
    startTimer(tbc, 1);
  }

  return E_SUC;
}

int bindAppKeyEntryGuard(const void *in)
{
  tbcCache_t *tbc = (tbcCache_t *)in;
  networkConfig_t *pconfig = (networkConfig_t *)tbc->pconfig;

  if (pconfig->pNodes[tbc->nodeIndex].bindingCnt != 0) {
    return true;
  }
  return false;
}

/* Design notes: */
/* 1. Needs to start the alarm in engine */
int bindAppKeyStateEntry(void *in, funcGuard guard)
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

  return bindAppKeyOnce(tbc, pconfig);
}

/* Design notes: */
/* 1. Needs to stop the alarm in engine */
int bindAppKeyStateInprogress(void *in, void *cache)
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
    case gecko_evt_mesh_config_client_binding_status_id:
    {
      WAIT_RESPONSE_CLEAR(tbc);
      switch (e->data.evt_mesh_config_client_binding_status.result) {
        case bg_err_success:
          RETRY_CLEAR(tbc);
          SUC_P(tbc, pconfig);
          break;
        case bg_err_timeout:
          /* bind any retry case here */
          if (!EVER_RETRIED(tbc)) {
            tbc->retry = BIND_APP_KEY_RETRY_TIMES;
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
                 e->data.evt_mesh_config_client_binding_status.result);
          __ERR_P(e->data.evt_mesh_config_client_binding_status.result,
                  tbc,
                  stateNames[tbc->state]);
          err_set_to_end(tbc, bg_err_timeout, bgevent_em);
          CS_LOG("Node[%d]: To <<st_end>> State\n", tbc->nodeIndex);
          return E_SUC;
      }

      if (iterateBindAppKeys(tbc, pconfig) == 1) {
        tbc->nextState = -1;
        return E_SUC;
      }

      return bindAppKeyOnce(tbc, pconfig);
    }
    break;

    default:
      accDEBUG("Unexpected event [0x%08x] happend in %s state.\n",
               evtId,
               stateNames[tbc->state]);
      return E_UNSPEC;
  }

  return E_SUC;
}

int bindAppKeyStateRetry(void *p, int reason)
{
  int ret = 0;
  hardASSERT(p);
  tbcCache_t *tbc = (tbcCache_t *)p;
  networkConfig_t *pconfig = (networkConfig_t *)tbc->pconfig;
  hardASSERT(pconfig);
  hardASSERT(reason < retry_on_max_em);

  ret = bindAppKeyOnce(tbc, pconfig);

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

int bindAppKeyStateExit(void *p)
{
  return E_SUC;
}

int isBindAppKeyRelatedPacket(uint32_t evtId)
{
  int i;
  for (i = 0; i < RELATE_EVENTS_NUM(); i++) {
    if (BGLIB_MSG_ID(evtId) == events[i]) {
      return 1;
    }
  }
  return 0;
}

static int iterateBindAppKeys(tbcCache_t *tbc, networkConfig_t *pconfig)
{
  while (++tbc->iterators[APP_KEY_ITERATOR_INDEX]
         != pconfig->pNodes[tbc->nodeIndex].bindingCnt) {
    if (E_SUC == getAppKeyIdByDummyId(
          pconfig,
          pconfig->pNodes[tbc->nodeIndex].bindings[tbc->iterators[APP_KEY_ITERATOR_INDEX]],
          NULL)) {
      break;
    }
  }

  if (tbc->iterators[APP_KEY_ITERATOR_INDEX]
      == pconfig->pNodes[tbc->nodeIndex].bindingCnt) {
    tbc->iterators[APP_KEY_ITERATOR_INDEX] = 0;
    if (++tbc->iterators[MODEL_ITERATOR_INDEX]
        == tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].numS
        + tbc->dcdP0.pElm[tbc->iterators[ELEMENT_ITERATOR_INDEX]].numV) {
      tbc->iterators[MODEL_ITERATOR_INDEX] = 0;
      if (++tbc->iterators[ELEMENT_ITERATOR_INDEX] == tbc->dcdP0.eleCnt) {
        tbc->iterators[ELEMENT_ITERATOR_INDEX] = 0;
        /* All bind appkey is done */
        return 1;
      }
    }
  }
  return 0;
}
