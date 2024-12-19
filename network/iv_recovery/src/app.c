/***************************************************************************//**
 * @file  app.c
 * @brief Application code
 *******************************************************************************
 * # License
 * <b>Copyright 2022 Silicon Laboratories Inc. www.silabs.com</b>
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

#define IV_RECOVERY_IMPLEMENTATION
//#define USE_LOGGING_SYSTEM

#ifdef USE_LOGGING_SYSTEM
#include "logging/logging.h"
#else
#define LOGE(...) app_log(__VA_ARGS__)
#define LOGW(...) app_log(__VA_ARGS__)
#define LOGI(...) app_log(__VA_ARGS__)
#define LOGD(...) app_log(__VA_ARGS__)
#define LOG_PLAIN(...) app_log(__VA_ARGS__)
#define SE_CALL(x) (x)
#endif

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

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/// number of active Bluetooth connections
static uint8_t num_connections = 0;

static bool init_done = false;

#ifdef IV_RECOVERY_IMPLEMENTATION

#define IV_TEST_MODE      1
#define IV_RECOVERY_MODE  0
#define SNB_STATE         1

static void iv_config(uint8_t iv_test_mode,
                      uint8_t iv_recovery_mode,
                      uint8_t snb_state)
{
  sl_status_t sc_test_mode, sc_recovery_mode, sc_snb_state;

  sc_test_mode = sl_btmesh_test_set_ivupdate_test_mode(iv_test_mode);
  sc_recovery_mode = sl_btmesh_node_set_ivrecovery_mode(iv_recovery_mode);
  sc_snb_state = sl_btmesh_test_set_beacon(1, &snb_state);

  LOGI("%s IV test mode %s\r\n",
       iv_test_mode ? "Enable" : "Disable",
       sc_test_mode == SL_STATUS_OK ? "SUCCESS" : "FAILED");

  LOGI("Set IV Recovery Mode %s %s\r\n",
       iv_recovery_mode ? "Enable" : "Disable",
       sc_recovery_mode == SL_STATUS_OK ? "SUCCESS" : "FAILED");

  LOGI("%s SNB %s\r\n",
       snb_state ? "Enable" : "Disable",
       sc_snb_state == SL_STATUS_OK ? "SUCCESS" : "FAILED");
}

#endif

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
#ifdef USE_LOGGING_SYSTEM
  INIT_LOG(0xFF);
#endif
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
  bd_addr address;
  uint8_t address_type;
  // Check reset conditions and continue if not reset.
  if (handle_reset_conditions()) {
    sc = sl_bt_system_get_identity_address(&address, &address_type);
    app_assert(sc == SL_STATUS_OK,
               "[E: 0x%04x] Failed to get Bluetooth address\n",
               (int)sc);
    set_device_name(&address);
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
#ifdef IV_RECOVERY_IMPLEMENTATION
      iv_config(IV_TEST_MODE, IV_RECOVERY_MODE, SNB_STATE);
#endif
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

#ifdef IV_RECOVERY_IMPLEMENTATION
    case sl_btmesh_evt_node_changed_ivupdate_state_id:
      LOGI("Current IV - %ld, Ongoing - %s\r\n",
           evt->data.evt_node_changed_ivupdate_state.iv_index,
           evt->data.evt_node_changed_ivupdate_state.state ? "YES" : "NO");
      break;

    case sl_btmesh_evt_node_ivrecovery_needed_id:
      LOGI("IV recovery is needed. Current IV = %ld, Received(Network) IV = %ld\r\n",
           evt->data.evt_node_ivrecovery_needed.node_iv_index,
           evt->data.evt_node_ivrecovery_needed.network_iv_index);
      break;
#endif

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
