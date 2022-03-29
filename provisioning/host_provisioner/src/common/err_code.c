/*************************************************************************
    > File Name: err_code.c
    > Author: Kevin
    > Created Time: 2019-05-13
    >Description:
 ************************************************************************/

/* Includes *********************************************************** */
#include "err_code.h"

/* Defines  *********************************************************** */

/* Global Variables *************************************************** */
const char *ERR_NAME[] = {
  "Success",
  "Not Supported",
  "Invalid Parameters",
  "IO Error",
  "Error File Content",
  "File Not Exist",
  "Not Found",
  "Not Enough Size",
  "Unspecific error",
  "Internal NULL",
  "Not init yet",
  "Not implemented",
  "Sub call error",
  "BGAPI retrun not success",
  "Busy state",
  "Out of Memory",

  /* -----------------State Machine Errors----------------- */
  "Guard of the Entry Failed",

  /* -----------------NCP Dev/Mesh Errors----------------- */
  "Not Ready",
  "Not Provisioned",

  /* -----------------Console Commands Errors----------------- */
  "Command Too Long",
  "Command Buffer Full",

  /* -----------------Conf_out Errors----------------- */
  "Read a Comment Line"
};

/* Static Variables *************************************************** */

/* Static Functions Declaractions ************************************* */
