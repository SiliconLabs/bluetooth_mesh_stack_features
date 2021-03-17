/*************************************************************************
    > File Name: conf_out.h
    > Author: Kevin
    > Created Time: 2019-03-21
    >Description:
 ************************************************************************/

#ifndef CONF_OUT_H
#define CONF_OUT_H

#include <stdlib.h>
#include <string.h>

#define MAX_KEY_SIZE                      30
#define MAX_VALUE_SIZE                    512
#define MAX_LINE_SIZE                     (MAX_KEY_SIZE + MAX_VALUE_SIZE + 1)
#define MAX_CONF_ITEMS                    20

#define SEPARATE_FLAG                     ':'

#define KEY_LAST_FILE                     "Last_Conf_File"
#define KEY_ACTION_LOADING_MODE           "Action_Loading_Mode"
#define KEY_NETTX_CNT                     "Network_TX"
#define KEY_NETTX_INT                     "Network_TX_Interval"
#define KEY_TIMEOUT                       "Config_Timeout"
#define KEY_TIMEOUT_LPN                   "Config_Timeout_LPN"

typedef struct __kv{
  char key[MAX_KEY_SIZE];
  char value[MAX_VALUE_SIZE];
}keyValue_t;

typedef struct __nvmConf{
  int count;
  keyValue_t *pKV[MAX_CONF_ITEMS];
}nvmConf_t;

static inline keyValue_t *newKV(void)
{
  keyValue_t *p = (keyValue_t *)malloc(sizeof(keyValue_t));
  memset(p, 0, sizeof(keyValue_t));
  return p;
}

int saveKeyValue(const char *pKey, const char *pValue);
char *getValueByKey(const char *pKey);
int loadOutputFile(const char *filename);
#endif
