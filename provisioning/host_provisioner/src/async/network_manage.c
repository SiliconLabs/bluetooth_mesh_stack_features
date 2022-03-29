/*************************************************************************
    > File Name: adding_device.c
    > Author: Kevin
    > Created Time: 2019-04-22
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
/* #include "adding_device.h" */
#include <unistd.h>

#include "network_manage.h"
#include "generic_main.h"
#include "utils.h"
#include "conf_generic_parser.h"
#include "prov_assert.h"
#include "bg_stack/gecko_bglib.h"
#include "device_database.h"
#include "conf_out.h"
#include "action.h"
#include "async_config_client.h"
#include "event_dispenser.h"

/* Defines  *********************************************************** */

#ifndef NMR_DEBUG
#define NMR_DEBUG 1
#endif
#if (NMR_DEBUG == 1)
#define nmrDEBUG(__fmt__, ...)  dbgPrint(__fmt__, ##__VA_ARGS__)
#else
#define nmrDEBUG(__fmt__, ...)
#define __ERR_P(err, cache, state)
#endif

#define DEV_INFO      "Dev Info:\n"
#define DEV_PADDING   "         "
#define DEV_KEY_INFO  "DevKey - 0x"
#define DEV_KEY_LSB   "DevKey(LSB for NA import) - "
#define UUID_INFO     "UUID   - 0x"
/* Global Variables *************************************************** */

/* Static Variables *************************************************** */
#if 0
static int needAdd = 0;
#endif
static provisioner_t *pProv = NULL;

/* Static Functions Declaractions ************************************* */
#if 0
static int isGenericEvent(uint32_t evtId);
#endif
static void reset(int type);
static int onMeshInitialized(struct gecko_msg_mesh_prov_initialized_evt_t *e);
static void loadDefaultNetTx(void);
static void loadDefaultTimeouts(void);
static inline void printDev(const uint8_t *pU,
                            const struct gecko_msg_mesh_prov_ddb_get_rsp_t *e);

int networkInit(void)
{
  uint16_t ret = 0;

  pProv = getProvisioner();
  loadDefaultNetTx();
  loadDefaultTimeouts();
  ddbInit();

  if (bg_err_success != (ret = gecko_cmd_mesh_prov_init()->result)) {
    CS_ERR("Error (0x%04x) Happened when trying initializing provisioner\n", ret);
    return E_BGAPI;
  }

  return E_SUC;
}

/* TODO - This need to rework */
int getNetworkIdByNodeId(int nodeId)
{
  return 0;
}

int genericMainLoop(void)
{
  return 0;
}

void normalReset(void)
{
  reset(0);
}

void factoryReset(void)
{
  reset(1);
}

int enOrDisUnprovBeaconScanning(int enable)
{
  uint16_t ret = 0;
  if (enable && !pProv->runningStatus.isScanning) {
    if (bg_err_success != (ret = gecko_cmd_mesh_prov_scan_unprov_beacons()->result)) {
      CS_ERR("Error (0x%04x) Happened when starting scanning\n", ret);
      return E_BGAPI;
    } else {
      CS_HLB("%s\n", "Scanning Started");
    }
  } else if ( !enable && pProv->runningStatus.isScanning ) {
    if (bg_err_success != (ret = gecko_cmd_mesh_prov_stop_scan_unprov_beacons()->result)) {
      CS_ERR("Error (0x%04x) Happened when stopping scanning\n", ret);
      return E_BGAPI;
    } else {
      CS_HLB("%s\n", "Scanning Stopped");
    }
  }
  pProv->runningStatus.isScanning = enable;
  return E_SUC;
}

int generic_event_handler(struct gecko_cmd_packet *e)
{
  int ret = 0;
  hardASSERT(e);
  switch (BGLIB_MSG_ID(e->header)) {
    /* General BLE */
    case gecko_evt_system_boot_id:
    {
      reset(0);
    }
    break;

    case gecko_evt_le_connection_closed_id:
    {
      CS_MSG("Connection closed, reason - 0x%04x\n",
             e->data.evt_le_connection_closed.reason);
    }

    case gecko_evt_mesh_prov_initialized_id:
    {
      pProv->state = initialized_em;
      CS_MSG("PROV_INIT\n");
      if (E_SUC == (ret = onMeshInitialized(&e->data.evt_mesh_prov_initialized))) {
        pProv->state = configured_idle_em;
        CS_MSG("\n ---> Ready <---\n$ ");
        fflush(stdout);
      } else {
        CS_ERR("Configure self failed[%d], check the reason.\n", ret);
#if (DEBUG == 0)
        exit(EXIT_FAILURE);
#endif
      }
    }
    break;

    case gecko_evt_mesh_prov_ddb_list_id:
    {
      struct gecko_msg_mesh_prov_ddb_get_rsp_t *rsp = NULL;
      rsp = gecko_cmd_mesh_prov_ddb_get(16, e->data.evt_mesh_prov_ddb_list.uuid.data);
      if (bg_err_success != rsp->result) {
        CS_ERR("Error (0x%04x) Happened when trying getting dcd\n", rsp->result);
        return E_BGAPI;
      } else {
        printDev(e->data.evt_mesh_prov_ddb_list.uuid.data,
                 rsp);
      }
    }
    break;

    case gecko_evt_le_gap_adv_timeout_id:
      break;
    default:
      CS_ERR("Unexpected event [0x%08x] happend in GENERIC EVENT HANDLER.\n",
             BGLIB_MSG_ID(e->header));
      return 0;
  }

  return 1;
}

static int createNetKeys(networkConfig_t *pconfig)
{
  int done = 0;
  struct gecko_msg_mesh_prov_create_network_rsp_t *createNetKeyRsp;

  if (!pconfig->keys.netkey.done) {
    createNetKeyRsp = gecko_cmd_mesh_prov_create_network(16, pconfig->keys.netkey.value);
    if (createNetKeyRsp->result == bg_err_success) {
      SC(setNetkeyIdToFile(pconfig->keys.netkey.refId,
                           createNetKeyRsp->network_id),
         "setNetkeyIdToFile");
      pconfig->keys.netkey.id = createNetKeyRsp->network_id;
      pProv->subnetCnt++;
      done = 1;
      CS_MSG("Network Key created successfully\n");
    } else if (createNetKeyRsp->result == bg_err_mesh_already_exists) {
      CS_MSG("Netkey already exists, pass.\n");
    } else if (createNetKeyRsp->result == bg_err_out_of_memory) {
      CS_MSG("Not enough memory, pass.\n");
      done = 0;
    } else if (createNetKeyRsp->result == bg_err_mesh_limit_reached) {
      CS_MSG("Limited number of netkey reached.\n");
      done = 0;
    } else {
      dbgPrint("Created netkey to stack error - 0x%04x\n", createNetKeyRsp->result);
    }
    SC(setNetkeyDoneToFile(pconfig->keys.netkey.refId, done),
       "setNetkeyDoneToFile");
    pconfig->keys.netkey.done = done;
  }
  return E_SUC;
}

static int createAppKeys(networkConfig_t *pconfig)
{
  struct gecko_msg_mesh_prov_create_appkey_rsp_t *createAppKeyRsp;
  if (pconfig->keys.netkey.done) {
    for (int i = 0; i < pconfig->keys.appKeyCnt; i++) {
      if (!pconfig->keys.appKeys[i].done) {
        createAppKeyRsp = gecko_cmd_mesh_prov_create_appkey(
          pconfig->keys.netkey.id,
          16,
          pconfig->keys.appKeys[i].value);
        if (createAppKeyRsp->result == bg_err_success) {
          SC(setAppkeyIdToFile(pconfig->keys.appKeys[i].refId, createAppKeyRsp->appkey_index),
             "setAppkeyIdToFile");
          pconfig->keys.appKeys[i].id = createAppKeyRsp->appkey_index;
          CS_MSG("AppKey[%d] created successfully\n", i);
        } else if (createAppKeyRsp->result == bg_err_mesh_already_exists) {
          CS_LOG("appkey already exists, pass.\n");
        } else if (createAppKeyRsp->result == bg_err_out_of_memory) {
          CS_LOG("not enough memory, break.\n");
          break;
        } else if (createAppKeyRsp->result == bg_err_mesh_limit_reached) {
          CS_LOG("Limited number of appkey reached, break.\n");
          pconfig->keys.appKeyCnt = i;
          break;
        }
        SC(setAppkeyDoneToFile(pconfig->keys.appKeys[i].refId, 1),
           "setAppkeyDoneToFile");
        pconfig->keys.appKeys[i].done = 1;
      }
    }
  }

  int tmp = 0;
  for (int i = 0; i < pconfig->keys.appKeyCnt; i++) {
    if (pconfig->keys.appKeys[i].done) {
      tmp++;
    }
  }
  pconfig->keys.activeAppKeyCnt = tmp;

  return E_SUC;
}

static int readDeviceDatabase(networkConfig_t *pconfig)
{
  int devCnt;
  struct gecko_msg_mesh_prov_ddb_list_devices_rsp_t *listDeviceRsp;
  struct gecko_cmd_packet *payload = NULL;

  listDeviceRsp = gecko_cmd_mesh_prov_ddb_list_devices();

  if (bg_err_success != listDeviceRsp->result) {
    CS_ERR("Error (0x%04x) happened when list devices.\n",
           listDeviceRsp->result);
    return E_BGAPI;
  }

  devCnt = listDeviceRsp->count;

  /* TODO - Add a timeout */
  while (devCnt) {
    if ((payload = gecko_peek_event()) != NULL) {
      switch (BGLIB_MSG_ID(payload->header)) {
        case gecko_evt_system_boot_id:
        {
          /* TODO - handle it */
          hardASSERT(0);
          return E_UNSPEC;
        }
        break;
        case gecko_evt_mesh_prov_ddb_list_id:
        {
          addOneDeviceToDDB(payload->data.evt_mesh_prov_ddb_list.uuid);
          devCnt--;
        }
        break;
        default:
          break;
      }
    }
  }

  return E_SUC;
}

static int setConfigTimeouts(networkConfig_t *pconfig)
{
  uint16_t ret = 0;
  if (bg_err_success != (ret = gecko_cmd_mesh_config_client_set_default_timeout(
                           pProv->configTimeouts.normalTimeout,
                           pProv->configTimeouts.lpnTimeout)->result)) {
    CS_ERR("Error (0x%04x) Happened when trying setting default timeouts\n", ret);
    return E_BGAPI;
  } else {
    CS_MSG("Set config client timeout [normal node/LPN] = [%dms/%dms] Success\n",
           pProv->configTimeouts.normalTimeout, pProv->configTimeouts.lpnTimeout);
  }
  return E_SUC;
}

static int setProvNetTx(networkConfig_t *pconfig)
{
  uint16_t ret;
  if (bg_err_success != (ret = gecko_cmd_mesh_test_set_nettx(
                           pProv->nettxConfig.nettxCnt,
                           pProv->nettxConfig.nettxInterval)->result)) {
    CS_ERR("Error (0x%04x) Happened when trying setting nettx\n", ret);
    return E_BGAPI;
  } else {
    CS_MSG("Set local network transmission Count/Interval [%d/%dms] Success\n",
           pProv->nettxConfig.nettxCnt + 1, (pProv->nettxConfig.nettxInterval + 1) * 10);
  }
  return E_SUC;
}

static int setProvisionerDefaultSettings(networkConfig_t *pconfig)
{
  int ret;
  if (E_SUC != (ret = setProvNetTx(pconfig))) {
    return ret;
  }
  if (E_SUC != (ret = setConfigTimeouts(pconfig))) {
    return ret;
  }
  return E_SUC;
}

static int onMeshInitialized(struct gecko_msg_mesh_prov_initialized_evt_t *e)
{
  int ret;
  uint16_t devCnt = 0;
  networkConfig_t *pconfig = NULL;

  (void)devCnt;
  /* pProv->state = initialized_em; */
  pProv->address = e->address;
  pProv->ivi = e->ivi;
  pProv->subnetCnt = e->networks;

  if (E_SUC != (ret = getNetworkConfig((void **)&pconfig))) {
    CS_ERR("Get config failed, check if XML is empty or not exist\n");
    return ret;
  }

  /* Network Keys */
  if (E_SUC != (ret = createNetKeys(pconfig))) {
    return ret;
  }

  /* App Keys */
  if (E_SUC != (ret = createAppKeys(pconfig))) {
    return ret;
  }

  /* read Deivce Database */
  if (E_SUC != (ret = readDeviceDatabase(pconfig))) {
    return ret;
  }

  /* set nettx and timeouts */
  if (E_SUC != (ret = setProvisionerDefaultSettings(pconfig))) {
    return ret;
  }

  return E_SUC;
}

static void resetNcpTarget(void)
{
  int ret = 0;

  if (bg_err_success != (ret = gecko_cmd_le_gap_end_procedure()->result)) {
    CS_ERR("Error (0x%04x) Happened when trying end gap procedure\n", ret);
    return;
  }

  preForceReSyncUpHostAndTarget();
  forceReSyncUpHostAndTarget();
  while (1 != getSyncUpStatus());

  if (E_SUC != (ret = genericInit())) {
    CS_ERR("Error (%d) Happened when trying init network\n", ret);
  }
}

static void reset(int type)
{
  uint16_t ret;
  if (pProv->connHandle != 0xFF) {
    if (bg_err_success != (ret = gecko_cmd_le_connection_close(pProv->connHandle)->result)) {
      CS_ERR("Error (0x%04x) Happened when trying disconnecting\n", ret);
    }
  }
  if (type) {
    confClearControlFields();
    if (bg_err_success != (ret = gecko_cmd_flash_ps_erase_all()->result)) {
      CS_ERR("Error (0x%04x) Happened when trying erase flash\n", ret);
    }
    sleep(1);
  }
  resetNcpTarget();
}

#if 0
static int isGenericEvent(uint32_t evtId)
{
  return 1;
}
#endif

static void loadDefaultNetTx(void)
{
  char *p = NULL, deft = '0';
  int val = 0;

  p = getValueByKey(KEY_NETTX_CNT);
  if (p != NULL && (*p >= '1' && *p <= '8')) {
    pProv->nettxConfig.nettxCnt = *p - '0' - 1;
  } else {
    deft = '0' + DEFAULT_NET_TX + 1;
    saveKeyValue(KEY_NETTX_CNT, &deft);
    pProv->nettxConfig.nettxCnt = DEFAULT_NET_TX;
  }

  p = getValueByKey(KEY_NETTX_INT);
  if (p == NULL) {
    deft = '0' + DEFAULT_NET_TX_INTERVAL + 1;
    saveKeyValue(KEY_NETTX_INT, &deft);
  } else {
    val = atoi(p);
    if (val >= 1 && val <= 32) {
      pProv->nettxConfig.nettxInterval = val - 1;
    } else {
      deft = '0' + DEFAULT_NET_TX_INTERVAL + 1;
      saveKeyValue(KEY_NETTX_INT, &deft);
      pProv->nettxConfig.nettxInterval = DEFAULT_NET_TX_INTERVAL;
    }
  }
}

static void loadDefaultTimeouts(void)
{
  char *p = NULL, deft = '0';
  int val = 0;

  p = getValueByKey(KEY_TIMEOUT);
  if (p == NULL) {
    deft = '0' + DEFAULT_CONFIG_TIMEOUT / 1000;
    saveKeyValue(KEY_TIMEOUT, &deft);
    pProv->configTimeouts.normalTimeout = DEFAULT_CONFIG_TIMEOUT;
  } else {
    val = atoi(p);
    pProv->configTimeouts.normalTimeout = val * 1000;
  }

  p = getValueByKey(KEY_TIMEOUT_LPN);
  if (p == NULL) {
    deft = '0' + DEFAULT_CONFIG_LPN_TIMEOUT / 1000;
    saveKeyValue(KEY_TIMEOUT_LPN, &deft);
    pProv->configTimeouts.lpnTimeout = DEFAULT_CONFIG_LPN_TIMEOUT;
  } else {
    val = atoi(p);
    pProv->configTimeouts.lpnTimeout = val * 1000;
  }
}

static inline void printDev(const uint8_t *pU,
                            const struct gecko_msg_mesh_prov_ddb_get_rsp_t *e)
{
  hardASSERT(e);
  hardASSERT(pU);

  printf(DEV_INFO);
  printf(DEV_PADDING);
  printf(UUID_INFO);
  for (int i = 15; i >= 0; i--) {
    if (i == 12) {
      printf("-");
    }
    printf("%02x",
           pU[i]);
    if (i == 10) {
      printf("-");
    }
  }
  printf("\n");
  printf(DEV_PADDING);
  printf(DEV_KEY_INFO);
  for (int i = 15; i >= 0; i--) {
    printf("%02x",
           e->device_key.data[i]);
  }
  printf("\n");
  printf(DEV_PADDING);
  printf(DEV_KEY_LSB);
  for (int i = 0; i < 16; i++) {
    printf("%02x",
           e->device_key.data[i]);
  }
  printf("\n");
}
