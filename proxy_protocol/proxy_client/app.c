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
 ******************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include "em_common.h"
#include "sl_status.h"

#include "sl_btmesh.h"
#include "sl_bluetooth.h"
#include "app.h"
#include "app_assert.h"
#include "app_log.h"

#include "gatt_db.h"

#include "sl_simple_timer.h"

/* Buttons and LEDs headers */
#include "sl_simple_button_instances.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#ifdef SL_CATALOG_BTMESH_WSTK_LCD_PRESENT
#include "sl_btmesh_wstk_lcd.h"
#endif // SL_CATALOG_BTMESH_WSTK_LCD_PRESENT

/* Light app headers */
#include "sl_btmesh_ctl_server_config.h"
#include "sl_btmesh_lighting_server_config.h"
#include "sl_btmesh_lc_server_config.h"
#include "sl_btmesh_factory_reset.h"

#include "app_led.h"
#include "sl_btmesh_provisioning_decorator.h"

#ifdef SL_CATALOG_BTMESH_WSTK_LCD_PRESENT
#define lcd_print(...) sl_btmesh_LCD_write(__VA_ARGS__)
#else
#define lcd_print(...)
#endif // SL_CATALOG_BTMESH_WSTK_LCD_PRESENT

#define UINT8_ARRAY_DUMP(array_base, array_size)                                    \
  do {                                                                              \
    for (int i_log_exlusive = 0; i_log_exlusive < (array_size); i_log_exlusive++) { \
      app_log((i_log_exlusive + 1) % 16 ? "%02X " : "%02X\r\n",                        \
             ((char*)(array_base))[i_log_exlusive]); }                              \
    app_log("\r\n");                                                                   \
  } while (0)

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

/*
 * ADV bearer should be disabled to save power
 */
#define ADV_BEARER_STATE               0

/*
 * Default connection parameters
 */

/* Interval (ms) = value * 0.625 */
#define SCAN_INTERVAL                  100
#define SCAN_WINDOW                    100

#define PASSIVE_SCAN                   0
#define ACTIVE_SCAN                    1
#define SCAN_TYPE                      PASSIVE_SCAN

#define PROXY_FILTER_WHITELIST         0
#define PROXY_FILTER_BLACKLIST         1

#define TIMER_ID_DELAY_TURN_OFF_ADV_BEARER  62
#define DELAY_TURN_OFF_ADV_BEARER_IN_TICKS  (4 * 32768)
#define TIMER_ID_DELAY_PROXY_CONN           63
#define DELAY_PROXY_CONN_IN_TICKS           (1 * 32768)

static const bd_addr target_server_node_bd_addr = {{0xa1, 0x92, 0xc5, 0xf9, 0xe3, 0xb4}};

/*******************************************************************************
 * Timer handles defines.
 ******************************************************************************/
#define TIMER_ID_RESTART               78

sl_sleeptimer_timer_handle_t sleep_timer_handle_delay_proxy_conn_1;
sl_sleeptimer_timer_handle_t sleep_timer_handle_delay_proxy_conn_2;
sl_sleeptimer_timer_handle_t sleep_timer_handle_delay_turn_off_adv_bearer_1;
sl_sleeptimer_timer_handle_t sleep_timer_handle_delay_turn_off_adv_bearer_2;
sl_sleeptimer_timer_handle_t sleep_timer_handle_delay_turn_off_adv_bearer_3;
sl_sleeptimer_timer_handle_t sleep_timer_restart_1;

void sleeptimer_callback(sl_sleeptimer_timer_handle_t *handle, void *data);

// periodic timer handle
static sl_simple_timer_t app_led_blinking_timer;

// periodic timer callback
static void app_led_blinking_timer_cb(sl_simple_timer_t *handle, void *data);



/*******************************************************************************
 * Global variables
 ******************************************************************************/

static uint32_t proxy_handle = 0xFFFF;
/// Flag for indicating DFU Reset must be performed
static uint8_t boot_to_dfu = 0;
/// Address of the Primary Element of the Node
static uint16_t my_address = 0;
/// Handle of the last opened LE connection
static uint8_t conn_handle = 0xFF;
/// Number of active Bluetooth connections
static uint8_t num_connections = 0;
/// Init flag
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
 * the two last bytes from the UUID of this device. Name is also
 * displayed on the LCD if present.
 *
 * @param[in] uuid  Pointer to device UUID.
 ******************************************************************************/
static void set_device_name(uuid_128 *uuid)
{
  char name[NAME_BUF_LEN];
  sl_status_t result;

  // Create unique device name using the last two bytes of the device UUID
  snprintf(name, NAME_BUF_LEN, "proxy node %02x%02x",
           uuid->data[14], uuid->data[15]);

  app_log("Device name: '%s'\r\n", name);

  result = sl_bt_gatt_server_write_attribute_value(gattdb_device_name,
                                                   0,
                                                   strlen(name),
                                                   (uint8_t *)name);
  if (result) {
    app_log("sl_bt_gatt_server_write_attribute_value failed, code %x\r\n",
            result);
  }
  // Show device name on the LCD
  lcd_print(name, SL_BTMESH_WSTK_LCD_ROW_NAME_CFG_VAL);
}

/***************************************************************************//**
 * Handles button press and does a factory reset
 *
 * @return true if there is no button press
 ******************************************************************************/
bool handle_reset_conditions(void)
{
  // If PB0 is held down then do full factory reset
  if (sl_simple_button_get_state(&sl_button_btn0)
      == SL_SIMPLE_BUTTON_PRESSED) {
    // Full factory reset
    sl_btmesh_initiate_full_reset();
    return false;
  }

#ifndef SINGLE_BUTTON
  // If PB1 is held down then do node factory reset
  if (sl_simple_button_get_state(&sl_button_btn1)
      == SL_SIMPLE_BUTTON_PRESSED) {
    // Node factory reset
    sl_btmesh_initiate_node_reset();
    return false;
  }
#endif // SL_CATALOG_BTN1_PRESENT
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
  char buf[BOOT_ERR_MSG_BUF_LEN];
  uuid_128 uuid;
  // Check reset conditions and continue if not reset.
  if (handle_reset_conditions()) {
    // Initialize Mesh stack in Node operation mode, wait for initialized event
    sc = sl_btmesh_node_init();
    if (sc) {
      snprintf(buf, BOOT_ERR_MSG_BUF_LEN, "init failed (0x%lx)", sc);
      lcd_print(buf, SL_BTMESH_WSTK_LCD_ROW_STATUS_CFG_VAL);
      app_log("Initialization failed (0x%x)\r\n", sc);
    } else {
      sc = sl_btmesh_node_get_uuid(&uuid);
      app_assert_status_f(sc, "Failed to get UUID\r\n");
      set_device_name(&uuid);
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
      app_log_debug("Connected\r\n");
      break;

    case sl_bt_evt_connection_closed_id:
      if (num_connections > 0) {
        if (--num_connections == 0) {
          lcd_print("", SL_BTMESH_WSTK_LCD_ROW_CONNECTION_CFG_VAL);
          app_log_debug("Disconnected\r\n");
        }
      }
      break;

    default:
      break;
  }
}

static void on_adv_scanned(sl_bt_evt_scanner_scan_report_t *e)
{
  /* Filter out unconnectable packets */
  if ((e->address_type & 0x7) != 0 && (e->address_type & 0x7) != 1) {
    return;
  }

  if (!memcmp(e->address.addr, target_server_node_bd_addr.addr, 6)) {
    UINT8_ARRAY_DUMP(e->address.addr, 6);
    sl_bt_connection_open(
      e->address,
      e->address_type,
      sl_bt_gap_phy_1m,
      &conn_handle);
    app_log_info("Matched, connecting...\r\n");
  }

  app_log_debug("Network ID: \n\t");
  UINT8_ARRAY_DUMP(e->data.data, e->data.len);
}

static void initiate_proxy_connection(uint32_t delay_ticks)
{
  sl_status_t sc;
  if (conn_handle == 0xFF) {
    return;
  }

  if (delay_ticks == 0) {
    sc = sl_btmesh_proxy_connect(conn_handle, &proxy_handle);
    app_log_info("Proxy handle = %ld\r\n", proxy_handle);
    lcd_print("Proxy connecting", SL_BTMESH_WSTK_LCD_ROW_FRIEND_CFG_VAL);
    if (sc != SL_STATUS_OK) {
      app_log_info("sc = %ld\r\n", sc);
    } else {
      app_log_info("Proxy handle = %ld\r\n", proxy_handle);
    }
  } else {
    sl_sleeptimer_start_timer(
      &sleep_timer_handle_delay_proxy_conn_1,
      delay_ticks,
      sleeptimer_callback,
      (void*)TIMER_ID_DELAY_PROXY_CONN,
      0,
      0);
  }
}

static void demo_set_whitelist(uint8_t count)
{
  app_log_debug("Set up whitelist\r\n");
  switch (count) {
    case 0:
      sl_btmesh_proxy_allow(
                proxy_handle,
                0,
                0xC021);
      break;
    case 1:
      sl_btmesh_proxy_allow(
                proxy_handle,
                0,
                0xC02F);
      break;
    default:
      break;
  }
}

static void demo_set_blacklist(uint8_t count)
{
  app_log_debug("Set up blacklist\r\n");
  switch (count) {
    case 0:
      sl_btmesh_proxy_deny(
                proxy_handle,
                0,
                0xC021);
      break;
    case 1:
      sl_btmesh_proxy_deny(
                proxy_handle,
                0,
                0xC02F);
      break;
    default:
      break;
  }
}

static void proxy_configuration(uint8_t type,
                                uint8_t count)
{
  type ? demo_set_blacklist(count) : demo_set_whitelist(count);
}

static void proxy_set_filter_type(uint8_t filter_type)
{
  /* Below example to use whitelist */
  sl_btmesh_proxy_set_filter_type(
            proxy_handle,
            0,
            filter_type);
  app_log_debug("Proxy filter configured.\r\n");
}

static void mesh_proxy_client_setting(void)
{
  sl_status_t sc;

  /* Configure the PB-ADV bearer state */
  sl_btmesh_test_set_adv_bearer_state(ADV_BEARER_STATE);
  app_log("ADV Bearer State - %s\r\n",
       ADV_BEARER_STATE ? "ON" : "OFF");

  /* Make sure gatt proxy feature is ON */
  uint8_t en = 1;
  sc = sl_btmesh_test_get_gatt_proxy(&en);
  app_assert_status_f(sc, "Failed to get GATT proxy state\r\n");
  app_log_info("en = %d\r\n", en);

  if (!en) {
    uint8_t value = 1;
    sc = sl_btmesh_test_set_gatt_proxy(value, &en);
    app_assert_status_f(sc, "Failed to set GATT proxy state\r\n");
    app_log("Proxy Enabled.\r\n");
  }
}

static void start_scan(void)
{
  sl_bt_scanner_stop();

  /* Set up discovery, including timing, type, then start discovery */
  sl_bt_scanner_set_parameters(
            SCAN_TYPE,
            SCAN_INTERVAL,
            SCAN_WINDOW);

  sl_bt_scanner_start(
            sl_bt_gap_phy_1m,
            sl_bt_scanner_discover_generic);
  app_log_debug("Scan Enabled.\r\n");
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
    case sl_bt_evt_connection_opened_id:
      app_log("evt:sl_bt_evt_connection_opened_id\r\n");
      num_connections++;
      app_log_info("conn_handle = %ld\r\n", conn_handle);
      conn_handle = evt->data.evt_connection_opened.connection;
      lcd_print("connected", SL_BTMESH_WSTK_LCD_ROW_CONNECTION_CFG_VAL);
      initiate_proxy_connection(DELAY_PROXY_CONN_IN_TICKS);
      break;

    case sl_bt_evt_connection_parameters_id:
      app_log("evt:sl_bt_evt_connection_parameters_id\r\n");
      break;

    case sl_bt_evt_gatt_server_user_write_request_id:
      if (evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_ota_control) {
        /* Set flag to enter to OTA mode */
        boot_to_dfu = 1;
        /* Send response to Write Request */
        sl_bt_gatt_server_send_user_write_response(
          evt->data.evt_gatt_server_user_write_request.connection,
          gattdb_ota_control,
          0);

        /* Close connection to enter to DFU OTA mode */
        sl_bt_connection_close(evt->data.evt_gatt_server_user_write_request.connection);
      }
      break;

    case sl_bt_evt_connection_closed_id:
     {
       handle_le_connection_events(evt);
       /* Check if need to boot to dfu mode */
       if (boot_to_dfu) {
         /* Enter to DFU OTA mode */
         sl_bt_system_reset(2);
       }

       app_log_warning("evt:sl_bt_evt_connection_closed_id, reason 0x%x\r\n",
                       evt->data.evt_connection_closed.reason);

       conn_handle = 0xFF;
       sl_sleeptimer_start_timer(
         &sleep_timer_handle_delay_proxy_conn_2,
         0,
         sleeptimer_callback,
         (void*)TIMER_ID_DELAY_PROXY_CONN,
         0,
         0);

       if (num_connections > 0) {
         if (--num_connections == 0) {
           char buf[30];
           memset(buf, 0, 30);
           sprintf(buf, "Reason = %x", evt->data.evt_connection_closed.reason);
           lcd_print(buf, SL_BTMESH_WSTK_LCD_ROW_CONNECTION_CFG_VAL);
           start_scan();
         }
       }
     }
     break;

    case sl_bt_evt_system_boot_id:
       handle_boot_event();
       break;

     case sl_bt_evt_system_external_signal_id:
     {
       switch (evt->data.evt_system_soft_timer.handle) {

         case TIMER_ID_RESTART:
           app_log("TIMER_ID_RESTART\r\n");
           // Restart timer expires, reset the device
           sl_bt_system_reset(0);
           break;

         case TIMER_ID_DELAY_TURN_OFF_ADV_BEARER:
           app_log("TIMER_ID_DELAY_TURN_OFF_ADV_BEARER\r\n");
           mesh_proxy_client_setting();
           start_scan();
           break;

         case TIMER_ID_DELAY_PROXY_CONN:
           app_log("TIMER_ID_DELAY_PROXY_CONN\r\n");
           initiate_proxy_connection(0);
           break;

         default:
           break;
       }
     }
     break;

     case sl_bt_evt_scanner_scan_report_id: /* Can be removed after node_init issues with legacy and extended scanner solved */
     case sl_bt_evt_scanner_legacy_advertisement_report_id:
     case sl_bt_evt_scanner_extended_advertisement_report_id:
       if (num_connections == 0) {
         on_adv_scanned(&evt->data.evt_scanner_scan_report);
       }
       break;

     default:
       app_log("Unhandled evt:%x\r\n", SL_BT_MSG_ID(evt->header));
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

        app_assert_status_f(sc, "Failed to start unprovisioned beaconing\r\n");
      }

      app_log_debug("Node initialized\r\n");

      // Initialize generic server models
      sl_btmesh_generic_server_init();

      if (evt->data.evt_node_initialized.provisioned) {
        app_log("Node is provisioned, address:%x, ivi:%ld\r\n",
                evt->data.evt_node_initialized.address,
                evt->data.evt_node_initialized.iv_index);

        my_address = evt->data.evt_node_initialized.address;
        init_done = true;

        sl_sleeptimer_start_timer(
          &sleep_timer_handle_delay_turn_off_adv_bearer_1,
          DELAY_TURN_OFF_ADV_BEARER_IN_TICKS,
          sleeptimer_callback,
          (void*)TIMER_ID_DELAY_TURN_OFF_ADV_BEARER,
          0,
          0);
      } else {
        app_log_debug("Starting unprovisioned beaconing...\r\n");
        sl_btmesh_node_start_unprov_beaconing(0x3);   // enable ADV and GATT provisioning bearer
      }
      break;

    case sl_btmesh_evt_node_provisioning_started_id:
      app_log_debug("evt:sl_btmesh_evt_node_provisioning_started_id\r\n");
      break;

    case sl_btmesh_evt_node_provisioned_id:
      init_done = true;

      sl_sleeptimer_start_timer(
        &sleep_timer_handle_delay_turn_off_adv_bearer_2,
        DELAY_TURN_OFF_ADV_BEARER_IN_TICKS,
        sleeptimer_callback,
        (void*)TIMER_ID_DELAY_TURN_OFF_ADV_BEARER,
        0,
        0);
      break;

    case sl_btmesh_evt_node_provisioning_failed_id:
      /* Start a one-shot timer that will trigger soft reset after small delay */
      sl_sleeptimer_start_timer(
        &sleep_timer_restart_1,
        2 * 32768,
        sleeptimer_callback,
        (void*)TIMER_ID_RESTART,
        0,
        0);
      break;

    case sl_btmesh_evt_node_key_added_id:
      app_log("Got new %s key with index %x\r\n",
              evt->data.evt_node_key_added.type == 0 ? "network" : "application",
              evt->data.evt_node_key_added.index);
      break;

    case sl_btmesh_evt_node_model_config_changed_id:
      app_log_debug("Model config changed\r\n");

      sl_sleeptimer_start_timer(
        &sleep_timer_handle_delay_turn_off_adv_bearer_3,
        DELAY_TURN_OFF_ADV_BEARER_IN_TICKS,
        sleeptimer_callback,
        (void*)TIMER_ID_DELAY_TURN_OFF_ADV_BEARER,
        0,
        0);
      break;

    case sl_btmesh_evt_generic_server_client_request_id:
    {
      app_log_debug("evt:sl_btmesh_evt_generic_server_client_request_id\r\n");

      static uint32_t requests = 1;
      char buf[30];
      memset(buf, 0, 30);
      sprintf(buf, "Req %lu", requests++);
      lcd_print(buf, SL_BTMESH_WSTK_LCD_ROW_FRIEND_CFG_VAL);
    }
    break;

    case sl_btmesh_evt_generic_server_state_changed_id:
      app_log_debug("evt:sl_btmesh_evt_generic_server_state_changed_id\r\n");
      break;

    case sl_btmesh_evt_node_reset_id:
      app_log_debug("evt:sl_btmesh_evt_node_reset_id\r\n");
      sl_btmesh_initiate_full_reset();
      break;

    case sl_btmesh_evt_proxy_connected_id:
     lcd_print("Proxy connected", SL_BTMESH_WSTK_LCD_ROW_FRIEND_CFG_VAL);
     app_log_info("Proxy connection (handle = 0x%lx) established.\r\n",
                  evt->data.evt_proxy_connected.handle);
     proxy_handle = evt->data.evt_proxy_connected.handle;
     proxy_set_filter_type(PROXY_FILTER_BLACKLIST);
     break;

   case sl_btmesh_evt_proxy_disconnected_id:
     lcd_print("Proxy disconnected", SL_BTMESH_WSTK_LCD_ROW_FRIEND_CFG_VAL);
     app_log_warning("Proxy connection (handle = 0x%lx) closed. Reason = 0x%04x\r\n",
                     evt->data.evt_proxy_disconnected.handle,
                     evt->data.evt_proxy_disconnected.reason);
     proxy_handle = 0xffff;
     break;

   case sl_btmesh_evt_proxy_filter_status_id:
     app_log("Filter length = %d\r\n",
          evt->data.evt_proxy_filter_status.count);
     proxy_configuration(evt->data.evt_proxy_filter_status.type,
                         evt->data.evt_proxy_filter_status.count);
     if (evt->data.evt_proxy_filter_status.count == 2) {
       lcd_print("Proxy configured", SL_BTMESH_WSTK_LCD_ROW_FRIEND_CFG_VAL);
     }
     break;

    default:
      break;
  }
}

/***************************************************************************//**
 * Provisioning Decorator Callbacks
 ******************************************************************************/
// Called when the Provisioning starts
void sl_btmesh_on_node_provisioning_started(uint16_t result)
{
  // Change buttons to LEDs in case of shared pin
  app_led_change_buttons_to_leds();

  sl_status_t sc = sl_simple_timer_start(
    &app_led_blinking_timer,
    APP_LED_BLINKING_TIMEOUT,
    app_led_blinking_timer_cb,
    NO_CALLBACK_DATA,
    true);

  app_assert_status_f(sc, "Failed to start periodic timer\r\n");

  app_show_btmesh_node_provisioning_started(result);
}

// Called when the Provisioning finishes successfully
void sl_btmesh_on_node_provisioned(uint16_t address,
                                   uint32_t iv_index)
{
  sl_status_t sc = sl_simple_timer_stop(&app_led_blinking_timer);
  app_assert_status_f(sc, "Failed to stop periodic timer\r\n");
  // Turn off LED
  init_done = true;
  app_led_set_level(LED_LEVEL_OFF);

  app_show_btmesh_node_provisioned(address, iv_index);
}

/***************************************************************************//**
 * Timer Callbacks
 ******************************************************************************/
static void app_led_blinking_timer_cb(sl_simple_timer_t *handle, void *data)
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

/***************************************************************************//**
 * Factory reset Callbacks
 ******************************************************************************/
void sl_btmesh_factory_reset_on_node_reset(void)
{
  app_show_btmesh_node_reset();
  sl_bt_nvm_erase(SL_BTMESH_LIGHTING_SERVER_PS_KEY_CFG_VAL);
  sl_bt_nvm_erase(SL_BTMESH_CTL_SERVER_PS_KEY_CFG_VAL);
  sl_bt_nvm_erase(SL_BTMESH_LC_SERVER_PS_KEY_CFG_VAL);
  sl_bt_nvm_erase(SL_BTMESH_LC_SERVER_PROPERTY_PS_KEY_CFG_VAL);
}

/***************************************************************************//**
 * Sleeptimer callback
 *
 * Note: This function is called from interrupt context
 *
 * @param[in] handle Handle of the sleeptimer instance
 * @param[in] data  Callback data
 ******************************************************************************/
void sleeptimer_callback(sl_sleeptimer_timer_handle_t *handle, void *data){
  (void)handle;
  (void)data;

  sl_bt_external_signal((uint32_t)data);
}
