/*************************************************************************
    > File Name: conf_generic_parser.h
    > Author: Kevin
    > Created Time: 2019-03-27
    >Description:
 ************************************************************************/

#ifndef CONF_GENERIC_PARSER_H
#define CONF_GENERIC_PARSER_H
#include <stdbool.h>

#include "err_code.h"
#include "bg_stack/bg_types.h"

#include "config.h"

#define NODE_INVALID_FLAG                 0xff

/* Strings used in the xml config files */
#define STR_NETWORK                       "Network"

#define STR_SUBNETS                       "Subnets"
#define STR_NETKEY                        "NetKey"
#define STR_APPKEY                        "AppKey"
#define STR_KEY                           "Key"

#define STR_NODES                         "Nodes"
#define STR_TEMPLATES                     "Templates"
#define STR_DONE                          "Done"
#define STR_NODE                          "Node"
#define STR_DUMMY_ID                      "RefId"
#define STR_ID                            "Id"
#define STR_VALUE                         "Value"
#define STR_ADDR                          "Addr"
#define STR_ERRBITS                       "Err"
#define STR_UUID                          "UUID"
#define STR_ID                            "Id"
#define STR_TTL                           "TTL"
#define STR_FEATURES                      "Features"
#define STR_BL                            "Blacklist"
#define STR_TMPL                          "Tmpl"
#define STR_BIND                          "Bind"
#define STR_PUB_BIND                      "Pub_bind"
#define STR_SUB                           "Sub"
#define STR_TRANS_CNT                     "Trans"
#define STR_TRANS_INTERVAL                "Tran_interval"
#define STR_PERIOD                        "Period"

#define STR_SECURE_NETWORK_BEACON "SNB"
#define STR_LPN "LPN"
#define STR_PROXY "Proxy"
#define STR_FRIEND "Friend"
#define STR_RELAY "Relay"
#define STR_COUNT "Count"
#define STR_INTERVAL "Interval"
#define STR_NET_RETRAN "Net_Retransmit"
#define STR_ENABLE "Enable"
#define STR_PUB                           "Pub"

#define DEFAULT_MAX_FILE_PATH_LENGTH      64

#define DEFAULT_NETKEY                  "03030303030303030303030303030303"
#define DEFAULT_APPKEY                  "03030303030303030303030303030303"

#define ENCODE_FMT                      "UTF-8"

enum {
  relay_em,
  proxy_em,
  friend_em,
  lpn_em, /* Informative, not affect anything */
  /* Below is extension which is not in the SPEC */
  ttl_em,
  net_trans_em,
  secure_netwokr_beacon_em,
  feature_max = 16
};

#define FBIT(x)  (1 << (x))
#define ALL_FEATURE_BITS  (FBIT(relay_em) | FBIT(proxy_em) | FBIT(friend_em))

#define FEATURE_ENABLE(featureBits, feature) \
  BIT_SET((featureBits), (feature));         \
  BIT_SET((featureBits), (feature) + 16)

#define FEATURE_DISABLE(featureBits, feature) \
  BIT_CLEAR((featureBits), (feature));        \
  BIT_SET((featureBits), (feature) + 16)

#define FEATURE_KEEP(featureBits, feature) \
  BIT_CLEAR((featureBits), (feature) + 16)

enum {
  netKeyE,
  appKeyE,
  NodeE
};

typedef enum {
  typeXML_e,
  typeJSON_e
} fileType_t;

typedef struct keyItem{
  uint16_t refId;
  uint16_t id;
  uint8_t value[16];
  uint8_t done;
}keyItem_t;

typedef struct keys{
  keyItem_t netkey;
  int appKeyCnt;
  int activeAppKeyCnt;
  keyItem_t *appKeys;
}keys_t;

typedef struct nodeConf{
  uuid_128 uuid;
  uint8_t ttl;
  char feature[4];
  uint16_t uniAddr;
  uint8_t blacklist;
  uint8_t done;
  uint8_t bindingCnt;
  uint8_t *bindings;
  uint16_t pubAddr;
  uint8_t pubBind;
  uint32_t pubPeriod;
  uint8_t subCnt;
  uint32_t errBits;
  uint16_t *subAddrs;
  uint8_t transCnt;
  uint16_t transIntv;

  /* New added */
  uint32_t features;
  uint8_t  relayRetransCount;
  uint16_t  relayRetransInterval;
}nodeConf_t;

typedef struct Config{
  keys_t keys;
  uint8_t nodeCnt;
  nodeConf_t *pNodes;
}networkConfig_t;

#define FL_CREATE                       (1UL << 0)
#define FL_TRUNC                        (1UL << 1)
#define FL_CLEAR                        (1UL << 2)

typedef enum {
  wholeFile,
  subnetCnt,
  appkeyCnt,
  nodeCnt,
  iterateUUID
}readType_t;

typedef enum {
  addItem_e,
  /* Key fields */
  netkeyDone_e,
  netkeyId_e,
  netkeyValue_e,
  appkeyDone_e,
  appkeyId_e,
  /* Node fields */
  errBits_e,
  unicastAddr_e,
  blacklist_e,
  done_e,
}writeType_t;

typedef errcode_t (*pfncInit)(void *(*pmalloc)(size_t),
                              void (*pfree)(void *),
                              const void *data);

typedef void (*pfncDeInit)(void);

typedef errcode_t (*pfncOpen)(const char nodeConFilePath[],
                              const char templateFilePath[],
                              unsigned int flags,
                              void **data);

typedef errcode_t (*pfncRead)(readType_t rdt,
                              const void *key,
                              void *data);

typedef errcode_t (*pfncWrite)(writeType_t wrt,
                               const void *key,
                               const void *data);

typedef errcode_t (*pfncClose)(void);

typedef errcode_t (*pfncFlush)(void);

typedef errcode_t (*pfncFreeConfig)(void **);

errcode_t parserInit(fileType_t t);

errcode_t confClearControlFields(void);

errcode_t setNetkeyIdToFile(uint16_t refId, uint16_t newId);

errcode_t setNetkeyDoneToFile(uint16_t id, uint8_t value);

errcode_t setNetkeyValueToFile(uint8_t refId, const uint8_t *newKey);

errcode_t setAppkeyIdToFile(uint16_t refId, uint16_t newId);

errcode_t setAppkeyDoneToFile(uint16_t id, uint8_t value);

errcode_t setNodeErrBitsToFile(uuid_128 uuid, uint32_t err);

errcode_t setNodeUniAddrToFile(uuid_128 uuid, uint16_t addr);

errcode_t setNodeBlacklistToFile(uuid_128 uuid, uint8_t value);

errcode_t setNodeDoneToFile(uuid_128 uuid, uint8_t value);

errcode_t freeConfig(void);

errcode_t reloadNetworkConfig(networkConfig_t **pCfg);
errcode_t loadNetworkConfig(networkConfig_t **pCfg,
                            const char *nodeConfigFilePath,
                            const char *templateFilePath,
                            bool createIfNotExist,
                            bool truncate);

bool nodeIsRecorded(const uint8_t *uuid);
errcode_t addOneNodeToConfigFile(const uint8_t *uuid);
errcode_t getNetworkConfig(void **configOut);

int pub_period_sanity_check(uint32_t period);
#endif
