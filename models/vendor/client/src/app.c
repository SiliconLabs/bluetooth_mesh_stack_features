/***************************************************************************//**
 * @file app.c
 * @brief Core application logic for the vendor client node.
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
 ******************************************************************************
 * # Experimental Quality
 * This code has not been formally tested and is provided as-is. It is not
 * suitable for production environments. In addition, this code will not be
 * maintained and there may be no bug maintenance planned for these resources.
 * Silicon Labs may update projects from time to time.
 ******************************************************************************/
#include "em_common.h"
#include "app_assert.h"
#include "app_log.h"
#include "sl_status.h"
#include "app.h"

#include "sl_btmesh_api.h"
#include "sl_bt_api.h"
#include "app_timer.h"

#include "em_cmu.h"
#include "em_gpio.h"
#include "em_rtcc.h"

#include "my_model_def.h"

#include "app_button_press.h"
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"

#ifdef PROV_LOCALLY
// Group Addresses
// Choose any 16-bit address starting at 0xC000
#define CUSTOM_STATUS_GRP_ADDR                      0xC001  // Server PUB address
#define CUSTOM_CTRL_GRP_ADDR                        0xC002  // Server SUB address

// The default settings of the network and the node
#define NET_KEY_IDX                                 0
#define APP_KEY_IDX                                 0
#define IVI                                         0
#define DEFAULT_TTL                                 5
// #define ELEMENT_ID
#endif // #ifdef PROV_LOCALLY

#define EX_B0_PRESS                                 ((1) << 5)
#define EX_B0_LONG_PRESS                            ((1) << 6)
#define EX_B1_PRESS                                 ((1) << 7)
#define EX_B1_LONG_PRESS                            ((1) << 8)

// Timing
// Check section 4.2.2.2 of Mesh Profile Specification 1.0 for format
#define STEP_RES_100_MILLI                          0
#define STEP_RES_1_SEC                              ((1) << 6)
#define STEP_RES_10_SEC                             ((2) << 6)
#define STEP_RES_10_MIN                             ((3) << 6)

#define STEP_RES_BIT_MASK                           0xC0

// Max x is 63
#define SET_100_MILLI(x)                            (uint8_t)(STEP_RES_100_MILLI | ((x) & (0x3F)))
#define SET_1_SEC(x)                                (uint8_t)(STEP_RES_1_SEC | ((x) & (0x3F)))
#define SET_10_SEC(x)                               (uint8_t)(STEP_RES_10_SEC | ((x) & (0x3F)))
#define SET_10_MIN(x)                               (uint8_t)(STEP_RES_10_MIN | ((x) & (0x3F)))

// Advertising Provisioning Bearer
#define PB_ADV                                      0x1
// GATT Provisioning Bearer
#define PB_GATT                                     0x2

// Used button indexes
#define BUTTON_PRESS_BUTTON_0                       0
#define BUTTON_PRESS_BUTTON_1                       1

uint8_t conn_handle = 0xFF;

static uint32_t periodic_timer_ms = 0;
static uint8_t update_interval = 0;
static unit_t unit = celsius;

static uint8_t period_idx = 0;
static uint8_t periods[] = {
  SET_100_MILLI(3),        /* 300ms */
  0,
  SET_100_MILLI(20),       /* 2s    */
  0,
  SET_1_SEC(10),           /* 10s   */
  0,
  SET_10_SEC(12),          /* 2min  */
  0,
  SET_10_MIN(1),           /* 10min */
  0
};

my_model_t my_model = {
  .elem_index = PRIMARY_ELEMENT,
  .vendor_id = MY_VENDOR_ID,
  .model_id = MY_MODEL_CLIENT_ID,
  .publish = 1,
  .opcodes_len = NUMBER_OF_OPCODES,
  .opcodes_data[0] = temperature_get,
  .opcodes_data[1] = temperature_status,
  .opcodes_data[2] = unit_get,
  .opcodes_data[3] = unit_set,
  .opcodes_data[4] = unit_set_unack,
  .opcodes_data[5] = unit_status,
  .opcodes_data[6] = update_interval_get,
  .opcodes_data[7] = update_interval_set,
  .opcodes_data[8] = update_interval_set_unack,
  .opcodes_data[9] = update_interval_status
};

#ifdef PROV_LOCALLY
static uint16_t uni_addr = 0;

static aes_key_128 enc_key = {
  .data = "\x03\x03\x03\x03\x03\x03\x03\x03\x03\x03\x03\x03\x03\x03\x03\x03"
};
#endif

static void factory_reset(void);
static void delay_reset_ms(uint32_t ms);
static void parse_period(uint8_t interval);

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
SL_WEAK void app_init(void)
{
  app_log("=================\r\n");
  app_log("Client Device\r\n");
  app_button_press_enable();
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
SL_WEAK void app_process_action(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application code here!                              //
  // This is called infinitely.                                              //
  // Do not call blocking functions from here!                               //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(struct sl_bt_msg *evt)
{
  sl_status_t sc;
  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_system_boot_id:
      // Factory reset the device if Button 0 or 1 is being pressed during reset
      if((sl_simple_button_get_state(&sl_button_btn0) == SL_SIMPLE_BUTTON_PRESSED) || (sl_simple_button_get_state(&sl_button_btn1) == SL_SIMPLE_BUTTON_PRESSED)) {
          factory_reset();
          break;
      }
      // Initialize Mesh stack in Node operation mode,
      // wait for initialized event
      app_log("Node init\r\n");
      sc = sl_btmesh_node_init();

      switch (sc) {
        case 0x00: break;
        case 0x02: app_log("Node already initialized\r\n"); break;
        default: app_assert_status_f(sc, "Failed to init node\r\n");
      }
      break;

    // -------------------------------
    // Handle Button Presses
    case sl_bt_evt_system_external_signal_id: {
      uint8_t opcode = 0, length = 0, data = 0;
      if(evt->data.evt_system_external_signal.extsignals & EX_B0_PRESS) {
          opcode = temperature_get;
          app_log("PB0 Pressed.\r\n");
      }
      if(evt->data.evt_system_external_signal.extsignals & EX_B0_LONG_PRESS) {
          opcode = update_interval_set_unack;
          length = 1;
          data = periods[period_idx];
          if(period_idx == sizeof(periods) - 1) {
              period_idx = 0;
          } else {
              period_idx++;
          }
          app_log("B0 Long Pressed.\r\n");
      }
      if(evt->data.evt_system_external_signal.extsignals & EX_B1_PRESS) {
          opcode = unit_get;
          app_log("PB1 Pressed.\r\n");
      }
      if(evt->data.evt_system_external_signal.extsignals & EX_B1_LONG_PRESS) {
          if (unit == celsius) {
              opcode = unit_set_unack;
              length = 1;
              data = fahrenheit;
          } else {
            opcode = unit_set;
            length = 1;
            data = celsius;
          }
          app_log("B1 Long Pressed.\r\n");
      }
      sc = sl_btmesh_vendor_model_set_publication(my_model.elem_index,
                                                  my_model.vendor_id,
                                                  my_model.model_id,
                                                  opcode,
                                                  1,
                                                  length,
                                                  &data);
      if(sc != SL_STATUS_OK) {
        app_log("Set publication error: 0x%04lX\r\n", sc);
      } else {
        app_log("Set publication done. Publishing...\r\n");
        sc = sl_btmesh_vendor_model_publish(my_model.elem_index,
                                            my_model.vendor_id,
                                            my_model.model_id);
        if (sc != SL_STATUS_OK) {
          app_log("Publish error = 0x%04lX\r\n", sc);
        } else {
          app_log("Publish done.\r\n");
        }
      }
    }
    break;

    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}

/**************************************************************************//**
 * Bluetooth Mesh stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth Mesh stack.
 *****************************************************************************/
void sl_btmesh_on_event(sl_btmesh_msg_t *evt)
{
  sl_status_t sc;
  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_btmesh_evt_node_initialized_id:
      app_log("Node initialized ...\r\n");
      sc = sl_btmesh_vendor_model_init(my_model.elem_index,
                                       my_model.vendor_id,
                                       my_model.model_id,
                                       my_model.publish,
                                       my_model.opcodes_len,
                                       my_model.opcodes_data);
      app_assert_status_f(sc, "Failed to initialize vendor model\r\n");

      if(evt->data.evt_node_initialized.provisioned) {
        app_log("Node already provisioned.\r\n");
      } else {
        app_log("Node unprovisioned\r\n");

#ifdef PROV_LOCALLY
        // Derive the unicast address from the LSB 2 bytes from the BD_ADDR
        bd_addr address;
        sc = sl_bt_system_get_identity_address(&address, 0);
        uni_addr = ((address.addr[1] << 8) | address.addr[0]) & 0x7FFF;
        app_log("Unicast Address = 0x%04X\r\n", uni_addr);
        app_log("Provisioning itself.\r\n");
        sc = sl_btmesh_node_set_provisioning_data(enc_key,
                                                  enc_key,
                                                  NET_KEY_IDX,
                                                  IVI,
                                                  uni_addr,
                                                  0);
        app_assert_status_f(sc, "Failed to provision itself\r\n");
        delay_reset_ms(100);
        break;

#else
        // Start unprovisioned Beaconing using PB-ADV and PB-GATT Bearers
        app_log("Send unprovisioned beacons.\r\n");
        sc = sl_btmesh_node_start_unprov_beaconing(PB_ADV | PB_GATT);
        app_assert_status_f(sc, "Failed to start unprovisioned beaconing\r\n");
#endif // #ifdef PROV_LOCALLY
      }

#ifdef PROV_LOCALLY
      // Set the publication and subscription
      uint16_t appkey_index;
      uint16_t pub_address;
      uint8_t ttl;
      uint8_t period;
      uint8_t retrans;
      uint8_t credentials;
      sc = sl_btmesh_test_get_local_model_pub(my_model.elem_index,
                                              my_model.vendor_id,
                                              my_model.model_id,
                                              &appkey_index,
                                              &pub_address,
                                              &ttl,
                                              &period,
                                              &retrans,
                                              &credentials);
      if (!sc && pub_address == CUSTOM_CTRL_GRP_ADDR) {
        app_log("Configuration done already.\r\n");
      } else {
        app_log("Pub setting result = 0x%04lX, pub setting address = 0x%04X\r\n", sc, pub_address);
        app_log("Add local app key ...\r\n");
        sc = sl_btmesh_test_add_local_key(1,
                                          enc_key,
                                          APP_KEY_IDX,
                                          NET_KEY_IDX);
        app_assert_status_f(sc, "Failed to add local app key\r\n");

        app_log("Bind local app key ...\r\n");
        sc = sl_btmesh_test_bind_local_model_app(my_model.elem_index,
                                                 APP_KEY_IDX,
                                                 my_model.vendor_id,
                                                 my_model.model_id);
        app_assert_status_f(sc, "Failed to bind local app key\r\n");

        app_log("Set local model pub ...\r\n");
        sc = sl_btmesh_test_set_local_model_pub(my_model.elem_index,
                                                APP_KEY_IDX,
                                                my_model.vendor_id,
                                                my_model.model_id,
                                                CUSTOM_CTRL_GRP_ADDR,
                                                DEFAULT_TTL,
                                                0, 0, 0);
        app_assert_status_f(sc, "Failed to set local model pub\r\n");

        app_log("Add local model sub ...\r\n");
        sc = sl_btmesh_test_add_local_model_sub(my_model.elem_index,
                                                my_model.vendor_id,
                                                my_model.model_id,
                                                CUSTOM_STATUS_GRP_ADDR);
        app_assert_status_f(sc, "Failed to add local model sub\r\n");

        app_log("Set relay ...\r\n");
        sc = sl_btmesh_test_set_relay(1, 0, 0);
        app_assert_status_f(sc, "Failed to set relay\r\n");

        app_log("Set Network tx state.\r\n");
        sc = sl_btmesh_test_set_nettx(2, 4);
        app_assert_status_f(sc, "Failed to set network tx state\r\n");
      }
#endif // #ifdef PROV_LOCALLY
      break;

    // -------------------------------
    // Provisioning Events
    case sl_btmesh_evt_node_provisioned_id:
      app_log("Provisioning done.\r\n");
      break;

    case sl_btmesh_evt_node_provisioning_failed_id:
      app_log("Provisioning failed. Result = 0x%04x\r\n",
              evt->data.evt_node_provisioning_failed.result);
      break;

    case sl_btmesh_evt_node_provisioning_started_id:
      app_log("Provisioning started.\r\n");
      break;

    case sl_btmesh_evt_node_key_added_id:
      app_log("got new %s key with index %x\r\n",
              evt->data.evt_node_key_added.type == 0 ? "network " : "application ",
              evt->data.evt_node_key_added.index);
      break;

    case sl_btmesh_evt_node_config_set_id:
      app_log("evt_node_config_set_id\r\n\t");
      break;

    case sl_btmesh_evt_node_model_config_changed_id:
      app_log("model config changed, type: %d, elem_addr: %x, model_id: %x, vendor_id: %x\r\n",
              evt->data.evt_node_model_config_changed.node_config_state,
              evt->data.evt_node_model_config_changed.element_address,
              evt->data.evt_node_model_config_changed.model_id,
              evt->data.evt_node_model_config_changed.vendor_id);
      break;

    // -------------------------------
    // Handle vendor model messages
    case sl_btmesh_evt_vendor_model_receive_id: {
      int32_t temperature = 0;
      sl_btmesh_evt_vendor_model_receive_t *rx_evt = (sl_btmesh_evt_vendor_model_receive_t *)&evt->data;
      app_log("Vendor model data received.\r\n\t"
              "Element index = %d\r\n\t"
              "Vendor id = 0x%04X\r\n\t"
              "Model id = 0x%04X\r\n\t"
              "Source address = 0x%04X\r\n\t"
              "Destination address = 0x%04X\r\n\t"
              "Destination label UUID index = 0x%02X\r\n\t"
              "App key index = 0x%04X\r\n\t"
              "Non-relayed = 0x%02X\r\n\t"
              "Opcode = 0x%02X\r\n\t"
              "Final = 0x%04X\r\n\t"
              "Payload: ",
              rx_evt->elem_index,
              rx_evt->vendor_id,
              rx_evt->model_id,
              rx_evt->source_address,
              rx_evt->destination_address,
              rx_evt->va_index,
              rx_evt->appkey_index,
              rx_evt->nonrelayed,
              rx_evt->opcode,
              rx_evt->final);
      for(int i = 0; i < evt->data.evt_vendor_model_receive.payload.len; i++) {
          app_log("%x ", evt->data.evt_vendor_model_receive.payload.data[i]);
      }
      app_log("\r\n");

      switch (rx_evt->opcode) {
        case temperature_status:
          temperature = *(uint32_t *) rx_evt->payload.data;
          app_log("Temperature = %ld.%ld %s\r\n",
                  temperature / 1000,
                  temperature % 1000,
                  unit == celsius ? (char * )"Celsius" : (char * )"Fahrenheit");
          break;
        case unit_status:
          unit = (unit_t) rx_evt->payload.data[0];
          app_log("Unit = %s\r\n",
                  unit == celsius ? (char * )"Celsius" : (char * )"Fahrenheit");
          break;
        case update_interval_status:
          update_interval = rx_evt->payload.data[0];
          app_log("Period received = %d\r\n", update_interval);
          parse_period(update_interval);
          break;

        default:
          break;
      }
      break;
    }

    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}

void app_button_press_cb(uint8_t button, uint8_t duration)
{
  // Selecting action by duration
  switch (duration) {
    case APP_BUTTON_PRESS_DURATION_SHORT:
      // Handling of button press less than 0.25s
    case APP_BUTTON_PRESS_DURATION_MEDIUM:
      // Handling of button press greater than 0.25s and less than 1s
      if (button == BUTTON_PRESS_BUTTON_0) {
        sl_bt_external_signal(EX_B0_PRESS);
      } else {
        sl_bt_external_signal(EX_B1_PRESS);
      }
      break;
    case APP_BUTTON_PRESS_DURATION_LONG:
      // Handling of button press greater than 1s and less than 5s
    case APP_BUTTON_PRESS_DURATION_VERYLONG:
      if (button == BUTTON_PRESS_BUTTON_0) {
        sl_bt_external_signal(EX_B0_LONG_PRESS);
      } else {
        sl_bt_external_signal(EX_B1_LONG_PRESS);
      }
      break;
    default:
      break;
  }
}

/// Reset
static void factory_reset(void)
{
  app_log("factory reset\r\n");
  sl_btmesh_node_reset();
  delay_reset_ms(100);
}

static void app_reset_timer_cb(app_timer_t *handle, void *data)
{
  (void)handle;
  (void)data;
  sl_bt_system_reboot();
}

static app_timer_t app_reset_timer;
static void delay_reset_ms(uint32_t ms)
{
  if(ms < 10) {
      ms = 10;
  }
  app_timer_start(&app_reset_timer,
                  ms,
                  app_reset_timer_cb,
                  NULL,
                  false);
}


/// Update Interval
static void parse_period(uint8_t interval)
{
  switch (interval & STEP_RES_BIT_MASK) {
    case STEP_RES_100_MILLI:
      periodic_timer_ms = 100 * (interval & (~STEP_RES_BIT_MASK));
      break;
    case STEP_RES_1_SEC:
      periodic_timer_ms = 1000 * (interval & (~STEP_RES_BIT_MASK));
      break;
    case STEP_RES_10_SEC:
      periodic_timer_ms = 10000 * (interval & (~STEP_RES_BIT_MASK));
      break;
    case STEP_RES_10_MIN:
      // 10 min = 600000ms
      periodic_timer_ms = 600000 * (interval & (~STEP_RES_BIT_MASK));
      break;
    default:
      break;
  }
  if (periodic_timer_ms) {
      app_log("Update period [hh:mm:ss:ms]= %02ld:%02ld:%02ld:%04ld\r\n",
              (periodic_timer_ms / (1000 * 60 * 60)),
              (periodic_timer_ms % (1000 * 60 * 60)) / (1000 * 60),
              (periodic_timer_ms % (1000 * 60)) / 1000,
              ((periodic_timer_ms % (1000)) / 1000) * 100);
  } else {
      app_log("Periodic update off.\r\n");
  }
}
