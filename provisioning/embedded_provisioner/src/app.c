/***************************************************************************//**
 * @file
 * @brief Silicon Labs Bluetooth mesh light example this example implements an
 * Embedded provisioner.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
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

/* System header */
#include <stdio.h>
#include <string.h>

#include "sl_btmesh.h"
#include "sl_bluetooth.h"
#include "gatt_db.h"

#include "app.h"
#include "app_log.h"
#include "app_assert.h"
#include "sl_btmesh_factory_reset.h"

#include "btmesh_change.h"

#include "config.h"
#include "sl_btmesh_lighting_client.h"
#include "sl_btmesh_ctl_client.h"

/* Buttons and LEDs headers */
#include "app_button_press.h"
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"

#ifdef SL_CATALOG_BTMESH_WSTK_LCD_PRESENT
#include "sl_btmesh_wstk_lcd.h"
#endif // SL_CATALOG_BTMESH_WSTK_LCD_PRESENT

/***************************************************************************//**
 * @addtogroup Application
 * @{
 ******************************************************************************/
#define LIGHT_CTRL_GRP_ADDR   0xC001
#define LIGHT_STATUS_GRP_ADDR 0xC002

#define LIGHT_MODEL_ID  0x1000 // Generic On/Off Server
#define SWITCH_MODEL_ID 0x1001 // Generic On/Off Client

#define DIM_LIGHT_MODEL_ID  0x1300 // Light Lightness Server
#define DIM_SWITCH_MODEL_ID 0x1302 // Light Lightness Client

#define BLE_MESH_UUID_LEN_BYTE (16)
#define BLE_ADDR_LEN_BYTE (6)

#define MAX_NUM_BTMESH_DEV (10)

device_table_entry_t bluetooth_device_table[MAX_NUM_BTMESH_DEV];

static const uint8_t fixed_netkey[16] = {0x23, 0x98, 0xdf, 0xa5, 0x09, 0x3e, 0x74, 0xbb, 0xc2, 0x45, 0x1f, 0xae, 0xea, 0xd7, 0x67, 0xcd};
static const uint8_t fixed_appkey[16] = {0x16, 0x39, 0x38, 0x03, 0x9b, 0x8d, 0x8a, 0x20, 0x81, 0x60, 0xa7, 0x93, 0x33, 0x3d, 0x03, 0x61};

static uint8_t network_id = 0x0;
static uint16_t appkey_index = 0x0;
static uint16_t provisionee_addr;
static uuid_128 provisionee_uuid;
static uint16_t sub_address;

/// Length of the display name buffer
#define NAME_BUF_LEN                   20

/// Increase step of physical values (lightness, color temperature)
#define INCREASE                       10
/// Decrease step of physical values (lightness, color temperature)
#define DECREASE                       (-10)
/// Used button index
#define BUTTON_PRESS_BUTTON_0          0
/// Used button index
#define BUTTON_PRESS_BUTTON_1          1

#ifdef SL_CATALOG_BTMESH_WSTK_LCD_PRESENT
#define lcd_print(...) sl_btmesh_LCD_write(__VA_ARGS__)
#else
#define lcd_print(...)
#endif // SL_CATALOG_BTMESH_WSTK_LCD_PRESENT

/* DCD parsing */

typedef struct {
  // model bindings to be done. for simplicity, all models are bound to same appkey in this example
  // (assuming there is exactly one appkey used and the same appkey is used for all model bindings)
  tsModel bind_model[4];
  uint8_t num_bind;
  uint8_t num_bind_done;

  // publish addresses for up to 4 models
  tsModel pub_model[4];
  uint16_t pub_address[4];
  uint8_t num_pub;
  uint8_t num_pub_done;

  // subscription addresses for up to 4 models
  tsModel sub_model[4];
  uint16_t sub_address[4];
  uint8_t num_sub;
  uint8_t num_sub_done;

} tsConfig;

// config data to be sent to last provisioned node:
tsConfig _sConfig;

// For selective provisioning
bool select_provisionee_mode = false;
int8_t select_provisionee_target = -1;
uint8_t max_provisionee_index = 0;

static int8_t is_bluetooth_device_present(const uint8_t * const addr);
static void config_check(void);

/*******************************************************************************
 * Application Init.
 ******************************************************************************/
SL_WEAK void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
  app_log("Boot\r\n");

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

/*
 * Add one publication setting to the list of configurations to be done
 * */
static void config_pub_add(uint16_t model_id, uint16_t vendor_id, uint16_t address)
{
  _sConfig.pub_model[_sConfig.num_pub].model_id = model_id;
  _sConfig.pub_model[_sConfig.num_pub].vendor_id = vendor_id;
  _sConfig.pub_address[_sConfig.num_pub] = address;
  _sConfig.num_pub++;
}

/*
 * Add one subscription setting to the list of configurations to be done
 * */
static void config_sub_add(uint16_t model_id, uint16_t vendor_id, uint16_t address)
{
  _sConfig.sub_model[_sConfig.num_sub].model_id = model_id;
  _sConfig.sub_model[_sConfig.num_sub].vendor_id = vendor_id;
  _sConfig.sub_address[_sConfig.num_sub] = address;
  _sConfig.num_sub++;
}

/*
 * Add one appkey/model bind setting to the list of configurations to be done
 * */
static void config_bind_add(uint16_t model_id, uint16_t vendor_id)
{
  _sConfig.bind_model[_sConfig.num_bind].model_id = model_id;
  _sConfig.bind_model[_sConfig.num_bind].vendor_id = vendor_id;
  _sConfig.num_bind++;
}

/* Initialize the stack */
void init_bluetooth_device_table()
{
  /* Reset working structure */
  memset(&bluetooth_device_table, 0x00, (sizeof(device_table_entry_t) * MAX_NUM_BTMESH_DEV));

  return;
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
  snprintf(name, NAME_BUF_LEN, "provisioner %02x:%02x",
           addr->addr[1], addr->addr[0]);

  app_log("Device name: '%s'\r\n", name);

  result = sl_bt_gatt_server_write_attribute_value(gattdb_device_name,
                                                   0,
                                                   strlen(name),
                                                   (uint8_t *)name);
  if(result) {
    app_log("sl_bt_gatt_server_write_attribute_value() failed, code %lx\r\n", result);
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
  if(sl_simple_button_get_state(&sl_button_btn0) == SL_SIMPLE_BUTTON_PRESSED) {
    // Full factory reset
    sl_btmesh_initiate_full_reset();
    return false;
  }

#ifndef SINGLE_BUTTON
  // If PB1 is held down then do node factory reset
  if(sl_simple_button_get_state(&sl_button_btn1) == SL_SIMPLE_BUTTON_PRESSED) {
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
  // Check reset conditions and continue ifnot reset.
  if (handle_reset_conditions()) {
    sc = sl_bt_system_get_identity_address(&address, &address_type);
    app_assert_status_f(sc, "Failed to get Bluetooth address\n");
    set_device_name(&address);
    init_bluetooth_device_table();
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
      app_log("Connection opened\r\n");
      break;
    case sl_bt_evt_connection_closed_id:
      app_log("Connection closed\r\n");
      break;
    default:
      break;
  }
}

void sl_btmesh_on_event(sl_btmesh_msg_t *evt)
{
  sl_btmesh_evt_config_client_dcd_data_t *pDCD;
  uint16_t result = 0;
  uint8_t idx; /* j short array index */
  int8_t dev_idx; /* device */
  uint16_t vendor_id;
  uint16_t model_id;
  uint16_t pub_address;
  sl_status_t sc;
  uint32_t handle;

  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_btmesh_evt_node_reset_id:
      sl_btmesh_initiate_full_reset();
      break;
    case sl_btmesh_evt_prov_initialized_id: {
      app_log("sl_btmesh_evt_prov_initialized_id\r\n");
      sc = sl_btmesh_prov_create_network(network_id,
                                         16,
                                         fixed_netkey);
      if(sc != SL_STATUS_OK) {
        /* Something went wrong */
        app_log("sl_btmesh_prov_create_network: failed 0x%.2lx\r\n", sc);
      } else {
        app_log("Success, netkey id = %x\r\n", network_id);
      }

      size_t max_application_key_size = 16;
      size_t application_key_len = 16;
      uint8_t application_key[16];
      sc = sl_btmesh_prov_create_appkey(network_id,
                                        appkey_index,
                                        16,
                                        fixed_appkey,
                                        max_application_key_size,
                                        &application_key_len,
                                        application_key);
      if(sc != SL_STATUS_OK) {
        /* Something went wrong */
        app_log("sl_btmesh_prov_create_appkey: failed 0x%.2lx\r\n", sc);
      } else {
        app_log("Success, appkey id = %x\r\n", appkey_index);
      }

      /* Networks  */
      app_log("networks: 0x%x ", evt->data.evt_prov_initialized.networks);

      /* address */
      app_log("address: 0x%x ", evt->data.evt_prov_initialized.address);

      /* ivi  */
      app_log("ivi: 0x%lx", evt->data.evt_prov_initialized.iv_index);
      app_log("\r\n");

      sl_btmesh_generic_client_init();

      sc = sl_btmesh_test_bind_local_model_app(0,
                                               appkey_index,
                                               0xFFFF,
                                               SWITCH_MODEL_ID);
      if(sc != SL_STATUS_OK) {
        /* Something went wrong */
        app_log("sl_btmesh_test_bind_local_model_app: failed 0x%.2lx\r\n", sc);
      } else {
        app_log("Success, sl_btmesh_test_bind_local_model_app: 0x%x\r\n", SWITCH_MODEL_ID);
      }

      sc = sl_btmesh_test_bind_local_model_app(0,
                                               appkey_index,
                                               0xFFFF,
                                               DIM_SWITCH_MODEL_ID);
      if(sc != SL_STATUS_OK) {
        /* Something went wrong */
        app_log("sl_btmesh_test_bind_local_model_app: failed 0x%.2lx\r\n", sc);
      } else {
        app_log("Success, sl_btmesh_test_bind_local_model_app: 0x%x\r\n", DIM_SWITCH_MODEL_ID);
      }

      sc = sl_btmesh_test_add_local_model_sub(0,
                                              0xFFFF,
                                              SWITCH_MODEL_ID,
                                              LIGHT_STATUS_GRP_ADDR);
      if(sc != SL_STATUS_OK) {
        /* Something went wrong */
        app_log("sl_btmesh_test_add_local_model_sub: failed 0x%.2lx\r\n", sc);
      } else {
        app_log("Success, sl_btmesh_test_add_local_model_sub: 0x%x\r\n", SWITCH_MODEL_ID);
      }

      sc = sl_btmesh_test_set_local_model_pub(0,
                                              appkey_index,
                                              0xFFFF,
                                              SWITCH_MODEL_ID,
                                              LIGHT_CTRL_GRP_ADDR,
                                              3,
                                              0,
                                              0,
                                              0);
      if(sc != SL_STATUS_OK) {
        /* Something went wrong */
        app_log("sl_btmesh_test_set_local_model_pub: failed 0x%.2lx\r\n", sc);
      } else {
        app_log("Success, sl_btmesh_test_set_local_model_pub: 0x%x\r\n", SWITCH_MODEL_ID);
      }

      sc = sl_btmesh_test_add_local_model_sub(0,
                                              0xFFFF,
                                              DIM_SWITCH_MODEL_ID,
                                              LIGHT_STATUS_GRP_ADDR);
      if(sc != SL_STATUS_OK) {
        /* Something went wrong */
        app_log("sl_btmesh_test_add_local_model_sub: failed 0x%.2lx\r\n", sc);
      } else {
        app_log("Success, sl_btmesh_test_add_local_model_sub: 0x%x\r\n", DIM_SWITCH_MODEL_ID);
      }

      sc = sl_btmesh_test_set_local_model_pub(0,
                                              appkey_index,
                                              0xFFFF,
                                              DIM_SWITCH_MODEL_ID,
                                              LIGHT_CTRL_GRP_ADDR,
                                              3,
                                              0,
                                              0,
                                              0);
      if(sc != SL_STATUS_OK) {
        /* Something went wrong */
        app_log("sl_btmesh_test_set_local_model_pub: failed 0x%.2lx\r\n", sc);
      } else {
        app_log("Success, sl_btmesh_test_set_local_model_pub: 0x%x\r\n", DIM_SWITCH_MODEL_ID);
      }

      app_log("\r\nLong-press BTN0 to provision the first seen Node. Long-press BTN1 to list the unprovisioned Nodes for targeted provisioning.\r\n\r\n");

      /* Scan for unprovisioned beacons */
      result = sl_btmesh_prov_scan_unprov_beacons();
    }
    break;
    case sl_btmesh_evt_prov_initialization_failed_id:
      app_log("failed: 0x%x ", evt->data.evt_prov_initialization_failed.result);
    break;
    case sl_btmesh_evt_prov_unprov_beacon_id:
      /* PB-ADV only */
      if(0 == evt->data.evt_prov_unprov_beacon.bearer) {
        /* Get BT mesh device index */
        dev_idx = is_bluetooth_device_present(&evt->data.evt_prov_unprov_beacon.address.addr[0]);

        /* fill up btmesh device struct */
        if(0 > dev_idx) {
          /* Device is not present */
          for(idx = 0; idx < MAX_NUM_BTMESH_DEV; idx++) {
            if(0x00 == bluetooth_device_table[idx].address.addr[0] &&
               0x00 == bluetooth_device_table[idx].address.addr[1] &&
               0x00 == bluetooth_device_table[idx].address.addr[2] &&
               0x00 == bluetooth_device_table[idx].address.addr[3] &&
               0x00 == bluetooth_device_table[idx].address.addr[4] &&
               0x00 == bluetooth_device_table[idx].address.addr[5]
            ){
              memcpy(&bluetooth_device_table[idx].address.addr[0], &evt->data.evt_prov_unprov_beacon.address.addr[0], BLE_ADDR_LEN_BYTE);
              memcpy(&bluetooth_device_table[idx].uuid.data[0], &evt->data.evt_prov_unprov_beacon.uuid.data[0], BLE_MESH_UUID_LEN_BYTE);

              /* Display info banner */
              app_log("URI hash: 0x%lx ", evt->data.evt_prov_unprov_beacon.uri_hash);
              app_log("bearer: 0x%x ", evt->data.evt_prov_unprov_beacon.bearer);
              app_log("address: 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x ", bluetooth_device_table[idx].address.addr[0],
                                                                 bluetooth_device_table[idx].address.addr[1],
                                                                 bluetooth_device_table[idx].address.addr[2],
                                                                 bluetooth_device_table[idx].address.addr[3],
                                                                 bluetooth_device_table[idx].address.addr[4],
                                                                 bluetooth_device_table[idx].address.addr[5]);
              app_log("(net id) %d (uuid) ", network_id);
              for(uint8_t i = 0; i < BLE_MESH_UUID_LEN_BYTE; i++) app_log("0x%x:", bluetooth_device_table[idx].uuid.data[i]);
              app_log("\r\n");
              app_log("address type: 0x%x", evt->data.evt_prov_unprov_beacon.address_type);
              app_log("\r\n");
              break;
            }
          }
        }
      }

      break;
    /* Provisioning */
    case sl_btmesh_evt_prov_provisioning_failed_id:
      app_log("provisioning failed\r\n");
      break;
    case sl_btmesh_evt_prov_device_provisioned_id:
      provisionee_addr = evt->data.evt_prov_device_provisioned.address;
      provisionee_uuid = evt->data.evt_prov_device_provisioned.uuid;
      app_log("Node successfully provisioned. Address: %4.4x, ", provisionee_addr);

      app_log("uuid 0x");
      for(uint8_t i = 0; i < BLE_MESH_UUID_LEN_BYTE; i++) app_log("%02X", provisionee_uuid.data[i]);
      app_log("\r\n");

      app_log(" getting dcd ...\r\n");

      sc = sl_btmesh_config_client_get_dcd(network_id,
                                           provisionee_addr,
                                           0,
                                           &handle);
      if(sc == 0x0181) {
        app_log(".");
      } else if(sc != SL_STATUS_OK) {
        app_log("sl_btmesh_config_client_get_dcd failed with result 0x%lX (%ld) addr %x\r\n", sc, sc, provisionee_addr);
      }
      else {
        app_log("requesting DCD from the node...\r\n");
      }

      break;

    /* Config events */
    case sl_btmesh_evt_config_client_dcd_data_id:
      pDCD = &evt->data.evt_config_client_dcd_data;
      app_log("DCD data event, received %u bytes\r\n", pDCD->data.len);

      // copy the data into one large array. the data may come in multiple smaller pieces.
      // the data is not decoded until all DCD events have been received (see below)
      if((_dcd_raw_len + pDCD->data.len) <= 256) {
        memcpy(&(_dcd_raw[_dcd_raw_len]), pDCD->data.data, pDCD->data.len);
        _dcd_raw_len += pDCD->data.len;
      }

      break;
    case sl_bt_evt_connection_opened_id:
      app_log("LE connection opened\r\n");
      break;

    case sl_btmesh_evt_config_client_dcd_data_end_id:
      app_log("DCD data end event. Decoding the data.\r\n");
      // decode the DCD content
      DCD_decode();

      // check the desired configuration settings depending on what's in the DCD
      config_check();

      sc = sl_btmesh_config_client_add_appkey(network_id,
                                              provisionee_addr,
                                              appkey_index,
                                              network_id,
                                              &handle);
      if(sc == SL_STATUS_OK) {
        app_log("Deploying appkey to node 0x%4.4x\r\n", provisionee_addr);
      } else {
        app_log("Appkey deployment failed. addr %x, error: %lx\r\n", provisionee_addr, sc);
      }

      lcd_print("DCD received", SL_BTMESH_WSTK_LCD_ROW_STATUS_CFG_VAL);

      break;
    case sl_btmesh_evt_config_client_appkey_status_id:
      result = evt->data.evt_config_client_appkey_status.result;
      if(result == SL_STATUS_OK) {
        app_log(" appkey added\r\n");
        /* move to next step which is binding appkey to models */

        // take the next model from the list of models to be bound with application key.
        // for simplicity, the same appkey is used for all models but it is possible to also use several appkeys
        model_id = _sConfig.bind_model[_sConfig.num_bind_done].model_id;
        vendor_id = _sConfig.bind_model[_sConfig.num_bind_done].vendor_id;

        app_log("APP BIND, config %d/%d:: model %4.4x key index %x\r\n", _sConfig.num_bind_done+1, _sConfig.num_bind, model_id, appkey_index);

        sc = sl_btmesh_config_client_bind_model(network_id,
                                                provisionee_addr,
                                                0,
                                                vendor_id,
                                                model_id,
                                                appkey_index,
                                                &handle);
        if(sc == SL_STATUS_OK) {
          app_log("Binding model 0x%4.4x\r\n", model_id);
        } else {
          app_log("Binding model %x, error: %lx\r\n", model_id, sc);
        }
      }
      break;
    case sl_btmesh_evt_config_client_binding_status_id:
      result = evt->data.evt_config_client_appkey_status.result;
      if(result == SL_STATUS_OK) {
        app_log(" bind complete\r\n");
        _sConfig.num_bind_done++;

        if(_sConfig.num_bind_done < _sConfig.num_bind) {
          // take the next model from the list of models to be bound with application key.
          // for simplicity, the same appkey is used for all models but it is possible to also use several appkeys
          model_id = _sConfig.bind_model[_sConfig.num_bind_done].model_id;
          vendor_id = _sConfig.bind_model[_sConfig.num_bind_done].vendor_id;

          app_log("APP BIND, config %d/%d:: model %4.4x key index %x\r\n", _sConfig.num_bind_done+1, _sConfig.num_bind, model_id, appkey_index);

          sc = sl_btmesh_config_client_bind_model(network_id,
                                                  provisionee_addr,
                                                  0,
                                                  vendor_id,
                                                  model_id,
                                                  appkey_index,
                                                  &handle);
          if(sc == SL_STATUS_OK) {
            app_log("Binding model 0x%4.4x\r\n", model_id);
          } else {
            app_log("Binding model %x, error: %lx\r\n", model_id, sc);
          }
        } else {
          // get the next model/address pair from the configuration list:
          model_id = _sConfig.pub_model[_sConfig.num_pub_done].model_id;
          vendor_id = _sConfig.pub_model[_sConfig.num_pub_done].vendor_id;
          pub_address = _sConfig.pub_address[_sConfig.num_pub_done];

          app_log("PUB SET, config %d/%d: model %4.4x -> address %4.4x\r\n", _sConfig.num_pub_done+1, _sConfig.num_pub, model_id, pub_address);

          sc = sl_btmesh_config_client_set_model_pub(network_id,
                                                     provisionee_addr,
                                                     0, /* element index */
                                                     vendor_id,
                                                     model_id,
                                                     pub_address,
                                                     appkey_index,
                                                     0, /* friendship credential flag */
                                                     3, /* Publication time-to-live value */
                                                     0, /* period = NONE */
                                                     0, /* Publication retransmission count */
                                                     50,  /* Publication retransmission interval */
                                                     &handle);
          if(sc == SL_STATUS_OK) {
            app_log(" waiting pub ack\r\n");
          }
        }
      } else {
        app_log(" appkey bind failed with code %x\r\n", result);
      }
      break;
    case sl_btmesh_evt_config_client_model_pub_status_id:
      result = evt->data.evt_config_client_model_pub_status.result;
      if(result == SL_STATUS_OK) {
        app_log(" pub set OK\r\n");
        _sConfig.num_pub_done++;

        if(_sConfig.num_pub_done < _sConfig.num_pub) {
          /* more publication settings to be done
          ** get the next model/address pair from the configuration list: */
          model_id = _sConfig.pub_model[_sConfig.num_pub_done].model_id;
          vendor_id = _sConfig.pub_model[_sConfig.num_pub_done].vendor_id;
          pub_address = _sConfig.pub_address[_sConfig.num_pub_done];

          app_log("PUB SET, config %d/%d: model %4.4x -> address %4.4x\r\n", _sConfig.num_pub_done+1, _sConfig.num_pub, model_id, pub_address);

          sc = sl_btmesh_config_client_set_model_pub(network_id,
                                                     provisionee_addr,
                                                     0, /* element index */
                                                     vendor_id,
                                                     model_id,
                                                     pub_address,
                                                     appkey_index,
                                                     0, /* friendship credential flag */
                                                     3, /* Publication time-to-live value */
                                                     0, /* period = NONE */
                                                     0, /* Publication retransmission count */
                                                     50,  /* Publication retransmission interval */
                                                     &handle);
        } else {
          // move to next step which is configuring subscription settings
          // get the next model/address pair from the configuration list:
          model_id = _sConfig.sub_model[_sConfig.num_sub_done].model_id;
          vendor_id = _sConfig.sub_model[_sConfig.num_sub_done].vendor_id;
          sub_address = _sConfig.sub_address[_sConfig.num_sub_done];

          app_log("SUB ADD, config %d/%d: model %4.4x -> address %4.4x\r\n", _sConfig.num_sub_done+1, _sConfig.num_sub, model_id, sub_address);

          sc = sl_btmesh_config_client_add_model_sub(network_id,
                                                     provisionee_addr,
                                                     0,
                                                     vendor_id,
                                                     model_id,
                                                     sub_address,
                                                     &handle);
          if(sc == SL_STATUS_OK) {
            app_log(" waiting sub ack\r\n");
          }
        }
      } else {
        app_log(" pub set failed with code %x\r\n", result);
      }
      break;
    case sl_btmesh_evt_config_client_model_sub_status_id:
      result = evt->data.evt_config_client_model_sub_status.result;
      if(result == SL_STATUS_OK) {
        app_log(" sub add OK\r\n");
        _sConfig.num_sub_done++;
        if(_sConfig.num_sub_done < _sConfig.num_sub) {
          // move to next step which is configuring subscription settings
          // get the next model/address pair from the configuration list:
          model_id = _sConfig.sub_model[_sConfig.num_sub_done].model_id;
          vendor_id = _sConfig.sub_model[_sConfig.num_sub_done].vendor_id;
          sub_address = _sConfig.sub_address[_sConfig.num_sub_done];

          app_log("SUB ADD, config %d/%d: model %4.4x -> address %4.4x\r\n", _sConfig.num_sub_done+1, _sConfig.num_sub, model_id, sub_address);

          sc = sl_btmesh_config_client_add_model_sub(network_id,
                                                     provisionee_addr,
                                                     0,
                                                     vendor_id,
                                                     model_id,
                                                     sub_address,
                                                     &handle);
          if(sc == SL_STATUS_OK) {
            app_log(" waiting sub ack\r\n");
          }
        } else {
          app_log("***\r\nconfiguration complete\r\n***\r\n");
        }
      } else {
        app_log(" sub add failed with code %x\r\n", result);
      }
      break;
    case sl_btmesh_evt_generic_server_client_request_id:
      app_log("server_address %x elem_index %x model_id %x client_address %x appkey_index %x\r\n",
              evt->data.evt_generic_server_client_request.server_address,
              evt->data.evt_generic_server_client_request.elem_index,
              evt->data.evt_generic_server_client_request.model_id,
              evt->data.evt_generic_server_client_request.client_address,
              evt->data.evt_generic_server_client_request.appkey_index);
      break;
    case sl_btmesh_evt_node_model_config_changed_id:
      app_log("node_config_state %x element_address %x vendor_id %x model_id %x\r\n",
              evt->data.evt_node_model_config_changed.node_config_state,
              evt->data.evt_node_model_config_changed.element_address,
              evt->data.evt_node_model_config_changed.vendor_id,
              evt->data.evt_node_model_config_changed.model_id);
      break;
    default:
      app_log("unhandled evt: %8.8x class %2.2x method %2.2x\r\n", (unsigned int)SL_BT_MSG_ID(evt->header),
                                                                   (unsigned int)((SL_BT_MSG_ID(evt->header) >> 16) & 0xFF),
                                                                   (unsigned int)((SL_BT_MSG_ID(evt->header) >> 24) & 0xFF));
      break;
  }

  return;
}

void provision_bluetooth_device(device_table_entry_t *device) 
{
  sl_status_t sc;
  /* provisioning using ADV bearer (this is the default) */
  sl_btmesh_prov_create_provisioning_session(network_id, device->uuid, 0);
  sc = sl_btmesh_prov_provision_adv_device(device->uuid);
  if(sc == SL_STATUS_OK) {
    app_log("Provisioning success: ");
    device->is_provisioned = 0x01;
  } else {
    app_log("Provisioning fail %lX: ", sc);
  }

  app_log("(net id) %d (uuid) ", network_id);
  for(uint8_t i = 0; i < BLE_MESH_UUID_LEN_BYTE; i++) app_log("0x%.2x", device->uuid.data[i]);
  app_log("\r\n");

  return;
}

void select_bluetooth_device(eMesh_Prov_Node_t eStrategy)
{
  if(eMESH_PROV_ALL == eStrategy ||
     eMESH_PROV_NEXT == eStrategy) {
    for(uint8_t dev_idx = 0; dev_idx < MAX_NUM_BTMESH_DEV; dev_idx++) {

      /* check if non null and unprovisioned */
      if((0x00 != bluetooth_device_table[dev_idx].address.addr[0] ||
          0x00 != bluetooth_device_table[dev_idx].address.addr[1] ||
          0x00 != bluetooth_device_table[dev_idx].address.addr[2] ||
          0x00 != bluetooth_device_table[dev_idx].address.addr[3] ||
          0x00 != bluetooth_device_table[dev_idx].address.addr[4] ||
          0x00 != bluetooth_device_table[dev_idx].address.addr[5]) &&
          0x00 == bluetooth_device_table[dev_idx].is_provisioned
      ){
        provision_bluetooth_device(&bluetooth_device_table[dev_idx]);
        break;
      }
    }
  }

  return;
}

void print_bluetooth_device_table()
{
  select_provisionee_mode = true;

  for(uint8_t dev_idx = 0; dev_idx < MAX_NUM_BTMESH_DEV; dev_idx++) {

    /* check if non null and unprovisioned */
    if((0x00 != bluetooth_device_table[dev_idx].address.addr[0] ||
        0x00 != bluetooth_device_table[dev_idx].address.addr[1] ||
        0x00 != bluetooth_device_table[dev_idx].address.addr[2] ||
        0x00 != bluetooth_device_table[dev_idx].address.addr[3] ||
        0x00 != bluetooth_device_table[dev_idx].address.addr[4] ||
        0x00 != bluetooth_device_table[dev_idx].address.addr[5]) &&
        0x00 == bluetooth_device_table[dev_idx].is_provisioned
    ){
      app_log("\nIndex: %d - ", dev_idx);
      app_log("Address: 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x ", bluetooth_device_table[dev_idx].address.addr[0],
                                                         bluetooth_device_table[dev_idx].address.addr[1],
                                                         bluetooth_device_table[dev_idx].address.addr[2],
                                                         bluetooth_device_table[dev_idx].address.addr[3],
                                                         bluetooth_device_table[dev_idx].address.addr[4],
                                                         bluetooth_device_table[dev_idx].address.addr[5]);
      max_provisionee_index = dev_idx;
    }
  }

  app_log("\nSelect device [Index] with BTN0/1 - Long-press BTN0 for more then 10 seconds to provision it:\nIndex:  ");
  return;
}

static int8_t is_bluetooth_device_present(const uint8_t * restrict const addr)
{
  uint8_t *pdata;
  int8_t res_val;

  /* Initialize locals */
  res_val=-1; /* Pessimistic assumption, dev isn't present */
  pdata=NULL;

  for(uint8_t idx=0; idx < MAX_NUM_BTMESH_DEV; idx++) {
      pdata=(uint8_t*)(&bluetooth_device_table[idx].address.addr[0]);

      if(addr[0] == pdata[0] &&
         addr[1] == pdata[1] &&
         addr[2] == pdata[2] &&
         addr[3] == pdata[3] &&
         addr[4] == pdata[4] &&
         addr[5] == pdata[5]
      ){
        res_val=idx;
        break;
      }
  }

  return res_val;
}

static void config_check()
{
  memset(&_sConfig, 0, sizeof(_sConfig));
  // scan the SIG models in the DCD data
  for(int i = 0; i < _sDCD_Prim.numSIGModels; i++) {
    if(_sDCD_Prim.SIG_models[i] == SWITCH_MODEL_ID) {
      config_pub_add(SWITCH_MODEL_ID, 0xFFFF, LIGHT_CTRL_GRP_ADDR);
      config_sub_add(SWITCH_MODEL_ID, 0xFFFF, LIGHT_STATUS_GRP_ADDR);
      config_bind_add(SWITCH_MODEL_ID, 0xFFFF);
    } else if(_sDCD_Prim.SIG_models[i] == LIGHT_MODEL_ID) {
      config_pub_add(LIGHT_MODEL_ID, 0xFFFF, LIGHT_STATUS_GRP_ADDR);
      config_sub_add(LIGHT_MODEL_ID, 0xFFFF, LIGHT_CTRL_GRP_ADDR);
      config_bind_add(LIGHT_MODEL_ID, 0xFFFF);
    } else if(_sDCD_Prim.SIG_models[i] == DIM_SWITCH_MODEL_ID) {
      config_pub_add(DIM_SWITCH_MODEL_ID, 0xFFFF, LIGHT_CTRL_GRP_ADDR);
      config_sub_add(DIM_SWITCH_MODEL_ID, 0xFFFF, LIGHT_STATUS_GRP_ADDR);
      config_bind_add(DIM_SWITCH_MODEL_ID, 0xFFFF);
    } else if(_sDCD_Prim.SIG_models[i] == DIM_LIGHT_MODEL_ID) {
      config_pub_add(DIM_LIGHT_MODEL_ID, 0xFFFF, LIGHT_STATUS_GRP_ADDR);
      config_sub_add(DIM_LIGHT_MODEL_ID, 0xFFFF, LIGHT_CTRL_GRP_ADDR);
      config_bind_add(DIM_LIGHT_MODEL_ID, 0xFFFF);
    }
  }
}

/***************************************************************************//**
 * Button press Callbacks
 ******************************************************************************/

void app_button_press_select_provisionee_cb(uint8_t button, uint8_t duration)
{
  // Selecting action by duration
  switch (duration) {
    case APP_BUTTON_PRESS_DURATION_SHORT:
      // Handling of button press less than 0.25s
      if(button == BUTTON_PRESS_BUTTON_0) {
        if(select_provisionee_target < max_provisionee_index) select_provisionee_target++;
      } else {
        if(select_provisionee_target > 0) select_provisionee_target--;
      }

      if(select_provisionee_target > max_provisionee_index || select_provisionee_target < 0) return;

      // Step back two positions and print the Index
      sl_iostream_t *stream = sl_iostream_get_default();
      sl_iostream_putchar(stream, 0x08);
      sl_iostream_putchar(stream, 0x08);
      app_log("%*d", 2, select_provisionee_target);

      break;
    case APP_BUTTON_PRESS_DURATION_VERYLONG:
      if(button == BUTTON_PRESS_BUTTON_0) {
        app_log("\n\n");
        select_provisionee_mode = false;

        if(select_provisionee_target > max_provisionee_index || select_provisionee_target < 0) {
            app_log("Please, select a valid device [Index]!\n");
            select_provisionee_target = -1;
            return;
        }
        if(!bluetooth_device_table[select_provisionee_target].is_provisioned) provision_bluetooth_device(&bluetooth_device_table[select_provisionee_target]);
        else app_log("Device is already provisioned!\n");

        select_provisionee_target = -1;
      }
      break;
    default:
      break;
  }

  return;
}

void app_button_press_cb(uint8_t button, uint8_t duration)
{
  if(select_provisionee_mode) {
      app_button_press_select_provisionee_cb(button, duration);
      return;
  }

  // Selecting action by duration
  switch (duration) {
    case APP_BUTTON_PRESS_DURATION_SHORT:
      // Handling of button press less than 0.25s
      if(button == BUTTON_PRESS_BUTTON_0) {
        sl_btmesh_change_lightness(DECREASE);
      } else {
        sl_btmesh_change_lightness(INCREASE);
      }
      break;
    case APP_BUTTON_PRESS_DURATION_MEDIUM:
      // Handling of button press greater than 0.25s and less than 1s
      if(button == BUTTON_PRESS_BUTTON_0) {
        sl_btmesh_change_temperature(DECREASE);
      } else {
        sl_btmesh_change_temperature(INCREASE);
      }
      break;
    case APP_BUTTON_PRESS_DURATION_LONG:
      // Handling of button press greater than 1s and less than 5s
      if(button == BUTTON_PRESS_BUTTON_0) {
        sl_btmesh_change_switch_position(SL_BTMESH_LIGHTING_CLIENT_OFF);
      } else {
        sl_btmesh_change_switch_position(SL_BTMESH_LIGHTING_CLIENT_ON);
      }
      break;
    case APP_BUTTON_PRESS_DURATION_VERYLONG:
      if(button == BUTTON_PRESS_BUTTON_0) {
        select_bluetooth_device(eMESH_PROV_NEXT);
      } else {
        print_bluetooth_device_table();
      }
      break;
    default:
      break;
  }
}
