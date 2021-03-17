/*************************************************************************
    > File Name: src/scanner.c
    > Author: Kevin
    > Created Time: 2019-03-13
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
#include <stdbool.h>

#include "config.h"
#include "utils.h"
#include "scanner.h"
#include "conf_generic_parser.h"

/* Defines  *********************************************************** */

errcode_t ScannerInit(void)
{
  return E_SUC;
}

int unprovisionedBeaconFoundCB(void *p)
{
  struct gecko_msg_mesh_prov_unprov_beacon_evt_t *e = p;
  int ret = 0;

  if (nodeIsRecorded(e->uuid.data)) {
    return E_SUC;
  }

  if (E_SUC != (ret = addOneNodeToConfigFile(e->uuid.data))) {
    return ret;
  }

  CS_MSG("Found an unprovisioned device, recorded it into the config file\n");
  return E_SUC;
}
