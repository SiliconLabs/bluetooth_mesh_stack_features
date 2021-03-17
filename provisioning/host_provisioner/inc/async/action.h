/*************************************************************************
    > File Name: /home/zhfu/work/projs/provisioner/inc/async/action_loader.h
    > Author: Kevin
    > Created Time: 2019-04-23
    >Description:
 ************************************************************************/

#ifndef ACTION_LOADER_H
#define ACTION_LOADER_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "utils.h"
#include "device_database.h"

#define MAX_ADD_LIST_SIZE                       DEFAULT_MAX_NODES
#define MAX_RECONFIG_LIST_SIZE                  DEFAULT_MAX_NODES
#define MAX_REMOVE_LIST_SIZE                    DEFAULT_MAX_NODES
#define MAX_BLACKLIST_LIST_SIZE                 DEFAULT_MAX_NODES

typedef enum {
  actionNone,
  actionTBA,
  actionTBR,
  actionTBBL,
  actionTBC,
  actionMAX
} nodeAction_t;

#define ACTION_BIT_OFFSET(x) (1 << (x))
#define SET_ACTION_BIT(x, action) BIT_SET((x), (action))
#define CLEAR_ACTION_BIT(x, action) BIT_CLEAR((x), (action))
#define IS_ACTION_BIT_SET(x, action) IS_BIT_SET((x), (action))

typedef struct actionCache{
  int changed;

  int nodeAction;
  int prioAction;

  int addPollDone;
  int recPollDone;
  int removePollDone;
  int blacklistPollDone;

  int addCnt;
  int addList[MAX_ADD_LIST_SIZE];

  int blacklistCnt;
  int blacklistList[MAX_BLACKLIST_LIST_SIZE];

  int removeCnt;
  int removeList[MAX_REMOVE_LIST_SIZE];

  int reConfigCnt;
  int reConfigList[MAX_RECONFIG_LIST_SIZE];
}actionCache_t;

typedef int (*funcRemoveDeviceFromDDB)(int t, int idxInPconfig);
typedef int (*funcIsDeviceInDDB)(int idxInPconfig);

void actionInit(funcRemoveDeviceFromDDB fn1,
                funcIsDeviceInDDB fn2);

static inline void clearAddCache(actionCache_t *cache)
{
  cache->addCnt = 0;
  memset(cache->addList, 0, sizeof(cache->addList));
}

static inline void clearRemoveCache(actionCache_t *cache)
{
  cache->removeCnt = 0;
  memset(cache->removeList, 0, sizeof(cache->removeList));
}

static inline void clearBlackListCache(actionCache_t *cache)
{
  cache->blacklistCnt = 0;
  memset(cache->blacklistList, 0, sizeof(cache->blacklistList));
}

static inline void clearRecCache(actionCache_t *cache)
{
  cache->reConfigCnt = 0;
  memset(cache->reConfigList, 0, sizeof(cache->reConfigList));
}

void clearList(int which);
void forceReloadAction(void);
int getActions(int forceReloadAll);
int getAction(int which, int forceReload);
void onDeviceDone(nodeAction_t which, int nodeId);
int getNodeByOffset(nodeAction_t which, int offset);
int getCnt(int which);
#ifdef __cplusplus
}
#endif
#endif //ACTION_LOADER_H
