/* System header */
#include <stdio.h>

/* This will be the model agregator: config and load model */
#include "config.h"
#include "bg_types.h"

// DCD content of the last provisioned device. (the example code decodes up to two elements, but
// only the primary element is used in the configuration to simplify the code)
tsDCD_ElemContent _sDCD_Prim;
tsDCD_ElemContent _sDCD_2nd; /* second DCD element is decoded if present, but not used for anything (just informative) */

extern uint8_t _dcd_raw[256]; // raw content of the DCD received from remote node
extern uint8_t _dcd_raw_len;

static void DCD_decode_element(tsDCD_Elem *pElem, tsDCD_ElemContent *pDest);

void DCD_decode(void)
{
  tsDCD_Header *pHeader;
  tsDCD_Elem *pElem;
  uint8_t byte_offset;

  pHeader = (tsDCD_Header *)&_dcd_raw;

  printf("DCD: company ID %4.4x, Product ID %4.4x\r\n", pHeader->companyID, pHeader->productID);

  pElem = (tsDCD_Elem *)pHeader->payload;

  // decode primary element:
  DCD_decode_element(pElem, &_sDCD_Prim);

  // check if DCD has more than one element by calculating where we are currently at the raw
  // DCD array and compare against the total size of the raw DCD:
  byte_offset = 10 + 4 + pElem->numSIGModels * 2 + pElem->numVendorModels * 4; // +10 for DCD header, +4 for header in the DCD element

  if(byte_offset < _dcd_raw_len)
  {
    // set elem pointer to the beginning of 2nd element:
    pElem = (tsDCD_Elem *)&(_dcd_raw[byte_offset]);

    printf("Decoding 2nd element (just informative, not used for anything)\r\n");
    DCD_decode_element(pElem, &_sDCD_2nd);
  }
}

/* function for decoding one element inside the DCD. Parameters:
 *  pElem: pointer to the beginning of element in the raw DCD data
 *  pDest: pointer to a struct where the decoded values are written
 * */
static void DCD_decode_element(tsDCD_Elem *pElem, tsDCD_ElemContent *pDest)
{
  uint16_t *pu16;
  int i;

  memset(pDest, 0, sizeof(*pDest));

  pDest->numSIGModels = pElem->numSIGModels;
  pDest->numVendorModels = pElem->numVendorModels;

  printf("Num sig models: %d\r\n", pDest->numSIGModels );
  printf("Num vendor models: %d\r\n", pDest->numVendorModels);

  if(pDest->numSIGModels > MAX_SIG_MODELS)
  {
    printf("ERROR: number of SIG models in DCD exceeds MAX_SIG_MODELS (%u) limit!\r\n", MAX_SIG_MODELS);
    return;
  }
  if(pDest->numVendorModels > MAX_VENDOR_MODELS)
  {
    printf("ERROR: number of VENDOR models in DCD exceeds MAX_VENDOR_MODELS (%u) limit!\r\n", MAX_VENDOR_MODELS);
    return;
  }

  // set pointer to the first model:
  pu16 = (uint16_t *)pElem->payload;

  // grab the SIG models from the DCD data
  for(i=0;i<pDest->numSIGModels;i++)
  {
    pDest->SIG_models[i] = *pu16;
    pu16++;
    printf("model ID: %4.4x\r\n", pDest->SIG_models[i]);
  }

  // grab the vendor models from the DCD data
  for (i = 0; i < pDest->numVendorModels; i++) {
    pDest->vendor_models[i].vendor_id = *pu16;
    pu16++;
    pDest->vendor_models[i].model_id = *pu16;
    pu16++;

    printf("vendor ID: %4.4x, model ID: %4.4x\r\n", pDest->vendor_models[i].vendor_id, pDest->vendor_models[i].model_id);
  }
}
