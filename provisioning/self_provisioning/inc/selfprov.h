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

#ifndef SELFPROV_H
#define SELFPROV_H

#define UNICAST_ADDRESS             0x0000
#define UADDR_ELEM_BITS             4
#define UADDR_RSVD_BITS             1

#define NETWORK_KEY                 {0x23, 0x98, 0xdf, 0xa5, 0x09, 0x3e, 0x74, 0xbb, 0xc2, 0x45, 0x1f, 0xae, 0xea, 0xd7, 0x67, 0xcd}
#define APPLICATION_KEY             {0x16, 0x39, 0x38, 0x03, 0x9b, 0x8d, 0x8a, 0x20, 0x81, 0x60, 0xa7, 0x93, 0x33, 0x3d, 0x03, 0x61}
#define DEVICE_KEY                  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

#define GRP_SVR_PUB_ADDRESS         0xC100
#define GRP_SVR_PUB_TTL             3
#define GRP_SVR_PUB_PERIOD          0
#define GRP_SVR_PUB_RETRANS         0
#define GRP_SVR_PUB_CREDENTIALS     0
#define GRP_SVR_SUB_ADDRESSES       { 0xC000 }

#define GRP_CLI_PUB_ADDRESS         0xC000
#define GRP_CLI_PUB_TTL             3
#define GRP_CLI_PUB_PERIOD          0
#define GRP_CLI_PUB_RETRANS         0
#define GRP_CLI_PUB_CREDENTIALS     0
#define GRP_CLI_SUB_ADDRESSES       { 0xC100 }

#define MAX_PUB_SUB_MODELS          20

#define PUB_SUB_SERVER_MODELS   { \
    MESH_MODEL_GENERIC_ONOFF_SERVER, \
    MESH_MODEL_GENERIC_LEVEL_SERVER, \
    MESH_MODEL_GENERIC_TRANSITION_TIME_SERVER, \
    MESH_MODEL_GENERIC_POWER_ONOFF_SERVER, \
    MESH_MODEL_GENERIC_POWER_LEVEL_SERVER, \
    MESH_MODEL_GENERIC_BATTERY_SERVER, \
    MESH_MODEL_GENERIC_LOCATION_SERVER, \
    MESH_MODEL_GENERIC_ADMIN_PROPERTY_SERVER, \
    MESH_MODEL_GENERIC_MANUF_PROPERTY_SERVER, \
    MESH_MODEL_GENERIC_USER_PROPERTY_SERVER, \
    MESH_MODEL_GENERIC_CLIENT_PROPERTY_SERVER, \
    MESH_MODEL_SENSOR_SERVER, \
    MESH_MODEL_TIME_SERVER, \
    MESH_MODEL_SCENE_SERVER, \
    MESH_MODEL_SCHEDULER_SERVER, \
    MESH_MODEL_LIGHT_LIGHTNESS_SERVER, \
    MESH_MODEL_LIGHT_CTL_SERVER, \
    MESH_MODEL_LIGHT_CTL_TEMPERATURE_SERVER, \
    MESH_MODEL_LIGHT_LC_SERVER \
}

#define PUB_SUB_CLIENT_MODELS   { \
    MESH_MODEL_GENERIC_ONOFF_CLIENT, \
    MESH_MODEL_GENERIC_LEVEL_CLIENT, \
    MESH_MODEL_GENERIC_TRANSITION_TIME_CLIENT, \
    MESH_MODEL_GENERIC_POWER_ONOFF_CLIENT, \
    MESH_MODEL_GENERIC_POWER_LEVEL_CLIENT, \
    MESH_MODEL_GENERIC_BATTERY_CLIENT, \
    MESH_MODEL_GENERIC_LOCATION_CLIENT, \
    MESH_MODEL_GENERIC_PROPERTY_CLIENT, \
    MESH_MODEL_SENSOR_CLIENT, \
    MESH_MODEL_TIME_CLIENT, \
    MESH_MODEL_SCENE_CLIENT, \
    MESH_MODEL_SCHEDULER_CLIENT, \
    MESH_MODEL_LIGHT_LIGHTNESS_CLIENT, \
    MESH_MODEL_LIGHT_CTL_CLIENT, \
    MESH_MODEL_LIGHT_LC_CLIENT \
}

enum mesh_model_id_e {
    MESH_MODEL_CONFIGURATION_SERVER             = 0x0000,
    MESH_MODEL_CONFIGURATION_CLIENT,
    MESH_MODEL_HEALTH_SERVER,
    MESH_MODEL_HEALTH_CLIENT,
    MESH_MODEL_GENERIC_ONOFF_SERVER             = 0x1000,
    MESH_MODEL_GENERIC_ONOFF_CLIENT,
    MESH_MODEL_GENERIC_LEVEL_SERVER,
    MESH_MODEL_GENERIC_LEVEL_CLIENT,
    MESH_MODEL_GENERIC_TRANSITION_TIME_SERVER,
    MESH_MODEL_GENERIC_TRANSITION_TIME_CLIENT,
    MESH_MODEL_GENERIC_POWER_ONOFF_SERVER,
    MESH_MODEL_GENERIC_POWER_ONOFF_SETUP_SERVER,
    MESH_MODEL_GENERIC_POWER_ONOFF_CLIENT,
    MESH_MODEL_GENERIC_POWER_LEVEL_SERVER,
    MESH_MODEL_GENERIC_POWER_LEVEL_SETUP_SERVER,
    MESH_MODEL_GENERIC_POWER_LEVEL_CLIENT,
    MESH_MODEL_GENERIC_BATTERY_SERVER,
    MESH_MODEL_GENERIC_BATTERY_CLIENT,
    MESH_MODEL_GENERIC_LOCATION_SERVER,
    MESH_MODEL_GENERIC_LOCATION_SETUP_SERVER,
    MESH_MODEL_GENERIC_LOCATION_CLIENT,
    MESH_MODEL_GENERIC_ADMIN_PROPERTY_SERVER,
    MESH_MODEL_GENERIC_MANUF_PROPERTY_SERVER,
    MESH_MODEL_GENERIC_USER_PROPERTY_SERVER,
    MESH_MODEL_GENERIC_CLIENT_PROPERTY_SERVER,
    MESH_MODEL_GENERIC_PROPERTY_CLIENT,
    MESH_MODEL_SENSOR_SERVER                    = 0x1100,
    MESH_MODEL_SENSOR_SETUP_SERVER,
    MESH_MODEL_SENSOR_CLIENT,
    MESH_MODEL_TIME_SERVER                      = 0x1200,
    MESH_MODEL_TIME_SETUP_SERVER,
    MESH_MODEL_TIME_CLIENT,
    MESH_MODEL_SCENE_SERVER,
    MESH_MODEL_SCENE_SETUP_SERVER,
    MESH_MODEL_SCENE_CLIENT,
    MESH_MODEL_SCHEDULER_SERVER,
    MESH_MODEL_SCHEDULER_SETUP_SERVER,
    MESH_MODEL_SCHEDULER_CLIENT,
    MESH_MODEL_LIGHT_LIGHTNESS_SERVER           = 0x1300,
    MESH_MODEL_LIGHT_LIGHTNESS_SETUP_SERVER,
    MESH_MODEL_LIGHT_LIGHTNESS_CLIENT,
    MESH_MODEL_LIGHT_CTL_SERVER,
    MESH_MODEL_LIGHT_CTL_SETUP_SERVER,
    MESH_MODEL_LIGHT_CTL_CLIENT,
    MESH_MODEL_LIGHT_CTL_TEMPERATURE_SERVER,
    MESH_MODEL_LIGHT_HSL_SERVER,
    MESH_MODEL_LIGHT_HSL_SETUP_SERVER,
    MESH_MODEL_LIGHT_HSL_CLIENT,
    MESH_MODEL_LIGHT_HSL_HUE_SERVER,
    MESH_MODEL_LIGHT_HSL_SATURATION_SERVER,
    MESH_MODEL_LIGHT_XYL_SERVER,
    MESH_MODEL_LIGHT_XYL_SETUP_SERVER,
    MESH_MODEL_LIGHT_XYL_CLIENT,
    MESH_MODEL_LIGHT_LC_SERVER,
    MESH_MODEL_LIGHT_LC_SETUP_SERVER,
    MESH_MODEL_LIGHT_LC_CLIENT
};

void self_provisioning(void);

#endif /* SELFPROV_H */
