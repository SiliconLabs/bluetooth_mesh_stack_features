/*************************************************************************
    > File Name: config_end.c
    > Author: Kevin
    > Created Time: 2019-04-28
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include "async/async_config_client.h"
#include "conf_generic_parser.h"
#include "generic_main.h"
#include "action.h"

/* Defines  *********************************************************** */

/* Global Variables *************************************************** */
extern const char *stateNames[];

/* Static Variables *************************************************** */

/* Static Functions Declaractions ************************************* */
static void onConfigSuccess(tbcCache_t *tbc, networkConfig_t *pconfig);
static void onConfigFail(tbcCache_t *tbc, networkConfig_t *pconfig);

int endStateEntry(void *in, funcGuard guard)
{
  /* Alarm SHOULD be set in the main engine */
  tbcCache_t *tbc = (tbcCache_t *)in;
  networkConfig_t *pconfig = (networkConfig_t *)tbc->pconfig;

  if (tbc->errCache.callErrBits != 0 || tbc->errCache.evtErrBits != 0) {
    onConfigFail(tbc, pconfig);
  } else {
    /* Node is configured successfully */
    onConfigSuccess(tbc, pconfig);
  }

  onDeviceDone(actionTBC, tbc->nodeIndex);
  forceGenericReloadActions();

  return E_SUC;
}

static void onConfigSuccess(tbcCache_t *tbc, networkConfig_t *pconfig)
{
  CS_SPLIT(1);
  CS_MSG(HL_SUCCESS " - Node[%d] Configured Properly.\n", tbc->nodeIndex);
  CS_SPLIT(1);

  pconfig->pNodes[tbc->nodeIndex].errBits = 0;
  setNodeErrBitsToFile(pconfig->pNodes[tbc->nodeIndex].uuid, 0);
  pconfig->pNodes[tbc->nodeIndex].done = 0x11;
  setNodeDoneToFile(pconfig->pNodes[tbc->nodeIndex].uuid, 0x11);
}

static void onConfigFail(tbcCache_t *tbc, networkConfig_t *pconfig)
{
  uint32_t err;
  if (tbc->errCache.evtErrBits) {
    CS_SPLIT(1);
    CS_ERR(HL_FAILED " due to unexpected event received.\n"
                     "       State Error Bit Mask- <<0x%08x>>\n"
                     "       BGLib Error code - <<0x%04x>>\n",
           tbc->errCache.evtErrBits,
           tbc->errCache.bgErr);
    CS_SPLIT(1);
  }

  if (tbc->errCache.callErrBits) {
    CS_SPLIT(1);
    if (tbc->errCache.callErrBits == 0x80000000) {
      CS_MSG("IGNORE: User interrupted the procedure.\n");
    } else {
      CS_ERR(HL_FAILED " due to unexpected return value of API calls.\n"
                       "       State Error Bit Mask- <<0x%08x>>\n"
                       "       BGLib Error code - <<0x%04x>>\n",
             tbc->errCache.callErrBits,
             tbc->errCache.bgErr);
    }
    CS_SPLIT(1);
  }

  err = (tbc->errCache.evtErrBits | tbc->errCache.callErrBits) & 0x7FFFFFFF;
  pconfig->pNodes[tbc->nodeIndex].errBits = err;
  setNodeErrBitsToFile(pconfig->pNodes[tbc->nodeIndex].uuid, err);
}
