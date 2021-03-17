/*************************************************************************
    > File Name: json_parser.h
    > Author: Kevin
    > Created Time: 2019-05-11
    >Description:
 ************************************************************************/

#ifndef JSON_PARSER_H
#define JSON_PARSER_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "config.h"
#include "conf_generic_parser.h"

#define JSON_FREE(x)        \
  do {                      \
    if ((x)) {              \
      json_object_put((x)); \
      (x) = NULL;           \
    }                       \
  } while (0)

#define ALLOCATE_NODES(x, cnt)                                                \
  do {                                                                        \
    (x)->nodeCnt = cnt;                                                       \
    if ((x)->pNodes) {                                                        \
      pfree((x)->pNodes);                                                     \
    }                                                                         \
    if ((x)->nodeCnt) {                                                       \
      (x)->pNodes = (nodeConf_t *)pmalloc(sizeof(nodeConf_t) * (x)->nodeCnt); \
      memset((x)->pNodes, 0, sizeof(nodeConf_t) * (x)->nodeCnt);              \
    } else {                                                                  \
      (x)->pNodes = NULL;                                                     \
    }                                                                         \
  } while (0)

#define ALLOCATE_KEYS(x, cnt)                                                  \
  do {                                                                         \
    (x)->appKeyCnt = cnt;                                                      \
    if ((x)->appKeyCnt) {                                                      \
      pfree((x)->appKeys);                                                     \
    }                                                                          \
    if ((x)->appKeyCnt) {                                                      \
      (x)->appKeys = (keyItem_t *)pmalloc(sizeof(keyItem_t) * (x)->appKeyCnt); \
      memset((x)->appKeys, 0, sizeof(keyItem_t) * (x)->appKeyCnt);             \
    } else {                                                                   \
      (x)->appKeys = NULL;                                                     \
    }                                                                          \
  } while (0)

#define FREE_CONF(x)                       \
  do {                                     \
    if ((x)) {                             \
      jsonConfigFreeConfig((void **)&(x)); \
      (x) = NULL;                          \
    }                                      \
  } while (0)

#define FREE_AND_NEW_CONF(x)                                   \
  do {                                                         \
    FREE_CONF((x));                                            \
    (x) = (networkConfig_t *)pmalloc(sizeof(networkConfig_t)); \
    memset((x), 0, sizeof(networkConfig_t));                   \
  } while (0)

errcode_t jsonConfigInit(void *(*pm)(size_t),
                         void (*pfr)(void *),
                         const void *data);
errcode_t jsonConfigOpen(const char nodeConFilePath[],
                         const char templateFilePath[],
                         unsigned int flags,
                         void **data);
errcode_t jsonConfigRead(readType_t rdt,
                         const void *key,
                         void *data);
errcode_t jsonConfigWrite(writeType_t wrt,
                          const void *key,
                          const void *data);
errcode_t jsonConfigClose(void);
void jsonConfigDeinit(void);

errcode_t jsonConfigFreeConfig(void **ppcfg);
errcode_t jsonConfigFlush(void);
void jsonTest(void);
#ifdef __cplusplus
}
#endif
#endif //JSON_PARSER_H
