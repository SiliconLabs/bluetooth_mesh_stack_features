/*************************************************************************
    > File Name: adding_devices.c
    > Author: Kevin
    > Created Time: 2019-04-24
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
#ifndef TBA_DEBUG
#define TBA_DEBUG 1
#endif
#if (TBA_DEBUG == 1)
#define tbaDEBUG(__fmt__, ...)  dbgPrint(__fmt__, ##__VA_ARGS__)
#else
#define tbaDEBUG(__fmt__, ...)
#endif

#define PROV_MSG \
  "Node[%d]: Provisioning Started\n"
#define PROV_SUC_MSG \
  "Node[%d]: Provisioning " HL_SUCCESS ", Address = <0x%04x>\n"
#define PROV_FAIL_MSG \
  "Node[%d]: Provisioning " HL_FAILED ", Reason <0x%04x>\n"

#define ONCE_P(nodeId) \
  do {                 \
    CS_MSG(            \
      PROV_MSG,        \
      nodeId);         \
  } while (0)

#define SUC_P(nodeId, addr) \
  do {                      \
    CS_MSG(                 \
      PROV_SUC_MSG,         \
      nodeId,               \
      addr);                \
  } while (0)

#define FAIL_P(nodeId, err) \
  do {                      \
    CS_ERR(                 \
      PROV_FAIL_MSG,        \
      nodeId,               \
      err);                 \
  } while (0)
/* Global Variables *************************************************** */

/* Static Variables *************************************************** */
static tbaStatus_t *pStatus = NULL;

#if 0
static const uint32_t events[] = {
  gecko_evt_mesh_prov_unprov_beacon_id,
  gecko_evt_mesh_prov_provisioning_failed_id,
  gecko_evt_mesh_prov_device_provisioned_id
};
#endif

#define RELATE_EVENTS_NUM() (sizeof(events) / sizeof(uint32_t))

/* Static Functions Declaractions ************************************* */
#if (SEPARATE_THREADS == 1)
pthread_t configClientThreadId;
static void startConfigClientThread(int enable, void *config);
#endif
#if 0
static int isAddingDevicesRelatedPacket(uint32_t evtId);
#endif
static void onUnprovBeaconFound(void *p);
static void onProvisionFinished(int success, void *p);
#if 0
static void addingDevicesEvtFilter(void);
#endif
static int getFreeAddingCacheId(void);
static int findtbaStatusByUuidRawData(const networkConfig_t *pconfig,
                                      const uint8_t *data);
static int isAlreadyInCaches(const networkConfig_t *pconfig,
                             const uint8_t *data);

void addingDevicesResetAll(void)
{
}

int addingDevicesInit(funcUnprovBeaconFoundCB fn,
                      funcProvDoneCB fn1)
{
  pStatus = gettbaStatus();
  pStatus->fnUnprovBeaconFoundCB = fn;
  pStatus->fnProvDoneCB = fn1;

#if 0
  addingDevicesEvtFilter();
#endif

  return 0;
}

int adding_devices_event_handler(struct gecko_cmd_packet *e)
{
  hardASSERT(e);

  if (!IS_ACTION_BIT_SET(getLoadedActions(), actionTBA)) {
    return 0;
  }

  switch (BGLIB_MSG_ID(e->header)) {
    case gecko_evt_mesh_prov_unprov_beacon_id:
    {
      onUnprovBeaconFound(&e->data.evt_mesh_prov_unprov_beacon);
    }
    break;

    case gecko_evt_mesh_prov_device_provisioned_id:
    {
      onProvisionFinished(true, &e->data.evt_mesh_prov_device_provisioned);
    }
    break;
    case gecko_evt_mesh_prov_provisioning_failed_id:
    {
      onProvisionFinished(false, &e->data.evt_mesh_prov_provisioning_failed);
    }
    break;

    default:
      return 0;
  }

  return 1;
}

int addingDevicesMainLoop(void *p)
{
  enOrDisUnprovBeaconScanning(1);
  return 0;
}

#if (SEPARATE_THREADS == 1)
static void startConfigClientThread(int enable, void *config)
{
  int ret;
  if (enable && !pStatus->configClientState) {
    /* start the thread  */
    if (0 != (ret = pthread_create(&configClientThreadId,
                                   NULL,
                                   accMainProcess,
                                   config))) {
      errExitEN(ret, "pthread_create ACC");
    }
    pStatus->configClientState = enable;
  } else if (!enable && pStatus->configClientState) {
    cleanUpConfigClientThread();
    /* stop the thread  */
    if (0 != (ret = pthread_cancel(configClientThreadId))) {
      errExitEN(ret, "pthread_cancel ACC");
    }
    pStatus->configClientState = enable;
  }
}
#endif

#if 0
static int isAddingDevicesRelatedPacket(uint32_t evtId)
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

static int startProvisionDevice(int cacheId)
{
  uint16_t ret = 0;
  networkConfig_t *pconfig = NULL;

  getNetworkConfig((void **)&pconfig);
  hardASSERT(pconfig);
  if (bg_err_success
      != (ret = gecko_cmd_mesh_prov_provision_device(
            getNetworkIdByNodeId(pStatus->tba[cacheId].nodeId),
            16,
            pconfig->pNodes[pStatus->tba[cacheId].nodeId].uuid.data)->result)) {
    FAIL_P(pStatus->tba[cacheId].nodeId, ret);
    return E_BGAPI;
  } else {
    ONCE_P(pStatus->tba[cacheId].nodeId);
    pStatus->tba[cacheId].isWaiting = true;
    pStatus->tba[cacheId].state = tba_busy_em;
  }
  return E_SUC;
}

static int onUnprovBeaconMatch(int cacheId, int offset)
{
  pStatus->tba[cacheId].nodeId = getNodeByOffset(actionTBA, offset);
  CS_MSG("Unprovisioned Beacon **MATCH**.\n");

  return startProvisionDevice(cacheId);
}

static void onUnprovBeaconFound(void *p)
{
  int i = 0, cacheId = 0;
  struct gecko_msg_mesh_prov_unprov_beacon_evt_t *e = p;
  networkConfig_t *pconfig = NULL;

  getNetworkConfig((void **)&pconfig);
  hardASSERT(e);
  hardASSERT(pconfig);

  if (getFreeModeStatus() && pStatus->fnUnprovBeaconFoundCB) {
    if (E_SUC != pStatus->fnUnprovBeaconFoundCB(e)) {
      CS_ERR("Add Unprov Beacon to File Failed.\n");
    }
  }

  if (e->bearer == 1) {
    /* TODO - Not support PB-GATT for now */
    return;
  }

  if (isAlreadyInCaches(pconfig, e->uuid.data)) {
    return;
  }

  if (-1 == (cacheId = getFreeAddingCacheId())) {
    return;
  }

  for (i = 0; i < getCnt(actionTBA); i++) {
    if (!memcmp(pconfig->pNodes[getNodeByOffset(actionTBA, i)].uuid.data,
                e->uuid.data,
                16)) {
      onUnprovBeaconMatch(cacheId, i);
    }
  }
}

static void onProvisionFinished(int success, void *p)
{
  uint32_t errbits = 0;
  int offset = 0;
  struct gecko_msg_mesh_prov_provisioning_failed_evt_t *ef = p;
  struct gecko_msg_mesh_prov_device_provisioned_evt_t *es = p;

  networkConfig_t *pconfig = NULL;

  getNetworkConfig((void **)&pconfig);
  hardASSERT(pconfig);
  hardASSERT(success >= 0 && success <= 1 && p);

  if (success) {
    /* inform tbc */
    offset = findtbaStatusByUuidRawData(pconfig, es->uuid.data);
    hardASSERT(offset != -1);
    addOneDeviceToDDBWithRawData(es->uuid.data);
    pconfig->pNodes[pStatus->tba[offset].nodeId].uniAddr = es->address;
    setNodeUniAddrToFile(pconfig->pNodes[pStatus->tba[offset].nodeId].uuid,
                         pconfig->pNodes[pStatus->tba[offset].nodeId].uniAddr);
    SUC_P(pStatus->tba[offset].nodeId,
          pconfig->pNodes[pStatus->tba[offset].nodeId].uniAddr);
    pconfig->pNodes[pStatus->tba[offset].nodeId].done = 0x1;
    setNodeDoneToFile(pconfig->pNodes[pStatus->tba[offset].nodeId].uuid, 0x01);
    setNewDeviceAddedFlag();
  } else {
    offset = findtbaStatusByUuidRawData(pconfig, ef->uuid.data);
    FAIL_P(pStatus->tba[offset].nodeId, ef->reason);
    /* TODO - to check */
    errbits = (1 << provisioning_em);
    pconfig->pNodes[pStatus->tba[offset].nodeId].errBits = errbits;;
    setNodeErrBitsToFile(pconfig->pNodes[pStatus->tba[offset].nodeId].uuid,
                         errbits);
  }

  pStatus->tba[offset].isWaiting = 0;
  pStatus->tba[offset].state = tba_idle_em;
  if (pStatus->fnProvDoneCB) {
    pStatus->fnProvDoneCB(&pStatus->tba[offset].nodeId);
  }
}

#if 0
static int isAddingDeviceFree(void *p)
{
  return getFreeAddingCacheId() == -1 ? 0 : 1;
}
#endif

/**
 * @brief unprovBeaconFilter return 0 not to report to tba
 *
 * @param p
 *
 * @return
 */
#if 0
static int unprovBeaconFilter(void *p)
{
  return 1;
}

static void addingDevicesEvtFilter(void)
{
  registerFilter(gecko_evt_mesh_prov_unprov_beacon_id, unprovBeaconFilter);
}
#endif

static int isAlreadyInCaches(const networkConfig_t *pconfig,
                             const uint8_t *data)
{
  int i = 0;
  for (i = 0; i < MAX_PROV_SESSIONS; i++) {
    if (pStatus->tba[i].state == tba_busy_em) {
      if (!memcmp(pconfig->pNodes[pStatus->tba[i].nodeId].uuid.data,
                  data,
                  16)) {
        return 1;
      }
    }
  }
  return 0;
}

static int getFreeAddingCacheId(void)
{
  int i = 0;
  for (i = 0; i < MAX_PROV_SESSIONS; i++) {
    if (pStatus->tba[i].state == tba_idle_em) {
      return i;
    }
  }
  return -1;
}

static int findtbaStatusByUuidRawData(const networkConfig_t *pconfig,
                                      const uint8_t *data)
{
  int i = 0;
  for (i = 0; i < MAX_PROV_SESSIONS; i++) {
    if (pStatus->tba[i].state == tba_busy_em
        && !memcmp(pconfig->pNodes[pStatus->tba[i].nodeId].uuid.data,
                   data,
                   16)) {
      return i;
    }
  }

  return -1;
}
