/*************************************************************************
    > File Name: device_database.h
    > Author: Kevin
    > Created Time: 2019-04-23
    >Description:
 ************************************************************************/

#ifndef DEVICE_DATABASE_H
#define DEVICE_DATABASE_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "bg_stack/gecko_bglib.h"

#define DEFAULT_MAX_NODES 255

#define MAX_DEVICE_COUNT_IN_DDB DEFAULT_MAX_NODES

typedef enum {
  typeTBR,
  typeTBBL
}remType_t;

typedef struct ddb{
  uint8_t nodeCnt;
  uuid_128 pNodes[MAX_DEVICE_COUNT_IN_DDB];
}ddb_t;

void ddbInit(void);
int addOneDeviceToDDB(uuid_128 u);
int removeDeviceFromLDDB(int t, int idxInPconfig);
int isNodeIndexInDeviceDB(int i);
int addOneDeviceToDDBWithRawData(const uint8_t *data);
#ifdef __cplusplus
}
#endif
#endif //DEVICE_DATABASE_H
