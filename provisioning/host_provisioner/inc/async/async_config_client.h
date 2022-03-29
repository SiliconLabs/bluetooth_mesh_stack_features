/*************************************************************************
    > File Name: async_config_client.h
    > Author: Kevin
    > Created Time: 2019-04-17
    >Description:
 ************************************************************************/

#ifndef ASYNC_CONFIG_CLIENT_H
#define ASYNC_CONFIG_CLIENT_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "config.h"
#include "utils.h"
#include "prov_assert.h"
#include "conf_generic_parser.h"

#if (ACC_DEBUG == 1)
#define accDEBUG(__fmt__, ...)  dbgPrint(__fmt__, ##__VA_ARGS__)
#define __ERR_P(err, cache, state)                       \
  accDEBUG(" *** Error<0x%04x> Happened in %s State.\n", \
           err, state)
#else
#define accDEBUG(__fmt__, ...)
#define __ERR_P(err, cache, state)
#endif

#define RETRY_MSG \
  "Node[%d]: --- Retry %s on TIMEOUT, remaining %d times before failure\n"
#define RETRY_OUT_MSG \
  "Node[%d]: All Retry Done BUT Still FAILED\n"
#define RETRY_TO_END_MSG \
  "Got continueous failure, device present?\n"
#define OOM_MSG \
  "Node[%d]: Resend Last %s Command Due to Out Of Memory\n"
#define OOM_SET_MSG \
  "Node[%d]: Out Of Memory Set for Last %s\n"
#define EXPIRED_MSG \
  "Node[%d]: Resend Last %s Command Due to Command Expired\n"

#define RETRY_OUT_PRINT(x) CS_ERR(RETRY_OUT_MSG, (x)->nodeIndex);
#define RETRY_ONCE_PRINT(x) \
  CS_LOG(RETRY_MSG, (x)->nodeIndex, stateNames[(x)->state], (x)->retry);
#define OOM_SET_PRINT(x) \
  CS_LOG(OOM_SET_MSG, (x)->nodeIndex, stateNames[(x)->state]);
#define OOM_ONCE_PRINT(x) \
  CS_LOG(OOM_MSG, (x)->nodeIndex, stateNames[(x)->state]);
#define EXPIRED_ONCE_PRINT(x) \
  CS_LOG(EXPIRED_MSG, (x)->nodeIndex, stateNames[(x)->state]);

#define TBC_ERR_BIT_OFFSET  1

#define NEW_STATE_INS(name)                               \
  stateInstance_t * pnew##name                            \
    = (stateInstance_t *)malloc(sizeof(stateInstance_t)); \
  do {                                                    \
    memset(pnew##name, 0, sizeof(stateInstance_t));       \
  } while (0)

#define UNASSIGNED_ADDRESS  0

#define ERROR_BIT(x)        (1UL << ((x)))
#define SET_ERROR_BIT(x, b) ((x) |= ERROR_BIT((b)))
#define SET_USER_ERR_BIT(x) ((x) |= 0x80000000)

typedef enum {
  on_timeout_em,
  on_oom_em,
  on_guard_timer_expired_em,
  retry_on_max_em
} retry_t;

typedef enum {
  bgapi_em,
  bgevent_em,
  bg_err_invalid_em
} bgErrType_t;

typedef enum {
  to_next_state_em = -1,
  /* Adding devices state(s) */
  provisioning_em = 0,
  provisioned_em,
  /* Configuring devices state(s) */
  getDcd_em,
  addAppKey_em,
  bindAppKey_em,
  setPub_em,
  addSub_em,
  setConfig_em,
  end_em,
  /* Removing devices state(s) */
  resetNode_em,
  resetNodeEnd_em
} tbcState_t;

typedef enum {
  keep_em,
  disable_em,
  enable_em
} featureState_t;

enum {
  once_em,
  success_em,
  failed_em
};

#define CONFIGURATION_STATE_MIN getDcd_em
#define CONFIGURATION_STATE_MAX resetNodeEnd_em

typedef struct errCache{
  uint32_t callErrBits; /* Bit mask for error happened in BGAPI calls in specific state */
  uint32_t evtErrBits; /* Bit mask for error happened in BGAPI received event in specific state */
  uint16_t bgErr; /* Cache of errer received from BGAPI calls or events */
}errCache_t;

typedef struct accInitConfig{
  int useDefaultStates;
  int enableStateMachine;
}accInitConfig_t;

#define EVER_RETRIED_BIT_OFFSET 7
#define WAITING_RESPONSE_BIT_OFFSET 6
#define OOM_BIT_OFFSET  5

#define WAITING_RESPONSE_BIT_MASK  (1 << WAITING_RESPONSE_BIT_OFFSET)
#define EVER_RETRIED_BIT_MASK  (1 << EVER_RETRIED_BIT_OFFSET)
#define OOM_BIT_MASK  (1 << OOM_BIT_OFFSET)
#define GUARD_TIMER_EXPIRED_BIT_MASK  (1 << GUARD_TIMER_EXPIRED_OFFSET)

#define WAIT_RESPONSE(x)  IS_BIT_SET((x)->flags, WAITING_RESPONSE_BIT_OFFSET)
#define WAIT_RESPONSE_CLEAR(x)  BIT_CLEAR((x)->flags, WAITING_RESPONSE_BIT_OFFSET)
#define WAIT_RESPONSE_SET(x)  BIT_SET((x)->flags, WAITING_RESPONSE_BIT_OFFSET)

#define EVER_RETRIED(x) IS_BIT_SET((x)->flags, EVER_RETRIED_BIT_OFFSET)
#define EVER_RETRIED_SET(x) BIT_SET((x)->flags, EVER_RETRIED_BIT_OFFSET)
#define EVER_RETRIED_CLEAR(x) BIT_CLEAR((x)->flags, EVER_RETRIED_BIT_OFFSET)

#define OOM(x) IS_BIT_SET((x)->flags, OOM_BIT_OFFSET)
#define OOM_SET(x)                       \
  do {                                   \
    OOM_SET_PRINT(x);                    \
    BIT_SET((x)->flags, OOM_BIT_OFFSET); \
  } while (0)
#define OOM_CLEAR(x) BIT_CLEAR((x)->flags, OOM_BIT_OFFSET)

#define RETRY_CLEAR(x)                              \
  do {                                              \
    BIT_CLEAR((x)->flags, EVER_RETRIED_BIT_OFFSET); \
    (x)->retry = 0;                                 \
  } while (0)

typedef struct _element{
  uint8_t numS;
  uint8_t numV;
  uint16_t *pSm;
  uint16_t *pVd;
  uint16_t *pVm;
}element_t;

typedef struct _dcd{
  uint16_t featureBits;
  uint8_t eleCnt;
  element_t *pElm;
}dcd_t;

#define DEFAULT_GUARD_SEC  15
#define ITERATOR_NUM  3

typedef struct __tbcCache{
  tbcState_t state; /* current state */
  int nextState; /* -1 to the next state in the state list, others to specific state. */
  uint8_t flags;
  int retry; /* Remaining retry times */
  errCache_t errCache; /* Error cache, check it when error */
  dcd_t dcdP0; /* DCD page 0 */
  void *pconfig; /* Config File pointer */
  int nodeIndex; /* Node Index in the Config File */
  uint32_t handle; /* Handle for Async handling */

  int selfId;
  int expired;

  /* Below variables need to be specified by other layers */
  uint32_t features; /* Feature bits of the devices */
  uint16_t featuresValidBits;
  int iterators[ITERATOR_NUM];

  uint16_t vendorId;
  uint16_t modelId;
}tbcCache_t;

/* typedef struct tbcStatus { */
/* int loadedOffset; */
/* } tbcStatus_t; */

typedef int (*funcGuard)(const void *);
typedef int (*funcEntry)(void *in, funcGuard guard);
typedef int (*funcInprogress)(void *in, void *cache);
typedef int (*funcRetry)(void *p, int reason);
typedef int (*funcExit)(void *p);
typedef int (*funcDelegate)(uint32_t evtId);

typedef struct __stateInstance{
  int             state;
  funcGuard       entryGuard;
  funcEntry       onStateEntry;
  funcInprogress  onStateInProgress;
  funcRetry       onStateRetry;
  funcExit        onStateExit;
  funcDelegate    isRelatedPacket;
  struct __stateInstance *next;
}stateInstance_t;

int getDcdEntryGuard(const void *in);
int getDcdStateEntry(void *in, funcGuard guard);
int getDcdStateInprogress(void *in, void *cache);
int getDcdStateRetry(void *p, int reason);
int getDcdStateExit(void *p);
int isGetDcdRelatedPacket(uint32_t evtId);

int addAppKeyEntryGuard(const void *in);
int addAppKeyStateEntry(void *in, funcGuard guard);
int addAppKeyStateInprogress(void *in, void *cache);
int addAppKeyStateRetry(void *p, int reason);
int addAppKeyStateExit(void *p);
int isAddAppKeyRelatedPacket(uint32_t evtId);

int bindAppKeyEntryGuard(const void *in);
int bindAppKeyStateEntry(void *in, funcGuard guard);
int bindAppKeyStateInprogress(void *in, void *cache);
int bindAppKeyStateRetry(void *p, int reason);
int bindAppKeyStateExit(void *p);
int isBindAppKeyRelatedPacket(uint32_t evtId);

int setPubEntryGuard(const void *in);
int setPubStateEntry(void *in, funcGuard guard);
int setPubStateInprogress(void *in, void *cache);
int setPubStateRetry(void *p, int reason);
int setPubStateExit(void *p);
int isSetPubRelatedPacket(uint32_t evtId);

int addSubEntryGuard(const void *in);
int addSubStateEntry(void *in, funcGuard guard);
int addSubStateInprogress(void *in, void *cache);
int addSubStateRetry(void *p, int reason);
int addSubStateExit(void *p);
int isAddSubRelatedPacket(uint32_t evtId);

int setConfigsEntryGuard(const void *in);
int setConfigsStateEntry(void *in, funcGuard guard);
int setConfigsStateInprogress(void *in, void *cache);
int setConfigsStateRetry(void *p, int reason);
int setConfigsStateExit(void *p);
int isSetConfigsRelatedPacket(uint32_t evtId);

int removeNodeEntryGuard(const void *in);
int removeNodeStateEntry(void *in, funcGuard guard);
int removeNodeStateInprogress(void *in, void *cache);
int removeNodeStateRetry(void *p, int reason);
int removeNodeStateExit(void *p);
int isRemoveNodeRelatedPacket(uint32_t evtId);

int endStateEntry(void *in, funcGuard guard);
int removeEndStateEntry(void *in, funcGuard guard);

static inline void err_set(tbcCache_t *tbc, uint16_t errcode, int which)
{
  if (which == bgapi_em) {
    BIT_SET(tbc->errCache.callErrBits, tbc->state);
  } else if (which == bgevent_em) {
    BIT_SET(tbc->errCache.evtErrBits, tbc->state);
  } else {
    return;
  }
  tbc->errCache.bgErr = errcode;
}

static inline void err_set_to_end(tbcCache_t *tbc, uint16_t errcode, int which)
{
  err_set(tbc, errcode, which);
  tbc->nextState = end_em;
}

static inline void err_set_to_rm_end(tbcCache_t *tbc, uint16_t errcode, int which)
{
  err_set(tbc, errcode, which);
  tbc->nextState = resetNodeEnd_em;
}

errcode_t addStateAfter(const stateInstance_t *ps, int state);
size_t getConfigStateNum(void);
void enOrDisConfigStateMachine(bool enable);

void clearFailList(void);
void accInit(void *p);
void asyncConfigClientReset(void);
void *accMainProcess(void *p);
int accMainLoop(void *p);
void setNewDeviceAddedFlag(void);
int getAppKeyIdByDummyId(networkConfig_t *pconfig,
                         uint16_t refId,
                         uint16_t *id);
void startTimer(tbcCache_t *tbc, int start);

int config_devices_event_handler(struct gecko_cmd_packet *e);

#ifdef __cplusplus
}
#endif
#endif //ASYNC_CONFIG_CLIENT_H
