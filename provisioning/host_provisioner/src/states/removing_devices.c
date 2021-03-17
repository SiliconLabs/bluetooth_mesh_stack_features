/*************************************************************************
    > File Name: removing_devices.c
    > Author: Kevin
    > Created Time: 2019-05-05
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

#define REMOVE_NODE_MSG \
  "Node[%d]: Removing Node with Address[0x%04x]\n"
#define REMOVE_NODE_SUC_MSG                       \
  "Node[%d]: Removing Node with Address[0x%04x] " \
  HL_SUCCESS "\n"
#define REMOVE_NODE_FAIL_MSG                      \
  "Node[%d]: Removing Node with Address[0x%04x] " \
  HL_FAILED ", Err <0x%04x>\n"

#define REMOVE_NODE_RETRY_TIMES 3

#define ELEMENT_ITERATOR_INDEX  0
#define MODEL_ITERATOR_INDEX  1
#define SUB_ADDR_ITERATOR_INDEX  2

#if 1
#define ONCE_P(cache, config)
#else
#define ONCE_P(cache, config)                    \
  do {                                           \
    CS_LOG(                                      \
      REMOVE_NODE_MSG,                           \
      cache->nodeIndex,                          \
      config->pNodes[cache->nodeIndex].uniAddr); \
  } while (0)
#endif

#define SUC_P(cache, config)                     \
  do {                                           \
    CS_LOG(                                      \
      REMOVE_NODE_SUC_MSG,                       \
      cache->nodeIndex,                          \
      config->pNodes[cache->nodeIndex].uniAddr); \
  } while (0)

#define FAIL_P(cache, config, err)              \
  do {                                          \
    CS_LOG(                                     \
      REMOVE_NODE_FAIL_MSG,                     \
      cache->nodeIndex,                         \
      config->pNodes[cache->nodeIndex].uniAddr, \
      err);                                     \
  } while (0)

/* Global Variables *************************************************** */
extern const char *stateNames[];

static const uint32_t events[] = {
  gecko_evt_mesh_config_client_reset_status_id
};

#define RELATE_EVENTS_NUM() (sizeof(events) / sizeof(uint32_t))
/* Static Variables *************************************************** */
typedef int (*funcPack)(void *, void *, void *);

/* Static Functions Declaractions ************************************* */

static int removeNodeOnce(tbcCache_t *tbc, networkConfig_t *pconfig)
{
  struct gecko_msg_mesh_config_client_reset_node_rsp_t *rsp;

  /* First one, should set */
  rsp = gecko_cmd_mesh_config_client_reset_node(
    getNetworkIdByNodeId(tbc->nodeIndex),
    pconfig->pNodes[tbc->nodeIndex].uniAddr
    );

  if (rsp->result != bg_err_success) {
    if (rsp->result == bg_err_out_of_memory) {
      OOM_SET(tbc);
      return E_OOM;
    }
    FAIL_P(tbc, pconfig, rsp->result);
    err_set_to_rm_end(tbc, rsp->result, bgapi_em);
    CS_LOG("Node[%d]: To <<END>> State\n", tbc->nodeIndex);
    return E_BGAPI;
  } else {
    ONCE_P(tbc, pconfig);
    WAIT_RESPONSE_SET(tbc);
    tbc->handle = rsp->handle;
    /* CS_LOG("Node[%d] add sub handle [%d]\n", tbc->nodeIndex, tbc->handle); */
    startTimer(tbc, 1);
  }

  return E_SUC;
}

int removeNodeEntryGuard(const void *in)
{
  tbcCache_t *tbc = (tbcCache_t *)in;
  networkConfig_t *pconfig = (networkConfig_t *)tbc->pconfig;

  if (pconfig->pNodes[tbc->nodeIndex].uniAddr != UNASSIGNED_ADDRESS) {
    return true;
  }
  return false;
}

/* Design notes: */
/* 1. Needs to start the alarm in engine */
int removeNodeStateEntry(void *in, funcGuard guard)
{
  /* Alarm SHOULD be set in the main engine */
  tbcCache_t *tbc = (tbcCache_t *)in;
  networkConfig_t *pconfig = (networkConfig_t *)tbc->pconfig;

  if (guard) {
    if (!guard(in)) {
      CS_MSG("To Next State Since %s Guard Not Passed\n",
             stateNames[tbc->state]);
      /* err_set_to_rm_end(tbc, 0, bg_err_invalid_em); */
      /* CS_MSG("Node[%d]: To <<st_end>> State\n", tbc->nodeIndex); */
      /* tbc->nextState = -1; */
      return E_TONEXT;
    }
  }

  return removeNodeOnce(tbc, pconfig);
}

/* Design notes: */
/* 1. Needs to stop the alarm in engine */
int removeNodeStateInprogress(void *in, void *cache)
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
    case gecko_evt_mesh_config_client_reset_status_id:
      WAIT_RESPONSE_CLEAR(tbc);
      switch (e->data.evt_mesh_config_client_reset_status.result) {
        case bg_err_success:
          RETRY_CLEAR(tbc);
          SUC_P(tbc, pconfig);
          tbc->nextState = -1;
          break;
        case bg_err_timeout:
          /* bind any retry case here */
          if (!EVER_RETRIED(tbc)) {
            tbc->retry = REMOVE_NODE_RETRY_TIMES;
            EVER_RETRIED_SET(tbc);
          } else if (tbc->retry <= 0) {
#if (BETTER_SOLUTION == 1)
            RETRY_CLEAR(tbc);
            RETRY_OUT_PRINT(tbc);
            err_set_to_rm_end(tbc, bg_err_timeout, bgevent_em);
            CS_LOG("Node[%d]: To <<st_end>> State\n", tbc->nodeIndex);
#else
            RETRY_CLEAR(tbc);
            RETRY_OUT_PRINT(tbc);
            tbc->nextState = -1;
#endif
          }
          break;
        default:
          FAIL_P(tbc,
                 pconfig,
                 e->data.evt_mesh_config_client_reset_status.result);
          __ERR_P(e->data.evt_mesh_config_client_reset_status.result,
                  tbc,
                  stateNames[tbc->state]);
          err_set_to_rm_end(tbc, bg_err_timeout, bgevent_em);
          CS_LOG("Node[%d]: To <<END>> State\n", tbc->nodeIndex);
          break;
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

int removeNodeStateRetry(void *p, int reason)
{
  int ret = 0;
  hardASSERT(p);
  tbcCache_t *tbc = (tbcCache_t *)p;
  networkConfig_t *pconfig = (networkConfig_t *)tbc->pconfig;
  hardASSERT(pconfig);
  hardASSERT(reason < retry_on_max_em);

  ret = removeNodeOnce(tbc, pconfig);

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

int removeNodeStateExit(void *p)
{
  return E_SUC;
}

int isRemoveNodeRelatedPacket(uint32_t evtId)
{
  int i;
  for (i = 0; i < RELATE_EVENTS_NUM(); i++) {
    if (BGLIB_MSG_ID(evtId) == events[i]) {
      return 1;
    }
  }
  return 0;
}
