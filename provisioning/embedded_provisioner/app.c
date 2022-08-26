/***************************************************************************//**
 * @file
 * @brief Silicon Labs Bluetooth mesh light example this example implements an
 * Embedded provisioner.
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

#include "config.h"

/* Buttons and LEDs headers */
/*#include "app_button_press.h"
#include "sl_simple_button.h"*/
#include "sl_simple_button_instances.h"

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
typedef struct{
  bd_addr address;
  uuid_128 uuid;
  uint8_t is_provisioned;
} device_table_entry_t;

#define MAX_NUM_BTMESH_DEV (10)

/// Advertising Provisioning Bearer
#define PB_ADV                         0x1
/// GATT Provisioning Bearer
#define PB_GATT                        0x2

device_table_entry_t bluetooth_device_table[MAX_NUM_BTMESH_DEV];

static const uint8_t fixed_netkey[16] = {0x23, 0x98, 0xdf, 0xa5, 0x09, 0x3e, 0x74, 0xbb, 0xc2, 0x45, 0x1f, 0xae, 0xea, 0xd7, 0x67, 0xcd};
static const uint8_t fixed_appkey[16] = {0x16, 0x39, 0x38, 0x03, 0x9b, 0x8d, 0x8a, 0x20, 0x81, 0x60, 0xa7, 0x93, 0x33, 0x3d, 0x03, 0x61};

/* DCD receive */
uint8_t _dcd_raw[256]; // raw content of the DCD received from remote node
uint8_t _dcd_raw_len = 0;

// DCD content of the last provisioned device. (the example code decodes up to two elements, but
// only the primary element is used in the configuration to simplify the code)
tsDCD_ElemContent _sDCD_Prim;
tsDCD_ElemContent _sDCD_2nd; /* second DCD element is decoded if present, but not used for anything (just informative) */



/// Length of the display name buffer
#define NAME_BUF_LEN                   20
/// Used button index
#define BUTTON_PRESS_BUTTON_0          0




/* DCD parsing */

typedef struct
{
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

}tsConfig;

// config data to be sent to last provisioned node:
tsConfig _sConfig;

static int8_t IsDevPresent(const uint8_t * const addr);
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
  app_log("Boot\n\r");
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
static void config_bind_add(uint16_t model_id, uint16_t vendor_id, uint16_t netkey_id, uint16_t appkey_id)
{
  _sConfig.bind_model[_sConfig.num_bind].model_id = model_id;
  _sConfig.bind_model[_sConfig.num_bind].vendor_id = vendor_id;
  _sConfig.num_bind++;
}

/* Initialize the stack */
void initBLEMeshStack_app(void)
{
  /* Reset working structure */
  memset(&bluetooth_device_table,0x00,(sizeof(device_table_entry_t)*MAX_NUM_BTMESH_DEV));

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
  snprintf(name, NAME_BUF_LEN, "switch node %02x:%02x",
           addr->addr[1], addr->addr[0]);

  app_log("Device name: '%s'\r\n", name);

  result = sl_bt_gatt_server_write_attribute_value(gattdb_device_name,
                                                   0,
                                                   strlen(name),
                                                   (uint8_t *)name);
  if (result) {
    app_log("sl_bt_gatt_server_write_attribute_value() failed, code %lx\r\n",
            result);
  }

  // Show device name on the LCD
  //lcd_print(name, SL_BTMESH_WSTK_LCD_ROW_NAME_CFG_VAL);
}

/***************************************************************************//**
 * Handles button press and does a factory reset
 *
 * @return true if there is no button press
 ******************************************************************************/
bool handle_reset_conditions(void)
{
  // If PB0 is held down then do full factory reset
  if (sl_simple_button_get_state(&sl_button_btn0)
      == SL_SIMPLE_BUTTON_PRESSED) {
    // Full factory reset
    sl_btmesh_initiate_full_reset();
    return false;
  }

#ifndef SINGLE_BUTTON
  // If PB1 is held down then do node factory reset
  if (sl_simple_button_get_state(&sl_button_btn1)
      == SL_SIMPLE_BUTTON_PRESSED) {
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
  // Check reset conditions and continue if not reset.
  if (handle_reset_conditions()) {
    sc = sl_bt_system_get_identity_address(&address, &address_type);
    app_assert_status_f(sc, "Failed to get Bluetooth address\n");
    set_device_name(&address);
    // Initialize Mesh stack in Node operation mode, wait for initialized event
    sc = sl_btmesh_prov_init();
    if (sc) {
      app_log("Initialization failed (0x%x)\r\n", sc);
    } else {
      //initBLEMeshStack_app();
    }
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
      app_log("Connection opened\n\r");
      break;
    case sl_bt_evt_connection_parameters_id:
      //app_log("Connection\n\r");
      break;
    case sl_bt_evt_connection_closed_id:
      app_log("Connection closed\n\r");
      break;

    case sl_bt_evt_scanner_scan_report_id:
      //app_log("Scan\n\r");
      break;
    default:
      break;
  }
}


static uint8_t network_id;
static uint16_t appkey_index;
static uint16_t provisionee_addr;
static uuid_128 provisionee_uuid;
static uint16_t sub_address;

void sl_btmesh_on_event(sl_btmesh_msg_t *evt)
{
  /*struct sl_status_t *new_netkey_rsp;
  struct sl_bt_msg_mesh_prov_create_appkey_rsp_t *new_appkey_rsp;
  struct sl_bt_msg_mesh_prov_device_provisioned_evt_t *prov_evt;
  struct sl_bt_msg_mesh_config_client_get_dcd_rsp_t* get_dcd_result;*/
  sl_btmesh_evt_config_client_dcd_data_t *pDCD;
  /*struct sl_bt_msg_mesh_config_client_add_appkey_rsp_t *appkey_deploy_evt;
  struct sl_bt_msg_mesh_config_client_bind_model_rsp_t *model_app_bind_result;
  struct sl_bt_msg_mesh_config_client_set_model_pub_rsp_t *model_pub_set_result;
  struct sl_bt_msg_mesh_config_client_add_model_sub_rsp_t *model_sub_add_result;*/
  uint16_t result;
  uint8_t idx; /* j short array index */
  int8_t dev_idx; /* device */
  uint16_t vendor_id;
  uint16_t model_id;
  uint16_t pub_address;

  /* Init local variables */
  result=0;

  sl_status_t sc;
  uint32_t handle;

  app_log("event: 0x%x ", SL_BT_MSG_ID(evt->header));

  switch (SL_BT_MSG_ID(evt->header)) {
      /*case sl_bt_evt_system_boot_id:
        app_log("Event Boot 2\n\r");
        // Initialize Mesh stack in Node operation mode, wait for initialized event
        if(SL_STATUS_OK != sl_btmesh_prov_init())
        {
            // Something went wrong
            app_log("sl_btmesh_node_init failed\n\r");
        } else {
            app_log("Init\n\r");
        }

        break;
    case sl_btmesh_evt_node_initialized_id:
      if (!(evt->data.evt_node_initialized.provisioned)) {
        app_log("sl_btmesh_evt_node_initialized_id\n\r");
        // Enable ADV and GATT provisioning bearer
        //sc = sl_btmesh_node_start_unprov_beaconing(PB_ADV | PB_GATT);

        //sc = ();
        app_assert(sc == SL_STATUS_OK,
                   "[E: 0x%04x] Failed to start unprovisioned beaconing\n",
                   (int)sc);
        //provisionBLEMeshStack_app(eMESH_PROV_NEXT);
      }
      break;*/
    case sl_btmesh_evt_node_reset_id:
      sl_btmesh_initiate_full_reset();
      break;
    case sl_btmesh_evt_prov_initialized_id: {
      app_log("sl_btmesh_evt_prov_initialized_id");
      network_id = 0x8;//new_netkey_rsp->network_id;
      sc = sl_btmesh_prov_create_network(network_id, 16, fixed_netkey);
      if(0 != sc)
      {
        /* Something went wrong */
          app_log("sl_btmesh_prov_create_network: failed 0x%.2lx\n\r", sc);
      }
      else
      {
          app_log("Success, netkey id = %x\r\n", network_id);
      }

      size_t max_application_key_size = 16;
      size_t application_key_len = 16;
      uint8_t application_key[16];
      appkey_index = 0x0;//new_appkey_rsp->appkey_index;
      sc = sl_btmesh_prov_create_appkey(network_id, appkey_index, 16, fixed_appkey, max_application_key_size, &application_key_len, application_key);

      if(0 != sc)
      {
        /* Something went wrong */
          app_log("sl_btmesh_prov_create_appkey: failed 0x%.2lx\n\r", sc);
      }
      else
      {
          app_log("Success, appkey id = %x\r\n", appkey_index);
      }

      /* Networks  */
      app_log("networks: 0x%x ", evt->data.evt_prov_initialized.networks);

      /* address */
      app_log("address: 0x%x ", evt->data.evt_prov_initialized.address);

      /* ivi  */
      app_log("ivi: 0x%lx", evt->data.evt_prov_initialized.iv_index);
      app_log("\n\r");

      app_log("Unprovisioned beacons scan");

      /* Scan for unprovisioned beacons */
      result = sl_btmesh_prov_scan_unprov_beacons();
      }
    break;
    case sl_btmesh_evt_prov_initialization_failed_id:
      app_log("failed: 0x%x ", evt->data.evt_prov_initialization_failed.result);
    break;
    case sl_btmesh_evt_prov_unprov_beacon_id:

      app_log("Unprovisioned something");
      /* PB-ADV only */
      if(0 == evt->data.evt_prov_unprov_beacon.bearer)
      {
        /* Get BT mesh device index */
        dev_idx = IsDevPresent( &evt->data.evt_prov_unprov_beacon.address.addr[0]);

        /* fill up btmesh device struct */
        if( 0 > dev_idx )
        {
          /* Device is not present */
          for(idx=0;idx<MAX_NUM_BTMESH_DEV;idx++)
          {
            if(   ( 0x00 == bluetooth_device_table[idx].address.addr[0] )
               && ( 0x00 == bluetooth_device_table[idx].address.addr[1] )
               && ( 0x00 == bluetooth_device_table[idx].address.addr[2] )
               && ( 0x00 == bluetooth_device_table[idx].address.addr[3] )
               && ( 0x00 == bluetooth_device_table[idx].address.addr[4] )
               && ( 0x00 == bluetooth_device_table[idx].address.addr[5] )
              )
            {
                app_log("To be prov");
              memcpy(&bluetooth_device_table[idx].address.addr[0], &evt->data.evt_prov_unprov_beacon.address.addr[0], BLE_ADDR_LEN_BYTE);
              memcpy(&bluetooth_device_table[idx].uuid.data[0], &evt->data.evt_prov_unprov_beacon.uuid.data[0], BLE_MESH_UUID_LEN_BYTE);

              /* Display info banner */
              printf("URI hash: 0x%lx ",evt->data.evt_prov_unprov_beacon.uri_hash);
              printf("bearer: 0x%x ",evt->data.evt_prov_unprov_beacon.bearer);
              printf("address: 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x ",bluetooth_device_table[idx].address.addr[0],
                                                               bluetooth_device_table[idx].address.addr[1],
                                                               bluetooth_device_table[idx].address.addr[2],
                                                               bluetooth_device_table[idx].address.addr[3],
                                                               bluetooth_device_table[idx].address.addr[4],
                                                               bluetooth_device_table[idx].address.addr[5]);
              printf("UUID: 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x ",bluetooth_device_table[idx].uuid.data[0],
                                                            bluetooth_device_table[idx].uuid.data[1],
                                                            bluetooth_device_table[idx].uuid.data[2],
                                                            bluetooth_device_table[idx].uuid.data[3],
                                                            bluetooth_device_table[idx].uuid.data[4],
                                                            bluetooth_device_table[idx].uuid.data[5]);
              printf("address type: 0x%x",evt->data.evt_prov_unprov_beacon.address_type);
              printf("\n\r");
              break;
            }
          }
        }
      }

      break;
    /* Provisioning */
    case sl_btmesh_evt_prov_provisioning_failed_id:
      printf("provisioning failed\n\r");
      break;
    case sl_btmesh_evt_prov_device_provisioned_id:

      //prov_evt = (struct sl_btmesh_evt_prov_device_provisioned_t*)&(evt->data);

      provisionee_addr = evt->data.evt_prov_device_provisioned.address;
      provisionee_uuid = evt->data.evt_prov_device_provisioned.uuid;
      printf("Node successfully provisioned. Address: %4.4x, ", provisionee_addr);

      printf("uuid 0x");
      for (uint8_t i = 0; i < BLE_MESH_UUID_LEN_BYTE; i++) printf("%02X", provisionee_uuid.data[i]);
      printf("\r\n");

      printf(" getting dcd ...\n\r");

      sc = sl_btmesh_config_client_get_dcd(0x0000, provisionee_addr, 0, &handle);
      if (sc == 0x0181) {
        printf("."); fflush(stdout);
      } else if(sc != 0x0){
        printf("sl_btmesh_config_client_get_dcd failed with result 0x%lX (%ld) addr %x\r\n", sc, sc, provisionee_addr);
      }
      else
      {
        printf("requesting DCD from the node...\r\n");
      }

      break;

    /* Config events */
    case sl_btmesh_evt_config_client_dcd_data_id:
      printf(" DCD event ...\n\r");
      pDCD = &evt->data.evt_config_client_dcd_data;
      printf("DCD data event, received %u bytes\r\n", pDCD->data.len);

      // copy the data into one large array. the data may come in multiple smaller pieces.
      // the data is not decoded until all DCD events have been received (see below)
      if((_dcd_raw_len + pDCD->data.len) <= 256)
      {
        memcpy(&(_dcd_raw[_dcd_raw_len]), pDCD->data.data, pDCD->data.len);
        _dcd_raw_len += pDCD->data.len;
      }

      break;
    case sl_bt_evt_connection_opened_id:
      printf("LE connection opened\r\n");
      break;

    case sl_btmesh_evt_config_client_dcd_data_end_id:
      printf("DCD data end event. Decoding the data.\r\n");
      // decode the DCD content
      DCD_decode();

      // check the desired configuration settings depending on what's in the DCD
      config_check();

      sc = sl_btmesh_config_client_add_appkey(network_id, provisionee_addr, appkey_index, network_id, &handle);
      if (sc == 0) {
        printf("Deploying appkey to node 0x%4.4x\r\n", provisionee_addr);
      }else{
        printf("Appkey deployment failed. addr %x, error: %lx\r\n", provisionee_addr, sc);
      }

      break;
    case sl_btmesh_evt_config_client_appkey_status_id:
      result = evt->data.evt_config_client_appkey_status.result;
      if(result == 0)
      {
        printf(" appkey added\r\n");
        /* move to next step which is binding appkey to models */

        // take the next model from the list of models to be bound with application key.
        // for simplicity, the same appkey is used for all models but it is possible to also use several appkeys
        model_id = _sConfig.bind_model[_sConfig.num_bind_done].model_id;
        vendor_id = _sConfig.bind_model[_sConfig.num_bind_done].vendor_id;

        printf("APP BIND, config %d/%d:: model %4.4x key index %x\r\n", _sConfig.num_bind_done+1, _sConfig.num_bind, model_id, appkey_index);

        sc = sl_btmesh_config_client_bind_model( network_id, provisionee_addr, 0, // element index
                                                                         appkey_index, vendor_id, model_id, &handle );
      }
      break;
    case sl_btmesh_evt_config_client_binding_status_id:
      result = evt->data.evt_config_client_appkey_status.result;
      if(result != SL_STATUS_OK)
      {
        printf(" bind complete\r\n");
        _sConfig.num_bind_done++;

        if(_sConfig.num_bind_done < _sConfig.num_bind)
        {
          // take the next model from the list of models to be bound with application key.
          // for simplicity, the same appkey is used for all models but it is possible to also use several appkeys
          model_id = _sConfig.bind_model[_sConfig.num_bind_done].model_id;
          vendor_id = _sConfig.bind_model[_sConfig.num_bind_done].vendor_id;

          printf("APP BIND, config %d/%d:: model %4.4x key index %x\r\n", _sConfig.num_bind_done+1, _sConfig.num_bind, model_id, appkey_index);

          sc = sl_btmesh_config_client_bind_model( network_id, provisionee_addr, 0, // element index
                                                                           appkey_index, vendor_id, model_id, &handle );
        }
        else
        {
          // get the next model/address pair from the configuration list:
          model_id = _sConfig.pub_model[_sConfig.num_pub_done].model_id;
          vendor_id = _sConfig.pub_model[_sConfig.num_pub_done].vendor_id;
          pub_address = _sConfig.pub_address[_sConfig.num_pub_done];

          printf("PUB SET, config %d/%d: model %4.4x -> address %4.4x\r\n", _sConfig.num_pub_done+1, _sConfig.num_pub, model_id, pub_address);

          sc = sl_btmesh_config_client_set_model_pub(network_id, provisionee_addr,
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

          if (sc == 0)
          {
            printf(" waiting pub ack\r\n");
          }
        }
      }
      else
      {
        printf(" appkey bind failed with code \r\n");
      }
      break;
    case sl_btmesh_evt_config_client_model_pub_status_id:
      result = evt->data.evt_config_client_model_pub_status.result;
      if(result != SL_STATUS_OK)
      {
        printf(" pub set OK\r\n");
        _sConfig.num_pub_done++;

        if(_sConfig.num_pub_done < _sConfig.num_pub)
        {
          /* more publication settings to be done
          ** get the next model/address pair from the configuration list: */
          model_id = _sConfig.pub_model[_sConfig.num_pub_done].model_id;
          vendor_id = _sConfig.pub_model[_sConfig.num_pub_done].vendor_id;
          pub_address = _sConfig.pub_address[_sConfig.num_pub_done];

          printf("PUB SET, config %d/%d: model %4.4x -> address %4.4x\r\n", _sConfig.num_pub_done+1, _sConfig.num_pub, model_id, pub_address);

          sc = sl_btmesh_config_client_set_model_pub(network_id, provisionee_addr,
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
        }
        else
        {
          // move to next step which is configuring subscription settings
          // get the next model/address pair from the configuration list:
          model_id = _sConfig.sub_model[_sConfig.num_sub_done].model_id;
          vendor_id = _sConfig.sub_model[_sConfig.num_sub_done].vendor_id;
          sub_address = _sConfig.sub_address[_sConfig.num_sub_done];

          printf("SUB ADD, config %d/%d: model %4.4x -> address %4.4x\r\n", _sConfig.num_sub_done+1, _sConfig.num_sub, model_id, sub_address);

          sc = sl_btmesh_config_client_add_model_sub(network_id, provisionee_addr, 0, vendor_id, model_id, sub_address, &handle);

          if (sc == 0x00)
          {
            printf(" waiting sub ack\r\n");
          }
        }
      }
      break;
    case sl_btmesh_evt_config_client_model_sub_status_id:
      result = evt->data.evt_config_client_model_sub_status.result;
      if(result != SL_STATUS_OK)
      {
        printf(" sub add OK\r\n");
        _sConfig.num_sub_done++;
        if(_sConfig.num_sub_done < _sConfig.num_sub)
        {
          // move to next step which is configuring subscription settings
          // get the next model/address pair from the configuration list:
          model_id = _sConfig.sub_model[_sConfig.num_sub_done].model_id;
          vendor_id = _sConfig.sub_model[_sConfig.num_sub_done].vendor_id;
          sub_address = _sConfig.sub_address[_sConfig.num_sub_done];

          printf("SUB ADD, config %d/%d: model %4.4x -> address %4.4x\r\n", _sConfig.num_sub_done+1, _sConfig.num_sub, model_id, sub_address);

          sc = sl_btmesh_config_client_add_model_sub(network_id, provisionee_addr, 0, vendor_id, model_id, sub_address, &handle);

          if (sc == 0x00)
          {
            printf(" waiting sub ack\r\n");
          }
        }
        else
        {
          printf("***\r\nconfiguration complete\r\n***\r\n");
        }
      }

      break;
    /*case gecko_evt_mesh_prov_dcd_status_id:
      printf("dcd status received\n\r");
      break;
    case gecko_evt_mesh_prov_config_status_id:
      printf("prov config status\n\r");
      break;
    case gecko_evt_le_gap_adv_timeout_id:
      // adv timeout events silently discarded
      break;
    default:*/
      printf("unhandled evt: %8.8x class %2.2x method %2.2x\r\n", (unsigned int)SL_BT_MSG_ID(evt->header),
                                                                  (unsigned int)((SL_BT_MSG_ID(evt->header) >> 16) & 0xFF),
                                                                  (unsigned int)((SL_BT_MSG_ID(evt->header) >> 24) & 0xFF) );

      if(sc  == 0x0000)
      {
        printf(" waiting bind ack\r\n");
      }

      break;
  }

  return;
}

void provisionBLEMeshStack_app(eMesh_Prov_Node_t eStrategy)
{
  uint8_t dev_idx; /* array index */
  sl_status_t sc;
  printf("provisionBLEMeshStack_app");

  if(    ( eMESH_PROV_ALL == eStrategy )
      || ( eMESH_PROV_NEXT == eStrategy )
    )
  {
    for( dev_idx=0;dev_idx<MAX_NUM_BTMESH_DEV;dev_idx++ )
    {

      /* check if non null and unprovisioned */
      if(  (   ( 0x00 != bluetooth_device_table[dev_idx].address.addr[0] )
            || ( 0x00 != bluetooth_device_table[dev_idx].address.addr[1] )
            || ( 0x00 != bluetooth_device_table[dev_idx].address.addr[2] )
            || ( 0x00 != bluetooth_device_table[dev_idx].address.addr[3] )
            || ( 0x00 != bluetooth_device_table[dev_idx].address.addr[4] )
            || ( 0x00 != bluetooth_device_table[dev_idx].address.addr[5] ) )
         && ( 0x00 == bluetooth_device_table[dev_idx].is_provisioned )
        )
      {
          printf("provisionBLEMeshStack_app 123");
        /* provisioning using ADV bearer (this is the default) */
        sc = sl_btmesh_prov_provision_adv_device(bluetooth_device_table[dev_idx].uuid);
        if (sc == 0) {
          printf("Provisioning success : ");
          bluetooth_device_table[dev_idx].is_provisioned=0x01;
        } else {
          printf("Provisioning fail %ld: ",sc);
        }

        printf("(net id) %d (uuid) ",network_id);
        for (uint8_t i = 0; i < BLE_MESH_UUID_LEN_BYTE; i++) printf("0x%.2x", bluetooth_device_table[dev_idx].uuid.data[i]);
        printf("\r\n");

        break;
      }
    }
  }

  return;
}

static int8_t IsDevPresent(const uint8_t * restrict const addr)
{
  uint8_t idx;
  uint8_t *pdata;
  int8_t res_val;

  /* Initialize locals */
  res_val=-1; /* Pessimistic assumption, dev isn't present */
  pdata=NULL;

  for( idx=0;idx<MAX_NUM_BTMESH_DEV;idx++ )
  {
      pdata=(uint8_t*)(&bluetooth_device_table[idx].address.addr[0]);

      if(   ( addr[0] == pdata[0] )
         && ( addr[1] == pdata[1] )
         && ( addr[2] == pdata[2] )
         && ( addr[3] == pdata[3] )
         && ( addr[4] == pdata[4] )
         && ( addr[5] == pdata[5] )
        )
      {
        res_val=idx;
        break;
      }
  }

  printf("IsDevPresent\r\n");
  return res_val;
}

static void config_check()
{
  int i;

  memset(&_sConfig, 0, sizeof(_sConfig));

  // scan the SIG models in the DCD data
  for(i=0;i<_sDCD_Prim.numSIGModels;i++)
  {
    if(_sDCD_Prim.SIG_models[i] == SWITCH_MODEL_ID)
    {
      config_pub_add(SWITCH_MODEL_ID, 0xFFFF, LIGHT_CTRL_GRP_ADDR);
      config_sub_add(SWITCH_MODEL_ID, 0xFFFF, LIGHT_STATUS_GRP_ADDR);
      config_bind_add(SWITCH_MODEL_ID, 0xFFFF, 0, 0);
    }
    else if(_sDCD_Prim.SIG_models[i] == LIGHT_MODEL_ID)
    {
      config_pub_add(LIGHT_MODEL_ID, 0xFFFF, LIGHT_STATUS_GRP_ADDR);
      config_sub_add(LIGHT_MODEL_ID, 0xFFFF, LIGHT_CTRL_GRP_ADDR);
      config_bind_add(LIGHT_MODEL_ID, 0xFFFF, 0, 0);
    }
    else if(_sDCD_Prim.SIG_models[i] == DIM_SWITCH_MODEL_ID)
    {
      config_pub_add(DIM_SWITCH_MODEL_ID, 0xFFFF, LIGHT_CTRL_GRP_ADDR);
      config_sub_add(DIM_SWITCH_MODEL_ID, 0xFFFF, LIGHT_STATUS_GRP_ADDR);
      config_bind_add(DIM_SWITCH_MODEL_ID, 0xFFFF, 0, 0);
    }
    else if(_sDCD_Prim.SIG_models[i] == DIM_LIGHT_MODEL_ID)
    {
      config_pub_add(DIM_LIGHT_MODEL_ID, 0xFFFF, LIGHT_STATUS_GRP_ADDR);
      config_sub_add(DIM_LIGHT_MODEL_ID, 0xFFFF, LIGHT_CTRL_GRP_ADDR);
      config_bind_add(DIM_LIGHT_MODEL_ID, 0xFFFF, 0, 0);
    }
  }
}

/*******************************************************************************
 * Callbacks
 ******************************************************************************/

/***************************************************************************//**
 * Button press Callbacks
 ******************************************************************************/
/*void app_button_press_cb(uint8_t button, uint8_t duration)
{
  // Selecting action by duration
  switch (duration) {
    case APP_BUTTON_PRESS_DURATION_SHORT:
      // Handling of button press less than 0.25s
      if (button == BUTTON_PRESS_BUTTON_0) {
      } else {
      }
      break;
    case APP_BUTTON_PRESS_DURATION_MEDIUM:
      // Handling of button press greater than 0.25s and less than 1s
      if (button == BUTTON_PRESS_BUTTON_0) {
      } else {
      }
      break;
    case APP_BUTTON_PRESS_DURATION_LONG:
      // Handling of button press greater than 1s and less than 5s
#ifdef SINGLE_BUTTON
#else
      if (button == BUTTON_PRESS_BUTTON_0) {
      } else {
      }
#endif
      break;
    case APP_BUTTON_PRESS_DURATION_VERYLONG:
      // Handling of button press greater than 5s
      if (button == BUTTON_PRESS_BUTTON_0) {
      } else {
      }
      break;
    default:
      break;
  }
}*/
