/***************************************************************************//**
<<<<<<< HEAD
 * @file  app.c
 * @brief Application code
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
=======
 * @file
 * @brief Application code
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
>>>>>>> main
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

<<<<<<< HEAD
#include <stdbool.h>
#include "em_common.h"
#include "sl_status.h"

#include "sl_btmesh.h"
#include "sl_bluetooth.h"
#include "app.h"

#include "gatt_db.h"

#include "sl_app_log.h"
#include "sl_app_assert.h"
#include "sl_simple_timer.h"

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
#define LOGE(...) sl_app_log(__VA_ARGS__)
#define LOGW(...) sl_app_log(__VA_ARGS__)
#define LOGI(...) sl_app_log(__VA_ARGS__)
#define LOGD(...) sl_app_log(__VA_ARGS__)
#define LOG_PLAIN(...) sl_app_log(__VA_ARGS__)
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
static sl_simple_timer_t app_led_blinking_timer;

// periodic timer callback
static void app_led_blinking_timer_cb(sl_simple_timer_t *handle, void *data);

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
  sc_snb_state = sl_btmesh_test_set_local_config(
                    sl_btmesh_node_beacon,
                    0,
                    1,
                    &snb_state);

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
  sl_app_log("BT mesh Light initialized\r\n");
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
=======
/* C Standard Library headers */
#include <stdio.h>

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"
#include "mesh_generic_model_capi_types.h"
#include "mesh_lib.h"

/* GPIO peripheral library */
#include <em_gpio.h>
#include "em_core.h"
#include <gpiointerrupt.h>

/* Lightbulb with mesh models */
#include "lightbulb.h"

/* Display Interface header */
#include "display_interface.h"

/* LED driver with support for PWM dimming */
#include "led_driver.h"

/* Own header */
#include "app.h"

/*
 * The logging module will be used if below is defined, to use printf, comment it out
 */
#define USE_LOG_MODULE

#ifdef USE_LOG_MODULE
#undef SUB_MODULE_NAME
#define SUB_MODULE_NAME "ivr"
#include "log.h"
#else
#define INIT_LOG()
#define LOGD(...) printf(__VA_ARGS__)
#define LOGI(...) printf(__VA_ARGS__)
#define LOGW(...) printf(__VA_ARGS__)
#define SE_CALL(x)  (x)
#define UINT8_ARRAY_DUMP(array_base, array_size)                                    \
  do {                                                                              \
    for (int i_log_exlusive = 0; i_log_exlusive < (array_size); i_log_exlusive++) { \
      printf((i_log_exlusive + 1) % 16 ? "%02X " : "%02X\n",                        \
             ((char*)(array_base))[i_log_exlusive]); }                              \
    printf("\n");                                                                   \
  } while (0)
#define ERROR_ADDRESSING()                                         \
  do {                                                             \
    printf("  |--> File - %s, Line - %d\r\n", __FILE__, __LINE__); \
  } while (0)
#endif

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/
/******************************************************************
 * Added by Kevin
 * ***************************************************************/
CORE_DECLARE_IRQ_STATE;
#define ASSERT(x) if (!(x)) { CORE_ENTER_ATOMIC(); while (1); }

#define EXT_SIGNAL_PB0_SHORT_PRESS      0x10
#define EXT_SIGNAL_PB1_SHORT_PRESS      0x40
#define IV_RECOVERY_MODE  0
#define SNB_STATE 1
#define IV_TEST_MODE  1

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

/*******************************************************************************
 * Timer handles defines.
 ******************************************************************************/
#define TIMER_ID_RESTART    78
#define TIMER_ID_FACTORY_RESET  77
#define TIMER_ID_PROVISIONING   66

/// Flag for indicating DFU Reset must be performed
static uint8_t boot_to_dfu = 0;
/// Address of the Primary Element of the Node
static uint16 _my_address = 0;
/// Number of active Bluetooth connections
static uint8 num_connections = 0;
/// Handle of the last opened LE connection
static uint8 conn_handle = 0xFF;
/// Flag for indicating that initialization was performed
static uint8 init_done = 0;

/***************************************************************************//**
 * This function is called to initiate factory reset. Factory reset may be
 * initiated by keeping one of the WSTK pushbuttons pressed during reboot.
 * Factory reset is also performed if it is requested by the provisioner
 * (event gecko_evt_mesh_node_reset_id).
 ******************************************************************************/
static void initiate_factory_reset(void)
{
  LOGD("factory reset\r\n");
  DI_Print("\n***\nFACTORY RESET\n***", DI_ROW_STATUS);

  /* if connection is open then close it before rebooting */
  if (conn_handle != 0xFF) {
    gecko_cmd_le_connection_close(conn_handle);
  }

  /* perform a factory reset by erasing PS storage. This removes all the keys and other settings
     that have been configured for this node */
  gecko_cmd_flash_ps_erase_all();
  // reboot after a small delay
  gecko_cmd_hardware_set_soft_timer(2 * 32768, TIMER_ID_FACTORY_RESET, 1);
>>>>>>> main
}

/***************************************************************************//**
 * Set device name in the GATT database. A unique name is generated using
 * the two last bytes from the Bluetooth address of this device. Name is also
<<<<<<< HEAD
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

  sl_app_log("Device name: '%s'\r\n", name);

  result = sl_bt_gatt_server_write_attribute_value(gattdb_device_name,
                                                   0,
                                                   strlen(name),
                                                   (uint8_t *)name);
  if (result) {
    sl_app_log("sl_bt_gatt_server_write_attribute_value failed, code %x\r\n",
               result);
  }
  // Show device name on the LCD
  lcd_print(name, BTMESH_WSTK_LCD_ROW_NAME);
}

/***************************************************************************//**
 * Handles button press and does a factory reset
 *
 * @return true if there is no button press
 ******************************************************************************/
bool handle_reset_conditions()
{
  // If either PB0 or PB1 is held down then do factory reset
  if ((sl_simple_button_get_state(sl_button_btn0.context)
       == SL_SIMPLE_BUTTON_PRESSED)
      || (sl_simple_button_get_state(sl_button_btn1.context)
          == SL_SIMPLE_BUTTON_PRESSED) ) {
    // Factory reset
    sl_btmesh_initiate_factory_reset();
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
    sl_app_assert(sc == SL_STATUS_OK,
                  "[E: 0x%04x] Failed to get Bluetooth address\n",
                  (int)sc);
    set_device_name(&address);
    // Initialize Mesh stack in Node operation mode, wait for initialized event
    sc = sl_btmesh_node_init();
    if (sc) {
      snprintf(buf, BOOT_ERR_MSG_BUF_LEN, "init failed (0x%x)", sc);
      lcd_print(buf, BTMESH_WSTK_LCD_ROW_STATUS);
      sl_app_log("Initialization failed (0x%x)\r\n", sc);
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
      lcd_print("connected", BTMESH_WSTK_LCD_ROW_CONNECTION);
      sl_app_log("Connected\r\n");
      break;

    case sl_bt_evt_connection_closed_id:
      if (num_connections > 0) {
        if (--num_connections == 0) {
          lcd_print("", BTMESH_WSTK_LCD_ROW_CONNECTION);
          sl_app_log("Disconnected\r\n");
        }
      }
      break;

    default:
      break;
  }
}

/***************************************************************************//**
=======
 * displayed on the LCD.
 *
 * @param[in] pAddr  Pointer to Bluetooth address.
 ******************************************************************************/
static void set_device_name(bd_addr *pAddr)
{
  char name[20];
  uint16 res;

  // create unique device name using the last two bytes of the Bluetooth address
  sprintf(name, "light node %02x:%02x", pAddr->addr[1], pAddr->addr[0]);

  LOGD("Device name: '%s'\r\n", name);

  // write device name to the GATT database
  res = gecko_cmd_gatt_server_write_attribute_value(gattdb_device_name, 0, strlen(name), (uint8 *)name)->result;
  if (res) {
    LOGD("gecko_cmd_gatt_server_write_attribute_value() failed, code %x\r\n", res);
  }

  // show device name on the LCD
  DI_Print(name, DI_ROW_NAME);
}

/***************************************************************************//**
 * This function prints debug information for mesh server state change event.
 *
 * @param[in] pEvt  Pointer to mesh_lib_generic_server_state_changed event.
 ******************************************************************************/
/* static void server_state_changed(struct gecko_msg_mesh_generic_server_state_changed_evt_t *pEvt) */
/* { */
/* int i; */
/*  */
/* LOGD("state changed: "); */
/* LOGD("model ID %4.4x, type %2.2x ", pEvt->model_id, pEvt->type); */
/* for (i = 0; i < pEvt->parameters.len; i++) { */
/* LOGD("%2.2x ", pEvt->parameters.data[i]); */
/* } */
/* LOGD("\r\n"); */
/* } */

static void button_init(void)
{
  // configure pushbutton PB0 and PB1 as inputs, with pull-up enabled
  GPIO_PinModeSet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN, gpioModeInputPull, 1);
  GPIO_PinModeSet(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN, gpioModeInputPull, 1);
}

void gpioint(uint8_t pin)
{
  if (pin == BSP_BUTTON0_PIN) {
    if (GPIO_PinInGet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN) == 0) {
    } else {
      gecko_external_signal(EXT_SIGNAL_PB0_SHORT_PRESS);
    }
  } else if (pin == BSP_BUTTON1_PIN) {
    if (GPIO_PinInGet(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN) == 0) {
    } else {
      gecko_external_signal(EXT_SIGNAL_PB1_SHORT_PRESS);
    }
  }
}
void enable_button_interrupts(void)
{
  GPIOINT_Init();

  /* configure interrupt for PB0 and PB1, both falling and rising edges */
  GPIO_ExtIntConfig(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN, BSP_BUTTON0_PIN, true, true, true);
  GPIO_ExtIntConfig(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN, BSP_BUTTON1_PIN, true, true, true);

  /* register the callback function that is invoked when interrupt occurs */
  GPIOINT_CallbackRegister(BSP_BUTTON0_PIN, gpioint);
  GPIOINT_CallbackRegister(BSP_BUTTON1_PIN, gpioint);
}
/*******************************************************************************
 * Initialise used bgapi classes.
 ******************************************************************************/
void gecko_bgapi_classes_init(void)
{
  gecko_bgapi_class_dfu_init();
  gecko_bgapi_class_system_init();
  gecko_bgapi_class_le_gap_init();
  gecko_bgapi_class_le_connection_init();
  //gecko_bgapi_class_gatt_init();
  gecko_bgapi_class_gatt_server_init();
  gecko_bgapi_class_hardware_init();
  gecko_bgapi_class_flash_init();
  gecko_bgapi_class_test_init();
  //gecko_bgapi_class_sm_init();
  //mesh_native_bgapi_init();
  gecko_bgapi_class_mesh_node_init();
  //gecko_bgapi_class_mesh_prov_init();
  gecko_bgapi_class_mesh_proxy_init();
  gecko_bgapi_class_mesh_proxy_server_init();
  //gecko_bgapi_class_mesh_proxy_client_init();
  //gecko_bgapi_class_mesh_generic_client_init();
  gecko_bgapi_class_mesh_generic_server_init();
  //gecko_bgapi_class_mesh_vendor_model_init();
  //gecko_bgapi_class_mesh_health_client_init();
  //gecko_bgapi_class_mesh_health_server_init();
  gecko_bgapi_class_mesh_test_init();
  //gecko_bgapi_class_mesh_lpn_init();
  gecko_bgapi_class_mesh_friend_init();

  INIT_LOG();

  button_init();
  enable_button_interrupts();
}

static void iv_config(uint8_t iv_test_mode,
                      uint8_t iv_recovery_mode,
                      uint8_t snb_state)
{
  LOGI("%s IV test mode %s\n",
       iv_test_mode ? "Enable" : "Disable",
       gecko_cmd_mesh_test_set_ivupdate_test_mode(iv_test_mode)->result
       ? "FAILED" : "SUCCESS");

  LOGI("Set IV Recovery Mode %s %s\n",
       iv_recovery_mode ? "Enable" : "Disable",
       gecko_cmd_mesh_node_set_ivrecovery_mode(iv_recovery_mode)->result
       ? "FAILED" : "SUCCESS");

  LOGI("%s SNB %s\n",
       snb_state ? "Enable" : "Disable",
       gecko_cmd_mesh_test_set_local_config(
         mesh_node_beacon,
         0,
         1,
         &snb_state)->result ? "FAILED" : "SUCCESS");
}
/*******************************************************************************
>>>>>>> main
 * Handling of stack events. Both Bluetooth LE and Bluetooth mesh events
 * are handled here.
 * @param[in] evt_id  Incoming event ID.
 * @param[in] evt     Pointer to incoming event.
 ******************************************************************************/
<<<<<<< HEAD
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

        sl_app_assert(sc == SL_STATUS_OK,
                      "[E: 0x%04x] Failed to start unprovisioned beaconing\n",
                      (int)sc);
      }
      break;

#ifdef IV_RECOVERY_IMPLEMENTATION
    case sl_btmesh_evt_node_changed_ivupdate_state_id:
      LOGI("Current IV - %d, Ongoing - %s\r\n",
           evt->data.evt_node_changed_ivupdate_state.iv_index,
           evt->data.evt_node_changed_ivupdate_state.state ? "YES" : "NO");
      break;

    case sl_btmesh_evt_node_ivrecovery_needed_id:
      LOGI("IV recovery is needed. Current IV = %d, Received(Network) IV = %d\r\n",
           evt->data.evt_node_ivrecovery_needed.node_iv_index,
           evt->data.evt_node_ivrecovery_needed.network_iv_index);
      sl_status_t sc = sl_btmesh_node_set_ivrecovery_mode(1);
      LOGI("Set IVR Enable %s\r\n", sc == SL_STATUS_OK ? "SUCCESS" : "FAILED");
      break;
#endif

    case sl_btmesh_evt_node_reset_id:
      sl_btmesh_initiate_factory_reset();
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

  sl_status_t sc = sl_simple_timer_start(&app_led_blinking_timer,
                                         APP_LED_BLINKING_TIMEOUT,
                                         app_led_blinking_timer_cb,
                                         NO_CALLBACK_DATA,
                                         true);

  sl_app_assert(sc == SL_STATUS_OK,
                "[E: 0x%04x] Failed to start periodic timer\n",
                (int)sc);

  app_show_btmesh_node_provisioning_started(result);
}

// Called when the Provisioning finishes successfully
void sl_btmesh_on_node_provisioned(uint16_t address,
                                   uint32_t iv_index)
{
  sl_status_t sc = sl_simple_timer_stop(&app_led_blinking_timer);
  sl_app_assert(sc == SL_STATUS_OK,
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
=======
void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt)
{
  uint16_t result;
  char buf[30];

  if (NULL == evt) {
    return;
  }

  switch (evt_id) {
    case gecko_evt_mesh_node_changed_ivupdate_state_id:
      LOGI("Current IV - %d, Ongoing - %s\n",
           evt->data.evt_mesh_node_changed_ivupdate_state.ivindex,
           evt->data.evt_mesh_node_changed_ivupdate_state.state ? "YES" : "NO");
      break;

    case gecko_evt_system_boot_id:
      // check pushbutton state at startup. If either PB0 or PB1 is held down then do factory reset
      if (GPIO_PinInGet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN) == 0 || GPIO_PinInGet(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN) == 0) {
        initiate_factory_reset();
      } else {
        struct gecko_msg_system_get_bt_address_rsp_t *pAddr = gecko_cmd_system_get_bt_address();

        set_device_name(&pAddr->address);

        // Initialize Mesh stack in Node operation mode, it will generate initialized event
        result = gecko_cmd_mesh_node_init()->result;
        if (result) {
          sprintf(buf, "init failed (0x%x)", result);
          DI_Print(buf, DI_ROW_STATUS);
        }

        // re-initialize LEDs (needed for those radio board that share same GPIO for button/LED)
        LEDS_init();
      }
      break;

    case gecko_evt_hardware_soft_timer_id:
      switch (evt->data.evt_hardware_soft_timer.handle) {
        case TIMER_ID_FACTORY_RESET:
          // reset the device to finish factory reset
          gecko_cmd_system_reset(0);
          break;

        case TIMER_ID_RESTART:
          // restart timer expires, reset the device
          gecko_cmd_system_reset(0);
          break;

        case TIMER_ID_PROVISIONING:
          // toggle LED to indicate the provisioning state
          if (!init_done) {
            LEDS_SetState(LED_STATE_PROV);
          }
          break;

        default:
          // lightbulb related timer events are handled by separate function
          handle_lightbulb_timer_evt(evt);
          break;
      }
      break;

    case gecko_evt_mesh_node_ivrecovery_needed_id:
      LOGI("IV recovery is needed. Current IV = %d, Received(Network) IV = %d\n",
           evt->data.evt_mesh_node_ivrecovery_needed.node_ivindex,
           evt->data.evt_mesh_node_ivrecovery_needed.network_ivindex);

      /* LOGI("Set IVR Enable %s\n", */
      /* gecko_cmd_mesh_node_set_ivrecovery_mode(1)->result ? "FAILED" : "SUCCESS"); */
      break;

    case gecko_evt_mesh_node_initialized_id:
      LOGD("node initialized\r\n");

      // Initialize generic server models
      gecko_cmd_mesh_generic_server_init();

      iv_config(IV_TEST_MODE, IV_RECOVERY_MODE, SNB_STATE);

      struct gecko_msg_mesh_node_initialized_evt_t *pData = (struct gecko_msg_mesh_node_initialized_evt_t *)&(evt->data);

      if (pData->provisioned) {
        LOGD("node is provisioned. address:%x, ivi:%ld\r\n", pData->address, pData->ivi);

        _my_address = pData->address;
        lightbulb_state_init();
        init_done = 1;

        DI_Print("provisioned", DI_ROW_STATUS);
      } else {
        LOGD("node is unprovisioned\r\n");
        DI_Print("unprovisioned", DI_ROW_STATUS);

        LOGD("starting unprovisioned beaconing...\r\n");
        gecko_cmd_mesh_node_start_unprov_beaconing(0x3);   // enable ADV and GATT provisioning bearer
      }
      break;

    case gecko_evt_mesh_node_provisioning_started_id:
      LOGD("Started provisioning\r\n");
      DI_Print("provisioning...", DI_ROW_STATUS);
      // start timer for blinking LEDs to indicate which node is being provisioned
      gecko_cmd_hardware_set_soft_timer(32768 / 4, TIMER_ID_PROVISIONING, 0);
      break;

    case gecko_evt_mesh_node_provisioned_id:
      lightbulb_state_init();
      init_done = 1;
      LOGD("node provisioned, got address=%x\r\n", evt->data.evt_mesh_node_provisioned.address);
      // stop LED blinking when provisioning complete
      gecko_cmd_hardware_set_soft_timer(0, TIMER_ID_PROVISIONING, 0);
      DI_Print("provisioned", DI_ROW_STATUS);
      break;

    case gecko_evt_mesh_node_provisioning_failed_id:
      LOGD("provisioning failed, code %x\r\n", evt->data.evt_mesh_node_provisioning_failed.result);
      DI_Print("prov failed", DI_ROW_STATUS);
      /* start a one-shot timer that will trigger soft reset after small delay */
      gecko_cmd_hardware_set_soft_timer(2 * 32768, TIMER_ID_RESTART, 1);
      break;

    case gecko_evt_mesh_node_key_added_id:
      LOGD("got new %s key with index %x\r\n", evt->data.evt_mesh_node_key_added.type == 0 ? "network" : "application",
           evt->data.evt_mesh_node_key_added.index);
      break;

    case gecko_evt_mesh_node_model_config_changed_id:
      LOGD("model config changed\r\n");
      break;

    case gecko_evt_mesh_generic_server_client_request_id:
      LOGD("evt gecko_evt_mesh_generic_server_client_request_id\r\n");
      // pass the server client request event to mesh lib handler that will invoke
      // the callback functions registered by application
      mesh_lib_generic_server_event_handler(evt);
      break;

    case gecko_evt_mesh_generic_server_state_changed_id:

      // uncomment following line to get debug prints for each server state changed event
      //server_state_changed(&(evt->data.evt_mesh_generic_server_state_changed));

      // pass the server state changed event to mesh lib handler that will invoke
      // the callback functions registered by application
      mesh_lib_generic_server_event_handler(evt);
      break;

    case gecko_evt_mesh_node_reset_id:
      LOGD("evt gecko_evt_mesh_node_reset_id\r\n");
      initiate_factory_reset();
      break;

    case gecko_evt_mesh_friend_friendship_established_id:
      LOGD("evt gecko_evt_mesh_friend_friendship_established, lpn_address=%x\r\n", evt->data.evt_mesh_friend_friendship_established.lpn_address);
      DI_Print("FRIEND", DI_ROW_FRIEND);
      break;

    case gecko_evt_mesh_friend_friendship_terminated_id:
      LOGD("evt gecko_evt_mesh_friend_friendship_terminated, reason=%x\r\n", evt->data.evt_mesh_friend_friendship_terminated.reason);
      DI_Print("NO LPN", DI_ROW_FRIEND);
      break;

    case gecko_evt_le_gap_adv_timeout_id:
      // adv timeout events silently discarded
      break;

    case gecko_evt_le_connection_opened_id:
      LOGD("evt:gecko_evt_le_connection_opened_id\r\n");
      num_connections++;
      conn_handle = evt->data.evt_le_connection_opened.connection;
      DI_Print("connected", DI_ROW_CONNECTION);
      break;

    case gecko_evt_le_connection_parameters_id:
      LOGD("evt:gecko_evt_le_connection_parameters_id\r\n");
      break;

    case gecko_evt_le_connection_closed_id:
      /* Check if need to boot to dfu mode */
      if (boot_to_dfu) {
        /* Enter to DFU OTA mode */
        gecko_cmd_system_reset(2);
      }

      LOGD("evt:conn closed, reason 0x%x\r\n", evt->data.evt_le_connection_closed.reason);
      conn_handle = 0xFF;
      if (num_connections > 0) {
        if (--num_connections == 0) {
          DI_Print("", DI_ROW_CONNECTION);
        }
      }
      break;
    case gecko_evt_gatt_server_user_write_request_id:
      if (evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_ota_control) {
        /* Set flag to enter to OTA mode */
        boot_to_dfu = 1;
        /* Send response to Write Request */
        gecko_cmd_gatt_server_send_user_write_response(
          evt->data.evt_gatt_server_user_write_request.connection,
          gattdb_ota_control,
          bg_err_success);

        /* Close connection to enter to DFU OTA mode */
        gecko_cmd_le_connection_close(evt->data.evt_gatt_server_user_write_request.connection);
      }
      break;

    case gecko_evt_system_external_signal_id:
    {
      uint16_t current_level;
      int16_t current_delta;
      char tmp[21];
      uint8_t en = 0;

      if (init_done && (evt->data.evt_system_external_signal.extsignals & EXT_SIGNAL_LED_LEVEL_CHANGED)) {
        /* this signal from the LED PWM driver indicates that the level has changed,
         * we use it here to update the LCD status */
        current_level = LEDS_GetLevel();
        sprintf(tmp, "Lightness: %5u%%", (current_level * 100 + 99) / 65535);
        DI_Print(tmp, DI_ROW_LIGHTNESS);
      }

      if (init_done && (evt->data.evt_system_external_signal.extsignals & EXT_SIGNAL_LED_TEMPERATURE_CHANGED)) {
        /* this signal from the LED driver indicates that the temperature level has changed,
         * we use it here to update the LCD status */
        current_level = LEDS_GetTemperature();
        sprintf(tmp, "ColorTemp: %5uK", current_level);
        DI_Print(tmp, DI_ROW_TEMPERATURE);

        current_delta = LEDS_GetDeltaUV();
        sprintf(tmp, "Delta UV: %6d ", current_delta);
        DI_Print(tmp, DI_ROW_DELTAUV);
      }

      if (init_done
          && (evt->data.evt_system_external_signal.extsignals
              & EXT_SIGNAL_PB0_SHORT_PRESS)) {
        en = 0;
        LOGI("Set IV Recovery Mode %s %s\n",
             en ? "Enable" : "Disable",
             gecko_cmd_mesh_node_set_ivrecovery_mode(en)->result
             ? "FAILED" : "SUCCESS");
      }

      if (init_done
          && (evt->data.evt_system_external_signal.extsignals
              & EXT_SIGNAL_PB1_SHORT_PRESS)) {
        en = 1;
        LOGI("Set IV Recovery Mode %s %s\n",
             en ? "Enable" : "Disable",
             gecko_cmd_mesh_node_set_ivrecovery_mode(en)->result
             ? "FAILED" : "SUCCESS");
      }
    }
    break;

    default:
      //LOGD("unhandled evt: %8.8x class %2.2x method %2.2x\r\n", evt_id, (evt_id >> 16) & 0xFF, (evt_id >> 24) & 0xFF);
      break;
  }
}

/** @} (end addtogroup app) */
/** @} (end addtogroup Application) */
>>>>>>> main
