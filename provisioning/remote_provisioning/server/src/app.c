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

/// Advertising Provisioning Bearer
#define PB_ADV                         0x1
/// GATT Provisioning Bearer
#define PB_GATT                        0x2

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;

void start_advertising();

/*******************************************************************************
 * Application Init.
 ******************************************************************************/
SL_WEAK void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
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
      app_log("boot\r\n");

      // Initialize Mesh stack in Node operation mode,
      // wait for initialized event
      sc = sl_btmesh_node_init();
      app_assert_status_f(sc, "Failed to init node");

      bd_addr address;
      uint8_t address_type;
      sc = sl_bt_system_get_identity_address(&address, &address_type);
      app_assert_status_f(sc, "sl_bt_system_get_identity_address failed");

      app_log("Bluetooth %s address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                 address_type ? "static random" : "public device",
                 address.addr[5],
                 address.addr[4],
                 address.addr[3],
                 address.addr[2],
                 address.addr[1],
                 address.addr[0]);
      break;
    case sl_bt_evt_scanner_scan_report_id:
      /*app_log("Bluetooth %s address: %02X:%02X:%02X:%02X:%02X:%02X\n",
              evt->data.evt_scanner_scan_report.address_type ? "static random" : "public device",
              evt->data.evt_scanner_scan_report.address.addr[5],
              evt->data.evt_scanner_scan_report.address.addr[4],
              evt->data.evt_scanner_scan_report.address.addr[3],
              evt->data.evt_scanner_scan_report.address.addr[2],
              evt->data.evt_scanner_scan_report.address.addr[1],
              evt->data.evt_scanner_scan_report.address.addr[0]);*/
      break;
    case sl_bt_evt_advertiser_timeout_id:
      // Ignore advertiser timeout events
      break;
    default:
      app_log("unhandled evt: %8.8x class %2.2x method %2.2x\r\n", (unsigned int)SL_BT_MSG_ID(evt->header),
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
    case sl_btmesh_evt_node_initialized_id:
      if (!evt->data.evt_node_initialized.provisioned) {
        // The Node is now initialized,
        // start unprovisioned Beaconing using PB-ADV and PB-GATT Bearers
        sc = sl_btmesh_node_start_unprov_beaconing(PB_ADV | PB_GATT);
        app_assert_status_f(sc, "Failed to start unprovisioned beaconing\r\n");
        start_advertising();
      }
      break;
    case sl_btmesh_evt_node_provisioning_started_id:
      sc = sl_bt_advertiser_stop(advertising_set_handle);
      app_assert_status_f(sc, "sl_bt_advertiser_stop failed");
      break;
    case sl_btmesh_evt_node_provisioned_id:
      app_log("I have been provisioned!\r\n");
      break;
    case sl_btmesh_evt_remote_provisioning_server_link_open_request_id:
      app_log("sl_btmesh_evt_remote_provisioning_server_link_open_request_id\r\n");
      break;
    default:
      app_log("unhandled evt: %8.8x class %2.2x method %2.2x\r\n", (unsigned int)SL_BT_MSG_ID(evt->header),
                                                                   (unsigned int)((SL_BT_MSG_ID(evt->header) >> 16) & 0xFF),
                                                                   (unsigned int)((SL_BT_MSG_ID(evt->header) >> 24) & 0xFF) );
      break;
  }
}

// We are using the older advertiser API, as the newer one is not yet Mesh compatible!
void start_advertising() {
  sl_status_t sc;
  sc = sl_bt_advertiser_create_set(&advertising_set_handle);
  app_assert_status_f(sc, "sl_bt_advertiser_create_set failed");
  uint8_t* data = (unsigned char*)"RemoteServer";
  sc = sl_bt_advertiser_set_data(advertising_set_handle,
                                 0,
                                 12,
                                 data);
  app_assert_status_f(sc, "sl_bt_advertiser_set_data failed");

  // Set advertising interval to 100ms.
  sc = sl_bt_advertiser_set_timing(
    advertising_set_handle,
    160, // min. adv. interval (milliseconds * 1.6)
    160, // max. adv. interval (milliseconds * 1.6)
    0,   // adv. duration
    0);  // max. num. adv. events
  app_assert_status_f(sc, "sl_bt_advertiser_set_timing failed");

  // Start advertising and enable connections.
  sc = sl_bt_advertiser_start(advertising_set_handle,
                              sl_bt_advertiser_user_data,
                              sl_bt_advertiser_non_connectable);
  app_assert_status_f(sc, "sl_bt_advertiser_start failed");
}
