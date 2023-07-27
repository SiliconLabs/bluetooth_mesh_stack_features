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
#include "em_common.h"
#include "app_assert.h"
#include "sl_status.h"
#include "app.h"

#include "sl_btmesh_api.h"
#include "sl_bt_api.h"

#include "app_button_press.h"
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"

/// Advertising Provisioning Bearer
#define PB_ADV                         0x1
/// GATT Provisioning Bearer
#define PB_GATT                        0x2

#define BUTTON_0                       0

#define SERVER_LIMIT                   4
#define REMOTE_LIMIT                   4

static const uint8_t fixed_netkey[16] = {0x23, 0x98, 0xdf, 0xa5, 0x09, 0x3e, 0x74, 0xbb, 0xc2, 0x45, 0x1f, 0xae, 0xea, 0xd7, 0x67, 0xcd};
static const uint8_t network_id = 0x0;
static const uint16_t enc_netkey_index = 0x0;

typedef struct {
  uuid_128 uuid;
  uint8_t is_provisioned;
} remote_device_t;

typedef struct {
  uuid_128 uuid;
  bd_addr address;
  uint8_t is_provisioned;
  uint16_t primary_element_address;
  remote_device_t remote_devices[REMOTE_LIMIT];
} server_device_t;

static const char remote_server_id[] = "RemoteServer";
static const uuid_128 empty_uuid = {{0x00}};
static const bd_addr empty_addr = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
static const remote_device_t remote_placeholder = {empty_uuid, 0x0};
static const server_device_t server_placeholder = {empty_uuid, empty_addr, 0x0, 0x0, {}};
server_device_t server_list[SERVER_LIMIT];

uuid_128 remote_uuid_to_provision = empty_uuid;

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

  // Fill up the server list
  for(uint8_t index = 0; index < SERVER_LIMIT; index++) {
    server_list[index] = server_placeholder;
    for(uint8_t index2 = 0; index2 < REMOTE_LIMIT; index2++) {
      server_list[index].remote_devices[index2] = remote_placeholder;
    }
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
      // Init the device as provisioner
      sc = sl_btmesh_prov_init();
      // Needed because of self-initializing component
      if(sc !=  SL_STATUS_INVALID_STATE) {
        app_assert_status_f(sc, "sl_btmesh_prov_init failed\r\n");
      }
      break;
    case sl_bt_evt_scanner_scan_report_id:
      /*app_log_debug("Bluetooth %s address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                    evt->data.evt_scanner_scan_report.address_type ? "static random" : "public device",
                    evt->data.evt_scanner_scan_report.address.addr[5],
                    evt->data.evt_scanner_scan_report.address.addr[4],
                    evt->data.evt_scanner_scan_report.address.addr[3],
                    evt->data.evt_scanner_scan_report.address.addr[2],
                    evt->data.evt_scanner_scan_report.address.addr[1],
                    evt->data.evt_scanner_scan_report.address.addr[0]);*/
      // We found a remote server
      if(!strcmp(remote_server_id, (char*)evt->data.evt_scanner_scan_report.data.data)) {
        app_log("Remote Provisioning Server device noticed\r\n");
        for(uint8_t index = 0; index < SERVER_LIMIT; index++) {
          // We already have this server in list
          if(!memcmp(evt->data.evt_scanner_scan_report.address.addr, server_list[index].address.addr, sizeof(evt->data.evt_scanner_scan_report.address.addr))) break;
          // We found an empty space in the list
          if(!memcmp(empty_addr.addr, server_list[index].address.addr, sizeof(empty_addr.addr))) {
            // Add the newly-found server to our list
            server_list[index].address = evt->data.evt_scanner_scan_report.address;
            break;
          }
          app_log_warning("server_list full\r\n");
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
void sl_btmesh_on_event(sl_btmesh_msg_t *evt)
{
  sl_status_t sc;
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

      // Check all the prerecorded server addresses
      for(uint8_t index = 0; index < SERVER_LIMIT; index++) {
        // If we found the address in the server_list, provision the device directly
        if(!memcmp(evt->data.evt_prov_unprov_beacon.address.addr, server_list[index].address.addr, sizeof(evt->data.evt_prov_unprov_beacon.address.addr))) {
          if(server_list[index].is_provisioned) break;
          app_log("Provisioning a Remote Server Provisioning device\r\n");
          server_list[index].uuid = evt->data.evt_prov_unprov_beacon.uuid;
          sc = sl_btmesh_prov_create_provisioning_session(network_id, evt->data.evt_prov_unprov_beacon.uuid, 0);
          app_assert_status_f(sc, "sl_btmesh_prov_create_provisioning_session failed\r\n");
          sc = sl_btmesh_prov_provision_adv_device(evt->data.evt_prov_unprov_beacon.uuid);
          app_assert_status_f(sc, "sl_btmesh_prov_provision_adv_device failed\r\n");
          server_list[index].is_provisioned = 0x1;
          break;
        }
      }
      break;
    case sl_btmesh_evt_prov_device_provisioned_id:
      app_log_debug("address: %x\r\n", evt->data.evt_prov_device_provisioned.address);
      // New device provisioned (both direct and remote!)
      char *device_type = "remote";
      // Roll through the server_list, if provisioned device is found by uuid, save the primary element address (where the Remote Provisioning Server Model should be)
      for(uint8_t index = 0; index < SERVER_LIMIT; index++) {
        if(!memcmp(evt->data.evt_prov_device_provisioned.uuid.data, server_list[index].uuid.data, sizeof(evt->data.evt_prov_unprov_beacon.uuid.data))) {
          server_list[index].primary_element_address = evt->data.evt_prov_device_provisioned.address;
          device_type = "server";
          break;
        }
      }
      app_log("New %s device provisioned: %x\r\n", device_type, evt->data.evt_prov_device_provisioned.address);
      if(!strcmp(device_type, "server")) app_log("Press BTN0 to scan for remote unprovisioned devices!\r\n");
      else remote_uuid_to_provision = empty_uuid;
      break;
    // This part is triggered by button press!
    case sl_btmesh_evt_remote_provisioning_client_scan_capabilities_id:
      // After knowing the capabilities, start scanning for remote unprovisioned devices
      app_log_debug("result: %x\r\nserver: %x\r\nmax_items: %x\r\nactive: %x\r\n",
                    evt->data.evt_remote_provisioning_client_scan_capabilities.result,
                    evt->data.evt_remote_provisioning_client_scan_capabilities.server,
                    evt->data.evt_remote_provisioning_client_scan_capabilities.max_items,
                    evt->data.evt_remote_provisioning_client_scan_capabilities.active);
      sc = sl_btmesh_remote_provisioning_client_start_scan(enc_netkey_index,
                                                           evt->data.evt_remote_provisioning_client_scan_capabilities.server,
                                                           0,
                                                           evt->data.evt_remote_provisioning_client_scan_capabilities.max_items,
                                                           10,
                                                           0,
                                                           empty_uuid);
      app_assert_status_f(sc, "sl_btmesh_remote_provisioning_client_start_scan failed\r\n");
      break;
    case sl_btmesh_evt_remote_provisioning_client_scan_status_id:
      // Status of the remote scanning (only sent once, has to be re-requested if we are curious of the progress)
      app_log_debug("result: %x\r\nserver: %x\r\nstatus: %x\r\nstate: %x\r\nmax_reports: %x\r\ntimeout_sec: %x\r\n",
                    evt->data.evt_remote_provisioning_client_scan_status.result,
                    evt->data.evt_remote_provisioning_client_scan_status.server,
                    evt->data.evt_remote_provisioning_client_scan_status.status,
                    evt->data.evt_remote_provisioning_client_scan_status.state,
                    evt->data.evt_remote_provisioning_client_scan_status.max_reports,
                    evt->data.evt_remote_provisioning_client_scan_status.timeout_sec);
      break;
    case sl_btmesh_evt_remote_provisioning_client_scan_report_id:
      // One remote unprovisioned device found, open a link to it
      app_log_debug("server: %x\r\nrssi: %x\r\noob: %x\r\nuri: %lx\r\n",
                    evt->data.evt_remote_provisioning_client_scan_report.server,
                    evt->data.evt_remote_provisioning_client_scan_report.rssi,
                    evt->data.evt_remote_provisioning_client_scan_report.oob,
                    evt->data.evt_remote_provisioning_client_scan_report.uri);
      // Look for our server
      int8_t server_index = -1;
      for(int8_t index = 0; index < SERVER_LIMIT; index++) {
        if(evt->data.evt_remote_provisioning_client_scan_report.server == server_list[index].primary_element_address) server_index = index;
      }
      if(server_index == -1) {
        app_log_warning("server not found!\r\n");
        break;
      }

      // Look for the remote
      int8_t remote_index = -1;
      for(int8_t index = 0; index < REMOTE_LIMIT; index++) {
        // Remote is already in the list
        if(!memcmp(evt->data.evt_remote_provisioning_client_scan_report.uuid.data, server_list[server_index].remote_devices[index].uuid.data, sizeof(evt->data.evt_remote_provisioning_client_scan_report.uuid.data))) {
          remote_index = -2;
          break;
        }
        // First empty place found
        if(!memcmp(empty_uuid.data, server_list[server_index].remote_devices[index].uuid.data, sizeof(empty_uuid.data))) {
          remote_index = index;
          break;
        }
      }
      // Remote is in list, break silently
      if(remote_index == -2) break;
      // Remote list is full
      if(remote_index == -1) {
        app_log_warning("remote_list full\r\n");
        break;
      }

      // Save the UUID of the unprovisioned remote device
      server_list[server_index].remote_devices[remote_index].uuid = evt->data.evt_remote_provisioning_client_scan_report.uuid;
      app_log("Remote Unprovisioned device found and added to list. Press BTN1 to provision it.\r\n");

      break;
    case sl_btmesh_evt_remote_provisioning_client_link_status_id:
      // Current state of the link opening
      app_log_debug("result: %x\r\nserver: %x\r\nstatus: %x\r\nstate: %x\r\n",
                    evt->data.evt_remote_provisioning_client_link_status.result,
                    evt->data.evt_remote_provisioning_client_link_status.server,
                    evt->data.evt_remote_provisioning_client_link_status.status,
                    evt->data.evt_remote_provisioning_client_link_status.state);
    break;

    case sl_btmesh_evt_remote_provisioning_client_link_report_id:
      // Report about the link changes
      app_log_debug("server: %x\r\nstatus: %x\r\nstate: %x\r\nreason: %x\r\n",
                    evt->data.evt_remote_provisioning_client_link_report.server,
                    evt->data.evt_remote_provisioning_client_link_report.status,
                    evt->data.evt_remote_provisioning_client_link_report.state,
                    evt->data.evt_remote_provisioning_client_link_report.reason);

      // If we successfully opened the link via the Remote Provisioning Server, start the provisioning session
      if(evt->data.evt_remote_provisioning_client_link_report.status == sl_btmesh_remote_provisioning_server_success) {
        // Provision the remote device
        sc = sl_btmesh_prov_create_provisioning_session(network_id, remote_uuid_to_provision, 0);
        app_assert_status_f(sc, "sl_btmesh_prov_create_provisioning_session failed\r\n");
        sc = sl_btmesh_prov_provision_remote_device(remote_uuid_to_provision,
                                                    evt->data.evt_remote_provisioning_client_link_report.server);
        app_assert_status_f(sc, "sl_btmesh_prov_provision_adv_device failed\r\n");

      }
    break;
    // Provisioning failed
    case sl_btmesh_evt_prov_provisioning_failed_id:
      app_log_debug("%x\r\n%x\r\n",
                    evt->data.evt_prov_provisioning_failed.reason,
                    evt->data.evt_prov_provisioning_failed.uuid.data[0]);
    break;
    default:
      app_log_debug("unhandled evt: %8.8x class %2.2x method %2.2x\r\n", (unsigned int)SL_BT_MSG_ID(evt->header),
                                                                         (unsigned int)((SL_BT_MSG_ID(evt->header) >> 16) & 0xFF),
                                                                         (unsigned int)((SL_BT_MSG_ID(evt->header) >> 24) & 0xFF) );
      break;
  }
}

void app_button_press_cb(uint8_t button, uint8_t duration)
{
  sl_status_t sc;
  // Selecting action by duration
  switch (duration) {
    case APP_BUTTON_PRESS_DURATION_LONG:
      // Handling of button press greater than 1s and less than 5s
      if (button == BUTTON_0) {
        // Iterate over all the Remote Provisioning Servers and scan for the capabilities
        for(uint8_t index = 0; index < SERVER_LIMIT; index++) {
          if(memcmp(empty_addr.addr, server_list[index].address.addr, sizeof(empty_addr.addr))) {
            sc = sl_btmesh_remote_provisioning_client_get_scan_capabilities(enc_netkey_index,
                                                                            server_list[index].primary_element_address,
                                                                            0);
            app_assert_status_f(sc, "sl_btmesh_remote_provisioning_client_get_scan_capabilities failed\r\n");
          }
        }
      } else {
          if(memcmp(remote_uuid_to_provision.data, empty_uuid.data, sizeof(remote_uuid_to_provision.data))) {
            app_log_warning("Provisioning in progress\r\n");
            return;
          }
          for(uint8_t index = 0; index < SERVER_LIMIT; index++) {
            for(uint8_t index2 = 0; index2 < REMOTE_LIMIT; index2++) {
              // Ignore the device, if it is already provisioned
              if(server_list[index].remote_devices[index2].is_provisioned) continue;
              // Break, if we reached an empty device
              if(!memcmp(empty_uuid.data, server_list[index].remote_devices[index2].uuid.data, sizeof(empty_uuid.data))) break;
              // Open link to the remote device
              remote_uuid_to_provision = server_list[index].remote_devices[index2].uuid;
              sc = sl_btmesh_remote_provisioning_client_open_link(enc_netkey_index,
                                                                  server_list[index].primary_element_address,
                                                                  0,
                                                                  10,
                                                                  0xff,
                                                                  remote_uuid_to_provision);
              app_assert_status_f(sc, "sl_btmesh_remote_provisioning_client_open_link failed\r\n");
              server_list[index].remote_devices[index2].is_provisioned = 0x1;
              return;
          }
        }
      }
      break;
    default:
      break;
  }
}

