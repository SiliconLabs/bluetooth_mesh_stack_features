/*************************************************************************
    > File Name: conf.c
    > Author: Kevin
    > Created Time: 2019-01-14
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"
#include "config.h"
#include "xml_parser.h"

/* Defines  *********************************************************** */
#define xmlASSERT(x)                  softASSERT((x))

#ifndef XML_DEBUG
#define ECHO_XML                      0
#define XML_DEBUG                     0
#endif

#if (XML_DEBUG == 1)
#define xmlDEBUG(__fmt__, ...)                dbgPrint(__fmt__, ##__VA_ARGS__)
#else
#define xmlDEBUG(__fmt__, ...)
#endif

/* Static Variables *************************************************** */
static networkConfig_t *pTmplConf = NULL;
#if (USE_GENERIC == 0)
static char fileName[200];
#endif

/* -------------------------------- */
static void *(*pmalloc)(size_t) = NULL;
static void (*pfree)(void *) = NULL;
static networkConfig_t *pNetConfig = NULL;
static char *template, *nodeconfig;
static xmlDocPtr fp = NULL;
static xmlNodePtr rootPtr = NULL;
static xmlNodePtr subnetPtr = NULL;
static xmlNodePtr nodesPtr = NULL;

/* Static Functions Declaractions ************************************* */

/* Global Variables *************************************************** */

/**
 * @brief	getChildrenCnt Get number of children with name #match
 *
 * @param	pIn pointer to XML Node
 * @param	match[] NULL to count all children, otherwise match the case
 *
 * @return number of children
 */
int getChildrenCnt(const xmlNodePtr pIn, const char match[])
{
  xmlNodePtr p = pIn;
  int ret = 0;
  if (!p || !p->children) {
    return 0;
  }
  p = p->children;

  do {
    if (p->type == XML_ELEMENT_NODE) {
      if (match == NULL) {
        ret++;
      } else if (!strcmp((char *)p->name, match)) {
        ret++;
      }
    }
    p = p->next;
  } while (p);

  return ret;
}

/**
 * @brief	getChild - get pointer of nearest child with node type
 *
 * @param	pIn - pointer to current node
 *
 * @return - pointer to child, NULL if no node type child
 */
xmlNodePtr getChild(const xmlNodePtr pIn)
{
  xmlNodePtr p = pIn;
  if (!p || !p->children) {
    return NULL;
  }
  p = p->children;
  while (p && p->type != XML_ELEMENT_NODE) {
    p = p->next;
  }
  return p;
}

/**
 * @brief	getNext - get next node
 *
 * @param	pIn - pointer to current node
 *
 * @return - pointer to next node, NULL if no node type node
 */
xmlNodePtr getNext(const xmlNodePtr pIn)
{
  xmlNodePtr p = pIn;
  if (!p) {
    return NULL;
  }
  do {
    p = p->next;
  } while (p && p->type != XML_ELEMENT_NODE);
  return p;
}

static errcode_t getProp(const xmlNodePtr pC,
                         const xmlChar *key,
                         void *data,
                         size_t length)
{
  xmlChar *pProp = NULL;
  int ret = 0;
  xmlASSERT(pC);
  xmlASSERT(key);

  pProp = xmlGetProp(pC, key);
  if (!pProp) {
    return E_NFND;
  }
  if (data) {
    ret = str2charBuf((char *)pProp, 0, (char *)data, length);
  }
  XML_FREE(pProp);

  return ret;
}

/**
 * @brief	loadTemplate - Load the template file
 *
 * @param	filename[] - template xml file path
 *
 * @return - 0 on success, #errcode_t on failure
 */
static errcode_t fxmlConfigLoadTemplate(void)
{
  xmlDocPtr doc = NULL;
  xmlNodePtr cur = NULL, pC = NULL, pCC = NULL;
  xmlChar *pProp = NULL;
  int ret = 0;

  hardASSERT(template);

  if ((doc = xmlReadFile(template, NULL, XML_PARSE_NOBLANKS)) == NULL) {
    xmlDEBUG("Open %s failed.\n", template);
    return E_IO;
  }

  if ((cur = xmlDocGetRootElement(doc)) == NULL) {
    xmlDEBUG("Root is empty\n");
    ret = E_FILE;
    goto end;
  }

  if (pTmplConf) {
    xmlConfigFreeConfig((void **)&pTmplConf);
  }

  pTmplConf = (networkConfig_t *)pmalloc(sizeof(networkConfig_t));
  memset(pTmplConf, 0, sizeof(networkConfig_t));

  softASSERT(!xmlStrcmp(cur->name, (xmlChar *)STR_TEMPLATES));
  pTmplConf->nodeCnt = getChildrenCnt(cur, NULL);
  if (pTmplConf->nodeCnt) {
    pTmplConf->pNodes = (nodeConf_t *)pmalloc(sizeof(nodeConf_t) * pTmplConf->nodeCnt);
    memset(pTmplConf->pNodes, 0, sizeof(nodeConf_t) * pTmplConf->nodeCnt);
  }
  pC = getChild(cur);

  for (int i = 0; i < pTmplConf->nodeCnt; i++) {
    /* Load Template ID */
    getProp(pC, BAD_CAST STR_ID, (void *)pTmplConf->pNodes[i].uuid.data, sizeof(uint8_t));
    /* Load TTL */
    getProp(pC, BAD_CAST STR_TTL, (void *)&pTmplConf->pNodes[i].ttl, sizeof(uint8_t));
    /* Load Feature bits */
    pProp = xmlGetProp(pC, (xmlChar *)STR_FEATURES);
    if (pProp) {
      strcpy(pTmplConf->pNodes[i].feature, (char *)pProp);
      XML_FREE(pProp);
    }
    /* Load Pub Address */
    getProp(pC, BAD_CAST STR_PUB, (void *)&pTmplConf->pNodes[i].pubAddr, sizeof(uint16_t));
    /* Load Pub Bind Key ID */
    getProp(pC, BAD_CAST STR_PUB_BIND, (void *)&pTmplConf->pNodes[i].pubBind, sizeof(uint8_t));
    /* Load transmission count */
    getProp(pC, BAD_CAST STR_TRANS_CNT, (void *)&pTmplConf->pNodes[i].transCnt, sizeof(uint8_t));
    /* Load transmission interval */
    getProp(pC, BAD_CAST STR_TRANS_INTERVAL, (void *)&pTmplConf->pNodes[i].transIntv, sizeof(uint16_t));
    pTmplConf->pNodes[i].transIntv *= 10;

    /* Load Bindings and Subs */
    pTmplConf->pNodes[i].bindingCnt = getChildrenCnt(pC, STR_BIND);
    pTmplConf->pNodes[i].subCnt = getChildrenCnt(pC, STR_SUB);
    if (pTmplConf->pNodes[i].bindingCnt) {
      pTmplConf->pNodes[i].bindings = (uint8_t *)pmalloc(sizeof(uint8_t) * pTmplConf->pNodes[i].bindingCnt);
      memset(pTmplConf->pNodes[i].bindings, 0, sizeof(uint8_t) * pTmplConf->pNodes[i].bindingCnt);
    }
    if (pTmplConf->pNodes[i].subCnt) {
      pTmplConf->pNodes[i].subAddrs = (uint16_t *)pmalloc(sizeof(uint16_t) * pTmplConf->pNodes[i].subCnt);
      memset(pTmplConf->pNodes[i].subAddrs, 0, sizeof(uint16_t) * pTmplConf->pNodes[i].subCnt);
    }
    pCC = getChild(pC);
    int b = 0, s = 0;
    for (int a = 0; a < pTmplConf->pNodes[i].bindingCnt + pTmplConf->pNodes[i].subCnt; a++) {
      if (!strcmp((char *)pCC->name, STR_BIND)) {
        softASSERT(!str2charBuf((char *)XML_GET_CONTENT(pCC->children),
                                0,
                                (char *)&pTmplConf->pNodes[i].bindings[b++],
                                sizeof(uint8_t)));
      } else if (!strcmp((char *)pCC->name, STR_SUB)) {
        softASSERT(!str2charBuf((char *)XML_GET_CONTENT(pCC->children),
                                0,
                                (char *)&pTmplConf->pNodes[i].subAddrs[s++],
                                sizeof(uint16_t)));
      }
      pCC = getNext(pCC);
    }
    pC = getNext(pC);
  }

  end:
  xmlFreeDoc(doc);
  xmlCleanupParser();
  xmlMemoryDump();
  return ret;
}

void xmlConfigDeinit(void)
{
  if (pNetConfig) {
    xmlConfigFreeConfig((void **)&pNetConfig);
  }
  if (pTmplConf) {
    xmlConfigFreeConfig((void **)&pTmplConf);
  }
  fp = NULL;
  rootPtr = NULL;
  subnetPtr = NULL;
  nodesPtr = NULL;
  if (template) {
    pfree(template);
    template = NULL;
  }
  if (nodeconfig) {
    pfree(nodeconfig);
    nodeconfig = NULL;
  }
}

errcode_t xmlConfigInit(void *(*pm)(size_t),
                        void (*pfr)(void *),
                        const void *data)
{
  if (!pm || !pfr) {
    return E_INVP;
  }

  pmalloc = pm;
  pfree = pfr;

  return E_SUC;
}

errcode_t xmlConfigFreeConfig(void **ppcfg)
{
  if (!pfree) {
    return E_NINIT;
  }
  if (ppcfg == NULL) {
    return E_INVP;
  }
#if 1
  networkConfig_t *p = *ppcfg;
  if (p) {
    if (p->pNodes) {
      for (int i = 0; i < p->nodeCnt; i++) {
        if (p->pNodes[i].bindings) {
          pfree(p->pNodes[i].bindings);
        }
        if (p->pNodes[i].subAddrs) {
          pfree(p->pNodes[i].subAddrs);
        }
      }
      pfree(p->pNodes);
    }
    pfree(p);
    *ppcfg = NULL;
  }
#else
  *ppcfg = NULL;
#endif
  return E_SUC;
}

static errcode_t setProp(int type,
                         const xmlNodePtr pC,
                         const char *key,
                         const char *field,
                         const char *value)
{
  xmlChar *pProp = NULL;
  int ret = E_NFND;
  xmlASSERT(pC);
  xmlASSERT(field);
  xmlASSERT(value);
  xmlASSERT(type <= NodeE);

  pProp = xmlGetProp(pC, BAD_CAST(type == NodeE ? STR_UUID : STR_DUMMY_ID));

  if (!pProp) {
    return E_NFND;
  }

  if (!xmlStrcmp(BAD_CAST key, pProp)) {
    if (NULL == xmlSetProp(pC, BAD_CAST field, BAD_CAST value)) {
      ret = E_UNSPEC;
    }
    ret = E_SUC;
  }

  XML_FREE(pProp);
  return ret;
}

static errcode_t fxmlConfigModifyFieldByKey(int type,
                                            int PropOrContent,
                                            const char *key,
                                            const char *field,
                                            const char *value)
{
  xmlNodePtr cur = NULL;
  int ret = E_NFND, found = 0;

  if (PropOrContent == contentE) {
    /* Not supported yet */
    return E_NSPT;
  }

  xmlASSERT(key);
  xmlASSERT(value);
  xmlASSERT(type <= NodeE);

  xmlASSERT(subnetPtr);
  xmlASSERT(nodesPtr);

  if (type == NodeE) {
    cur = getChild(nodesPtr);
  } else {
    cur = getChild(subnetPtr);
  }

  while (cur) {
    switch (type) {
      case netKeyE:
        if (xmlStrcmp(cur->name, BAD_CAST STR_NETKEY)) {
          cur = getNext(cur);
          continue;
        }
        if (E_SUC == (ret = setProp(type, cur, key, field, value))) {
          found = 1;
        }
        break;
      case appKeyE:
        if (!xmlStrcmp(cur->name, BAD_CAST STR_NETKEY)) {
          cur = getChild(cur);
          continue;
        } else if (!xmlStrcmp(cur->name, BAD_CAST STR_APPKEY)) {
          if (E_SUC == (ret = setProp(type, cur, key, field, value))) {
            found = 1;
          }
        }
        break;
      case NodeE:
        if (xmlStrcmp(cur->name, BAD_CAST STR_NODE)) {
          cur = getNext(cur);
          continue;
        }
        if (E_SUC == (ret = setProp(type, cur, key, field, value))) {
          found = 1;
        }
        break;
    }
    if (found) {
      break;
    }
    cur = getNext(cur);
  }

  if (ret == E_SUC) {
    ret = xmlConfigFlush();
  }
  return ret;
}

static errcode_t createNewXmlFile(void)
{
  int ret = E_SUC;

  xmlASSERT(nodeconfig);

  xmlDEBUG("create new xml file\n");
  fp = xmlNewDoc(BAD_CAST "1.0");
  if (fp == NULL) {
    return E_IO;
  }

  rootPtr = xmlNewNode(NULL, BAD_CAST "network");
  xmlDocSetRootElement(fp, rootPtr);

  /* Create subnets and nodes root node */
  subnetPtr = xmlNewChild(rootPtr, NULL, BAD_CAST STR_SUBNETS, NULL);
  if (subnetPtr == NULL) {
    return E_UNSPEC;
  }

  nodesPtr = xmlNewChild(rootPtr, NULL, BAD_CAST STR_NODES, NULL);
  if (nodesPtr == NULL) {
    return E_UNSPEC;
  }

  /* Create netkey and appkeys */
  xmlNodePtr netkeyNode = xmlNewChild(subnetPtr, NULL, BAD_CAST STR_NETKEY, NULL);
  if (netkeyNode == NULL) {
    return E_UNSPEC;
  }
  xmlNewProp(netkeyNode, BAD_CAST STR_DUMMY_ID, BAD_CAST "0000");
  xmlNewProp(netkeyNode, BAD_CAST STR_ID, BAD_CAST "0000");
  xmlNewProp(netkeyNode, BAD_CAST STR_VALUE, BAD_CAST DEFAULT_NETKEY);
  xmlNewProp(netkeyNode, BAD_CAST STR_DONE, BAD_CAST "00");

  xmlNodePtr appkeyNode = xmlNewChild(netkeyNode, NULL, BAD_CAST STR_APPKEY, NULL);
  if (netkeyNode == NULL) {
    return E_UNSPEC;
  }
  xmlNewProp(appkeyNode, BAD_CAST STR_DUMMY_ID, BAD_CAST "0000");
  xmlNewProp(appkeyNode, BAD_CAST STR_ID, BAD_CAST "0000");
  xmlNewProp(appkeyNode, BAD_CAST STR_VALUE, BAD_CAST DEFAULT_APPKEY);
  xmlNewProp(appkeyNode, BAD_CAST STR_DONE, BAD_CAST "00");

  if (E_SUC != (ret = xmlConfigFlush())) {
    return ret;
  }

  /* TODO - only for debug ease */
  if (-1 == chown(nodeconfig, 1000, 1000)) {
    perror("chown");
    CS_ERR("FAILED to change own of %s\n", nodeconfig);
  }

  pNetConfig = (networkConfig_t *)pmalloc(sizeof(networkConfig_t));
  memset(pNetConfig, 0, sizeof(networkConfig_t));
  pNetConfig->keys.netkey.done = 0;
  pNetConfig->keys.netkey.refId = 0;
  pNetConfig->keys.netkey.id = 0;
  softASSERT(!str2charBuf(DEFAULT_NETKEY,
                          0,
                          (char *)pNetConfig->keys.netkey.value,
                          16));
  pNetConfig->keys.appKeyCnt = 1;
  pNetConfig->keys.appKeys = (keyItem_t *)pmalloc(sizeof(keyItem_t));
  memset(pNetConfig->keys.appKeys, 0, sizeof(keyItem_t));
  pNetConfig->keys.appKeys[0].done = 0;
  pNetConfig->keys.appKeys[0].refId = 0;
  pNetConfig->keys.appKeys[0].id = 0;
  softASSERT(!str2charBuf(DEFAULT_APPKEY,
                          0,
                          (char *)pNetConfig->keys.appKeys[0].value,
                          16));
  return ret;
}

errcode_t xmlConfigFlush(void)
{
  xmlSaveFormatFileEnc(nodeconfig, fp, ENCODE_FMT, 1);
  return E_SUC;
}

/**
 * @brief	loadNodeConfFromTemplate - load a node configuration from template
 * file
 *
 * @param	pN - pointer to the node
 * @param	id - template ID
 *
 * @return - 0 on success, #errcode_t on failure
 */
static errcode_t fxmlConfigLoadNodeFromTemplate(nodeConf_t *pN,
                                                uint8_t id)
{
  uint8_t *pBindings = NULL;
  uint16_t *pSubAddrs = NULL;

  xmlASSERT(pN);
  /* The calling function should ensure pN != NULL */
  nodeConf_t *pTN = getTmplById(pTmplConf, id);
  if (pTN == NULL) {
    return E_NFND;
  }

  memcpy(pN, pTN, sizeof(nodeConf_t));

  if (pTN->bindingCnt) {
    pBindings = (uint8_t *)pmalloc(sizeof(uint8_t) * pTN->bindingCnt);
    pN->bindings = pBindings;
    memcpy(pN->bindings, pTN->bindings, sizeof(uint8_t) * pTN->bindingCnt);
  }
  if (pTN->subCnt) {
    pSubAddrs = (uint16_t *)pmalloc(sizeof(uint16_t) * pTN->subCnt);
    pN->subAddrs = pSubAddrs;
    memcpy(pN->subAddrs, pTN->subAddrs, sizeof(sizeof(uint16_t) * pTN->subCnt));
  }
  return E_SUC;
}
/**
 * @brief	fxmlConfigLoadAllNodes - load all node configurations
 *
 * @param	pConf - PTR to target
 * @param	cur - first node PTR
 *
 * @return - 0 on success, #errcode_t on failure
 */
static errcode_t fxmlConfigLoadAllNodes(networkConfig_t *pConf,
                                        xmlNodePtr cur,
                                        bool clearControlFields)
{
  xmlNodePtr pC = NULL, pCC = NULL;
  xmlChar *pProp = NULL;
  bool loadTmpl = 0;
  int ret = 0, cyc = 0;
  xmlASSERT(pConf);
  xmlASSERT(cur);
  xmlASSERT(pmalloc);
  xmlASSERT(pfree);

#if 1
  cyc = pConf->nodeCnt = getChildrenCnt(cur, NULL);
  if (cyc) {
    pConf->pNodes = (nodeConf_t *)pmalloc(sizeof(nodeConf_t) * pConf->nodeCnt);
    memset(pConf->pNodes, 0, sizeof(nodeConf_t) * pConf->nodeCnt);
  }

  pC = getChild(cur);

  for (int i = 0, j = 0; i < cyc; i++) {
    /* Load Nodes */
    /* pProp = xmlGetProp(pC, (xmlChar *)STR_TMPL); */
    uint8_t id = NODE_INVALID_FLAG;
    ret = getProp(pC, BAD_CAST STR_TMPL, &id, sizeof(uint8_t));
    if (ret == E_SUC) {
      if (!pTmplConf) {
        return E_INTNUL;
      }

      if (id == NODE_INVALID_FLAG) {
        pConf->nodeCnt--;
        pC = getNext(pC);
        continue;
      }

      if (E_SUC != (ret = fxmlConfigLoadNodeFromTemplate(&pConf->pNodes[j], id))) {
        return ret;
      }
      loadTmpl = 1;
    }

    /* Load TTL	 */
    getProp(pC, BAD_CAST STR_TTL, &pConf->pNodes[j].ttl, sizeof(uint8_t));
    /* Load Feature bits */
    pProp = xmlGetProp(pC, (xmlChar *)STR_FEATURES);
    if (loadTmpl == 0 || pProp != NULL) {
      strcpy(pConf->pNodes[j].feature, (char *)pProp);
    }
    XML_FREE(pProp);
    /* Load transmission count */
    getProp(pC, BAD_CAST STR_TRANS_CNT, &pConf->pNodes[j].transCnt, sizeof(uint8_t));
    /* Load transmission interval */
    getProp(pC, BAD_CAST STR_TRANS_INTERVAL, &pConf->pNodes[j].transIntv, sizeof(uint16_t));
    /* Load Pub address */
    getProp(pC, BAD_CAST STR_PUB, &pConf->pNodes[j].pubAddr, sizeof(uint16_t));
    /* Load Pub Bind Key ID */
    getProp(pC, BAD_CAST STR_PUB_BIND, &pConf->pNodes[j].pubBind, sizeof(uint8_t));

    /* Load Bindings and Subs */
    int bdCnt = getChildrenCnt(pC, STR_BIND);
    int subCnt = getChildrenCnt(pC, STR_SUB);
    if (bdCnt) {
      pConf->pNodes[j].bindingCnt = bdCnt;
      if (pConf->pNodes[j].bindings) {
        pfree(pConf->pNodes[j].bindings);
      }
      pConf->pNodes[j].bindings = (uint8_t *)pmalloc(sizeof(uint8_t) * pConf->pNodes[j].bindingCnt);
      memset(pConf->pNodes[j].bindings, 0, sizeof(uint8_t) * pConf->pNodes[j].bindingCnt);
    }

    if (subCnt) {
      pConf->pNodes[j].subCnt = subCnt;
      if (pConf->pNodes[j].subAddrs) {
        pfree(pConf->pNodes[j].subAddrs);
      }
      pConf->pNodes[j].subAddrs = (uint16_t *)pmalloc(sizeof(uint16_t) * pConf->pNodes[j].subCnt);
      memset(pConf->pNodes[j].subAddrs, 0, sizeof(uint16_t) * pConf->pNodes[j].subCnt);
    }

    pCC = getChild(pC);
    int b = 0, s = 0;
    for (int a = 0; a < bdCnt + subCnt; a++) {
      if (!strcmp((char *)pCC->name, STR_BIND)) {
        softASSERT(!str2charBuf((char *)XML_GET_CONTENT(pCC->children),
                                0,
                                (char *)&pConf->pNodes[j].bindings[b++],
                                sizeof(uint8_t)));
      } else if (!strcmp((char *)pCC->name, STR_SUB)) {
        softASSERT(!str2charBuf((char *)XML_GET_CONTENT(pCC->children),
                                0,
                                (char *)&pConf->pNodes[j].subAddrs[s++],
                                sizeof(uint16_t)));
      }
      pCC = getNext(pCC);
    }

    /* Load UUID */
    getProp(pC, BAD_CAST STR_UUID, pConf->pNodes[j].uuid.data, 16);

    if (clearControlFields) {
      xmlSetProp(pC, BAD_CAST STR_ADDR, BAD_CAST "0000");
      pNetConfig->pNodes[j].uniAddr = 0;
      xmlSetProp(pC, BAD_CAST STR_ERRBITS, BAD_CAST "00000000");
      pConf->pNodes[j].errBits = 0;
      xmlSetProp(pC, BAD_CAST STR_BL, BAD_CAST "00");
      pConf->pNodes[j].blacklist = 0;
      xmlSetProp(pC, BAD_CAST STR_DONE, BAD_CAST "00");
      pConf->pNodes[j].done = 0;
    } else {
      /* Load addr */
      getProp(pC, BAD_CAST STR_ADDR, &pConf->pNodes[j].uniAddr, sizeof(uint16_t));
      /* Load err */
      getProp(pC, BAD_CAST STR_ERRBITS, &pConf->pNodes[j].errBits, sizeof(uint32_t));
      /* Load blacklist */
      getProp(pC, BAD_CAST STR_BL, &pConf->pNodes[j].blacklist, sizeof(uint8_t));
      /* Load done */
      getProp(pC, BAD_CAST STR_DONE, &pConf->pNodes[j].done, sizeof(uint8_t));
    }

    pC = getNext(pC);
    j++;
  }
#endif
  return E_SUC;
}

static errcode_t fxmlConfigOpen(bool clearControlFields)
{
  int ret = E_SUC;
  xmlNodePtr cur = NULL, pC = NULL, pCC = NULL;

  if ((cur = xmlDocGetRootElement(fp)) == NULL) {
    xmlDEBUG("Root is empty\n");
    ret = E_FILE;
    goto fail;
  }

  pNetConfig = (networkConfig_t *)pmalloc(sizeof(networkConfig_t));
  memset(pNetConfig, 0, sizeof(networkConfig_t));

  rootPtr = cur;
  cur = getChild(cur);

  while (cur) {
    if (!strcmp((char *)cur->name, STR_SUBNETS)) {
      subnetPtr = cur;
      pC = getChild(cur);
      /* Load Network Key */
      /* Get the ref ID */
      getProp(pC, BAD_CAST STR_DUMMY_ID, &pNetConfig->keys.netkey.refId, sizeof(uint16_t));
      /* Get the ID */
      getProp(pC, BAD_CAST STR_ID, &pNetConfig->keys.netkey.id, sizeof(uint16_t));
      /* Get netkey value */
      getProp(pC, BAD_CAST STR_VALUE, pNetConfig->keys.netkey.value, 16);
      /* Get the done flag */
      if (clearControlFields) {
        xmlSetProp(pC, BAD_CAST STR_DONE, BAD_CAST "00");
        pNetConfig->keys.netkey.done = 0;
      } else {
        getProp(pC, BAD_CAST STR_DONE, &pNetConfig->keys.netkey.done, sizeof(uint8_t));
      }
      int appKeyCnt = getChildrenCnt(pC, NULL);
      pNetConfig->keys.appKeyCnt = appKeyCnt;
      if (appKeyCnt) {
        /* Load Application Keys */
        pNetConfig->keys.appKeys = (keyItem_t *)pmalloc(sizeof(keyItem_t) * appKeyCnt);
        memset(pNetConfig->keys.appKeys, 0, sizeof(keyItem_t) * appKeyCnt);
        pCC = getChild(pC);
        for (int a = 0; a < appKeyCnt; a++) {
          /* Get the ref ID */
          getProp(pCC, BAD_CAST STR_DUMMY_ID, &pNetConfig->keys.appKeys[a].refId, sizeof(uint16_t));
          /* Get the ID */
          getProp(pCC, BAD_CAST STR_ID, &pNetConfig->keys.appKeys[a].id, sizeof(uint16_t));
          /* Get appkey value */
          getProp(pCC, BAD_CAST STR_VALUE, pNetConfig->keys.appKeys[a].value, 16);
          /* Get the done flag */
          if (clearControlFields) {
            xmlSetProp(pCC, BAD_CAST STR_DONE, BAD_CAST "00");
            pNetConfig->keys.appKeys[a].done = 0;
          } else {
            getProp(pCC, BAD_CAST STR_DONE, &pNetConfig->keys.appKeys[a].done, sizeof(uint8_t));
          }
          pCC = getNext(pCC);
        }
      }
    } else if (!strcmp((char *)cur->name, STR_NODES)) {
      nodesPtr = cur;
      if (E_SUC != (ret = fxmlConfigLoadAllNodes(pNetConfig, cur, clearControlFields))) {
        goto fail;
      }
    } else {
      /* XML format error	 */
      ret = E_FILE;
      goto fail;
    }
    cur = getNext(cur);
  }

  int tmp = 0;
  for (int i = 0; i < pNetConfig->keys.appKeyCnt; i++) {
    if (pNetConfig->keys.appKeys[i].done) {
      tmp++;
    }
  }
  pNetConfig->keys.activeAppKeyCnt = tmp;

  int found = 0;
  /* Check all the bindings in template are valid */
  for (int i = 0; i < pNetConfig->nodeCnt; i++) {
    for (int j = 0; j < pNetConfig->pNodes[i].bindingCnt; j++) {
      found = 0;
      for (int k = 0; k < pNetConfig->keys.appKeyCnt; k++) {
        if (pNetConfig->pNodes[i].bindings[j] == pNetConfig->keys.appKeys[k].refId) {
          found = 1;
          break;
        }
      }
      if (!found) {
        CS_ERR("Node[i-%d, j-%d] contains INVALID ref ID of bindings. Exiting\n", i, j);
        exit(EXIT_FAILURE);
      }
    }
  }

  if (clearControlFields) {
    xmlConfigFlush();
  }
  return E_SUC;

  fail:
  if (pNetConfig) {
    xmlConfigFreeConfig((void **)&pNetConfig);
  }
  return ret;
}

static errcode_t xmlConfigAddNodeItem(const uint8_t *uuidData)
{
  char uuidStr[33];
  int ret = 0;
  xmlNodePtr pNode = NULL;

  xmlASSERT(uuidData);
  xmlASSERT(nodesPtr);

  memset(uuidStr, 0, 33);

  if (E_SUC != (ret = charBuf2str((char *)uuidData, 16, 0, uuidStr, 33))) {
    return ret;
  }

  pNode = xmlNewChild(nodesPtr, NULL, BAD_CAST STR_NODE, NULL);
  xmlNewProp(pNode, BAD_CAST STR_UUID, BAD_CAST uuidStr);
  xmlNewProp(pNode, BAD_CAST STR_ADDR, BAD_CAST "0000");
  xmlNewProp(pNode, BAD_CAST STR_ERRBITS, BAD_CAST "00000000");
  xmlNewProp(pNode, BAD_CAST STR_TMPL, BAD_CAST "FF");
  xmlNewProp(pNode, BAD_CAST STR_BL, BAD_CAST "00");
  xmlNewProp(pNode, BAD_CAST STR_DONE, BAD_CAST "00");

  if (E_SUC != (ret = xmlConfigFlush())) {
    return ret;
  }
#if (ECHO_XML == 1)
  xmlSaveFormatFileEnc("-", pXml, ENCODE_FMT, 1);
#endif
  if (E_SUC != (ret = fxmlConfigOpen(false))) {
    return ret;
  }

  return E_SUC;
}

errcode_t xmlConfigOpen(const char nodeConFilePath[],
                        const char templateFilePath[],
                        unsigned int flags,
                        void **data)
{
  int ret = E_SUC;

  if (!pmalloc || !pfree) {
    return E_NINIT;
  }

  if (!(flags & FL_CLEAR)) {
    /* if (!nodeConFilePath || !data) { */
    if (!nodeConFilePath) {
      if (!nodeconfig) {
        return E_INVP;
      }
    } else {
      if (nodeconfig) {
        pfree(nodeconfig);
        nodeconfig = NULL;
      }
      nodeconfig = pmalloc(strlen(nodeConFilePath) + 1);
      strcpy(nodeconfig, nodeConFilePath);
      nodeconfig[strlen(nodeConFilePath)] = '\0';
    }
  } else if (!nodeconfig) {
    return E_INVP;
  }

  if (pNetConfig) {
    xmlConfigFreeConfig((void **)&pNetConfig);
  }

  if (templateFilePath) {
    if (template) {
      pfree(template);
      template = NULL;
    }
    template = pmalloc(strlen(templateFilePath) + 1);
    strcpy(template, templateFilePath);
    template[strlen(templateFilePath)] = '\0';

    if (E_SUC != (ret = fxmlConfigLoadTemplate())) {
      return ret;
    }
  }

  if (fp) {
    xmlConfigClose();
  }
  fp = xmlReadFile(nodeconfig, NULL, XML_PARSE_NOBLANKS);

  if (!fp ) {
    if (!(flags & FL_CREATE)) {
      ret = E_NEXIST;
      goto fail;
    }
    if (E_SUC != (ret = createNewXmlFile())) {
      goto fail;
    }
  } else if (flags & FL_TRUNC) {
    if (E_SUC != (ret = createNewXmlFile())) {
      goto fail;
    }
  } else {
    if (E_SUC != (ret = fxmlConfigOpen((flags & FL_CLEAR) ? true : false))) {
      goto fail;
    }
  }

  if (data) {
    *data = pNetConfig;
  }
  return E_SUC;

  fail:
  if (pTmplConf) {
    xmlConfigFreeConfig((void **)&pTmplConf);
  }
  if (pNetConfig) {
    xmlConfigFreeConfig((void **)&pNetConfig);
  }
  fp = NULL;
  rootPtr = NULL;
  subnetPtr = NULL;
  nodesPtr = NULL;
  xmlDEBUG("xml Open failed, err code %d\n", ret);
  return ret;
}

errcode_t xmlConfigRead(readType_t rdt,
                        const void *key,
                        void *data)
{
  int ret = E_SUC, i = 0;
  static int seek = 0;
  xmlNodePtr pC = NULL;

  if (!data) {
    return E_INVP;
  }

  if (!pNetConfig) {
    xmlDEBUG("Open the xml file first.\n");
    return E_INTNUL;
  }

  switch (rdt) {
    case wholeFile:
      *(networkConfig_t **)data = pNetConfig;
      break;
    case iterateUUID:
      if (!*(int *)key) {
        seek = 0;
      }
      pC = getChild(nodesPtr);
      for ( i = 0; i < getChildrenCnt(nodesPtr, STR_NODE); i++) {
        if (i == seek) {
          /* Load UUID */
          getProp(pC, BAD_CAST STR_UUID, data, 16);
          seek++;
          return E_SUC;
        }
        pC = getNext(pC);
      }
      ret = E_NFND;
      break;
    case subnetCnt:
      /* TODO - currently only 1 network key support */
      *(size_t *)data = 1;
      break;
    case appkeyCnt:
      /* TODO - currently only 1 network key support */
      *(size_t *)data = pNetConfig->keys.appKeyCnt;
      break;
    case nodeCnt:
      *(size_t *)data = pNetConfig->nodeCnt;
      break;
    default:
      ret = E_NIMPL;
      break;
  }

  return ret;
}

errcode_t xmlConfigWrite(writeType_t wrt,
                         const void *key,
                         const void *data)
{
  char keyBuf[33], valueBuf[33], *pfield = NULL;;
  int type = 0;

  if (!data || (!key && wrt != addItem_e)) {
    return E_INVP;
  }

  if (!pNetConfig) {
    xmlDEBUG("Open the xml file first.\n");
    return E_INTNUL;
  }

  memset(keyBuf, 0, 33);
  memset(valueBuf, 0, 33);

  switch (wrt) {
    case addItem_e:
      return xmlConfigAddNodeItem((const uint8_t *)data);
      break;
    case netkeyDone_e:
      uint16ToStr(*(uint16_t *)key, keyBuf);
      uint8ToStr(*(uint8_t *)data, valueBuf);
      pfield = STR_DONE;
      type = netKeyE;
      break;
    case netkeyId_e:
      uint16ToStr(*(uint16_t *)key, keyBuf);
      uint16ToStr(*(uint16_t *)data, valueBuf);
      pfield = STR_ID;
      type = netKeyE;
      break;
    case netkeyValue_e:
      uint16ToStr(*(uint16_t *)key, keyBuf);
      charBuf2str(data, 16, 0, valueBuf, 32);
      pfield = STR_VALUE;
      type = netKeyE;
      break;
    case appkeyDone_e:
      uint16ToStr(*(uint16_t *)key, keyBuf);
      uint8ToStr(*(uint8_t *)data, valueBuf);
      pfield = STR_DONE;
      type = appKeyE;
      break;
    case appkeyId_e:
      uint16ToStr(*(uint16_t *)key, keyBuf);
      uint16ToStr(*(uint16_t *)data, valueBuf);
      pfield = STR_ID;
      type = appKeyE;
      break;
    case errBits_e:
      charBuf2str((const char *)(((uuid_128 *)key)->data), 16, 0, keyBuf, 32);
      uint32ToStr(*(uint32_t *)data, valueBuf);
      pfield = STR_ERRBITS;
      type = NodeE;
      break;
    case unicastAddr_e:
      charBuf2str((const char *)(((uuid_128 *)key)->data), 16, 0, keyBuf, 32);
      uint16ToStr(*(uint16_t *)data, valueBuf);
      pfield = STR_ADDR;
      type = NodeE;
      break;
    case blacklist_e:
      charBuf2str((const char *)(((uuid_128 *)key)->data), 16, 0, keyBuf, 32);
      uint8ToStr(*(uint8_t *)data, valueBuf);
      pfield = STR_BL;
      type = NodeE;
      break;
    case done_e:
      charBuf2str((const char *)(((uuid_128 *)key)->data), 16, 0, keyBuf, 32);
      uint8ToStr(*(uint8_t *)data, valueBuf);
      pfield = STR_DONE;
      type = NodeE;
      break;
  }

  return fxmlConfigModifyFieldByKey(type, propE, keyBuf, pfield, valueBuf);
}

errcode_t xmlConfigClose(void)
{
  if (fp) {
    xmlFreeDoc(fp);
    xmlCleanupParser();
    xmlMemoryDump();
  }

  return E_SUC;
}
