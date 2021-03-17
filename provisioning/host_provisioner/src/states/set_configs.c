/*************************************************************************
    > File Name: set_configs.c
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
#define SET_TTL_MSG \
  "Node[%d]: Set Default TTL to [%d]\n"
#define SET_TTL_SUC_MSG \
  "Node[%d]:  --- Set Default TTL to [%d] " HL_SUCCESS "\n"
#define SET_TTL_FAIL_MSG \
  "Node[%d]:  --- Set Default TTL to [%d] " HL_FAILED ", Err <0x%04x>\n"

#define SET_FRIEND_MSG \
  "Node[%d]: Set Friend [%s]\n"
#define SET_FRIEND_SUC_MSG \
  "Node[%d]:  --- Set Friend [%s] " HL_SUCCESS "\n"
#define SET_FRIEND_FAIL_MSG \
  "Node[%d]:  --- Set Friend [%s] " HL_FAILED ", Err <0x%04x>\n"

#define SET_RELAY_MSG \
  "Node[%d]: Set Relay [%s]\n"
#define SET_RELAY_SUC_MSG \
  "Node[%d]:  --- Set Relay [%s] " HL_SUCCESS "\n"
#define SET_RELAY_FAIL_MSG \
  "Node[%d]:  --- Set Relay [%s] " HL_FAILED ", Err <0x%04x>\n"

#define SET_PROXY_MSG \
  "Node[%d]: Set Proxy [%s]\n"
#define SET_PROXY_SUC_MSG \
  "Node[%d]:  --- Set Proxy [%s] " HL_SUCCESS "\n"
#define SET_PROXY_FAIL_MSG \
  "Node[%d]:  --- Set Proxy [%s] " HL_FAILED ", Err <0x%04x>\n"

#define SET_NETTX_MSG \
  "Node[%d]: Set Network Retransmission to [count-%d : interval-%dms]\n"
#define SET_NETTX_SUC_MSG                                                    \
  "Node[%d]:  --- Set Network Retransmission to [count-%d : interval-%dms] " \
  HL_SUCCESS "\n"
#define SET_NETTX_FAIL_MSG                                                   \
  "Node[%d]:  --- Set Network Retransmission to [count-%d : interval-%dms] " \
  HL_FAILED ", Err <0x%04x>\n"

#define SET_SNB_MSG \
  "Node[%d]: Set Secure Network Beancon %s\n"
#define SET_SNB_SUC_MSG                      \
  "Node[%d]: Set Secure Network Beancon %s " \
  HL_SUCCESS "\n"
#define SET_SNB_FAIL_MSG                     \
  "Node[%d]: Set Secure Network Beancon %s " \
  HL_FAILED ", Err <0x%04x>\n"

#define SET_CONFIGS_RETRY_TIMES 5

#define ELEMENT_ITERATOR_INDEX  0
#define MODEL_ITERATOR_INDEX  1
#define SUB_ADDR_ITERATOR_INDEX  2

/* Global Variables *************************************************** */
extern const char *stateNames[];

static const uint32_t events[] = {
  gecko_evt_mesh_config_client_default_ttl_status_id,
  gecko_evt_mesh_config_client_gatt_proxy_status_id,
  gecko_evt_mesh_config_client_friend_status_id,
  gecko_evt_mesh_config_client_relay_status_id,
  gecko_evt_mesh_config_client_beacon_status_id,
  gecko_evt_mesh_config_client_network_transmit_status_id
};

#define RELATE_EVENTS_NUM() (sizeof(events) / sizeof(uint32_t))
/* Static Variables *************************************************** */
typedef int (*funcPack)(void *, void *, void *);

/* Static Functions Declaractions ************************************* */
static int getNextConfigItem(tbcCache_t *tbc);
static void set_configs_print_state(int which,
                                    int process,
                                    tbcCache_t *tbc,
                                    networkConfig_t *pconfig,
                                    uint16_t err);

static int setConfigsOnce(tbcCache_t *tbc, networkConfig_t *pconfig)
{
  struct gecko_msg_mesh_config_client_set_network_transmit_rsp_t *ntrsp;
  struct gecko_msg_mesh_config_client_set_relay_rsp_t *rrsp;
  struct gecko_msg_mesh_config_client_set_friend_rsp_t *frsp;
  struct gecko_msg_mesh_config_client_set_gatt_proxy_rsp_t *prsp;
  struct gecko_msg_mesh_config_client_set_default_ttl_rsp_t *trsp;
  struct gecko_msg_mesh_config_client_set_beacon_rsp_t *brsp;
  uint16_t *retval = NULL;
  uint32_t *handle = NULL;
  int which;

  which = getNextConfigItem(tbc);

  switch (which) {
    case relay_em:
      rrsp = gecko_cmd_mesh_config_client_set_relay(
        getNetworkIdByNodeId(tbc->nodeIndex),
        pconfig->pNodes[tbc->nodeIndex].uniAddr,
        IS_BIT_SET(tbc->features, relay_em),
        pconfig->pNodes[tbc->nodeIndex].relayRetransCount,
        pconfig->pNodes[tbc->nodeIndex].relayRetransInterval
        ? pconfig->pNodes[tbc->nodeIndex].relayRetransInterval : 50);
      retval = &rrsp->result;
      handle = &rrsp->handle;
      break;
    case proxy_em:
      prsp = gecko_cmd_mesh_config_client_set_gatt_proxy(
        getNetworkIdByNodeId(tbc->nodeIndex),
        pconfig->pNodes[tbc->nodeIndex].uniAddr,
        IS_BIT_SET(tbc->features, proxy_em));
      retval = &prsp->result;
      handle = &prsp->handle;
      break;
    case friend_em:
      frsp = gecko_cmd_mesh_config_client_set_friend(
        getNetworkIdByNodeId(tbc->nodeIndex),
        pconfig->pNodes[tbc->nodeIndex].uniAddr,
        IS_BIT_SET(tbc->features, friend_em));
      retval = &frsp->result;
      handle = &frsp->handle;
      break;
    case ttl_em:
      trsp = gecko_cmd_mesh_config_client_set_default_ttl(
        getNetworkIdByNodeId(tbc->nodeIndex),
        pconfig->pNodes[tbc->nodeIndex].uniAddr,
        pconfig->pNodes[tbc->nodeIndex].ttl);
      retval = &trsp->result;
      handle = &trsp->handle;
      break;
    case net_trans_em:
      ntrsp = gecko_cmd_mesh_config_client_set_network_transmit(
        getNetworkIdByNodeId(tbc->nodeIndex),
        pconfig->pNodes[tbc->nodeIndex].uniAddr,
        pconfig->pNodes[tbc->nodeIndex].transCnt,
        pconfig->pNodes[tbc->nodeIndex].transIntv);
      retval = &ntrsp->result;
      handle = &ntrsp->handle;
      break;
    case secure_netwokr_beacon_em:
      brsp = gecko_cmd_mesh_config_client_set_beacon(
        getNetworkIdByNodeId(tbc->nodeIndex),
        pconfig->pNodes[tbc->nodeIndex].uniAddr,
        IS_BIT_SET(tbc->features, secure_netwokr_beacon_em));
      retval = &brsp->result;
      handle = &brsp->handle;
      break;
    default:
      return E_NFND;
  }

  if (*retval != bg_err_success) {
    if (*retval == bg_err_out_of_memory) {
      OOM_SET(tbc);
      return E_OOM;
    }
    set_configs_print_state(which, failed_em, tbc, pconfig, *retval);
    err_set_to_end(tbc, *retval, bgapi_em);
    CS_LOG("Node[%d]: To <<st_end>> State\n", tbc->nodeIndex);
    return E_BGAPI;
  } else {
    /* TODO: Uncomment below line to print the ONCE_P */
    /* set_configs_print_state(which, once_em, tbc, pconfig, 0); */
    WAIT_RESPONSE_SET(tbc);
    tbc->handle = *handle;
    startTimer(tbc, 1);
  }

  return E_SUC;
}

int setConfigsEntryGuard(const void *in)
{
  tbcCache_t *tbc = (tbcCache_t *)in;

  if ((tbc->features & 0xFFFF0000) != 0) {
    return true;
  }
  return false;
}

static int getNextConfigItem(tbcCache_t *tbc)
{
  int bits = 0;
  bits = ((tbc->features >> 16) ^ tbc->featuresValidBits) & 0x0000FFFF;
  if (!bits) {
    return -1;
  }
  return getLeftFirstOne(bits);
}

int setConfigsStateEntry(void *in, funcGuard guard)
{
  int ret = 0;
  tbcCache_t *tbc = (tbcCache_t *)in;
  networkConfig_t *pconfig = (networkConfig_t *)tbc->pconfig;

  if (guard) {
    if (!guard(in)) {
      CS_MSG("To Next State Since %s Guard Not Passed\n",
             stateNames[tbc->state]);
      return E_TONEXT;
    }
  }

  ret = setConfigsOnce(tbc, pconfig);

  if (ret == E_NFND) {
    tbc->nextState = -1;
    ret = E_SUC;
  }
  return ret;
}

int setConfigsStateInprogress(void *in, void *cache)
{
  int ret = 0, which = -1;
  uint32_t evtId;
  struct gecko_cmd_packet *e = NULL;
  tbcCache_t *tbc = (tbcCache_t *)cache;
  networkConfig_t *pconfig = (networkConfig_t *)tbc->pconfig;
  hardASSERT(pconfig);
  uint16_t *retval = NULL;
  hardASSERT(tbc);
  hardASSERT(in);
  e = in;

  evtId = BGLIB_MSG_ID(e->header);
  startTimer(tbc, 0);
  switch (evtId) {
    case gecko_evt_mesh_config_client_relay_status_id:
    {
      which = relay_em;
      retval = &e->data.evt_mesh_config_client_relay_status.result;
    }
    break;

    case gecko_evt_mesh_config_client_friend_status_id:
    {
      which = friend_em;
      retval = &e->data.evt_mesh_config_client_friend_status.result;
    }
    break;

    case gecko_evt_mesh_config_client_gatt_proxy_status_id:
    {
      which = proxy_em;
      retval = &e->data.evt_mesh_config_client_gatt_proxy_status.result;
    }
    break;

    case gecko_evt_mesh_config_client_default_ttl_status_id:
    {
      which = ttl_em;
      retval = &e->data.evt_mesh_config_client_default_ttl_status.result;
    }
    break;

    case gecko_evt_mesh_config_client_network_transmit_status_id:
    {
      which = net_trans_em;
      retval = &e->data.evt_mesh_config_client_network_transmit_status.result;
    }
    break;

    case gecko_evt_mesh_config_client_beacon_status_id:
    {
      which = secure_netwokr_beacon_em;
      retval = &e->data.evt_mesh_config_client_beacon_status.result;
    }
    break;

    default:
      CS_ERR("Unexpected event [0x%08x] happend in %s state.\n",
             evtId,
             stateNames[tbc->state]);
      return E_UNSPEC;
  }

  WAIT_RESPONSE_CLEAR(tbc);
  switch (*retval) {
    case bg_err_success:
      RETRY_CLEAR(tbc);
      set_configs_print_state(which, success_em, tbc, pconfig, 0);
      BIT_SET(tbc->featuresValidBits, which);
      break;
    case bg_err_timeout:
      /* bind any retry case here */
      if (!EVER_RETRIED(tbc)) {
        tbc->retry = SET_CONFIGS_RETRY_TIMES;
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
      set_configs_print_state(which,
                              failed_em,
                              tbc,
                              pconfig,
                              *retval);
      err_set_to_end(tbc, bg_err_timeout, bgevent_em);
      CS_LOG("Node[%d]: To <<st_end>> State\n", tbc->nodeIndex);
      return E_SUC;
  }

  if (getNextConfigItem(tbc) == -1) {
    tbc->nextState = -1;
    return E_SUC;
  }

  ret = setConfigsOnce(tbc, pconfig);

  if (ret == E_NFND) {
    tbc->nextState = -1;
    ret = E_SUC;
  }

  return ret;
}

int setConfigsStateRetry(void *p, int reason)
{
  int ret = 0;
  hardASSERT(p);
  tbcCache_t *tbc = (tbcCache_t *)p;
  networkConfig_t *pconfig = (networkConfig_t *)tbc->pconfig;
  hardASSERT(pconfig);
  hardASSERT(reason < retry_on_max_em);

  ret = setConfigsOnce(tbc, pconfig);

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

int setConfigsStateExit(void *p)
{
  return E_SUC;
}

int isSetConfigsRelatedPacket(uint32_t evtId)
{
  int i;
  for (i = 0; i < RELATE_EVENTS_NUM(); i++) {
    if (BGLIB_MSG_ID(evtId) == events[i]) {
      return 1;
    }
  }
  return 0;
}

static void set_configs_print_state(int which,
                                    int process,
                                    tbcCache_t *tbc,
                                    networkConfig_t *pconfig,
                                    uint16_t err)
{
  switch (which) {
    case relay_em:
      switch (process) {
        case once_em:
          CS_LOG(SET_RELAY_MSG,
                 tbc->nodeIndex,
                 IS_BIT_SET(tbc->features, relay_em) ? "ON" : "OFF");
          break;
        case success_em:
          CS_LOG(SET_RELAY_SUC_MSG,
                 tbc->nodeIndex,
                 IS_BIT_SET(tbc->features, relay_em) ? "ON" : "OFF");
          break;
        case failed_em:
          CS_LOG(SET_RELAY_FAIL_MSG,
                 tbc->nodeIndex,
                 IS_BIT_SET(tbc->features, relay_em) ? "ON" : "OFF",
                 err);
          break;
        default:
          return;
      }
      break;
    case proxy_em:
      switch (process) {
        case once_em:
          CS_LOG(SET_PROXY_MSG,
                 tbc->nodeIndex,
                 IS_BIT_SET(tbc->features, proxy_em) ? "ON" : "OFF");
          break;
        case success_em:
          CS_LOG(SET_PROXY_SUC_MSG,
                 tbc->nodeIndex,
                 IS_BIT_SET(tbc->features, proxy_em) ? "ON" : "OFF");
          break;
        case failed_em:
          CS_LOG(SET_PROXY_FAIL_MSG,
                 tbc->nodeIndex,
                 IS_BIT_SET(tbc->features, proxy_em) ? "ON" : "OFF",
                 err);
          break;
        default:
          return;
      }
      break;
    case friend_em:
      switch (process) {
        case once_em:
          CS_LOG(SET_FRIEND_MSG,
                 tbc->nodeIndex,
                 IS_BIT_SET(tbc->features, friend_em) ? "ON" : "OFF");
          break;
        case success_em:
          CS_LOG(SET_FRIEND_SUC_MSG,
                 tbc->nodeIndex,
                 IS_BIT_SET(tbc->features, friend_em) ? "ON" : "OFF");
          break;
        case failed_em:
          CS_LOG(SET_FRIEND_FAIL_MSG,
                 tbc->nodeIndex,
                 IS_BIT_SET(tbc->features, friend_em) ? "ON" : "OFF",
                 err);
          break;
        default:
          return;
      }
      break;
    case ttl_em:
      switch (process) {
        case once_em:
          CS_LOG(SET_TTL_MSG,
                 tbc->nodeIndex,
                 pconfig->pNodes[tbc->nodeIndex].ttl);
          break;
        case success_em:
          CS_LOG(SET_TTL_SUC_MSG,
                 tbc->nodeIndex,
                 pconfig->pNodes[tbc->nodeIndex].ttl);
          break;
        case failed_em:
          CS_LOG(SET_TTL_FAIL_MSG,
                 tbc->nodeIndex,
                 pconfig->pNodes[tbc->nodeIndex].ttl,
                 err);
          break;
        default:
          return;
      }
      break;
    case net_trans_em:
      switch (process) {
        case once_em:
          CS_LOG(SET_NETTX_MSG,
                 tbc->nodeIndex,
                 pconfig->pNodes[tbc->nodeIndex].transCnt,
                 pconfig->pNodes[tbc->nodeIndex].transIntv);
          break;
        case success_em:
          CS_LOG(SET_NETTX_SUC_MSG,
                 tbc->nodeIndex,
                 pconfig->pNodes[tbc->nodeIndex].transCnt,
                 pconfig->pNodes[tbc->nodeIndex].transIntv);
          break;
        case failed_em:
          CS_LOG(SET_NETTX_FAIL_MSG,
                 tbc->nodeIndex,
                 pconfig->pNodes[tbc->nodeIndex].transCnt,
                 pconfig->pNodes[tbc->nodeIndex].transIntv,
                 err);
          break;
        default:
          return;
      }
      break;
    case secure_netwokr_beacon_em:
      switch (process) {
        case once_em:
          CS_LOG(SET_SNB_MSG,
                 tbc->nodeIndex,
                 IS_BIT_SET(tbc->features, secure_netwokr_beacon_em) ? "ON" : "OFF");
          break;
        case success_em:
          CS_LOG(SET_SNB_SUC_MSG,
                 tbc->nodeIndex,
                 IS_BIT_SET(tbc->features, secure_netwokr_beacon_em) ? "ON" : "OFF");
          break;
        case failed_em:
          CS_LOG(SET_SNB_FAIL_MSG,
                 tbc->nodeIndex,
                 IS_BIT_SET(tbc->features, secure_netwokr_beacon_em) ? "ON" : "OFF",
                 err);
          break;
        default:
          return;
      }
      break;
    default:
      return;
  }
}
