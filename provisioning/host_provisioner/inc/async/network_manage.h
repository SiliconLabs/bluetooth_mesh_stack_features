/*************************************************************************
    > File Name: network_manage.h
    > Author: Kevin
    > Created Time: 2019-04-22
    >Description:
 ************************************************************************/

#ifndef NETWORK_MANAGE_H
#define NETWORK_MANAGE_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "gecko_bglib.h"

#define MAX_SUBNETS 7
#define GENERIC_CMD_PRIORITY  1
#define TBA_CMD_PRIORITY  1
#define TBBL_CMD_PRIORITY  1
#define TBR_CMD_PRIORITY  1
#define TBC_CMD_PRIORITY  2
#define TBC_RETRY_CMD_PRIORITY  1

#define DEFAULT_NET_TX  0
#define DEFAULT_NET_TX_INTERVAL 0
#define DEFAULT_CONFIG_TIMEOUT  5000
#define DEFAULT_CONFIG_LPN_TIMEOUT  (10 * 1000)

typedef struct nettxConfig{
  uint8_t nettxCnt;
  uint8_t nettxInterval;
}nettxConfig_t;

typedef struct configTimeouts{
  uint32_t normalTimeout;
  uint32_t lpnTimeout;
} configTimeouts_t;

int networkInit(void);
int genericMainLoop(void);
void normalReset(void);
void factoryReset(void);

int enOrDisUnprovBeaconScanning(int enable);
int generic_event_handler(struct gecko_cmd_packet *e);

int getNetworkIdByNodeId(int nodeId);
/* #define RELAY_BIT                                   (0x01 << 0) */
/* #define PROXY_BIT                                   (0x01 << 1) */
/* #define FRIEND_BIT                                  (0x01 << 2) */
/* #define LPN_BIT                                     (0x01 << 3) */
/*  */
/* #define FEATURE_BITS_VALID                          (0x01 << 4) */
/* #define TTL_BIT                                     (0x01 << 5) */
/* #define NET_TRANS_BIT                               (0x01 << 6) */
/*  */
/* #define ALL_FEATURE_BITS                            (RELAY_BIT | PROXY_BIT | FRIEND_BIT) */

static inline bool featureIsEnabled(const char *pFeatures, char c)
{
  if ((c != 'r'
       && c != 'f'
       && c != 'p'
       && c != 'l')
      || pFeatures == NULL
      || *pFeatures == '\0') {
    return false;
  }

  for (int i = 0; i < 4; i++) {
    if (pFeatures[i] == c) {
      return true;
    }
  }
  return false;
}

#ifdef __cplusplus
}
#endif
#endif //NETWORK_MANAGE_H
