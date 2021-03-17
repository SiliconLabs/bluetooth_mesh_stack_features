/*************************************************************************
    > File Name: generic_main.h
    > Author: Kevin
    > Created Time: 2019-04-22
    >Description:
 ************************************************************************/

#ifndef GENERIC_MAIN_H
#define GENERIC_MAIN_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "adding_devices.h"
#include "blacklisting_devices.h"
#include "network_manage.h"

typedef enum {
  not_sync_up_em = -1,
  syncup_done_em = 0,
  initialized_em,
  configured_idle_em,
  adding_devices_em,
  configuring_devices_em,
  removing_devices_em,
  blacklisting_devices_em
} provState_t;

typedef enum {
  not_start_em,
  starting_em,
  started_em,
  stopping_em
}progressStatus_t;

typedef struct runningStatus{
  int isScanning;
  int freeMode;
  volatile progressStatus_t inProgress;
  volatile int actions;
  volatile int forceReloadActions;
  int actionPos;
}runningStatus_t;

typedef struct provisioner{
  int state; /* provState_t */
  tbaStatus_t tbaStatus;
  tbblStatus_t tbblStatus;
  /* tbrStatus_t tbrStatus; */
  /* tbcStatus_t tbcStatus; */
  uint16_t address;
  uint32_t ivi;
  uint8_t subnetCnt;
  /* uint16_t networkIds[MAX_SUBNETS]; */
  uint8_t connHandle;
  configTimeouts_t configTimeouts;
  nettxConfig_t nettxConfig;
  runningStatus_t runningStatus;
}provisioner_t;

provisioner_t *getProvisioner(void);
int getFreeModeStatus(void);
int getProvState(void);
int getScanningStatus(void);
int getProgressStatus(void);
int getActionPos(void);
int getLoadedActions(void);
tbaStatus_t *gettbaStatus(void);
tbblStatus_t *gettbblStatus(void);
void printActionSeq(void);

void setFreeMode(int enable);
void wrapActions(void);

void invalidProvisionerState(void);

void *pGenericMain(void *pIn);
void forceGenericReloadActions(void);
void startOrStopProgress(int start);
void preForceReSyncUpHostAndTarget(void);
int genericInit(void);
#ifdef __cplusplus
}
#endif
#endif //GENERIC_MAIN_H
