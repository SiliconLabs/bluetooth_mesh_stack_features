/***************************************************************************//**
 * @file
 * @brief Application code
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
 *******************************************************************************
 * # Experimental Quality
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
 ******************************************************************************/

#include "sl_btmesh.h"
#include "sl_btmesh_ctl_client.h"
#include "sl_btmesh_lighting_client.h"

/// Maximum lightness percentage value
#define LIGHTNESS_PCT_MAX              100
/// Maximum temperature percentage value
#define TEMPERATURE_PCT_MAX       100

/// lightness level percentage
static uint8_t lightness_percent = 0;
/// lightness level percentage when switching ON
static uint8_t lightness_percent_switch_on = LIGHTNESS_PCT_MAX;
/// Initial temperature percentage value
#define TEMPERATURE_PCT_INIT      50
/// temperature level percentage
static uint8_t temperature_percent = TEMPERATURE_PCT_INIT;

/*******************************************************************************
 * This function change the lightness and sends it to the server.
 *
 * @param[in] change_percentage  Defines lightness percentage change,
 * possible values are  -100% - + 100%.
 *
 ******************************************************************************/
void sl_btmesh_change_lightness(int8_t change_percentage)
{
  // Adjust light brightness, using Light Lightness model
  if (change_percentage > 0) {
    lightness_percent += change_percentage;
    if (lightness_percent > LIGHTNESS_PCT_MAX) {
#if (SL_BTMESH_LIGHT_LIGHTNESS_WRAP_ENABLED_CFG_VAL != 0)
      lightness_percent = 0;
#else
      lightness_percent = LIGHTNESS_PCT_MAX;
#endif
    }
  } else {
    if (lightness_percent < (-change_percentage)) {
#if (SL_BTMESH_LIGHT_LIGHTNESS_WRAP_ENABLED_CFG_VAL != 0)
      lightness_percent = LIGHTNESS_PCT_MAX;
#else
      lightness_percent = 0;
#endif
    } else {
      lightness_percent += change_percentage;
    }
  }

  if (lightness_percent != 0) {
    lightness_percent_switch_on = lightness_percent;
  }
  sl_btmesh_set_lightness(lightness_percent);
}


/*******************************************************************************
 * This function change the color temperature and sends it to the server.
 *
 * @param[in] change_percentage  Defines the color temperature percentage change,
 * possible values are  -100% - + 100%.
 *
 ******************************************************************************/
void sl_btmesh_change_temperature(int8_t change_percentage)
{
  // Adjust light brightness, using Light Lightness model
  if (change_percentage > 0) {
    temperature_percent += change_percentage;
    if (temperature_percent > TEMPERATURE_PCT_MAX) {
#if (SL_BTMESH_CTL_CLIENT_TEMPERATURE_WRAP_ENABLED_CFG_VAL != 0)
      temperature_percent = 0;
#else
      temperature_percent = TEMPERATURE_PCT_MAX;
#endif
    }
  } else {
    if (temperature_percent < (-change_percentage)) {
#if (SL_BTMESH_CTL_CLIENT_TEMPERATURE_WRAP_ENABLED_CFG_VAL != 0)
      temperature_percent = TEMPERATURE_PCT_MAX;
#else
      temperature_percent = 0;
#endif
    } else {
      temperature_percent += change_percentage;
    }
  }

  sl_btmesh_set_temperature(temperature_percent);
}