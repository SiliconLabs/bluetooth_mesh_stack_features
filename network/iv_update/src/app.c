/***************************************************************************//**
 * @file  app.c
 * @brief Application code
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
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

#include "btmesh_change.h"

/* Buttons and LEDs headers */
#include "app_button_press.h"
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"
#include "sl_simple_led.h"
#include "sl_simple_led_instances.h"

#ifdef SL_COMPONENT_CATALOG_PRESENT
#include "sl_component_catalog.h"
#endif // SL_COMPONENT_CATALOG_PRESENT

#ifdef SL_CATALOG_BTMESH_WSTK_LCD_PRESENT
#include "sl_btmesh_wstk_lcd.h"
#endif // SL_CATALOG_BTMESH_WSTK_LCD_PRESENT

/* Switch app headers */
#include "app_timer.h"
#include "sl_btmesh_factory_reset.h"
#include "sl_btmesh_lighting_client.h"
#include "sl_btmesh_ctl_client.h"
//#include "sl_btmesh_scene_client.h"
#include "sl_btmesh_provisioning_decorator.h"

#define IV_UPDATE_IMPLEMENTATION
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
/// Increase step of physical values (lightness, color temperature)
#define INCREASE                       10
/// Decrease step of physical values (lightness, color temperature)
#define DECREASE                       (-10)
/// Length of the display name buffer
#define NAME_BUF_LEN                   20
/// Length of boot error message buffer
#define BOOT_ERR_MSG_BUF_LEN           30
/// Used button indexes
#define BUTTON_PRESS_BUTTON_0          0
#define BUTTON_PRESS_BUTTON_1          1

#ifdef SL_CATALOG_BTMESH_WSTK_LCD_PRESENT
#define lcd_print(...) sl_btmesh_LCD_write(__VA_ARGS__)
#else
#define lcd_print(...)
#endif // SL_CATALOG_BTMESH_WSTK_LCD_PRESENT

/// periodic timer handle
static app_timer_t app_led_blinking_timer;

/// periodic timer callback
static void app_led_blinking_timer_cb(app_timer_t *handle, void *data);
// Handling of boot event
static void handle_boot_event(void);
// Handling of le connection events
static void handle_le_connection_events(sl_bt_msg_t *evt);
// Set device name in the GATT database
static void set_device_name(bd_addr *addr);

/*******************************************************************************
 * Global variables
 ******************************************************************************/
/// number of active Bluetooth connections
static uint8_t num_connections = 0;

static bool init_done = false;

#ifdef IV_UPDATE_IMPLEMENTATION

#ifndef MIN
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#endif

#define IDLE              0
#define ONGOING           1

#define IV_TEST_MODE      1
#define IV_RECOVERY_MODE  1
#define SNB_STATE         1

#define TEST_IV_HOP       30
#define MAX_IV_HOP        42

typedef enum {
  normal_update,
  test_update,
  test_end
} test_p_t;

static int iv_update_state = IDLE;
static uint32_t iv_index = 0;
static test_p_t iv_update_normal_state = test_end;
static test_p_t iv_update_hop_state = test_end;

static void print_seqnum(void)
{
  sl_status_t sc;
  uint32_t seqnum = 0, count = 0;

  sc = sl_btmesh_node_get_element_seqnum(0, &seqnum);
  if (sc == SL_STATUS_OK) {
    LOGI("Test Get SEQ  - 0x%08lx\r\n", seqnum);
  } else {
    LOGE("Get SEQ ERR - 0x%04lx\r\n", sc);
  }

  sc = sl_btmesh_node_get_seq_remaining(0, &count);
  if (sc == SL_STATUS_OK) {
    LOGI("Remaining SEQ - 0x%08lx\r\n", count);
  } else {
    LOGE("Get Remaining SEQ ERR - 0x%04lx\r\n", sc);
  }
}

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

static void test_iv_update_recovery(test_p_t test_mode)
{
  sl_status_t sc;
  switch (test_mode) {
    case normal_update:
      if (iv_update_state == ONGOING) {
        return;
      }
      sc = sl_btmesh_node_request_ivupdate();
      if (sc == SL_STATUS_OK) {
        iv_update_state = ONGOING;
        LOGI("Request IV Update SUCCESS\r\n");
      } else {
        LOGE("Request IV Update FAILED - 0x%04lx\r\n", sc);
      }
      break;
    case test_update:
      if (iv_update_state == ONGOING) {
        return;
      }
      sc = sl_btmesh_test_set_iv_index(iv_index + MIN(TEST_IV_HOP, MAX_IV_HOP - 1));
      if (sc == SL_STATUS_OK) {
        iv_index += MIN(TEST_IV_HOP, MAX_IV_HOP - 1);
        LOGI("IV index has been set to %ld\r\n", iv_index);
      } else {
        LOGE("IV index set error - 0x%04lx\r\n", sc);
      }
      sc = sl_btmesh_test_set_ivupdate_state(1);
      if (sc == SL_STATUS_OK) {
        iv_update_state = ONGOING;
        LOGI("Force Set IV Update To Ongoing %ld\r\n", iv_index);
      } else {
        LOGE("Force Set IV Update To Ongoing Error - 0x%04lx\r\n", sc);
      }
      break;
    case test_end:
      if (iv_update_state == IDLE) {
        return;
      }
      sc = sl_btmesh_test_set_ivupdate_state(0);
      if (sc == SL_STATUS_OK) {
        iv_update_state = IDLE;
        LOGI("Force Ending IV Update SUCCESS\r\n");
      } else {
        LOGE("Force Ending IV Update FAILED - 0x%04lx\r\n", sc);
      }
      break;
    default:
      break;
  }
}

static void iv_update_normal(void)
{
  iv_update_normal_state = (iv_update_normal_state == normal_update) ? test_end : normal_update;
  test_iv_update_recovery(iv_update_normal_state);
}

static void iv_update_hop(void)
{
  iv_update_hop_state = (iv_update_hop_state == test_update) ? test_end : test_update;
  test_iv_update_recovery(iv_update_hop_state);
}

#endif

/***************************************************************************//**
 * Change buttons to LEDs in case of shared pin
 *
 ******************************************************************************/
void change_buttons_to_leds(void)
{
  app_button_press_disable();
  // Disable button and enable led
  sl_simple_button_disable(&sl_button_btn0);
  sl_led_init(&sl_led_led0);
  // Disable button and enable led
#ifndef SINGLE_BUTTON
  sl_simple_button_disable(&sl_button_btn1);
#endif // SINGLE_BUTTON
#ifndef SINGLE_LED
  sl_led_init(&sl_led_led1);
#endif // SINGLE_LED
}

/***************************************************************************//**
 * Change LEDs to buttons in case of shared pin
 *
 ******************************************************************************/
void change_leds_to_buttons(void)
{
  // Enable buttons
  sl_simple_button_enable(&sl_button_btn0);
#ifndef SINGLE_BUTTON
  sl_simple_button_enable(&sl_button_btn1);
#endif // SINGLE_BUTTON
  // Wait
  sl_sleeptimer_delay_millisecond(1);
  // Enable button presses
  app_button_press_enable();
}

/***************************************************************************//**
 * Application Init.
 ******************************************************************************/
SL_WEAK void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
  app_log("BT mesh Switch initialized\r\n");
  // Ensure right init order in case of shared pin for enabling buttons
  change_buttons_to_leds();
  // Change LEDs to buttons in case of shared pin
  change_leds_to_buttons();
#ifdef USE_LOGGING_SYSTEM
  INIT_LOG(0xFF);
#endif
}

/***************************************************************************//**
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
 * displayed on the LCD.
 *
 * @param[in] addr  Pointer to Bluetooth address.
 ******************************************************************************/
static void set_device_name(bd_addr *addr)
{
  char name[NAME_BUF_LEN];
  sl_status_t result;

  // Create unique device name using the last two bytes of the Bluetooth address
  snprintf(name, NAME_BUF_LEN, "switch node %02x:%02x",
           addr->addr[1], addr->addr[0]);

  app_log("Device name: '%s'\r\n", name);

  result = sl_bt_gatt_server_write_attribute_value(gattdb_device_name,
                                                   0,
                                                   strlen(name),
                                                   (uint8_t *)name);
  if (result) {
    app_log("sl_bt_gatt_server_write_attribute_value() failed, code %lx\r\n",
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
#ifdef SINGLE_BUTTON
  // If PB0 is held down then do factory reset
  if (sl_simple_button_get_state(&sl_button_btn0)
      == SL_SIMPLE_BUTTON_PRESSED) {
#else
  // If either PB0 or PB1 is held down then do factory reset
  if ((sl_simple_button_get_state(&sl_button_btn0)
       == SL_SIMPLE_BUTTON_PRESSED)
      || (sl_simple_button_get_state(&sl_button_btn1)
          == SL_SIMPLE_BUTTON_PRESSED) ) {
#endif // SL_CATALOG_BTN1_PRESENT
    // Disable button presses
    app_button_press_disable();
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
  char buf[BOOT_ERR_MSG_BUF_LEN];
  // Check reset conditions and continue if not reset.
  if (handle_reset_conditions()) {
    sc = sl_bt_system_get_identity_address(&address, &address_type);
    app_assert(sc == SL_STATUS_OK,
               "[E: 0x%04x] Failed to get Bluetooth address\n",
               (int)sc);
    set_device_name(&address);
    // Initialize Mesh stack in Node operation mode, wait for initialized event
    sc = sl_btmesh_node_init();
    if (sc) {
      snprintf(buf, BOOT_ERR_MSG_BUF_LEN, "init failed (0x%lx)", sc);
      lcd_print(buf, SL_BTMESH_WSTK_LCD_ROW_STATUS_CFG_VAL);
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
 * @param[in] evt    Pointer to incoming event.
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
#ifdef IV_UPDATE_IMPLEMENTATION
      iv_config(IV_TEST_MODE, IV_RECOVERY_MODE, SNB_STATE);
#endif
      if (!(evt->data.evt_node_initialized.provisioned)) {
        // Enable ADV and GATT provisioning bearer
        sc = sl_btmesh_node_start_unprov_beaconing(PB_ADV | PB_GATT);

        app_assert(sc == SL_STATUS_OK,
                   "[E: 0x%04x] Failed to start unprovisioned beaconing\n",
                   (int)sc);
      }
#ifdef IV_UPDATE_IMPLEMENTATION
      else {
        print_seqnum();
      }
#endif
      break;

#ifdef IV_UPDATE_IMPLEMENTATION
    case sl_btmesh_evt_node_changed_ivupdate_state_id:
      LOGI("Current IV - %ld, Ongoing - %s\r\n",
           evt->data.evt_node_changed_ivupdate_state.iv_index,
           evt->data.evt_node_changed_ivupdate_state.state ? "YES" : "NO");
      iv_index = evt->data.evt_node_changed_ivupdate_state.iv_index;
      iv_update_state = evt->data.evt_node_changed_ivupdate_state.state;
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

/*******************************************************************************
 * Callbacks
 ******************************************************************************/

/***************************************************************************//**
 * Button press Callbacks
 ******************************************************************************/
void app_button_press_cb(uint8_t button, uint8_t duration)
{
  // Selecting action by duration
  switch (duration) {
    case APP_BUTTON_PRESS_DURATION_SHORT:
      // Handling of button press less than 0.25s
      if (button == BUTTON_PRESS_BUTTON_0) {
        sl_btmesh_change_lightness(DECREASE);
      } else {
        sl_btmesh_change_lightness(INCREASE);
      }
      break;
    case APP_BUTTON_PRESS_DURATION_MEDIUM:
      // Handling of button press greater than 0.25s and less than 1s
      if (button == BUTTON_PRESS_BUTTON_0) {
        sl_btmesh_change_temperature(DECREASE);
      } else {
        sl_btmesh_change_temperature(INCREASE);
      }
      break;
    case APP_BUTTON_PRESS_DURATION_LONG:
      // Handling of button press greater than 1s and less than 5s
#ifdef SINGLE_BUTTON
      sl_btmesh_change_switch_position(SL_BTMESH_LIGHTING_CLIENT_TOGGLE);
#else
      if (button == BUTTON_PRESS_BUTTON_0) {
        sl_btmesh_change_switch_position(SL_BTMESH_LIGHTING_CLIENT_OFF);
      } else {
        sl_btmesh_change_switch_position(SL_BTMESH_LIGHTING_CLIENT_ON);
      }
#endif
      break;
    case APP_BUTTON_PRESS_DURATION_VERYLONG:
      // Handling of button press greater than 5s
#ifndef IV_UPDATE_IMPLEMENTATION
      if (button == BUTTON_PRESS_BUTTON_0) {
        sl_btmesh_select_scene(1);
      } else {
        sl_btmesh_select_scene(2);
      }
#else //IV_UPDATE_IMPLEMENTATION
      if (button == BUTTON_PRESS_BUTTON_0) {
        iv_update_hop();
      } else {
        iv_update_normal();
      }
      print_seqnum();
#endif
      break;
    default:
      break;
  }
}

/*******************************************************************************
 * Provisioning Decorator Callbacks
 ******************************************************************************/
// Called when the Provisioning starts
void sl_btmesh_on_node_provisioning_started(uint16_t result)
{
  // Change buttons to LEDs in case of shared pin
  change_buttons_to_leds();

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
  sl_led_turn_off(&sl_led_led0);
#ifndef SINGLE_LED
  sl_led_turn_off(&sl_led_led1);
#endif // SINGLE_LED
  change_leds_to_buttons();

  app_show_btmesh_node_provisioned(address, iv_index);

#ifdef IV_UPDATE_IMPLEMENTATION
  print_seqnum();
#endif
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
    sl_led_toggle(&sl_led_led0);
#ifndef SINGLE_LED
    sl_led_toggle(&sl_led_led1);
#endif // SINGLE_LED
  }
}
