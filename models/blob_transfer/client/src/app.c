/***************************************************************************//**
 * @file
 * @brief Core application logic.
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
#include <stdio.h>
#include "em_common.h"
#include "app_assert.h"
#include "sl_status.h"
#include "app.h"

#include "sl_btmesh_api.h"
#include "sl_bt_api.h"
#include "sl_board_control.h"
#include "glib.h"
#include "dmd.h"

/* Buttons and LEDs headers */
#include "app_button_press.h"
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"

#include "sl_btmesh_blob_transfer_client.h"

#ifdef SL_CATALOG_BTMESH_FACTORY_RESET_PRESENT
#include "sl_btmesh_factory_reset.h"
#endif // SL_CATALOG_BTMESH_FACTORY_RESET_PRESENT

/// Advertising Provisioning Bearer
#define PB_ADV                         0x1
/// GATT Provisioning Bearer
#define PB_GATT                        0x2

#define BUTTON_0                       0

#define CLIENT_LIMIT                   4

#define GRP_ADDR_BLOB                  0xC001
#define GRP_ADDR_BLOB_ID               0xC002

static GLIB_Context_t glibContext;

uint8_t row, column = 0;

static uint16_t appkey_index = 0x0;
static const uint8_t network_id = 0x0;
static const uint16_t enc_netkey_index = 0x0;
static const uint8_t fixed_netkey[16] = {0x23, 0x98, 0xdf, 0xa5, 0x09, 0x3e, 0x74, 0xbb, 0xc2, 0x45, 0x1f, 0xae, 0xea, 0xd7, 0x67, 0xcd};
static const uint8_t fixed_appkey[16] = {0x16, 0x39, 0x38, 0x03, 0x9b, 0x8d, 0x8a, 0x20, 0x81, 0x60, 0xa7, 0x93, 0x33, 0x3d, 0x03, 0x61};

static const uint8_t vendor_model_elem_index = 0x0;
static const uint8_t blob_server_model_elem_index = 0x0;
static const uint8_t blob_client_model_elem_index = 0x1;

static uint16_t provisionee_addr;
static uuid_128 provisionee_uuid;

typedef struct {
  uuid_128 uuid;
  bd_addr address;
  uint8_t is_provisioned;
  uint16_t primary_element_address;
} client_device_t;

static const char client_device_id[] = "ClientDevice";
static const uuid_128 empty_uuid = {{0x00}};
static const bd_addr empty_addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
static const client_device_t client_placeholder = {empty_uuid, empty_addr, 0x0, 0x0};
client_device_t client_list[CLIENT_LIMIT];
uint8_t servers[CLIENT_LIMIT*2];
uint8_t servers_size = 0;
uint16_t data_id = 0;

sl_bt_uuid_64_t blob_id = {{0}};
uuid_128 virtual_address = {{0}};
uint8_t data[2048];

static void handle_blob_transfer_client_notification(const sl_btmesh_blob_transfer_client_notification_t *const notification);

/***************************************************************************//**
 * Handles button press and does a factory reset
 *
 * @return true if there is no button press
 ******************************************************************************/
static bool handle_reset_conditions(void);

/*******************************************************************************
 * Application Init.
 ******************************************************************************/
SL_WEAK void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
  app_log("boot\r\n");
  handle_reset_conditions();

  // Fill up the server list
  for(uint8_t index = 0; index < CLIENT_LIMIT; index++) {
    client_list[index] = client_placeholder;
  }

  // Enable buttons
  sl_simple_button_enable(&sl_button_btn0);
#ifndef SINGLE_BUTTON
  sl_simple_button_enable(&sl_button_btn1);
#endif // SINGLE_BUTTON
  // Wait
  sl_sleeptimer_delay_millisecond(1);
  // Enable button presses
  app_button_press_enable();

  uint32_t status;
  /* Enable the memory lcd */
  status = sl_board_enable_display();
  EFM_ASSERT(status == SL_STATUS_OK);

  /* Initialize the DMD support for memory lcd display */
  status = DMD_init(0);
  EFM_ASSERT(status == DMD_OK);

  /* Initialize the glib context */
  status = GLIB_contextInit(&glibContext);
  EFM_ASSERT(status == GLIB_OK);

  glibContext.backgroundColor = White;
  glibContext.foregroundColor = Black;

  /* Fill lcd with background color */
  GLIB_clear(&glibContext);

  /* Use Narrow font */
  GLIB_setFont(&glibContext, (GLIB_Font_t *) &GLIB_FontNarrow6x8);

  DMD_updateDisplay();
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

  int rec = -1;
  while((rec = getchar()) != -1) {
    if(rec == 0xAA) {
      GLIB_clear(&glibContext);
      row = 0;
      column = 0;
      data_id = 0;
      return;
    }

    data[data_id] = rec;

    for(uint8_t bit = 0; bit < 8; bit++) {
      if(!(data[data_id] & (1 << bit))) GLIB_drawPixel(&glibContext, column, row);
      column++;
    }

    data_id++;

    if(column == 128) {
      column = 0;
      row++;
    }
    if(row == 128) {
      DMD_updateDisplay();
    }
  }
}

/*******************************************************************************
 * Handles button press and does a factory reset
 ******************************************************************************/
static bool handle_reset_conditions(void)
{
#ifdef SL_CATALOG_BTMESH_FACTORY_RESET_PRESENT
  // If PB0 is held down then do full factory reset
  if (sl_simple_button_get_state(&sl_button_btn0)
      == SL_SIMPLE_BUTTON_PRESSED) {
    // Full factory reset
    sl_btmesh_initiate_full_reset();
    return false;
  }

#if SL_SIMPLE_BUTTON_COUNT >= 2
  // If PB1 is held down then do node factory reset
  if (sl_simple_button_get_state(&sl_button_btn1)
      == SL_SIMPLE_BUTTON_PRESSED) {
    // Node factory reset
    sl_btmesh_initiate_node_reset();
    return false;
  }
#endif // SL_CATALOG_BTN1_PRESENT
#endif // SL_CATALOG_BTMESH_FACTORY_RESET_PRESENT
  return true;
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(struct sl_bt_msg *evt)
{
  //sl_status_t sc;
  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_system_boot_id:
      // Init the device as provisioner
      //sc = sl_btmesh_prov_init();
      //app_assert_status_f(sc, "sl_btmesh_prov_init failed\r\n");
      break;
    case sl_bt_evt_scanner_legacy_advertisement_report_id:
      /*app_log_debug("Bluetooth %s address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                    evt->data.evt_scanner_scan_report.address_type ? "static random" : "public device",
                    evt->data.evt_scanner_scan_report.address.addr[5],
                    evt->data.evt_scanner_scan_report.address.addr[4],
                    evt->data.evt_scanner_scan_report.address.addr[3],
                    evt->data.evt_scanner_scan_report.address.addr[2],
                    evt->data.evt_scanner_scan_report.address.addr[1],
                    evt->data.evt_scanner_scan_report.address.addr[0]);*/
      // We found a client device
      if(!strcmp(client_device_id, (char*)evt->data.evt_scanner_legacy_advertisement_report.data.data)) {
        app_log("Client device noticed\r\n");
        for(uint8_t index = 0; index < CLIENT_LIMIT; index++) {
          // We already have this device in list
          if(!memcmp(evt->data.evt_scanner_legacy_advertisement_report.address.addr, client_list[index].address.addr, sizeof(evt->data.evt_scanner_legacy_advertisement_report.address.addr))) break;
          // We found an empty space in the list
          if(!memcmp(empty_addr.addr, client_list[index].address.addr, sizeof(empty_addr.addr))) {
            // Add the newly-found device to our list
              client_list[index].address = evt->data.evt_scanner_legacy_advertisement_report.address;
            break;
          }
          app_log_warning("client_list full\r\n");
        }
      }
      break;
    case sl_bt_evt_advertiser_timeout_id:
      // Ignore advertiser timeout events
      break;
    default:
      app_log_debug("unhandled evt: %8.8x class %2.2x method %2.2x\r\n", (unsigned int)SL_BT_MSG_ID(evt->header),
                                                                         (unsigned int)((SL_BT_MSG_ID(evt->header) >> 16) & 0xFF),
                                                                         (unsigned int)((SL_BT_MSG_ID(evt->header) >> 24) & 0xFF) );
      break;
  }
}

/**************************************************************************//**
 * Bluetooth Mesh stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth Mesh stack.
 *****************************************************************************/

uint32_t handle_bind_1400;
uint32_t handle_bind_2000;
uint32_t handle_sub_1400;
uint32_t handle_sub_2000;
uint16_t handle_send_blob_id;

void sl_btmesh_on_event(sl_btmesh_msg_t *evt)
{
  sl_status_t sc;
  uint16_t result;
  uint32_t handle;

  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_btmesh_evt_prov_initialized_id:
      // Create the network
      sc = sl_btmesh_prov_create_network(network_id, 16, fixed_netkey);
      if(sc != SL_STATUS_OK) {
        /* Something went wrong */
        app_log("sl_btmesh_prov_create_network: failed 0x%.2lx\r\n", sc);
      } else {
        app_log("Success, netkey id = %x\r\n", network_id);
      }

      app_log("networks: 0x%x\r\naddress: 0x%x\r\niv_index: 0x%lx\r\n",
              evt->data.evt_prov_initialized.networks,
              evt->data.evt_prov_initialized.address,
              evt->data.evt_prov_initialized.iv_index);

      uint8_t opcodes[] = {0};
      sc = sl_btmesh_vendor_model_init(vendor_model_elem_index,
                                       0x1000,
                                       0x2001,
                                       1,
                                       1,
                                       opcodes);
      if(sc != SL_STATUS_OK) {
        /* Something went wrong */
        app_log("sl_btmesh_vendor_model_init: failed 0x%.2lx\r\n", sc);
      } else {
        app_log("Success, sl_btmesh_vendor_model_init: 0x%x\r\n", 0x2001);
      }

      size_t max_application_key_size = 16;
      size_t application_key_len = 16;
      uint8_t application_key[16];
      sc = sl_btmesh_prov_create_appkey(network_id, appkey_index, 16, fixed_appkey, max_application_key_size, &application_key_len, application_key);

      if(sc != SL_STATUS_OK) {
        /* Something went wrong */
        app_log("sl_btmesh_prov_create_appkey: failed 0x%.2lx\r\n", sc);
      } else {
        app_log("Success, appkey id = %x\r\n", appkey_index);
      }

      sc = sl_btmesh_test_bind_local_model_app(blob_client_model_elem_index,
                                               appkey_index,
                                               0xFFFF,
                                               0x1401);
      if(sc != SL_STATUS_OK) {
        /* Something went wrong */
        app_log("sl_btmesh_test_bind_local_model_app: failed 0x%.2lx\r\n", sc);
      } else {
        app_log("Success, sl_btmesh_test_bind_local_model_app: 0x%x\r\n", 0x1401);
      }

      sc = sl_btmesh_test_set_local_model_pub(blob_client_model_elem_index,
                                              appkey_index,
                                              0xFFFF,
                                              0x1401,
                                              GRP_ADDR_BLOB,
                                              3,
                                              0,
                                              0,
                                              0);
      if(sc != SL_STATUS_OK) {
        /* Something went wrong */
        app_log("sl_btmesh_test_set_local_model_pub: failed 0x%.2lx\r\n", sc);
      } else {
        app_log("Success, sl_btmesh_test_set_local_model_pub: 0x%x\r\n", 0x1401);
      }

      sc = sl_btmesh_test_bind_local_model_app(vendor_model_elem_index,
                                               appkey_index,
                                               0x1000,
                                               0x2001);
      if(sc != SL_STATUS_OK) {
        /* Something went wrong */
        app_log("sl_btmesh_test_bind_local_model_app: failed 0x%.2lx\r\n", sc);
      } else {
        app_log("Success, sl_btmesh_test_bind_local_model_app: 0x%x\r\n", 0x2001);
      }

      sc = sl_btmesh_test_set_local_model_pub(vendor_model_elem_index,
                                              appkey_index,
                                              0x1000,
                                              0x2001,
                                              GRP_ADDR_BLOB_ID,
                                              3,
                                              0,
                                              0,
                                              0);
      if(sc != SL_STATUS_OK) {
        /* Something went wrong */
        app_log("sl_btmesh_test_set_local_model_pub: failed 0x%.2lx\r\n", sc);
      } else {
        app_log("Success, sl_btmesh_test_set_local_model_pub: 0x%x\r\n", 0x2001);
      }

      /* Scan for unprovisioned beacons */
      sc = sl_btmesh_prov_scan_unprov_beacons();
      break;
    case sl_btmesh_evt_prov_initialization_failed_id:
      app_log("sl_btmesh_evt_prov_initialization_failed_id: 0x%x ", evt->data.evt_prov_initialization_failed.result);
      break;
    case sl_btmesh_evt_prov_unprov_beacon_id:
      // Unprovisioned device found
      /*app_log_debug("URI hash: 0x%lx\r\n",evt->data.evt_prov_unprov_beacon.uri_hash);
      app_log_debug("bearer: 0x%x\r\n",evt->data.evt_prov_unprov_beacon.bearer);
      app_log_debug("address: 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x\r\n",
                    evt->data.evt_prov_unprov_beacon.address.addr[0],
                    evt->data.evt_prov_unprov_beacon.address.addr[1],
                    evt->data.evt_prov_unprov_beacon.address.addr[2],
                    evt->data.evt_prov_unprov_beacon.address.addr[3],
                    evt->data.evt_prov_unprov_beacon.address.addr[4],
                    evt->data.evt_prov_unprov_beacon.address.addr[5]);*/

      // Check all the prerecorded device addresses
      for(uint8_t index = 0; index < CLIENT_LIMIT; index++) {
        // If we found the address in the server_list, provision the device directly
        if(!memcmp(evt->data.evt_prov_unprov_beacon.address.addr, client_list[index].address.addr, sizeof(evt->data.evt_prov_unprov_beacon.address.addr))) {
          if(client_list[index].is_provisioned) break;
          app_log("Provisioning a Client device\r\n");
          client_list[index].uuid = evt->data.evt_prov_unprov_beacon.uuid;
          sc = sl_btmesh_prov_create_provisioning_session(network_id, evt->data.evt_prov_unprov_beacon.uuid, 0);
          app_assert_status_f(sc, "sl_btmesh_prov_create_provisioning_session failed\r\n");
          sc = sl_btmesh_prov_provision_adv_device(evt->data.evt_prov_unprov_beacon.uuid);
          app_assert_status_f(sc, "sl_btmesh_prov_provision_adv_device failed\r\n");
          client_list[index].is_provisioned = 0x1;
          break;
        }
      }
      break;
    case sl_btmesh_evt_prov_device_provisioned_id:
      app_log_debug("address: %x\r\n", evt->data.evt_prov_device_provisioned.address);
      provisionee_addr = evt->data.evt_prov_device_provisioned.address;
      provisionee_uuid = evt->data.evt_prov_device_provisioned.uuid;
      // Roll through the server_list, if provisioned device is found by uuid, save the primary element address (where the Remote Provisioning Server Model should be)
      for(uint8_t index = 0; index < CLIENT_LIMIT; index++) {
        if(!memcmp(provisionee_uuid.data, client_list[index].uuid.data, sizeof(evt->data.evt_prov_unprov_beacon.uuid.data))) {
          client_list[index].primary_element_address = provisionee_addr;
          break;
        }
      }
      app_log("New client device provisioned: %x\r\n", provisionee_addr);

      sc = sl_btmesh_config_client_add_appkey(enc_netkey_index, provisionee_addr, appkey_index, network_id, &handle);
      if (sc == SL_STATUS_OK) {
        app_log("Deploying appkey to node 0x%4.4x\r\n", provisionee_addr);
      } else {
        app_log("Appkey deployment failed. addr %x, error: %lx\r\n", provisionee_addr, sc);
      }
      break;
    case sl_btmesh_evt_config_client_appkey_status_id:
      result = evt->data.evt_config_client_appkey_status.result;
      if(result == SL_STATUS_OK) {
        app_log(" appkey added\r\n");

        sc = sl_btmesh_config_client_bind_model(enc_netkey_index, provisionee_addr, vendor_model_elem_index, 0x1000, 0x2000, appkey_index, &handle_bind_2000);
        if (sc == SL_STATUS_OK) {
          app_log("sl_btmesh_config_client_bind_model to node 0x%4.4x\r\n", provisionee_addr);
        } else {
          app_log("sl_btmesh_config_client_bind_model addr %x, error: %lx\r\n", provisionee_addr, sc);
        }
      }
      break;
    case sl_btmesh_evt_config_client_binding_status_id:
      result = evt->data.evt_config_client_binding_status.result;
      if(result == SL_STATUS_OK) {
        app_log(" bind complete\r\n");

        if(evt->data.evt_config_client_binding_status.handle == handle_bind_2000) {
          sc = sl_btmesh_config_client_add_model_sub(enc_netkey_index, provisionee_addr, vendor_model_elem_index, 0x1000, 0x2000, GRP_ADDR_BLOB_ID, &handle_sub_2000);
          if (sc == SL_STATUS_OK) {
            app_log(" waiting sub ack\r\n");
          } else {
            app_log("sl_btmesh_config_client_add_model_sub addr %x, error: %lx\r\n", provisionee_addr, sc);
          }
        } else {
          sc = sl_btmesh_config_client_add_model_sub(enc_netkey_index, provisionee_addr, blob_server_model_elem_index, 0xFFFF, 0x1400, GRP_ADDR_BLOB, &handle_sub_1400);
          if (sc == SL_STATUS_OK) {
            app_log(" waiting sub ack\r\n");
          } else {
            app_log("sl_btmesh_config_client_add_model_sub addr %x, error: %lx\r\n", provisionee_addr, sc);
          }
        }
      } else {
        app_log(" bind error %x\r\n", result);
      }
      break;
    case sl_btmesh_evt_config_client_model_sub_status_id:
      result = evt->data.evt_config_client_model_sub_status.result;
      if(result == SL_STATUS_OK) {
        if(evt->data.evt_config_client_binding_status.handle == handle_sub_2000) {
          app_log(" sub add OK\r\n");
          sc = sl_btmesh_config_client_bind_model(network_id, provisionee_addr, blob_server_model_elem_index, 0xFFFF, 0x1400, appkey_index, &handle_bind_1400);
          if (sc == SL_STATUS_OK) {
            app_log("sl_btmesh_config_client_bind_model to node 0x%4.4x\r\n", provisionee_addr);
          } else {
            app_log("sl_btmesh_config_client_bind_model addr %x, error: %lx\r\n", provisionee_addr, sc);
          }
        } else {
          app_log(" sub add OK\r\n");
          servers[servers_size] = provisionee_addr & 0x00FF;
          servers_size++;
          servers[servers_size] = provisionee_addr >> 8;
          servers_size++;
        }
      } else {
        app_log(" sub error %x\r\n", result);
      }
      break;
    // Provisioning failed
    case sl_btmesh_evt_prov_provisioning_failed_id:
      app_log_debug("%x\r\n%x\r\n",
                    evt->data.evt_prov_provisioning_failed.reason,
                    evt->data.evt_prov_provisioning_failed.uuid.data[0]);
      break;
    case sl_btmesh_evt_vendor_model_send_complete_id:
      result = evt->data.evt_vendor_model_send_complete.result;
      if(result == SL_STATUS_OK && handle_send_blob_id == evt->data.evt_vendor_model_send_complete.handle) {
        send_image();
      }
      break;
    default:
      app_log_debug("unhandled evt: %8.8x class %2.2x method %2.2x\r\n", (unsigned int)SL_BT_MSG_ID(evt->header),
                                                                         (unsigned int)((SL_BT_MSG_ID(evt->header) >> 16) & 0xFF),
                                                                         (unsigned int)((SL_BT_MSG_ID(evt->header) >> 24) & 0xFF) );
      break;
  }
}

/***************************************************************************//**
 * Button press Callbacks
 ******************************************************************************/
void app_button_press_cb(uint8_t button, uint8_t duration)
{
  // Selecting action by duration
  switch (duration) {
    case APP_BUTTON_PRESS_DURATION_LONG:
      // Handling of button press greater than 1s and less than 5s
      if (button == BUTTON_0) {
        if(servers_size > 0) send_blob_id();
      }
      break;
    default:
      break;
  }
}

static void handle_blob_transfer_client_notification(const sl_btmesh_blob_transfer_client_notification_t *const notification) {
  app_log("Progress %ld\r\n", notification->params.progress.confirmed_tx_bytes);
}

void send_image() {

  sl_status_t sc;

  sc = sl_btmesh_blob_transfer_client_setup(blob_client_model_elem_index,
                                            blob_id,
                                            2048,
                                            appkey_index,
                                            5,
                                            10,
                                            GRP_ADDR_BLOB,
                                            virtual_address,
                                            1,
                                            servers_size,
                                            servers);
  app_assert_status_f(sc, "sl_btmesh_blob_transfer_client_setup failed");

  app_log("sl_btmesh_blob_transfer_client_setup done\r\n");

  sc = sl_btmesh_blob_transfer_client_setup_data_provider_array(blob_client_model_elem_index,
                                                                data,
                                                                2048);
  app_assert_status_f(sc, "sl_btmesh_blob_transfer_client_setup_data_provider_array failed");

  app_log("sl_btmesh_blob_transfer_client_setup_data_provider_array done\r\n");

  sc = sl_btmesh_blob_transfer_client_start(blob_client_model_elem_index,
                                            sl_btmesh_mbt_client_mbt_transfer_mode_both,
                                            handle_blob_transfer_client_notification);
  app_assert_status_f(sc, "sl_btmesh_blob_transfer_client_start failed");
}

void send_blob_id() {

  sl_status_t sc;

  blob_id.data[0] = blob_id.data[0] + 1;
  sc = sl_btmesh_vendor_model_send_tracked(GRP_ADDR_BLOB_ID,
                                           0,
                                           appkey_index,
                                           vendor_model_elem_index,
                                           0x1000,
                                           0x2001,
                                           0,
                                           0,
                                           0,
                                           1,
                                           1,
                                           &blob_id.data[0],
                                           &handle_send_blob_id);
  if (sc == SL_STATUS_OK) {
    app_log("sl_btmesh_vendor_model_send to node 0x%4.4x\r\n", provisionee_addr);
  } else {
    app_log("sl_btmesh_vendor_model_send addr %x, error: %lx\r\n", provisionee_addr, sc);
  }
}