/*************************************************************************
    > File Name: scanner.h
    > Author: Kevin
    > Created Time: 2019-03-13
    >Description:
 ************************************************************************/

#ifndef SCANNER_H
#define SCANNER_H

#include "err_code.h"
#include "bg_stack/gecko_bglib.h"

#if 0
int ScannerInit(const char *pFN);
int unprovisionedBeaconFoundCB(struct gecko_msg_mesh_prov_unprov_beacon_evt_t *e);
#endif

errcode_t ScannerInit(void);
int unprovisionedBeaconFoundCB(void *p);

#endif
