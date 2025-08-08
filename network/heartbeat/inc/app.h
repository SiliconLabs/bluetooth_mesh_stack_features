/***************************************************************************//**
 * @file
 * @brief Application interface provided to main().
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
 ******************************************************************************/
#ifndef APP_H
#define APP_H

/* System header */
#include <stdio.h>
#include "sl_bluetooth.h"

/**************************************************************************//**
 * Proceed with execution. (Indicate that it is required to run the application
 * process action.)
 *****************************************************************************/
void app_proceed(void);

/**************************************************************************//**
 * Check if it is required to process with execution.
 * @return true if required, false otherwise.
 *****************************************************************************/
bool app_is_process_required(void);

/**************************************************************************//**
 * Initialize the application for BT Mesh.
 *****************************************************************************/
void app_permanent_memory_alloc(void);

/**************************************************************************//**
 * Custom
 *****************************************************************************/

typedef struct {
  uint16_t address;
  uuid_128 uuid;
} node_registry_entry_t;

typedef struct {
  uint16_t source_address;
  uint16_t target_address;
  bool checked;
} route_registry_entry_t;

void init_node_registry();
bool is_webbrowser_ready();
void start_full_mapping();
void find_route(uint16_t);

#endif // APP_H
