/*************************************************************************
    > File Name: xml_parser.h
    > Author: Kevin
    > Created Time: 2019-03-06
    >Description:
 ************************************************************************/

#ifndef CONF_PARSER_H
#define CONF_PARSER_H

#include <stddef.h>
#include <stdint.h>
#include "prov_assert.h"
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include "config.h"
#include "conf_generic_parser.h"
#include "bg_types.h"
#include "err_code.h"

#define IO_BUF_SIZE                       256

enum {
  propE,
  contentE
};

#define XML_FREE(x)               if ((x)) { xmlFree((x)); (x) = NULL; }

xmlNodePtr getNext(const xmlNodePtr pIn);
int getChildrenCnt(const xmlNodePtr pIn, const char match[]);
xmlNodePtr getChild(const xmlNodePtr pIn);
#if (USE_GENERIC == 0)
int confClearControlFields(void);
int setNodeErrBitsToFile(uuid_128 uuid, uint32_t err);
int setNodeUniAddrToFile(uuid_128 uuid, uint16_t addr);
int setAppkeyDoneToFile(uint16_t id, uint8_t value);
int setNetkeyDoneToFile(uint16_t id, uint8_t value);
int setNodeBlacklistToFile(uuid_128 uuid, uint8_t value);
int setNodeDoneToFile(uuid_128 uuid, uint8_t value);
int setNetkeyIdToFile(uint16_t oldId, uint16_t newId);
int setAppkeyIdToFile(uint16_t oldId, uint16_t newId);
void freeConfig(networkConfig_t **ppCfg);
int loadTemplate(const char filename[]);
int loadKeysNodesFromXML(const char filename[], networkConfig_t **pCfg);
#endif

static inline nodeConf_t *getTmplById(const networkConfig_t *pTmplConf, uint8_t id)
{
  for (int i = 0; i < pTmplConf->nodeCnt; i++) {
    if (id == pTmplConf->pNodes[i].uuid.data[0]) {
      return &pTmplConf->pNodes[i];
    }
  }
  return NULL;
}

/* ------------------------------------- */

errcode_t xmlConfigInit(void *(*pm)(size_t),
                        void (*pfr)(void *),
                        const void *data);
void xmlConfigDeinit(void);
errcode_t xmlConfigFreeConfig(void **ppcfg);
errcode_t xmlConfigFlush(void);
errcode_t xmlConfigOpen(const char nodeConFilePath[],
                        const char templateFilePath[],
                        unsigned int flags,
                        void **data);
errcode_t xmlConfigRead(readType_t rdt,
                        const void *key,
                        void *data);
errcode_t xmlConfigWrite(writeType_t wrt,
                         const void *key,
                         const void *data);
errcode_t xmlConfigClose(void);

#endif
