/***************************************************************************//**
 * @file
 * @brief Core application logic.
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

#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"

#ifdef SL_CATALOG_BTMESH_WSTK_LCD_PRESENT
#include "sl_btmesh_wstk_lcd.h"
#endif // SL_CATALOG_BTMESH_WSTK_LCD_PRESENT

/***************************************************************************//**
 * @addtogroup Application
 * @{
 ******************************************************************************/

#define BLE_MESH_UUID_LEN_BYTE (16)
#define BLE_ADDR_LEN_BYTE (6)

#define MAX_NUM_BTMESH_DEV (10)
#define MAX_NUM_ROUTES ((MAX_NUM_BTMESH_DEV+1)*MAX_NUM_BTMESH_DEV)

node_registry_entry_t node_registry[MAX_NUM_BTMESH_DEV];
route_registry_entry_t route_registry[MAX_NUM_ROUTES];

static const uint8_t fixed_netkey[16] = {0x23, 0x98, 0xdf, 0xa5, 0x09, 0x3e, 0x74, 0xbb, 0xc2, 0x45, 0x1f, 0xae, 0xea, 0xd7, 0x67, 0xcd};
static const uint8_t fixed_appkey[16] = {0x16, 0x39, 0x38, 0x03, 0x9b, 0x8d, 0x8a, 0x20, 0x81, 0x60, 0xa7, 0x93, 0x33, 0x3d, 0x03, 0x61};

static uint8_t network_id = 0x0;
static uint16_t appkey_index = 0x0;

/// Length of the display name buffer
#define NAME_BUF_LEN  20

#ifdef SL_CATALOG_BTMESH_WSTK_LCD_PRESENT
#define lcd_print(...) sl_btmesh_LCD_write(__VA_ARGS__)
#else
#define lcd_print(...)
#endif // SL_CATALOG_BTMESH_WSTK_LCD_PRESENT

enum {
  UNPROVISIONED_DEVICE,
  PROVISION_DEVICE,
  NODE_PROVISIONED,
  HEARTBEAT_REPORT,
  REMAP_NETWORK
};

char name[NAME_BUF_LEN];
int8_t proxy_connection = -1;
bool notification_enabled = false;
static uint16_t provisioner_address = 0x0;
sl_sleeptimer_timer_handle_t sleeptimer_timer_handle;

/*******************************************************************************
 * Application Early Init
 ******************************************************************************/
void app_init_early(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once before the OS is initialized if RTOS is used.       //
  // This function precedes permanent memory allocations.                    //
  /////////////////////////////////////////////////////////////////////////////
}

/**************************************************************************//**
 * Application Init.
 *****************************************************************************/
void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
  app_log("Boot\r\n");
}

/**************************************************************************//**
 * Application Process Action.
 *****************************************************************************/
void app_process_action(void)
{
  if (app_is_process_required()) {
    /////////////////////////////////////////////////////////////////////////////
    // Put your additional application code here!                              //
    // This will run each time app_proceed() is called.                        //
    // Do not call blocking functions from here!                               //
    /////////////////////////////////////////////////////////////////////////////
  }
}

/* Initialize the stack */
void init_node_registry()
{
  /* Reset working structure */
  memset(&node_registry, 0x00, (sizeof(node_registry_entry_t) * MAX_NUM_BTMESH_DEV));

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
  sl_status_t sc;

  // Create unique device name using the last two bytes of the Bluetooth address
  snprintf(name, NAME_BUF_LEN, "provisioner %02x:%02x",
           addr->addr[1],
           addr->addr[0]);

  app_log("Device name: '%s'\r\n", name);

  sc = sl_bt_gatt_server_write_attribute_value(gattdb_device_name,
                                               0,
                                               strlen(name),
                                               (uint8_t *)name);
  if(sc) {
    app_log("sl_bt_gatt_server_write_attribute_value() failed, code %lx\r\n", sc);
  }
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
    init_node_registry();
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
  sl_status_t sc;
  uint32_t handle;

  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_system_boot_id:
      handle_boot_event();
      break;
    case sl_bt_evt_connection_opened_id:
      proxy_connection = evt->data.evt_connection_opened.connection;
      app_log("Connection opened\r\n");
      break;
    case sl_bt_evt_connection_closed_id:
      proxy_connection = -1;
      notification_enabled = false;
      app_log("Connection closed\r\n");
      break;
    case sl_bt_evt_gatt_server_user_write_request_id:
      //app_log("sl_bt_evt_gatt_server_user_write_request_id\r\n");
      if(evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_mesh_custom_data_in) {
        switch(evt->data.evt_gatt_server_user_write_request.value.data[0]) {
          case PROVISION_DEVICE: {

            uuid_128 uuid;
            memcpy(uuid.data, &evt->data.evt_gatt_server_user_write_request.value.data[1], 16);
            /* provisioning using ADV bearer (this is the default) */
            sl_btmesh_prov_create_provisioning_session(network_id, uuid, 0);
            sc = sl_btmesh_prov_provision_adv_device(uuid);
            if(sc == SL_STATUS_OK) {
              app_log("Provisioning success: ");
            } else {
              app_log("Provisioning fail %lX: ", sc);
            }

          } break;
          case REMAP_NETWORK: {
            start_full_mapping();
          } break;
        }
      }
      break;
    case sl_bt_evt_scanner_legacy_advertisement_report_id:
      break;
    case sl_bt_evt_gatt_server_characteristic_status_id:

      if (evt->data.evt_gatt_server_characteristic_status.status_flags == sl_bt_gatt_server_client_config) {
        notification_enabled = evt->data.evt_gatt_server_characteristic_status.client_config_flags == sl_bt_gatt_server_notification;
      }

      // Send the Node infos, if notification is enabled
      if(notification_enabled) {
        for(uint8_t node_registry_id = 0; node_registry_id < MAX_NUM_BTMESH_DEV; node_registry_id++) {
          // Jump out at the end of the registry
          if(node_registry[node_registry_id].address == 0x0) break;

          struct __attribute__((packed)) { // 19 bytes
            uint8_t   type;
            uint16_t  address;
            uuid_128  uuid;
          } payload = {
            NODE_PROVISIONED,
            node_registry[node_registry_id].address,
            node_registry[node_registry_id].uuid
          };

          sc = sl_bt_gatt_server_send_notification(proxy_connection,
                                                    gattdb_mesh_custom_data_out,
                                                    sizeof(payload),
                                                    (uint8_t *)&payload);

          if(sc != SL_STATUS_OK) {
            /* Something went wrong */
            app_log("sl_bt_gatt_server_send_notification: failed 0x%.2lx\r\n", sc);
          } //else {
          //  app_log("Success, sl_bt_gatt_server_send_notification: 0x%.2lx\r\n", sc);
          //}
        }

        start_full_mapping();
      }

      break;
    case sl_bt_evt_system_external_signal_id:

      app_log("Success, sl_bt_evt_system_external_signal_id: %ld\r\n", evt->data.evt_system_external_signal.extsignals);
      uint16_t route_registry_id = (uint16_t)evt->data.evt_system_external_signal.extsignals;
      sc = sl_btmesh_config_client_get_heartbeat_sub(0,
                                                     route_registry[route_registry_id].target_address,
                                                     &handle);

      if(sc != SL_STATUS_OK) {
        /* Something went wrong */
        app_log("sl_btmesh_config_client_get_heartbeat_sub: failed 0x%.2lx\r\n", sc);
      } else {
        app_log("Success, sl_btmesh_config_client_get_heartbeat_sub: 0x%.2lx\r\n", sc);
      }

      //find_route(++route_registry_id);
      break;
    default:
      app_log("unhandled evt: %8.8x class %2.2x method %2.2x\r\n", (unsigned int)SL_BT_MSG_ID(evt->header),
                                                                   (unsigned int)((SL_BT_MSG_ID(evt->header) >> 16) & 0xFF),
                                                                   (unsigned int)((SL_BT_MSG_ID(evt->header) >> 24) & 0xFF));
      break;
  }
}

bool is_webbrowser_ready()
{
  return proxy_connection >= 0 && notification_enabled;
}

void sl_btmesh_on_event(sl_btmesh_msg_t *evt)
{
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

      sc = sl_btmesh_node_get_element_address(0,
                                              &provisioner_address);
      if(sc != SL_STATUS_OK) {
        /* Something went wrong */
        app_log("sl_btmesh_node_get_element_address: failed 0x%.2lx\r\n", sc);
      } else {
        app_log("Success, sl_btmesh_node_get_element_address: 0x%.2lx\r\n", sc);
      }

      sc = sl_btmesh_config_client_set_gatt_proxy(0,
                                                  provisioner_address,
                                                  1,
                                                  &handle);
      if(sc != SL_STATUS_OK) {
        /* Something went wrong */
        app_log("sl_btmesh_config_client_set_gatt_proxy: failed 0x%.2lx\r\n", sc);
      } else {
        app_log("Success, sl_btmesh_config_client_set_gatt_proxy: 0x%.2lx\r\n", sc);
      }

      // Write the Mesh Provisioner Server address into the Characteristic Value
      sc = sl_bt_gatt_server_write_attribute_value(gattdb_mesh_custom_data_out,
                                                   0,
                                                   sizeof(provisioner_address),
                                                   (uint8_t *)&provisioner_address);

      if(sc != SL_STATUS_OK) {
        /* Something went wrong */
        app_log("sl_bt_gatt_server_write_attribute_value: failed 0x%.2lx\r\n", sc);
      } else {
        app_log("Success, sl_bt_gatt_server_write_attribute_value: 0x%.2lx\r\n", sc);
      }

      app_log("\r\nPlease, connect with the included Web Application to: '%s'\r\n\r\n", name);

      /* Scan for unprovisioned beacons */
      sc = sl_btmesh_prov_scan_unprov_beacons();
    }
    break;
    case sl_btmesh_evt_prov_initialization_failed_id:
      app_log("sl_btmesh_evt_prov_initialization_failed_id failed: 0x%x ", evt->data.evt_prov_initialization_failed.result);
    break;
    case sl_btmesh_evt_prov_unprov_beacon_id:
      /* PB-ADV only */
      if(0 == evt->data.evt_prov_unprov_beacon.bearer) {

        if(is_webbrowser_ready()) {

          struct {
            uint8_t   type;
            bd_addr   address;
            uuid_128  uuid;
            int8_t    rssi;
          } payload = {
            UNPROVISIONED_DEVICE,
            evt->data.evt_prov_unprov_beacon.address,
            evt->data.evt_prov_unprov_beacon.uuid,
            evt->data.evt_prov_unprov_beacon.rssi
          };

          sc = sl_bt_gatt_server_send_notification(proxy_connection,
                                                   gattdb_mesh_custom_data_out,
                                                   sizeof(payload),
                                                   (uint8_t *)&payload);

          if(sc != SL_STATUS_OK) {
            /* Something went wrong */
            app_log("sl_bt_gatt_server_send_notification: failed 0x%.2lx\r\n", sc);
          } //else {
          //  app_log("Success, sl_bt_gatt_server_send_notification: 0x%.2lx\r\n", sc);
          //}
        }
      }

      break;
    /* Provisioning */
    case sl_btmesh_evt_prov_provisioning_failed_id:
      app_log("Provisioning failed\r\n");
      break;
    case sl_btmesh_evt_prov_device_provisioned_id:

      uint8_t node_registry_id = 0;

      for(node_registry_id = 0; node_registry_id < MAX_NUM_BTMESH_DEV; node_registry_id++) {
        // The check index is empty
        if(node_registry[node_registry_id].address == 0) {
            node_registry[node_registry_id].address = evt->data.evt_prov_device_provisioned.address;
            memcpy(&node_registry[node_registry_id].uuid.data[0], &evt->data.evt_prov_device_provisioned.uuid.data[0], BLE_MESH_UUID_LEN_BYTE);
            break;
        }
        // The checked index has the address of the current device
        if(node_registry[node_registry_id].address == evt->data.evt_prov_device_provisioned.address) {
            app_log("Node already provisioned before: 0x%x\r\n", evt->data.evt_prov_device_provisioned.address);
            break;
        }
      }

      if(node_registry_id == MAX_NUM_BTMESH_DEV) {
        app_log("Node registry size limit exceeded: %d\r\n", MAX_NUM_BTMESH_DEV);
        return;
      }

      if(is_webbrowser_ready()) {

        struct __attribute__((packed)) { // 19 bytes
          uint8_t   type;
          uint16_t  address;
          uuid_128  uuid;
        } payload = {
          NODE_PROVISIONED,
          evt->data.evt_prov_device_provisioned.address,
          evt->data.evt_prov_device_provisioned.uuid
        };

        sc = sl_bt_gatt_server_send_notification(proxy_connection,
                                                  gattdb_mesh_custom_data_out,
                                                  sizeof(payload),
                                                  (uint8_t *)&payload);

        if(sc != SL_STATUS_OK) {
          /* Something went wrong */
          app_log("sl_bt_gatt_server_send_notification: failed 0x%.2lx\r\n", sc);
        } //else {
        //  app_log("Success, sl_bt_gatt_server_send_notification: 0x%.2lx\r\n", sc);
        //}
      }

      memset(&route_registry, 0x00, (sizeof(route_registry_entry_t) * MAX_NUM_ROUTES));

      uint16_t route_registry_id = 0;

      // Add the Provisioner check
      route_registry[route_registry_id].source_address = evt->data.evt_prov_device_provisioned.address;
      route_registry[route_registry_id].target_address = provisioner_address;
      route_registry[route_registry_id].checked = false;
      route_registry_id++;

      for(uint8_t node_registry_id = 0; node_registry_id < MAX_NUM_BTMESH_DEV; node_registry_id++) {
        // Jump out at the end of the registry
        if(node_registry[node_registry_id].address == 0x0) break;
        // Jump out if it would be a self-loop route
        if(node_registry[node_registry_id].address == evt->data.evt_prov_device_provisioned.address) break;

        route_registry[route_registry_id].source_address = node_registry[node_registry_id].address;
        route_registry[route_registry_id].target_address = evt->data.evt_prov_device_provisioned.address;
        route_registry[route_registry_id].checked = false;
        route_registry_id++;
      }

      find_route(0);
      break;

    case sl_btmesh_evt_config_client_heartbeat_pub_status_id:

      app_log("sl_btmesh_evt_config_client_heartbeat_pub_status_id: %d, destination_address: %x, netkey_index: %x, count_log: %x, period_log: %x, ttl: %x, features: %x\r\n",
              evt->data.evt_config_client_heartbeat_pub_status.result,
              evt->data.evt_config_client_heartbeat_pub_status.destination_address,
              evt->data.evt_config_client_heartbeat_pub_status.netkey_index,
              evt->data.evt_config_client_heartbeat_pub_status.count_log,
              evt->data.evt_config_client_heartbeat_pub_status.period_log,
              evt->data.evt_config_client_heartbeat_pub_status.ttl,
              evt->data.evt_config_client_heartbeat_pub_status.features);
      break;
    case sl_btmesh_evt_config_client_heartbeat_sub_status_id:

      app_log("sl_btmesh_evt_config_client_heartbeat_sub_status_id: %d, source_address: %x, destination_address: %x, count_log: %d, min_hops: %d, max_hops: %d\r\n",
              evt->data.evt_config_client_heartbeat_sub_status.result,
              evt->data.evt_config_client_heartbeat_sub_status.source_address,
              evt->data.evt_config_client_heartbeat_sub_status.destination_address,
              evt->data.evt_config_client_heartbeat_sub_status.count_log,
              evt->data.evt_config_client_heartbeat_sub_status.min_hops,
              evt->data.evt_config_client_heartbeat_sub_status.max_hops);

      if(evt->data.evt_config_client_heartbeat_sub_status.count_log > 0 &&
         evt->data.evt_config_client_heartbeat_sub_status.result == SL_STATUS_OK) {


        uint16_t next_route_registry_id = 0;
        for(uint16_t route_registry_id = 0; route_registry_id < MAX_NUM_ROUTES; route_registry_id++) {
          // Skip, if we already checked this route
          if(route_registry[route_registry_id].checked == true) continue;
          // If the route is same in both directions, mark both of them checked
          if((route_registry[route_registry_id].source_address == evt->data.evt_config_client_heartbeat_sub_status.source_address &&
            route_registry[route_registry_id].target_address == evt->data.evt_config_client_heartbeat_sub_status.destination_address) ||
            (route_registry[route_registry_id].source_address == evt->data.evt_config_client_heartbeat_sub_status.destination_address &&
            route_registry[route_registry_id].target_address == evt->data.evt_config_client_heartbeat_sub_status.source_address)) {
            route_registry[route_registry_id].checked = true;
            continue;
          }
          // If we did not check this route yet, save it to be checked next (should not be the first route)
          if(route_registry[route_registry_id].checked == false && next_route_registry_id == 0) {
            next_route_registry_id = route_registry_id;
            continue;
          }
        }

        if(is_webbrowser_ready()) {

          struct __attribute__((packed)) { //
            uint8_t   type;
            uint16_t  source_address;
            uint16_t  destination_address;
            uint8_t   min_hops;
            uint8_t   max_hops;
          } payload = {
            HEARTBEAT_REPORT,
            evt->data.evt_config_client_heartbeat_sub_status.source_address,
            evt->data.evt_config_client_heartbeat_sub_status.destination_address,
            evt->data.evt_config_client_heartbeat_sub_status.min_hops,
            evt->data.evt_config_client_heartbeat_sub_status.max_hops
          };

          sc = sl_bt_gatt_server_send_notification(proxy_connection,
                                                   gattdb_mesh_custom_data_out,
                                                   sizeof(payload),
                                                   (uint8_t *)&payload);

          if(sc != SL_STATUS_OK) {
            /* Something went wrong */
            app_log("sl_bt_gatt_server_send_notification: failed 0x%.2lx\r\n", sc);
          } //else {
          //  app_log("Success, sl_bt_gatt_server_send_notification: 0x%.2lx\r\n", sc);
          //}
        }

        find_route(next_route_registry_id);
      }

      break;
    default:
      app_log("unhandled evt: %8.8x class %2.2x method %2.2x\r\n", (unsigned int)SL_BT_MSG_ID(evt->header),
                                                                   (unsigned int)((SL_BT_MSG_ID(evt->header) >> 16) & 0xFF),
                                                                   (unsigned int)((SL_BT_MSG_ID(evt->header) >> 24) & 0xFF));
      break;
  }

  return;
}

void sleeptimer_callback(sl_sleeptimer_timer_handle_t *handle, void *data)
{
  sl_sleeptimer_stop_timer(handle);
  uint16_t *route_registry_id = (uint16_t *)data;
  sl_bt_external_signal(*route_registry_id);
}

void start_full_mapping()
{
  memset(&route_registry, 0x00, (sizeof(route_registry_entry_t) * MAX_NUM_ROUTES));

  uint16_t route_registry_id = 0;

  for(uint8_t node_registry_id_outer = 0; node_registry_id_outer < MAX_NUM_BTMESH_DEV; node_registry_id_outer++) {
    // Jump out at the end of the registry
    if(node_registry[node_registry_id_outer].address == 0x0) break;

    // Add the Provisioner check
    route_registry[route_registry_id].source_address = node_registry[node_registry_id_outer].address;
    route_registry[route_registry_id].target_address = provisioner_address;
    route_registry[route_registry_id].checked = false;
    route_registry_id++;

    for(uint8_t node_registry_id_inner = 0; node_registry_id_inner < MAX_NUM_BTMESH_DEV; node_registry_id_inner++) {
      // Jump out at the end of the registry
      if(node_registry[node_registry_id_inner].address == 0x0) break;
      // Jump out if it would be a self-loop route
      if(node_registry[node_registry_id_outer].address == node_registry[node_registry_id_inner].address) break;

      route_registry[route_registry_id].source_address = node_registry[node_registry_id_outer].address;
      route_registry[route_registry_id].target_address = node_registry[node_registry_id_inner].address;
      route_registry[route_registry_id].checked = false;
      route_registry_id++;
    }
  }

  find_route(0);
}

void find_route(uint16_t route_registry_id)
{
  sl_status_t sc;
  uint32_t handle;
  static uint16_t callback_data;
  callback_data = route_registry_id;

  if(route_registry_id >= MAX_NUM_ROUTES ||
     route_registry[route_registry_id].checked == true ||
     route_registry[route_registry_id].source_address == 0x0 ||
     route_registry[route_registry_id].target_address == 0x0) return;

  sc = sl_btmesh_config_client_set_heartbeat_sub(0,
                                                 route_registry[route_registry_id].target_address,
                                                 route_registry[route_registry_id].source_address,
                                                 route_registry[route_registry_id].target_address,
                                                 0x05,
                                                 &handle);

  if(sc != SL_STATUS_OK) {
    /* Something went wrong */
    app_log("sl_btmesh_config_client_set_heartbeat_sub: failed 0x%.2lx\r\n", sc);
  } else {
    app_log("Success, sl_btmesh_config_client_set_heartbeat_sub: 0x%x\r\n", route_registry[route_registry_id].target_address);
  }

  sc = sl_btmesh_config_client_set_heartbeat_pub(0,
                                                 route_registry[route_registry_id].source_address,
                                                 route_registry[route_registry_id].target_address,
                                                 0,
                                                 0x02,
                                                 0x01,
                                                 5,
                                                 0x0,
                                                 &handle);

  if(sc != SL_STATUS_OK) {
    /* Something went wrong */
    app_log("sl_btmesh_config_client_set_heartbeat_pub: failed 0x%.2lx\r\n", sc);
  } else {
    app_log("Success, sl_btmesh_config_client_set_heartbeat_pub: 0x%x\r\n", route_registry[route_registry_id].source_address);
  }

  sc = sl_sleeptimer_start_periodic_timer_ms(&sleeptimer_timer_handle,
                                             5000,
                                             sleeptimer_callback,
                                             (void *)&callback_data,
                                             0,
                                             0);

  if(sc != SL_STATUS_OK) {
    /* Something went wrong */
    app_log("sl_sleeptimer_start_periodic_timer_ms: failed 0x%.2lx\r\n", sc);
  } else {
    app_log("Success, sl_sleeptimer_start_periodic_timer_ms: 0x%.2lx\r\n", sc);
  }
}