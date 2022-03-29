/*************************************************************************
    > File Name: /home/zhfu/work/projs/provisioner/src/conf_out.c
    > Author: Kevin
    > Created Time: 2019-03-21
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include "stdio.h"

#include "utils.h"
#include "err_code.h"
#include "conf_out.h"
#include "prov_assert.h"
#include "config.h"

/* Defines  *********************************************************** */

/* Static Variables *************************************************** */
static int loaded = 0;
static nvmConf_t conf;
static FILE *pf = NULL;

/* Static Functions Declaractions ************************************* */

/* Global Variables *************************************************** */

static int getKeyValueSeparate(char *input, char **pValue)
{
  /* strchr */
  char *p = input;
  int ret = E_FILE;

  if (*p == '#') {
    return E_COMT;
  }

  while (*p != '\0') {
    if (*p == SEPARATE_FLAG) {
      *p = '\0';
      *pValue = p + 1;
      ret = E_SUC;
    } else if (ret == E_SUC && *p == '\n') {
      *p = '\0';
      break;
    }
    p++;
  }

  return ret;
}

static int saveToFile(const char filename[])
{
  char buf[MAX_LINE_SIZE], *pFileName = NULL;

  softASSERT(pf == NULL);
  /* softASSERT(filename == NULL); */
  pFileName = OUTPUT_FILENAME;
  pf = fopen(pFileName, "w+");
  for (int i = 0; i < conf.count; i++) {
    snprintf(buf, MAX_LINE_SIZE, "%s:%s\n", conf.pKV[i]->key, conf.pKV[i]->value);
    fputs(buf, pf);
  }
  fclose(pf);
  pf = NULL;

  return E_SUC;
}

int saveKeyValue(const char *pKey, const char *pValue)
{
  int i = 0;
  if (!pKey || !pValue) {
    return E_INVP;
  }

  if (!loaded) {
    loadOutputFile(NULL);
    hardASSERT(loaded);
  }

  for (; i < conf.count; i++) {
    if (!strcmp(pKey, conf.pKV[i]->key)) {
      strncpy(conf.pKV[i]->value, pValue, MAX_VALUE_SIZE);
      goto save;
    }
  }

  conf.pKV[conf.count] = newKV();
  strncpy(conf.pKV[conf.count]->key, pKey, MAX_KEY_SIZE);
  strncpy(conf.pKV[conf.count]->value, pValue, MAX_VALUE_SIZE);
  conf.count++;

  save:
  saveToFile(NULL);
  return E_SUC;
}

char *getValueByKey(const char *pKey)
{
  if (!loaded) {
    loadOutputFile(NULL);
    hardASSERT(loaded);
  }

  if (!pKey) {
    return NULL;
  }
  for (int i = 0; i < conf.count; i++) {
    if (!strcmp(pKey, conf.pKV[i]->key)) {
      return conf.pKV[i]->value;
    }
  }

  return NULL;
}

static void clearMemory(void)
{
  for (int i = 0; i < conf.count; i++) {
    if (conf.pKV[i]) {
      free(conf.pKV[i]);
      conf.pKV[i] = NULL;
    }
  }
  memset(&conf, 0, sizeof(nvmConf_t));
}

int loadOutputFile(const char *filename)
{
  char buf[MAX_LINE_SIZE];
  char *pRet = NULL, *pValue = NULL, *pFileName = NULL;
  int ret = 0, exist = 0;
#if 0
  if (!filename) {
    return E_INVP;
  }
#else
  pFileName = OUTPUT_FILENAME;
#endif

  pf = fopen(pFileName, "r+");

  if (pf == NULL) {
    /* File not exits, create */
    pf = fopen(pFileName, "w+");
  }
  softASSERT(pf);

  clearMemory();

  while ((pRet = fgets(buf, MAX_LINE_SIZE, pf)) != NULL) {
    if (E_SUC != (ret = getKeyValueSeparate(buf, &pValue))) {
      if (ret != E_COMT) {
        CS_ERR("The output file contains invalid line, format - [key:value]\n");
      }
      continue;
    }

    for (int i = 0; i < conf.count; i++) {
      if (!strcmp(buf, conf.pKV[i]->key)) {
        exist = 1;
        break;
      }
    }
    if (exist) {
      continue;
    }

    conf.pKV[conf.count] = newKV();
    strncpy(conf.pKV[conf.count]->key, buf, MAX_KEY_SIZE);
    strncpy(conf.pKV[conf.count]->value, pValue, MAX_VALUE_SIZE);
    conf.count++;
  }

  fclose(pf);
  pf = NULL;
  loaded = 1;
  return 0;
}
