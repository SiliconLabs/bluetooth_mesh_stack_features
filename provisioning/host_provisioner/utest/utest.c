/*************************************************************************
    > File Name: /home/zhfu/work/projs/provisioner/utest/utest.c
    > Author: Kevin
    > Created Time: 2019-03-06
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include "config.h"
/* #include "app.h" */
#include "utils.h"
#include "uart.h"
#include "scanner.h"
#include "conf_out.h"

#if (XML_SUP == 1)
#include "xml_parser.h"
#endif
#include "conf_generic_parser.h"

#include "gecko_bglib.h"

#include "utest.h"

/* Defines  *********************************************************** */
#pragma GCC diagnostic ignored "-Wunused-function"

/* Static Variables *************************************************** */

/* Static Functions Declaractions ************************************* */
static void conf_out_test(void);
static void scanner_test(void);
static void conf_parser_test(void);

/* Global Variables *************************************************** */
void utestAll(int enable, int end)
{
  if (!enable) {
    return;
  }

  /* conf_out_test(); */
  /* scanner_test(); */
  conf_parser_test();

  if (end) {
    exit(EXIT_SUCCESS);
  }
}

static void conf_out_test(void)
{
  char buf[200];
  loadOutputFile(NULL);

  printf("-------------------------------------------------\n");
  saveKeyValue("lastfile", "first");
  snprintf(buf, 200, "%s %s", "cat", OUTPUT_FILENAME);
  system(buf);

  printf("-------------------------------------------------\n");
  saveKeyValue("lastfile", "should changed");
  snprintf(buf, 200, "%s %s", "cat", OUTPUT_FILENAME);
  system(buf);

  printf("-------------------------------------------------\n");
  saveKeyValue("newconfffff", "a very lllllllllllllllllllllllllllong value");
  snprintf(buf, 200, "%s %s", "cat", OUTPUT_FILENAME);
  system(buf);

  printf("-------------------------------------------------\n");
  saveKeyValue("newconfffff", "a very more lllllllllllllllllllllllllllong value");
  snprintf(buf, 200, "%s %s", "cat", OUTPUT_FILENAME);
  system(buf);
}

static void scanner_test(void)
{
  struct gecko_msg_mesh_prov_unprov_beacon_evt_t *evt;
  evt = (struct gecko_msg_mesh_prov_unprov_beacon_evt_t *)malloc(sizeof(struct gecko_msg_mesh_prov_unprov_beacon_evt_t) + 16);
  memset(evt, 0, sizeof(struct gecko_msg_mesh_prov_unprov_beacon_evt_t));

  ScannerInit();

  int ret = unprovisionedBeaconFoundCB(evt);
  printf("unprovisionedBeaconFoundCB ret = %d\n", ret);
  memset(evt->uuid.data, 1, 16);
  ret = unprovisionedBeaconFoundCB(evt);
  printf("unprovisionedBeaconFoundCB ret = %d\n", ret);
  memset(evt->uuid.data, 2, 16);
  ret = unprovisionedBeaconFoundCB(evt);
  printf("unprovisionedBeaconFoundCB ret = %d\n", ret);
}

void conf_parser_test(void)
{
#if (XML_SUP == 1)
  int ret;
  hardASSERT(0 == xmlConfigInit(malloc, free, NULL));
  networkConfig_t *pConfig = NULL;
  for (int i = 0; i < 4000; i++) {
    ret = xmlConfigOpen("config/conf_utest.xml",
                        "config/template.xml",
                        FL_CREATE | FL_TRUNC,
                        (void **)&pConfig);
    if (ret) {
      CS_ERR("xmlConfigOpen error [%d]\n", ret);
    }
    if (pConfig) {
      xmlConfigFreeConfig(NULL);
      pConfig = NULL;
    }
    hardASSERT(!pConfig);
  }

  ret = xmlConfigOpen("config/conf_full.xml",
                      "config/template.xml",
                      0,
                      (void **)&pConfig);
  if (ret) {
    CS_ERR("xmlConfigOpen error [%d]\n", ret);
  } else {
    if (pConfig) {
      xmlConfigFreeConfig((void **)&pConfig);
    }
  }

  CS_LOG("conf_parser_test passed.\n");

  return;
  ret = xmlConfigOpen("config/conf_full.xml",
                      "config/template.xml",
                      0,
                      (void **)&pConfig);
  if (ret) {
    CS_ERR("xmlConfigOpen error [%d]\n",
           ret);
  } else {
    /* if (pConfig) { */
    /* free(pConfig); */
    /* } */
#if (ADD_NOT_USED_FUNC == 1)
    char nkID[] = "00";
    char akID[] = "01";
    char key1[] = "000b573152aa2d7665447362616c6953";
    char key2[] = "000b573152a22d7665447362616c6953";
    char key3[] = "001b573152a22d7665447362616c6953";

    ret = modifyDoneFlag("conftest.xml", netKeyE, nkID, "01");
    uint8_t u = 0;
    printf("%d modify ret = %d\n", u++, ret);

    ret = modifyDoneFlag("conftest.xml", appKeyE, akID, "01");
    printf("%d modify ret = %d\n", u++, ret);

    ret = modifyDoneFlag("conftest.xml", NodeE, key1, "01");
    printf("%d modify ret = %d\n", u++, ret);

    ret = modifyDoneFlag("conftest.xml", NodeE, key2, "01");
    printf("%d modify ret = %d\n", u++, ret);

    ret = modifyDoneFlag("conftest.xml", NodeE, key3, "01");
    printf("%d modify ret = %d\n", u++, ret);
#endif
  }

#if (USE_OUTPUT_FILE == 1)
  memset(&cfged, 0, sizeof(Configed_t));
  ret = loadOutputFile("conf.out", &cfged);
  CS_LOG("load ret = %d\n", ret);
  cfged.netkeyCnt = 4;
  cfged.appkeyCnt = 3;
  cfged.nodeCnt = 6;

  int i;
  for (i = 0; i < 4; i++) {
    cfged.pNetKeyIds[i] = i;
  }

  for (i = 0; i < 3; i++) {
    cfged.pAppKeyIds[i] = i;
  }

  for (i = 0; i < 6; i++) {
    for (int a = 0; a < 16; a++) {
      cfged.pNodes[i].data[a] = i + a;
    }
  }

  ret = saveOutputFile("conf.out", &cfged);
  CS_LOG("save ret = %d\n", ret);
#endif
#endif
}
