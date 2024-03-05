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
#include "sl_board_control.h"
#include "glib.h"
#include "dmd.h"

#include "sl_btmesh_blob_transfer_server.h"

/// Advertising Provisioning Bearer
#define PB_ADV                         0x1
/// GATT Provisioning Bearer
#define PB_GATT                        0x2

static GLIB_Context_t glibContext;

// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;

static const uint8_t vendor_model_elem_index = 0x0;
static const uint8_t blob_server_model_elem_index = 0x0;

static const uint8_t client_device_id[] = "ClientDevice";
uint16_t data_id = 0;
uint8_t data[2048];

sl_bt_uuid_64_t blob_id = {{0}};

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
      // Initialize Mesh stack in Node operation mode,
      // wait for initialized event
      //sc = sl_btmesh_node_init();
      //app_assert_status_f(sc, "Failed to init node");

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
    case sl_bt_evt_scanner_legacy_advertisement_report_id:
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
    // -------------------------------
    // Default event handler.
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
    case sl_btmesh_evt_node_initialized_id:
      app_log("node init\r\n");

      uint8_t opcodes[] = {0};
      sc = sl_btmesh_vendor_model_init(vendor_model_elem_index,
                                       0x1000,
                                       0x2000,
                                       1,
                                       1,
                                       opcodes);
      if(sc != SL_STATUS_OK) {
        /* Something went wrong */
        app_log("sl_btmesh_vendor_model_init: failed 0x%.2lx\r\n", sc);
      } else {
        app_log("Success, sl_btmesh_vendor_model_init: 0x%x\r\n", 0x2000);
      }

      if (!evt->data.evt_node_initialized.provisioned) {
        // The Node is now initialized,
        // start unprovisioned Beaconing using PB-ADV and PB-GATT Bearers
        //sc = sl_btmesh_node_start_unprov_beaconing(PB_ADV | PB_GATT);
        //app_assert_status_f(sc, "Failed to start unprovisioned beaconing\n");
        start_advertising();
      }
      break;
    case sl_btmesh_evt_node_provisioning_started_id:
      sc = sl_bt_advertiser_stop(advertising_set_handle);
      app_assert_status_f(sc, "sl_bt_advertiser_stop failed");
      break;
    case sl_btmesh_evt_node_provisioned_id:
      app_log("%x\r\n", evt->data.evt_node_provisioned.address);
      break;
    case sl_btmesh_evt_node_key_added_id:
      app_log("sl_btmesh_evt_node_key_added_id\r\n");
      uint8_t type = evt->data.evt_node_key_added.type;
      if(type == 0x01) {
        app_log("appkey added\r\n");
      }
      break;

    case sl_btmesh_evt_mbt_server_chunk_id:
      for(uint16_t index = 0; index < evt->data.evt_mbt_server_chunk.data.len; index++) {
        data[evt->data.evt_mbt_server_chunk.total_offset + index] = evt->data.evt_mbt_server_chunk.data.data[index];
        data_id++;
      }
    break;
    case sl_btmesh_evt_mbt_server_block_complete_id:

      if(data_id != 2048) return;

      data_id = 0;
      uint8_t row = 0;
      uint8_t column = 0;

      GLIB_clear(&glibContext);

      while(data_id != 2048) {

        for(uint8_t bit = 0; bit < 8; bit++) {
          if(!(data[data_id] & (1 << bit))) GLIB_drawPixel(&glibContext, column, row);
          column++;
        }

        if(column == 128) {
          column = 0;
          row++;
        }

        data_id++;
      }

      data_id = 0;
      DMD_updateDisplay();
    break;
    case sl_btmesh_evt_vendor_model_receive_id:
      app_log("sl_btmesh_evt_vendor_model_receive_id: %x\r\n", evt->data.evt_vendor_model_receive.payload.data[0]);

      blob_id.data[0] = evt->data.evt_vendor_model_receive.payload.data[0];

      sc = sl_btmesh_blob_transfer_server_start(blob_server_model_elem_index,
                                                &blob_id,
                                                10,
                                                5);
      app_assert_status_f(sc, "sl_btmesh_blob_transfer_server_start failed\n");
      break;
    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////

    // -------------------------------
    // Default event handler.
    default:
      app_log_debug("unhandled evt: %8.8x class %2.2x method %2.2x\r\n", (unsigned int)SL_BT_MSG_ID(evt->header),
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
  sc = sl_bt_legacy_advertiser_set_data(advertising_set_handle,
                                        0,
                                        12,
                                        client_device_id);
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
  sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                     sl_bt_advertiser_non_connectable);
  app_assert_status_f(sc, "sl_bt_advertiser_start failed");
}