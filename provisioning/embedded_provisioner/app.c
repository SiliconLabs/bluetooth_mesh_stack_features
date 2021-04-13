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

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"
#include <gecko_configuration.h>
#include <mesh_sizes.h>

#include "config.h"
#include "stack.h"

/***************************************************************************//**
 * @addtogroup Application
 * @{
 ******************************************************************************/
#define LIGHT_CTRL_GRP_ADDR   0xC001
#define LIGHT_STATUS_GRP_ADDR 0xC002

#define VENDOR_GRP_ADDR       0xC003

/* models used by simple light example (on/off only)
 * The beta SDK 1.0.1 and 1.1.0 examples are based on these
 * */
#define LIGHT_MODEL_ID  0x1000 // Generic On/Off Server
#define SWITCH_MODEL_ID 0x1001 // Generic On/Off Client

/*
 * Lightness models used in the dimming light example of 1.2.0 SDK
 * */
#define DIM_LIGHT_MODEL_ID  0x1300 // Light Lightness Server
#define DIM_SWITCH_MODEL_ID 0x1302 // Light Lightness Client

#define BLE_MESH_UUID_LEN_BYTE (16)
#define BLE_ADDR_LEN_BYTE (6)
typedef struct{
  uint8_t address[BLE_ADDR_LEN_BYTE];
  uint8_t uuid[BLE_MESH_UUID_LEN_BYTE];
  uint8_t is_provisioned;
} device_table_entry_t;

#define MAX_NUM_BTMESH_DEV (10)

// bluetooth stack heap
#define MAX_CONNECTIONS 2

device_table_entry_t bluetooth_device_table[MAX_NUM_BTMESH_DEV];

static const uint8 fixed_netkey[16] = {0x23, 0x98, 0xdf, 0xa5, 0x09, 0x3e, 0x74, 0xbb, 0xc2, 0x45, 0x1f, 0xae, 0xea, 0xd7, 0x67, 0xcd};
static const uint8 fixed_appkey[16] = {0x16, 0x39, 0x38, 0x03, 0x9b, 0x8d, 0x8a, 0x20, 0x81, 0x60, 0xa7, 0x93, 0x33, 0x3d, 0x03, 0x61};

/* DCD receive */
uint8 _dcd_raw[256]; // raw content of the DCD received from remote node
uint8 _dcd_raw_len = 0;

// DCD content of the last provisioned device. (the example code decodes up to two elements, but
// only the primary element is used in the configuration to simplify the code)
tsDCD_ElemContent _sDCD_Prim;
tsDCD_ElemContent _sDCD_2nd; /* second DCD element is decoded if present, but not used for anything (just informative) */

/* DCD parsing */

typedef struct
{
  // model bindings to be done. for simplicity, all models are bound to same appkey in this example
  // (assuming there is exactly one appkey used and the same appkey is used for all model bindings)
  tsModel bind_model[4];
  uint8 num_bind;
  uint8 num_bind_done;

  // publish addresses for up to 4 models
  tsModel pub_model[4];
  uint16 pub_address[4];
  uint8 num_pub;
  uint8 num_pub_done;

  // subscription addresses for up to 4 models
  tsModel sub_model[4];
  uint16 sub_address[4];
  uint8 num_sub;
  uint8 num_sub_done;

}tsConfig;

// config data to be sent to last provisioned node:
tsConfig _sConfig;

void mesh_native_bgapi_init(void);
bool mesh_bgapi_listener(struct gecko_cmd_packet *evt);
static int8_t IsDevPresent(const uint8_t * const addr);
static void config_check(void);

/*
 * Add one publication setting to the list of configurations to be done
 * */
static void config_pub_add(uint16 model_id, uint16 vendor_id, uint16 address)
{
  _sConfig.pub_model[_sConfig.num_pub].model_id = model_id;
  _sConfig.pub_model[_sConfig.num_pub].vendor_id = vendor_id;
  _sConfig.pub_address[_sConfig.num_pub] = address;
  _sConfig.num_pub++;
}

/*
 * Add one subscription setting to the list of configurations to be done
 * */
static void config_sub_add(uint16 model_id, uint16 vendor_id, uint16 address)
{
  _sConfig.sub_model[_sConfig.num_sub].model_id = model_id;
  _sConfig.sub_model[_sConfig.num_sub].vendor_id = vendor_id;
  _sConfig.sub_address[_sConfig.num_sub] = address;
  _sConfig.num_sub++;
}

/*
 * Add one appkey/model bind setting to the list of configurations to be done
 * */
static void config_bind_add(uint16 model_id, uint16 vendor_id, uint16 netkey_id, uint16 appkey_id)
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

static uint8 network_id;
static uint16 appkey_index;
static uint16 provisionee_addr;
static uint16 sub_address;

/* BLE stack device state machine */
void bkgndBLEMeshStack_app(void)
{
  struct gecko_msg_mesh_prov_create_network_rsp_t *new_netkey_rsp;
  struct gecko_msg_mesh_prov_create_appkey_rsp_t *new_appkey_rsp;
  struct gecko_msg_mesh_prov_device_provisioned_evt_t *prov_evt;
  struct gecko_msg_mesh_config_client_get_dcd_rsp_t* get_dcd_result;
  struct gecko_msg_mesh_config_client_dcd_data_evt_t *pDCD;
  struct gecko_msg_mesh_config_client_add_appkey_rsp_t *appkey_deploy_evt;
  struct gecko_msg_mesh_config_client_bind_model_rsp_t *model_app_bind_result;
  struct gecko_msg_mesh_config_client_set_model_pub_rsp_t *model_pub_set_result;
  struct gecko_msg_mesh_config_client_add_model_sub_rsp_t *model_sub_add_result;
  uint16 result;
  uint8 idx; /* j short array index */
  int8 dev_idx; /* device */
  uint16 vendor_id;
  uint16 model_id;
  uint16 pub_address;

  /* Init local variables */
  result=0;

  /* event handling */
  struct gecko_cmd_packet *evt = gecko_wait_event();
  bool pass = mesh_bgapi_listener(evt);
  if (pass /*&& event_is_not_deprecated(evt)*/) {
    switch (BGLIB_MSG_ID(evt->header)) {
      case gecko_evt_system_boot_id:
        /* Initialize Mesh stack in Node operation mode, wait for initialized event */
        if(0 != gecko_cmd_mesh_prov_init()->result)
        {
          /* Something went wrong */
          printf("gecko_cmd_mesh_prov_init failed\n\r");
        }

        break;
      case gecko_evt_mesh_prov_initialized_id:
        new_netkey_rsp = gecko_cmd_mesh_prov_create_network(16, fixed_netkey);
        if(0 != new_netkey_rsp->result)
        {
          /* Something went wrong */
          printf("gecko_cmd_mesh_prov_create_network: failed 0x%.2x\n\r",new_netkey_rsp->result);
        }
        else
        {
          network_id=new_netkey_rsp->network_id;
          printf("Success, netkey id = %x\r\n", new_netkey_rsp->network_id);
        }
        new_appkey_rsp = gecko_cmd_mesh_prov_create_appkey(new_netkey_rsp->network_id, 16, &fixed_appkey[0]);

        if(0 != new_appkey_rsp->result)
        {
          /* Something went wrong */
          printf("gecko_cmd_mesh_prov_create_appkey: failed 0x%.2x\n\r",new_appkey_rsp->result);
        }
        else
        {
          appkey_index=new_appkey_rsp->appkey_index;
          printf("Success, appkey id = %x\r\n", new_appkey_rsp->appkey_index);
        }

        /* Networks  */
        printf("networks: 0x%x ",evt->data.evt_mesh_prov_initialized.networks);

        /* address */
        printf("address: 0x%x ",evt->data.evt_mesh_prov_initialized.address);

        /* ivi  */
        printf("ivi: 0x%lx",evt->data.evt_mesh_prov_initialized.ivi);
        printf("\n\r");

        /* Scan for unprovisioned beacons */
        result=gecko_cmd_mesh_prov_scan_unprov_beacons()->result;

        break;
      case gecko_evt_mesh_prov_unprov_beacon_id:

        /* PB-ADV only */
        if(0 == evt->data.evt_mesh_prov_unprov_beacon.bearer)
        {
          /* Get BT mesh device index */
          dev_idx = IsDevPresent( &evt->data.evt_mesh_prov_unprov_beacon.address.addr[0]);

          /* fill up btmesh device struct */
          if( 0 > dev_idx )
          {
            /* Device is not present */
            for(idx=0;idx<MAX_NUM_BTMESH_DEV;idx++)
            {
              if(   ( 0x00 == bluetooth_device_table[idx].address[0] )
                 && ( 0x00 == bluetooth_device_table[idx].address[1] )
                 && ( 0x00 == bluetooth_device_table[idx].address[2] )
                 && ( 0x00 == bluetooth_device_table[idx].address[3] )
                 && ( 0x00 == bluetooth_device_table[idx].address[4] )
                 && ( 0x00 == bluetooth_device_table[idx].address[5] )
                )
              {
                memcpy(&bluetooth_device_table[idx].address[0],&evt->data.evt_mesh_prov_unprov_beacon.address.addr[0],BLE_ADDR_LEN_BYTE);
                memcpy(&bluetooth_device_table[idx].uuid[0],&evt->data.evt_mesh_prov_unprov_beacon.uuid.data[0],BLE_MESH_UUID_LEN_BYTE);

                /* Display info banner */
                printf("URI hash: 0x%lx ",evt->data.evt_mesh_prov_unprov_beacon.uri_hash);
                printf("bearer: 0x%x ",evt->data.evt_mesh_prov_unprov_beacon.bearer);
                printf("address: 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x ",bluetooth_device_table[idx].address[0],
                                                                 bluetooth_device_table[idx].address[1],
                                                                 bluetooth_device_table[idx].address[2],
                                                                 bluetooth_device_table[idx].address[3],
                                                                 bluetooth_device_table[idx].address[4],
                                                                 bluetooth_device_table[idx].address[5]);
                printf("UUID: 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x ",bluetooth_device_table[idx].uuid[0],
                                                              bluetooth_device_table[idx].uuid[1],
                                                              bluetooth_device_table[idx].uuid[2],
                                                              bluetooth_device_table[idx].uuid[3],
                                                              bluetooth_device_table[idx].uuid[4],
                                                              bluetooth_device_table[idx].uuid[5]);
                printf("address type: 0x%x",evt->data.evt_mesh_prov_unprov_beacon.address_type);
                printf("\n\r");
                break;
              }
            }
          }
        }

        break;
      /* Provisioning */
      case gecko_evt_mesh_prov_provisioning_failed_id:
        printf("provisioning failed\n\r");
        break;
      case gecko_evt_mesh_prov_device_provisioned_id:

        prov_evt = (struct gecko_msg_mesh_prov_device_provisioned_evt_t*)&(evt->data);

        provisionee_addr = prov_evt->address;
        printf("Node successfully provisioned. Address: %4.4x, ", provisionee_addr);

        printf("uuid 0x");
        for (uint8_t i = 0; i < prov_evt->uuid.len; i++) printf("%02X", prov_evt->uuid.data[i]);
        printf("\r\n");

        printf(" getting dcd ...\n\r");

        get_dcd_result = gecko_cmd_mesh_config_client_get_dcd(network_id, provisionee_addr, 0);
        if (get_dcd_result->result == 0x0181) {
          printf("."); fflush(stdout);
        } else if(get_dcd_result->result != 0x0){
          printf("gecko_cmd_mesh_config_client_get_dcd failed with result 0x%X (%d) addr %x\r\n", get_dcd_result->result, get_dcd_result->result, prov_evt->address);
        }
        else
        {
          printf("requesting DCD from the node...\r\n");
        }

        break;

      /* Config events */
      case gecko_evt_mesh_config_client_dcd_data_id:
        pDCD = (struct gecko_msg_mesh_config_client_dcd_data_evt_t *)&(evt->data);
        printf("DCD data event, received %u bytes\r\n", pDCD->data.len);

        // copy the data into one large array. the data may come in multiple smaller pieces.
        // the data is not decoded until all DCD events have been received (see below)
        if((_dcd_raw_len + pDCD->data.len) <= 256)
        {
          memcpy(&(_dcd_raw[_dcd_raw_len]), pDCD->data.data, pDCD->data.len);
          _dcd_raw_len += pDCD->data.len;
        }

        break;
      case gecko_evt_le_connection_opened_id:
        printf("LE connection opened\r\n");
        break;

      case gecko_evt_mesh_config_client_dcd_data_end_id:
        printf("DCD data end event. Decoding the data.\r\n");
        // decode the DCD content
        DCD_decode();

        // check the desired configuration settings depending on what's in the DCD
        config_check();

        appkey_deploy_evt = gecko_cmd_mesh_config_client_add_appkey(network_id, provisionee_addr, appkey_index, network_id);
        if (appkey_deploy_evt->result == 0) {
          printf("Deploying appkey to node 0x%4.4x\r\n", provisionee_addr);
        }else{
          printf("Appkey deployment failed. addr %x, error: %x\r\n", provisionee_addr, appkey_deploy_evt->result);
        }

        break;
      case gecko_evt_mesh_config_client_appkey_status_id:
        result = evt->data.evt_mesh_config_client_appkey_status.result;
        if(result == 0)
        {
          printf(" appkey added\r\n");
          /* move to next step which is binding appkey to models */

          // take the next model from the list of models to be bound with application key.
          // for simplicity, the same appkey is used for all models but it is possible to also use several appkeys
          model_id = _sConfig.bind_model[_sConfig.num_bind_done].model_id;
          vendor_id = _sConfig.bind_model[_sConfig.num_bind_done].vendor_id;

          printf("APP BIND, config %d/%d:: model %4.4x key index %x\r\n", _sConfig.num_bind_done+1, _sConfig.num_bind, model_id, appkey_index);

          model_app_bind_result = gecko_cmd_mesh_config_client_bind_model( network_id, provisionee_addr, 0, // element index
                                                                           appkey_index, vendor_id, model_id );
        }
        break;
      case gecko_evt_mesh_config_client_binding_status_id:
        result = evt->data.evt_mesh_config_client_appkey_status.result;
        if(result == bg_err_success)
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

            model_app_bind_result = gecko_cmd_mesh_config_client_bind_model( network_id, provisionee_addr, 0, // element index
                                                                             appkey_index, vendor_id, model_id );
          }
          else
          {
            // get the next model/address pair from the configuration list:
            model_id = _sConfig.pub_model[_sConfig.num_pub_done].model_id;
            vendor_id = _sConfig.pub_model[_sConfig.num_pub_done].vendor_id;
            pub_address = _sConfig.pub_address[_sConfig.num_pub_done];

            printf("PUB SET, config %d/%d: model %4.4x -> address %4.4x\r\n", _sConfig.num_pub_done+1, _sConfig.num_pub, model_id, pub_address);

            model_pub_set_result = gecko_cmd_mesh_config_client_set_model_pub(network_id, provisionee_addr,
              0, /* element index */
              vendor_id,
              model_id,
              pub_address,
              appkey_index,
              0, /* friendship credential flag */
              3, /* Publication time-to-live value */
              0, /* period = NONE */
              0, /* Publication retransmission count */
              50  /* Publication retransmission interval */ );

            if (model_pub_set_result->result == 0)
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
      case gecko_evt_mesh_config_client_model_pub_status_id:
        result = evt->data.evt_mesh_config_client_model_pub_status.result;
        if(result == bg_err_success)
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

            model_pub_set_result = gecko_cmd_mesh_config_client_set_model_pub(network_id, provisionee_addr,
              0, /* element index */
              vendor_id,
              model_id,
              pub_address,
              appkey_index,
              0, /* friendship credential flag */
              3, /* Publication time-to-live value */
              0, /* period = NONE */
              0, /* Publication retransmission count */
              50  /* Publication retransmission interval */ );
          }
          else
          {
            // move to next step which is configuring subscription settings
            // get the next model/address pair from the configuration list:
            model_id = _sConfig.sub_model[_sConfig.num_sub_done].model_id;
            vendor_id = _sConfig.sub_model[_sConfig.num_sub_done].vendor_id;
            sub_address = _sConfig.sub_address[_sConfig.num_sub_done];

            printf("SUB ADD, config %d/%d: model %4.4x -> address %4.4x\r\n", _sConfig.num_sub_done+1, _sConfig.num_sub, model_id, sub_address);

            model_sub_add_result = gecko_cmd_mesh_config_client_add_model_sub(network_id, provisionee_addr, 0, vendor_id, model_id, sub_address);

            if (model_sub_add_result->result == 0x00)
            {
              printf(" waiting sub ack\r\n");
            }
          }
        }
        break;
      case gecko_evt_mesh_config_client_model_sub_status_id:
        result = evt->data.evt_mesh_config_client_model_sub_status.result;
        if(result == bg_err_success)
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

            model_sub_add_result = gecko_cmd_mesh_config_client_add_model_sub(network_id, provisionee_addr, 0, vendor_id, model_id, sub_address);

            if (model_sub_add_result->result == 0x00)
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
      case gecko_evt_mesh_prov_dcd_status_id:
        printf("dcd status received\n\r");
        break;
      case gecko_evt_mesh_prov_config_status_id:
        printf("prov config status\n\r");
        break;
      case gecko_evt_le_gap_adv_timeout_id:
        /* adv timeout events silently discarded */
        break;
      default:
        printf("unhandled evt: %8.8x class %2.2x method %2.2x\r\n", (unsigned int)BGLIB_MSG_ID(evt->header),
                                                                    (unsigned int)((BGLIB_MSG_ID(evt->header) >> 16) & 0xFF),
                                                                    (unsigned int)((BGLIB_MSG_ID(evt->header) >> 24) & 0xFF) );

        if(model_app_bind_result->result  == 0x0000)
        {
          printf(" waiting bind ack\r\n");
        }

        break;
    }
  }

  return;
}

void provisionBLEMeshStack_app(eMesh_Prov_Node_t eStrategy)
{
  struct gecko_msg_mesh_prov_provision_device_rsp_t *prov_resp_adv;
  uint8 dev_idx; /* array index */
  uint8 idx; /* array index */

  if(    ( eMESH_PROV_ALL == eStrategy )
      || ( eMESH_PROV_NEXT == eStrategy )
    )
  {
    for( dev_idx=0;dev_idx<MAX_NUM_BTMESH_DEV;dev_idx++ )
    {
      /* check if non null and unprovisioned */
      if(  (   ( 0x00 != bluetooth_device_table[dev_idx].address[0] )
            || ( 0x00 != bluetooth_device_table[dev_idx].address[1] )
            || ( 0x00 != bluetooth_device_table[dev_idx].address[2] )
            || ( 0x00 != bluetooth_device_table[dev_idx].address[3] )
            || ( 0x00 != bluetooth_device_table[dev_idx].address[4] )
            || ( 0x00 != bluetooth_device_table[dev_idx].address[5] ) )
         && ( 0x00 == bluetooth_device_table[dev_idx].is_provisioned )
        )
      {
        /* provisioning using ADV bearer (this is the default) */
        prov_resp_adv = gecko_cmd_mesh_prov_provision_device(network_id, BLE_MESH_UUID_LEN_BYTE, &bluetooth_device_table[dev_idx].uuid[0]);
        if (prov_resp_adv->result == 0) {
          printf("Provisioning success : ");
          bluetooth_device_table[dev_idx].is_provisioned=0x01;
        } else {
          printf("Provisioning fail %d: ",prov_resp_adv->result);
        }

        printf("(net id) %d (uuid) ",network_id);
        for( idx=0;idx<BLE_MESH_UUID_LEN_BYTE;idx++ )printf("0x%.2x ",bluetooth_device_table[dev_idx].uuid[idx]);
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
      pdata=(uint8_t*)(&bluetooth_device_table[idx].address[0]);

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
