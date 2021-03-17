/*************************************************************************
    > File Name: json_parser.c
    > Author: Kevin
    > Created Time: 2019-05-11
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include "json_parser.h"

#include "prov_assert.h"
#include "utils.h"
#include "err_code.h"
#include <json.h>

/* Defines  *********************************************************** */
#ifndef JSON_ECHO_DBG
#define JSON_ECHO_DBG 0
#endif

enum {
  config_min_em = -1,
  nodeUUID_em = 0,
  nodeRefId_em = 1,
  nodeTTL_em = 2,
  nodePub_em = 3,
  nodeFeatures_em = 4,
  nodeLPN_em = 5,
  nodeRelayRetranCount_em = 6,
  nodeRelayRetranInterval_em = 7,
  nodeProxy_em = 8,
  nodeFriend_em = 9,
  nodeRelay_em = 10,
  nodePubAddr_em = 11,
  nodePubKey_em = 12,
  nodeBind_em = 13,
  nodeSub_em = 14,
  nodeNetRetran_em = 15,
  nodeNetRetranCount_em = 16,
  nodeNetRetranInterval_em = 17,
  nodeSNB_em = 18,
  nodeDone_em = 19,
  nodeBL_em = 20,
  nodeErr_em = 21,
  nodeAddr_em = 22,
  nodePubPeriod_em = 23,

  keyRefId_em = 50,
  keyId_em = 51,
  keyValue_em = 52,
  keyDone_em = 53,
  keyAppKey_em = 54,
  config_max_em
};

typedef void (*pfuncValueFound)(networkConfig_t *pconfig,
                                int nodeIndex,
                                json_object *obj);
#define PARAM_CHECK() \
  hardASSERT(ptarget && obj && which > config_min_em && which < config_max_em)

typedef struct field {
  const char *strKey;
  int configIndex;
} field_t;

/* Global Variables *************************************************** */
static const field_t KEY_keys[] = {
  { STR_DUMMY_ID, keyRefId_em },
  { STR_ID, keyId_em },
  { STR_VALUE, keyValue_em },
  { STR_DONE, keyDone_em },
  { STR_APPKEY, keyAppKey_em }
};

static const field_t NODE_Keys[] = {
  { STR_UUID, nodeUUID_em },
  { STR_ID, nodeRefId_em },
  { STR_TTL, nodeTTL_em },
  { STR_FEATURES, nodeFeatures_em },
  { STR_SECURE_NETWORK_BEACON, nodeSNB_em },
  { STR_PUB, nodePub_em },
  { STR_BIND, nodeBind_em },
  { STR_SUB, nodeSub_em },
  { STR_NET_RETRAN, nodeNetRetran_em },
  { STR_DONE, nodeDone_em },
  { STR_BL, nodeBL_em },
  { STR_ERRBITS, nodeErr_em },
  { STR_ADDR, nodeAddr_em }
};

#define KEY_KEYS_NUM() sizeof(KEY_keys) / sizeof(field_t)
#define NODE_KEYS_NUM() sizeof(NODE_Keys) / sizeof(field_t)

/* Static Variables *************************************************** */
static int clear = 0;
static networkConfig_t *pNetConfig = NULL, *pTmplConf = NULL;
static char *template, *nodeconfig;
static void *(*pmalloc)(size_t) = malloc;
static void (*pfree)(void *) = free;

static json_object *rootPtr = NULL;
static json_object *subnetPtr = NULL;
static json_object *nodesPtr = NULL;
static json_object *templatePtr = NULL;

/* static json_object_iter iter; */
/* Static Functions Declaractions ************************************* */
static void fjsonConfigLoadAllKeys(networkConfig_t *pconfig,
                                   json_object *obj);
static void fjsonConfigLoadAllNodes(networkConfig_t *pconfig,
                                    json_object *obj);
static errcode_t fjsonConfigLoadTemplate(void);
static void storeConfig(int which,
                        void *ptarget,
                        int nodeIndex,
                        json_object *obj,
                        json_object *parent);
static void fjsonConfigLoadAllAppKeys(keys_t *keys,
                                      json_object *obj);
static errcode_t jsonConfigAddNodeItem(const uint8_t *uuidData);
static void compact_nodes(networkConfig_t *pconfig,
                          size_t node_amount);

errcode_t jsonConfigInit(void *(*pm)(size_t),
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

void jsonConfigDeinit(void)
{
  if (pNetConfig) {
    jsonConfigFreeConfig((void **)&pNetConfig);
  }
  if (pTmplConf) {
    jsonConfigFreeConfig((void **)&pTmplConf);
  }
  jsonConfigClose();
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

static errcode_t setProp(int type,
                         json_object *pC,
                         const char *key,
                         const char *field,
                         const char *value)
{
  char *pProp = NULL;
  int ret = E_NFND;
  json_object *tmp = NULL;
  softASSERT(pC);
  softASSERT(field);
  softASSERT(value);
  softASSERT(type <= NodeE);

  if (json_object_object_get_ex(pC, type == NodeE ? STR_UUID : STR_DUMMY_ID, &tmp)) {
    pProp = (char *)json_object_get_string(tmp);
    if (type != NodeE) {
      pProp += 2;
    }
  } else {
    return E_NFND;
  }

  if (!strcmp(key, pProp)) {
    json_object_object_add(pC, field, json_object_new_string(value));
    ret = E_SUC;
  }

  return ret;
}

static errcode_t fjsonConfigModifyFieldByKey(int type,
                                             const char *key,
                                             const char *field,
                                             const char *value)
{
  json_object *iter = NULL, *tmp = NULL;
  int ret = E_NFND, i = 0;

  softASSERT(key);
  softASSERT(value);
  softASSERT(type <= NodeE);

  softASSERT(subnetPtr);
  softASSERT(nodesPtr);

  switch (type) {
    case netKeyE:
      if (NULL != (iter = json_object_array_get_idx(subnetPtr, 0))) {
        if (E_SUC == (ret = setProp(type, iter, key, field, value))) {
        }
      }
      break;
    case appKeyE:
      if (NULL != (iter = json_object_array_get_idx(subnetPtr, 0))) {
        if (json_object_object_get_ex(iter, STR_APPKEY, &tmp)) {
          for (i = 0; i < json_object_array_length(tmp); i++) {
            iter = json_object_array_get_idx(tmp, i);
            if (E_SUC == (ret = setProp(type, iter, key, field, value))) {
              break;
            }
          }
        }
      }
      break;
    case NodeE:
      for (i = 0; i < json_object_array_length(nodesPtr); i++) {
        iter = json_object_array_get_idx(nodesPtr, i);
        if (E_SUC == (ret = setProp(type, iter, key, field, value))) {
          break;
        }
      }
      break;
    default:
      hardASSERT(0);
      break;
  }

  if (ret == E_SUC) {
    ret = jsonConfigFlush();
  }
  return ret;
}

errcode_t jsonConfigFreeConfig(void **ppcfg)
{
  if (!pfree) {
    return E_NINIT;
  }
  if (ppcfg == NULL) {
    return E_INVP;
  }

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
  return E_SUC;
}

errcode_t jsonConfigFlush(void)
{
  json_object_to_file_ext(nodeconfig, rootPtr, JSON_C_TO_STRING_PRETTY);
  return E_SUC;
}

errcode_t jsonConfigClose(void)
{
  json_object_put(rootPtr);
  rootPtr = NULL;
  return E_SUC;
}

static errcode_t createNewJsonFile(void)
{
  int ret = 0;
  json_object *array = NULL, *tmp = NULL, *tmp1 = NULL;
  hardASSERT(nodeconfig);

  if (rootPtr) {
    jsonConfigClose();
  }

  rootPtr = json_object_new_object();
  subnetPtr = json_object_new_array();
  nodesPtr = json_object_new_array();
  if (!subnetPtr || !rootPtr || !nodesPtr) {
    return E_UNSPEC;
  }
  tmp = json_object_new_object();
  json_object_object_add(tmp, STR_DUMMY_ID, json_object_new_string("0x0000"));
  json_object_object_add(tmp, STR_ID, json_object_new_string("0x0000"));
  json_object_object_add(
    tmp,
    STR_VALUE,
    json_object_new_string("0x03030303030303030303030303030303"));
  json_object_object_add(tmp, STR_DONE, json_object_new_string("0x00"));
  array = json_object_new_array();
  tmp1 = json_object_new_object();
  json_object_object_add(tmp1, STR_DUMMY_ID, json_object_new_string("0x0000"));
  json_object_object_add(tmp1, STR_ID, json_object_new_string("0x0000"));
  json_object_object_add(
    tmp1,
    STR_VALUE,
    json_object_new_string("0x03030303030303030303030303030303"));
  json_object_object_add(tmp1, STR_DONE, json_object_new_string("0x00"));
  json_object_array_add(array, tmp1);
  json_object_object_add(tmp, STR_APPKEY, array);
  json_object_array_add(subnetPtr, tmp);

  json_object_object_add(rootPtr, STR_SUBNETS, subnetPtr);
  json_object_object_add(rootPtr, STR_NODES, nodesPtr);

  if (E_SUC != (ret = jsonConfigFlush())) {
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
  return E_SUC;
}

static errcode_t fjsonConfigOpen(bool clearControlFields)
{
  int i = 0, j = 0, k = 0, found = 0;
  size_t len = 0;

  hardASSERT(rootPtr);

  clear = clearControlFields;

  /* TODO */
  FREE_AND_NEW_CONF(pNetConfig);
  json_object_object_foreach(rootPtr, key, val){
    if (!memcmp(STR_SUBNETS, key, strlen(STR_SUBNETS))) {
      subnetPtr = val;
      fjsonConfigLoadAllKeys(pNetConfig, val);
    } else if (!memcmp(STR_NODES, key, strlen(STR_NODES))) {
      nodesPtr = val;
      fjsonConfigLoadAllNodes(pNetConfig, val);
      len = json_object_array_length(val);
      compact_nodes(pNetConfig, len);
    }
  }

  pNetConfig->keys.activeAppKeyCnt = 0;
  for (i = 0; i < pNetConfig->keys.appKeyCnt; i++) {
    if (pNetConfig->keys.appKeys[i].done) {
      pNetConfig->keys.activeAppKeyCnt++;
    }
  }

  /* Check all the bindings in template are valid */
  for (i = 0; i < pNetConfig->nodeCnt; i++) {
    for (j = 0; j < pNetConfig->pNodes[i].bindingCnt; j++) {
      found = 0;
      for (k = 0; k < pNetConfig->keys.appKeyCnt; k++) {
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
    jsonConfigFlush();
  }
  clear = 0;
  return E_SUC;

  /* fail: */
  /* FREE_CONF(pNetConfig); */
  /* clear = 0; */
  /* return ret; */
}

errcode_t jsonConfigRead(readType_t rdt,
                         const void *key,
                         void *data)
{
  int ret = E_SUC;
  static int seek = 0;
  json_object *pC = NULL, *tmp = NULL;
  char *val = NULL;

  if (!data) {
    return E_INVP;
  }

  if (!pNetConfig) {
    CS_ERR("Open the json file first.\n");
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
      if (NULL == (pC = json_object_array_get_idx(nodesPtr, seek++))) {
        return E_NFND;
      }

      if (json_object_object_get_ex(pC, STR_UUID, &tmp)) {
        val = (char *)json_object_get_string(tmp);
      } else {
        return E_FILE;
      }

      if (E_SUC != (ret = str2charBuf(val, 0, data, 16))) {
        CS_ERR("ret = %d, val = %s\n", ret, val);
        softASSERT(0);
        return ret;
      }
      return E_SUC;
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

errcode_t jsonConfigWrite(writeType_t wrt,
                          const void *key,
                          const void *data)
{
  char keyBuf[33], valueBuf[33 + 2], *pfield = NULL;;
  int type = 0;

  if (!data || (!key && wrt != addItem_e)) {
    return E_INVP;
  }

  if (!pNetConfig) {
    CS_ERR("Open the json file first.\n");
    return E_INTNUL;
  }

  memset(keyBuf, 0, 33);
  memset(valueBuf, 0, 33 + 2);

  switch (wrt) {
    case addItem_e:
      return jsonConfigAddNodeItem((const uint8_t *)data);
      break;
    case netkeyDone_e:
      uint16ToStr(*(uint16_t *)key, keyBuf);
      uint8ToStr(*(uint8_t *)data, valueBuf + 2);
      pfield = STR_DONE;
      type = netKeyE;
      break;
    case netkeyId_e:
      uint16ToStr(*(uint16_t *)key, keyBuf);
      uint16ToStr(*(uint16_t *)data, valueBuf + 2);
      pfield = STR_ID;
      type = netKeyE;
      break;
    case netkeyValue_e:
      uint16ToStr(*(uint16_t *)key, keyBuf);
      charBuf2str(data, 16, 0, valueBuf + 2, 32);
      pfield = STR_VALUE;
      type = netKeyE;
      break;
    case appkeyDone_e:
      uint16ToStr(*(uint16_t *)key, keyBuf);
      uint8ToStr(*(uint8_t *)data, valueBuf + 2);
      pfield = STR_DONE;
      type = appKeyE;
      break;
    case appkeyId_e:
      uint16ToStr(*(uint16_t *)key, keyBuf);
      uint16ToStr(*(uint16_t *)data, valueBuf + 2);
      pfield = STR_ID;
      type = appKeyE;
      break;
    case errBits_e:
      charBuf2str((const char *)(((uuid_128 *)key)->data), 16, 0, keyBuf, 32);
      uint32ToStr(*(uint32_t *)data, valueBuf + 2);
      pfield = STR_ERRBITS;
      type = NodeE;
      break;
    case unicastAddr_e:
      charBuf2str((const char *)(((uuid_128 *)key)->data), 16, 0, keyBuf, 32);
      uint16ToStr(*(uint16_t *)data, valueBuf + 2);
      pfield = STR_ADDR;
      type = NodeE;
      break;
    case blacklist_e:
      charBuf2str((const char *)(((uuid_128 *)key)->data), 16, 0, keyBuf, 32);
      uint8ToStr(*(uint8_t *)data, valueBuf + 2);
      pfield = STR_BL;
      type = NodeE;
      break;
    case done_e:
      charBuf2str((const char *)(((uuid_128 *)key)->data), 16, 0, keyBuf, 32);
      uint8ToStr(*(uint8_t *)data, valueBuf + 2);
      pfield = STR_DONE;
      type = NodeE;
      break;
  }
  valueBuf[0] = '0';
  valueBuf[1] = 'x';

  return fjsonConfigModifyFieldByKey(type, keyBuf, pfield, valueBuf);
}

errcode_t jsonConfigOpen(const char nodeConFilePath[],
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
    jsonConfigFreeConfig((void **)&pNetConfig);
  }

  if (templateFilePath) {
    if (template) {
      pfree(template);
      template = NULL;
    }
    template = pmalloc(strlen(templateFilePath) + 1);
    strcpy(template, templateFilePath);
    template[strlen(templateFilePath)] = '\0';

    if (E_SUC != (ret = fjsonConfigLoadTemplate())) {
      return ret;
    }
  }

  if (rootPtr) {
    jsonConfigClose();
  }

  if (-1 == access(nodeconfig, F_OK)) {
    if (!(flags & FL_CREATE)) {
      ret = E_NEXIST;
      goto fail;
    }
    if (E_SUC != (ret = createNewJsonFile())) {
      goto fail;
    }
  } else {
    rootPtr = json_object_from_file(nodeconfig);
    errASSERT(rootPtr, "Json File Format ERROR\n");
    if (flags & FL_TRUNC) {
      if (E_SUC != (ret = createNewJsonFile())) {
        goto fail;
      }
    } else {
      if (E_SUC != (ret = fjsonConfigOpen((flags & FL_CLEAR) ? true : false))) {
        goto fail;
      }
    }
  }

  if (data) {
    *data = pNetConfig;
  }
  return E_SUC;

  fail:
  jsonConfigDeinit();
  CS_ERR("json Open failed, err code %d - %s\n", ret, ERR_NAME[ret]);
  return ret;
}

static void toJsonStr(const char *in,
                      size_t inLen,
                      char *out,
                      size_t outLen,
                      int prefix)
{
  int ret = 0;
  char *pout, outLength;
  pout = prefix ? out + 2 : out;
  outLength = prefix ? outLen - 2 : outLen;

  ret = charBuf2str(in, inLen, 0, pout, outLength);
  hardASSERT(ret == E_SUC);
  if (prefix) {
    out[0] = '0';
    out[1] = 'x';
  }
}

static errcode_t jsonConfigAddNodeItem(const uint8_t *uuidData)
{
  char uuidStr[33];
  int ret = 0;
  json_object *jObNode = NULL;

  hardASSERT(uuidData);
  hardASSERT(nodesPtr);

  memset(uuidStr, 0, 33);

  toJsonStr((char *)uuidData, 16, uuidStr, 33, 0);

  jObNode = json_object_new_object();
  json_object_object_add(jObNode, "UUID", json_object_new_string(uuidStr));
  json_object_object_add(jObNode, "Addr", json_object_new_string("0x0000"));
  json_object_object_add(jObNode, "Err", json_object_new_string("0x00000000"));
  json_object_object_add(jObNode, "Tmpl", json_object_new_string("0xFF"));
  json_object_object_add(jObNode, "Blacklist", json_object_new_string("0x00"));
  json_object_object_add(jObNode, "Done", json_object_new_string("0x00"));

  json_object_array_add(nodesPtr, jObNode);
#if (JSON_ECHO_DBG == 1)
  CS_LOG("new json node: %s\n", json_object_to_json_string(jObNode));
#endif

  if (E_SUC != (ret = jsonConfigFlush())) {
    return ret;
  }

  if (E_SUC != (ret = fjsonConfigOpen(false))) {
    return ret;
  }

  return E_SUC;
}

void jsonTest(void)
{
  int ret = E_SUC;
  uint8_t uuid[16] = "\x00\x0b\x57\x31\x55\x5c\x2d\x76\x65\x44\x73\x62\x61\x6c\x69\x53";
  jsonConfigAddNodeItem(uuid);
  /* fjsonConfigLoadTemplate(); */
  ret = jsonConfigOpen("/home/zhfu/work/projs/provisioner/config/conf_home.json",
                       "/home/zhfu/work/projs/provisioner/config/template.json",
                       0,
                       NULL);
  if (ret) {
    CS_ERR("ret %d\n", ret);
  }
  uint16_t refId = 0x0000, newId = 0x1100;
  ret = jsonConfigWrite(netkeyId_e, &refId, &newId);
  if (ret) {
    CS_ERR("ret %d\n", ret);
  }
}

static void storeFeature(int which,
                         char *val,
                         networkConfig_t *pconfig,
                         int nodeIndex,
                         json_object *obj)
{
  int valtmp = 0, featureEM = 0;
  char *p = (char *)&valtmp, *p1 = NULL;
  int len = sizeof(int);
  nodeConf_t *pnconfig = &pconfig->pNodes[nodeIndex];
  json_object *tmp = NULL;

  switch (which) {
    case nodeLPN_em:
      return;
    case nodeProxy_em:
      featureEM = proxy_em;
      break;
    case nodeFriend_em:
      featureEM = friend_em;
      break;
    case nodeSNB_em:
      featureEM = secure_netwokr_beacon_em;
      break;
    case nodeRelay_em:
      featureEM = relay_em;
      if (json_object_object_get_ex(obj, STR_ENABLE, &tmp)) {
        p1 = (char *)json_object_get_string(tmp);
        softASSERT(!str2charBuf(p1 + 2, 0, p, len));
        if (*p) {
          FEATURE_ENABLE(pnconfig->features, featureEM);
          if (json_object_object_get_ex(obj, STR_COUNT, &tmp)) {
            storeConfig(nodeRelayRetranCount_em, pconfig, nodeIndex, tmp, obj);
          }
          if (json_object_object_get_ex(obj, STR_INTERVAL, &tmp)) {
            storeConfig(nodeRelayRetranInterval_em, pconfig, nodeIndex, tmp, obj);
          }
        } else {
          FEATURE_DISABLE(pnconfig->features, featureEM);
        }
      }
      return;
    /* break; */
    case nodeTTL_em:
      featureEM = ttl_em;
      p = (char *)&pnconfig->ttl;
      len = sizeof(uint8_t);
      break;
    default:
      softASSERT(0);
      break;
  }

  softASSERT(!str2charBuf(val, 0, p, len));
  if (*p) {
    FEATURE_ENABLE(pnconfig->features, featureEM);
  } else {
    FEATURE_DISABLE(pnconfig->features, featureEM);
  }

  if (which == nodeRelay_em && valtmp) {
    if (json_object_object_get_ex(obj, STR_COUNT, &tmp)) {
      storeConfig(nodeRelayRetranCount_em, pconfig, nodeIndex, tmp, obj);
    }
    if (json_object_object_get_ex(obj, STR_INTERVAL, &tmp)) {
      storeConfig(nodeRelayRetranInterval_em, pconfig, nodeIndex, tmp, obj);
    }
  }
}

static void storeArray(int which,
                       nodeConf_t *pnconfig,
                       json_object *obj,
                       size_t unitSize)
{
  int num = 0, i = 0;
  uint8_t *pcnt = NULL;
  void **parray = NULL;
  json_object *iter = NULL;
  hardASSERT(json_type_array == json_object_get_type(obj));
  num = json_object_array_length(obj);

  switch (which) {
    case nodeBind_em:
      pcnt = &pnconfig->bindingCnt;
      parray = (void **)&pnconfig->bindings;
      break;
    case nodeSub_em:
      pcnt = &pnconfig->subCnt;
      parray = (void **)&pnconfig->subAddrs;
      break;
    default:
      hardASSERT(0);
      break;
  }

  *pcnt = num;
  if (*pcnt) {
    *parray = pmalloc(unitSize * (*pcnt));
    memset(*parray, 0, unitSize * (*pcnt));
  }

  for (i = 0; i < num; i++) {
    iter = json_object_array_get_idx(obj, i);
    softASSERT(!str2charBuf(json_object_get_string(iter) + 2,
                            0,
                            *parray + unitSize * i,
                            unitSize));
  }
}

static void storeConfig(int which,
                        void *ptarget,
                        int nodeIndex,
                        json_object *obj,
                        json_object *parent)
{
  networkConfig_t *pconfig = NULL;
  keys_t *keys = NULL;
  keyItem_t *key = NULL;
  char *val = NULL;
  void *in = NULL, *out = NULL;
  int len = 0, ret = 0;
  json_object *tmp = NULL;
  PARAM_CHECK();
  val = (char *)json_object_get_string(obj);
  hardASSERT(val);
  pconfig = (networkConfig_t *)ptarget;
  keys = (keys_t *)ptarget;
  if (nodeIndex == -1) {
    key = &keys->netkey;
  } else {
    key = &keys->appKeys[nodeIndex];
  }

  switch (which) {
    case nodeUUID_em:
      in = val;
      out = (char *)pconfig->pNodes[nodeIndex].uuid.data;
      len = 16;
      break;
    case nodeRefId_em:
      in = val + 2;
      out = (char *)pconfig->pNodes[nodeIndex].uuid.data;
      len = 16;
      break;
    case nodeFeatures_em:
#if 0
      if (json_object_object_get_ex(obj, STR_LPN, &tmp)) {
        storeConfig(nodeLPN_em, pconfig, nodeIndex, tmp, obj);
      }
#endif
      if (json_object_object_get_ex(obj, STR_PROXY, &tmp)) {
        storeConfig(nodeProxy_em, pconfig, nodeIndex, tmp, obj);
      }
      if (json_object_object_get_ex(obj, STR_FRIEND, &tmp)) {
        storeConfig(nodeFriend_em, pconfig, nodeIndex, tmp, obj);
      }
      if (json_object_object_get_ex(obj, STR_RELAY, &tmp)) {
        storeConfig(nodeRelay_em, pconfig, nodeIndex, tmp, obj);
      }
      return;

    case nodeLPN_em:
    case nodeProxy_em:
    case nodeFriend_em:
    case nodeRelay_em:
    case nodeTTL_em:
    case nodeSNB_em:
      storeFeature(which, val + 2, pconfig, nodeIndex, obj);
      return;

    case nodeNetRetran_em:
      FEATURE_ENABLE(pconfig->pNodes[nodeIndex].features, net_trans_em);
      if (json_object_object_get_ex(obj, STR_COUNT, &tmp)) {
        storeConfig(nodeNetRetranCount_em, pconfig, nodeIndex, tmp, obj);
      }
      if (json_object_object_get_ex(obj, STR_INTERVAL, &tmp)) {
        storeConfig(nodeNetRetranInterval_em, pconfig, nodeIndex, tmp, obj);
      }
      return;
    case nodePub_em:
      if (json_object_object_get_ex(obj, STR_ADDR, &tmp)) {
        storeConfig(nodePubAddr_em, pconfig, nodeIndex, tmp, obj);
      }
      if (json_object_object_get_ex(obj, STR_KEY, &tmp)) {
        storeConfig(nodePubKey_em, pconfig, nodeIndex, tmp, obj);
      }
      if (json_object_object_get_ex(obj, STR_PERIOD, &tmp)) {
        storeConfig(nodePubPeriod_em, pconfig, nodeIndex, tmp, obj);
        pconfig->pNodes[nodeIndex].pubPeriod =
          pub_period_sanity_check(pconfig->pNodes[nodeIndex].pubPeriod)
          ? pconfig->pNodes[nodeIndex].pubPeriod : 0;
      }
      return;
    case nodeNetRetranCount_em:
      in = val + 2;
      out = (char *)&pconfig->pNodes[nodeIndex].transCnt;
      len = sizeof(uint8_t);
      break;
    case nodeNetRetranInterval_em:
      in = val + 2;
      out = (char *)&pconfig->pNodes[nodeIndex].transIntv;
      len = sizeof(uint16_t);
      break;
    case nodeRelayRetranCount_em:
      in = val + 2;
      out = (char *)&pconfig->pNodes[nodeIndex].relayRetransCount;
      len = sizeof(uint8_t);
      break;
    case nodeRelayRetranInterval_em:
      in = val + 2;
      out = (char *)&pconfig->pNodes[nodeIndex].relayRetransInterval;
      len = sizeof(uint16_t);
      break;
    case nodePubAddr_em:
      in = val + 2;
      out = (char *)&pconfig->pNodes[nodeIndex].pubAddr;
      len = sizeof(uint16_t);
      break;
    case nodePubKey_em:
      in = val + 2;
      out = (char *)&pconfig->pNodes[nodeIndex].pubBind;
      len = sizeof(uint8_t);
      break;
    case nodePubPeriod_em:
      in = val + 2;
      out = (char *)&pconfig->pNodes[nodeIndex].pubPeriod;
      len = sizeof(uint32_t);
      break;
    case nodeBind_em:
    case nodeSub_em:
      storeArray(which,
                 &pconfig->pNodes[nodeIndex],
                 obj,
                 which == nodeBind_em ? sizeof(uint8_t) : sizeof(uint16_t));
      return;

    case nodeDone_em:
      if (clear) {
        json_object_object_add(parent, STR_DONE, json_object_new_string("0x00"));
        pconfig->pNodes[nodeIndex].done = 0;
        return;
      } else {
        in = val + 2;
        out = (char *)&pconfig->pNodes[nodeIndex].done;
        len = sizeof(uint8_t);
      }
      break;
    case nodeBL_em:
      if (clear) {
        json_object_object_add(parent, STR_BL, json_object_new_string("0x00"));
        pconfig->pNodes[nodeIndex].blacklist = 0;
        return;
      } else {
        in = val + 2;
        out = (char *)&pconfig->pNodes[nodeIndex].blacklist;
        len = sizeof(uint8_t);
      }
      break;
    case nodeErr_em:
      if (clear) {
        json_object_object_add(parent, STR_ERRBITS, json_object_new_string("0x00000000"));
        pconfig->pNodes[nodeIndex].errBits = 0;
        return;
      } else {
        in = val + 2;
        out = (char *)&pconfig->pNodes[nodeIndex].errBits;
        len = sizeof(uint32_t);
      }
      break;
    case nodeAddr_em:
      if (clear) {
        json_object_object_add(parent, STR_ADDR, json_object_new_string("0x0000"));
        pconfig->pNodes[nodeIndex].uniAddr = 0;
        return;
      } else {
        in = val + 2;
        out = (char *)&pconfig->pNodes[nodeIndex].uniAddr;
        len = sizeof(uint16_t);
      }
      break;

    case keyRefId_em:
      in = val + 2;
      out = (char *)&key->refId;
      len = sizeof(uint16_t);
      break;
    case keyId_em:
      if (clear) {
        json_object_object_add(parent, STR_ID, json_object_new_string("0x0000"));
        key->id = 0;
        return;
      } else {
        in = val + 2;
        out = (char *)&key->id;
        len = sizeof(uint16_t);
      }
      break;
    case keyValue_em:
      in = val + 2;
      out = (char *)key->value;
      len = 16;
      break;
    case keyDone_em:
      if (clear) {
        json_object_object_add(parent, STR_DONE, json_object_new_string("0x00"));
        key->done = 0;
        return;
      } else {
        in = val + 2;
        out = (char *)&key->done;
        len = sizeof(uint8_t);
      }
      break;
    case keyAppKey_em:
      fjsonConfigLoadAllAppKeys(keys, obj);
      return;

    default:
      CS_ERR("%d Missing Handling\n", which);
      hardASSERT(0);
      break;
  }

  if (E_SUC != (ret = str2charBuf(in, 0, out, len))) {
    CS_ERR("ret = %d, which = %d, val = %s\n", ret, which, val);
    softASSERT(0);
  }
}

static inline nodeConf_t *getTmplById(const networkConfig_t *pTmplConf, uint8_t id)
{
  for (int i = 0; i < pTmplConf->nodeCnt; i++) {
    if (id == pTmplConf->pNodes[i].uuid.data[0]) {
      return &pTmplConf->pNodes[i];
    }
  }
  return NULL;
}

static int loadNodeConfFromTemplate(nodeConf_t *pN, uint8_t id)
{
  uint8_t *pBindings = NULL;
  uint16_t *pSubAddrs = NULL;
  /* The calling function should ensure pN != NULL */
  nodeConf_t *pTN = getTmplById(pTmplConf, id);
  if (pTN == NULL) {
    return E_INVP;
  }

  memcpy(pN, pTN, sizeof(nodeConf_t));

  if (pTN->bindingCnt) {
    pBindings = (uint8_t *)malloc(sizeof(uint8_t) * pTN->bindingCnt);
    pN->bindings = pBindings;
    memcpy(pN->bindings, pTN->bindings, sizeof(uint8_t) * pTN->bindingCnt);
  }
  if (pTN->subCnt) {
    pSubAddrs = (uint16_t *)malloc(sizeof(uint16_t) * pTN->subCnt);
    pN->subAddrs = pSubAddrs;
    memcpy(pN->subAddrs, pTN->subAddrs, sizeof(sizeof(uint16_t) * pTN->subCnt));
  }

  return E_SUC;
}

static void fjsonConfigLoadOneNode(networkConfig_t *pconfig,
                                   int nodeIndex,
                                   json_object *iter)
{
  int j = 0, ret = 0;
  uint8_t tmplId = NODE_INVALID_FLAG;
  char *val = NULL;
  json_object *tmp = NULL;

  memset(&pconfig->pNodes[nodeIndex].features, 0, sizeof(uint32_t));
  if (json_object_object_get_ex(iter, STR_TMPL, &tmp)) {
    val = (char *)json_object_get_string(tmp);
    if (E_SUC != (ret = str2charBuf(val + 2, 0, (char *)&tmplId, sizeof(uint8_t)))) {
      CS_ERR("ret = %d, val = %s\n", ret, val);
      softASSERT(0);
    }
    if (tmplId == NODE_INVALID_FLAG) {
      pconfig->nodeCnt--;
      return;
    }
    if (E_SUC
        != (ret = loadNodeConfFromTemplate(&pconfig->pNodes[nodeIndex],
                                           tmplId))) {
      CS_ERR("Load Node From Template Error: %d - %s\n",
             ret,
             ERR_NAME[ret]);
      softASSERT(0);
    }
  }

  for (j = 0; j < NODE_KEYS_NUM(); j++) {
    if (json_object_object_get_ex(iter, NODE_Keys[j].strKey, &tmp)) {
      storeConfig(NODE_Keys[j].configIndex, pconfig, nodeIndex, tmp, iter);
    }
  }
}

#define SET_FREE_FLAG(free, i)  BIT_SET((free)[(i) / 8], (i) % 8);
#define CLEAR_FREE_FLAG(free, i) BIT_CLEAR((free)[(i) / 8], (i) % 8);

static int get_lowest_free(const uint32_t free[8])
{
  int i = 0, ret = -1;
  for (i = 0; i < 8; i++) {
    ret = getLeftFirstOne(free[i]);
    if (ret == -1) {
      continue;
    } else {
      ret += i * 32;
      break;
    }
  }

  return ret;
}

static void compact_nodes(networkConfig_t *pconfig,
                          size_t node_amount)
{
  uint32_t free[8];
  int i = 0, packed = 1, tmp = 0;

  memset(free, 0, sizeof(uint32_t) * 8);

  for (i = 0; i < node_amount; i++) {
    if (array_all_0(pconfig->pNodes[i].uuid.data, 16)) {
      SET_FREE_FLAG(free, i);
      packed = 0;
    } else if (packed == 0) {
      tmp = get_lowest_free(free);
      memcpy(&pconfig->pNodes[tmp], &pconfig->pNodes[i], sizeof(nodeConf_t));
      CLEAR_FREE_FLAG(free, tmp);
      memset(&pconfig->pNodes[i], 0, sizeof(nodeConf_t));
      SET_FREE_FLAG(free, i);
    }
  }
}

static void fjsonConfigLoadAllNodes(networkConfig_t *pconfig,
                                    json_object *obj)
{
  int i = 0, len = 0;
  json_object *iter = NULL;
  hardASSERT(json_type_array == json_object_get_type(obj));
  hardASSERT(pconfig);

  len = json_object_array_length(obj);
  ALLOCATE_NODES(pconfig, len);

  for (i = 0; i < len; i++) {
    iter = json_object_array_get_idx(obj, i);
#if (JSON_ECHO_DBG == 1)
    CS_LOG("%d --- %s\n", i, json_object_to_json_string(iter));
#endif
    fjsonConfigLoadOneNode(pconfig, i, iter);
  }
}

static void fjsonConfigLoadOneKey(keys_t *keys,
                                  int offset,
                                  json_object *iter)
{
  int j = 0;
  json_object *tmp = NULL;
  for (j = 0; j < KEY_KEYS_NUM(); j++) {
    if (json_object_object_get_ex(iter, KEY_keys[j].strKey, &tmp)) {
      storeConfig(KEY_keys[j].configIndex, keys, offset, tmp, iter);
    }
  }
}

static void fjsonConfigLoadAllAppKeys(keys_t *keys,
                                      json_object *obj)
{
  int i = 0, len = 0;
  json_object *iter = NULL;
  hardASSERT(json_type_array == json_object_get_type(obj));
  hardASSERT(keys);

  len = json_object_array_length(obj);
  ALLOCATE_KEYS(keys, len);
  for (i = 0; i < len; i++) {
    iter = json_object_array_get_idx(obj, i);
#if (JSON_ECHO_DBG == 1)
    CS_LOG("%d --- %s\n", i, json_object_to_json_string(iter));
#endif
    fjsonConfigLoadOneKey(keys, i, iter);
  }
}

static void fjsonConfigLoadAllKeys(networkConfig_t *pconfig,
                                   json_object *obj)
{
  int i = 0, len = 0;
  json_object *iter = NULL;
  hardASSERT(json_type_array == json_object_get_type(obj));
  hardASSERT(pconfig);

  len = json_object_array_length(obj);

  for (i = 0; i < len; i++) {
    if (NULL != (iter = json_object_array_get_idx(obj, i))) {
#if (JSON_ECHO_DBG == 1)
      CS_LOG("%d --- %s\n", i, json_object_to_json_string(iter));
#endif
      fjsonConfigLoadOneKey(&pconfig->keys, -1, iter);
    }
  }
}

static errcode_t fjsonConfigLoadTemplate(void)
{
  json_object *obj = NULL;

  hardASSERT(template);
  JSON_FREE(templatePtr);

  if ((templatePtr = json_object_from_file(template)) == NULL) {
    CS_ERR("Open %s failed.\n", template);
    return E_IO;
  }

  obj = (json_object *)json_object_get_object(templatePtr)->head->v;

  FREE_AND_NEW_CONF(pTmplConf);

  fjsonConfigLoadAllNodes(pTmplConf, obj);

  /* TODO - if need to free anything? */
  return E_SUC;
}
