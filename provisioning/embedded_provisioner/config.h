/***************************************************************************//**
 * @file
 * @brief Silicon Labs Bluetooth mesh light example this example implements an
 * Embedded provisioner.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 *******************************************************************************
 * # Experimental Quality
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
 ******************************************************************************/
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
