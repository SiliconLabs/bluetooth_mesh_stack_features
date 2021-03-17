/*************************************************************************
    > File Name: adding_devices.h
    > Author: Kevin
    > Created Time: 2019-04-25
    >Description:
 ************************************************************************/

#ifndef ADDING_DEVICES_H
#define ADDING_DEVICES_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "config.h"
#include "gecko_bglib.h"

typedef enum {
  tba_idle_em,
  tba_busy_em
} tbaState_t;

typedef int (*funcUnprovBeaconFoundCB)(void *);
typedef int (*funcProvDoneCB)(void *);

typedef struct tbaCache {
  tbaState_t state;
  int nodeId;
  int isWaiting;
} tbaCache_t;

typedef struct tbaStatus {
  int configClientState;
  tbaCache_t tba[MAX_PROV_SESSIONS];
  funcUnprovBeaconFoundCB fnUnprovBeaconFoundCB;
  funcProvDoneCB fnProvDoneCB;
} tbaStatus_t;

void addingDevicesResetAll(void);
int addingDevicesInit(funcUnprovBeaconFoundCB fn,
                      funcProvDoneCB fn1);
int addingDevicesMainLoop(void *p);

int adding_devices_event_handler(struct gecko_cmd_packet *e);
#ifdef __cplusplus
}
#endif
#endif //ADDING_DEVICES_H
