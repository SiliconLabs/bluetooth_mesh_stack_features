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

/* System header */
#include <stdio.h>
#include <string.h>

#include "em_common.h"
#include "app_assert.h"
#include "sl_status.h"
#include "app.h"
#include "gatt_db.h"

#include "sl_btmesh_api.h"
#include "sl_bt_api.h"
#include "sl_board_control.h"
#include "glib.h"
#include "dmd.h"

/* Buttons and LEDs headers */
#include "sl_simple_button.h"
#include "sl_simple_button_instances.h"

#include "sl_btmesh_blob_transfer_server.h"

#ifdef SL_CATALOG_BTMESH_FACTORY_RESET_PRESENT
#include "sl_btmesh_factory_reset.h"
#endif // SL_CATALOG_BTMESH_FACTORY_RESET_PRESENT

/// Advertising Provisioning Bearer
#define PB_ADV                         0x1
/// GATT Provisioning Bearer
#define PB_GATT                        0x2

static GLIB_Context_t glibContext;

#define BUTTON_0                       0

/// Length of the display name buffer
#define NAME_BUF_LEN  18
char name[NAME_BUF_LEN];

#define NETWORK_KEY                 {0x23, 0x98, 0xdf, 0xa5, 0x09, 0x3e, 0x74, 0xbb, 0xc2, 0x45, 0x1f, 0xae, 0xea, 0xd7, 0x67, 0xcd}
#define APPLICATION_KEY             {0x16, 0x39, 0x38, 0x03, 0x9b, 0x8d, 0x8a, 0x20, 0x81, 0x60, 0xa7, 0x93, 0x33, 0x3d, 0x03, 0x61}
#define DEVICE_KEY                  {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

#define UADDR_ELEM_BITS             4
#define UADDR_RSVD_BITS             1

#define GRP_ADDR_BLOB               0xC001
#define GRP_ADDR_BLOB_ID            0xC002
#define GRP_ADDR_SERVER_ADDR        0xC003

static uint16_t address;
static uint16_t appkey_index = 0x0;
static const uint8_t network_id = 0x0;
static const uint8_t vendor_model_elem_index = 0x0;

static const uint8_t blob_id_server_opcodes[] = {0x0};
static const uint8_t server_address_opcodes[] = {0x1};

uint16_t data_id = 0;
uint8_t data[2048];

sl_bt_uuid_64_t blob_id = {{0}};

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
  app_log("Boot\r\n");

  // Enable buttons
  sl_simple_button_enable(&sl_button_btn0);
#ifndef SINGLE_BUTTON
  sl_simple_button_enable(&sl_button_btn1);
#endif // SINGLE_BUTTON
  // Wait
  sl_sleeptimer_delay_millisecond(1);

  uint32_t sc;

  /* Initialize the DMD support for memory lcd display */
  sc = DMD_init(0);
  app_assert_status_f(sc, "DMD_init failed");

  /* Initialize the glib context */
  sc = GLIB_contextInit(&glibContext);
  app_assert_status_f(sc, "GLIB_contextInit failed");

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
  snprintf(name, NAME_BUF_LEN, "blob_server %02x:%02x",
           addr->addr[1],
           addr->addr[0]);

  app_log("Device name: '%s'\r\n", name);

  sc = sl_bt_gatt_server_write_attribute_value(gattdb_device_name,
                                               0,
                                               strlen(name),
                                               (uint8_t *)name);
  app_assert_status_f(sc, "sl_bt_gatt_server_write_attribute_value failed");
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
  }
}


/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the dummy weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(struct sl_bt_msg *evt)
{
  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_bt_evt_system_boot_id:
      handle_boot_event();

      aes_key_128 netkey = {NETWORK_KEY};
      aes_key_128 appkey = {APPLICATION_KEY};
      aes_key_128 devkey = {DEVICE_KEY};
      sl_status_t sc;
      uint32_t count;
      bd_addr btaddr;
      uint8_t type;

      app_log("Self provisioning\r\n");

      // Network key
      count = 0;
      sc = sl_btmesh_node_get_key_count(0, &count);
      app_assert_status_f(sc, "sl_btmesh_node_get_key_count failed");

      sc = sl_bt_system_get_identity_address(&btaddr, &type);
      app_assert_status_f(sc, "sl_bt_system_get_identity_address failed");

      address = *(uint16_t *)&btaddr.addr[0] ^
                *(uint16_t *)&btaddr.addr[2] ^
                *(uint16_t *)&btaddr.addr[4];
      address &= (0xFFFF<<UADDR_ELEM_BITS);
      address &= (0xFFFF>>(UADDR_RSVD_BITS+1));

      if (count == 0) {
          // Set provisioning data
          sc = sl_btmesh_node_set_provisioning_data(devkey,
                                                    netkey,
                                                    network_id,
                                                    0,
                                                    address,
                                                    0);
          app_assert_status_f(sc, "sl_btmesh_node_set_provisioning_data failed");

          app_log("Node self provisioned, address = %04x\r\n", address);
      }

      // Application key
      count = 0;
      sc = sl_btmesh_node_get_key_count(1, &count);
      app_assert_status_f(sc, "sl_btmesh_node_get_key_count failed");

      if (count == 0) {
          // Add application key
          sc = sl_btmesh_test_add_local_key(1,
                                            appkey,
                                            appkey_index,
                                            network_id);
          app_assert_status_f(sc, "sl_btmesh_test_add_local_key failed");


          app_log("Node self configured, resetting\r\n");
          sl_bt_system_reboot();
      }
      break;
    case sl_bt_evt_scanner_legacy_advertisement_report_id:
      break;
    case sl_bt_evt_advertiser_timeout_id:
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
  switch (SL_BT_MSG_ID(evt->header)) {
    case sl_btmesh_evt_node_initialized_id:
      sl_status_t sc;
      // Vendor Model Init for the BLOB ID
      sc = sl_btmesh_vendor_model_init(vendor_model_elem_index,
                                       0x1000,
                                       0x2000,
                                       0,
                                       1,
                                       blob_id_server_opcodes);
      if(sc != SL_STATUS_OK) app_log("sl_btmesh_vendor_model_init: failed 0x%.2lx\r\n", sc);

      // Bind Vendor Model to Appkey
      sc = sl_btmesh_test_bind_local_model_app(vendor_model_elem_index,
                                               appkey_index,
                                               0x1000,
                                               0x2000);
      if(sc != SL_STATUS_OK) app_log("sl_btmesh_test_bind_local_model_app: failed 0x%.2lx\r\n", sc);

      // Subscribing configuration
      sc = sl_btmesh_test_add_local_model_sub(vendor_model_elem_index,
                                              0x1000,
                                              0x2000,
                                              GRP_ADDR_BLOB_ID);
      if(sc != SL_STATUS_OK) app_log("sl_btmesh_test_add_local_model_sub: failed 0x%.2lx\r\n", sc);

      // Bind the BLOB Model to Appkey
      sc = sl_btmesh_test_bind_local_model_app(vendor_model_elem_index,
                                               appkey_index,
                                               0xFFFF,
                                               0x1400);
      if(sc != SL_STATUS_OK) app_log("sl_btmesh_test_bind_local_model_app: failed 0x%.2lx\r\n", sc);

      // Subscribing configuration
      sc = sl_btmesh_test_add_local_model_sub(vendor_model_elem_index,
                                              0xFFFF,
                                              0x1400,
                                              GRP_ADDR_BLOB);
      if(sc != SL_STATUS_OK) app_log("sl_btmesh_test_add_local_model_sub: failed 0x%.2lx\r\n", sc);

      // Vendor Model Init for the Server Address
      sc = sl_btmesh_vendor_model_init(vendor_model_elem_index,
                                       0x1000,
                                       0x2002,
                                       1,
                                       1,
                                       server_address_opcodes);
      if(sc != SL_STATUS_OK) app_log("sl_btmesh_vendor_model_init: failed 0x%.2lx\r\n", sc);

      // Bind Vendor Model to Appkey
      sc = sl_btmesh_test_bind_local_model_app(vendor_model_elem_index,
                                               appkey_index,
                                               0x1000,
                                               0x2002);
      if(sc != SL_STATUS_OK) app_log("sl_btmesh_test_bind_local_model_app: failed 0x%.2lx\r\n", sc);

      // Publish configuration
      sc = sl_btmesh_test_set_local_model_pub(vendor_model_elem_index,
                                              appkey_index,
                                              0x1000,
                                              0x2002,
                                              GRP_ADDR_SERVER_ADDR,
                                              3,
                                              0,
                                              0,
                                              0);
      if(sc != SL_STATUS_OK) app_log("sl_btmesh_test_set_local_model_pub: failed 0x%.2lx\r\n", sc);

      uint16_t addr_handle;
      sc = sl_btmesh_vendor_model_send_tracked(GRP_ADDR_SERVER_ADDR,
                                               0,
                                               appkey_index,
                                               vendor_model_elem_index,
                                               0x1000,
                                               0x2002,
                                               0,
                                               0,
                                               server_address_opcodes[0],
                                               1,
                                               2,
                                               (uint8_t*)&address,
                                               &addr_handle);
      if(sc != SL_STATUS_OK) app_log("sl_btmesh_vendor_model_send_tracked: failed 0x%.2lx\r\n", sc);
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
      if(evt->data.evt_vendor_model_receive.vendor_id == 0x1000 &&
         evt->data.evt_vendor_model_receive.model_id == 0x2000) {
        app_log("sl_btmesh_evt_vendor_model_receive_id: %x\r\n", evt->data.evt_vendor_model_receive.payload.data[0]);

        blob_id.data[0] = evt->data.evt_vendor_model_receive.payload.data[0];

        sc = sl_btmesh_blob_transfer_server_start(vendor_model_elem_index,
                                                  &blob_id,
                                                  10,
                                                  5);
        app_assert_status_f(sc, "sl_btmesh_blob_transfer_server_start failed\n");
      }
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