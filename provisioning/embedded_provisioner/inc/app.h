/***************************************************************************//**
 * @file
 * @brief Silicon Labs Bluetooth mesh light example this example implements an
 * Embedded provisioner.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef APP_H
#define APP_H

/* System header */
#include <stdio.h>

#include "sl_bluetooth.h"

/* Provisioning strategy */
typedef enum {
  eMESH_PROV_ALL=0,
  eMESH_PROV_NEXT
} eMesh_Prov_Node_t;

typedef struct {
  bd_addr address;
  uuid_128 uuid;
  uint8_t is_provisioned;
} device_table_entry_t;

/***************************************************************************//**
 * Application Init.
 ******************************************************************************/
void app_init(void);

/***************************************************************************//**
 * Application Process Action.
 ******************************************************************************/
void app_process_action(void);

void init_bluetooth_device_table();

void provision_bluetooth_device(device_table_entry_t *device);

void select_bluetooth_device(eMesh_Prov_Node_t eStrategy);

void print_bluetooth_device_table();

void app_button_press_select_provisionee_cb(uint8_t button, uint8_t duration);

void app_button_press_cb(uint8_t button, uint8_t duration);

#endif
