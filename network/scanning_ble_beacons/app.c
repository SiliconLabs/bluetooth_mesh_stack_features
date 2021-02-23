/***************************************************************************//**
 * @file
 * @brief Application code: Scanning BLE beacons in Bluetooth Mesh mode.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

/* C Standard Library headers */
#include <stdio.h>

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"
#include "mesh_generic_model_capi_types.h"
#include "mesh_lib.h"

/* GPIO peripheral library */
#include <em_gpio.h>

/* Lightbulb with mesh models */
#include "lightbulb.h"

/* Display Interface header */
#include "display_interface.h"

/* LED driver with support for PWM dimming */
#include "led_driver.h"

/* Own header */
#include "app.h"

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

/*******************************************************************************
 * Timer handles defines.
 ******************************************************************************/
#define TIMER_ID_RESTART    78
#define TIMER_ID_FACTORY_RESET  77
#define TIMER_ID_PROVISIONING   66

/// Flag for indicating DFU Reset must be performed
static uint8_t boot_to_dfu = 0;
/// Address of the Primary Element of the Node
static uint16 _my_address = 0;
/// Number of active Bluetooth connections
static uint8 num_connections = 0;
/// Handle of the last opened LE connection
static uint8 conn_handle = 0xFF;
/// Flag for indicating that initialization was performed
static uint8 init_done = 0;

/***************************************************************************//**
 * This function is called to initiate factory reset. Factory reset may be
 * initiated by keeping one of the WSTK pushbuttons pressed during reboot.
 * Factory reset is also performed if it is requested by the provisioner
 * (event gecko_evt_mesh_node_reset_id).
 ******************************************************************************/
static void initiate_factory_reset(void)
{
  printf("factory reset\r\n");
  DI_Print("\n***\nFACTORY RESET\n***", DI_ROW_STATUS);

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

/***************************************************************************//**
 * Set device name in the GATT database. A unique name is generated using
 * the two last bytes from the Bluetooth address of this device. Name is also
 * displayed on the LCD.
 *
 * @param[in] pAddr  Pointer to Bluetooth address.
 ******************************************************************************/
static void set_device_name(bd_addr *pAddr)
{
  char name[20];
  uint16 res;

  // create unique device name using the last two bytes of the Bluetooth address
  sprintf(name, "light node %02x:%02x", pAddr->addr[1], pAddr->addr[0]);

  printf("Device name: '%s'\r\n", name);

  // write device name to the GATT database
  res = gecko_cmd_gatt_server_write_attribute_value(gattdb_device_name, 0, strlen(name), (uint8 *)name)->result;
  if (res) {
    printf("gecko_cmd_gatt_server_write_attribute_value() failed, code %x\r\n", res);
  }

  // show device name on the LCD
  DI_Print(name, DI_ROW_NAME);
}

/***************************************************************************//**
 * This function prints debug information for mesh server state change event.
 *
 * @param[in] pEvt  Pointer to mesh_lib_generic_server_state_changed event.
 ******************************************************************************/
static void server_state_changed(struct gecko_msg_mesh_generic_server_state_changed_evt_t *pEvt)
{
  int i;

  printf("state changed: ");
  printf("model ID %4.4x, type %2.2x ", pEvt->model_id, pEvt->type);
  for (i = 0; i < pEvt->parameters.len; i++) {
    printf("%2.2x ", pEvt->parameters.data[i]);
  }
  printf("\r\n");
}

/***************************************************************************//**
 * This function looks through advertising data in scan response events and searches
 * for the mesh proxy UUID 0x1828. If a proxy advertisement is found, the address of the
 * sender and RSSI are printer to debug output.
 *
 * @param[in] pResp  Pointer to gecko_msg_le_gap_scan_response_evt_t event.
 ******************************************************************************/
static void print_scan_resp(struct gecko_msg_le_gap_scan_response_evt_t *pResp)
{
	// decoding advertising packets is done here. The list of AD types can be found
	// at: https://www.bluetooth.com/specifications/assigned-numbers/Generic-Access-Profile

	// example of adv data including proxy service data:020106030328180c16281800f03f4f79774c65a2
	// (UUID 0x1828)

	// 020106-03032818-0c16281800f03f4f79774c65a2
	const uint8 proxy_UUID[16] = {0x28, 0x18};

	int i = 0;
	int ad_match_found = 0;
	int ad_len;
	int ad_type;

	while (i < (pResp->data.len - 1))
	{

		ad_len  = pResp->data.data[i];
		ad_type = pResp->data.data[i+1];

		if (ad_type == 0x03)
		{
			// type 0x03= Complete List of 16-bit Service Class UUIDs

			if(memcmp(proxy_UUID, &(pResp->data.data[i+2]),2) == 0)
			{
				ad_match_found = 1;
			}
		}

		//jump to next AD record
		i = i + ad_len + 1;
	}

	if(ad_match_found)
	{
		for(i=5;i>=0;i--)
		{
			printf("%2.2x", pResp->address.addr[i]);
		}
	printf(", RSSI: %d\r\n", pResp->rssi);
	}
}

/*******************************************************************************
 * Initialise used bgapi classes.
 ******************************************************************************/
void gecko_bgapi_classes_init(void)
{
  gecko_bgapi_class_dfu_init();
  gecko_bgapi_class_system_init();
  gecko_bgapi_class_le_gap_init();
  gecko_bgapi_class_le_connection_init();
  //gecko_bgapi_class_gatt_init();
  gecko_bgapi_class_gatt_server_init();
  gecko_bgapi_class_hardware_init();
  gecko_bgapi_class_flash_init();
  gecko_bgapi_class_test_init();
  //gecko_bgapi_class_sm_init();
  //mesh_native_bgapi_init();
  gecko_bgapi_class_mesh_node_init();
  //gecko_bgapi_class_mesh_prov_init();
  gecko_bgapi_class_mesh_proxy_init();
  gecko_bgapi_class_mesh_proxy_server_init();
  //gecko_bgapi_class_mesh_proxy_client_init();
  //gecko_bgapi_class_mesh_generic_client_init();
  gecko_bgapi_class_mesh_generic_server_init();
  //gecko_bgapi_class_mesh_vendor_model_init();
  //gecko_bgapi_class_mesh_health_client_init();
  //gecko_bgapi_class_mesh_health_server_init();
  //gecko_bgapi_class_mesh_test_init();
  //gecko_bgapi_class_mesh_lpn_init();
  gecko_bgapi_class_mesh_friend_init();
}

/*******************************************************************************
 * Handling of stack events. Both Bluetooth LE and Bluetooth mesh events
 * are handled here.
 * @param[in] evt_id  Incoming event ID.
 * @param[in] evt     Pointer to incoming event.
 ******************************************************************************/
void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt)
{
  uint16_t result;
  char buf[30];

  if (NULL == evt) {
    return;
  }

  switch (evt_id) {
    case gecko_evt_system_boot_id:
      // check pushbutton state at startup. If either PB0 or PB1 is held down then do factory reset
      if (GPIO_PinInGet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN) == 0 || GPIO_PinInGet(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN) == 0) {
        initiate_factory_reset();
      } else {
        struct gecko_msg_system_get_bt_address_rsp_t *pAddr = gecko_cmd_system_get_bt_address();

        set_device_name(&pAddr->address);

        // Initialize Mesh stack in Node operation mode, it will generate initialized event
        result = gecko_cmd_mesh_node_init()->result;
        if (result) {
          sprintf(buf, "init failed (0x%x)", result);
          DI_Print(buf, DI_ROW_STATUS);
        }

        // re-initialize LEDs (needed for those radio board that share same GPIO for button/LED)
        LEDS_init();

        //Configure the advertising event filter
        gecko_cmd_mesh_node_set_adv_event_filter(0x7,0,NULL);
      }
      break;

    case gecko_evt_hardware_soft_timer_id:
      switch (evt->data.evt_hardware_soft_timer.handle) {
        case TIMER_ID_FACTORY_RESET:
          // reset the device to finish factory reset
          gecko_cmd_system_reset(0);
          break;

        case TIMER_ID_RESTART:
          // restart timer expires, reset the device
          gecko_cmd_system_reset(0);
          break;

        case TIMER_ID_PROVISIONING:
          // toggle LED to indicate the provisioning state
          if (!init_done) {
            LEDS_SetState(LED_STATE_PROV);
          }
          break;

        default:
          // lightbulb related timer events are handled by separate function
          handle_lightbulb_timer_evt(evt);
          break;
      }
      break;

    case gecko_evt_mesh_node_initialized_id:
      printf("node initialized\r\n");

      // Initialize generic server models
      gecko_cmd_mesh_generic_server_init();

      struct gecko_msg_mesh_node_initialized_evt_t *pData = (struct gecko_msg_mesh_node_initialized_evt_t *)&(evt->data);

      if (pData->provisioned) {
        printf("node is provisioned. address:%x, ivi:%ld\r\n", pData->address, pData->ivi);

        _my_address = pData->address;
        lightbulb_state_init();
        init_done = 1;

        DI_Print("provisioned", DI_ROW_STATUS);
      } else {
        printf("node is unprovisioned\r\n");
        DI_Print("unprovisioned", DI_ROW_STATUS);

        printf("starting unprovisioned beaconing...\r\n");
        gecko_cmd_mesh_node_start_unprov_beaconing(0x3);   // enable ADV and GATT provisioning bearer
      }
      break;

    case gecko_evt_mesh_node_provisioning_started_id:
      printf("Started provisioning\r\n");
      DI_Print("provisioning...", DI_ROW_STATUS);
      // start timer for blinking LEDs to indicate which node is being provisioned
      gecko_cmd_hardware_set_soft_timer(32768 / 4, TIMER_ID_PROVISIONING, 0);
      break;

    case gecko_evt_mesh_node_provisioned_id:
      lightbulb_state_init();
      init_done = 1;
      printf("node provisioned, got address=%x\r\n", evt->data.evt_mesh_node_provisioned.address);
      // stop LED blinking when provisioning complete
      gecko_cmd_hardware_set_soft_timer(0, TIMER_ID_PROVISIONING, 0);
      DI_Print("provisioned", DI_ROW_STATUS);
      break;

    case gecko_evt_mesh_node_provisioning_failed_id:
      printf("provisioning failed, code %x\r\n", evt->data.evt_mesh_node_provisioning_failed.result);
      DI_Print("prov failed", DI_ROW_STATUS);
      /* start a one-shot timer that will trigger soft reset after small delay */
      gecko_cmd_hardware_set_soft_timer(2 * 32768, TIMER_ID_RESTART, 1);
      break;

    case gecko_evt_mesh_node_key_added_id:
      printf("got new %s key with index %x\r\n", evt->data.evt_mesh_node_key_added.type == 0 ? "network" : "application",
             evt->data.evt_mesh_node_key_added.index);
      break;

    case gecko_evt_mesh_node_model_config_changed_id:
      printf("model config changed\r\n");
      break;

    case gecko_evt_mesh_generic_server_client_request_id:
      printf("evt gecko_evt_mesh_generic_server_client_request_id\r\n");
      // pass the server client request event to mesh lib handler that will invoke
      // the callback functions registered by application
      mesh_lib_generic_server_event_handler(evt);
      break;

    case gecko_evt_mesh_generic_server_state_changed_id:

      // uncomment following line to get debug prints for each server state changed event
      //server_state_changed(&(evt->data.evt_mesh_generic_server_state_changed));

      // pass the server state changed event to mesh lib handler that will invoke
      // the callback functions registered by application
      mesh_lib_generic_server_event_handler(evt);
      break;

    case gecko_evt_mesh_node_reset_id:
      printf("evt gecko_evt_mesh_node_reset_id\r\n");
      initiate_factory_reset();
      break;

    case gecko_evt_mesh_friend_friendship_established_id:
      printf("evt gecko_evt_mesh_friend_friendship_established, lpn_address=%x\r\n", evt->data.evt_mesh_friend_friendship_established.lpn_address);
      DI_Print("FRIEND", DI_ROW_FRIEND);
      break;

    case gecko_evt_mesh_friend_friendship_terminated_id:
      printf("evt gecko_evt_mesh_friend_friendship_terminated, reason=%x\r\n", evt->data.evt_mesh_friend_friendship_terminated.reason);
      DI_Print("NO LPN", DI_ROW_FRIEND);
      break;

    case gecko_evt_le_gap_adv_timeout_id:
      // adv timeout events silently discarded
      break;

    case gecko_evt_le_connection_opened_id:
      printf("evt:gecko_evt_le_connection_opened_id\r\n");
      num_connections++;
      conn_handle = evt->data.evt_le_connection_opened.connection;
      DI_Print("connected", DI_ROW_CONNECTION);
      break;

    case gecko_evt_le_connection_parameters_id:
      printf("evt:gecko_evt_le_connection_parameters_id\r\n");
      break;

    case gecko_evt_le_connection_closed_id:
      /* Check if need to boot to dfu mode */
      if (boot_to_dfu) {
        /* Enter to DFU OTA mode */
        gecko_cmd_system_reset(2);
      }

      printf("evt:conn closed, reason 0x%x\r\n", evt->data.evt_le_connection_closed.reason);
      conn_handle = 0xFF;
      if (num_connections > 0) {
        if (--num_connections == 0) {
          DI_Print("", DI_ROW_CONNECTION);
        }
      }
      break;
    case gecko_evt_gatt_server_user_write_request_id:
      if (evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_ota_control) {
        /* Set flag to enter to OTA mode */
        boot_to_dfu = 1;
        /* Send response to Write Request */
        gecko_cmd_gatt_server_send_user_write_response(
          evt->data.evt_gatt_server_user_write_request.connection,
          gattdb_ota_control,
          bg_err_success);

        /* Close connection to enter to DFU OTA mode */
        gecko_cmd_le_connection_close(evt->data.evt_gatt_server_user_write_request.connection);
      }
      break;

    case gecko_evt_system_external_signal_id:
    {
      uint16_t current_level;
      int16_t current_delta;
      char tmp[21];

      if (init_done && (evt->data.evt_system_external_signal.extsignals & EXT_SIGNAL_LED_LEVEL_CHANGED)) {
        /* this signal from the LED PWM driver indicates that the level has changed,
         * we use it here to update the LCD status */
        current_level = LEDS_GetLevel();
        sprintf(tmp, "Lightness: %5u%%", (current_level * 100 + 99) / 65535);
        DI_Print(tmp, DI_ROW_LIGHTNESS);
      }

      if (init_done && (evt->data.evt_system_external_signal.extsignals & EXT_SIGNAL_LED_TEMPERATURE_CHANGED)) {
        /* this signal from the LED driver indicates that the temperature level has changed,
         * we use it here to update the LCD status */
        current_level = LEDS_GetTemperature();
        sprintf(tmp, "ColorTemp: %5uK", current_level);
        DI_Print(tmp, DI_ROW_TEMPERATURE);

        current_delta = LEDS_GetDeltaUV();
        sprintf(tmp, "Delta UV: %6d ", current_delta);
        DI_Print(tmp, DI_ROW_DELTAUV);
      }
    }
    break;

    case gecko_evt_le_gap_scan_response_id:
       	print_scan_resp(&(evt->data.evt_le_gap_scan_response));
    break;

    default:
      printf("unhandled evt: %8.8x class %2.2x method %2.2x\r\n", evt_id, (evt_id >> 16) & 0xFF, (evt_id >> 24) & 0xFF);
      break;
  }
}


/** @} (end addtogroup app) */
/** @} (end addtogroup Application) */
