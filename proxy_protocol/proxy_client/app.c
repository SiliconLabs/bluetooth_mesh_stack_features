/***************************************************************************//**
 * @file
 * @brief Application code
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

/* Lightbulb with mesh models */
#include "lightbulb.h"

/* Display Interface header */
#include "display_interface.h"

/* LED driver with support for PWM dimming */
#include "led_driver.h"

/* Own header */
#include "app.h"

#define USE_LOG_MODULE

#ifdef USE_LOG_MODULE
#undef SUB_MODULE_NAME
#define SUB_MODULE_NAME "proxy_c"
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

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/
/******************************************************************
 * Added by Kevin
 * ***************************************************************/
CORE_DECLARE_IRQ_STATE;
#define ASSERT(x) if (!(x)) { CORE_ENTER_ATOMIC(); while (1); }
/*
 * ADV bearer should be disabled to save power
 */
#define ADV_BEARER_STATE  0

#define USE_FAKE_NETWORK_ID 1

/*
 * Default connection parameters
 */
/* Interval (ms) = value * 1.25 */
#define MIN_CONN_INTERVAL 100
#define MAX_CONN_INTERVAL 200
#define CONN_LATENCY  0
/* Timeout (ms) = value * 10 */
#define CONN_SPV_TIMEOUT  200
/* Use default value for the timing of connection events */
#define MIN_CE_LENGTH 0
#define MAX_CE_LENGTH 0xffff

/* Interval (ms) = value * 0.625 */
#define SCAN_INTERVAL 100
#define SCAN_WINDOW 100

#define PASSIVE_SCAN  0
#define ACTIVE_SCAN 1
#define SCAN_TYPE PASSIVE_SCAN

#define PROXY_FILTER_WHITELIST  0
#define PROXY_FILTER_BLACKLIST  1

#define PROXY_SERVICE_UUID  "\x28\x18"
#define ADV_TYPE_16BIT_SERVICE  0x16

#define TIMER_ID_DELAY_TURN_OFF_ADV_BEARER  62
#define DELAY_TURN_OFF_ADV_BEARER_IN_TICKS (4 * 32768)
#define TIMER_ID_DELAY_PROXY_CONN 63
#define DELAY_PROXY_CONN_IN_TICKS (1 * 32768)

#if (USE_FAKE_NETWORK_ID == 1)
static const uint8_t dummy_network_id[8] = "\x54\x3b\xeb\x54\x36\x5a\xa1\x0d";
#else
static const uint8_t target_server_node_bd_addr[6] = "\x00\x0b\x57\xef\x01\x3d";
#endif

static uint32_t proxy_handle = 0xffff;

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
}

/***************************************************************************//**
 * Set device name in the GATT database. A unique name is generated using
 * the two last bytes from the Bluetooth address of this device. Name is also
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
#if 0
static void server_state_changed(struct gecko_msg_mesh_generic_server_state_changed_evt_t *pEvt)
{
  int i;

  LOGD("state changed: ");
  LOGD("model ID %4.4x, type %2.2x ", pEvt->model_id, pEvt->type);
  for (i = 0; i < pEvt->parameters.len; i++) {
    LOGD("%2.2x ", pEvt->parameters.data[i]);
  }
  LOGD("\r\n");
}
#endif

/*******************************************************************************
 * Initialise used bgapi classes.
 ******************************************************************************/
void gecko_bgapi_classes_init(void)
{
  gecko_bgapi_class_dfu_init();
  gecko_bgapi_class_system_init();
  gecko_bgapi_class_le_gap_init();
  gecko_bgapi_class_le_connection_init();
  gecko_bgapi_class_gatt_init();
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
  gecko_bgapi_class_mesh_proxy_client_init();
  //gecko_bgapi_class_mesh_generic_client_init();
  gecko_bgapi_class_mesh_generic_server_init();
  //gecko_bgapi_class_mesh_vendor_model_init();
  //gecko_bgapi_class_mesh_health_client_init();
  //gecko_bgapi_class_mesh_health_server_init();
  gecko_bgapi_class_mesh_test_init();
  //gecko_bgapi_class_mesh_lpn_init();
  /* gecko_bgapi_class_mesh_friend_init(); */
}

static void mesh_proxy_client_setting(void)
{
  struct gecko_msg_mesh_test_get_local_config_rsp_t *rsp = NULL;

  /* Configure the PB-ADV bearer state */
  SE_CALL(gecko_cmd_mesh_test_set_adv_bearer_state(ADV_BEARER_STATE));
  LOGD("ADV Bearer State - %s\n",
       ADV_BEARER_STATE ? "ON" : "OFF");

  /* Make sure gatt proxy feature is ON */
  rsp = gecko_cmd_mesh_test_get_local_config(mesh_node_gatt_proxy, 0);
  ASSERT(rsp->result == bg_err_success);

  if (!rsp->data.data[0]) {
    uint8_t en = 1;
    SE_CALL(gecko_cmd_mesh_test_set_local_config(
              mesh_node_gatt_proxy,
              0,
              1,
              &en));
    LOGD("Proxy Enabled.\n");
  }
}

static void start_scan(void)
{
  /* Set filter to pass the connectable adv packets to application layer */
  SE_CALL(gecko_cmd_mesh_node_set_adv_event_filter(0x01, 0, NULL));

  SE_CALL(gecko_cmd_le_gap_end_procedure());
  /* Set the default timing for all connctions to be established later on */
  SE_CALL(gecko_cmd_le_gap_set_conn_timing_parameters(
            MIN_CONN_INTERVAL,
            MAX_CONN_INTERVAL,
            CONN_LATENCY,
            CONN_SPV_TIMEOUT,
            MIN_CE_LENGTH,
            MAX_CE_LENGTH));

  /* Set up discovery, including timing, type, then start discovery */
  SE_CALL(gecko_cmd_le_gap_set_discovery_timing(
            le_gap_phy_1m,
            SCAN_INTERVAL,
            SCAN_WINDOW));
  SE_CALL(gecko_cmd_le_gap_set_discovery_type(
            le_gap_phy_1m,
            SCAN_TYPE));
  SE_CALL(gecko_cmd_le_gap_start_discovery(
            le_gap_phy_1m,
            le_gap_discover_generic));
  LOGD("Scan Enabled.\n");
}

/**
 * @brief match - Since there isn't any API for parsing the proxy adv payload,
 * this function fake the match by any means.
 *
 * @param data
 * @param network_id
 * @param unicast_addr
 *
 * @return
 */
static uint8_t match(const struct gecko_msg_le_gap_scan_response_evt_t *e,
                     const uint8_t *network_id,
                     uint16_t unicast_addr)
{
  size_t pos = 0, tmp = 0;
  uint8_t adv_len, adv_type;
  const uint8_t *uuid = (const uint8_t *)PROXY_SERVICE_UUID;

  while (pos < e->data.len) {
    adv_len = e->data.data[pos];
    adv_type = e->data.data[pos + 1];
    tmp = pos + 2;
    pos += 1 + adv_len;
    if (adv_type != ADV_TYPE_16BIT_SERVICE) {
      continue;
    }
    if (adv_len == 12
        && !memcmp(&e->data.data[tmp], uuid, 2)
        && e->data.data[tmp + 2] == 0x00) {
      /* Network ID */
      return !memcmp(&e->data.data[tmp + 3], network_id, 8);
    }

    if (adv_len == 20
        && !memcmp(&e->data.data[tmp], uuid, 2)
        && e->data.data[tmp + 2] == 0x01) {
      /* Node identity - No api for parsing the payload, return 0 for now*/
      return 0;
    }
  }
  return 0;
}

static void on_adv_scanned(const struct gecko_msg_le_gap_scan_response_evt_t *e)
{
  ASSERT(e);

  /* Filter out unconnectable packets */
  if ((e->address_type & 0x7) != 0 && (e->address_type & 0x7) != 1) {
    return;
  }
#if (USE_FAKE_NETWORK_ID == 1)
  if (match(e, dummy_network_id, 0)) {
    SE_CALL(gecko_cmd_le_gap_end_procedure());
    SE_CALL(gecko_cmd_le_gap_connect(
              e->address,
              e->address_type,
              le_gap_phy_1m));
    LOGI("Matched, connecting...\n");
  }
#else
  if (!memcmp(e->address, target_server_node_bd_addr, 6)) {
    /* SE_CALL(gecko_cmd_le_gap_end_procedure()); */
    SE_CALL(gecko_cmd_le_gap_connect(
              e->address,
              e->address_type,
              le_gap_phy_1m));
    LOGI("Matched, connecting...\n");
  }
#endif
  LOGD("Network ID: \n\t");
  UINT8_ARRAY_DUMP(e->data.data, e->data.len);
}

static void initiate_proxy_connection(uint32_t delay_ticks)
{
  struct gecko_msg_mesh_proxy_connect_rsp_t *rsp = NULL;
  if (conn_handle == 0xFF) {
    return;
  }

  if (delay_ticks == 0) {
    rsp = gecko_cmd_mesh_proxy_connect(conn_handle);
    DI_Print("Proxy connecting", DI_ROW_FRIEND);
    if (rsp->result) {
      ERROR_ADDRESSING();
    } else {
      /* Duplicated with event ??? */
      proxy_handle = rsp->handle;
      LOGI("Proxy handle = %ld\n", proxy_handle);
    }
  } else {
    SE_CALL(gecko_cmd_hardware_set_soft_timer(
              delay_ticks,
              TIMER_ID_DELAY_PROXY_CONN,
              1));
  }
}

static void demo_set_whitelist(uint8_t count)
{
  LOGD("set up whitelist\n");
  switch (count) {
    case 0:
      SE_CALL(gecko_cmd_mesh_proxy_allow(
                proxy_handle,
                0xC021,
                0));
      break;
    case 1:
      SE_CALL(gecko_cmd_mesh_proxy_allow(
                proxy_handle,
                0xC02F,
                0));
      break;
    default:
      break;
  }
}

static void demo_set_blacklist(uint8_t count)
{
  LOGD("set up blacklist\n");
}

static void proxy_configuration(uint8_t type,
                                uint8_t count)
{
  type ? demo_set_blacklist(count) : demo_set_whitelist(count);
}

static void proxy_set_filter_type(uint8_t filter_type)
{
  /* Below example to use whitelist */
  SE_CALL(gecko_cmd_mesh_proxy_set_filter_type(
            proxy_handle,
            !!filter_type,
            0));
  LOGD("Proxy filter configured.\n");
}

/*******************************************************************************
 * Handling of stack events. Both Bluetooth LE and Bluetooth mesh events
 * are handled here.
 * @param[in] evt_id  Incoming event ID.
 * @param[in] evt     Pointer to incoming event.
 ******************************************************************************/
void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt)
{
  uint16_t result;
  char buf[30];

  if (NULL == evt) {
    return;
  }

  switch (evt_id) {
    case gecko_evt_system_boot_id:
      // check pushbutton state at startup. If either PB0 or PB1 is held down then do factory reset
      if (GPIO_PinInGet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN) == 0 || GPIO_PinInGet(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN) == 0) {
        initiate_factory_reset();
      } else {
        INIT_LOG();
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

        case TIMER_ID_DELAY_TURN_OFF_ADV_BEARER:
          mesh_proxy_client_setting();
          start_scan();
          break;

        case TIMER_ID_DELAY_PROXY_CONN:
          initiate_proxy_connection(0);
          break;

        default:
          // lightbulb related timer events are handled by separate function
          handle_lightbulb_timer_evt(evt);
          break;
      }
      break;

    case gecko_evt_mesh_node_initialized_id:
      LOGD("node initialized\r\n");

      // Initialize generic server models
      gecko_cmd_mesh_generic_server_init();

      struct gecko_msg_mesh_node_initialized_evt_t *pData = (struct gecko_msg_mesh_node_initialized_evt_t *)&(evt->data);

      if (pData->provisioned) {
        LOGD("node is provisioned. address:%x, ivi:%ld\r\n", pData->address, pData->ivi);

        _my_address = pData->address;
        lightbulb_state_init();
        init_done = 1;

        gecko_cmd_hardware_set_soft_timer(
          DELAY_TURN_OFF_ADV_BEARER_IN_TICKS,
          TIMER_ID_DELAY_TURN_OFF_ADV_BEARER,
          1);

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

      gecko_cmd_hardware_set_soft_timer(
        DELAY_TURN_OFF_ADV_BEARER_IN_TICKS,
        TIMER_ID_DELAY_TURN_OFF_ADV_BEARER,
        1);
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
      gecko_cmd_hardware_set_soft_timer(
        DELAY_TURN_OFF_ADV_BEARER_IN_TICKS,
        TIMER_ID_DELAY_TURN_OFF_ADV_BEARER,
        1);
      break;

    case gecko_evt_mesh_generic_server_client_request_id:
    {
      LOGD("evt gecko_evt_mesh_generic_server_client_request_id\r\n");
      // pass the server client request event to mesh lib handler that will invoke
      // the callback functions registered by application
      mesh_lib_generic_server_event_handler(evt);

      static uint32_t num = 1;
      char buf[30];
      memset(buf, 0, 30);
      sprintf(buf, "Req %lu", num++);
      DI_Print(buf, DI_ROW_FRIEND);
    }
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

    case gecko_evt_le_gap_adv_timeout_id:
      // adv timeout events silently discarded
      break;

    case gecko_evt_le_connection_opened_id:
      LOGD("evt:gecko_evt_le_connection_opened_id\r\n");
      num_connections++;
      conn_handle = evt->data.evt_le_connection_opened.connection;
      DI_Print("connected", DI_ROW_CONNECTION);
      initiate_proxy_connection(DELAY_PROXY_CONN_IN_TICKS);
      break;

    case gecko_evt_le_connection_parameters_id:
      LOGD("evt:gecko_evt_le_connection_parameters_id\r\n");
      break;

    case gecko_evt_le_connection_closed_id:
    {
      /* Check if need to boot to dfu mode */
      if (boot_to_dfu) {
        /* Enter to DFU OTA mode */
        gecko_cmd_system_reset(2);
      }

      LOGW("evt:conn closed, reason 0x%x\r\n", evt->data.evt_le_connection_closed.reason);

      conn_handle = 0xFF;
      SE_CALL(gecko_cmd_hardware_set_soft_timer(
                0,
                TIMER_ID_DELAY_PROXY_CONN,
                1));
      if (num_connections > 0) {
        if (--num_connections == 0) {
          memset(buf, 0, 30);
          sprintf(buf, "Reason = %x", evt->data.evt_le_connection_closed.reason);
          DI_Print(buf, DI_ROW_CONNECTION);
          start_scan();
        }
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
    }
    break;

    case gecko_evt_le_gap_scan_response_id:
    case gecko_evt_le_gap_extended_scan_response_id:
      if (num_connections == 0) {
        on_adv_scanned(&evt->data.evt_le_gap_scan_response);
      }
      break;

    case gecko_evt_mesh_proxy_connected_id:
      DI_Print("Proxy connected", DI_ROW_FRIEND);
      LOGI("Proxy connection (handle = 0x%lx) established.\n",
           evt->data.evt_mesh_proxy_connected.handle);
      proxy_handle = evt->data.evt_mesh_proxy_connected.handle;
      proxy_set_filter_type(PROXY_FILTER_BLACKLIST);

      /* SE_CALL(gecko_cmd_le_gap_end_procedure()); */
      break;

    case gecko_evt_mesh_proxy_disconnected_id:
      DI_Print("Proxy disconnected", DI_ROW_FRIEND);
      LOGW("Proxy connection (handle = 0x%lx) closed. Reason = 0x%04x\n",
           evt->data.evt_mesh_proxy_disconnected.handle,
           evt->data.evt_mesh_proxy_disconnected.reason);
      proxy_handle = 0xffff;
      break;

    case gecko_evt_mesh_proxy_filter_status_id:
      LOGD("Filter length = %d\n",
           evt->data.evt_mesh_proxy_filter_status.count);
      proxy_configuration(evt->data.evt_mesh_proxy_filter_status.type,
                          evt->data.evt_mesh_proxy_filter_status.count);
      if (evt->data.evt_mesh_proxy_filter_status.count == 2) {
        DI_Print("Proxy configured", DI_ROW_FRIEND);
      }
      break;
    default:
      //LOGD("unhandled evt: %8.8x class %2.2x method %2.2x\r\n", evt_id, (evt_id >> 16) & 0xFF, (evt_id >> 24) & 0xFF);
      break;
  }
}

/** @} (end addtogroup app) */
/** @} (end addtogroup Application) */
