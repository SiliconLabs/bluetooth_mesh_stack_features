/***************************************************************************//**
 * @file  app.c
 * @brief Application code
*******************************************************************************
* # License
* <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
*******************************************************************************
*
* SPDX-License-Identifier: Zlib
*
* The licensor of this software is Silicon Laboratories Inc.
*
* This software is provided \'as-is\', without any express or implied
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

#include <stdio.h>
#include <stdbool.h>
#include "em_common.h"
#include "sl_status.h"

#include "sl_btmesh.h"
#include "sl_bluetooth.h"
#include "app.h"

#include "gatt_db.h"

#include "app_log.h"
#include "app_assert.h"
#include "app_timer.h"

/* Buttons and LEDs headers */
#include "sl_simple_button_instances.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#ifdef SL_CATALOG_BTMESH_WSTK_LCD_PRESENT
#include "sl_btmesh_wstk_lcd.h"
#endif // SL_CATALOG_BTMESH_WSTK_LCD_PRESENT

/* Switch app headers */
#include "sl_btmesh_factory_reset.h"
#include "sl_btmesh_lighting_server.h"
#include "sl_btmesh_ctl_server.h"

#include "app_led.h"
#include "sl_btmesh_provisioning_decorator.h"

#ifdef SL_CATALOG_BTMESH_WSTK_LCD_PRESENT
#define lcd_print(...) sl_btmesh_LCD_write(__VA_ARGS__)
#else
#define lcd_print(...)
#endif // SL_CATALOG_BTMESH_WSTK_LCD_PRESENT

/// High Priority
#define HIGH_PRIORITY                  0
/// No Timer Options
#define NO_FLAGS                       0
/// Callback has no parameters
#define NO_CALLBACK_DATA               (void *)NULL
/// Timeout for Blinking LED during provisioning
#define APP_LED_BLINKING_TIMEOUT       250
/// Connection uninitialized
#define UNINITIALIZED_CONNECTION       0xFF
/// Advertising Provisioning Bearer
#define PB_ADV                         0x1
/// GATT Provisioning Bearer
#define PB_GATT                        0x2
/// LED switched off (lightness = 0)
#define LED_LEVEL_OFF                  0
/// Length of the display name buffer
#define NAME_BUF_LEN                   20
/// Length of boot error message buffer
#define BOOT_ERR_MSG_BUF_LEN           30
// periodic timer handle
static app_timer_t app_led_blinking_timer;

// periodic timer callback
static void app_led_blinking_timer_cb(app_timer_t *handle, void *data);
static void print_scan_resp(sl_bt_evt_scanner_legacy_advertisement_report_t *pResp);
/*******************************************************************************
 * Global variables
 ******************************************************************************/
/// number of active Bluetooth connections
static uint8_t num_connections = 0;

static bool init_done = false;

/*******************************************************************************
 * Application Init.
 ******************************************************************************/
SL_WEAK void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
  app_log("BT mesh Light initialized\r\n");
  app_led_init();
}

/*******************************************************************************
 * Application Process Action.
 ******************************************************************************/
SL_WEAK void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}

/***************************************************************************//**
 * Set device name in the GATT database. A unique name is generated using
 * the two last bytes from the Bluetooth address of this device. Name is also
 * displayed on the LCD if present.
 *
 * @param[in] addr  Pointer to Bluetooth address.
 ******************************************************************************/
static void set_device_name(bd_addr *addr)
{
  char name[NAME_BUF_LEN];
  sl_status_t result;

  // Create unique device name using the last two bytes of the Bluetooth address
  snprintf(name, NAME_BUF_LEN, "light node %02x:%02x",
           addr->addr[1], addr->addr[0]);

  app_log("Device name: '%s'\r\n", name);

  result = sl_bt_gatt_server_write_attribute_value(gattdb_device_name,
                                                   0,
                                                   strlen(name),
                                                   (uint8_t *)name);
  if (result) {
    app_log("sl_bt_gatt_server_write_attribute_value failed, code %lx\r\n",
            result);
  }
  // Show device name on the LCD
  lcd_print(name, SL_BTMESH_WSTK_LCD_ROW_NAME_CFG_VAL);
  lcd_print("Scanning Example", SL_BTMESH_WSTK_LCD_ROW_STATUS_CFG_VAL);
}

/***************************************************************************//**
 * Handles button press and does a factory reset
 *
 * @return true if there is no button press
 ******************************************************************************/
bool handle_reset_conditions()
{
  // If either PB0 or PB1 is held down then do factory reset
  if (sl_simple_button_get_state(&sl_button_btn0) == SL_SIMPLE_BUTTON_PRESSED) {
    // Factory reset
    sl_btmesh_initiate_full_reset();
    return false;
  }
  return true;
}

/***************************************************************************//**
 * Handling of boot event.
 * If needed it performs factory reset. In other case it sets device name
 * and initialize mesh node.
 ******************************************************************************/
static void handle_boot_event(void)
{
  sl_status_t sc;
  bd_addr address;
  uint8_t address_type;
  // Check reset conditions and continue if not reset.
  if (handle_reset_conditions()) {
    sc = sl_bt_system_get_identity_address(&address, &address_type);
    app_assert(sc == SL_STATUS_OK,
               "[E: 0x%04x] Failed to get Bluetooth address\n",
               (int)sc);
    set_device_name(&address);
    // Initialize Mesh stack in Node operation mode,
    // wait for initialized event
    app_log("Node init\r\n");
    sc = sl_btmesh_node_init();

    switch (sc) {
      case 0x00: break;
      case 0x02: app_log("Node already initialized\r\n"); break;
      default: app_assert_status_f(sc, "Failed to init node\r\n");
    }
  }
}

/***************************************************************************//**
 *  Handling of le connection events.
 *  It handles:
 *   - le_connection_opened
 *   - le_connection_parameters
 *   - le_connection_closed
 *
 *  @param[in] evt  Pointer to incoming connection event.
 ******************************************************************************/
static void handle_le_connection_events(sl_bt_msg_t *evt)
{
  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_connection_opened_id:
      num_connections++;
      lcd_print("connected", SL_BTMESH_WSTK_LCD_ROW_CONNECTION_CFG_VAL);
      app_log("Connected\r\n");
      break;

    case sl_bt_evt_connection_closed_id:
      if (num_connections > 0) {
        if (--num_connections == 0) {
          lcd_print("", SL_BTMESH_WSTK_LCD_ROW_CONNECTION_CFG_VAL);
          app_log("Disconnected\r\n");
        }
      }
      break;

    default:
      break;
  }
}

/***************************************************************************//**
 * Handling of stack events. Both Bluetooth LE and Bluetooth mesh events
 * are handled here.
 * @param[in] evt_id  Incoming event ID.
 * @param[in] evt     Pointer to incoming event.
 ******************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_system_boot_id:
      handle_boot_event();
      break;

    case sl_bt_evt_connection_opened_id:
    case sl_bt_evt_connection_parameters_id:
    case sl_bt_evt_connection_closed_id:
      handle_le_connection_events(evt);
      break;

    case sl_bt_evt_scanner_legacy_advertisement_report_id:
      print_scan_resp(&evt->data.evt_scanner_legacy_advertisement_report);
      break;
    default:
      break;
  }
}

/***************************************************************************//**
 * Bluetooth Mesh stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Pointer to incoming event from the Bluetooth Mesh stack.
 ******************************************************************************/
void sl_btmesh_on_event(sl_btmesh_msg_t *evt)
{
  sl_status_t sc;

  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_btmesh_evt_node_initialized_id:
      if (!(evt->data.evt_node_initialized.provisioned)) {
        // Enable ADV and GATT provisioning bearer
        sc = sl_btmesh_node_start_unprov_beaconing(PB_ADV | PB_GATT);

        switch (sc) {
          case 0x00: break;
          case 0x02: app_log("Unprovisioned beaconing already running\r\n"); break;
          default: app_assert_status_f(sc, "Failed to start unprovisioned beaconing\n");
        }
      }
      break;

    case sl_btmesh_evt_node_reset_id:
      sl_btmesh_initiate_full_reset();
      break;

    default:
      break;
  }
}

/***************************************************************************//**
 * Callbacks
 ******************************************************************************/

/***************************************************************************//**
 * Provisioning Decorator Callbacks
 ******************************************************************************/
// Called when the Provisioning starts
void sl_btmesh_on_node_provisioning_started(uint16_t result)
{
  // Change buttons to LEDs in case of shared pin
  app_led_change_buttons_to_leds();

  sl_status_t sc = app_timer_start(&app_led_blinking_timer,
                                   APP_LED_BLINKING_TIMEOUT,
                                   app_led_blinking_timer_cb,
                                   NO_CALLBACK_DATA,
                                   true);

  app_assert(sc == SL_STATUS_OK,
             "[E: 0x%04x] Failed to start periodic timer\n",
             (int)sc);

  app_show_btmesh_node_provisioning_started(result);
}

// Called when the Provisioning finishes successfully
void sl_btmesh_on_node_provisioned(uint16_t address,
                                   uint32_t iv_index)
{
  sl_status_t sc = app_timer_stop(&app_led_blinking_timer);
  app_assert(sc == SL_STATUS_OK,
             "[E: 0x%04x] Failed to stop periodic timer\n",
             (int)sc);
  // Turn off LED
  init_done = true;
  app_led_set_level(LED_LEVEL_OFF);

  app_show_btmesh_node_provisioned(address, iv_index);
}

/***************************************************************************//**
 * Timer Callbacks
 ******************************************************************************/
static void app_led_blinking_timer_cb(app_timer_t *handle, void *data)
{
  (void)data;
  (void)handle;
  if (!init_done) {
    // Toggle LEDs
    static uint16_t level = 0;
    level = app_led_get_max() - level;
    app_led_set_level(level);
  }
}

void sl_btmesh_lighting_level_pwm_cb(uint16_t level)
{
  app_led_set_level(level);
}

void sl_btmesh_lighting_color_pwm_cb(uint16_t color)
{
  app_led_set_color(color);
}

static void print_scan_resp(sl_bt_evt_scanner_legacy_advertisement_report_t *pResp)
{
  // decoding advertising packets is done here. The list of AD types can be found
  // at: https://www.bluetooth.com/specifications/assigned-numbers/Generic-Access-Profile

  // example of adv data including proxy service data:020106030328181416281801fd65f619452875558774bc
  // (UUID 0x1828)

  // 020106-03032818-1416281801fd65f619452875558774bc
  const uint8_t proxy_UUID[16] = {0x28, 0x18};

  int i = 0;
  int ad_match_found = 0;
  int ad_len;
  int ad_type;

  while (i < (pResp->data.len - 1))
  {
    ad_len  = pResp->data.data[i];
    ad_type = pResp->data.data[i+1];

    if (ad_type == 0x03)
    {
      // type 0x03= Complete List of 16-bit Service Class UUIDs

      if(memcmp(proxy_UUID, &(pResp->data.data[i+2]),2) == 0)
      {
        ad_match_found = 1;
      }
    }

    //jump to next AD record
    i = i + ad_len + 1;
  }

  if(ad_match_found)
  {
    for(i=5;i>=0;i--)
    {
      app_log("%2.2x", pResp->address.addr[i]);
    }
    app_log(", RSSI: %d\r\n", pResp->rssi);
  }
}