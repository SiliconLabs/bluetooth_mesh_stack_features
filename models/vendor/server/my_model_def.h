/***************************************************************************//**
 * @file my_model_def.h
 * @brief Vendor model definitions
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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
 ******************************************************************************
 * # Experimental Quality
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
 ******************************************************************************/

#ifndef MY_MODEL_DEF_H_
#define MY_MODEL_DEF_H_

// provisioning locally
#define PROV_LOCALLY

#define PRIMARY_ELEMENT                 0
#define MY_VENDOR_ID                    0x1111

#define MY_MODEL_SERVER_ID              0x1111
#define MY_MODEL_CLIENT_ID              0x2222

#define TEMP_DATA_LENGTH                4
#define UPDATE_INTERVAL_LENGTH          1
#define UNIT_DATA_LENGTH                1

#define NUMBER_OF_OPCODES               10

#define ACK_REQ                         (0x1)
#define STATUS_UPDATE_REQ               (0x2)

#define INDEX_OF(x)                     ((x) - 1)

typedef enum {
  temperature_get = 0x1,
  temperature_status,
  unit_get,
  unit_set,
  unit_set_unack,
  unit_status,
  update_interval_get,
  update_interval_set,
  update_interval_set_unack,
  update_interval_status
} my_msg_t;

typedef enum {
  celsius = 0x1,
  fahrenheit
} unit_t;

typedef struct {
  uint16_t elem_index;
  uint16_t vendor_id;
  uint16_t model_id;
  uint8_t publish; // publish - 1, not - 0
  uint8_t opcodes_len;
  uint8_t opcodes_data[NUMBER_OF_OPCODES];
} my_model_t;

#endif /* MY_MODEL_DEF_H_ */
