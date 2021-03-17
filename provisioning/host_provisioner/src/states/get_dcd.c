/*************************************************************************
    > File Name: get_dcd.c
    > Author: Kevin
    > Created Time: 2019-04-18
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
#define GENERIC_ONOFF_SERVER_MDID       0x1000
#define GENERIC_ONOFF_CLIENT_MDID       0x1001

#define CONFIGURATION_SERVER_MDID       0x0000
#define CONFIGURATION_CLIENT_MDID       0x0001
#define HEALTH_SERVER_MDID              0x0002
#define HEALTH_CLIENT_MDID              0x0003

#define GET_DCD_MSG                     "Node[%d]: Get DCD\n"
#define GET_DCD_SUC_MSG                 "Node[%d]:  --- Get DCD " HL_SUCCESS "\n"
#define GET_DCD_FAIL_MSG                "Node[%d]:  --- Get DCD " HL_FAILED ", Err <0x%04x>\n"

#define GET_DCD_RETRY_TIMES             5

#if 1
#define ONCE_P(cache, config)
#else
#define ONCE_P(cache, config)              \
  do {                                     \
    CS_LOG(GET_DCD_MSG, cache->nodeIndex); \
  } while (0)
#endif

#define SUC_P(cache, config)                   \
  do {                                         \
    CS_LOG(GET_DCD_SUC_MSG, cache->nodeIndex); \
  } while (0)

#define FAIL_P(cache, config, err) \
  do {                             \
    CS_ERR(GET_DCD_FAIL_MSG,       \
           cache->nodeIndex,       \
           err);                   \
  } while (0)

/* Global Variables *************************************************** */
extern const char *stateNames[];

static const uint32_t events[] = {
  gecko_evt_mesh_config_client_dcd_data_id,
  gecko_evt_mesh_config_client_dcd_data_end_id
};

#define RELATE_EVENTS_NUM() (sizeof(events) / sizeof(uint32_t))
/* Static Variables *************************************************** */

/* Static Functions Declaractions ************************************* */
static void storeDCD(const uint8_t *data,
                     uint8_t len,
                     dcd_t *pD);

static int getDcdOnce(tbcCache_t *tbc, networkConfig_t *pconfig)
{
  struct gecko_msg_mesh_config_client_get_dcd_rsp_t *rsp;

  rsp = gecko_cmd_mesh_config_client_get_dcd(
    getNetworkIdByNodeId(tbc->nodeIndex),
    pconfig->pNodes[tbc->nodeIndex].uniAddr,
    0);

  if (rsp->result != bg_err_success) {
    if (rsp->result == bg_err_out_of_memory) {
      OOM_SET(tbc);
      return E_OOM;
    }
    FAIL_P(tbc, pconfig, rsp->result);
    err_set_to_end(tbc, rsp->result, bgapi_em);
    CS_ERR("Node[%d]: To <<st_end>> State\n", tbc->nodeIndex);
    return E_BGAPI;
  } else {
    ONCE_P(tbc, pconfig);
    WAIT_RESPONSE_SET(tbc);
    tbc->handle = rsp->handle;
    startTimer(tbc, 1);
  }

  return E_SUC;
}

int getDcdEntryGuard(const void *in)
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
int getDcdStateEntry(void *in, funcGuard guard)
{
  /* Alarm SHOULD be set in the main engine */
  tbcCache_t *tbc = (tbcCache_t *)in;
  networkConfig_t *pconfig = (networkConfig_t *)tbc->pconfig;

  if (guard) {
    if (!guard(in)) {
      CS_MSG("To Next State Since %s Guard Not Passed\n",
             stateNames[tbc->state]);
      /* err_set_to_end(tbc, 0, bg_err_invalid_em); */
      /* CS_ERR("Node[%d]: To <<st_end>> State\n", tbc->nodeIndex); */
      /* tbc->nextState = -1; */
      return E_TONEXT;
    }
  }

  return getDcdOnce(tbc, pconfig);
}

/* Design notes: */
/* 1. Needs to stop the alarm in engine */
int getDcdStateInprogress(void *in, void *cache)
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
    case gecko_evt_mesh_config_client_dcd_data_id:
    {
      /* Ignore pages other than 0 for now */
      if (e->data.evt_mesh_config_client_dcd_data.page == 0) {
        accDEBUG("Node[%d]: DCD Page 0 Received\n", tbc->nodeIndex);
        storeDCD(e->data.evt_mesh_config_client_dcd_data.data.data,
                 e->data.evt_mesh_config_client_dcd_data.data.len,
                 &tbc->dcdP0);
      }
    }
    break;

    case gecko_evt_mesh_config_client_dcd_data_end_id:
    {
      WAIT_RESPONSE_CLEAR(tbc);
      switch (e->data.evt_mesh_config_client_dcd_data_end.result) {
        case bg_err_success:
          RETRY_CLEAR(tbc);
          SUC_P(tbc, pconfig);
          if (pconfig->pNodes[tbc->nodeIndex].errBits > ERROR_BIT(getDcd_em)
              && pconfig->pNodes[tbc->nodeIndex].errBits > ERROR_BIT(end_em)) {
            for (int a = addAppKey_em; a < end_em; a++) {
              if (pconfig->pNodes[tbc->nodeIndex].errBits & ERROR_BIT(a)) {
                CS_LOG("Node[%d]: Configure the node from <<<%s>>> state\n",
                       tbc->nodeIndex,
                       stateNames[a]);
                tbc->nextState = a;
              }
            }
          } else {
            tbc->nextState = -1;
          }
          break;
        case bg_err_timeout:
          /* add any retry case here */
          if (!EVER_RETRIED(tbc)) {
            tbc->retry = GET_DCD_RETRY_TIMES;
            EVER_RETRIED_SET(tbc);
          } else if (tbc->retry <= 0) {
            RETRY_CLEAR(tbc);
            RETRY_OUT_PRINT(tbc);
            err_set_to_end(tbc, bg_err_timeout, bgevent_em);
            CS_LOG("Node[%d]: To <<st_end>> State\n", tbc->nodeIndex);
          }
          break;
        default:
          FAIL_P(tbc,
                 pconfig,
                 e->data.evt_mesh_config_client_dcd_data_end.result);
          __ERR_P(e->data.evt_mesh_config_client_dcd_data_end.result,
                  tbc,
                  stateNames[tbc->state]);
          err_set_to_end(tbc, bg_err_timeout, bgevent_em);
          accDEBUG("To <<st_end>> State\n");
          break;
      }
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

int getDcdStateRetry(void *p, int reason)
{
  int ret = 0;
  hardASSERT(p);
  tbcCache_t *tbc = (tbcCache_t *)p;
  networkConfig_t *pconfig = (networkConfig_t *)tbc->pconfig;
  hardASSERT(pconfig);
  hardASSERT(reason < retry_on_max_em);

  ret = getDcdOnce(tbc, pconfig);

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

int getDcdStateExit(void *p)
{
  return E_SUC;
}

int isGetDcdRelatedPacket(uint32_t evtId)
{
  int i;
  for (i = 0; i < RELATE_EVENTS_NUM(); i++) {
    if (BGLIB_MSG_ID(evtId) == events[i]) {
      return 1;
    }
  }
  return 0;
}

static void storeDCD(const uint8_t *data,
                     uint8_t len,
                     dcd_t *pD)
{
  uint8_t i = 0, eles = 0, numS = 0, numV = 0;
  i = 12;
  numS = data[i];
  numV = data[i + 1];
  i += 2;
  do {
    i += sizeof(uint16_t) * numS;
    i += sizeof(uint16_t) * numV * 2;
    eles++;
  } while (i < len);

  pD->eleCnt = eles;

  i = 8;
  pD->featureBits = BUILD_UINT16(data[i], data[i + 1]);
  i += 2;
  pD->pElm = (element_t *)malloc(sizeof(element_t) * pD->eleCnt);
  memset(pD->pElm, 0, sizeof(element_t) * pD->eleCnt);
  i += 2;   /* skip loc */
  for (uint8_t e = 0; e < pD->eleCnt; e++) {
    if (e) {
      i += 2;   /* skip loc */
    }
    pD->pElm[e].numS = data[i++];
    pD->pElm[e].numV = data[i++];
    if (pD->pElm[e].numS) {
      pD->pElm[e].pSm = (uint16_t *)malloc(sizeof(uint16_t) * pD->pElm[e].numS);
      memset(pD->pElm[e].pSm, 0, sizeof(uint16_t) * pD->pElm[e].numS);
      uint8_t offset = 0;
      for (uint8_t ms = 0; ms < pD->pElm[e].numS; ms++) {
        uint16_t mdid = BUILD_UINT16(data[i], data[i + 1]);
        if (mdid == GENERIC_ONOFF_CLIENT_MDID) {
          CS_LOG("-|||>>>OnOff Client Node (Switch)<<<|||-\n");
        } else if (mdid == GENERIC_ONOFF_SERVER_MDID) {
          CS_LOG("-|||>>>OnOff Server Node (Light)<<<|||-\n");
        }
        i += 2;
        if (mdid == CONFIGURATION_CLIENT_MDID
            || mdid == CONFIGURATION_SERVER_MDID
            || mdid == HEALTH_CLIENT_MDID
            || mdid == HEALTH_SERVER_MDID) {
          offset++;
          continue;
        }
        pD->pElm[e].pSm[ms - offset] = mdid;
      }
      pD->pElm[e].numS -= offset;
    }
    if (pD->pElm[e].numV) {
      pD->pElm[e].pVd = (uint16_t *)malloc(sizeof(uint16_t) * pD->pElm[e].numV);
      pD->pElm[e].pVm = (uint16_t *)malloc(sizeof(uint16_t) * pD->pElm[e].numV);
      memset(pD->pElm[e].pVd, 0, sizeof(uint16_t) * pD->pElm[e].numV);
      memset(pD->pElm[e].pVm, 0, sizeof(uint16_t) * pD->pElm[e].numV);
      for (uint8_t ms = 0; ms < pD->pElm[e].numS; ms++) {
        pD->pElm[e].pVd[ms] = BUILD_UINT16(data[i], data[i + 1]);
        i += 2;
        pD->pElm[e].pVm[ms] = BUILD_UINT16(data[i], data[i + 1]);
        i += 2;
      }
    }
  }
}
