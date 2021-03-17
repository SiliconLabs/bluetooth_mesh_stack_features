/*************************************************************************
    > File Name: conf_generic_parser.c
    > Author: Kevin
    > Created Time: 2019-03-27
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include <stdio.h>
#include <stdbool.h>

#include "config.h"
#include "utils.h"
#include "conf_generic_parser.h"
#include "prov_assert.h"

#if (XML_SUP == 1)
#include "xml_parser.h"
#endif
#if (JSON_SUP == 1)
#include "json_parser.h"
#endif

#include <pthread.h>
/* Defines  *********************************************************** */
#define CHECK_REGS()  if (!funcsRegister) { return E_NINIT; }

#ifndef CGP_DEBUG
#define CGP_DEBUG 0
#endif
#if (CGP_DEBUG == 1)
#define cgpDEBUG(__fmt__, ...)  dbgPrint(__fmt__, ##__VA_ARGS__)
#else
#define cgpDEBUG(__fmt__, ...)
#endif

/* Static Variables *************************************************** */
#if (USE_GENERIC == 1)
static pfncInit pfInit = NULL;
static pfncOpen pfOpen = NULL;
static pfncRead pfRead = NULL;
static pfncWrite pfWrite = NULL;
static pfncClose pfClose = NULL;
static pfncFlush pfFlush = NULL;
static pfncFreeConfig pfFree = NULL;
static pfncDeInit pfDeinit = NULL;

static bool funcsRegister = false;

static pthread_mutex_t parserMutex = PTHREAD_MUTEX_INITIALIZER;
#define LOCK()  LOCK_MUTEX(parserMutex)
#define UNLOCK()  UNLOCK_MUTEX(parserMutex)
#define RET_DECLA() int ret;

#endif
/* Static Functions Declarcgpions ************************************* */

/* Global Variables *************************************************** */
static errcode_t iparserInit(const pfncInit pInit,
                             const pfncOpen pOpen,
                             const pfncRead pRead,
                             const pfncWrite pWrite,
                             const pfncClose pClose,
                             const pfncFlush pFlush,
                             const pfncFreeConfig pFree,
                             const pfncDeInit pDeinit,
                             void *(*pm)(size_t),
                             void (*pfr)(void *),
                             const void *data)
{
  errcode_t err;
  if (!pInit || !pOpen || !pRead || !pWrite || !pClose || !pFlush || !pDeinit
      || !pm || !pfr) {
    return E_INVP;
  }
  pfInit = pInit;
  pfOpen = pOpen;
  pfRead = pRead;
  pfWrite = pWrite;
  pfClose = pClose;
  pfFlush = pFlush;
  pfFree  = pFree;
  pfDeinit = pDeinit;
  funcsRegister = true;
  cgpDEBUG("parser Init\n");

  err = pfInit(pm, pfr, data);
  return err;
}

errcode_t parserInit(fileType_t t)
{
  RET_DECLA();
  errcode_t err;
  LOCK();
  switch (t) {
    case typeXML_e:
#if (XML_SUP == 1)
      err = iparserInit(xmlConfigInit,
                        xmlConfigOpen,
                        xmlConfigRead,
                        xmlConfigWrite,
                        xmlConfigClose,
                        xmlConfigFlush,
                        xmlConfigFreeConfig,
                        xmlConfigDeinit,
                        malloc,
                        free,
                        NULL);
#else
      err = E_NSPT;
#endif
      break;
    case typeJSON_e:
#if (JSON_SUP == 1)
      err = iparserInit(jsonConfigInit,
                        jsonConfigOpen,
                        jsonConfigRead,
                        jsonConfigWrite,
                        jsonConfigClose,
                        jsonConfigFlush,
                        jsonConfigFreeConfig,
                        jsonConfigDeinit,
                        malloc,
                        free,
                        NULL);
#else
      err = E_NSPT;
#endif
      break;
    default:
      err = E_NSPT;
      break;
  }
  UNLOCK();
  return err;
}

errcode_t confClearControlFields(void)
{
  RET_DECLA();
  errcode_t err;
  CHECK_REGS();
  LOCK();
  err = pfOpen(NULL, NULL, FL_CLEAR, NULL);
  UNLOCK();
  return err;
}

errcode_t setNetkeyValueToFile(uint8_t refId, const uint8_t *newKey)
{
  RET_DECLA();
  errcode_t err;
  CHECK_REGS();
  LOCK();
  err = pfWrite(netkeyValue_e, &refId, newKey);
  UNLOCK();
  return err;
}

errcode_t setNetkeyIdToFile(uint16_t refId, uint16_t newId)
{
  RET_DECLA();
  errcode_t err;
  CHECK_REGS();
  LOCK();
  err = pfWrite(netkeyId_e, &refId, &newId);
  UNLOCK();
  return err;
}

errcode_t setNetkeyDoneToFile(uint16_t id, uint8_t value)
{
  RET_DECLA();
  errcode_t err;
  CHECK_REGS();
  LOCK();
  err = pfWrite(netkeyDone_e, &id, &value);
  UNLOCK();
  return err;
}

errcode_t setAppkeyIdToFile(uint16_t refId, uint16_t newId)
{
  RET_DECLA();
  errcode_t err;
  CHECK_REGS();
  LOCK();
  err = pfWrite(appkeyId_e, &refId, &newId);
  UNLOCK();
  return err;
}

errcode_t setAppkeyDoneToFile(uint16_t id, uint8_t value)
{
  RET_DECLA();
  errcode_t err;
  CHECK_REGS();
  LOCK();
  err = pfWrite(appkeyDone_e, &id, &value);
  UNLOCK();
  return err;
}

errcode_t setNodeErrBitsToFile(uuid_128 uuid, uint32_t err)
{
  RET_DECLA();
  errcode_t err1;
  CHECK_REGS();
  LOCK();
  err1 = pfWrite(errBits_e, &uuid, &err);
  UNLOCK();
  return err1;
}

errcode_t setNodeUniAddrToFile(uuid_128 uuid, uint16_t addr)
{
  RET_DECLA();
  errcode_t err;
  CHECK_REGS();
  LOCK();
  err = pfWrite(unicastAddr_e, &uuid, &addr);
  UNLOCK();
  return err;
}

errcode_t setNodeBlacklistToFile(uuid_128 uuid, uint8_t value)
{
  RET_DECLA();
  errcode_t err;
  CHECK_REGS();
  LOCK();
  err = pfWrite(blacklist_e, &uuid, &value);
  UNLOCK();
  return err;
}

errcode_t setNodeDoneToFile(uuid_128 uuid, uint8_t value)
{
  RET_DECLA();
  errcode_t err;
  CHECK_REGS();
  LOCK();
  err = pfWrite(done_e, &uuid, &value);
  UNLOCK();
  return err;
}

errcode_t freeConfig(void)
{
  RET_DECLA();
  errcode_t err;
  CHECK_REGS();
  LOCK();
  err = pfFree(NULL);
  UNLOCK();
  return err;
}

errcode_t reloadNetworkConfig(networkConfig_t **pCfg)
{
  RET_DECLA();
  errcode_t err;
  CHECK_REGS();
  LOCK();
  err = pfOpen(NULL, NULL, 0, (void *)pCfg);
  UNLOCK();
  return err;
}

errcode_t loadNetworkConfig(networkConfig_t **pCfg,
                            const char *nodeConfigFilePath,
                            const char *templateFilePath,
                            bool createIfNotExist,
                            bool truncate)
{
  RET_DECLA();
  errcode_t err;
  unsigned int flags = 0;
  CHECK_REGS();
  LOCK();
  flags |= createIfNotExist ? FL_CREATE : 0;
  flags |= truncate ? FL_TRUNC : 0;

  err = pfOpen(nodeConfigFilePath, templateFilePath, flags, (void *)pCfg);
  UNLOCK();
  return err;
}

bool nodeIsRecorded(const uint8_t *uuid)
{
  RET_DECLA();
  int i = 0;
  uint8_t data[16];
  CHECK_REGS();
  LOCK();
  while (E_SUC == pfRead(iterateUUID, &i, data)) {
    i = 1;
    if (!memcmp(uuid, data, 16)) {
      UNLOCK();
      return true;
    }
  }
  UNLOCK();
  return false;
}

errcode_t addOneNodeToConfigFile(const uint8_t *uuid)
{
  RET_DECLA();
  errcode_t err;
  if (!uuid) {
    return E_INVP;
  }
  CHECK_REGS();
  LOCK();
  err = pfWrite(addItem_e, NULL, uuid);
  UNLOCK();
  return err;
}

errcode_t getNetworkConfig(void **configOut)
{
  RET_DECLA();
  errcode_t err;
  if (!configOut) {
    return E_INVP;
  }
  CHECK_REGS();
  LOCK();
  err = pfRead(wholeFile, NULL, configOut);
  UNLOCK();
  return err;
}

#define PUB_PERIOD_MS_MIN 100
#define PUB_PERIOD_MS_MAX 6300
#define PUB_PERIOD_SEC_MIN  1000
#define PUB_PERIOD_SEC_MAX  (63 * 1000)
#define PUB_PERIOD_10SEC_MIN  (10 * 1000)
#define PUB_PERIOD_10SEC_MAX  (63 * 10 * 1000)
#define PUB_PERIOD_MIN_MIN  (10 * 60 * 1000)
#define PUB_PERIOD_MIN_MAX  (63 * 10 * 60 * 1000)

int pub_period_sanity_check(uint32_t period)
{
  if ( (period >= PUB_PERIOD_MS_MIN && period <= PUB_PERIOD_MS_MAX)
       || (period >= PUB_PERIOD_SEC_MIN && period <= PUB_PERIOD_SEC_MAX)
       || (period >= PUB_PERIOD_10SEC_MIN && period <= PUB_PERIOD_10SEC_MAX)
       || (period >= PUB_PERIOD_MIN_MIN && period <= PUB_PERIOD_MIN_MAX)) {
    return 1;
  }
  return 0;
}
