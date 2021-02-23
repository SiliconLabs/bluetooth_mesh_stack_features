/***********************************************************************************************//**
 * \file   server.app.c
 * \brief  Silicon Labs BT Mesh Empty Example Project
 *
 * This example demonstrates the bare minimum needed for a Blue Gecko BT Mesh C application.
 * The application starts unprovisioned Beaconing after boot
 ***************************************************************************************************
 * <b> (C) Copyright 2017 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

/* Board headers */
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"
#include <gecko_configuration.h>
#include <mesh_sizes.h>

/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"
#include "em_rtcc.h"
#include <em_gpio.h>

/* Device initialization header */
#include "hal-config.h"

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif

#include "app.h"
#if (LOG_MODULE_PRESENT)
#define LOG_LEVEL                   LVL_VERBOSE
#undef  SUB_MODULE_NAME
#define SUB_MODULE_NAME             "client_app"
#include "log.h"
#endif

#include "my_model_def.h"

#ifdef PROV_LOCALLY
#include "self_test.h"

/* The default settings of the network and the node */
#define NET_KEY_IDX                 0
#define APP_KEY_IDX                 0
#define IVI                         0
#define DEFAULT_TTL                 5
/* #define ELEMENT_ID                  0 */
#endif /* #ifdef PROV_LOCALLY */

#define BUTTON0_PIN                 (6)
#define BUTTON0_PORT                (gpioPortF)
#define BUTTON1_PIN                 (7)
#define BUTTON1_PORT                (gpioPortF)
#define LED0_PIN                    (4)
#define LED0_PORT                   (gpioPortF)
#define LED1_PIN                    (5)
#define LED1_PORT                   (gpioPortF)

/* Timer Id used in this project */
#define RESET_TIMER_ID              62

// bluetooth stack heap
#define MAX_CONNECTIONS             1
#define EX_PB0_PRESS                ((1) << 5)
#define EX_PB0_LONG_PRESS           ((1) << 6)
#define EX_PB1_PRESS                ((1) << 7)
#define EX_PB1_LONG_PRESS           ((1) << 8)

#define RES_100_MILLI_TICKS         3277
#define RES_1_SEC_TICKS             (32768)
#define RES_10_SEC_TICKS            ((32768) * (10))
#define RES_10_MIN_TICKS            ((32768) * (60) * (10))

#define RES_100_MILLI               0
#define RES_1_SEC                   ((1) << 6)
#define RES_10_SEC                  ((2) << 6)
#define RES_10_MIN                  ((3) << 6)
#define RES_BIT_MASK                0xC0

// Max x is 63
#define SET_100_MILLI(x)            (uint8_t)(RES_100_MILLI | ((x) & (0x3F)))
#define SET_1_SEC(x)                (uint8_t)(RES_1_SEC | ((x) & (0x3F)))
#define SET_10_SEC(x)               (uint8_t)(RES_10_SEC | ((x) & (0x3F)))
#define SET_10_MIN(x)               (uint8_t)(RES_10_MIN | ((x) & (0x3F)))

uint32_t real_time_ticks = 0;
uint8_t update_interval = 0;
static unit_t unit = celsius;
uint8_t conn_handle = 0xFF;
static uint8_t pb0_state = 0;
static uint8_t pb1_state = 0;
static uint32_t pb1_counter = 0;
static uint32_t pb0_counter = 0;

uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS) + BTMESH_HEAP_SIZE + 2560];

// Bluetooth advertisement set configuration
//
// At minimum the following is required:
// * One advertisement set for Bluetooth LE stack (handle number 0)
// * One advertisement set for Mesh data (handle number 1)
// * One advertisement set for Mesh unprovisioned beacons (handle number 2)
// * One advertisement set for Mesh unprovisioned URI (handle number 3)
// * N advertisement sets for Mesh GATT service advertisements
// (one for each network key, handle numbers 4 .. N+3)
//
#define MAX_ADVERTISERS (4 + MESH_CFG_MAX_NETKEYS)

static gecko_bluetooth_ll_priorities linklayer_priorities = GECKO_BLUETOOTH_PRIORITIES_DEFAULT;
// bluetooth stack configuration
extern const struct bg_gattdb_def bg_gattdb_data;

// Flag for indicating DFU Reset must be performed
uint8_t boot_to_dfu = 0;

const gecko_configuration_t config =
{
  /* To enable low power functionality, remove comment from the next line. */
  /* .sleep.flags = SLEEP_FLAGS_DEEP_SLEEP_ENABLE, */
  .bluetooth.max_connections = MAX_CONNECTIONS,
  .bluetooth.max_advertisers = MAX_ADVERTISERS,
  .bluetooth.heap = bluetooth_stack_heap,
  .bluetooth.heap_size = sizeof(bluetooth_stack_heap) - BTMESH_HEAP_SIZE,
  .bluetooth.sleep_clock_accuracy = 100,
  .gattdb = &bg_gattdb_data,
  .bluetooth.linklayer_priorities = &linklayer_priorities,
  .btmesh_heap_size = BTMESH_HEAP_SIZE,
#if (HAL_PA_ENABLE)
  .pa.config_enable = 1,   // Set this to be a valid PA config
#if defined(FEATURE_PA_INPUT_FROM_VBAT)
  .pa.input = GECKO_RADIO_PA_INPUT_VBAT,     // Configure PA input to VBAT
#else
  .pa.input = GECKO_RADIO_PA_INPUT_DCDC,
#endif // defined(FEATURE_PA_INPUT_FROM_VBAT)
#endif // (HAL_PA_ENABLE)
/* #if (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER) */
  /* .pa.config_enable = 1, // Enable high power PA */
  /* .pa.input = GECKO_RADIO_PA_INPUT_VBAT, // Configure PA input to VBAT */
/* #endif // (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER) */
  .max_timers = 16,
};

static uint8_t index = 0;

uint8_t periods[] = {
  SET_100_MILLI(3),        /* 300ms */
  0,
  SET_100_MILLI(20),       /* 2s    */
  0,
  SET_1_SEC(10),           /* 10s   */
  0,
  SET_10_SEC(12),          /* 2min  */
  0,
  SET_10_MIN(1),           /* 10min */
  0
};

my_model_t my_model = {
  .elem_index = PRIMARY_ELEMENT,
  .vendor_id = MY_VENDOR_ID,
  .model_id = MY_MODEL_CLIENT_ID,
  .publish = 1,
  .opcodes_len = NUMBER_OF_OPCODES,
  .opcodes_data[0] = temperature_get,
  .opcodes_data[1] = temperature_status,
  .opcodes_data[2] = unit_get,
  .opcodes_data[3] = unit_set,
  .opcodes_data[4] = unit_set_unack,
  .opcodes_data[5] = unit_status,
  .opcodes_data[6] = update_interval_get,
  .opcodes_data[7] = update_interval_set,
  .opcodes_data[8] = update_interval_set_unack,
  .opcodes_data[9] = update_interval_status
};

#ifdef PROV_LOCALLY
static uint16_t uni_addr = 0;

static aes_key_128 enc_key = {
  .data = "\x03\x03\x03\x03\x03\x03\x03\x03\x03\x03\x03\x03\x03\x03\x03\x03"
};
#endif
/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/
static inline void stack_init(const gecko_configuration_t *config)
{
  linklayer_priorities.scan_max = linklayer_priorities.adv_min + 1;
  /* Move initialization here */
  gecko_stack_init(config);

  gecko_bgapi_class_dfu_init();
  gecko_bgapi_class_system_init();
  gecko_bgapi_class_le_gap_init();
  gecko_bgapi_class_le_connection_init();
  /* gecko_bgapi_class_gatt_init(); */
  gecko_bgapi_class_gatt_server_init();
#if (SDK < 140)
  gecko_bgapi_class_endpoint_init();
#endif
  gecko_bgapi_class_hardware_init();
  gecko_bgapi_class_flash_init();
  gecko_bgapi_class_test_init();
  /* gecko_bgapi_class_sm_init(); */

  gecko_bgapi_class_mesh_node_init();

  gecko_bgapi_class_mesh_proxy_init();
  gecko_bgapi_class_mesh_proxy_server_init();
  /* gecko_bgapi_class_mesh_proxy_client_init(); */

  gecko_bgapi_class_mesh_generic_client_init();
  gecko_bgapi_class_mesh_generic_server_init();

  gecko_bgapi_class_mesh_vendor_model_init();

  /* gecko_bgapi_class_mesh_health_client_init(); */
  /* gecko_bgapi_class_mesh_health_server_init(); */

  /* gecko_bgapi_class_mesh_friend_init(); */
  /* gecko_bgapi_class_mesh_lpn_init(); */

#ifdef PROV_LOCALLY
  gecko_bgapi_class_mesh_test_init();
#endif /* #ifdef PROV_LOCALLY */
  /* gecko_initCoexHAL(); */

  /* For LPN and BG13, remove comment for the next line */
  /* CMU_ClockEnable(cmuClock_RTCC, true); */

  /* Add user specific initialization here if any */
}

static void GPIO_Common_IRQHandler()
{
  uint32_t int_flag = GPIO_IntGet();
  GPIO_IntClear(int_flag);
  if (int_flag & (1 << BUTTON0_PIN)) {
    pb0_state = !GPIO_PinInGet(BUTTON0_PORT, BUTTON0_PIN);
    if (pb0_state == 1) {
      // press, start counting
      pb0_counter = RTCC_CounterGet();
    } else {
      pb0_counter = RTCC_CounterGet() - pb0_counter;
      if (pb0_counter > 32768) {
        gecko_external_signal(EX_PB0_LONG_PRESS);
      } else {
        gecko_external_signal(EX_PB0_PRESS);
      }
    }
  }
  if (int_flag & (1 << BUTTON1_PIN)) {
    pb1_state = !GPIO_PinInGet(BUTTON1_PORT, BUTTON1_PIN);
    if (pb1_state == 1) {
      // press, start counting
      pb1_counter = RTCC_CounterGet();
    } else {
      pb1_counter = RTCC_CounterGet() - pb1_counter;
      if (pb1_counter > 32768) {
        gecko_external_signal(EX_PB1_LONG_PRESS);
      } else {
        gecko_external_signal(EX_PB1_PRESS);
      }
    }
  }
}

void GPIO_EVEN_IRQHandler()
{
  GPIO_Common_IRQHandler();
}
void GPIO_ODD_IRQHandler()
{
  GPIO_Common_IRQHandler();
}

void gpio_init()
{
  // Enable GPIO clock.
  CMU_ClockEnable(cmuClock_GPIO, true);

  // Configure PB0 as input and enable interrupt.
  GPIO_PinModeSet(BUTTON0_PORT, BUTTON0_PIN, gpioModeInputPullFilter, 1);
  GPIO_IntConfig(BUTTON0_PORT, BUTTON0_PIN, true, true, true);

  // Configure PB1 as input and enable interrupt.
  GPIO_PinModeSet(BUTTON1_PORT, BUTTON1_PIN, gpioModeInputPullFilter, 1);
  GPIO_IntConfig(BUTTON1_PORT, BUTTON1_PIN, true, true, true);

  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);

  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);

  // Turn off by defaut
  GPIO_PinModeSet(LED0_PORT, LED0_PIN, gpioModePushPull, 0);
  GPIO_PinModeSet(LED1_PORT, LED1_PIN, gpioModePushPull, 0);
}

static void parse_period(uint8_t interval)
{
  switch (interval & RES_BIT_MASK) {
    case RES_100_MILLI:
      real_time_ticks = RES_100_MILLI_TICKS * (interval & (~RES_BIT_MASK));
      break;
    case RES_1_SEC:
      real_time_ticks = RES_1_SEC_TICKS * (interval & (~RES_BIT_MASK));
      break;
    case RES_10_SEC:
      real_time_ticks = RES_10_SEC_TICKS * (interval & (~RES_BIT_MASK));
      break;
    case RES_10_MIN:
      real_time_ticks = RES_10_MIN_TICKS * (interval & (~RES_BIT_MASK));
      break;
    default:
      break;
  }
  if (real_time_ticks) {
    LOGI("Update Period [hh:mm:ss:ms]= %02d:%02d:%02d:%04d\r\n",
         real_time_ticks / (32768 * 60 * 60),
         (real_time_ticks % (32768 * 60 * 60)) / (32768 * 60),
         (real_time_ticks % (32768 * 60)) / 32768,
         ((real_time_ticks % (32768)) / 3277) * 100);
  } else {
    LOGI("Periodical update off.\r\n");
  }
}

/* Delay a centain time then reset */
static void delay_reset(uint32_t ms)
{
  if (ms < 10) {
    ms = 10;
  }
  gecko_cmd_hardware_set_soft_timer(328 * (ms / 10), RESET_TIMER_ID, 1);
}

static void handle_gecko_generic(struct gecko_cmd_packet *evt)
{
  switch (BGLIB_MSG_ID(evt->header)) {
    case gecko_evt_system_external_signal_id: {
      uint8_t opcode = 0, length = 0, data = 0;
      struct gecko_msg_mesh_vendor_model_set_publication_rsp_t *set_pub_ret;
      struct gecko_msg_mesh_vendor_model_publish_rsp_t *pub_ret;
      if (evt->data.evt_system_external_signal.extsignals & EX_PB0_PRESS) {
        opcode = temperature_get;
        LOGD("PB0 Pressed.\r\n");
      }
      if (evt->data.evt_system_external_signal.extsignals & EX_PB0_LONG_PRESS) {
        opcode = update_interval_set_unack;
        length = 1;
        data = periods[index];
        //				LOGW("Period to set = %d\r\n", data);
        if (index == sizeof(periods) - 1) {
          index = 0;
        } else {
          index++;
        }
        LOGD("PB0 Long Pressed.\r\n");
      }
      if (evt->data.evt_system_external_signal.extsignals & EX_PB1_PRESS) {
        opcode = unit_get;
        LOGD("PB1 Pressed.\r\n");
      }
      if (evt->data.evt_system_external_signal.extsignals & EX_PB1_LONG_PRESS) {
        if (unit == celsius) {
          opcode = unit_set_unack;
          length = 1;
          data = fahrenheit;
        } else {
          opcode = unit_set;
          length = 1;
          data = celsius;
        }
        LOGD("PB1 Long Pressed.\r\n");
      }
      set_pub_ret = gecko_cmd_mesh_vendor_model_set_publication(my_model.elem_index,
                                                                my_model.vendor_id,
                                                                my_model.model_id,
                                                                opcode,
                                                                1,
                                                                length,
                                                                &data);
      if (set_pub_ret->result) {
        LOGE("Set publication error = 0x%04X\r\n", set_pub_ret->result);
        ERROR_ADDRESSING();
      } else {
        LOGD("Set publication done. Publishing...\r\n");
        pub_ret = gecko_cmd_mesh_vendor_model_publish(my_model.elem_index,
                                                      my_model.vendor_id,
                                                      my_model.model_id);
        if (pub_ret->result) {
          LOGE("Publish error = 0x%04X\r\n", pub_ret->result);
          ERROR_ADDRESSING();
        } else {
          LOGD("Publish done.\r\n");
        }
      }
    }
    break;
  }
}

static void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt);
bool mesh_bgapi_listener(struct gecko_cmd_packet *evt);

static void factory_reset(void)
{
  LOGW("Factory Reseting...\n");
  gecko_cmd_flash_ps_erase_all();
  gecko_cmd_hardware_set_soft_timer(32768 * 1, RESET_TIMER_ID, 1);
}

void AppHandler(void)
{
  stack_init(&config);

  /* Add extra initialization code here */
  gpio_init();
  INIT_LOG();

  CMU_ClockEnable(cmuClock_RTCC, true);
  while (1) {
    struct gecko_cmd_packet *evt = gecko_wait_event();
    bool pass = mesh_bgapi_listener(evt);
    handle_gecko_generic(evt);
    if (pass) {
      handle_gecko_event(BGLIB_MSG_ID(evt->header), evt);
    }
  }
}

static void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt)
{
  switch (evt_id) {
    case gecko_evt_system_boot_id:
      LOGD("<ROLE here> - System boot\n");
      if (!GPIO_PinInGet(BUTTON0_PORT, BUTTON0_PIN)
          || !GPIO_PinInGet(BUTTON1_PORT, BUTTON1_PIN)) {
        factory_reset();
        return;
      }
      SE_CALL(gecko_cmd_mesh_node_init());
      break;
    case gecko_evt_mesh_vendor_model_receive_id: {
      uint32_t temperature = 0;
      struct gecko_msg_mesh_vendor_model_receive_evt_t *recv_evt
        = (struct gecko_msg_mesh_vendor_model_receive_evt_t *) &evt->data;
      LOGD("Vendor model data received.\r\n\t"
           "Element index = %d\r\n\t"
           "Vendor id = 0x%04X\r\n\t"
           "Model id = 0x%04X\r\n\t"
           "Source address = 0x%04X\r\n\t"
           "Destination address = 0x%04X\r\n\t"
           "Destination label UUID index = 0x%02X\r\n\t"
           "App key index = 0x%04X\r\n\t"
           "Non-relayed = 0x%02X\r\n\t"
           "Opcode = 0x%02X\r\n\t"
           "Final = 0x%04X\r\n\t"
           "Payload: ",
           recv_evt->elem_index,
           recv_evt->vendor_id,
           recv_evt->model_id,
           recv_evt->source_address,
           recv_evt->destination_address,
           recv_evt->va_index,
           recv_evt->appkey_index,
           recv_evt->nonrelayed,
           recv_evt->opcode,
           recv_evt->final);
      UINT8_ARRAY_DUMP(recv_evt->payload.data, recv_evt->payload.len);
      LOGN();

      GPIO_PinOutToggle(LED1_PORT,
                        LED1_PIN);
      switch (recv_evt->opcode) {
        case temperature_status:
          temperature = *(uint32_t *) recv_evt->payload.data;
          LOGI("Temperature = %d.%d %s\r\n",
               temperature / 1000,
               temperature % 1000,
               unit == celsius ? (char * )"Celsius" : (char * )"Fahrenheit");
          break;
        case unit_status:
          unit = (unit_t) recv_evt->payload.data[0];
          LOGI("Unit = %s\r\n",
               unit == celsius ? (char * )"Celsius" : (char * )"Fahrenheit");
          break;
        case update_interval_status:
          update_interval = recv_evt->payload.data[0];
          /* LOGW("Period received = %d\r\n", update_interval); */
          parse_period(update_interval);
          break;
        /* Client should not response to other opcodes */
        default:
          break;
      }
    }
    break;

    case gecko_evt_hardware_soft_timer_id:
      switch (evt->data.evt_hardware_soft_timer.handle) {
        case RESET_TIMER_ID:
          LOGW("Reset Device\n");
          gecko_cmd_system_reset(0);
          break;
      }
      break;

    case gecko_evt_mesh_node_initialized_id:
    {
      struct gecko_msg_mesh_node_initialized_evt_t *ret
        = (struct gecko_msg_mesh_node_initialized_evt_t *)&evt->data;
      LOGD("Node Initialized...\n");
      struct gecko_msg_mesh_vendor_model_init_rsp_t *vm_init_ret;
      vm_init_ret = gecko_cmd_mesh_vendor_model_init(my_model.elem_index,
                                                     my_model.vendor_id,
                                                     my_model.model_id,
                                                     my_model.publish,
                                                     my_model.opcodes_len,
                                                     my_model.opcodes_data);
      if (vm_init_ret->result) {
        LOGE("Vendor model init error = 0x%04X\r\n", vm_init_ret->result);
        ERROR_ADDRESSING();
      } else {
        LOGI("Vendor model init done. --- ");
        LOGW("Client. \r\n");
      }
      /* Provision itself if not provisioned yet */
      if (ret->provisioned) {
        GPIO_PinOutSet(LED0_PORT,
                       LED0_PIN);
        LOGD("Node provisioned already.\n");
      } else {
        GPIO_PinOutClear(LED0_PORT,
                         LED0_PIN);
        LOGD("Node unprovisioned.\n");
#ifdef PROV_LOCALLY

        /* Derive the unicast address from the LSB 2 bytes from the BD_ADDR */
        struct gecko_msg_system_get_bt_address_rsp_t *bd_ret
          = gecko_cmd_system_get_bt_address();
        uni_addr = ((bd_ret->address.addr[1] << 8) | bd_ret->address.addr[0]) & 0x7FFF;
        LOGD("Unicast Address = 0x%04X\n", uni_addr);
        LOGD("Provisioning itself.\n");
        SE_CALL(gecko_cmd_mesh_node_set_provisioning_data(enc_key,
                                                          enc_key,
                                                          NET_KEY_IDX,
                                                          IVI,
                                                          uni_addr,
                                                          0));
        delay_reset(100);
        break;
#else
        LOGD("Sending unprovisioned device beacon\n");
        SE_CALL(gecko_cmd_mesh_node_start_unprov_beaconing(3));
#endif /* #ifdef PROV_LOCALLY */
      }

#ifdef PROV_LOCALLY
      /* Set the publication and subscription */
      struct gecko_msg_mesh_test_get_local_model_pub_rsp_t *pub_setting
        = gecko_cmd_mesh_test_get_local_model_pub(my_model.elem_index,
                                                  my_model.vendor_id,
                                                  my_model.model_id);
      if (!pub_setting->result && pub_setting->pub_address == SERVER_PUB_ADDR) {
        LOGD("Configuration done already.\n");
      } else {
        LOGD("Pub setting result = 0x%04X, pub setting address = 0x%04X\n",
             pub_setting->result, pub_setting->pub_address);
        LOGD("Add local app key...\n");
        SE_CALL(gecko_cmd_mesh_test_add_local_key(1,
                                                  enc_key,
                                                  APP_KEY_IDX,
                                                  NET_KEY_IDX));
        LOGD("Bind local app key...\n");
        SE_CALL(gecko_cmd_mesh_test_bind_local_model_app(my_model.elem_index,
                                                         APP_KEY_IDX,
                                                         my_model.vendor_id,
                                                         my_model.model_id));
        LOGD("Set local model pub...\n");
        SE_CALL(gecko_cmd_mesh_test_set_local_model_pub(my_model.elem_index,
                                                        APP_KEY_IDX,
                                                        my_model.vendor_id,
                                                        my_model.model_id,
                                                        SERVER_SUB_ADDR,
                                                        DEFAULT_TTL,
                                                        0, 0, 0));
        LOGD("Add local model sub...\n");
        SE_CALL(gecko_cmd_mesh_test_add_local_model_sub(my_model.elem_index,
                                                        my_model.vendor_id,
                                                        my_model.model_id,
                                                        SERVER_PUB_ADDR));
        LOGD("Set relay...\n");
        SE_CALL(gecko_cmd_mesh_test_set_relay(1, 0, 0));
        LOGD("Set Network tx state.\n");
        SE_CALL(gecko_cmd_mesh_test_set_nettx(2, 4));
        /* LOGD("add local model sub...\n"); */
        /* SE_CALL(gecko_cmd_mesh_test_add_local_model_sub(0, my_model.vendor_id, MODEL_ID, DUT_SUB_ADDR)); */
      }
#endif /* #ifdef PROV_LOCALLY */
      break;
    }

    case gecko_evt_mesh_node_provisioned_id: {
      GPIO_PinOutSet(LED0_PORT,
                     LED0_PIN);
      LOGI("Provisioning done.\r\n");
    }
    break;

    case gecko_evt_mesh_node_provisioning_failed_id:
      LOGW("Provisioning failed. Result = 0x%04x\r\n",
           evt->data.evt_mesh_node_provisioning_failed.result);
      break;

    case gecko_evt_mesh_node_provisioning_started_id:
      LOGI("Provisioning started.\r\n");
      break;

    case gecko_evt_mesh_node_key_added_id:
      LOGI("got new %s key with index %x\r\n",
           evt->data.evt_mesh_node_key_added.type == 0 ? "network" : "application",
           evt->data.evt_mesh_node_key_added.index);
      break;

    case gecko_evt_mesh_node_config_set_id: {
      LOGD("gecko_evt_mesh_node_config_set_id\r\n\t");
      struct gecko_msg_mesh_node_config_set_evt_t *set_evt
        = (struct gecko_msg_mesh_node_config_set_evt_t *) &evt->data;
      UINT8_ARRAY_DUMP(set_evt->value.data, set_evt->value.len);
    }
    break;
    case gecko_evt_mesh_node_model_config_changed_id:
      LOGI("model config changed\r\n");
      break;

    case gecko_evt_le_connection_closed_id:
      /* Check if need to boot to dfu mode */
      break;
    default:
      break;
  }
}
