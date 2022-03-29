/*************************************************************************
    > File Name: err_code.h
    > Author: Kevin
    > Created Time: 2019-03-06
    >Description:
 ************************************************************************/

#ifndef ERR_CODE_H
#define ERR_CODE_H

typedef enum {
  E_SUC = 0,
  E_NSPT = 1, /* Not supported */
  E_INVP = 2, /* Invalid parameters */
  E_IO = 3, /* IO error */
  E_FILE = 4, /* Error file content */
  E_NEXIST = 5,  /* File not exist */
  E_NFND = 6, /* Not found */
  E_NOESIZE = 7,  /* Not enough size */
  E_UNSPEC = 8,  /* Unspecific error */
  E_INTNUL = 9,  /* Internal NULL */
  E_NINIT = 10, /* Not init yet */
  E_NIMPL = 11, /* Not implemented */
  E_SUBC = 12, /* Sub call error */
  E_BGAPI = 13,  /* BGAPI retrun not success */
  E_BUSY = 14,  /* Busy state */
  E_OOM = 15,  /* Out of Memory */

  /* -----------------State Machine Errors----------------- */
  E_TONEXT,  /* Guard of the entry failed */

  /* -----------------NCP Dev/Mesh Errors----------------- */
  E_NOTRD, /* Not ready */
  E_NOTPVS, /* Not provisioned */

  /* -----------------Console Commands Errors----------------- */
  E_CMDTL, /* Command too long */
  E_BFFL,  /* Command Buffer Full */

  /* -----------------Conf_out Errors----------------- */
  E_COMT  /* Read a comment line */
}errcode_t;

extern const char *ERR_NAME[];
#endif
