/*************************************************************************
    > File Name: blacklisting_devices.c
    > Author: Kevin
    > Created Time: 2019-04-25
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include "generic_main.h"

#include "action.h"
#include "prov_assert.h"
#include "utils.h"
#include "async_config_client.h"
#include "conf_generic_parser.h"

/* Defines  *********************************************************** */
#ifndef TBBL_DEBUG
#define TBBL_DEBUG 1
#endif
#if (TBBL_DEBUG == 1)
#define tbblDEBUG(__fmt__, ...)  dbgPrint(__fmt__, ##__VA_ARGS__)
#else
#define tbblDEBUG(__fmt__, ...)
#define __ERR_P(err, cache, state)
#endif

#define KRF_MSG                                "Key Refresh Started\n"
/* #define KRF_SUC_MSG                            "Provisioning " HL_SUCCESS ", Address = <0x%04x>\n" */
/* #define KRF_FAIL_MSG                           "Provisioning " HL_FAILED ", Reason <0x%04x>\n" */

/* Global Variables *************************************************** */
enum {
  KRF_node_update_em,
  KRF_network_update_em
};
/* Static Variables *************************************************** */
static tbblStatus_t *pStatus = NULL;

#if 0
static const uint32_t events[] = {
  gecko_evt_mesh_prov_key_refresh_node_update_id,
  gecko_evt_mesh_prov_key_refresh_phase_update_id,
  gecko_evt_mesh_prov_key_refresh_complete_id
};
#endif

#define RELATE_EVENTS_NUM() (sizeof(events) / sizeof(uint32_t))
/* Static Functions Declaractions ************************************* */
#if 0
static int isBlacklistingDevicesRelatedPacket(uint32_t evtId);
#endif
static void onKRFNodeUpdate(int which, void *p);
static void onKeyRefreshFinished(void *p);
static size_t sendDevicesToStackUntilOOM(networkConfig_t *pconfig);
static int startKRF(networkConfig_t *pconfig);
static void tbblOnFinished(const networkConfig_t *pconfig);

int blacklistingDevicesInit(void)
{
  pStatus = gettbblStatus();
  return 0;
}

int blacklistingDevicesMainLoop(void *p)
{
  int busy = 0, remainings = 0;
  size_t nodesCntSentToStackToBL = 0;
  networkConfig_t *pconfig = NULL;
  getNetworkConfig((void **)&pconfig);
  hardASSERT(pconfig);

  remainings = getCnt(actionTBBL);
  if (!remainings) {
    hardASSERT(0);
  }

  switch (pStatus->state) {
    case tbbl_idle_em:
      if (0 == (nodesCntSentToStackToBL = sendDevicesToStackUntilOOM(pconfig))) {
        CS_ERR("Shouldn't get here\n");
        return 0;
      }
      startKRF(pconfig);
      busy = 1;
      break;
    case tbbl_busy_em:
      break;
    case tbbl_done_em:
      tbblOnFinished(pconfig);
      break;
  }
  return busy;
}

static size_t sendDevicesToStackUntilOOM(networkConfig_t *pconfig)
{
  uint16_t ret = 0;
  int bak;
  int nodeId = 0;
  tbblCache_t *cache = &pStatus->cache;

  bak = cache->offset;

  if (cache->offset != getCnt(actionTBBL)) {
    do {
      nodeId = getNodeByOffset(actionTBBL, cache->offset);

      if (bg_err_success != (ret = gecko_cmd_mesh_prov_set_key_refresh_blacklist(
                               getNetworkIdByNodeId(nodeId),
                               1,
                               16,
                               pconfig->pNodes[nodeId].uuid.data)->result)) {
        CS_ERR("Error (0x%04x) happened when trying to add one device to blacklist list.\n",
               ret);
      }
    } while (ret == bg_err_success
             && ++cache->offset != getCnt(actionTBBL));
    if (cache->offset != bak) {
      GET_HL_STR(cnt,
                 "%d %s",
                 cache->offset - bak,
                 cache->offset - bak > 1 ? "nodes" : "node");
      CS_MSG("Sent %s to stack to blacklist.\n", hlstr_cnt);
    }
  }

  return cache->offset - bak;
}

static int startKRF(networkConfig_t *pconfig)
{
  int ret = 0;
  const int numAppKeys = 0; /* TODO: Not update app key, hard coded to 0 for now */

  if (bg_err_success != (ret = gecko_cmd_mesh_prov_key_refresh_start(
                           pconfig->keys.netkey.id,
                           numAppKeys,
                           16 * numAppKeys,
                           NULL)->result)) {
    CS_ERR("Failed : KRF start, error code <<0x%04x>>\n", ret);
    return E_BGAPI;
  } else {
    CS_MSG(KRF_MSG);
    pStatus->isWaiting = true;
    pStatus->state = tbbl_busy_em;
  }
  return E_SUC;
}

int blacklist_devices_event_handler(struct gecko_cmd_packet *e)
{
  hardASSERT(e);

  if (!IS_ACTION_BIT_SET(getLoadedActions(), actionTBBL)) {
    return 0;
  }

  switch (BGLIB_MSG_ID(e->header)) {
    case gecko_evt_mesh_prov_key_refresh_node_update_id:
    {
      onKRFNodeUpdate(KRF_node_update_em, &e->data.evt_mesh_prov_key_refresh_node_update);
    }
    break;

    case gecko_evt_mesh_prov_key_refresh_phase_update_id:
    {
      onKRFNodeUpdate(KRF_network_update_em, &e->data.evt_mesh_prov_key_refresh_phase_update);
    }
    break;

    case gecko_evt_mesh_prov_key_refresh_complete_id:
    {
      onKeyRefreshFinished(&e->data.evt_mesh_prov_key_refresh_complete);
    }
    break;

    default:
      tbblDEBUG("Unexpected event [0x%08x] happend in TBBL EVENT HANDLER.\n",
                BGLIB_MSG_ID(e->header));
      return 0;
  }
  return 1;
}
#if 0
static int isBlacklistingDevicesRelatedPacket(uint32_t evtId)
{
  int i;
  for (i = 0; i < RELATE_EVENTS_NUM(); i++) {
    if (BGLIB_MSG_ID(evtId) == events[i]) {
      return 1;
    }
  }
  return 0;
}
#endif

static int getNodeIdByUUID(const networkConfig_t *pconfig, const uint8_t *pUuidData, int *pI)
{
  if (pI == NULL || pUuidData == NULL) {
    return E_INVP;
  }

  for (int a = 0; a < pconfig->nodeCnt; a++) {
    if (!memcmp(pconfig->pNodes[a].uuid.data,
                pUuidData,
                16)) {
      *pI = a;
      return E_SUC;
    }
  }

  return E_NFND;
}

static void setPhaseSucBit(int phase, int ndid)
{
  uint32_t *p;
  if (phase == 0) {
    p = pStatus->cache.phase0Devs;
  } else if (phase == 1) {
    p = pStatus->cache.phase1Devs;
  } else if (phase == 2) {
    p = pStatus->cache.phase2Devs;
  } else {
    return;
  }

  int ofs = ndid / 32;
  int bit = ndid % 32;

  p[ofs] |= (1 << bit);
}

static void onKRFNodeUpdate(int which, void *p)
{
  struct gecko_msg_mesh_prov_key_refresh_node_update_evt_t *en = p;
  struct gecko_msg_mesh_prov_key_refresh_phase_update_evt_t *ep = p;
  networkConfig_t *pconfig = NULL;
  getNetworkConfig((void **)&pconfig);
  hardASSERT(pconfig);

  if (which == KRF_node_update_em) {
    int ndid = 0;
    softASSERT(0 == getNodeIdByUUID(pconfig,
                                    en->uuid.data,
                                    &ndid));
    CS_MSG("Node[%d] has moved to [%d] phase - netkey ID [%d]\n",
           ndid,
           en->phase,
           en->key);
    setPhaseSucBit(en->phase, ndid);
  } else {
    CS_MSG("Whole network has moved to [%d] phase - netkey ID [%d]\n",
           ep->phase,
           ep->key);
  }
}

static void onKeyRefreshFinished(void *p)
{
  int ret;
  struct gecko_msg_mesh_prov_key_refresh_complete_evt_t *e = p;
  struct gecko_msg_mesh_test_get_key_rsp_t *rsp;

  networkConfig_t *pconfig = NULL;
  getNetworkConfig((void **)&pconfig);
  hardASSERT(pconfig);

  pStatus->state = tbbl_idle_em;
  pStatus->isWaiting = false;

  CS_MSG("Key refresh complete <<%s>>, err[0x%04x]\nNew Network Key Id is 0x%04x\n",
         e->result == 0 ? "YES" : "NO",
         e->result,
         e->key);
  if (e->result) {
    goto out;
  }

  pconfig->keys.netkey.id = e->key;
  ret = setNetkeyIdToFile(pconfig->keys.netkey.refId, e->key);
  rsp = gecko_cmd_mesh_test_get_key(mesh_test_key_type_net,
                                    0,
                                    1);
  if (rsp->result != bg_err_success) {
    CS_ERR("Error when get network key, error code - 0x%04x\n",
           rsp->result);
    goto out;
  } else {
    softASSERT(rsp->id == pconfig->keys.netkey.id);
    memcpy(pconfig->keys.netkey.value, rsp->key.data, 16);
    ret = setNetkeyValueToFile(pconfig->keys.netkey.refId,
                               rsp->key.data);
    CS_LOG("New network key recorded.\n");
  }

  pconfig->keys.netkey.id = e->key;
  softASSERT(ret == 0);
  /* pconfig->keys.netkey.refId = e->data.evt_mesh_prov_key_refresh_complete.key; */

  out:
  if (pStatus->cache.offset == getCnt(actionTBBL)) {
    pStatus->state = tbbl_done_em;
  }
}

int removeBlacklistedNodes(void)
{
  int cnt = 0, i = 0, idx = 0, ret = 0;
  cnt = getCnt(actionTBBL);
  for (i = 0; i < cnt; i++) {
    idx = getNodeByOffset(actionTBBL, i);
    if (removeDeviceFromLDDB(typeTBBL, idx) == E_SUC) {
      ret++;
    }
  }
  return ret;
}

static int tbblOnSuccess(const networkConfig_t *pCfgXml)
{
  int ret = 0, gap, i, ofs, bit, ndid;
  uint8_t origDevs = 0, remainingDevs = 0, realBLed = 0;

  for (i = 0; i < pCfgXml->nodeCnt; i++) {
    if (pCfgXml->pNodes[i].blacklist == 1 && pCfgXml->pNodes[i].done == 0) {
      continue;
    }
    origDevs++;
  }

  /* Remove nodes from DDB */
  ret = removeBlacklistedNodes();
  softASSERT(ret == pStatus->cache.offset); /* TODO - remove */
  for (i = 0; i < 8; i++) {
    remainingDevs += (uint8_t)bitPopcount(pStatus->cache.phase0Devs[i]);
  }

  realBLed = origDevs - remainingDevs;
  if (getCnt(actionTBBL) == realBLed) {
    CS_MSG("SUCCESS: %d nodes have been removed from the network.\n", realBLed);
  } else {
    CS_ERR("FATAL: Intent to remove %d nodes, "
           "but %d nodes have been removed from the network.\n",
           getCnt(actionTBBL),
           realBLed);
    /* TODO - Remove the unexpected nodes from the LDDB */
    gap = realBLed - getCnt(actionTBBL);

    for (i = 0; i < getCnt(actionTBBL); i++) {
      ndid = getNodeByOffset(actionTBBL, i);
      ofs = ndid / 32;
      bit = ndid % 32;
      pStatus->cache.phase0Devs[ofs] |= (1 << bit);
    }

    ndid = 0;
    for (i = 0; i < pCfgXml->nodeCnt; i++) {
      ofs = i / 32;
      bit = i % 32;
      if ((pStatus->cache.phase0Devs[ofs] & (1U << bit)) == 0) {
        CS_MSG("Removing node[%d] from the network due to it missed key refresh, "
               "Re-add it if needed\n", i);
        removeDeviceFromLDDB(typeTBBL, i);
        if (++ndid == gap) {
          dbgPrint("All unexpected removed nodes are removed from LDDB\n");
          break;
        }
      }
    }
  }
  return ret;
}

static void displayPhaseSucBit(void)
{
  for (int i = 0; i < 8; i++) {
    if (pStatus->cache.phase0Devs[i]
        || pStatus->cache.phase1Devs[i]
        || pStatus->cache.phase2Devs[i]) {
      CS_MSG("%d-%d device bits:\n", i * 32, i * 32 + 31);
      CS_MSG("\t phase 1: 0x%08x\n", pStatus->cache.phase1Devs[i]);
      CS_MSG("\t phase 2: 0x%08x\n", pStatus->cache.phase2Devs[i]);
      CS_MSG("\t phase 0: 0x%08x\n", pStatus->cache.phase0Devs[i]);
    }
  }
}

static void tbblOnFinished(const networkConfig_t *pconfig)
{
  tbblOnSuccess(pconfig);
  displayPhaseSucBit();

  memset(&pStatus->cache, 0, sizeof(tbblCache_t));
  clearList(actionTBBL);
  memset(pStatus, 0, sizeof(tbblStatus_t));
  forceGenericReloadActions();
}
