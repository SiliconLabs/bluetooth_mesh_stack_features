/*************************************************************************
    > File Name: device_database.c
    > Author: Kevin
    > Created Time: 2019-04-23
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include "device_database.h"

#include "err_code.h"
#include "prov_assert.h"
#include "utils.h"
#include "network_manage.h"
#include "conf_generic_parser.h"
/* Defines  *********************************************************** */

/* Global Variables *************************************************** */

/* Static Variables *************************************************** */
static ddb_t deviceDB;

/* Static Functions Declaractions ************************************* */

void ddbInit(void)
{
  memset(&deviceDB, 0, sizeof(ddb_t));
}

int addOneDeviceToDDB(uuid_128 u)
{
  return addOneDeviceToDDBWithRawData(u.data);
}

int addOneDeviceToDDBWithRawData(const uint8_t *data)
{
  if (deviceDB.nodeCnt == MAX_DEVICE_COUNT_IN_DDB) {
    return E_NOESIZE;
  }

  memcpy(deviceDB.pNodes[deviceDB.nodeCnt++].data, data, 16);
  return E_SUC;
}

static int removeDeviceFromDDB(uuid_128 u)
{
  for (int i = 0; i < deviceDB.nodeCnt; i++) {
    if (!memcmp(&u, &deviceDB.pNodes[i], sizeof(uuid_128))) {
      memmove(&deviceDB.pNodes[i],
              &deviceDB.pNodes[i + 1],
              sizeof(uuid_128) * (deviceDB.nodeCnt - i - 1));
      deviceDB.nodeCnt--;
      return E_SUC;
    }
  }
  return E_NFND;
}

int removeDeviceFromLDDB(int t, int idxInPconfig)
{
  uint16_t ret = 0;
  networkConfig_t *pconfig = NULL;

  if (E_SUC != (ret = getNetworkConfig((void **)&pconfig))) {
    CS_ERR("Get config failed.\n");
    exit(EXIT_FAILURE);
  }

  if (bg_err_success != (ret = gecko_cmd_mesh_prov_ddb_delete(
                           pconfig->pNodes[idxInPconfig].uuid)->result)) {
    char uuidSt[16 * 2 + 1];
    softASSERT(0 == charBuf2str(
                 (const char *)pconfig->pNodes[idxInPconfig].uuid.data,
                 16,
                 0,
                 uuidSt,
                 sizeof(uuidSt)));
    CS_ERR("Failed to remove node[%d] from ddb, uuid = %s, error code <0x%04x>\n",
           idxInPconfig,
           uuidSt,
           ret);
    return E_BGAPI;
  } else {
    CS_MSG("Remove node[%d] from the network\n", idxInPconfig);
    pconfig->pNodes[idxInPconfig].blacklist = 1;
    pconfig->pNodes[idxInPconfig].done = 0;
    setNodeBlacklistToFile(pconfig->pNodes[idxInPconfig].uuid,
                           t == typeTBR ? 0x10 : 0x01);
    setNodeDoneToFile(pconfig->pNodes[idxInPconfig].uuid,
                      0);
    setNodeUniAddrToFile(pconfig->pNodes[idxInPconfig].uuid,
                         0);
    removeDeviceFromDDB(pconfig->pNodes[idxInPconfig].uuid);
  }
  return E_SUC;
}

int isNodeIndexInDeviceDB(int i)
{
  networkConfig_t *pconfig = NULL;
  getNetworkConfig((void **)&pconfig);
  hardASSERT(pconfig);

  for (int a = 0; a < deviceDB.nodeCnt; a++) {
    if (!memcmp(&pconfig->pNodes[i].uuid, &deviceDB.pNodes[a], sizeof(uuid_128))) {
      return true;
    }
  }
  return false;
}
