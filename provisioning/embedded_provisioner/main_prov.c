/***********************************************************************************************//**
 * \file   main.c
 * \brief  BT Mesh provisioner example
 *
 *  Simple provisioner example that can be dropped on top of the soc-btmesh-light example, by replacing
 *  the main.c with this file.
 *
 *  Additional changes needed:
 *  - Configuration Client model needs to be added into the DCD
 *  - adjust following parameters in the memory configuration (in the DCD editor)
 *     - Max Provisioned Devices,
 *     - Max Provisioned Device Netkeys
 *     - Max Foundation Client Cmds
 *     Default value for these is zero. Must use non-zero values to enable provisioning and configuration
 *     of devices.
 *  - After modifying the DCD and the memory config, remember to press Generate button to re-generate the dcd.c source
 *
 *  This example can recognize the generic on/off and light lightness models used in the
 *  lighting demo and configure the switch and light nodes so that light control works (on/off and dimming commands).
 *
 *  Configuration of vendor models is optional and it is disabled by default. To enable vendor model
 *  config symbol CONFIGURE_VENDOR_MODEL must be defined (see below)
 *
 *  Known issues and limitations:
 *   - this is an initial provisioner example code with limited testing and features
 *   - code cleanup and better error handling TBD.
 *   - the max number of provisioned devices is small (up to 14-15 at most) because of limitations in
 *     Persistent Store capacity
 *
 ***************************************************************************************************
 * <b> (C) Copyright 2018 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

/* compile time options */

// uncomment this to enable configuration of vendor model
//#define CONFIGURE_VENDOR_MODEL

// uncomment this to enable provisioning using PB-GATT (PB-ADV is used by default)
//#define PROVISION_OVER_GATT

/* C Standard Library headers */
#include <stdlib.h>
#include <stdio.h>

/* Board headers */
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"

/* WSTK specific includes */
#include "retargetserial.h"
#include "graphics.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"
#include <gecko_configuration.h>
#include "mesh_generic_model_capi_types.h"
#include "mesh_lighting_model_capi_types.h"
#include "mesh_lib.h"
#include <mesh_sizes.h>

/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"
#include <em_gpio.h>

/* Device initialization header */
#include "hal-config.h"

/* LED driver with support for PWM dimming */
#include "led_driver.h"
#include "lcd_driver.h"

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

uint8_t netkey_id = 0xff;
uint8_t appkey_id = 0xff;
uint8_t ask_user_input = false;
uint16 provisionee_address = 0xFFFF;

uint8_t config_retrycount = 0;
uint8_t _uuid_copy[16];
#ifdef PROVISION_OVER_GATT
bd_addr             bt_address;
uint8               bt_address_type;
#endif

// uncomment this to use fixed network and application keys (for debugging only)
//#define USE_FIXED_KEYS

#ifdef USE_FIXED_KEYS
const uint8 fixed_netkey[16] = {0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3};
const uint8 fixed_appkey[16] = {4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,7};
#endif

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

struct mesh_generic_state current, target;

void mesh_native_bgapi_init(void);
bool mesh_bgapi_listener(struct gecko_cmd_packet *evt);

// Maximum number of simultaneous Bluetooth connections
#define MAX_CONNECTIONS 2

// heap for Bluetooth stack
uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS) + BTMESH_HEAP_SIZE + 1760];

typedef struct
{
	uint16 err;
	const char *pShortDescription;
} tsErrCode;

#define STATUS_OK                      0
#define STATUS_BUSY                    0x181

/*
 * Look-up table for mapping error codes to strings. Not a complete
 * list, for full description of error codes, see
 * Bluetooth LE and Mesh Software API Reference Manual */

tsErrCode _sErrCodes[] = {
		{0x0c01, "already_exists"},
		{0x0c02, "does_not_exist"},
		{0x0c03, "limit_reached"},
		{0x0c04, "invalid_address"},
		{0x0c05, "malformed_data"},
		{0x0183, "not_implemented"},
		{0x0185, "timeout"},
};

const char err_unknown[] = "<?>";

const char * res2str(uint16 err)
{
	int i;

	for(i=0;i<sizeof(_sErrCodes)/sizeof(tsErrCode);i++)
	{
		if(err == _sErrCodes[i].err)
		{
			return _sErrCodes[i].pShortDescription;
		}
	}

	// code was not found in the lookup table
	return err_unknown;
}

/*
 * Maximum number of Bluetooth advertisement sets.
 * 1 is allocated for Bluetooth LE stack
 * 1 one for Bluetooth mesh stack
 * 1 needs to be allocated for each Bluetooth mesh network
 *   - Currently up to 4 networks are supported at a time
 */
#define MAX_ADVERTISERS (2 + 4)

// Bluetooth stack configuration
const gecko_configuration_t config =
{
  .bluetooth.max_connections = MAX_CONNECTIONS,
  .bluetooth.max_advertisers = MAX_ADVERTISERS,
  .bluetooth.heap = bluetooth_stack_heap,
  .bluetooth.heap_size = sizeof(bluetooth_stack_heap) - BTMESH_HEAP_SIZE,
  .bluetooth.sleep_clock_accuracy = 100,
  .gattdb = &bg_gattdb_data,
  .btmesh_heap_size = BTMESH_HEAP_SIZE,
#if (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
  .pa.config_enable = 1, // Enable high power PA
  .pa.input = GECKO_RADIO_PA_INPUT_VBAT, // Configure PA input to VBAT
#endif // (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
};

/** Timer Frequency used. */
#define TIMER_CLK_FREQ ((uint32)32768)
/** Convert msec to timer ticks. */
#define TIMER_MS_2_TIMERTICK(ms) ((TIMER_CLK_FREQ * ms) / 1000)

#define TIMER_ID_RESTART    78
#define TIMER_ID_FACTORY_RESET  77
#define TIMER_ID_BUTTON_POLL              49


#define TIMER_ID_GET_DCD				  20
#define TIMER_ID_APPKEY_ADD				  21
#define TIMER_ID_APPKEY_BIND		      22
#define TIMER_ID_PUB_SET 				  23
#define TIMER_ID_SUB_ADD				  24

/** global variables */

static uint8 num_connections = 0;     /* number of active Bluetooth connections */
static uint8 conn_handle = 0xFF;      /* handle of the last opened LE connection */

enum {
	init,
	scanning,
	connecting,
	provisioning,
	provisioned,
	waiting_dcd,
	waiting_appkey_ack,
	waiting_bind_ack,
	waiting_pub_ack,
	waiting_sub_ack
} state;

static void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt);

/**
 * button initialization. Configure pushbuttons PB0,PB1
 * as inputs.
 */
static void button_init()
{
  // configure pushbutton PB0 and PB1 as inputs, with pull-up enabled
  GPIO_PinModeSet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN, gpioModeInputPull, 1);
  GPIO_PinModeSet(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN, gpioModeInputPull, 1);
}

/**
 * Set device name in the GATT database. A unique name is generated using
 * the two last bytes from the Bluetooth address of this device. Name is also
 * displayed on the LCD.
 */
void set_device_name(bd_addr *pAddr)
{
  char name[20];
  uint16 res;

  // create unique device name using the last two bytes of the Bluetooth address
  sprintf(name, "light node %x:%x", pAddr->addr[1], pAddr->addr[0]);

  printf("Device name: '%s'\r\n", name);

  res = gecko_cmd_gatt_server_write_attribute_value(gattdb_device_name, 0, strlen(name), (uint8 *)name)->result;
  if (res) {
    printf("gecko_cmd_gatt_server_write_attribute_value() failed, code %x\r\n", res);
  }

  // show device name on the LCD
  LCD_write(name, LCD_ROW_NAME);
}

/**
 *  this function is called to initiate factory reset. Factory reset may be initiated
 *  by keeping one of the WSTK pushbuttons pressed during reboot. Factory reset is also
 *  performed if it is requested by the provisioner (event gecko_evt_mesh_node_reset_id)
 */
void initiate_factory_reset(void)
{
  printf("factory reset\r\n");
  LCD_write("\n***\nFACTORY RESET\n***", LCD_ROW_STATUS);

  /* if connection is open then close it before rebooting */
  if (conn_handle != 0xFF) {
    gecko_cmd_le_connection_close(conn_handle);
  }

  /* perform a factory reset by erasing PS storage. This removes all the keys and other settings
     that have been configured for this node */
  gecko_cmd_flash_ps_erase_all();

  // reboot after a small delay
  gecko_cmd_hardware_set_soft_timer(2 * 32768, TIMER_ID_FACTORY_RESET, 1);
}

int main()
{
  // Initialize device
  initMcu();
  // Initialize board
  initBoard();
  // Initialize application
  initApp();

  gecko_stack_init(&config);
  gecko_bgapi_class_dfu_init();
  gecko_bgapi_class_system_init();
  gecko_bgapi_class_le_gap_init();
  gecko_bgapi_class_le_connection_init();
  gecko_bgapi_class_gatt_init();
  gecko_bgapi_class_gatt_server_init();
  gecko_bgapi_class_hardware_init();
  gecko_bgapi_class_flash_init();
  gecko_bgapi_class_test_init();
  gecko_bgapi_class_sm_init();
  mesh_native_bgapi_init();
  gecko_bgapi_class_mesh_config_client_init();
  gecko_initCoexHAL();

  RETARGET_SerialInit();

  /* initialize LEDs and buttons. Note: some radio boards share the same GPIO for button & LED.
   * Initialization is done in this order so that default configuration will be "button" for those
   * radio boards with shared pins. LEDS_init() is called later as needed to (re)initialize the LEDs
   * */
  LEDS_init();
  button_init();

  LCD_init();

  while (1) {
    struct gecko_cmd_packet *evt = gecko_wait_event();
    bool pass = mesh_bgapi_listener(evt);
    if (pass) {
      handle_gecko_event(BGLIB_MSG_ID(evt->header), evt);
    }
  }
}

static void  button_poll()
{
	int key;

	if(ask_user_input == false){
		return;
	}

	/* provisioning can be accepted/rejected either by pushbuttons PB1/PB0
	 * or alternatively by entering 'y' / 'n' on keyboard */
	key = RETARGET_ReadChar();

	if (GPIO_PinInGet(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN) == 0 || key == 'y') {
		ask_user_input = false;
		printf("Sending prov request\r\n");

#ifndef PROVISION_OVER_GATT
		// provisioning using ADV bearer (this is the default)
		struct gecko_msg_mesh_prov_provision_device_rsp_t *prov_resp_adv;
		prov_resp_adv = gecko_cmd_mesh_prov_provision_device(netkey_id, 16, _uuid_copy);

		if (prov_resp_adv->result == 0) {
			state = provisioning;
		} else {
			printf("Failed call to provision node. %x\r\n", prov_resp_adv->result);
		}
#else
		// provisioning using GATT bearer. First we must open a connection to the remote device
		if(gecko_cmd_le_gap_open(bt_address, bt_address_type)->result == 0)
		{
			printf("trying to open a connection\r\n");
		}
		else
		{
			printf("le_gap_open failed\r\n");
		}
		state = connecting;

#endif
	}
	else if (GPIO_PinInGet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN) == 0 || key == 'n') {
		ask_user_input = false;
	}

}

typedef struct
{
	uint16 model_id;
	uint16 vendor_id;
} tsModel;

// max number of SIG models in the DCD
#define MAX_SIG_MODELS    16

// max number of vendor models in the DCD
#define MAX_VENDOR_MODELS 4

/* struct for storing the content of one element in the DCD */
typedef struct
{
	uint16 SIG_models[MAX_SIG_MODELS];
	uint8 numSIGModels;

	tsModel vendor_models[MAX_VENDOR_MODELS];
	uint8_t numVendorModels;
}tsDCD_ElemContent;

// DCD content of the last provisioned device. (the example code decodes up to two elements, but
// only the primary element is used in the configuration to simplify the code)
tsDCD_ElemContent _sDCD_Prim;
tsDCD_ElemContent _sDCD_2nd; /* second DCD element is decoded if present, but not used for anything (just informative) */

uint8 _dcd_raw[256]; // raw content of the DCD received from remote node
uint8 _dcd_raw_len = 0;

/* this struct is used to help decoding the raw DCD data */
typedef struct
{
	uint16 companyID;
	uint16 productID;
	uint16 version;
	uint16 replayCap;
	uint16 featureBitmask;
	uint8 payload[1];
} tsDCD_Header;

/* this struct is used to help decoding the raw DCD data */
typedef struct
{
	uint16 location;
	uint8 numSIGModels;
	uint8 numVendorModels;
	uint8 payload[1];
} tsDCD_Elem;

/* function for decoding one element inside the DCD. Parameters:
 *  pElem: pointer to the beginning of element in the raw DCD data
 *  pDest: pointer to a struct where the decoded values are written
 * */
static void DCD_decode_element(tsDCD_Elem *pElem, tsDCD_ElemContent *pDest)
{
	uint16 *pu16;
	int i;

	memset(pDest, 0, sizeof(*pDest));

	pDest->numSIGModels = pElem->numSIGModels;
	pDest->numVendorModels = pElem->numVendorModels;

	printf("Num sig models: %d\r\n", pDest->numSIGModels );
	printf("Num vendor models: %d\r\n", pDest->numVendorModels);

	if(pDest->numSIGModels > MAX_SIG_MODELS)
	{
		printf("ERROR: number of SIG models in DCD exceeds MAX_SIG_MODELS (%u) limit!\r\n", MAX_SIG_MODELS);
		return;
	}
	if(pDest->numVendorModels > MAX_VENDOR_MODELS)
	{
		printf("ERROR: number of VENDOR models in DCD exceeds MAX_VENDOR_MODELS (%u) limit!\r\n", MAX_VENDOR_MODELS);
		return;
	}

	// set pointer to the first model:
	pu16 = (uint16 *)pElem->payload;

	// grab the SIG models from the DCD data
	for(i=0;i<pDest->numSIGModels;i++)
	{
		pDest->SIG_models[i] = *pu16;
		pu16++;
		printf("model ID: %4.4x\r\n", pDest->SIG_models[i]);
	}

	// grab the vendor models from the DCD data
	for (i = 0; i < pDest->numVendorModels; i++) {
		pDest->vendor_models[i].vendor_id = *pu16;
		pu16++;
		pDest->vendor_models[i].model_id = *pu16;
		pu16++;

		printf("vendor ID: %4.4x, model ID: %4.4x\r\n", pDest->vendor_models[i].vendor_id, pDest->vendor_models[i].model_id);
	}
}

static void DCD_decode()
{
	tsDCD_Header *pHeader;
	tsDCD_Elem *pElem;
	uint8 byte_offset;

	pHeader = (tsDCD_Header *)&_dcd_raw;

	printf("DCD: company ID %4.4x, Product ID %4.4x\r\n", pHeader->companyID, pHeader->productID);

	pElem = (tsDCD_Elem *)pHeader->payload;

	// decode primary element:
	DCD_decode_element(pElem, &_sDCD_Prim);

	// check if DCD has more than one element by calculating where we are currently at the raw
	// DCD array and compare against the total size of the raw DCD:
	byte_offset = 10 + 4 + pElem->numSIGModels * 2 + pElem->numVendorModels * 4; // +10 for DCD header, +4 for header in the DCD element

	if(byte_offset < _dcd_raw_len)
	{
		// set elem pointer to the beginning of 2nd element:
		pElem = (tsDCD_Elem *)&(_dcd_raw[byte_offset]);

		printf("Decoding 2nd element (just informative, not used for anything)\r\n");
		DCD_decode_element(pElem, &_sDCD_2nd);
	}
}

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

#define LIGHT_CTRL_GRP_ADDR     0xC001
#define LIGHT_STATUS_GRP_ADDR   0xC002

#define VENDOR_GRP_ADDR         0xC003

/* models used by simple light example (on/off only)
 * The beta SDK 1.0.1 and 1.1.0 examples are based on these
 * */
#define LIGHT_MODEL_ID            0x1000 // Generic On/Off Server
#define SWITCH_MODEL_ID           0x1001 // Generic On/Off Client

/*
 * Lightness models used in the dimming light example of 1.2.0 SDK
 * */
#define DIM_LIGHT_MODEL_ID              0x1300 // Light Lightness Server
#define DIM_SWITCH_MODEL_ID             0x1302 // Light Lightness Client


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

/*
 * This function scans for the SIG models in the DCD that was read from a freshly provisioned node.
 * Based on the models that are listed, the publish/subscribe addresses are added into a configuration list
 * that is later used to configure the node.
 *
 * This example configures generic on/off client and lightness client to publish
 * to "light control" group address and subscribe to "light status" group address.
 *
 * Similarly, generic on/off server and lightness server (= the light node) models
 * are configured to subscribe to "light control" and publish to "light status" group address.
 *
 * Alternative strategy for automatically filling the configuration data would be to e.g. use the product ID from the DCD.
 *
 * NOTE: this example only checks the primary element of the node. Other elements are ignored.
 * */
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

#ifdef CONFIGURE_VENDOR_MODEL
	// scan the vendor models found in the DCD
	for(i=0;i<_sDCD_Prim.numVendorModels;i++)
	{
		// this example only handles vendor model with vendor ID 0x02FF (Silabs) and model ID 0xABCD.
		// if such model found, configure it to publish/subscribe to a single group address
		if((_sDCD_Prim.vendor_models[i].model_id == 0xABCD) && (_sDCD_Prim.vendor_models[i].vendor_id == 0x02FF))
		{
			config_pub_add(0xABCD, 0x02FF, VENDOR_GRP_ADDR);
			config_sub_add(0xABCD, 0x02FF, VENDOR_GRP_ADDR);
			// using single appkey to bind all models. It could be also possible to use different appkey for the
			// vendor models
			config_bind_add(0xABCD, 0x02FF, 0, 0);
		}
	}

#endif
}


static void config_retry(uint8 timer_handle)
{
	/* maximum number of retry attempts is limited to 5 in this example. If the limit
	 * is reached, then there is probably something wrong with the configuration and
	 * there is no point to do try again anymore */
	const uint8 max_retrycount = 5;

	if(config_retrycount > max_retrycount)
	{
		printf("ERROR: max limit of configuration retries reached\r\n");
	}
	else
	{
		config_retrycount++;
	}

	printf(" trying again, attempt %u/%u\r\n", config_retrycount, max_retrycount);
	gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(500), timer_handle, 1);

}

static void trigger_next_state(uint8 timer_handle)
{
	// when moving to new state in the configuration state machine, the retry counter is cleared:
	config_retrycount = 0;

	gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(100), timer_handle, 1);
}

/**
 * Handling of stack events. Both Bluetooth LE and Bluetooth mesh events are handled here.
 */
static void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt)
{
  if (NULL == evt) {
    return;
  }

  switch (evt_id) {
    case gecko_evt_system_boot_id:
      // check pushbutton state at startup. If either PB0 or PB1 is held down then do factory reset
      if ((GPIO_PinInGet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN) == 0) || (GPIO_PinInGet(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN) == 0)) {
        initiate_factory_reset();
      } else {
    	  printf("Initializing as provisioner\r\n");

    	  state = init;
    	  // init as provisioner
    	  struct gecko_msg_mesh_prov_init_rsp_t *prov_init_rsp = gecko_cmd_mesh_prov_init();
    	  if (prov_init_rsp->result == 0) {
    		  printf("Successfully initialized\r\n");
    	  } else {
    		  printf("Error initializing node as provisioner. Error %x\r\n", prov_init_rsp->result);
    	  }
      }
      break;

    case gecko_evt_hardware_soft_timer_id:
      switch (evt->data.evt_hardware_soft_timer.handle) {

      case TIMER_ID_BUTTON_POLL:
    	  button_poll();
    	  break;

      case TIMER_ID_GET_DCD:
      {
    	  // clear the old DCD from memory before requesting new one:
    	  _dcd_raw_len = 0;

    	  struct gecko_msg_mesh_config_client_get_dcd_rsp_t* get_dcd_result = gecko_cmd_mesh_config_client_get_dcd(netkey_id, provisionee_address, 0);
    	  if (get_dcd_result->result == 0x0181) {
    		  printf("."); fflush(stdout);
    		  gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(1000), TIMER_ID_GET_DCD, 1);
    	  } else if(get_dcd_result->result != 0x0){
    		  printf("gecko_cmd_mesh_config_client_get_dcd failed with result 0x%X (%s) addr %x\r\n", get_dcd_result->result, res2str(get_dcd_result->result), provisionee_address);
    		  gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(1000), TIMER_ID_GET_DCD, 1);
    	  }
    	  else
    	  {
    		  printf("requesting DCD from the node...\r\n");
    		  state = waiting_dcd;
    	  }

      }
      break;

      case TIMER_ID_APPKEY_ADD:
      {
    	  struct gecko_msg_mesh_config_client_add_appkey_rsp_t *appkey_deploy_evt;
    	  gecko_cmd_mesh_config_client_add_appkey(netkey_id, provisionee_address, appkey_id, netkey_id);
    	  if (appkey_deploy_evt->result == 0) {
    		  printf("Deploying appkey to node 0x%4.4x\r\n", provisionee_address);
    		  state = waiting_appkey_ack;
    	  }else{
    		  printf("Appkey deployment failed. addr %x, error: %x\r\n", provisionee_address, appkey_deploy_evt->result);
    		  // try again:
    		  config_retry(TIMER_ID_APPKEY_ADD);
    	  }
      }
    	  break;

      case TIMER_ID_APPKEY_BIND:
      {
    	  uint16 vendor_id;
    	  uint16 model_id;

    	  // take the next model from the list of models to be bound with application key.
    	  // for simplicity, the same appkey is used for all models but it is possible to also use several appkeys
    	  model_id = _sConfig.bind_model[_sConfig.num_bind_done].model_id;
    	  vendor_id = _sConfig.bind_model[_sConfig.num_bind_done].vendor_id;

    	  printf("APP BIND, config %d/%d:: model %4.4x key index %x\r\n", _sConfig.num_bind_done+1, _sConfig.num_bind, model_id, appkey_id);

    	  struct gecko_msg_mesh_config_client_bind_model_rsp_t *model_app_bind_result = gecko_cmd_mesh_config_client_bind_model(netkey_id,
    			  provisionee_address,
    			  0, // element index
				  appkey_id,
				  vendor_id,
				  model_id);

    	  if(model_app_bind_result->result  == STATUS_OK)
    	  {
    		  printf(" waiting bind ack\r\n");
    		  state = waiting_bind_ack;
    	  }
    	  else if (model_app_bind_result->result == STATUS_BUSY) {
    		  printf("."); fflush(stdout);
    		  gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(500), TIMER_ID_APPKEY_BIND, 1);
    	  } else if(model_app_bind_result->result != STATUS_OK){
    		  printf("gecko_cmd_mesh_config_client_bind_model failed with result 0x%X\r\n", model_app_bind_result->result);
    		  // try again:
    		  config_retry(TIMER_ID_APPKEY_BIND);
    	  }
      }
      break;

      case TIMER_ID_PUB_SET:
      {
    	  uint16 vendor_id;
    	  uint16 model_id;
    	  uint16 pub_address;

    	  // get the next model/address pair from the configuration list:
    	  model_id = _sConfig.pub_model[_sConfig.num_pub_done].model_id;
    	  vendor_id = _sConfig.pub_model[_sConfig.num_pub_done].vendor_id;
    	  pub_address = _sConfig.pub_address[_sConfig.num_pub_done];

    	  printf("PUB SET, config %d/%d: model %4.4x -> address %4.4x\r\n", _sConfig.num_pub_done+1, _sConfig.num_pub, model_id, pub_address);

    	  struct gecko_msg_mesh_config_client_set_model_pub_rsp_t *model_pub_set_result = gecko_cmd_mesh_config_client_set_model_pub(netkey_id,
    			  provisionee_address,
				  0, /* element index */
				  vendor_id,
				  model_id,
				  pub_address,
				  appkey_id,
				  0, /* friendship credential flag */
				  3, /* Publication time-to-live value */
				  0, /* period = NONE */
				  0, /* Publication retransmission count */
				  50  /* Publication retransmission interval */
    	  );

    	  if (model_pub_set_result->result == STATUS_OK)
    	  {
    		  printf(" waiting pub ack\r\n");
    		  state = waiting_pub_ack;
    	  }
    	  else if (model_pub_set_result->result == STATUS_BUSY) {
    		  printf("."); fflush(stdout);
    		  gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(500), TIMER_ID_PUB_SET, 1);
    	  } else if(model_pub_set_result->result != STATUS_OK){
    		  printf("gecko_cmd_mesh_config_client_set_model_pub failed with result 0x%X\r\n", model_pub_set_result->result);
    	  }
      }
      break;

      case TIMER_ID_SUB_ADD:
      {
    	  uint16 vendor_id = 0xFFFF;
    	  uint16 model_id;
    	  uint16 sub_address;

    	  // get the next model/address pair from the configuration list:
    	  model_id = _sConfig.sub_model[_sConfig.num_sub_done].model_id;
    	  vendor_id = _sConfig.sub_model[_sConfig.num_sub_done].vendor_id;
    	  sub_address = _sConfig.sub_address[_sConfig.num_sub_done];

    	  printf("SUB ADD, config %d/%d: model %4.4x -> address %4.4x\r\n", _sConfig.num_sub_done+1, _sConfig.num_sub, model_id, sub_address);

    	  struct gecko_msg_mesh_config_client_add_model_sub_rsp_t *model_sub_add_result = gecko_cmd_mesh_config_client_add_model_sub(netkey_id,
    			  provisionee_address,
    			  0, /* element index */
				  vendor_id,
				  model_id,
				  sub_address);

    	  if (model_sub_add_result->result == STATUS_OK)
    	  {
    		  printf(" waiting sub ack\r\n");
    		  state = waiting_sub_ack;
    	  }
    	  if (model_sub_add_result->result == STATUS_BUSY) {
    		  printf("."); fflush(stdout);
    	  } else if(model_sub_add_result->result != STATUS_OK){
    		  printf("gecko_cmd_mesh_config_client_add_model_sub failed with result 0x%X\r\n",model_sub_add_result->result);
    	  }


      }
      break;

        case TIMER_ID_FACTORY_RESET:
          gecko_cmd_system_reset(0);
          break;

        case TIMER_ID_RESTART:
          gecko_cmd_system_reset(0);
          break;

        default:
          break;
      }

      break;

      case gecko_evt_mesh_config_client_dcd_data_id:
      {
    	  struct gecko_msg_mesh_config_client_dcd_data_evt_t *pDCD = (struct gecko_msg_mesh_config_client_dcd_data_evt_t *)&(evt->data);
    	  printf("DCD data event, received %u bytes\r\n", pDCD->data.len);

    	  // copy the data into one large array. the data may come in multiple smaller pieces.
    	  // the data is not decoded until all DCD events have been received (see below)
    	  if((_dcd_raw_len + pDCD->data.len) <= 256)
    	  {
    		  memcpy(&(_dcd_raw[_dcd_raw_len]), pDCD->data.data, pDCD->data.len);
    		  _dcd_raw_len += pDCD->data.len;
    	  }
      }

    	  break;

      case gecko_evt_mesh_config_client_dcd_data_end_id:
      {
    	  uint16 res;

    	  res = evt->data.evt_mesh_config_client_dcd_data_end.result;
    	  if(res != bg_err_success)
    	  {
    		  printf("reading DCD failed with code 0x%x (%s)\r\n", res, res2str(res));
    		  // try again:
    		  config_retry(TIMER_ID_GET_DCD);
    	  }
    	  else
    	  {
    		  printf("DCD data end event. Decoding the data.\r\n");
    		  // decode the DCD content
    		  DCD_decode();

    		  // check the desired configuration settings depending on what's in the DCD
    		  config_check();

    		  // sanity check: make sure there is at least one application key to be bound to a model
    		  if(_sConfig.num_bind == 0)
    		  {
    			  printf("ERROR: don't know how to configure this node, no appkey bindings defined\r\n");
    		  }
    		  else
    		  {
    			  // next step : send appkey to device
    			  trigger_next_state(TIMER_ID_APPKEY_ADD);
    		  }
    	  }
      }
    	  break;

      case gecko_evt_mesh_config_client_appkey_status_id:
    	  /* This event is created when a response for an add application key or a remove
    	   * application key request is received, or the request times out.  */
    	  if(state == waiting_appkey_ack)
    	  {
    		  uint16 res = evt->data.evt_mesh_config_client_appkey_status.result;
    		  if(res == bg_err_success)
    		  {
    			  printf(" appkey added\r\n");
    			  // move to next step which is binding appkey to models
    			  trigger_next_state(TIMER_ID_APPKEY_BIND);
    		  }
    		  else
    		  {
    			  printf(" add appkey failed with code 0x%x (%s)\r\n", res, res2str(res));
    			  // try again:
    			  config_retry(TIMER_ID_APPKEY_ADD);
    		  }
    	  }
    	  else
    	  {
    		  printf("ERROR: unexpected appkey status event? (state = %u)\r\n", state);
    	  }
    	break;

      case gecko_evt_mesh_config_client_binding_status_id:
    	  /* Status event for binding and unbinding application keys and models. */
    	  if(state == waiting_bind_ack)
    	  {
    		  uint16 res = evt->data.evt_mesh_config_client_appkey_status.result;
    		  if(res == bg_err_success)
    		  {
    			  printf(" bind complete\r\n");
    			  _sConfig.num_bind_done++;

    			  if(_sConfig.num_bind_done < _sConfig.num_bind)
    			  {
    				  // more model<->appkey bindings to be done
    				  trigger_next_state(TIMER_ID_APPKEY_BIND);
    			  }
    			  else
    			  {
    				  // move to next step which is configuring publish settings
    				  trigger_next_state(TIMER_ID_PUB_SET);
    			  }
    		  }
    		  else
    		  {
    			  printf(" appkey bind failed with code 0x%x (%s)\r\n", res, res2str(res));
    			  // try again:
    			  config_retry(TIMER_ID_APPKEY_BIND);
    		  }
    	  }
    	  else
    	  {
    		  printf("ERROR: unexpected binding status event? (state = %u)\r\n", state);
    	  }
    	  break;

      case gecko_evt_mesh_config_client_model_pub_status_id:
    	  /*Status event for get model publication state, set model publication state, commands. */
    	  if(state == waiting_pub_ack)
    	  {
    		  uint16 res = evt->data.evt_mesh_config_client_model_pub_status.result;
    		  if(res == bg_err_success)
    		  {
    			  printf(" pub set OK\r\n");
    			  _sConfig.num_pub_done++;

    			  if(_sConfig.num_pub_done < _sConfig.num_pub)
    			  {
    				  // more publication settings to be done
    				  trigger_next_state(TIMER_ID_PUB_SET);
    			  }
    			  else
    			  {
    				  // move to next step which is configuring subscription settings
    				  trigger_next_state(TIMER_ID_SUB_ADD);
    			  }
    		  }
    		  else
    		  {
    			  printf(" setting pub failed with code 0x%x (%s)\r\n", res, res2str(res));
    			  // try again:
    			  config_retry(TIMER_ID_PUB_SET);
    		  }
    	  }
    	  else
    	  {
    		  printf("ERROR: unexpected pub status event? (state = %u)\r\n", state);
    	  }
    	  break;

      case gecko_evt_mesh_config_client_model_sub_status_id:
    	  /* status event for subscription changes */
    	  if(state == waiting_sub_ack)
    	  {
    		  uint16 res = evt->data.evt_mesh_config_client_model_sub_status.result;
    		  if(res == bg_err_success)
    		  {
    			  printf(" sub add OK\r\n");
    			  _sConfig.num_sub_done++;
    			  if(_sConfig.num_sub_done < _sConfig.num_sub)
    			  {
    				  // more subscription settings to be done
    				  trigger_next_state(TIMER_ID_SUB_ADD);
    			  }
    			  else
    			  {
    				  printf("***\r\nconfiguration complete\r\n***\r\n");
#ifdef PROVISION_OVER_GATT
    				  // close connection if provisioning was done over PB-GATT:
    				  printf("closing connection...\r\n");
    				  gecko_cmd_le_connection_close(conn_handle);

    				  // restart scanning of unprov beacons
    				  struct gecko_msg_mesh_prov_scan_unprov_beacons_rsp_t *scan_rsp;
    				  scan_rsp = gecko_cmd_mesh_prov_scan_unprov_beacons();

    				  if (scan_rsp->result == 0) {
    					  state = scanning;
    				  } else {
    					  printf("Failure initializing unprovisioned beacon scan. Result: %x\r\n", scan_rsp->result);
    				  }
#else
    				  state = scanning;
#endif
    			  }
    		  }
    		  else
    		  {
    			  printf(" setting sub failed with code 0x%x (%s)\r\n", res, res2str(res));
    			  // try again:
    			  config_retry(TIMER_ID_SUB_ADD);
    		  }
    	  }
    	  else
    	  {
    		  printf("ERROR: unexpected sub status event? (state = %u)\r\n", state);
    	  }
    	  break;

    case gecko_evt_le_gap_adv_timeout_id:
      // adv timeout events silently discarded
      break;

    case gecko_evt_le_connection_opened_id:
      printf("connection opened\r\n");
      num_connections++;
      conn_handle = evt->data.evt_le_connection_opened.connection;
      LCD_write("connected", LCD_ROW_CONNECTION);

      if(state == connecting)
      {
    	  printf("connection opened, proceeding provisioning over GATT\r\n");
    	  struct gecko_msg_mesh_prov_provision_gatt_device_rsp_t *prov_resp_adv;
    	  prov_resp_adv = gecko_cmd_mesh_prov_provision_gatt_device(netkey_id, conn_handle, 16, _uuid_copy);

    	  if (prov_resp_adv->result == 0) {
    		  state = provisioning;
    	  } else {
    		  printf("ERROR: gecko_cmd_mesh_prov_provision_gatt_device failed with code %x\r\n", prov_resp_adv->result);
    	  }
      }
      break;

    case gecko_evt_le_connection_parameters_id:
    {
    	struct gecko_msg_le_connection_parameters_evt_t *p = &(evt->data.evt_le_connection_parameters);
    	printf("connection params: interval %u, latency %u, timeout %u, security %u, txsize %u\r\n", \
    			p->interval, p->latency, p->timeout, p->security_mode, p->txsize);
    }
    break;

    case gecko_evt_le_connection_closed_id:

      printf("connection closed, reason 0x%x\r\n", evt->data.evt_le_connection_closed.reason);
      conn_handle = 0xFF;
      if (num_connections > 0) {
        if (--num_connections == 0) {
          LCD_write("", LCD_ROW_CONNECTION);
        }
      }
      break;

    case gecko_evt_gatt_server_user_write_request_id:

      break;

    case gecko_evt_system_external_signal_id:
    {

    }
    break;

    case gecko_evt_mesh_prov_initialized_id:
    {
    	struct gecko_msg_mesh_prov_initialized_evt_t *initialized_evt;
    	initialized_evt = (struct gecko_msg_mesh_prov_initialized_evt_t *)&(evt->data);

    	printf("gecko_cmd_mesh_prov_init_id\r\n");
    	printf("networks: %x\r\n", initialized_evt->networks);
    	printf("address: %x\r\n", initialized_evt->address);
    	printf("ivi: %x\r\n", (unsigned int)initialized_evt->ivi);

    	LCD_write("provisioner", LCD_ROW_STATUS);

    	if (initialized_evt->networks > 0) {
    		printf("network keys already exist\r\n");
    		netkey_id = 0;
    		appkey_id = 0;
    	} else {
    		printf("Creating a new netkey\r\n");

    		struct gecko_msg_mesh_prov_create_network_rsp_t *new_netkey_rsp;
#ifdef USE_FIXED_KEYS
    		new_netkey_rsp = gecko_cmd_mesh_prov_create_network(16, fixed_netkey);
#else
    		new_netkey_rsp = gecko_cmd_mesh_prov_create_network(0, (const uint8 *)"");
#endif

    		if (new_netkey_rsp->result == 0) {
    			netkey_id = new_netkey_rsp->network_id;
    			printf("Success, netkey id = %x\r\n", netkey_id);
    		} else {
    			printf("Failed to create new netkey. Error: %x", new_netkey_rsp->result);
    		}

    		printf("Creating a new appkey\r\n");


    		struct gecko_msg_mesh_prov_create_appkey_rsp_t *new_appkey_rsp;

#ifdef USE_FIXED_KEYS
    		new_appkey_rsp = gecko_cmd_mesh_prov_create_appkey(netkey_id, 16, fixed_appkey);
#else
    		new_appkey_rsp = gecko_cmd_mesh_prov_create_appkey(netkey_id, 0, (const uint8 *)"");
#endif

    		if (new_netkey_rsp->result == 0) {
    			appkey_id = new_appkey_rsp->appkey_index;
    			printf("Success, appkey_id = %x\r\n", appkey_id);
    			printf("Appkey: ");
    			for (uint32_t i = 0; i < new_appkey_rsp->key.len; ++i) {
    				printf("%02x ", new_appkey_rsp->key.data[i]);
    			}
    			printf("\r\n");
    		} else {
    			printf("Failed to create new appkey. Error: %x", new_appkey_rsp->result);
    		}
    	}

    	printf("Starting to scan for unprovisioned device beacons\r\n");

    	struct gecko_msg_mesh_prov_scan_unprov_beacons_rsp_t *scan_rsp;
    	scan_rsp = gecko_cmd_mesh_prov_scan_unprov_beacons();

    	if (scan_rsp->result == 0) {
    		state = scanning;
    	} else {
    		printf("Failure initializing unprovisioned beacon scan. Result: %x\r\n", scan_rsp->result);
    	}

    	// start timer for button polling
    	 gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(100), TIMER_ID_BUTTON_POLL, 0);

    	break;
    }

    case gecko_evt_mesh_prov_unprov_beacon_id:
    {
    	struct gecko_msg_mesh_prov_unprov_beacon_evt_t *beacon_evt = (struct gecko_msg_mesh_prov_unprov_beacon_evt_t *)&(evt->data);
    	int i;

    	// this example can handle only one bearer type at a time: either PB-GATT or PB-ADV
#ifdef PROVISION_OVER_GATT
#define bearer_type 1
#else
#define bearer_type 0
#endif

    	if ((state == scanning) && (ask_user_input == false) && (evt->data.evt_mesh_prov_unprov_beacon.bearer == bearer_type)) {
			printf("Unprovisioned beacon, UUID: ");

			for(i=0;i<beacon_evt->uuid.len;i++)
			{
				printf("%2.2x", beacon_evt->uuid.data[i]);
			}

			// show also the same shortened version that is used on the LCD of switch / light examples
			printf(" (%2.2x %2.2x)", beacon_evt->uuid.data[11], beacon_evt->uuid.data[10]);
			printf("type: %s", evt->data.evt_mesh_prov_unprov_beacon.bearer ? "PB-GATT" : "PB-ADV");
			printf("\r\n");

			memcpy(_uuid_copy, beacon_evt->uuid.data, 16);
#ifdef PROVISION_OVER_GATT
			bt_address = beacon_evt->address;
			bt_address_type = beacon_evt->address_type;
#endif
			printf("-> confirm? (use buttons or keys 'y' / 'n')\r\n");
			// suspend reporting of unprov beacons until user has rejected or accepted this one using buttons PB0 / PB1
			ask_user_input = true;
    	}
    	break;
    }

    case gecko_evt_mesh_prov_provisioning_failed_id:
    {
    	struct gecko_msg_mesh_prov_provisioning_failed_evt_t *fail_evt = (struct gecko_msg_mesh_prov_provisioning_failed_evt_t*)&(evt->data);

    	printf("Provisioning failed. Reason: %x\r\n", fail_evt->reason);
    	state = scanning;

    	break;
    }

    case gecko_evt_mesh_prov_device_provisioned_id:
    {
    	struct gecko_msg_mesh_prov_device_provisioned_evt_t *prov_evt = (struct gecko_msg_mesh_prov_device_provisioned_evt_t*)&(evt->data);

    	printf("Node successfully provisioned. Address: %4.4x, ", prov_evt->address);
    	state = provisioned;

    	printf("uuid 0x");
    	for (uint8_t i = 0; i < prov_evt->uuid.len; i++) printf("%02X", prov_evt->uuid.data[i]);
    	printf("\r\n");

    	provisionee_address = prov_evt->address;

    	/* kick of next phase which is reading DCD from the newly provisioned node */
    	trigger_next_state(TIMER_ID_GET_DCD);
    	break;
    }

    /* these events are silently ignored */
    case gecko_evt_gatt_mtu_exchanged_id:
    case gecko_evt_gatt_procedure_completed_id:
    case gecko_evt_gatt_service_id:
    case gecko_evt_gatt_characteristic_id:
    case gecko_evt_gatt_characteristic_value_id:
    case gecko_evt_le_connection_phy_status_id:
    	break;

    default:
      printf("unhandled evt: %8.8x class %2.2x method %2.2x\r\n", (unsigned int)evt_id, (unsigned int)((evt_id >> 16) & 0xFF), (unsigned int)((evt_id >> 24) & 0xFF));
      break;
  }
}
