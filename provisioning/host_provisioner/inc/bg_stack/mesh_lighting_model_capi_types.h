/******************************************************************************/
/**
 * @file   mesh_lighting_model_capi_types.h
 * @brief  Silicon Labs Bluetooth Mesh Lighting Model API
 *
 * Please see the @ref mesh_generic "generic model API"
 *
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/*
 * C API for lighting models
 */

#ifndef MESH_LIGHTING_MODEL_CAPI_TYPES_H
#define MESH_LIGHTING_MODEL_CAPI_TYPES_H

/*
 * Lighting model IDs
 */
/** Light lightness server */
#define MESH_LIGHTING_LIGHTNESS_SERVER_MODEL_ID 0x1300
/** Light lightness setup server */
#define MESH_LIGHTING_LIGHTNESS_SETUP_SERVER_MODEL_ID 0x1301
/** Light lightness client */
#define MESH_LIGHTING_LIGHTNESS_CLIENT_MODEL_ID 0x1302

/** Light CTL server */
#define MESH_LIGHTING_CTL_SERVER_MODEL_ID 0x1303
/** Light CTL setup server */
#define MESH_LIGHTING_CTL_SETUP_SERVER_MODEL_ID 0x1304
/** Light CTL client */
#define MESH_LIGHTING_CTL_CLIENT_MODEL_ID 0x1305
/** Light CTL temperature server */
#define MESH_LIGHTING_CTL_TEMPERATURE_SERVER_MODEL_ID 0x1306

/** Light CTL Temperature Minimum */
#define MESH_LIGHTING_CTL_TEMPERATURE_MIN 0x0320

/** Light CTL Temperature Maximum */
#define MESH_LIGHTING_CTL_TEMPERATURE_MAX 0x4e20

#endif /* MESH_LIGHTING_MODEL_CAPI_TYPES_H */
