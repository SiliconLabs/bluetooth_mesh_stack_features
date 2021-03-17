/*************************************************************************
    > File Name: blacklisting_devices.h
    > Author: Kevin
    > Created Time: 2019-04-25
    >Description:
 ************************************************************************/

#ifndef BLACKLISTING_DEVICES_H
#define BLACKLISTING_DEVICES_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>

typedef enum {
  tbbl_idle_em,
  tbbl_busy_em,
  tbbl_done_em
} tbblState_t;

typedef struct _tbblCache{
  uint16_t err;
  uint8_t retried;
  int retry;
  uint16_t offset;
  uint32_t phase1Devs[8];
  uint32_t phase2Devs[8];
  uint32_t phase0Devs[8];
}tbblCache_t;

typedef struct tbblStatus {
  int state;
  tbblCache_t cache;
  int isWaiting;
} tbblStatus_t;

int blacklistingDevicesInit(void);
int blacklistingDevicesMainLoop(void *p);

int blacklist_devices_event_handler(struct gecko_cmd_packet *e);
#ifdef __cplusplus
}
#endif
#endif //BLACKLISTING_DEVICES_H
