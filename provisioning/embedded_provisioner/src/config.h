#ifndef CONFIG_H
#define CONFIG_H

#include "bg_types.h"

// max number of SIG models in the DCD
#define MAX_SIG_MODELS    16

// max number of vendor models in the DCD
#define MAX_VENDOR_MODELS 4

typedef struct
{
  uint16_t model_id;
  uint16_t vendor_id;
} tsModel;

/* struct for storing the content of one element in the DCD */
typedef struct
{
  uint16_t SIG_models[MAX_SIG_MODELS];
  uint8_t numSIGModels;

  tsModel vendor_models[MAX_VENDOR_MODELS];
  uint8_t numVendorModels;
}tsDCD_ElemContent;

/* this struct is used to help decoding the raw DCD data */
typedef struct
{
  uint16_t companyID;
  uint16_t productID;
  uint16_t version;
  uint16_t replayCap;
  uint16_t featureBitmask;
  uint8_t payload[1];
} tsDCD_Header;

/* this struct is used to help decoding the raw DCD data */
typedef struct
{
  uint16_t location;
  uint8_t numSIGModels;
  uint8_t numVendorModels;
  uint8_t payload[1];
} tsDCD_Elem;

void DCD_decode(void);
#endif
