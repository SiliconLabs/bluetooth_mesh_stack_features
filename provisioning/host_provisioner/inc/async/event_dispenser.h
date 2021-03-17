/*************************************************************************
    > File Name: event_dispenser.h
    > Author: Kevin
    > Created Time: 2019-07-03
    >Description:
 ************************************************************************/

#ifndef EVENT_DISPENSER_H
#define EVENT_DISPENSER_H
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "gecko_bglib.h"

typedef enum {
  to_tba_em = 0,
  to_tbb_em = 1,
  to_tbc_em = 2,
  to_generic_em = 3,
  to_max_invalid = 4
} set_t;

typedef int (*evt_handler)(struct gecko_cmd_packet *);

typedef struct event_node{
  uint32_t event_id;
  struct event_node *next;
}event_node_t;

typedef struct event_set{
  size_t size; /* informative */
  event_node_t *head;
  event_node_t *tail;
}event_set_t;

typedef struct event_func_pair{
  event_set_t set;
  evt_handler fn;
}event_func_pair_t;

typedef enum {
  noInitEnum,
  unsecureEnum,
  secureEnum
} smode_t;

typedef struct interfaceArgs{
  smode_t  smode;
  char     *configFilePath;
  char     *pSerialPort;
  uint32_t baudRate;
  char     *pSerSockPath;
  bool     encrypted;
  void(*bglib_output)(uint32_t len1, uint8_t * data1);
  int32_t (*bglib_input)(uint32_t len1, uint8_t* data1);
  int32_t(*bglib_peek)(void);
} interfaceArgs_t;

void bglib_interface_init_sync(const interfaceArgs_t *p);
void event_dispenser(void);

int getSyncUpStatus(void);
void forceReSyncUpHostAndTarget(void);
char *get_config_file_path(void);
#ifdef __cplusplus
}
#endif
#endif //EVENT_DISPENSER_H
