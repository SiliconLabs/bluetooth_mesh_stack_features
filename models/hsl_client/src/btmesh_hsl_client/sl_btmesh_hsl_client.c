/***************************************************************************//**
 * @file
 * @brief Bt Mesh HSL Client module
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
 ******************************************************************************/
#include <stdbool.h>
#include "em_common.h"
#include "sl_status.h"

#include "sl_btmesh_api.h"
#include "sl_bt_api.h"
#include "sl_btmesh_dcd.h"

#include "sl_btmesh_generic_model_capi_types.h"
#include "sl_btmesh_lib.h"

#include "app_assert.h"
#include "sl_simple_timer.h"
#include "sl_btmesh_lighting_client.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#ifdef SL_CATALOG_APP_LOG_PRESENT
#include "app_log.h"
#endif // SL_CATALOG_APP_LOG_PRESENT

#include <gecko_sdk_4.0.1/app/bluetooth/common/btmesh_hsl_client/sl_btmesh_hsl_client.h>

// Warning! The app_btmesh_util shall be included after the component configuration
// header file in order to provide the component specific logging macro.
#include "app_btmesh_util.h"

/***************************************************************************//**
 * @addtogroup Lighting
 * @{
 ******************************************************************************/

/// No flags used for message
#define NO_FLAGS                  0
/// Immediate transition time is 0 seconds
#define IMMEDIATE                 0
/// Callback has not parameters
#define NO_CALLBACK_DATA          (void *)NULL
/// High Priority
#define HIGH_PRIORITY             0
/// Delay unit value for request for hsl messages in millisecond
#define REQ_DELAY_MS              50

/// Max HSL index support
#define HSL_INDEX_MAX              7

#define UINT16_TO_PERCENTAGE(level) ((((uint32_t)(level) * 100) + 32767) / 65535)
#define UINT16_TO_DEGREE(level) ((((uint32_t)(level) * 360) + 32767) / 65535)

/// periodic timer handle
static sl_simple_timer_t hsl_retransmission_timer;

/// periodic timer callback
static void hsl_retransmission_timer_cb(sl_simple_timer_t *handle,
                                        void *data);

/// hue possible values from 0 to 65535, which are mapped to 0° to 360°
static uint16_t target_hue;
/// saturation possible values from 0 to 65535, which are mapped to 0% to 100%
static uint16_t target_saturation;
/// lightness possible values from 0 to 65535, which are mapped to 0% to 100%
static uint16_t target_lightness;

static uint8_t hsl_index = 0;

/// number of hcl requests to be sent
static uint8_t hsl_request_count = 0;
/// hsl transaction identifier
static uint8_t hsl_trid = 0;

/***************************************************************************//**
 * This function publishes one light HSL request to change the HSL status
 * of light(s) in the group.
 *
 * param[in] retrans  Indicates if this is the first request or a retransmission,
 *                    possible values are 0 = first request, 1 = retransmission.
 *
 * @note This application sends multiple HSL requests for each
 *       medium button press to improve reliability.
 *       The transaction ID is not incremented in case of a retransmission.
 ******************************************************************************/
static void send_hsl_request(uint8_t retrans)
{
  struct mesh_generic_request req;
  sl_status_t sc;

  req.kind = mesh_lighting_request_hsl;

  req.hsl.hue = target_hue;
  req.hsl.saturation = target_saturation;
  req.hsl.lightness = target_lightness;

  // Increment transaction ID for each request, unless it's a retransmission
  if (retrans == 0) {
    hsl_trid++;
  }

  // Delay for the request is calculated so that the last request will have
  // a zero delay and each of the previous request have delay that increases
  // in 50 ms steps. For example, when using three HSL requests
  // per button press the delays are set as 100, 50, 0 ms
  uint16_t delay = (hsl_request_count - 1) * REQ_DELAY_MS;

  sc = mesh_lib_generic_client_publish(MESH_LIGHTING_HSL_CLIENT_MODEL_ID,
                                       BTMESH_HSL_CLIENT_MAIN,
                                       hsl_trid,
                                       &req,
                                       IMMEDIATE,     // transition
                                       delay,
                                       NO_FLAGS       // flags
                                       );

  if (sc == SL_STATUS_OK) {
    log_info(HSL_CLIENT_LOGGING_RECALL_SUCCESS, hsl_trid, delay);
  } else {
    log_btmesh_status_f(sc, HSL_CLIENT_LOGGING_CLIENT_PUBLISH_FAIL);
  }

  // Keep track of how many requests has been sent
  if (hsl_request_count > 0) {
    hsl_request_count--;
  }
}

/*******************************************************************************
 * This function change the color HSL and sends it to the server.
 *
 * @param[in] change_percentage  Defines the color temperature percentage change,
 * possible values are  -100% - + 100%.
 *
 ******************************************************************************/
void sl_btmesh_change_hsl(void)
{
  // Adjust HSL
  hsl_index++;

  sl_btmesh_set_hsl(hsl_index%HSL_INDEX_MAX);
}

/*******************************************************************************
 * This function change the HSL and send it to the server.
 *
 * @param[in] new_hsl  Defines new hsl
 * value as index, configure in table.
 *
 ******************************************************************************/
void sl_btmesh_set_hsl(uint8_t new_hsl)
{
  char *color[HSL_INDEX_MAX]={"Off","Red","Green","Blue","Orange","Pink","Purple"};
  //{hue(0-360 degree), saturation(0-100%),lightness(0-100%)}
  uint16_t hsl_table[HSL_INDEX_MAX][3] = {
      {0,0,0},       //Off
      {0,100,50},    //Red
      {120,100,50},  //Green
      {240,100,50},  //Blue
      {39,100,50},   //Orange
      {300,76,72},   //Pink
      {248,53,58},   //Purple
  };
  target_hue = (double)hsl_table[new_hsl][0]/360*65535;
  target_saturation = (double)hsl_table[new_hsl][1]/100*65535;
  target_lightness = (double)hsl_table[new_hsl][2]/100*65535;

  app_log("BT mesh HSL set light to  %s\r\n", color[new_hsl]);
  app_log("Hue:      %4udeg\r\n", UINT16_TO_DEGREE(target_hue));
  app_log("Saturation: %4u%%\r\n", UINT16_TO_PERCENTAGE(target_saturation));
  app_log("Lightness:     %5u%%\r\n", UINT16_TO_PERCENTAGE(target_lightness));

  //printf(HSL_CLIENT_LOGGING_NEW_TEMP_SET, target_hue, target_saturation, target_lightness);

  // Request is sent multiple times to improve reliability
  hsl_request_count = HSL_CLIENT_RETRANSMISSION_COUNT;

  send_hsl_request(0);  //Send the first request

  // If there are more requests to send, start a repeating soft timer
  // to trigger retransmission of the request after 50 ms delay
  if (hsl_request_count > 0) {
    sl_status_t sc = sl_simple_timer_start(&hsl_retransmission_timer,
                                           HSL_CLIENT_RETRANSMISSION_TIMEOUT,
                                           hsl_retransmission_timer_cb,
                                           NO_CALLBACK_DATA,
                                           true);
    app_assert_status_f(sc, "Failed to start periodic timer\n");
  }
}

/***************************************************************************//**
 * Timer Callbacks
 * @param[in] handle pointer to handle instance
 * @param[in] data pointer to input data
 ******************************************************************************/
static void  hsl_retransmission_timer_cb(sl_simple_timer_t *handle,
                                         void *data)
{
  (void)data;
  (void)handle;

  send_hsl_request(1);   // Retransmit hsl message
  // Stop retransmission timer if it was the last attempt
  if (hsl_request_count == 0) {
    sl_status_t sc = sl_simple_timer_stop(&hsl_retransmission_timer);
    app_assert_status_f(sc, "Failed to stop periodic timer\n");
  }
}
/** @} (end addtogroup Lighting) */
